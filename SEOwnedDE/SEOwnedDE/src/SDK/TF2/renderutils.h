#pragma once
#include "imaterialsystem.h"

namespace RenderUtils
{
	inline void RenderLine(const Vector& v1, const Vector& v2, Color_t c, bool bZBuffer)
	{
		using Fn = void(__cdecl *)(const Vector&, const Vector&, Color_t, bool);
		reinterpret_cast<Fn>(Signatures::RenderLine.Get())(v1, v2, c, bZBuffer);
	}

	inline void RenderBox(const Vector& origin, const QAngle& angles, const Vector& mins, const Vector& maxs, Color_t c, bool bZBuffer, bool bInsideOut = false)
	{
		using Fn = void(__cdecl *)(const Vector&, const QAngle&, const Vector&, const Vector&, Color_t, bool, bool);
		reinterpret_cast<Fn>(Memory::RelToAbs(Signatures::RenderBox.Get()))(origin, angles, mins, maxs, c, bZBuffer, bInsideOut);
	}

	inline void RenderWireframeBox(const Vector& vOrigin, const QAngle& angles, const Vector& vMins, const Vector& vMaxs, Color_t c, bool bZBuffer)
	{
		using Fn = void(__cdecl *)(const Vector&, const QAngle&, const Vector&, const Vector&, Color_t, bool);
		reinterpret_cast<Fn>(Signatures::RenderWireframeBox.Get())(vOrigin, angles, vMins, vMaxs, c, bZBuffer);
	}
}
