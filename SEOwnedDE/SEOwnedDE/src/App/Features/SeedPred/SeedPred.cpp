#include "SeedPred.h"

#include "../CFG.h"

float CalcMantissaStep(float val)
{
	// Calculate the delta to the next representable value
	const float nextValue = std::nextafter(val, std::numeric_limits<float>::infinity());
	float mantissaStep = nextValue - val;
	mantissaStep *= 1000.f;

	// Calculate a lookup table for the steps
	static const std::vector<float> MANTISSAS = []
	{
		std::vector<float> result;
		result.reserve(16);
		for (int i = 0; i < 16; i++)
		{
			result.push_back(std::powf(2, i));
		}

		return result;
	}();

	// Get the closest mantissa
	const auto it = std::ranges::lower_bound(MANTISSAS, mantissaStep);
	const float closestResult = (it == MANTISSAS.end()) ? mantissaStep : *it;

	return closestResult;
}

void CSeedPred::AskForPlayerPerf()
{
	if (!CFG::Exploits_SeedPred_Active)
	{
		Reset();
		return;
	}

	// Does the current weapon have spread?
	const auto weapon = H::Entities->GetWeapon();
	if (!weapon || !(weapon->GetDamageType() & DMG_BULLET) || H::AimUtils->GetWeaponType(weapon) != EWeaponType::HITSCAN || weapon->GetWeaponSpread() <= 0.0f)
	{
		Reset();
		return;
	}

	// Are we dead?
	if (const auto local = H::Entities->GetLocal())
	{
		if (local->deadflag())
		{
			Reset();
			return;
		}
	}

	// Are we already waiting? | TODO: Add timer so it doesn't eat CPU...
	if (m_WaitingForPP)
	{
		return;
	}

	// Request perf data
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
						// Is our server time synchronized?
						const float delta = m_ServerTime - m_GuessTime;
						if (delta == 0.0f)
						{
							m_Synced = true;
							m_SyncOffset = m_GuessDelta;
						}
					}

					// Update the guessed delta & time
					m_GuessDelta = m_ServerTime - m_PrevServerTime;
					m_GuessTime = m_ServerTime + m_GuessDelta;
				}
			}
		}

		return true;
	}

	return std::regex_match(msg, std::regex(R"(\d+.\d+\s\d+\s\d+)"));
}

int CSeedPred::GetSeed()
{
	const float time = (m_ServerTime + m_SyncOffset + m_ResponseTime) * 1000.0f;
	const auto seed = std::bit_cast<int32_t>(time) & 255;
	return seed;
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
		return;

	const auto weapon{H::Entities->GetWeapon()};
	if (!weapon || !(weapon->GetDamageType() & DMG_BULLET))
		return;

	// Does the weapon have spread?
	const auto spread{weapon->GetWeaponSpread()};
	if (spread <= 0.0f)
		return;

	auto bulletsPerShot{weapon->GetWeaponInfo()->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_nBulletsPerShot};
	bulletsPerShot = static_cast<int>(SDKUtils::AttribHookValue(static_cast<float>(bulletsPerShot), "mult_bullets_per_shot", weapon));

	//credits to cathook for average spread stuff

	std::vector<Vec3> bulletCorrections{};
	Vec3 averageSpread{};
	int seed = GetSeed();

	for (int bullet = 0; bullet < bulletsPerShot; bullet++)
	{
		SDKUtils::RandomSeed(seed++);

		// Check if we'll get a guaranteed perfect shot
		if (bullet == 0)
		{
			const auto timeSinceLastShot = (local->m_nTickBase() * TICK_INTERVAL) - weapon->m_flLastFireTime();

			if ((bulletsPerShot == 1 && timeSinceLastShot > 1.25f) || (bulletsPerShot > 1 && timeSinceLastShot > 0.25f))
			{
				return;
			}
		}

		// No perfect shot. Let's guess the spread!
		const auto x{SDKUtils::RandomFloat(-0.5f, 0.5f) + SDKUtils::RandomFloat(-0.5f, 0.5f)};
		const auto y{SDKUtils::RandomFloat(-0.5f, 0.5f) + SDKUtils::RandomFloat(-0.5f, 0.5f)};

		Vec3 forward{}, right{}, up{};
		Math::AngleVectors(cmd->viewangles, &forward, &right, &up);

		// Calculate the spread vector
		Vec3 fixedSpread = forward + (right * x * spread) + (up * y * spread);
		fixedSpread.NormalizeInPlace();
		averageSpread += fixedSpread;

		bulletCorrections.push_back(fixedSpread);
	}

	averageSpread /= static_cast<float>(bulletsPerShot);

	// Find the closest spread to average
	const auto fixedSpread = std::ranges::min_element(bulletCorrections,
	                                                  [&](const Vec3& lhs, const Vec3& rhs)
	                                                  {
		                                                  return lhs.DistTo(averageSpread) < rhs.DistTo(averageSpread);
	                                                  });

	// Is there a minimum?
	if (fixedSpread == bulletCorrections.end())
	{
		return;
	}

	Vec3 fixedAngles{};
	Math::VectorAngles(*fixedSpread, fixedAngles);

	// Apply the spread correction
	const Vec3 correction = cmd->viewangles - fixedAngles;
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

	// Anti-Screenshot
	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	// Indicator
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

		y += 10;

		H::Draw->String
		(
			H::Fonts->Get(EFonts::ESP_SMALL),
			x, y,
			{200, 200, 200, 255}, POS_DEFAULT,
			std::format("seed: {}", GetSeed()).c_str()
		);
	}
}
