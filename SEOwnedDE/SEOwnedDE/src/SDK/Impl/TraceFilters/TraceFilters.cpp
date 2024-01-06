#include "TraceFilters.h"
#include "../../SDK.h"

bool CTraceFilterHitscan::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
	auto pLocal = H::Entities->GetLocal();
	auto pWeapon = H::Entities->GetWeapon();

	if (!pLocal || !pWeapon)
		return false;

	if (!pServerEntity || pServerEntity == m_pIgnore || pServerEntity == pLocal)
		return false;

	if (auto pEntity = static_cast<IClientEntity *>(pServerEntity)->As<C_BaseEntity>())
	{
		switch (pEntity->GetClassId())
		{
			case ETFClassIds::CFuncAreaPortalWindow:
			case ETFClassIds::CFuncRespawnRoomVisualizer:
			case ETFClassIds::CSniperDot:
			case ETFClassIds::CTFAmmoPack: return false;

			case ETFClassIds::CTFMedigunShield:
			{
				if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
					return false;

				break;
			}

			case ETFClassIds::CTFPlayer:
			case ETFClassIds::CObjectSentrygun:
			case ETFClassIds::CObjectDispenser:
			case ETFClassIds::CObjectTeleporter:
			{
				switch (pWeapon->GetWeaponID())
				{
					case TF_WEAPON_SNIPERRIFLE:
					case TF_WEAPON_SNIPERRIFLE_CLASSIC:
					case TF_WEAPON_SNIPERRIFLE_DECAP:
					{
						if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
							return false;

						break;
					}

					default: break;
				}

				break;
			}

			default: break;
		}

	}

	return true;
}

bool CTraceFilterWorldCustom::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
	if (auto pEntity = static_cast<IClientEntity *>(pServerEntity)->As<C_BaseEntity>())
	{
		switch (pEntity->GetClassId())
		{
			case ETFClassIds::CTFPlayer:
			case ETFClassIds::CObjectSentrygun:
			case ETFClassIds::CObjectDispenser:
			case ETFClassIds::CObjectTeleporter: return pEntity == m_pTarget;

			case ETFClassIds::CObjectCartDispenser:
			case ETFClassIds::CBaseDoor:
			case ETFClassIds::CPhysicsProp:
			case ETFClassIds::CDynamicProp:
			case ETFClassIds::CBaseEntity:
			case ETFClassIds::CFuncTrackTrain: return true;

			default: return false;
		}
	}

	return false;
}

bool CTraceFilterArc::ShouldHitEntity(IHandleEntity* pServerEntity, int contentsMask)
{
	if (const auto pEntity = static_cast<IClientEntity*>(pServerEntity)->As<C_BaseEntity>())
	{
		switch (pEntity->GetClassId())
		{
			case ETFClassIds::CTFPlayer:
			case ETFClassIds::CObjectSentrygun:
			case ETFClassIds::CObjectDispenser:
			case ETFClassIds::CObjectTeleporter:
			case ETFClassIds::CObjectCartDispenser:
			case ETFClassIds::CBaseDoor:
			case ETFClassIds::CPhysicsProp:
			case ETFClassIds::CDynamicProp:
			case ETFClassIds::CBaseEntity:
			case ETFClassIds::CFuncTrackTrain:
			{
				return true;
			}

			default:
			{
				return false;
			}
		}
	}

	return false;
}
