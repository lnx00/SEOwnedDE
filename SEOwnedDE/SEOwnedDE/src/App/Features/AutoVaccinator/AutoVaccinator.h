#pragma once

#include "../../../SDK/SDK.h"

class CAutoVaccinator
{
public:
	void run(CUserCmd *cmd);
	void processPlayerHurt(IGameEvent *event);
	void preventReload(CUserCmd *cmd);
};

MAKE_SINGLETON_SCOPED(CAutoVaccinator, AutoVaccinator, F);