#include "Crits.h"

#include "../CFG.h"

bool IsFiring(const CUserCmd* pCmd, C_TFWeaponBase* weapon)
{
	if (weapon->GetSlot() != 2)
	{
		if (!weapon->HasPrimaryAmmoForShot())
			return false;

		const int nWeaponID = weapon->GetWeaponID();

		if (nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER || nWeaponID == TF_WEAPON_CANNON)
			return (G::nOldButtons & IN_ATTACK) && !(pCmd->buttons & IN_ATTACK);

		if (weapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
			return G::bCanPrimaryAttack;
	}

	return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}

int FindCritCmd(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon, bool bCrit)
{
	const int nBackupRandomSeed = *SDKUtils::RandomSeed();
	int nCommandNumber = pCmd->command_number;
	int nCritCommand = nCommandNumber;

	for (int n = 0; n < 4096; n++)
	{
		*SDKUtils::RandomSeed() = MD5_PseudoRandom(nCommandNumber) & 0x7FFFFFFF;

		bool bCalc = false;

		if (pWeapon->GetSlot() == 2)
		{
			bCalc = pWeapon->CalcIsAttackCriticalHelperMelee();
		}
		else
		{
			bCalc = pWeapon->CalcIsAttackCriticalHelper();
		}

		if (bCrit ? bCalc : !bCalc)
		{
			nCritCommand = nCommandNumber;
			break;
		}

		nCommandNumber++;
	}

	*SDKUtils::RandomSeed() = nBackupRandomSeed;
	return nCritCommand;
}

void CCrits::Run(CUserCmd* pCmd)
{
	const auto pLocal = H::Entities->GetLocal();

	if (!pLocal || pLocal->deadflag() || pLocal->IsCritBoosted() || pLocal->IsMiniCritBoosted())
		return;

	static auto tf_weapon_criticals = I::CVar->FindVar("tf_weapon_criticals");

	if (!tf_weapon_criticals->GetInt())
	{
		return;
	}

	const auto pWeapon = H::Entities->GetWeapon();

	if (!pWeapon || pWeapon->GetWeaponID() == TF_WEAPON_KNIFE || !IsFiring(pCmd, pWeapon))
		return;

	// Melee crits
	if (pWeapon->GetSlot() == 2)
	{
		static auto tf_weapon_criticals_melee = I::CVar->FindVar("tf_weapon_criticals_melee");

		if (!tf_weapon_criticals_melee->GetInt())
		{
			return;
		}

		if (H::Input->IsDown(CFG::Exploits_Crits_Force_Crit_Key_Melee))
		{
			bool wantCrit = true;

			// Check if the aim target should be critted
			if (G::nTargetIndex)
			{
				const auto ent = I::ClientEntityList->GetClientEntity(G::nTargetIndex);

				if (ent && ent->GetClassId() == ETFClassIds::CTFPlayer && ent->As<C_TFPlayer>()->m_iTeamNum() == pLocal->m_iTeamNum())
				{
					wantCrit = false;
				}
			}

			if (wantCrit)
			{
				pCmd->command_number = FindCritCmd(pCmd, pWeapon, true);
			}
			else
			{
				if (CFG::Exploits_Crits_Skip_Random_Crits)
				{
					pCmd->command_number = FindCritCmd(pCmd, pWeapon, false);
				}
			}
		}
		else
		{
			if (CFG::Exploits_Crits_Skip_Random_Crits)
			{
				pCmd->command_number = FindCritCmd(pCmd, pWeapon, false);
			}
		}
	}

	// Hitscan crits
	else
	{
		if (H::Input->IsDown(CFG::Exploits_Crits_Force_Crit_Key))
		{
			pCmd->command_number = FindCritCmd(pCmd, pWeapon, true);
		}
		else
		{
			if (CFG::Exploits_Crits_Skip_Random_Crits)
			{
				pCmd->command_number = FindCritCmd(pCmd, pWeapon, false);
			}
		}
	}
}
