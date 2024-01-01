#pragma once

#include "../../LagRecords/LagRecords.h"

class CAimbotMelee
{
	struct Target_t
	{
		C_BaseEntity* m_pEntity = nullptr;
		Vec3 m_vPosition = {};
		Vec3 m_vAngleTo = {};
		float m_flFOVTo = 0.0f;
		float m_flDistanceTo = 0.0f;
		float m_flSimulationTime = -1.0f;
		const LagRecord_t* m_pLagRecord = nullptr;
		bool m_bMeleeTraceHit = false;
	};

	std::vector<Target_t> m_vecTargets = {};

	bool CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, Target_t& target);
	bool GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, Target_t& outTarget);
	bool ShouldAim(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
	void Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vAngles);
	bool ShouldFire(const Target_t& target);
	void HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);

public:
	bool IsFiring(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
	void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotMelee, AimbotMelee, F);
