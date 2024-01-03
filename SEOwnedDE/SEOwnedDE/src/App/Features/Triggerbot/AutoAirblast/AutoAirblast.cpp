#include "AutoAirblast.h"

#include "../../CFG.h"

struct TargetProjectile
{
	C_BaseProjectile* Projectile = nullptr;
	Vec3 Position = {};
};

bool FindTargetProjectile(C_TFPlayer* local, TargetProjectile& outTarget)
{
	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PROJECTILES_ENEMIES))
	{
		if (!pEntity)
		{
			continue;
		}

		const auto pProjectile = pEntity->As<C_BaseProjectile>();

		if (!pProjectile)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_Rocket && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_Rocket)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_SentryRocket && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_SentryRocket)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_Jar && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_Jar)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_JarGas && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_JarGas)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_JarMilk && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_JarMilk)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_Arrow && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_Arrow)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_Flare && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_Flare)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_Cleaver && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_Cleaver)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_HealingBolt && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_HealingBolt)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_PipebombProjectile && pProjectile->GetClassId() == ETFClassIds::CTFGrenadePipebombProjectile)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_BallOfFire && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_BallOfFire)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_EnergyRing && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_EnergyRing)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Ignore_EnergyBall && pProjectile->GetClassId() == ETFClassIds::CTFProjectile_EnergyBall)
		{
			continue;
		}

		Vec3 vel{};
		pProjectile->EstimateAbsVelocity(vel);

		if (pProjectile->GetClassId() == ETFClassIds::CTFGrenadePipebombProjectile
			&& (pProjectile->As<C_TFGrenadePipebombProjectile>()->m_bTouched()
				|| pProjectile->As<C_TFGrenadePipebombProjectile>()->m_iType() == TF_PROJECTILE_PIPEBOMB_PRACTICE))
		{
			continue;
		}

		if (pProjectile->GetClassId() == ETFClassIds::CTFProjectile_Arrow && fabsf(vel.Length()) <= 10.0f)
		{
			continue;
		}

		auto pos = pProjectile->m_vecOrigin() + (vel * SDKUtils::GetLatency());

		if (pos.DistTo(local->GetShootPos()) > 160.0f)
		{
			continue;
		}

		if (CFG::Triggerbot_AutoAirblast_Mode == 0
			&& Math::CalcFov(I::EngineClient->GetViewAngles(), Math::CalcAngle(local->GetShootPos(), pos)) > 60.0f)
		{
			continue;
		}

		CTraceFilterWorldCustom filter{};
		trace_t trace{};

		H::AimUtils->Trace(local->GetShootPos(), pos, MASK_SOLID, &filter, &trace);

		if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
		{
			continue;
		}

		outTarget.Projectile = pProjectile;
		outTarget.Position = pos;

		return true;
	}

	return false;
}

void CAutoAirblast::Run(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!CFG::Triggerbot_AutoAirblast_Active)
	{
		return;
	}

	if (!G::bCanSecondaryAttack || (pWeapon->GetWeaponID() != TF_WEAPON_FLAMETHROWER && pWeapon->GetWeaponID() != TF_WEAPON_FLAME_BALL)
		|| pWeapon->m_iItemDefinitionIndex() == Pyro_m_ThePhlogistinator)
	{
		return;
	}

	TargetProjectile targetProjectile{};
	if (!FindTargetProjectile(pLocal, targetProjectile))
	{
		return;
	}

	pCmd->buttons |= IN_ATTACK2;

	// Rage airblast
	if (CFG::Triggerbot_AutoAirblast_Mode == 1)
	{
		if (CFG::Triggerbot_AutoAirblast_Aim_Assist)
		{
			auto getOwner = [](C_BaseProjectile* proj) -> C_BaseEntity* {
				const auto owner{ proj->m_hOwnerEntity().Get() };

				if (owner)
				{
					return owner;
				}
				if (proj->GetClassId() == ETFClassIds::CTFGrenadePipebombProjectile)
				{
					if (const auto launcher = proj->As<C_TFGrenadePipebombProjectile>()->m_hLauncher().Get())
					{
						return launcher->m_hOwnerEntity().Get();
					}
				}

				return nullptr;
			};

			const auto owner = getOwner(targetProjectile.Projectile);
			if (owner && owner->GetClassId() == ETFClassIds::CTFPlayer)
			{
				CTraceFilterWorldCustom filter{};
				trace_t trace{};

				H::AimUtils->Trace(pLocal->GetShootPos(), owner->GetCenter(), MASK_SOLID, &filter, &trace);

				if (trace.fraction > 0.99f && !trace.allsolid && !trace.startsolid)
				{
					pCmd->viewangles = Math::CalcAngle(pLocal->GetShootPos(), owner->GetCenter());
				}

				else
				{
					pCmd->viewangles = Math::CalcAngle(pLocal->GetShootPos(), targetProjectile.Position);
				}
			}
		}

		else
		{
			pCmd->viewangles = Math::CalcAngle(pLocal->GetShootPos(), targetProjectile.Position);
		}

		// Silent
		if (CFG::Triggerbot_AutoAirblast_Aim_Mode == 1)
		{
			G::bSilentAngles = true;
		}
	}
}
