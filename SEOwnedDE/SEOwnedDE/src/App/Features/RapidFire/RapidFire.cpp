#include "RapidFire.h"

#include "../CFG.h"

bool IsRapidFireWeapon(C_TFWeaponBase* pWeapon)
{
	if (!pWeapon)
		return false;

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_MINIGUN:
	case TF_WEAPON_PISTOL:
	case TF_WEAPON_PISTOL_SCOUT:
	case TF_WEAPON_SMG: return true;
	default: return false;
	}
}

bool CRapidFire::ShouldStart(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	if (G::nTicksSinceCanFire < 24 || G::nTargetIndex <= 1 || !G::bFiring || Shifting::bShifting || Shifting::bRecharging || Shifting::bShiftingWarp)
		return false;

	if (Shifting::nAvailableTicks < CFG::Exploits_RapidFire_Ticks)
		return false;

	if (!(H::Input->IsDown(CFG::Exploits_RapidFire_Key)))
		return false;

	if (!IsWeaponSupported(pWeapon))
		return false;

	return true;
}

void CRapidFire::Run(CUserCmd* pCmd, bool* pSendPacket)
{
	Shifting::bRapidFireWantShift = false;

	const auto pLocal = H::Entities->GetLocal();

	if (!pLocal)
		return;

	const auto pWeapon = H::Entities->GetWeapon();

	if (!pWeapon)
		return;

	if (ShouldStart(pLocal, pWeapon))
	{
		//hacky
		if (G::nTicksTargetSame < CFG::Exploits_RapidFire_Min_Ticks_Target_Same)
		{
			pCmd->buttons &= ~IN_ATTACK;
			G::bFiring = false;
			return;
		}

		Shifting::bRapidFireWantShift = true;

		m_ShiftCmd = *pCmd;
		m_bShiftSilentAngles = G::bSilentAngles || G::bPSilentAngles;
		m_bSetCommand = false;

		/*if (CFG::Exploits_RapidFire_Antiwarp)
		{
			Vec3 vAngle = {};
			Math::VectorAngles(pLocal->m_vecVelocity(), vAngle);

			pCmd->viewangles.x = 90.0f;
			pCmd->viewangles.y = vAngle.y;
			pCmd->forwardmove = pCmd->sidemove = 0.0f;

			G::bSilentAngles = true;
		}*/

		pCmd->buttons &= ~IN_ATTACK;

		*pSendPacket = true;

		m_vShiftStart = pLocal->m_vecOrigin();
		m_bStartedShiftOnGround = pLocal->m_fFlags() & FL_ONGROUND;
	}
}

bool CRapidFire::ShouldExitCreateMove(CUserCmd* pCmd)
{
	const auto pLocal = H::Entities->GetLocal();

	if (!pLocal)
		return false;

	if (Shifting::bShifting && !Shifting::bShiftingWarp)
	{
		m_ShiftCmd.command_number = pCmd->command_number;

		if (!m_bSetCommand)
		{
			*pCmd = m_ShiftCmd;
			m_bSetCommand = true;
		}

		if (CFG::Exploits_RapidFire_Antiwarp && m_bStartedShiftOnGround)
		{
			*pCmd = m_ShiftCmd;
			const float moveScale = Math::RemapValClamped(static_cast<float>(CFG::Exploits_RapidFire_Ticks), 14.0f, 22.0f, 0.605f, 1.0f);
			SDKUtils::WalkTo(pCmd, pLocal->m_vecOrigin(), m_vShiftStart, moveScale);
		}

		else
		{
			const auto pWeapon = H::Entities->GetWeapon();
			if (IsRapidFireWeapon(pWeapon))
				*pCmd = m_ShiftCmd;
		}

		return true;
	}

	return false;
}

bool CRapidFire::IsWeaponSupported(C_TFWeaponBase* pWeapon)
{
	const auto nWeaponType = H::AimUtils->GetWeaponType(pWeapon);

	if (nWeaponType == EWeaponType::MELEE || nWeaponType == EWeaponType::OTHER)
		return false;

	const auto nWeaponID = pWeapon->GetWeaponID();

	if (nWeaponID == TF_WEAPON_CROSSBOW
		|| nWeaponID == TF_WEAPON_COMPOUND_BOW
		|| nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC
		|| nWeaponID == TF_WEAPON_FLAREGUN
		|| nWeaponID == TF_WEAPON_FLAREGUN_REVENGE
		|| nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER
		|| nWeaponID == TF_WEAPON_FLAMETHROWER
		|| nWeaponID == TF_WEAPON_CANNON
		|| pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
		return false;

	return true;
}
