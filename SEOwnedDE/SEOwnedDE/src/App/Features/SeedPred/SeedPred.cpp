#include "SeedPred.h"

#include "../CFG.h"

float CalcMantissaStep(float val)
{
	const auto rawVal{reinterpret_cast<int&>(val)};
	const auto exponent{(rawVal >> 23) & 0xFF};

	const auto result{powf(2.0f, static_cast<float>(exponent - (127 + 23))) * 1000.0f};

	static std::vector<float> mantissas{};

	if (mantissas.empty())
	{
		auto mantissa{1.0f};

		for (auto n{0}; n < 16; n++)
		{
			mantissas.push_back(mantissa);

			mantissa *= 2.0f;
		}
	}

	auto closest = [](const std::vector<float>& vec, float value)
	{
		const auto it{std::ranges::lower_bound(vec, value)};
		if (it == vec.end())
		{
			return value;
		}

		return *it;
	};

	return closest(mantissas, result);
}

void CSeedPred::AskForPlayerPerf()
{
	if (!CFG::Exploits_SeedPred_Active)
	{
		Reset();
		return;
	}

	const auto weapon{H::Entities->GetWeapon()};
	if (!weapon || !(weapon->GetDamageType() & DMG_BULLET) || H::AimUtils->GetWeaponType(weapon) != EWeaponType::HITSCAN || weapon->GetWeaponSpread() <= 0.0f)
	{
		Reset();
		return;
	}

	if (C_TFPlayer* local{H::Entities->GetLocal()})
	{
		if (local->deadflag())
		{
			Reset();
			return;
		}
	}

	if (m_WaitingForPP)
	{
		return;
	}

	I::ClientState->SendStringCmd("playerperf");

	m_AskTime = static_cast<float>(Plat_FloatTime());

	m_WaitingForPP = true;
}

bool CSeedPred::ParsePlayerPerf(bf_read& msgData)
{
	if (!CFG::Exploits_SeedPred_Active)
	{
		return false;
	}

	char rawMsg[256]{};

	msgData.ReadString(rawMsg, sizeof(rawMsg), true);
	msgData.Seek(0);

	std::string msg(rawMsg);

	msg.erase(msg.begin()); //STX

	std::smatch matches{};

	std::regex_match(msg, matches, std::regex(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)"));

	if (matches.size() == 2)
	{
		m_WaitingForPP = false;

		//credits to kgb for idea

		const float newServerTime{std::stof(matches[1].str())};

		if (newServerTime > m_ServerTime)
		{
			m_PrevServerTime = m_ServerTime;

			m_ServerTime = newServerTime;

			m_ResponseTime = static_cast<float>(Plat_FloatTime() - m_AskTime);

			//if (!m_Synced)
			{
				if (m_PrevServerTime > 0.0f)
				{
					if (m_GuessTime > 0.0f)
					{
						const float delta{m_ServerTime - m_GuessTime};

						if (delta == 0.0f)
						{
							m_Synced = true;

							m_SyncOffset = m_GuessDelta;
						}
					}

					m_GuessDelta = m_ServerTime - m_PrevServerTime;

					m_GuessTime = m_ServerTime + (m_GuessDelta);
				}
			}
		}

		return true;
	}

	return std::regex_match(msg, std::regex(R"(\d+.\d+\s\d+\s\d+)"));
}

int CSeedPred::GetSeed()
{
	float time{(m_ServerTime + m_SyncOffset + m_ResponseTime) * 1000.0f};

	return *reinterpret_cast<int*>(reinterpret_cast<char*>(&time)) & 255;
}

void CSeedPred::Reset()
{
	m_Synced = false;
	m_ServerTime = 0.0f;
	m_PrevServerTime = 0.0f;
	m_AskTime = 0.0f;
	m_GuessTime = 0.0f;
	m_SyncOffset = 0.0f;
	m_WaitingForPP = false;
	m_GuessDelta = 0.0f;
	m_ResponseTime = 0.0f;
}

void CSeedPred::AdjustAngles(CUserCmd* cmd)
{
	if (!CFG::Exploits_SeedPred_Active || !m_Synced || !cmd || !G::bFiring)
	{
		return;
	}

	const auto local{H::Entities->GetLocal()};
	if (!local)
	{
		return;
	}

	const auto weapon{H::Entities->GetWeapon()};
	if (!weapon || !(weapon->GetDamageType() & DMG_BULLET))
	{
		return;
	}

	const auto spread{weapon->GetWeaponSpread()};
	if (spread <= 0.0f)
	{
		return;
	}

	auto bulletsPerShot{weapon->GetWeaponInfo()->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_nBulletsPerShot};
	bulletsPerShot = static_cast<int>(SDKUtils::AttribHookValue(static_cast<float>(bulletsPerShot), "mult_bullets_per_shot", weapon));

	//credits to cathook for average spread stuff

	std::vector<Vec3> bulletCorrections{};
	Vec3 averageSpread{};
	auto seed{GetSeed()};

	for (auto bullet{0}; bullet < bulletsPerShot; bullet++)
	{
		SDKUtils::RandomSeed(seed++);

		auto firePerfect{false};

		if (bullet == 0)
		{
			const auto timeSinceLastShot{(local->m_nTickBase() * TICK_INTERVAL) - weapon->m_flLastFireTime()};

			if (bulletsPerShot > 1 && timeSinceLastShot > 0.25f)
			{
				firePerfect = true;
			}
			else if (bulletsPerShot == 1 && timeSinceLastShot > 1.25f)
			{
				firePerfect = true;
			}
		}

		if (firePerfect)
		{
			return;
		}

		const auto x{SDKUtils::RandomFloat(-0.5f, 0.5f) + SDKUtils::RandomFloat(-0.5f, 0.5f)};
		const auto y{SDKUtils::RandomFloat(-0.5f, 0.5f) + SDKUtils::RandomFloat(-0.5f, 0.5f)};

		Vec3 forward{}, right{}, up{};

		Math::AngleVectors(cmd->viewangles, &forward, &right, &up);

		Vector fixedSpread{forward + (right * x * spread) + (up * y * spread)};

		fixedSpread.NormalizeInPlace();

		averageSpread += fixedSpread;

		bulletCorrections.push_back(fixedSpread);
	}

	averageSpread /= static_cast<float>(bulletsPerShot);

	Vec3 fixedSpread{FLT_MAX, FLT_MAX, FLT_MAX};

	for (const auto& curSpread : bulletCorrections)
	{
		if (curSpread.DistTo(averageSpread) < fixedSpread.DistTo(averageSpread))
		{
			fixedSpread = curSpread;
		}
	}

	Vec3 fixedAngles{};
	Math::VectorAngles(fixedSpread, fixedAngles);

	const Vec3 correction{cmd->viewangles - fixedAngles};
	cmd->viewangles += correction;
	Math::ClampAngles(cmd->viewangles);

	G::bSilentAngles = true;
}

void CSeedPred::Paint()
{
	if (!CFG::Exploits_SeedPred_Active || I::EngineVGui->IsGameUIVisible() || SDKUtils::BInEndOfMatch() || m_ServerTime <= 0.0f)
	{
		return;
	}

	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	if (CFG::Exploits_SeedPred_DrawIndicator)
	{
		const std::chrono::hh_mm_ss time{std::chrono::seconds(static_cast<int>(m_ServerTime))};

		int x{2};
		int y{2};

		H::Draw->String
		(
			H::Fonts->Get(EFonts::ESP_SMALL),
			x, y,
			{200, 200, 200, 255}, POS_DEFAULT,
			std::format("{}h {}m {}s (step {:.0f})", time.hours().count(), time.minutes().count(), time.seconds().count(), CalcMantissaStep(m_ServerTime)).c_str()
		);

		y += 10;

		H::Draw->String
		(
			H::Fonts->Get(EFonts::ESP_SMALL),
			x, y,
			!m_Synced ? Color_t{250, 130, 49, 255} : Color_t{32, 191, 107, 255}, POS_DEFAULT,
			!m_Synced ? "syncing.." : std::format("synced ({})", m_SyncOffset).c_str()
		);
	}
}
