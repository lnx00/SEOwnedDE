#include "AimbotProjectile.h"

#include "../../CFG.h"
#include "../../MovementSimulation/MovementSimulation.h"
#include "../../ProjectileSim/ProjectileSim.h"

constexpr int GAUGE_OFFSET = 0x1B40; // TODO: ???

void DrawProjPath(const CUserCmd* pCmd, float time)
{
	if (!pCmd || !G::bFiring)
	{
		return;
	}

	const auto pLocal = H::Entities->GetLocal();
	if (!pLocal || pLocal->deadflag())
	{
		return;
	}

	const auto pWeapon = H::Entities->GetWeapon();
	if (!pWeapon)
	{
		return;
	}

	ProjectileInfo info = {};
	if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, pCmd->viewangles, info))
	{
		return;
	}

	if (!F::ProjectileSim->Init(info))
	{
		return;
	}

	for (auto n = 0; n < TIME_TO_TICKS(time); n++)
	{
		auto pre{ F::ProjectileSim->GetOrigin() };

		F::ProjectileSim->RunTick();

		auto post{ F::ProjectileSim->GetOrigin() };

		I::DebugOverlay->AddLineOverlay(pre, post, 255, 255, 255, false, 10.0f);
	}
}

void DrawMovePath(const std::vector<Vec3>& vPath)
{
	// Line
	if (CFG::Visuals_Draw_Movement_Path_Style == 1)
	{
		for (size_t n = 1; n < vPath.size(); n++)
		{
			I::DebugOverlay->AddLineOverlay(vPath[n], vPath[n - 1], 255, 255, 255, false, 10.0f);
		}
	}

	// Dashed
	if (CFG::Visuals_Draw_Movement_Path_Style == 2)
	{
		for (size_t n = 1; n < vPath.size(); n++)
		{
			if (n % 2 == 0)
			{
				continue;
			}

			I::DebugOverlay->AddLineOverlay(vPath[n], vPath[n - 1], 255, 255, 255, false, 10.0f);
		}
	}

	// Alternative line
	if (CFG::Visuals_Draw_Movement_Path_Style == 3)
	{
		for (size_t n = 1; n < vPath.size(); n++)
		{
			if (n != 1)
			{
				Vec3 right{};

				Math::AngleVectors(Math::CalcAngle(vPath[n], vPath[n - 1]), nullptr, &right, nullptr);

				const Vec3& start{ vPath[n - 1] };
				const Vec3 endL{ vPath[n - 1] + (right * 5.0f) };
				const Vec3 endR{ vPath[n - 1] - (right * 5.0f) };

				I::DebugOverlay->AddLineOverlay(start, endL, 255, 255, 255, false, 10.0f);
				I::DebugOverlay->AddLineOverlay(start, endR, 255, 255, 255, false, 10.0f);
			}

			I::DebugOverlay->AddLineOverlay(vPath[n], vPath[n - 1], 255, 255, 255, false, 10.0f);
		}
	}
}

Vec3 GetOffsetShootPos(C_TFPlayer* local, C_TFWeaponBase* weapon, const CUserCmd* pCmd)
{
	auto out{ local->GetShootPos() };

	switch (weapon->GetWeaponID())
	{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_FLAREGUN_REVENGE:
		case TF_WEAPON_SYRINGEGUN_MEDIC:
		case TF_WEAPON_FLAME_BALL:
		case TF_WEAPON_CROSSBOW:
		case TF_WEAPON_FLAMETHROWER:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
		{
			if (weapon->m_iItemDefinitionIndex() != Soldier_m_TheOriginal)
			{
				Vec3 vOffset = { 23.5f, 12.0f, -3.0f };

				if (local->m_fFlags() & FL_DUCKING)
					vOffset.z = 8.0f;

				H::AimUtils->GetProjectileFireSetup(pCmd->viewangles, vOffset, &out);
			}

			break;
		}

		case TF_WEAPON_COMPOUND_BOW:
		{
			Vec3 vOffset = { 20.5f, 12.0f, -3.0f };

			if (local->m_fFlags() & FL_DUCKING)
				vOffset.z = 8.0f;

			H::AimUtils->GetProjectileFireSetup(pCmd->viewangles, vOffset, &out);

			break;
		}

		default: break;
	}

	return out;
}

bool CAimbotProjectile::GetProjectileInfo(C_TFWeaponBase* pWeapon)
{
	m_CurProjInfo = {};

	auto curTime = [&]() -> float
	{
		if (const auto pLocal = H::Entities->GetLocal())
		{
			return static_cast<float>(pLocal->m_nTickBase()) * I::GlobalVars->interval_per_tick;
		}

		return I::GlobalVars->curtime;
	};

	switch (pWeapon->GetWeaponID())
	{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_PARTICLE_CANNON:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		{
			m_CurProjInfo = { 1100.0f, 0.0f };
			m_CurProjInfo.m_flSpeed = SDKUtils::AttribHookValue(m_CurProjInfo.m_flSpeed, "mult_projectile_speed", pWeapon);

			if (C_TFPlayer* local{ H::Entities->GetLocal() })
			{
				if (const int rocket_specialist{ static_cast<int>(SDKUtils::AttribHookValue(0.0f, "rocket_specialist", local)) })
				{
					m_CurProjInfo.m_flSpeed *= Math::RemapValClamped(static_cast<float>(rocket_specialist), 1.0f, 4.0f, 1.15f, 1.6f);
					m_CurProjInfo.m_flSpeed = std::min(m_CurProjInfo.m_flSpeed, 3000.0f);
				}
			}

			break;
		}

		case TF_WEAPON_GRENADELAUNCHER:
		{
			m_CurProjInfo = { 1200.0f, 1.0f, true };
			m_CurProjInfo.m_flSpeed = SDKUtils::AttribHookValue(m_CurProjInfo.m_flSpeed, "mult_projectile_speed", pWeapon);

			break;
		}

		case TF_WEAPON_PIPEBOMBLAUNCHER:
		{
			const float flChargeBeginTime = pWeapon->As<C_TFPipebombLauncher>()->m_flChargeBeginTime();
			const float flCharge = curTime() - flChargeBeginTime;

			if (flChargeBeginTime)
			{
				m_CurProjInfo.m_flSpeed = Math::RemapValClamped
				(
					flCharge,
					0.0f,
					SDKUtils::AttribHookValue(4.0f, "stickybomb_charge_rate", pWeapon),
					900.0f,
					2400.0f
				);
			}

			else
			{
				m_CurProjInfo.m_flSpeed = 900.0f;
			}

			m_CurProjInfo.m_flGravityMod = 1.0f;
			m_CurProjInfo.m_bPipes = true;

			break;
		}

		case TF_WEAPON_CANNON:
		{
			m_CurProjInfo = { 1454.0f, 1.0f, true };
			break;
		}

		case TF_WEAPON_COMPOUND_BOW:
		{
			const float flChargeBeginTime = pWeapon->As<C_TFPipebombLauncher>()->m_flChargeBeginTime();
			const float flCharge = curTime() - flChargeBeginTime;

			if (flChargeBeginTime)
			{
				m_CurProjInfo.m_flSpeed = 1800.0f + std::clamp<float>(flCharge, 0.0f, 1.0f) * 800.0f;
				m_CurProjInfo.m_flGravityMod = Math::RemapValClamped(flCharge, 0.0f, 1.0f, 0.5f, 0.1f);
			}

			else
			{
				m_CurProjInfo.m_flSpeed = 1800.0f;
				m_CurProjInfo.m_flGravityMod = 0.5f;
			}

			break;
		}

		case TF_WEAPON_CROSSBOW:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
		{
			m_CurProjInfo = { 2400.0f, 0.2f };
			break;
		}

		case TF_WEAPON_SYRINGEGUN_MEDIC:
		{
			m_CurProjInfo = { 1000.0f, 0.3f };
			break;
		}

		case TF_WEAPON_FLAREGUN:
		{
			m_CurProjInfo = { 2000.0f, 0.3f };
			break;
		}

		case TF_WEAPON_FLAREGUN_REVENGE:
		{
			m_CurProjInfo = { 3000.0f, 0.45f };
			break;
		}

		case TF_WEAPON_FLAME_BALL:
		{
			m_CurProjInfo = { 3000.0f, 0.0f };
			break;
		}

		case TF_WEAPON_FLAMETHROWER:
		{
			m_CurProjInfo = { 2000.0f, 0.0f };
			m_CurProjInfo.m_bFlamethrower = true;
			break;
		}

		case TF_WEAPON_RAYGUN:
		case TF_WEAPON_DRG_POMSON:
		{
			m_CurProjInfo = { 1200.0f, 0.0f };

			break;
		}

		default: break;
	}

	return m_CurProjInfo.m_flSpeed > 0.0f;
}

bool CAimbotProjectile::CalcProjAngle(const Vec3& vFrom, const Vec3& vTo, Vec3& vAngleOut, float& flTimeOut)
{
	const auto pWeapon = H::Entities->GetWeapon();
	if (!pWeapon)
		return false;

	const Vec3 v = vTo - vFrom;
	const float dx = sqrt(v.x * v.x + v.y * v.y);
	const float dy = v.z;
	float v0 = m_CurProjInfo.m_flSpeed;
	const float g = SDKUtils::GetGravity() * m_CurProjInfo.m_flGravityMod;

	if (g)
	{
		if (m_CurProjInfo.m_bPipes)
		{
			if (v0 > k_flMaxVelocity)
			{
				v0 = k_flMaxVelocity;
			}
		}

		const auto root{ v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0) };
		if (root < 0.0f)
		{
			return false;
		}

		vAngleOut = { -RAD2DEG(atanf((v0 * v0 - sqrtf(root)) / (g * dx))), RAD2DEG(atan2f(v.y, v.x)), 0.0f };
		flTimeOut = dx / (cosf(-DEG2RAD(vAngleOut.x)) * v0);

		if (m_CurProjInfo.m_bPipes)
		{
			//do 2nd pass for drag | TODO: Math > Magic

			auto magic{ 0.0f };

			if (pWeapon->GetWeaponID() == TF_WEAPON_GRENADELAUNCHER)
			{
				if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad)
				{
					magic = 0.07f;
				}

				else
				{
					magic = 0.11f;
				}
			}

			if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
			{
				magic = 0.16f;
			}

			if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
			{
				magic = 0.35f;
			}

			v0 -= (v0 * flTimeOut) * magic;

			auto root{ v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0) };

			if (root < 0.0f)
			{
				return false;
			}

			vAngleOut = { -RAD2DEG(atanf((v0 * v0 - sqrtf(root)) / (g * dx))), RAD2DEG(atan2f(v.y, v.x)), 0.0f };
			flTimeOut = dx / (cosf(-DEG2RAD(vAngleOut.x)) * v0);
		}
	}

	else
	{
		vAngleOut = Math::CalcAngle(vFrom, vTo);
		flTimeOut = vFrom.DistTo(vTo) / v0;
	}

	if (m_CurProjInfo.m_bPipes)
	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
		{
			if (flTimeOut > 0.95f)
			{
				return false;
			}
		}
		else
		{
			if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheIronBomber)
			{
				if (flTimeOut > 1.4f)
				{
					return false;
				}
			}

			else
			{
				if (flTimeOut > 2.0f)
				{
					return false;
				}
			}
		}
	}

	if ((pWeapon->GetWeaponID() == TF_WEAPON_FLAME_BALL || pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER) && flTimeOut > 0.18f)
	{
		return false;
	}

	return true;
}

void CAimbotProjectile::OffsetPlayerPosition(C_TFWeaponBase* pWeapon, Vec3& vPos, C_TFPlayer* pPlayer, bool bDucked, bool bOnGround)
{
	const float flMaxZ{ (bDucked ? 62.0f : 82.0f) * pPlayer->m_flModelScale() };

	switch (CFG::Aimbot_Projectile_Aim_Position)
	{
		// Feet
		case 0:
		{
			vPos.z += (flMaxZ * 0.2f);
			m_LastAimPos = 0;
			break;
		}

		// Body
		case 1:
		{
			vPos.z += (flMaxZ * 0.5f);
			m_LastAimPos = 1;
			break;
		}

		// Head
		case 2:
		{
			if (CFG::Aimbot_Projectile_Advanced_Head_Aim)
			{
				const Vec3 vDelta = pPlayer->GetHitboxPos(HITBOX_HEAD) - pPlayer->m_vecOrigin();
				vPos.x += vDelta.x;
				vPos.y += vDelta.y;
			}

			vPos.z += (flMaxZ * 0.85f);
			m_LastAimPos = 2;
			break;
		}

		// Auto
		case 3:
		{
			if (pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)
			{
				if (CFG::Aimbot_Projectile_Advanced_Head_Aim)
				{
					const Vec3 vDelta = pPlayer->GetHitboxPos(HITBOX_HEAD) - pPlayer->m_vecOrigin();
					vPos.x += vDelta.x;
					vPos.y += vDelta.y;
				}

				vPos.z += (flMaxZ * 0.92f);
				m_LastAimPos = 2;
			}

			else
			{
				switch (pWeapon->GetWeaponID())
				{
					case TF_WEAPON_ROCKETLAUNCHER:
					case TF_WEAPON_PARTICLE_CANNON:
					case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
					case TF_WEAPON_GRENADELAUNCHER:
					case TF_WEAPON_CANNON:
					{
						if (bOnGround)
						{
							vPos.z += (flMaxZ * 0.2f);
							m_LastAimPos = 0;
						}

						else
						{
							vPos.z += (flMaxZ * 0.5f);
							m_LastAimPos = 1;
						}

						break;
					}
					case TF_WEAPON_PIPEBOMBLAUNCHER:
					{
						vPos.z += (flMaxZ * 0.1f);
						m_LastAimPos = 0;
						break;
					}

					default:
					{
						vPos.z += (flMaxZ * 0.5f);
						m_LastAimPos = 1;
						break;
					}
				}
			}

			break;
		}

		default: break;
	}
}

bool CAimbotProjectile::CanArcReach(const Vec3& vFrom, const Vec3& vTo, const Vec3& vAngleTo, float flTargetTime, C_BaseEntity* pTarget)
{
	const auto pLocal = H::Entities->GetLocal();
	if (!pLocal)
	{
		return false;
	}

	const auto pWeapon = H::Entities->GetWeapon();
	if (!pWeapon)
	{
		return false;
	}

	ProjectileInfo info{};
	if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, vAngleTo, info))
	{
		return false;
	}

	if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad)
	{
		info.m_speed += 45.0f; //need to do this for some reason
	}

	if (!F::ProjectileSim->Init(info, true))
	{
		return false;
	}

	CTraceFilterWorldCustom filter{};
	filter.m_pTarget = pTarget;

	//I::DebugOverlay->ClearAllOverlays();

	for (auto n = 0; n < TIME_TO_TICKS(flTargetTime * 1.2f); n++)
	{
		auto pre{ F::ProjectileSim->GetOrigin() };

		F::ProjectileSim->RunTick();

		auto post{ F::ProjectileSim->GetOrigin() };

		trace_t trace{};

		Vec3 mins{ -6.0f, -6.0f, -6.0f };
		Vec3 maxs{ 6.0f, 6.0f, 6.0f };

		switch (info.m_type)
		{
			case TF_PROJECTILE_PIPEBOMB:
			case TF_PROJECTILE_PIPEBOMB_REMOTE:
			case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			case TF_PROJECTILE_CANNONBALL:
			{
				mins = { -8.0f, -8.0f, -8.0f };
				maxs = { 8.0f, 8.0f, 20.0f };

				break;
			}

			case TF_PROJECTILE_FLARE:
			{
				mins = { -8.0f, -8.0f, -8.0f };
				maxs = { 8.0f, 8.0f, 8.0f };

				break;
			}

			default:
			{
				break;
			}
		}

		H::AimUtils->TraceHull(pre, post, mins, maxs, MASK_SOLID, &filter, &trace);

		if (trace.m_pEnt == pTarget)
		{
			return true;
		}

		if (trace.DidHit())
		{
			if (info.m_pos.DistTo(trace.endpos) > info.m_pos.DistTo(vTo))
			{
				return true;
			}

			if (trace.endpos.DistTo(vTo) > 40.0f)
			{
				return false;
			}

			H::AimUtils->Trace(trace.endpos, vTo, MASK_SOLID, &filter, &trace);

			return !trace.DidHit() || trace.m_pEnt == pTarget;
		}

		//I::DebugOverlay->AddBoxOverlay(post, mins, maxs, Math::CalcAngle(pre, post), 255, 255, 255, 2, 60.0f);
	}

	return true;
}

bool CAimbotProjectile::CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vFrom, const Vec3& vTo, const Target_t& target, float flTargetTime)
{
	Vec3 vLocalPos = vFrom;

	switch (pWeapon->GetWeaponID())
	{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_FLAREGUN_REVENGE:
		case TF_WEAPON_SYRINGEGUN_MEDIC:
		case TF_WEAPON_FLAME_BALL:
		case TF_WEAPON_CROSSBOW:
		case TF_WEAPON_FLAMETHROWER:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
		{
			if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheOriginal)
			{
				Vec3 vOffset = { 23.5f, 12.0f, -3.0f };

				if (pLocal->m_fFlags() & FL_DUCKING)
					vOffset.z = 8.0f;

				H::AimUtils->GetProjectileFireSetup(target.m_vAngleTo, vOffset, &vLocalPos);
			}

			break;
		}

		case TF_WEAPON_COMPOUND_BOW:
		{
			Vec3 vOffset = { 20.5f, 12.0f, -3.0f };

			if (pLocal->m_fFlags() & FL_DUCKING)
				vOffset.z = 8.0f;

			H::AimUtils->GetProjectileFireSetup(target.m_vAngleTo, vOffset, &vLocalPos);

			break;
		}

		default: break;
	}

	if (m_CurProjInfo.m_flGravityMod != 0.f)
	{
		return CanArcReach(vFrom, vTo, target.m_vAngleTo, flTargetTime, target.m_pEntity);
	}

	if (m_CurProjInfo.m_bFlamethrower)
	{
		return H::AimUtils->TraceFlames(target.m_pEntity, vLocalPos, vTo);
	}
	return H::AimUtils->TraceProjectile(target.m_pEntity, vLocalPos, vTo);
}

bool CAimbotProjectile::SolveTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, Target_t& target)
{
	Vec3 vLocalPos = pLocal->GetShootPos();

	if (m_CurProjInfo.m_bPipes)
	{
		const Vec3 vOffset = { 16.0f, 8.0f, -6.0f };
		H::AimUtils->GetProjectileFireSetup(pCmd->viewangles, vOffset, &vLocalPos);
	}

	m_TargetPath.clear();

	if (target.m_pEntity->GetClassId() == ETFClassIds::CTFPlayer)
	{
		const auto pPlayer = target.m_pEntity->As<C_TFPlayer>();

		const bool bDucked = pPlayer->m_fFlags() & FL_DUCKING;
		const bool bOnGround = pPlayer->m_fFlags() & FL_ONGROUND;

		if (!F::MovementSimulation->Initialize(pPlayer))
			return false;

		for (int nTick = 0; nTick < TIME_TO_TICKS(CFG::Aimbot_Projectile_Max_Simulation_Time); nTick++)
		{
			m_TargetPath.push_back(F::MovementSimulation->GetOrigin());

			F::MovementSimulation->RunTick(TICKS_TO_TIME(nTick));

			Vec3 vTarget = F::MovementSimulation->GetOrigin();

			OffsetPlayerPosition(pWeapon, vTarget, pPlayer, bDucked, bOnGround);

			float flTimeToTarget = 0.0f;

			if (!CalcProjAngle(vLocalPos, vTarget, target.m_vAngleTo, flTimeToTarget))
				continue;

			target.m_flTimeToTarget = flTimeToTarget;

			int nTargetTick = TIME_TO_TICKS(flTimeToTarget + SDKUtils::GetLatency());

			//fuck you KGB
			/*if (CFG::Aimbot_Projectile_Aim_Type == 1)
				nTargetTick += 1;*/

			//[20:57]
			//todellinen menninkäinen:
			//crazy
			//cant u do me like kgb
			//do the like
			//credits: m-fed
			//for my
			//original code

			if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
			{
				const auto sticky_arm_time{ SDKUtils::AttribHookValue(0.8f, "sticky_arm_time", pLocal) };
				if (TICKS_TO_TIME(nTargetTick) < sticky_arm_time)
				{
					nTargetTick += TIME_TO_TICKS(fabsf(flTimeToTarget - sticky_arm_time));
				}
			}

			if ((nTargetTick == nTick || nTargetTick == nTick - 1))
			{
				auto runSplash = [&]()
				{
					auto isRocketLauncher{ pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER };
					auto isDirectHit{ pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT };
					auto isAirStrike{ pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheAirStrike };

					if (!isRocketLauncher && !isDirectHit && !isAirStrike)
					{
						return false;
					}

					Vec3 mins{ target.m_pEntity->m_vecMins() };
					Vec3 maxs{ target.m_pEntity->m_vecMaxs() };

					auto center{ F::MovementSimulation->GetOrigin() + Vec3(0.0f, 0.0f, (mins.z + maxs.z) * 0.5f) };

					auto numPoints{ 80 };
					auto radius{ isRocketLauncher ? 180.0f : 80.0f };

					if (isAirStrike)
					{
						radius = 130.0f;
					}

					std::vector<Vec3> potential{};
					for (int n = 0; n < numPoints; n++)
					{
						auto a1{ acosf(1.0f - 2.0f * (static_cast<float>(n) / static_cast<float>(numPoints))) };
						auto a2{ (static_cast<float>(PI) * (3.0f - sqrtf(5.0f))) * static_cast<float>(n) };

						auto point{ center + Vec3{ sinf(a1) * cosf(a2), sinf(a1) * sinf(a2), cosf(a1) }.Scale(radius) };

						CTraceFilterWorldCustom filter{};
						trace_t trace{};

						H::AimUtils->Trace(center, point, MASK_SOLID, &filter, &trace);

						if (trace.fraction > 0.99f)
						{
							continue;
						}

						potential.push_back(trace.endpos);
					}

					std::ranges::sort(potential, [&](const Vec3& a, const Vec3& b)
					{
						return a.DistTo(F::MovementSimulation->GetOrigin()) < b.DistTo(F::MovementSimulation->GetOrigin());
					});

					for (auto& point : potential)
					{
						if (!CalcProjAngle(vLocalPos, point, target.m_vAngleTo, target.m_flTimeToTarget))
						{
							continue;
						}

						trace_t trace = {};
						CTraceFilterWorldCustom filter = {};

						H::AimUtils->TraceHull
						(
							GetOffsetShootPos(pLocal, pWeapon, pCmd),
							point,
							{ -4.0f, -4.0f, -4.0f },
							{ 4.0f, 4.0f, 4.0f },
							MASK_SOLID,
							&filter,
							&trace
						);

						if (trace.fraction < 0.9f || trace.startsolid || trace.allsolid)
						{
							continue;
						}

						H::AimUtils->Trace(trace.endpos, point, MASK_SOLID, &filter, &trace);

						if (trace.fraction < 1.0f)
						{
							continue;
						}

						return true;
					}

					return false;
				};

				if (CFG::Aimbot_Projectile_Rocket_Splash == 2 && runSplash())
				{
					F::MovementSimulation->Restore();

					return true;
				}

				if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
				{
					F::MovementSimulation->Restore();

					return true;
				}

				if (CFG::Aimbot_Projectile_BBOX_Multipoint && pWeapon->GetWeaponID() != TF_WEAPON_COMPOUND_BOW)
				{
					const int nOld = CFG::Aimbot_Projectile_Aim_Position;

					for (int n = 0; n < 3; n++)
					{
						if (n == m_LastAimPos)
							continue;

						CFG::Aimbot_Projectile_Aim_Position = n;

						Vec3 vTargetMp = F::MovementSimulation->GetOrigin();

						OffsetPlayerPosition(pWeapon, vTargetMp, pPlayer, bDucked, bOnGround);

						CFG::Aimbot_Projectile_Aim_Position = nOld;

						if (CalcProjAngle(vLocalPos, vTargetMp, target.m_vAngleTo, target.m_flTimeToTarget))
						{
							if (CanSee(pLocal, pWeapon, vLocalPos, vTargetMp, target, target.m_flTimeToTarget))
							{
								F::MovementSimulation->Restore();

								return true;
							}
						}
					}
				}

				if (CFG::Aimbot_Projectile_Rocket_Splash == 1 && runSplash())
				{
					F::MovementSimulation->Restore();

					return true;
				}
			}
		}

		F::MovementSimulation->Restore();
	}

	else
	{
		const Vec3 vTarget = target.m_vPosition;

		float flTimeToTarget = 0.0f;

		auto runSplash = [&]()
		{
			const auto isRocketLauncher{ pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER };
			const auto isDirectHit{ pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT };
			const auto isAirStrike{ pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheAirStrike };

			if (!isRocketLauncher && !isDirectHit && !isAirStrike)
			{
				return false;
			}

			const auto center{ target.m_pEntity->GetCenter() };

			constexpr auto numPoints{ 80 };
			auto radius{ isRocketLauncher ? 150.0f : 70.0f };

			if (isAirStrike)
			{
				radius = 130.0f;
			}

			std::vector<Vec3> potential{};

			for (int n = 0; n < numPoints; n++)
			{
				const auto a1{ acosf(1.0f - 2.0f * (static_cast<float>(n) / static_cast<float>(numPoints))) };
				const auto a2{ (static_cast<float>(PI) * (3.0f - sqrtf(5.0f))) * static_cast<float>(n) };

				auto point{ center + Vec3{ sinf(a1) * cosf(a2), sinf(a1) * sinf(a2), cosf(a1) }.Scale(radius) };

				CTraceFilterWorldCustom filter{};
				trace_t trace{};

				H::AimUtils->Trace(center, point, MASK_SOLID, &filter, &trace);

				if (trace.fraction > 0.99f)
				{
					continue;
				}

				potential.push_back(trace.endpos);
			}

			std::ranges::sort(potential, [&](const Vec3& a, const Vec3& b)
			{
				return a.DistTo(center) < b.DistTo(center);
			});

			//I::DebugOverlay->ClearAllOverlays();

			for (auto& point : potential)
			{
				//I::DebugOverlay->AddBoxOverlay(point, {}, { 1.0f, 1.0f, 1.0f }, {}, 255, 255, 255, 255, 0.1f);

				if (!CalcProjAngle(vLocalPos, point, target.m_vAngleTo, target.m_flTimeToTarget))
				{
					continue;
				}

				trace_t trace = {};
				CTraceFilterWorldCustom filter = {};

				H::AimUtils->TraceHull
				(
					GetOffsetShootPos(pLocal, pWeapon, pCmd),
					point,
					{ -4.0f, -4.0f, -4.0f },
					{ 4.0f, 4.0f, 4.0f },
					MASK_SOLID,
					&filter,
					&trace
				);

				if (trace.fraction < 0.9f || trace.startsolid || trace.allsolid)
				{
					continue;
				}

				H::AimUtils->Trace(trace.endpos, point, MASK_SOLID, &filter, &trace);

				if (trace.fraction < 1.0f)
				{
					continue;
				}

				return true;
			}

			return false;
		};

		if (!CalcProjAngle(vLocalPos, vTarget, target.m_vAngleTo, flTimeToTarget))
			return false;

		target.m_flTimeToTarget = flTimeToTarget;

		int nTargetTick = TIME_TO_TICKS(flTimeToTarget + SDKUtils::GetLatency());

		if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
		{
			nTargetTick += TIME_TO_TICKS(fabsf(flTimeToTarget - SDKUtils::AttribHookValue(0.8f, "sticky_arm_time", pLocal)));
		}

		if (nTargetTick <= TIME_TO_TICKS(CFG::Aimbot_Projectile_Max_Simulation_Time))
		{
			if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
			{
				return true;
			}
			if (CFG::Aimbot_Projectile_Rocket_Splash && runSplash())
			{
				return true;
			}
		}
	}

	m_TargetPath.clear();
	return false;
}

bool CAimbotProjectile::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, Target_t& outTarget)
{
	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	m_vecTargets.clear();

	if (CFG::Aimbot_Target_Players)
	{
		const auto nGroup = pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ? EEntGroup::PLAYERS_ALL : EEntGroup::PLAYERS_ENEMIES;

		for (const auto pEntity : H::Entities->GetGroup(nGroup))
		{
			if (!pEntity || pEntity == pLocal)
				continue;

			const auto pPlayer = pEntity->As<C_TFPlayer>();

			if (pPlayer->deadflag() || pPlayer->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
				continue;

			if (pPlayer->m_iTeamNum() != pLocal->m_iTeamNum())
			{
				if (CFG::Aimbot_Ignore_Friends && pPlayer->IsPlayerOnSteamFriendsList())
					continue;

				if (CFG::Aimbot_Ignore_Invisible && pPlayer->IsInvisible())
					continue;

				if (CFG::Aimbot_Ignore_Invulnerable && pPlayer->IsInvulnerable())
					continue;

				if (CFG::Aimbot_Ignore_Taunting && pPlayer->InCond(TF_COND_TAUNTING))
					continue;
			}

			else
			{
				if (pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW)
				{
					if (pPlayer->m_iHealth() >= pPlayer->GetMaxHealth() || pPlayer->IsInvulnerable())
					{
						continue;
					}
				}
			}

			Vec3 vPos = pPlayer->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
			const float flDistTo = vLocalPos.DistTo(vPos);

			if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
				continue;

			m_vecTargets.emplace_back(pPlayer, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	if (CFG::Aimbot_Target_Buildings)
	{
		const auto isRescueRanger{ pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE };

		for (const auto pEntity : H::Entities->GetGroup(isRescueRanger ? EEntGroup::BUILDINGS_ALL : EEntGroup::BUILDINGS_ENEMIES))
		{
			if (!pEntity)
				continue;

			const auto pBuilding = pEntity->As<C_BaseObject>();

			if (pBuilding->m_bPlacing())
				continue;

			if (isRescueRanger && pBuilding->m_iTeamNum() == pLocal->m_iTeamNum() && pBuilding->m_iHealth() >= pBuilding->m_iMaxHealth())
			{
				continue;
			}

			Vec3 vPos = pBuilding->GetCenter(); //fuck teleporters when aimed at with pipes lma

			/*if (pEntity->GetClassId() == ETFClassIds::CObjectTeleporter || pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
			{
				vPos = pBuilding->m_vecOrigin();
			}*/

			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
			const float flDistTo = vLocalPos.DistTo(vPos);

			if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
				continue;

			m_vecTargets.emplace_back(pBuilding, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	if (m_vecTargets.empty())
		return false;

	std::ranges::sort(m_vecTargets, [&](const Target_t& a, const Target_t& b) -> bool
	{
		switch (CFG::Aimbot_Projectile_Sort)
		{
			case 0: return a.m_flFOVTo < b.m_flFOVTo;
			case 1: return a.m_flDistanceTo < b.m_flDistanceTo;
			default: return false;
		}
	});

	const auto maxTargets{ std::min(CFG::Aimbot_Projectile_Max_Processing_Targets, static_cast<int>(m_vecTargets.size())) };
	auto targetsScanned{ 0 };

	for (auto& target : m_vecTargets)
	{
		if (target.m_vPosition.DistTo(vLocalPos) > 400.0f && targetsScanned >= maxTargets)
		{
			continue;
		}

		if (!SolveTarget(pLocal, pWeapon, pCmd, target))
		{
			targetsScanned++;

			continue;
		}

		if (CFG::Aimbot_Projectile_Sort == 0 && Math::CalcFov(vLocalAngles, target.m_vAngleTo) > CFG::Aimbot_Projectile_FOV)
		{
			continue;
		}

		outTarget = target;
		return true;
	}

	return false;
}

bool CAimbotProjectile::ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	return CFG::Aimbot_Projectile_Aim_Type != 1 || IsFiring(pCmd, pLocal, pWeapon) && pWeapon->HasPrimaryAmmoForShot();
}

void CAimbotProjectile::Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vAngles)
{
	Vec3 vAngleTo = vAngles - pLocal->m_vecPunchAngle();

	if (m_CurProjInfo.m_bPipes)
	{
		Vec3 vAngle = {}, vForward = {}, vUp = {};
		Math::AngleVectors(vAngleTo, &vForward, nullptr, &vUp);
		const Vec3 vVelocity = (vForward * m_CurProjInfo.m_flSpeed) - (vUp * 200.0f);
		Math::VectorAngles(vVelocity, vAngle);
		vAngleTo.x = vAngle.x;
	}

	Math::ClampAngles(vAngleTo);

	switch (CFG::Aimbot_Projectile_Aim_Type)
	{
		case 0:
		{
			pCmd->viewangles = vAngleTo;
			break;
		}

		case 1:
		{
			if (m_CurProjInfo.m_bFlamethrower ? true : G::bCanPrimaryAttack)
			{
				H::AimUtils->FixMovement(pCmd, vAngleTo);

				pCmd->viewangles = vAngleTo;

				if (m_CurProjInfo.m_bFlamethrower)
				{
					G::bSilentAngles = true;
				}

				else
				{
					G::bPSilentAngles = true;
				}
			}

			break;
		}

		default: break;
	}
}

bool CAimbotProjectile::ShouldFire(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	if (!CFG::Aimbot_AutoShoot)
	{
		//fucking fuck
		if (pWeapon->GetWeaponID() == TF_WEAPON_FLAME_BALL)
		{
			const auto flGauge = reinterpret_cast<float*>(reinterpret_cast<DWORD>(pLocal) + GAUGE_OFFSET);

			if (!flGauge || *flGauge < 100.0f)
				pCmd->buttons &= ~IN_ATTACK;
		}

		return false;
	}

	return true;
}

void CAimbotProjectile::HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon, C_TFPlayer* pLocal, const Target_t& target)
{
	const bool bIsBazooka = pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka;
	if (!bIsBazooka && !pWeapon->HasPrimaryAmmoForShot())
		return;

	const int nWeaponID = pWeapon->GetWeaponID();
	if (nWeaponID == TF_WEAPON_COMPOUND_BOW || nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER)
	{
		if (pWeapon->As<C_TFPipebombLauncher>()->m_flChargeBeginTime() > 0.0f)
			pCmd->buttons &= ~IN_ATTACK;

		else pCmd->buttons |= IN_ATTACK;
	}

	else if (nWeaponID == TF_WEAPON_CANNON)
	{
		if (CFG::Aimbot_Projectile_Auto_Double_Donk)
		{
			const float flDetonateTime = pWeapon->As<C_TFGrenadeLauncher>()->m_flDetonateTime();
			const float flDetonateMaxTime = SDKUtils::AttribHookValue(0.0f, "grenade_launcher_mortar_mode", pWeapon);
			float flCharge = Math::RemapValClamped(flDetonateTime - I::GlobalVars->curtime, 0.0f, flDetonateMaxTime, 0.0f, 1.0f);

			if (I::GlobalVars->curtime > flDetonateTime)
				flCharge = 1.0f;

			if (flCharge < target.m_flTimeToTarget * 0.8f)
				pCmd->buttons &= ~IN_ATTACK;

			else pCmd->buttons |= IN_ATTACK;
		}

		else
		{
			if (pWeapon->As<C_TFGrenadeLauncher>()->m_flDetonateTime() > 0.0f)
				pCmd->buttons &= ~IN_ATTACK;

			else pCmd->buttons |= IN_ATTACK;
		}
	}

	else if (nWeaponID == TF_WEAPON_FLAME_BALL)
	{
		const auto flGauge = reinterpret_cast<float*>(reinterpret_cast<DWORD>(pLocal) + GAUGE_OFFSET);

		if (flGauge && *flGauge >= 100.0f)
			pCmd->buttons |= IN_ATTACK;

		else pCmd->buttons &= ~IN_ATTACK;
	}

	else
	{
		pCmd->buttons |= IN_ATTACK;
	}

	if (bIsBazooka && pWeapon->HasPrimaryAmmoForShot())
		pCmd->buttons &= ~IN_ATTACK;
}

bool CAimbotProjectile::IsFiring(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	if (!pWeapon->HasPrimaryAmmoForShot())
		return false;

	const int nWeaponID = pWeapon->GetWeaponID();
	if (nWeaponID == TF_WEAPON_COMPOUND_BOW || nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER || nWeaponID == TF_WEAPON_CANNON)
	{
		return (G::nOldButtons & IN_ATTACK) && !(pCmd->buttons & IN_ATTACK);
	}

	if (nWeaponID == TF_WEAPON_FLAME_BALL)
	{
		const auto flGauge = reinterpret_cast<float*>(reinterpret_cast<DWORD>(pLocal) + GAUGE_OFFSET);
		return flGauge && *flGauge >= 100.0f && (pCmd->buttons & IN_ATTACK);
	}

	if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
		return G::bCanPrimaryAttack;

	if (nWeaponID == TF_WEAPON_FLAMETHROWER)
	{
		return pCmd->buttons & IN_ATTACK;
	}

	return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}

void CAimbotProjectile::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	if (!CFG::Aimbot_Projectile_Active)
		return;

	if (!GetProjectileInfo(pWeapon))
		return;

	if (CFG::Aimbot_Projectile_Sort == 0)
		G::flAimbotFOV = CFG::Aimbot_Projectile_FOV;

	if (Shifting::bShifting && !Shifting::bShiftingWarp)
		return;

	if (!H::Input->IsDown(CFG::Aimbot_Key))
		return;

	Target_t target = {};
	if (GetTarget(pLocal, pWeapon, pCmd, target) && target.m_pEntity)
	{
		G::nTargetIndexEarly = target.m_pEntity->entindex();

		G::nTargetIndex = target.m_pEntity->entindex();

		if (ShouldFire(pCmd, pLocal, pWeapon))
			HandleFire(pCmd, pWeapon, pLocal, target);

		const bool bIsFiring = IsFiring(pCmd, pLocal, pWeapon);

		G::bFiring = bIsFiring;

		if (ShouldAim(pCmd, pLocal, pWeapon) || bIsFiring)
		{
			/*const Vec3 ang_center{ Math::CalcAngle(pLocal->GetShootPos(), Target.m_vPosition) };
			const Vec3 ang_offset{ Math::CalcAngle(getOffsetShootPos(pLocal, pWeapon, pCmd), Target.m_vPosition)};

			const float correction_scale{ Math::RemapValClamped(pLocal->GetShootPos().DistTo(Target.m_vPosition), 0.0f, 19202.0f, 0.0f, 1.0f) };

			const float base_val{ pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW ? 5.3f : 6.5f };

			const Vec3 correction{ (ang_offset - ang_center) * (base_val * correction_scale) };

			Target.m_vAngleTo -= correction;*/

			Aim(pCmd, pLocal, pWeapon, target.m_vAngleTo);

			if (bIsFiring && m_TargetPath.size() > 1)
			{
				I::DebugOverlay->ClearAllOverlays();

				//drawProjPath(pCmd, Target.m_flTimeToTarget);
				DrawMovePath(m_TargetPath);
				m_TargetPath.clear();
			}
		}
	}
}
