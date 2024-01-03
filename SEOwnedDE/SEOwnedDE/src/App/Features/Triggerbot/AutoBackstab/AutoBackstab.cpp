#include "AutoBackstab.h"

#include "../../CFG.h"

#include "../../LagRecords/LagRecords.h"

bool IsBehindAndFacingTarget(const Vec3& ownerCenter, const Vec3& ownerViewangles, const Vec3& targetCenter, const Vec3& targetEyeAngles)
{
	Vec3 toTarget = targetCenter - ownerCenter;
	toTarget.z = 0.0f;
	toTarget.NormalizeInPlace();

	Vec3 ownerForward{};
	Math::AngleVectors(ownerViewangles, &ownerForward, nullptr, nullptr);
	ownerForward.z = 0.0f;
	ownerForward.NormalizeInPlace();

	Vec3 targetForward{};
	Math::AngleVectors(targetEyeAngles, &targetForward, nullptr, nullptr);
	targetForward.z = 0.0f;
	targetForward.NormalizeInPlace();

	return toTarget.Dot(targetForward) > (0.0f + 0.03125f)
		&& toTarget.Dot(ownerForward) > (0.5f + 0.03125f)
		&& targetForward.Dot(ownerForward) > (-0.3f + 0.03125f);
}

bool CanKnifeOneShot(C_TFPlayer* target, bool crit, bool miniCrit)
{
	if (!target || target->IsInvulnerable())
	{
		return false;
	}

	const auto pWeapon{ target->m_hActiveWeapon().Get() };

	if (!pWeapon)
	{
		return false;
	}

	int dmgMult = 1;

	if (miniCrit || target->IsMarked())
	{
		dmgMult = 2;
	}

	if (crit)
	{
		dmgMult = 3;
	}

	if (pWeapon->As<C_TFWeaponBase>()->m_iItemDefinitionIndex() == Heavy_t_FistsofSteel)
	{
		return target->m_iHealth() <= 80 * dmgMult;
	}

	return target->m_iHealth() <= 40 * dmgMult;
}

void CAutoBackstab::Run(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!CFG::Triggerbot_AutoBackstab_Active)
	{
		return;
	}

	if (!G::bCanPrimaryAttack || pLocal->m_bFeignDeathReady() || pLocal->m_flInvisibility() > 0.0f || pWeapon->GetWeaponID() != TF_WEAPON_KNIFE)
	{
		return;
	}

	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
	{
		if (!pEntity)
		{
			continue;
		}

		const auto pPlayer = pEntity->As<C_TFPlayer>();

		if (!pPlayer || pPlayer->deadflag() || pPlayer->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
		{
			continue;
		}

		if (CFG::Triggerbot_AutoBackstab_Ignore_Friends && pPlayer->IsPlayerOnSteamFriendsList())
		{
			continue;
		}

		if (CFG::Triggerbot_AutoBackstab_Ignore_Invisible && pPlayer->IsInvisible())
		{
			continue;
		}

		if (CFG::Triggerbot_AutoBackstab_Ignore_Invulnerable && pPlayer->IsInvulnerable())
		{
			continue;
		}

		// Knife if lethal
		auto canKnife = false;
		if (CFG::Triggerbot_AutoBackstab_Knife_If_Lethal)
		{
			canKnife = CanKnifeOneShot(pPlayer, pLocal->IsCritBoosted(), pLocal->IsMiniCritBoosted());
		}

		auto angleTo{ I::EngineClient->GetViewAngles() };

		if (CFG::Triggerbot_AutoBacktab_Mode == 1)
		{
			angleTo = Math::CalcAngle(pLocal->GetShootPos(), pPlayer->GetCenter());
		}

		if (canKnife || IsBehindAndFacingTarget(pLocal->GetCenter(), angleTo, pPlayer->GetCenter(), pPlayer->GetEyeAngles()))
		{
			Vec3 forward{};
			Math::AngleVectors(angleTo, &forward);

			auto to = pLocal->GetShootPos() + (forward * 47.0f);

			if (H::AimUtils->TraceEntityMelee(pPlayer, pLocal->GetShootPos(), to))
			{
				if (CFG::Triggerbot_AutoBacktab_Mode == 1)
				{
					pCmd->viewangles = angleTo;

					if (CFG::Triggerbot_AutoBacktab_Aim_Mode == 1)
					{
						G::bPSilentAngles = true;
					}
				}

				pCmd->buttons |= IN_ATTACK;

				if (CFG::Misc_Accuracy_Improvements)
				{
					pCmd->tick_count = TIME_TO_TICKS(pPlayer->m_flSimulationTime() + SDKUtils::GetLerp());
				}

				return;
			}
		}

		int numRecords = 0;

		if (!F::LagRecords->HasRecords(pPlayer, &numRecords))
		{
			continue;
		}

		for (int n = 1; n < numRecords; n++)
		{
			const auto record = F::LagRecords->GetRecord(pPlayer, n, true);

			if (!record)
			{
				continue;
			}

			// Rage mode
			if (CFG::Triggerbot_AutoBacktab_Mode == 1)
			{
				angleTo = Math::CalcAngle(pLocal->GetShootPos(), record->Center);
			}

			if (canKnife || IsBehindAndFacingTarget(pLocal->GetCenter(), angleTo, record->Center, pPlayer->GetEyeAngles()))
			{
				F::LagRecordMatrixHelper->Set(record);

				Vec3 forward{};
				Math::AngleVectors(angleTo, &forward);

				auto to = pLocal->GetShootPos() + (forward * 47.0f);

				if (!H::AimUtils->TraceEntityMelee(pPlayer, pLocal->GetShootPos(), to))
				{
					F::LagRecordMatrixHelper->Restore();

					continue;
				}

				F::LagRecordMatrixHelper->Restore();

				if (CFG::Triggerbot_AutoBacktab_Mode == 1)
				{
					pCmd->viewangles = angleTo;

					if (CFG::Triggerbot_AutoBacktab_Aim_Mode == 1)
					{
						G::bPSilentAngles = true;
					}
				}

				pCmd->buttons |= IN_ATTACK;

				if (CFG::Misc_Accuracy_Improvements)
				{
					pCmd->tick_count = TIME_TO_TICKS(record->SimulationTime + SDKUtils::GetLerp());
				}
				else
				{
					pCmd->tick_count = TIME_TO_TICKS(record->SimulationTime + GetClientInterpAmount());
				}

				return;
			}
		}
	}
}
