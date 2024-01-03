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

class CAimbotCommon
{
public:
	template <typename T, typename = std::enable_if<std::is_base_of_v<AimTarget_t, T>>>
	void Sort(std::vector<T>& targets, int sortMode)
	{
		std::ranges::sort(targets, [&](const AimTarget_t& a, const AimTarget_t& b)
		{
			switch (sortMode)
			{
				case 0: return a.FOVTo < b.FOVTo;
				case 1: return a.DistanceTo < b.DistanceTo;
				default: return false;
			}
		});
	}
};

MAKE_SINGLETON_SCOPED(CAimbotCommon, AimbotCommon, F);
