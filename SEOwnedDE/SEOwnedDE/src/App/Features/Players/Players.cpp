#include "Players.h"

void CPlayers::Parse()
{
	if (m_LogPath.empty())
	{
		m_LogPath = std::filesystem::current_path().string() + "\\SEOwnedDE\\";

		if (!std::filesystem::exists(m_LogPath))
		{
			std::filesystem::create_directories(m_LogPath);
		}

		m_LogPath += "players.json";

		if (!std::filesystem::exists(m_LogPath))
		{
			std::ofstream file(m_LogPath, std::ios::app);

			if (!file.is_open())
			{
				return;
			}

			file.close();
		}
	}

	if (!m_Players.empty())
	{
		return;
	}

	std::ifstream file(m_LogPath);

	if (!file.is_open() || file.peek() == std::ifstream::traits_type::eof())
	{
		return;
	}

	nlohmann::json j{};

	file >> j;

	for (const auto& item : j.items())
	{
		Player p
		{
			HASH_RT(item.key().c_str()),
			{
				j[item.key()]["ignored"].get<bool>(),
				j[item.key()]["cheater"].get<bool>(),
				j[item.key()]["retardlegit"].get<bool>()
			}
		};

		m_Players.push_back(p);
	}
}

void CPlayers::Mark(int entindex, const PlayerPriority& info)
{
	if (entindex == I::EngineClient->GetLocalPlayer())
	{
		return;
	}

	player_info_t playerInfo{};

	if (!I::EngineClient->GetPlayerInfo(entindex, &playerInfo) || playerInfo.fakeplayer)
	{
		return;
	}

	Player* ptr{};

	auto steam_id{HASH_RT(std::string_view(playerInfo.guid).data())};

	for (auto& pl : m_Players)
	{
		if (pl.SteamID != steam_id)
		{
			continue;
		}

		ptr = &pl;

		break;
	}

	if (!ptr)
	{
		m_Players.push_back({steam_id, info});

		ptr = &m_Players.back();
	}

	ptr->Info = info;

	nlohmann::json j{};

	std::ifstream readFile(m_LogPath);

	if (readFile.is_open() && readFile.peek() != std::ifstream::traits_type::eof())
	{
		readFile >> j;
	}

	readFile.close();

	std::ofstream file(m_LogPath);

	if (!file.is_open())
	{
		return;
	}

	auto key{std::string(playerInfo.guid)};

	j[key]["ignored"] = ptr->Info.Ignored;
	j[key]["cheater"] = ptr->Info.Cheater;
	j[key]["retardlegit"] = ptr->Info.RetardLegit;

	if (!ptr->Info.Ignored && !ptr->Info.Cheater && !ptr->Info.RetardLegit)
	{
		j.erase(std::string(playerInfo.guid));
	}

	file << std::setw(4) << j;

	file.close();
}

bool CPlayers::GetInfo(int entindex, PlayerPriority& out)
{
	if (entindex == I::EngineClient->GetLocalPlayer())
	{
		return false;
	}

	player_info_t playerInfo{};

	if (!I::EngineClient->GetPlayerInfo(entindex, &playerInfo) || playerInfo.fakeplayer)
	{
		return false;
	}

	const auto steamID{HASH_RT(std::string_view(playerInfo.guid).data())};

	for (const auto& pl : m_Players)
	{
		if (pl.SteamID != steamID)
		{
			continue;
		}

		out = pl.Info;

		return true;
	}

	return false;
}

bool CPlayers::GetInfoGUID(const std::string& guid, PlayerPriority& out)
{
	const auto steamID{HASH_RT(guid.c_str())};

	for (const auto& pl : m_Players)
	{
		if (pl.SteamID != steamID)
		{
			continue;
		}

		out = pl.Info;

		return true;
	}

	return false;
}
