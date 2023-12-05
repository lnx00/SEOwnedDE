#pragma once

#include "../../../SDK/SDK.h"

class CSeedPred
{
public:
	void AskForPlayerPerf();
	bool ParsePlayerPerf(bf_read& msgData);
	int GetSeed();
	void Reset();
	void AdjustAngles(CUserCmd* cmd);
	void Paint();
};

MAKE_SINGLETON_SCOPED(CSeedPred, SeedPred, F);
