#include "Players.h"

void CPlayers::Parse()
{
	// Init player data file path
	if (m_LogPath.empty())
	{
		m_LogPath = U::Storage->GetWorkFolder() / "players.json";

		if (!exists(m_LogPath))
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

	// Open the file
	std::ifstream logFile(m_LogPath);
	if (!logFile.is_open() || logFile.peek() == std::ifstream::traits_type::eof())
	{
		return;
	}

	// Load all players
	nlohmann::json j = nlohmann::json::parse(logFile);
	for (const auto& item : j.items())
	{
		m_Players.emplace_back(
			HASH_RT(item.key().c_str()),
			PlayerPriority{
				j[item.key()]["ignored"].get<bool>(),
				j[item.key()]["cheater"].get<bool>(),
				j[item.key()]["retardlegit"].get<bool>()
			}
		);
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
	auto steamID = HASH_RT(std::string_view(playerInfo.guid).data());

	for (auto& pl : m_Players)
	{
		if (pl.SteamID != steamID)
		{
			continue;
		}

		ptr = &pl;
		break;
	}

	if (!ptr)
	{
		m_Players.push_back({ steamID, info });
		ptr = &m_Players.back();
	}

	ptr->Info = info;

	// Load the current playerlist
	nlohmann::json j{};
	std::ifstream readFile(m_LogPath);
	if (readFile.is_open() && readFile.peek() != std::ifstream::traits_type::eof())
	{
		readFile >> j;
	}

	readFile.close();

	// Open the output file
	std::ofstream outFile(m_LogPath);
	if (!outFile.is_open())
	{
		return;
	}

	//auto key = std::string(playerInfo.guid);
	auto& playerEntry = j[playerInfo.guid];

	playerEntry["ignored"] = ptr->Info.Ignored;
	playerEntry["cheater"] = ptr->Info.Cheater;
	playerEntry["retardlegit"] = ptr->Info.RetardLegit;

	if (!ptr->Info.Ignored && !ptr->Info.Cheater && !ptr->Info.RetardLegit)
	{
		j.erase(std::string(playerInfo.guid));
	}

	outFile << std::setw(4) << j;
	outFile.close();
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

	const auto steamID = HASH_RT(std::string_view(playerInfo.guid).data());
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
	const auto steamID = HASH_RT(guid.c_str());
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
