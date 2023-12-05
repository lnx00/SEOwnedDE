#include "SeedPred.h"

#include "../CFG.h"

bool synced{false};
float server_time{0.0f};
float prev_server_time{0.0f};
float ask_time{0.0f};
float guess_time{0.0f};
float sync_offset{0.0f};
bool waiting_for_pp{false};
float guess_delta{0.0f};
float response_time{0.0f};

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

	if (waiting_for_pp)
	{
		return;
	}

	I::ClientState->SendStringCmd("playerperf");

	ask_time = static_cast<float>(Plat_FloatTime());

	waiting_for_pp = true;
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
		waiting_for_pp = false;

		//credits to kgb for idea

		const float newServerTime{std::stof(matches[1].str())};

		if (newServerTime > server_time)
		{
			prev_server_time = server_time;

			server_time = newServerTime;

			response_time = static_cast<float>(Plat_FloatTime() - ask_time);

			//if (!synced)
			{
				if (prev_server_time > 0.0f)
				{
					if (guess_time > 0.0f)
					{
						const float delta{server_time - guess_time};

						if (delta == 0.0f)
						{
							synced = true;

							sync_offset = guess_delta;
						}
					}

					guess_delta = server_time - prev_server_time;

					guess_time = server_time + (guess_delta);
				}
			}
		}

		return true;
	}

	return std::regex_match(msg, std::regex(R"(\d+.\d+\s\d+\s\d+)"));
}

int CSeedPred::GetSeed()
{
	float time{(server_time + sync_offset + response_time) * 1000.0f};

	return *reinterpret_cast<int*>(reinterpret_cast<char*>(&time)) & 255;
}

void CSeedPred::Reset()
{
	synced = false;
	server_time = 0.0f;
	prev_server_time = 0.0f;
	ask_time = 0.0f;
	guess_time = 0.0f;
	sync_offset = 0.0f;
	waiting_for_pp = false;
	guess_delta = 0.0f;
	response_time = 0.0f;
}

void CSeedPred::AdjustAngles(CUserCmd* cmd)
{
	if (!CFG::Exploits_SeedPred_Active || !synced || !cmd || !G::bFiring)
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

	for (const auto& spread : bulletCorrections)
	{
		if (spread.DistTo(averageSpread) < fixedSpread.DistTo(averageSpread))
		{
			fixedSpread = spread;
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
	if (!CFG::Exploits_SeedPred_Active || I::EngineVGui->IsGameUIVisible() || SDKUtils::BInEndOfMatch() || server_time <= 0.0f)
	{
		return;
	}

	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	if (CFG::Exploits_SeedPred_DrawIndicator)
	{
		auto calcMantissaStep = [](float val)
		{
			auto raw_val{reinterpret_cast<int&>(val)};
			auto exponent{(raw_val >> 23) & 0xFF};

			auto result{powf(2.0f, static_cast<float>(exponent - (127 + 23))) * 1000.0f};

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
				auto it{std::lower_bound(vec.begin(), vec.end(), value)};

				if (it == vec.end())
				{
					return value;
				}

				return *it;
			};

			return closest(mantissas, result);
		};

		std::chrono::hh_mm_ss time{std::chrono::seconds(static_cast<int>(server_time))};

		int x{2};
		int y{2};

		H::Draw->String
		(
			H::Fonts->Get(EFonts::ESP_SMALL),
			x, y,
			{200, 200, 200, 255}, POS_DEFAULT,
			std::format("{}h {}m {}s (step {:.0f})", time.hours().count(), time.minutes().count(), time.seconds().count(), calcMantissaStep(server_time)).c_str()
		);

		y += 10;

		H::Draw->String
		(
			H::Fonts->Get(EFonts::ESP_SMALL),
			x, y,
			!synced ? Color_t{250, 130, 49, 255} : Color_t{32, 191, 107, 255}, POS_DEFAULT,
			!synced ? "syncing.." : std::format("synced ({})", sync_offset).c_str()
		);
	}
}
