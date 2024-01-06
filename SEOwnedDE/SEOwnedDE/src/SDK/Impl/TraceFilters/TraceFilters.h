#pragma once
#include "../../TF2/IEngineTrace.h"

class CTraceFilterHitscan : public CTraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask) override;

	TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}

	C_BaseEntity *m_pIgnore = nullptr;
};

class CTraceFilterWorldCustom : public CTraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask) override;

	TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}

	C_BaseEntity *m_pTarget = nullptr;
};

class CTraceFilterArc : public CTraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pServerEntity, int contentsMask) override;

	TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}
};