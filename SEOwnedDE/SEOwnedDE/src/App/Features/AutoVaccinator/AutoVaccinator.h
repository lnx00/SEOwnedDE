#pragma once

#include "../../../SDK/SDK.h"

class CAutoVaccinator
{
public:
	void Run(CUserCmd *cmd);
	void ProcessPlayerHurt(IGameEvent *event);
	void PreventReload(CUserCmd *cmd);
};

MAKE_SINGLETON_SCOPED(CAutoVaccinator, AutoVaccinator, F);