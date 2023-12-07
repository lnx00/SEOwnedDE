#pragma once

#include "../../../SDK/SDK.h"

class CStickyJump
{
public:
	void Run(CUserCmd* cmd);
};

MAKE_SINGLETON_SCOPED(CStickyJump, StickyJump, F);
