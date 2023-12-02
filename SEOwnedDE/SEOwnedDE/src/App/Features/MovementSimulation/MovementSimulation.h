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

		void Store(C_TFPlayer* pPlayer)
		{
			m_vecOrigin = pPlayer->m_vecOrigin();
			m_vecVelocity = pPlayer->m_vecVelocity();
			m_vecBaseVelocity = pPlayer->m_vecBaseVelocity();
			m_vecViewOffset = pPlayer->m_vecViewOffset();
			m_hGroundEntity = pPlayer->m_hGroundEntity();
			m_fFlags = pPlayer->m_fFlags();
			m_flDucktime = pPlayer->m_flDucktime();
			m_flDuckJumpTime = pPlayer->m_flDuckJumpTime();
			m_bDucked = pPlayer->m_bDucked();
			m_bDucking = pPlayer->m_bDucking();
			m_bInDuckJump = pPlayer->m_bInDuckJump();
			m_flModelScale = pPlayer->m_flModelScale();
			m_nButtons = pPlayer->m_nButtons();
			m_flLastMovementStunChange = pPlayer->m_flLastMovementStunChange();
			m_flStunLerpTarget = pPlayer->m_flStunLerpTarget();
			m_bStunNeedsFadeOut = pPlayer->m_bStunNeedsFadeOut();
			m_flPrevTauntYaw = pPlayer->m_flPrevTauntYaw();
			m_flTauntYaw = pPlayer->m_flTauntYaw();
			m_flCurrentTauntMoveSpeed = pPlayer->m_flCurrentTauntMoveSpeed();
			m_iKartState = pPlayer->m_iKartState();
			m_flVehicleReverseTime = pPlayer->m_flVehicleReverseTime();
			m_flHypeMeter = pPlayer->m_flHypeMeter();
			m_flMaxspeed = pPlayer->m_flMaxspeed();
			m_nAirDucked = pPlayer->m_nAirDucked();
			m_bJumping = pPlayer->m_bJumping();
			m_iAirDash = pPlayer->m_iAirDash();
			m_flWaterJumpTime = pPlayer->m_flWaterJumpTime();
			m_flSwimSoundTime = pPlayer->m_flSwimSoundTime();
			m_surfaceProps = pPlayer->m_surfaceProps();
			m_pSurfaceData = pPlayer->m_pSurfaceData();
			m_surfaceFriction = pPlayer->m_surfaceFriction();
			m_chTextureType = pPlayer->m_chTextureType();
			m_vecPunchAngle = pPlayer->m_vecPunchAngle();
			m_vecPunchAngleVel = pPlayer->m_vecPunchAngleVel();
			m_MoveType = pPlayer->m_MoveType();
			m_MoveCollide = pPlayer->m_MoveCollide();
			m_vecLadderNormal = pPlayer->m_vecLadderNormal();
			m_flGravity = pPlayer->m_flGravity();
			m_nWaterLevel = pPlayer->m_nWaterLevel_C_BaseEntity();
			m_nWaterType = pPlayer->m_nWaterType();
			m_flFallVelocity = pPlayer->m_flFallVelocity();
			m_nPlayerCond = pPlayer->m_nPlayerCond();
			m_nPlayerCondEx = pPlayer->m_nPlayerCondEx();
			m_nPlayerCondEx2 = pPlayer->m_nPlayerCondEx2();
			m_nPlayerCondEx3 = pPlayer->m_nPlayerCondEx3();
			m_nPlayerCondEx4 = pPlayer->m_nPlayerCondEx4();
			_condition_bits = pPlayer->_condition_bits();
		}

		void Restore(C_TFPlayer* pPlayer)
		{
			pPlayer->m_vecOrigin() = m_vecOrigin;
			pPlayer->m_vecVelocity() = m_vecVelocity;
			pPlayer->m_vecBaseVelocity() = m_vecBaseVelocity;
			pPlayer->m_vecViewOffset() = m_vecViewOffset;
			pPlayer->m_hGroundEntity() = m_hGroundEntity;
			pPlayer->m_fFlags() = m_fFlags;
			pPlayer->m_flDucktime() = m_flDucktime;
			pPlayer->m_flDuckJumpTime() = m_flDuckJumpTime;
			pPlayer->m_bDucked() = m_bDucked;
			pPlayer->m_bDucking() = m_bDucking;
			pPlayer->m_bInDuckJump() = m_bInDuckJump;
			pPlayer->m_flModelScale() = m_flModelScale;
			pPlayer->m_nButtons() = m_nButtons;
			pPlayer->m_flLastMovementStunChange() = m_flLastMovementStunChange;
			pPlayer->m_flStunLerpTarget() = m_flStunLerpTarget;
			pPlayer->m_bStunNeedsFadeOut() = m_bStunNeedsFadeOut;
			pPlayer->m_flPrevTauntYaw() = m_flPrevTauntYaw;
			pPlayer->m_flTauntYaw() = m_flTauntYaw;
			pPlayer->m_flCurrentTauntMoveSpeed() = m_flCurrentTauntMoveSpeed;
			pPlayer->m_iKartState() = m_iKartState;
			pPlayer->m_flVehicleReverseTime() = m_flVehicleReverseTime;
			pPlayer->m_flHypeMeter() = m_flHypeMeter;
			pPlayer->m_flMaxspeed() = m_flMaxspeed;
			pPlayer->m_nAirDucked() = m_nAirDucked;
			pPlayer->m_bJumping() = m_bJumping;
			pPlayer->m_iAirDash() = m_iAirDash;
			pPlayer->m_flWaterJumpTime() = m_flWaterJumpTime;
			pPlayer->m_flSwimSoundTime() = m_flSwimSoundTime;
			pPlayer->m_surfaceProps() = m_surfaceProps;
			pPlayer->m_pSurfaceData() = m_pSurfaceData;
			pPlayer->m_surfaceFriction() = m_surfaceFriction;
			pPlayer->m_chTextureType() = m_chTextureType;
			pPlayer->m_vecPunchAngle() = m_vecPunchAngle;
			pPlayer->m_vecPunchAngleVel() = m_vecPunchAngleVel;
			pPlayer->m_flJumpTime() = m_flJumpTime;
			pPlayer->m_MoveType() = m_MoveType;
			pPlayer->m_MoveCollide() = m_MoveCollide;
			pPlayer->m_vecLadderNormal() = m_vecLadderNormal;
			pPlayer->m_flGravity() = m_flGravity;
			pPlayer->m_nWaterLevel_C_BaseEntity() = m_nWaterLevel;
			pPlayer->m_nWaterType() = m_nWaterType;
			pPlayer->m_flFallVelocity() = m_flFallVelocity;
			pPlayer->m_nPlayerCond() = m_nPlayerCond;
			pPlayer->m_nPlayerCondEx() = m_nPlayerCondEx;
			pPlayer->m_nPlayerCondEx2() = m_nPlayerCondEx2;
			pPlayer->m_nPlayerCondEx3() = m_nPlayerCondEx3;
			pPlayer->m_nPlayerCondEx4() = m_nPlayerCondEx4;
			pPlayer->_condition_bits() = _condition_bits;
		}
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
