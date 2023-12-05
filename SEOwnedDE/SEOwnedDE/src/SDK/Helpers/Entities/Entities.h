#pragma once

#include "../../TF2/c_tf_player.h"

enum class EEntGroup
{
	PLAYERS_ALL,
	PLAYERS_ENEMIES,
	PLAYERS_TEAMMATES,
	PLAYERS_OBSERVER,

	BUILDINGS_ALL,
	BUILDINGS_ENEMIES,
	BUILDINGS_TEAMMATES,

	PROJECTILES_ALL,
	PROJECTILES_ENEMIES,
	PROJECTILES_TEAMMATES,
	PROJECTILES_LOCAL_STICKIES,

	HEALTHPACKS,
	AMMOPACKS,
	HALLOWEEN_GIFT,
	MVM_MONEY
};

class CEntityHelper
{
public:
	C_TFPlayer* GetLocal();
	C_TFWeaponBase* GetWeapon();

private:
	std::map<EEntGroup, std::vector<C_BaseEntity*>> m_mapGroups = {};
	std::map<int, bool> m_mapHealthPacks = {};
	std::map<int, bool> m_mapAmmoPacks = {};

	bool IsHealthPack(C_BaseEntity* pEntity)
	{
		return m_mapHealthPacks.contains(pEntity->m_nModelIndex());
	}

	bool IsAmmoPack(C_BaseEntity* pEntity)
	{
		return m_mapAmmoPacks.contains(pEntity->m_nModelIndex());
	}

public:
	void UpdateCache();
	void UpdateModelIndexes();
	void ClearCache();

	void ClearModelIndexes()
	{
		m_mapHealthPacks.clear();
		m_mapAmmoPacks.clear();
	}

	const std::vector<C_BaseEntity*>& GetGroup(const EEntGroup group) { return m_mapGroups[group]; }
};

MAKE_SINGLETON_SCOPED(CEntityHelper, Entities, H);
