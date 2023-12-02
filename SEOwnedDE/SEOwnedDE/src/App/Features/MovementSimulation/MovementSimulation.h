#pragma once

#include "../../../SDK/SDK.h"

class CMovementSimulation
{
	//since we're going to call game functions some entity data will be modified (we'll modify it too), we'll have to restore it after running
	class CPlayerDataBackup
	{
	public:
		Vec3 m_vecOrigin = {};
		Vec3 m_vecVelocity = {};
		Vec3 m_vecBaseVelocity = {};
		Vec3 m_vecViewOffset = {};
		C_BaseEntity* m_hGroundEntity = nullptr;
		int m_fFlags = 0;
		float m_flDucktime = 0.0f;
		float m_flDuckJumpTime = 0.0f;
		bool m_bDucked = false;
		bool m_bDucking = false;
		bool m_bInDuckJump = false;
		float m_flModelScale = 0.0f;
		int m_nButtons = 0;
		float m_flLastMovementStunChange = 0.0f;
		float m_flStunLerpTarget = 0.0f;
		bool m_bStunNeedsFadeOut = false;
		float m_flPrevTauntYaw = 0.0f;
		float m_flTauntYaw = 0.0f;
		float m_flCurrentTauntMoveSpeed = 0.0f;
		int m_iKartState = 0;
		float m_flVehicleReverseTime = 0.0f;
		float m_flHypeMeter = 0.0f;
		float m_flMaxspeed = 0.0f;
		int m_nAirDucked = 0;
		bool m_bJumping = false;
		int m_iAirDash = 0;
		float m_flWaterJumpTime = 0.0f;
		float m_flSwimSoundTime = 0.0f;
		int m_surfaceProps = 0;
		void* m_pSurfaceData = nullptr;
		float m_surfaceFriction = 0.0f;
		char m_chTextureType = 0;
		Vec3 m_vecPunchAngle = {};
		Vec3 m_vecPunchAngleVel = {};
		float m_flJumpTime = 0.0f;
		unsigned char m_MoveType = 0;
		unsigned char m_MoveCollide = 0;
		Vec3 m_vecLadderNormal = {};
		float m_flGravity = 0.0f;
		unsigned char m_nWaterLevel = 0;
		unsigned char m_nWaterType = 0;
		float m_flFallVelocity = 0.0f;
		int m_nPlayerCond = 0;
		int m_nPlayerCondEx = 0;
		int m_nPlayerCondEx2 = 0;
		int m_nPlayerCondEx3 = 0;
		int m_nPlayerCondEx4 = 0;
		int _condition_bits = 0;

		void Store(C_TFPlayer* pPlayer);
		void Restore(C_TFPlayer* pPlayer);
	};
	CPlayerDataBackup m_PlayerDataBackup = {};

	C_TFPlayer* m_pPlayer = nullptr;
	CMoveData m_MoveData = {};
	bool m_bRunning = false;
	float m_flYawTurnRate = 0.0f;

	bool m_bOldInPrediction = false;
	bool m_bOldFirstTimePredicted = false;
	float m_flOldFrametime = 0.0f;

	void SetupMoveData(C_TFPlayer* pPlayer, CMoveData* pMoveData);

public:
	bool Initialize(C_TFPlayer* pPlayer);
	void Restore();
	void RunTick(float flTimeToTarget = 0.0f);

	const Vec3& GetOrigin() { return m_MoveData.m_vecAbsOrigin; }
	bool IsRunning() { return m_bRunning; }
};

MAKE_SINGLETON_SCOPED(CMovementSimulation, MovementSimulation, F);
