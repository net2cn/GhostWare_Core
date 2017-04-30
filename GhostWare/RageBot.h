#pragma once

#include "Hacks.h"

#define M_PI 3.14159265358979323846
#define M_PI_F 3.14159265358979323846

#define IA 16807
#define IM 2147483647
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define MAX_RANDOM_RANGE 0x7FFFFFFFUL

#define AM (1.0/IM)
#define EPS 1.2e-7
#define RNMX (1.0-EPS) 

class CRageBot : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool& bSendPacket);
	void    RandomSeed(int iSeed);
	float    RandomFloat(float flMinVal = 0.0f, float flMaxVal = 1.0f);
	void ClampAngles(Vector& vecAngles);
	void NormalizeVector(Vector& vec);

	// Targetting
	int GetTargetCrosshair();
	int GetTargetDistance();
	int GetTargetHealth();
	bool TargetMeetsRequirements(IClientEntity* pEntity);
	float FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int HitBox);
	int HitScan(IClientEntity* pEntity);
	bool AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd);
	void CompansateSpread(CUserCmd *pCmd, CBaseCombatWeapon* pWeapon);

	// Functionality
	void DoAimbot(CUserCmd *pCmd, bool& bSendPacket);
	void DoNoSpread(CUserCmd *pCmd);
	void DoNoRecoil(CUserCmd *pCmd);

	// AntiAim
	void DoAntiAim(CUserCmd *pCmd, bool& bSendPacket);

	// AimStep
	bool IsAimStepping;
	Vector LastAimstepAngle;
	Vector LastAngle;

	// Aimbot
	bool IsLocked;
	int TargetID;
	int HitBox;
	Vector AimPoint;

	int        m_idum;
	int        m_iy;
	int        m_iv[NTAB];
	int        GenerateRandomNumber(void);

};