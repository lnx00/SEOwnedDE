#pragma once

#include "../../../SDK/SDK.h"

class CMiscVisuals
{
public:
	void AimbotFOVCircle();
	void ViewModelSway();
	void DetailProps();
	void ShiftBar();

	void SniperLines();
	void ProjectileArc();
};

MAKE_SINGLETON_SCOPED(CMiscVisuals, MiscVisuals, F);
