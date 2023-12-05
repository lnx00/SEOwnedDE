#pragma once

#include "../../../SDK/SDK.h"

class CSpectatorList
{
	struct Spectator_t
	{
		std::wstring Name = {};
		int m_nMode = 0;
	};

	std::vector<Spectator_t> m_vecSpectators = {};

	bool GetSpectators();
	void Drag();

public:
	void Run();
};

MAKE_SINGLETON_SCOPED(CSpectatorList, SpectatorList, F);
