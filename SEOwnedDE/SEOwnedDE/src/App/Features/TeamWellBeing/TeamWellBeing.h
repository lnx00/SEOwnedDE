#pragma once

#include "../../../SDK/SDK.h"

class CTeamWellBeing
{
	void Drag();

public:
	void Run();
};

MAKE_SINGLETON_SCOPED(CTeamWellBeing, TeamWellBeing, F);
