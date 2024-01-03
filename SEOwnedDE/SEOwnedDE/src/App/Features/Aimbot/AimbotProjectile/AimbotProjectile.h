#pragma once
#include "../AimbotCommon/AimbotCommon.h"

class CAimbotProjectile
{
	struct ProjTarget_t : AimTarget_t
	{
		float TimeToTarget = 0.0f;
	};

	std::vector<ProjTarget_t> m_vecTargets = {};
	std::vector<Vec3> m_TargetPath = {};
	int m_LastAimPos = 0; // 0 = feet, 1 = body, 2 = head

	struct ProjectileInfo_t
	{
		float Speed = 0.0f;
		float GravityMod = 0.0f;
		bool Pipes = false;
		bool Flamethrower = false;
	};

	ProjectileInfo_t m_CurProjInfo = {};

	bool GetProjectileInfo(C_TFWeaponBase* pWeapon);
	bool CalcProjAngle(const Vec3& vFrom, const Vec3& vTo, Vec3& vAngleOut, float& flTimeOut);
	void OffsetPlayerPosition(C_TFWeaponBase* pWeapon, Vec3& vPos, C_TFPlayer* pPlayer, bool bDucked, bool bOnGround);
	bool CanArcReach(const Vec3& vFrom, const Vec3& vTo, const Vec3& vAngleTo, float flTargetTime, C_BaseEntity* pTarget);
	bool CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vFrom, const Vec3& vTo, const ProjTarget_t& target, float flTargetTime);
	bool SolveTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, ProjTarget_t& target);

	bool GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, ProjTarget_t& outTarget);
	bool ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
	void Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vAngles);
	bool ShouldFire(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
	void HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon, C_TFPlayer* pLocal, const ProjTarget_t& target);

public:
	bool IsFiring(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
	void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotProjectile, AimbotProjectile, F);
