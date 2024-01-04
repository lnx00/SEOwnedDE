#include "../../SDK/SDK.h"
#include "../Features/CFG.h"
#include "../Features/LagRecords/LagRecords.h"

//this here belongs to boss
//bless boss

C_BaseEntity* GetRootMoveParent(C_BaseEntity* baseEnt)
{
	auto pEntity = baseEnt;
	auto pParent = baseEnt->GetMoveParent();

	auto its{ 0 };

	while (pParent)
	{
		if (its > 32) //XD
		{
			break;
		}

		its++;

		pEntity = pParent;
		pParent = pEntity->GetMoveParent();
	}

	return pEntity;
}

MAKE_HOOK(
	C_BaseAnimating_SetupBones, Signatures::C_BaseAnimating_SetupBones.Get(),
	bool, __fastcall, C_BaseAnimating* ecx, void* edx, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	if (CFG::Misc_SetupBones_Optimization && !F::LagRecords->IsSettingUpBones())
	{
		const auto baseEnt = reinterpret_cast<C_BaseEntity*>(reinterpret_cast<uintptr_t>(ecx) - 0x4);

		if (baseEnt)
		{
			const auto owner = GetRootMoveParent(baseEnt);
			const auto ent = owner ? owner : baseEnt;

			if (ent->GetClassId() == ETFClassIds::CTFPlayer && ent != H::Entities->GetLocal())
			{
				if (pBoneToWorldOut)
				{
					if (const auto bones = ent->As<C_BaseAnimating>()->GetCachedBoneData())
					{
						std::memcpy(pBoneToWorldOut, bones->Base(), sizeof(matrix3x4_t) * std::min(nMaxBones, bones->Count()));
					}
				}

				return true;
			}
		}
	}

	return CALL_ORIGINAL(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}
