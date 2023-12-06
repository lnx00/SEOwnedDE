#pragma once

#include "../../../SDK/SDK.h"

class CSpyCamera
{
	void Drag();

public:
	void Run();
};

MAKE_SINGLETON_SCOPED(CSpyCamera, SpyCamera, F);
