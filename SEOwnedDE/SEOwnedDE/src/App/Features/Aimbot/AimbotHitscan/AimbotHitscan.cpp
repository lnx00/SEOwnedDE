#include "AimbotHitscan.h"

#include "../../CFG.h"

int CAimbotHitscan::GetAimHitbox(C_TFWeaponBase* pWeapon)
{
	switch (CFG::Aimbot_Hitscan_Hitbox)
	{
		case 0: return HITBOX_HEAD;
		case 1: return HITBOX_PELVIS;
		case 2:
		{
			if (pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
				return (pWeapon->As<C_TFSniperRifle>()->m_flChargedDamage() >= 150.0f) ? HITBOX_HEAD : HITBOX_PELVIS;

			return H::AimUtils->IsWeaponCapableOfHeadshot(pWeapon) ? HITBOX_HEAD : HITBOX_PELVIS;
		}
		default: return HITBOX_PELVIS;
	}
}

bool CAimbotHitscan::ScanHead(C_TFPlayer* pLocal, HitscanTarget_t& target)
{
	if (!CFG::Aimbot_Hitscan_Scan_Head)
		return false;

	const auto pPlayer = target.Entity->As<C_TFPlayer>();
	if (!pPlayer)
		return false;

	const auto pModel = pPlayer->GetModel();
	if (!pModel)
		return false;

	const auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
	if (!pHDR)
		return false;

	const auto pSet = pHDR->pHitboxSet(pPlayer->m_nHitboxSet());
	if (!pSet)
		return false;

	const auto pBox = pSet->pHitbox(HITBOX_HEAD);
	if (!pBox)
		return false;

	matrix3x4_t boneMatrix[128] = {};
	if (!pPlayer->SetupBones(boneMatrix, 128, 0x100, I::GlobalVars->curtime))
		return false;

	const Vec3 vMins = pBox->bbmin;
	const Vec3 vMaxs = pBox->bbmax;

	const std::array vPoints = {
		Vec3((vMins.x + vMaxs.x) * 0.5f, vMins.y * 0.7f, (vMins.z + vMaxs.z) * 0.5f)
	};

	const Vec3 vLocalPos = pLocal->GetShootPos();
	for (const auto& vPoint : vPoints)
	{
		Vec3 vTransformed = {};
		Math::VectorTransform(vPoint, boneMatrix[pBox->bone], vTransformed);

		int nHitHitbox = -1;

		if (!H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vTransformed, &nHitHitbox))
			continue;

		if (nHitHitbox != HITBOX_HEAD)
			continue;

		target.Position = vTransformed;
		target.AngleTo = Math::CalcAngle(vLocalPos, vTransformed);
		target.WasMultiPointed = true;

		return true;
	}

	return false;
}

bool CAimbotHitscan::ScanBody(C_TFPlayer* pLocal, HitscanTarget_t& target)
{
	const bool bScanningBody = CFG::Aimbot_Hitscan_Scan_Body;
	const bool bScaningArms = CFG::Aimbot_Hitscan_Scan_Arms;
	const bool bScanningLegs = CFG::Aimbot_Hitscan_Scan_Legs;

	if (!bScanningBody && !bScaningArms && !bScanningLegs)
		return false;

	const auto pPlayer = target.Entity->As<C_TFPlayer>();
	if (!pPlayer)
		return false;

	const Vec3 vLocalPos = pLocal->GetShootPos();
	for (int n = 1; n < pPlayer->GetNumOfHitboxes(); n++)
	{
		if (n == target.AimedHitbox)
			continue;

		const int nHitboxGroup = pPlayer->GetHitboxGroup(n);

		if (!bScanningBody && (nHitboxGroup == HITGROUP_CHEST || nHitboxGroup == HITGROUP_STOMACH))
			continue;

		if (!bScaningArms && (nHitboxGroup == HITGROUP_LEFTARM || nHitboxGroup == HITGROUP_RIGHTARM))
			continue;

		if (!bScanningLegs && (nHitboxGroup == HITGROUP_LEFTLEG || nHitboxGroup == HITGROUP_RIGHTLEG))
			continue;

		Vec3 vHitbox = pPlayer->GetHitboxPos(n);

		if (!H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vHitbox))
			continue;

		target.Position = vHitbox;
		target.AngleTo = Math::CalcAngle(vLocalPos, vHitbox);

		return true;
	}

	return false;
}

bool CAimbotHitscan::ScanBuilding(C_TFPlayer* pLocal, HitscanTarget_t& target)
{
	if (!CFG::Aimbot_Hitscan_Scan_Buildings)
		return false;

	const auto pObject = target.Entity->As<C_BaseObject>();
	if (!pObject)
		return false;

	const Vec3 vLocalPos = pLocal->GetShootPos();

	if (pObject->GetClassId() == ETFClassIds::CObjectSentrygun)
	{
		for (int n = 0; n < pObject->GetNumOfHitboxes(); n++)
		{
			Vec3 vHitbox = pObject->GetHitboxPos(n);

			if (!H::AimUtils->TraceEntityBullet(pObject, vLocalPos, vHitbox))
				continue;

			target.Position = vHitbox;
			target.AngleTo = Math::CalcAngle(vLocalPos, vHitbox);

			return true;
		}
	}

	else
	{
		const Vec3 vMins = pObject->m_vecMins();
		const Vec3 vMaxs = pObject->m_vecMaxs();

		const std::array vPoints = {
			Vec3(vMins.x * 0.9f, ((vMins.y + vMaxs.y) * 0.5f), ((vMins.z + vMaxs.z) * 0.5f)),
			Vec3(vMaxs.x * 0.9f, ((vMins.y + vMaxs.y) * 0.5f), ((vMins.z + vMaxs.z) * 0.5f)),
			Vec3(((vMins.x + vMaxs.x) * 0.5f), vMins.y * 0.9f, ((vMins.z + vMaxs.z) * 0.5f)),
			Vec3(((vMins.x + vMaxs.x) * 0.5f), vMaxs.y * 0.9f, ((vMins.z + vMaxs.z) * 0.5f)),
			Vec3(((vMins.x + vMaxs.x) * 0.5f), ((vMins.y + vMaxs.y) * 0.5f), vMins.z * 0.9f),
			Vec3(((vMins.x + vMaxs.x) * 0.5f), ((vMins.y + vMaxs.y) * 0.5f), vMaxs.z * 0.9f)
		};

		const matrix3x4_t& transform = pObject->RenderableToWorldTransform();
		for (const auto& vPoint : vPoints)
		{
			Vec3 vTransformed = {};
			Math::VectorTransform(vPoint, transform, vTransformed);

			if (!H::AimUtils->TraceEntityBullet(pObject, vLocalPos, vTransformed))
				continue;

			target.Position = vTransformed;
			target.AngleTo = Math::CalcAngle(vLocalPos, vTransformed);

			return true;
		}
	}

	return false;
}

bool CAimbotHitscan::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget)
{
	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	m_vecTargets.clear();

	// Find player targets
	if (CFG::Aimbot_Target_Players)
	{
		const int nAimHitbox = GetAimHitbox(pWeapon);

		for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
		{
			if (!pEntity)
				continue;

			const auto pPlayer = pEntity->As<C_TFPlayer>();
			if (pPlayer->deadflag() || pPlayer->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
				continue;

			if (CFG::Aimbot_Ignore_Friends && pPlayer->IsPlayerOnSteamFriendsList())
				continue;

			if (CFG::Aimbot_Ignore_Invisible && pPlayer->IsInvisible())
				continue;

			if (CFG::Aimbot_Ignore_Invulnerable && pPlayer->IsInvulnerable())
				continue;

			if (CFG::Aimbot_Ignore_Taunting && pPlayer->InCond(TF_COND_TAUNTING))
				continue;

			if (CFG::Aimbot_Hitscan_Target_LagRecords)
			{
				int nRecords = 0;

				if (!F::LagRecords->HasRecords(pPlayer, &nRecords))
					continue;

				for (int n = 1; n < nRecords; n++)
				{
					const auto pRecord = F::LagRecords->GetRecord(pPlayer, n, true);

					if (!pRecord || !F::LagRecords->DiffersFromCurrent(pRecord))
						continue;

					Vec3 vPos = SDKUtils::GetHitboxPosFromMatrix(pPlayer, nAimHitbox, const_cast<matrix3x4_t*>(pRecord->BoneMatrix));
					Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
					const float flFOVTo = CFG::Aimbot_Hitscan_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
					const float flDistTo = vLocalPos.DistTo(vPos);

					if (CFG::Aimbot_Hitscan_Sort == 0 && flFOVTo > CFG::Aimbot_Hitscan_FOV)
						continue;

					m_vecTargets.emplace_back(AimTarget_t {
						pPlayer, vPos, vAngleTo, flFOVTo, flDistTo
					}, nAimHitbox, pRecord->SimulationTime, pRecord);
				}
			}

			/*if (CFG::Aimbot_Hitscan_Aim_Type != 2)
			{
				if (TIME_TO_TICKS(pPlayer->m_flSimulationTime() - pPlayer->m_flOldSimulationTime()) < 1)
					continue;
			}*/

			Vec3 vPos = pPlayer->GetHitboxPos(nAimHitbox);
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = CFG::Aimbot_Hitscan_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
			const float flDistTo = vLocalPos.DistTo(vPos);

			if (CFG::Aimbot_Hitscan_Sort == 0 && flFOVTo > CFG::Aimbot_Hitscan_FOV)
				continue;

			m_vecTargets.emplace_back(AimTarget_t { pPlayer, vPos, vAngleTo, flFOVTo, flDistTo}, nAimHitbox, pPlayer->m_flSimulationTime());
		}
	}

	// Find Building targets
	if (CFG::Aimbot_Target_Buildings)
	{
		for (const auto pEntity : H::Entities->GetGroup(EEntGroup::BUILDINGS_ENEMIES))
		{
			if (!pEntity)
				continue;

			const auto pBuilding = pEntity->As<C_BaseObject>();
			if (pBuilding->m_bPlacing())
				continue;

			Vec3 vPos = pBuilding->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = CFG::Aimbot_Hitscan_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
			const float flDistTo = vLocalPos.DistTo(vPos);

			if (CFG::Aimbot_Hitscan_Sort == 0 && flFOVTo > CFG::Aimbot_Hitscan_FOV)
				continue;

			m_vecTargets.emplace_back(AimTarget_t { pBuilding, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	// Find stickybomb targets
	if (CFG::Aimbot_Hitscan_Target_Stickies)
	{
		for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PROJECTILES_ENEMIES))
		{
			if (!pEntity || pEntity->GetClassId() != ETFClassIds::CTFGrenadePipebombProjectile)
			{
				continue;
			}

			const auto pipe = pEntity->As<C_TFGrenadePipebombProjectile>();
			if (!pipe || !pipe->m_bTouched() || !pipe->HasStickyEffects() || pipe->m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE)
			{
				continue;
			}

			Vec3 vPos = pipe->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = CFG::Aimbot_Hitscan_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
			const float flDistTo = vLocalPos.DistTo(vPos);

			if (CFG::Aimbot_Hitscan_Sort == 0 && flFOVTo > CFG::Aimbot_Hitscan_FOV)
				continue;

			m_vecTargets.emplace_back(AimTarget_t {pipe, vPos, vAngleTo, flFOVTo, flDistTo});
		}
	}

	if (m_vecTargets.empty())
		return false;

	// Sort by target priority
	F::AimbotCommon->Sort(m_vecTargets, CFG::Aimbot_Hitscan_Sort);

	// Find and return the first valid target
	for (auto& target : m_vecTargets)
	{
		switch (target.Entity->GetClassId())
		{
			case ETFClassIds::CTFPlayer:
			{
				if (!target.LagRecord)
				{
					int nHitHitbox = -1;

					if (!H::AimUtils->TraceEntityBullet(target.Entity, vLocalPos, target.Position, &nHitHitbox))
					{
						if (target.AimedHitbox == HITBOX_HEAD)
						{
							if (!ScanHead(pLocal, target))
								continue;
						}

						else if (target.AimedHitbox == HITBOX_PELVIS)
						{
							if (!ScanBody(pLocal, target))
								continue;
						}

						else
						{
							continue;
						}
					}

					else
					{
						if (nHitHitbox != target.AimedHitbox && target.AimedHitbox == HITBOX_HEAD)
							ScanHead(pLocal, target);
					}
				}

				else
				{
					F::LagRecordMatrixHelper->Set(target.LagRecord);

					const bool bTraceResult = H::AimUtils->TraceEntityBullet(target.Entity, vLocalPos, target.Position);

					F::LagRecordMatrixHelper->Restore();

					if (!bTraceResult)
						continue;
				}

				break;
			}

			case ETFClassIds::CObjectSentrygun:
			case ETFClassIds::CObjectDispenser:
			case ETFClassIds::CObjectTeleporter:
			{
				if (!H::AimUtils->TraceEntityBullet(target.Entity, vLocalPos, target.Position))
				{
					if (!ScanBuilding(pLocal, target))
						continue;
				}

				break;
			}

			case ETFClassIds::CTFGrenadePipebombProjectile:
			{
				if (!H::AimUtils->TraceEntityBullet(target.Entity, vLocalPos, target.Position))
				{
					continue;
				}

				break;
			}

			default: continue;
		}

		outTarget = target;
		return true;
	}

	return false;
}

bool CAimbotHitscan::ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	if (CFG::Aimbot_Hitscan_Aim_Type == 1 && (!IsFiring(pCmd, pWeapon) || !pWeapon->HasPrimaryAmmoForShot()))
		return false;

	if (CFG::Aimbot_Hitscan_Aim_Type == 2)
	{
		const int nWeaponID = pWeapon->GetWeaponID();
		if (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP)
		{
			if (!G::bCanPrimaryAttack)
				return false;
		}
	}

	if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN && pWeapon->As<C_TFMinigun>()->m_iWeaponState() == AC_STATE_DRYFIRE)
		return false;

	return true;
}

void CAimbotHitscan::Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, const Vec3& vAngles)
{
	Vec3 vAngleTo = vAngles - pLocal->m_vecPunchAngle();
	Math::ClampAngles(vAngleTo);

	switch (CFG::Aimbot_Hitscan_Aim_Type)
	{
		// Plain
		case 0:
		{
			pCmd->viewangles = vAngleTo;
			break;
		}
		
		// Silent
		case 1:
		{
			if (G::bCanPrimaryAttack)
			{
				H::AimUtils->FixMovement(pCmd, vAngleTo);
				pCmd->viewangles = vAngleTo;
				G::bSilentAngles = true;
			}

			break;
		}

		// Smooth
		case 2:
		{
			Vec3 vDelta = vAngleTo - pCmd->viewangles;
			Math::ClampAngles(vDelta);

			// Apply smoothing
			if (vDelta.Length() > 0.0f && CFG::Aimbot_Hitscan_Smoothing > 0.f)
				pCmd->viewangles += vDelta / CFG::Aimbot_Hitscan_Smoothing;

			break;
		}

		default: break;
	}
}

bool CAimbotHitscan::ShouldFire(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const HitscanTarget_t& target)
{
	if (!CFG::Aimbot_AutoShoot)
		return false;

	const bool bIsMachina = pWeapon->m_iItemDefinitionIndex() == Sniper_m_TheMachina || pWeapon->m_iItemDefinitionIndex() == Sniper_m_ShootingStar;
	const bool bCapableOfHeadshot = H::AimUtils->IsWeaponCapableOfHeadshot(pWeapon);
	const bool bIsSydneySleeper = pWeapon->m_iItemDefinitionIndex() == Sniper_m_TheSydneySleeper;
	const bool bIsSniper = pLocal->m_iClass() == TF_CLASS_SNIPER;

	if (bIsMachina && !pLocal->IsZoomed())
		return false;

	if (CFG::Aimbot_Hitscan_Wait_For_Headshot)
	{
		if (target.Entity->GetClassId() == ETFClassIds::CTFPlayer && bCapableOfHeadshot && !G::bCanHeadshot)
			return false;
	}

	if (CFG::Aimbot_Hitscan_Wait_For_Charge)
	{
		if (target.Entity->GetClassId() == ETFClassIds::CTFPlayer && bIsSniper && (bCapableOfHeadshot || bIsSydneySleeper))
		{
			const auto pPlayer = target.Entity->As<C_TFPlayer>();
			const auto pSniperRifle = pWeapon->As<C_TFSniperRifle>();

			const int nHealth = pPlayer->m_iHealth();
			const bool bIsCritBoosted = pLocal->IsCritBoosted();

			if (target.AimedHitbox == HITBOX_HEAD && !bIsSydneySleeper)
			{
				if (nHealth > 150)
				{
					const float flDamage = Math::RemapValClamped(pSniperRifle->m_flChargedDamage(), 0.0f, 150.0f, 0.0f, 450.0f);
					const int nDamage = static_cast<int>(flDamage);

					if (nDamage < nHealth && nDamage != 450)
						return false;
				}

				else
				{
					if (!bIsCritBoosted && !G::bCanHeadshot)
						return false;
				}
			}

			else
			{
				if (nHealth > (bIsCritBoosted ? 150 : 50))
				{
					float flMult = pPlayer->IsMarked() ? 1.36f : 1.0f;

					if (bIsCritBoosted)
						flMult = 3.0f;

					const float flMax = 150.0f * flMult;
					const int nDamage = static_cast<int>(pSniperRifle->m_flChargedDamage() * flMult);

					if (nDamage < pPlayer->m_iHealth() && nDamage != static_cast<int>(flMax))
						return false;
				}
			}
		}
	}

	if (CFG::Aimbot_Hitscan_Minigun_TapFire)
	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN)
		{
			if (pLocal->GetAbsOrigin().DistTo(target.Position) >= 900.0f)
			{
				if ((pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime() <= 0.25f)
					return false;
			}
		}
	}

	if (CFG::Aimbot_Hitscan_Advanced_Smooth_AutoShoot && CFG::Aimbot_Hitscan_Aim_Type == 2)
	{
		Vec3 vForward = {};
		Math::AngleVectors(pCmd->viewangles, &vForward);
		const Vec3 vTraceStart = pLocal->GetShootPos();
		const Vec3 vTraceEnd = vTraceStart + (vForward * 8192.0f);

		if (target.Entity->GetClassId() == ETFClassIds::CTFPlayer)
		{
			const auto pPlayer = target.Entity->As<C_TFPlayer>();

			if (!target.LagRecord)
			{
				int nHitHitbox = -1;

				if (!H::AimUtils->TraceEntityBullet(pPlayer, vTraceStart, vTraceEnd, &nHitHitbox))
					return false;

				if (target.AimedHitbox == HITBOX_HEAD)
				{
					if (nHitHitbox != HITBOX_HEAD)
						return false;

					if (!target.WasMultiPointed)
					{
						Vec3 vMins = {}, vMaxs = {}, vCenter = {};
						matrix3x4_t matrix = {};
						pPlayer->GetHitboxInfo(nHitHitbox, &vCenter, &vMins, &vMaxs, &matrix);

						vMins *= 0.5f;
						vMaxs *= 0.5f;

						if (!Math::RayToOBB(vTraceStart, vForward, vCenter, vMins, vMaxs, matrix))
							return false;
					}
				}
			}

			else
			{
				F::LagRecordMatrixHelper->Set(target.LagRecord);

				int nHitHitbox = -1;

				if (!H::AimUtils->TraceEntityBullet(pPlayer, vTraceStart, vTraceEnd, &nHitHitbox))
				{
					F::LagRecordMatrixHelper->Restore();
					return false;
				}

				if (target.AimedHitbox == HITBOX_HEAD)
				{
					if (nHitHitbox != HITBOX_HEAD)
					{
						F::LagRecordMatrixHelper->Restore();
						return false;
					}

					Vec3 vMins = {}, vMaxs = {}, vCenter = {};
					SDKUtils::GetHitboxInfoFromMatrix(pPlayer, nHitHitbox, const_cast<matrix3x4_t*>(target.LagRecord->BoneMatrix), &vCenter, &vMins, &vMaxs);

					vMins *= 0.5f;
					vMaxs *= 0.5f;

					if (!Math::RayToOBB(vTraceStart, vForward, vCenter, vMins, vMaxs, *target.LagRecord->BoneMatrix))
					{
						F::LagRecordMatrixHelper->Restore();
						return false;
					}
				}

				F::LagRecordMatrixHelper->Restore();
			}
		}

		else
		{
			if (!H::AimUtils->TraceEntityBullet(target.Entity, vTraceStart, vTraceEnd, nullptr))
			{
				return false;
			}
		}
	}

	return true;
}

// Handles and updated the IN_ATTACK state
void CAimbotHitscan::HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
	if (!pWeapon->HasPrimaryAmmoForShot())
		return;

	if (pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
	{
		if (G::nOldButtons & IN_ATTACK)
		{
			pCmd->buttons &= ~IN_ATTACK;
		}
		else
		{
			pCmd->buttons |= IN_ATTACK;
		}
	}

	else
	{
		pCmd->buttons |= IN_ATTACK;
	}
}

bool CAimbotHitscan::IsFiring(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
	if (!pWeapon->HasPrimaryAmmoForShot())
		return false;

	if (pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
		return !(pCmd->buttons & IN_ATTACK) && (G::nOldButtons & IN_ATTACK);

	return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}

void CAimbotHitscan::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	if (!CFG::Aimbot_Hitscan_Active)
		return;

	if (CFG::Aimbot_Hitscan_Sort == 0)
		G::flAimbotFOV = CFG::Aimbot_Hitscan_FOV;

	if (Shifting::bShifting && !Shifting::bShiftingWarp)
		return;

	const bool isFiring = IsFiring(pCmd, pWeapon);

	HitscanTarget_t target = {};
	if (GetTarget(pLocal, pWeapon, target) && target.Entity)
	{
		G::nTargetIndexEarly = target.Entity->entindex();

		const auto aimKeyDown = H::Input->IsDown(CFG::Aimbot_Key);
		if (aimKeyDown || isFiring)
		{
			G::nTargetIndex = target.Entity->entindex();

			// Auto Scope
			if (CFG::Aimbot_Hitscan_Auto_Scope
				&& !pLocal->IsZoomed() && pLocal->m_iClass() == TF_CLASS_SNIPER && pWeapon->GetSlot() == WEAPON_SLOT_PRIMARY && G::bCanPrimaryAttack)
			{
				pCmd->buttons |= IN_ATTACK2;
				return;
			}

			// Auto Shoot
			if (CFG::Aimbot_AutoShoot && pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
				pCmd->buttons |= IN_ATTACK;

			// Spin up minigun
			if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN)
			{
				const int nState = pWeapon->As<C_TFMinigun>()->m_iWeaponState();
				if (nState == AC_STATE_IDLE || nState == AC_STATE_STARTFIRING)
					G::bCanPrimaryAttack = false; // TODO: hack

				pCmd->buttons |= IN_ATTACK2;
			}

			// Update attack state
			if (ShouldFire(pCmd, pLocal, pWeapon, target))
			{
				HandleFire(pCmd, pWeapon);
			}

			const bool bIsFiring = IsFiring(pCmd, pWeapon);
			G::bFiring = bIsFiring;

			// Are we ready to aim?
			if (ShouldAim(pCmd, pLocal, pWeapon) || bIsFiring)
			{
				if (aimKeyDown)
				{
					Aim(pCmd, pLocal, target.AngleTo);
				}

				if (CFG::Misc_Accuracy_Improvements)
				{
					if (bIsFiring && target.Entity->GetClassId() == ETFClassIds::CTFPlayer)
					{
						pCmd->tick_count = TIME_TO_TICKS(target.SimulationTime + SDKUtils::GetLerp());
					}
				}
				else
				{
					if (bIsFiring && target.LagRecord)
					{
						pCmd->tick_count = TIME_TO_TICKS(target.SimulationTime + GetClientInterpAmount());
					}
				}
			}
		}
	}
}
