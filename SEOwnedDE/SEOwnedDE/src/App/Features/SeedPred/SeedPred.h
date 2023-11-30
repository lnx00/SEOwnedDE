#pragma once

#include "../../../SDK/SDK.h"

class CSeedPred
{
public:
	void askForPlayerPerf();
	bool parsePlayerPerf(bf_read &msg_data);
	int getSeed();
	void reset();
	void adjustAngles(CUserCmd *cmd);
	void paint();
};

MAKE_SINGLETON_SCOPED(CSeedPred, SeedPred, F);