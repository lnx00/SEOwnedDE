#pragma once

#include "../../../SDK/SDK.h"

class CRapidFire
{
	CUserCmd m_ShiftCmd = {};
	bool m_bShiftSilentAngles = false;
	bool m_bSetCommand = false;

	bool ShouldStart(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);

public:
	void Run(CUserCmd* pCmd, bool* pSendPacket);
	bool ShouldExitCreateMove(CUserCmd* pCmd);
	bool IsWeaponSupported(C_TFWeaponBase* pWeapon);

	bool GetShiftSilentAngles() { return m_bShiftSilentAngles; }
};

MAKE_SINGLETON_SCOPED(CRapidFire, RapidFire, F);
