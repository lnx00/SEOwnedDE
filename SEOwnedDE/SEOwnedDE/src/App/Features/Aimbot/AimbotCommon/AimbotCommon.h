#pragma once
#include "../../../../SDK/SDK.h"

struct AimTarget_t
{
	C_BaseEntity* Entity = nullptr;
    Vec3 Position = {};
    Vec3 AngleTo = {};
	float FOVTo = 0.0f;
	float DistanceTo = 0.0f;
};
