#pragma once
#include "../AimbotCommon/AimbotCommon.h"
#include "../../LagRecords/LagRecords.h"

class CAimbotHitscan
{
	struct HitscanTarget_t : AimTarget_t
	{
		int AimedHitbox = -1;
		float SimulationTime = -1.0f;
		const LagRecord_t* LagRecord = nullptr;
		bool WasMultiPointed = false;
	};

	std::vector<HitscanTarget_t> m_vecTargets = {};

	int GetAimHitbox(C_TFWeaponBase* pWeapon);
	bool ScanHead(C_TFPlayer* pLocal, HitscanTarget_t& target);
	bool ScanBody(C_TFPlayer* pLocal, HitscanTarget_t& target);
	bool ScanBuilding(C_TFPlayer* pLocal, HitscanTarget_t& target);
	bool GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget);
	bool ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
	void Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, const Vec3& vAngles);
	bool ShouldFire(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const HitscanTarget_t& target);
	void HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);

public:
	bool IsFiring(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
	void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotHitscan, AimbotHitscan, F);
