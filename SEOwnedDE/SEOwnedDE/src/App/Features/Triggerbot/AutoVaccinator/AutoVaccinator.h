#pragma once

#include "../../../../SDK/SDK.h"

class CAutoVaccinator
{
	int m_SimResType = MEDIGUN_NUM_RESISTS;
	int m_GoalResType = MEDIGUN_BULLET_RESIST;
	bool m_IsChangingRes = false;
	bool m_ShouldPop = false;

	void Reset();

public:
	void Run(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* cmd);
	void ProcessPlayerHurt(IGameEvent* event);
	void PreventReload(CUserCmd* cmd);
};

MAKE_SINGLETON_SCOPED(CAutoVaccinator, AutoVaccinator, F);
