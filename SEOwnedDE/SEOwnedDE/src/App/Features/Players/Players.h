#pragma once

#include "../../../SDK/SDK.h"

struct PlayerPriority
{
	bool Ignored{};
	bool Cheater{};
	bool RetardLegit{};
};

class CPlayers
{
	struct Player
	{
		hash::hash_t SteamID = {};
		PlayerPriority Info = {};
	};

	std::unordered_map<hash::hash_t, PlayerPriority> m_Players;
	std::filesystem::path m_LogPath;

public:
	void Parse();
	void Mark(int entindex, const PlayerPriority& info);
	bool GetInfo(int entindex, PlayerPriority& out);
	bool GetInfoGUID(const std::string& guid, PlayerPriority& out);
};

MAKE_SINGLETON_SCOPED(CPlayers, Players, F);
