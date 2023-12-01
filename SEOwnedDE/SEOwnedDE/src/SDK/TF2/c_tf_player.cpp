#include "c_tf_player.h"

#include "../../App/Features/Players/Players.h"

bool C_TFPlayer::IsPlayerOnSteamFriendsList()
{
	auto result{ reinterpret_cast<bool(__thiscall *)(void *, void *)>(Signatures::C_TFPlayer_IsPlayerOnSteamFriendsList.Get())(this, this) };

	if (!result)
	{
		CPlayers::PlayerInfo info{};

		return F::Players->getInfo(entindex(), info) && info.m_ignored;
	}

	return result;
}