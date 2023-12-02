#pragma once

#include "../../../SDK/SDK.h"

class CMisc
{
public:
	void Bunnyhop(CUserCmd* pCmd);
	void AutoStrafer(CUserCmd* pCmd);
	void NoiseMakerSpam();
	void FastStop(CUserCmd* pCmd);

	void AutoRocketJump(CUserCmd* cmd);
	void AutoDisguise(CUserCmd* cmd);
	void AutoMedigun(CUserCmd* cmd);
	void MovementLock(CUserCmd* cmd);
	void MvmInstaRespawn();
};

MAKE_SINGLETON_SCOPED(CMisc, Misc, F);
