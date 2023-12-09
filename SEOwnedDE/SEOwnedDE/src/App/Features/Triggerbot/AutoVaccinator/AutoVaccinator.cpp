#include "AutoVaccinator.h"

#include "../../CFG.h"

constexpr float HEALTH_LIMIT = 0.85f;

medigun_resist_types_t WeaponIDToResType(int weaponID)
{
	switch (weaponID)
	{
	case TF_WEAPON_SHOTGUN_PRIMARY:
	case TF_WEAPON_SHOTGUN_SOLDIER:
	case TF_WEAPON_SHOTGUN_HWG:
	case TF_WEAPON_SHOTGUN_PYRO:
	case TF_WEAPON_SCATTERGUN:
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_MINIGUN:
	case TF_WEAPON_SMG:
	case TF_WEAPON_PISTOL:
	case TF_WEAPON_PISTOL_SCOUT:
	case TF_WEAPON_REVOLVER:
	case TF_WEAPON_SENTRY_BULLET:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_SENTRY_REVENGE:
	case TF_WEAPON_HANDGUN_SCOUT_PRIMARY:
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_HANDGUN_SCOUT_SECONDARY:
	case TF_WEAPON_SODA_POPPER:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_PEP_BRAWLER_BLASTER:
	case TF_WEAPON_CHARGED_SMG:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	case TF_WEAPON_RAYGUN:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	case TF_WEAPON_DRG_POMSON:
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	case TF_WEAPON_LASER_POINTER:
		{
			return MEDIGUN_BULLET_RESIST;
		}

	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_GRENADELAUNCHER:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_FLAMETHROWER_ROCKET:
	case TF_WEAPON_GRENADE_DEMOMAN:
	case TF_WEAPON_SENTRY_ROCKET:
	case TF_WEAPON_PUMPKIN_BOMB:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_CANNON:
	case TF_WEAPON_PARTICLE_CANNON:
		{
			return MEDIGUN_BLAST_RESIST;
		}

	case TF_WEAPON_FLAMETHROWER:
	case TF_WEAPON_FLAREGUN:
	case TF_WEAPON_FLAREGUN_REVENGE:
	case TF_WEAPON_FLAME_BALL:
		{
			return MEDIGUN_FIRE_RESIST;
		}

	default:
		{
			return MEDIGUN_NUM_RESISTS;
		}
	}
}

bool IsPlayerInDanger(C_TFPlayer* player, medigun_resist_types_t& dangerType)
{
	if (!player || player->IsInvulnerable())
	{
		return false;
	}

	const auto percentHealth{
		Math::RemapValClamped
		(
			static_cast<float>(player->m_iHealth()), 0.0f, static_cast<float>(player->GetMaxHealth()), 0.0f, 1.0f
		)
	};

	//I::DebugOverlay->ClearAllOverlays();

	// Check for dangerous players
	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
	{
		if (!pEntity)
			continue;

		const auto enemy{ pEntity->As<C_TFPlayer>() };
		if (!enemy || enemy->deadflag())
			continue;

		// Is it a sniper?
		if (enemy->m_iClass() != TF_CLASS_SNIPER)
			continue;

		// Sniper rifle?
		const auto weapon{ enemy->m_hActiveWeapon().Get()->As<C_TFWeaponBase>() };
		if (!weapon || weapon->GetSlot() != WEAPON_SLOT_PRIMARY || weapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)
			continue;

		// In-Scope?
		bool zoomed{ enemy->InCond(TF_COND_ZOOMED) };
		if (weapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
		{
			zoomed = weapon->As<C_TFSniperRifleClassic>()->m_bCharging();
		}

		if (!enemy->IsCritBoosted() && !enemy->IsMiniCritBoosted() && !zoomed)
			continue;

		auto mins{ player->m_vecMins() };
		auto maxs{ player->m_vecMaxs() };

		mins.x *= 3.0f;
		mins.y *= 3.0f;
		mins.z *= 1.5f;

		maxs.x *= 3.0f;
		maxs.y *= 3.0f;
		maxs.z *= 1.5f;

		Vec3 forward{};
		Math::AngleVectors(enemy->GetEyeAngles(), &forward);

		// Can the sniper shoot us?
		if (!Math::RayToOBB(enemy->GetShootPos(), forward, player->m_vecOrigin(), mins, maxs, player->RenderableToWorldTransform()))
			continue;

		//I::DebugOverlay->AddSweptBoxOverlay(player->m_vecOrigin(), player->m_vecOrigin(), mins, maxs, {}, 0, 255, 0, 255, 0.1f);

		const auto visibleFromCenter
		{
			H::AimUtils->TraceEntityAutoDet
			(
				pEntity,
				player->GetCenter(),
				enemy->m_vecOrigin() + Vec3{ 0.0f, 0.0f, enemy->m_vecMaxs().z }
			)
		};

		const auto visibleFromHead
		{
			H::AimUtils->TraceEntityAutoDet
			(
				pEntity,
				player->m_vecOrigin() + Vec3{ 0.0f, 0.0f, player->m_vecMaxs().z },
				enemy->m_vecOrigin() + Vec3{ 0.0f, 0.0f, enemy->m_vecMaxs().z }
			)
		};

		if (!visibleFromCenter && !visibleFromHead)
			continue;

		// Dangerous sniper!
		dangerType = MEDIGUN_BULLET_RESIST;
		return true;
	}

	size_t numClosePipebombs{};

	// Check for dangerous projectiles
	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PROJECTILES_ENEMIES))
	{
		if (!pEntity)
			continue;

		const auto visibleFromCenter{ H::AimUtils->TraceEntityAutoDet(pEntity, player->GetCenter(), pEntity->GetCenter()) };
		const auto visibleFromHead{ H::AimUtils->TraceEntityAutoDet(pEntity, player->m_vecOrigin() + Vec3{ 0.0f, 0.0f, player->m_vecMaxs().z }, pEntity->GetCenter()) };

		if (!visibleFromCenter && !visibleFromHead)
			continue;

		Vec3 vel{};
		pEntity->EstimateAbsVelocity(vel);

		auto entOrigin{ pEntity->m_vecOrigin() + (vel * SDKUtils::GetLatency()) };

		switch (pEntity->GetClassId())
		{
		case ETFClassIds::CTFProjectile_Arrow:
			{
				if (vel.IsZero() || entOrigin.DistTo(player->GetCenter()) > 150.0f)
					continue;

				// Dangerous arrows
				dangerType = MEDIGUN_BULLET_RESIST;
				return true;
			}

		case ETFClassIds::CTFProjectile_HealingBolt:
			{
				if (vel.IsZero() || entOrigin.DistTo(player->GetCenter()) > 150.0f)
					continue;

				const auto arrow{ pEntity->As<C_TFProjectile_Arrow>() };
				if (!arrow->m_bCritical() && percentHealth >= HEALTH_LIMIT)
					continue;

				// Dangerous medigun bolt
				dangerType = MEDIGUN_BULLET_RESIST;
				return true;
			}

		case ETFClassIds::CTFProjectile_Rocket:
		case ETFClassIds::CTFProjectile_SentryRocket:
		case ETFClassIds::CTFProjectile_EnergyBall:
			{
				if (entOrigin.DistTo(player->GetCenter()) > 250.0f)
					continue;

				const auto rocket{ pEntity->As<C_TFProjectile_Rocket>() };
				if (!rocket->m_bCritical() && percentHealth >= 1.0f)
					continue;

				// Dangerous rocket
				dangerType = MEDIGUN_BLAST_RESIST;
				return true;
			}

		case ETFClassIds::CTFGrenadePipebombProjectile:
			{
				if (entOrigin.DistTo(player->GetCenter()) > 250.0f)
					continue;

				const auto bomb{ pEntity->As<C_TFGrenadePipebombProjectile>() };
				if (bomb->m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE)
					continue;

				if (!bomb->m_bCritical() && percentHealth >= 1.0f && entOrigin.DistTo(player->GetCenter()) >= 100.0f)
				{
					numClosePipebombs++;
					continue;
				}

				// Dangerous pipebomb
				dangerType = MEDIGUN_BLAST_RESIST;
				return true;
			}

		case ETFClassIds::CTFProjectile_Flare:
			{
				if (entOrigin.DistTo(player->GetCenter()) > 150.0f)
					continue;

				const auto flare{ pEntity->As<C_TFProjectile_Flare>() };
				if (!flare->m_bCritical() && !player->InCond(TF_COND_BURNING) && !player->InCond(TF_COND_BURNING_PYRO))
					continue;

				// Dangerous flare
				dangerType = MEDIGUN_FIRE_RESIST;
				return true;
			}

		case ETFClassIds::CTFProjectile_BallOfFire:
			{
				if (entOrigin.DistTo(player->GetCenter()) > 150.0f)
					continue;

				if (!player->InCond(TF_COND_BURNING) && !player->InCond(TF_COND_BURNING_PYRO))
					continue;

				// Dangerous fireball
				dangerType = MEDIGUN_FIRE_RESIST;
				return true;
			}

		case ETFClassIds::CTFProjectile_EnergyRing:
			{
				if (entOrigin.DistTo(player->GetCenter()) > 150.0f)
					continue;

				if (percentHealth >= HEALTH_LIMIT)
					continue;

				// Dangerous energy ball
				dangerType = MEDIGUN_BULLET_RESIST;
				return true;
			}

		default: {}
		}
	}

	if (numClosePipebombs > 0)
	{
		dangerType = MEDIGUN_BLAST_RESIST;

		if (numClosePipebombs > 1)
		{
			return true;
		}
	}

	// Check for dangerous sentry guns
	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::BUILDINGS_ENEMIES))
	{
		if (!pEntity || pEntity->GetClassId() != ETFClassIds::CObjectSentrygun)
			continue;

		C_ObjectSentrygun* sentrygun{ pEntity->As<C_ObjectSentrygun>() };
		if (!sentrygun)
			continue;

		if (sentrygun->m_hAutoAimTarget() != player && sentrygun->m_hEnemy() != player)
			continue;

		dangerType = MEDIGUN_BULLET_RESIST;

		if (percentHealth < HEALTH_LIMIT)
			return true;
	}

	return false;
}

bool PlayerHasResUber(medigun_resist_types_t res, C_TFPlayer* player)
{
	switch (res)
	{
	case MEDIGUN_BULLET_RESIST:
		return player->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST);

	case MEDIGUN_BLAST_RESIST:
		return player->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST);

	case MEDIGUN_FIRE_RESIST:
		return player->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST);

	default:
		return false;
	}
}

void CAutoVaccinator::Reset()
{
	m_SimResType = MEDIGUN_NUM_RESISTS;
	m_GoalResType = MEDIGUN_NUM_RESISTS;
	m_IsChangingRes = false;
	m_ShouldPop = false;
}

void CAutoVaccinator::Run(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* cmd)
{
	if (!CFG::AutoVaccinator_Active || !G::bCanPrimaryAttack)
	{
		Reset();
		return;
	}

	// Valid weapon?
	if (pWeapon->GetWeaponID() != TF_WEAPON_MEDIGUN)
	{
		Reset();
		return;
	}

	// Valid medigun?
	const auto medigun{ pWeapon->As<C_WeaponMedigun>() };
	if (!medigun || medigun->GetChargeType() < 3)
	{
		Reset();
		return;
	}

	// Update resistant type
	const auto curResType{ medigun->GetResistType() };
	if (m_SimResType == MEDIGUN_NUM_RESISTS)
	{
		m_SimResType = curResType;
	}

	// Switch resistance
	if (m_SimResType != m_GoalResType && m_GoalResType != MEDIGUN_NUM_RESISTS)
	{
		m_IsChangingRes = true;

		if (!(G::nOldButtons & IN_RELOAD))
		{
			cmd->buttons |= IN_RELOAD;

			m_SimResType++;

			if (m_SimResType > 2)
			{
				m_SimResType = 0;
			}
		}

		return;
	}
	m_IsChangingRes = false;

	m_GoalResType = MEDIGUN_NUM_RESISTS;

	if (cmd->buttons & IN_RELOAD)
	{
		cmd->buttons &= ~IN_RELOAD;
	}

	/*if ((cmd->buttons & IN_RELOAD) && !(G::nOldButtons & IN_RELOAD) && G::bCanPrimaryAttack)
		{
			m_SimResType++;

			if (m_SimResType > 2)
			{
				m_SimResType = 0;
			}

			m_GoalResType = m_SimResType;
		}*/

	// Pop vaccinator
	if (m_ShouldPop)
	{
		m_ShouldPop = false;

		cmd->buttons |= IN_ATTACK2;

		return;
	}

	// Update danger status
	if (!m_IsChangingRes && !m_ShouldPop)
	{
		const auto healTarget{ medigun->m_hHealingTarget().Get()->As<C_TFPlayer>() };
		medigun_resist_types_t dangerType{ MEDIGUN_NUM_RESISTS };

		if (IsPlayerInDanger(healTarget, dangerType) && !PlayerHasResUber(dangerType, healTarget))
		{
			m_GoalResType = dangerType;

			if (medigun->m_flChargeLevel() >= 0.25f)
			{
				m_ShouldPop = true;

				if (CFG::AutoVaccinator_Pop == 1 && !healTarget->IsPlayerOnSteamFriendsList())
				{
					m_ShouldPop = false;
				}
			}
		}
		else
		{
			if (IsPlayerInDanger(pLocal, dangerType) && !PlayerHasResUber(dangerType, pLocal))
			{
				m_GoalResType = dangerType;

				if (medigun->m_flChargeLevel() >= 0.25f)
				{
					m_ShouldPop = true;
				}
			}
		}

		if (dangerType != MEDIGUN_NUM_RESISTS)
		{
			m_GoalResType = dangerType;
		}
	}
}

void CAutoVaccinator::ProcessPlayerHurt(IGameEvent* event)
{
	if (!CFG::AutoVaccinator_Active)
		return;

	if (m_IsChangingRes || m_ShouldPop)
		return;

	const auto pLocal{ H::Entities->GetLocal() };
	if (!pLocal || pLocal->deadflag())
		return;

	const auto weapon{ H::Entities->GetWeapon() };
	if (!weapon || weapon->GetWeaponID() != TF_WEAPON_MEDIGUN)
		return;

	const auto medigun{ weapon->As<C_WeaponMedigun>() };
	if (!medigun || medigun->GetChargeType() < 3)
		return;

	const auto victim{ GET_ENT_FROM_USER_ID(event->GetInt("userid")) };
	const auto attacker{ GET_ENT_FROM_USER_ID(event->GetInt("attacker")) };

	if (!victim || victim == attacker)
		return;

	const auto health{ event->GetInt("health") };
	if (health <= 0)
		return;

	const auto weaponID{ event->GetInt("weaponid") };
	if (!weaponID)
		return;

	if (victim != pLocal && victim != medigun->m_hHealingTarget())
		return;

	const auto victimEnt{ victim->As<C_TFPlayer>() };
	if (!victimEnt)
		return;

	const auto percentHealth{
		Math::RemapValClamped
		(
			static_cast<float>(health), 0.0f, static_cast<float>(victimEnt->GetMaxHealth()), 0.0f, 1.0f
		)
	};

	m_GoalResType = WeaponIDToResType(weaponID);

	if (medigun->m_flChargeLevel() >= 0.25f)
	{
		if (m_GoalResType == MEDIGUN_BULLET_RESIST && !PlayerHasResUber(MEDIGUN_BULLET_RESIST, victimEnt))
		{
			if (event->GetBool("crit") || event->GetBool("minicrit") || percentHealth < HEALTH_LIMIT)
			{
				m_ShouldPop = true;
			}
		}

		if (!m_ShouldPop && m_GoalResType == MEDIGUN_FIRE_RESIST && !PlayerHasResUber(MEDIGUN_FIRE_RESIST, victimEnt))
		{
			if (event->GetBool("crit") || event->GetBool("minicrit") || percentHealth < HEALTH_LIMIT)
			{
				m_ShouldPop = true;
			}
		}
	}

	const auto healTarget{ medigun->m_hHealingTarget().Get() };
	if (victim == healTarget && CFG::AutoVaccinator_Pop == 1 && !healTarget->As<C_TFPlayer>()->IsPlayerOnSteamFriendsList())
	{
		m_ShouldPop = false;
	}
}

void CAutoVaccinator::PreventReload(CUserCmd* cmd)
{
	if (!CFG::AutoVaccinator_Active || m_IsChangingRes)
		return;

	const auto weapon{ H::Entities->GetWeapon() };
	if (!weapon || weapon->GetWeaponID() != TF_WEAPON_MEDIGUN)
		return;

	const auto medigun{ weapon->As<C_WeaponMedigun>() };
	if (!medigun || medigun->GetChargeType() < 3)
		return;

	if (cmd->buttons & IN_RELOAD)
	{
		cmd->buttons &= ~IN_RELOAD;
	}
}
