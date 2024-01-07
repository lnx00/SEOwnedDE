#include "../../SDK/SDK.h"

MAKE_HOOK(
	CMultiPlayerAnimState_ResetGestureSlot, Signatures::CMultiPlayerAnimState_ResetGestureSlot.Get(),
	void, __fastcall, CMultiPlayerAnimState* ecx, void* edx, int iGestureSlot)
{
	if (const auto pLocal = H::Entities->GetLocal())
	{
		if (const auto pAnimState = ecx)
		{
			if (pAnimState->m_pEntity == pLocal && iGestureSlot == GESTURE_SLOT_VCD && G::bStartedFakeTaunt)
			{
				return;
			}
		}
	}

	CALL_ORIGINAL(ecx, edx, iGestureSlot);
}
