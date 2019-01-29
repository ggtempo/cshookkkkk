#pragma once
#include "Vec3.hpp"
#include "Globals.hpp"

typedef enum _fieldtypes
{
	FIELD_FLOAT = 0,		// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_ENTITY,			// An entity offset (EOFFSET)
	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EVARS,			// EVARS *
	FIELD_EDICT,			// edict_t *, or edict_t *  (same thing)
	FIELD_VECTOR,			// Any vector
	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_POINTER,			// Arbitrary data pointer... to be removed, use an array of FIELD_CHARACTER
	FIELD_INTEGER,			// Any integer or enum
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_TYPECOUNT,		// MUST BE LAST
} FIELDTYPE;

typedef struct 
{
	FIELDTYPE		fieldType;
	char			*fieldName;
	int				fieldOffset;
	short			fieldSize;
	short			flags;
} TYPEDESCRIPTION;

#define WEAPON_ALLWEAPONS (~(1 << WEAPON_SUIT))
#define WEAPON_SUIT 31
#define MAX_WEAPONS 32

#define WPNSTATE_USP_SILENCED (1<<0)
#define WPNSTATE_GLOCK18_BURST_MODE (1<<1)
#define WPNSTATE_M4A1_SILENCED (1<<2)
#define WPNSTATE_ELITE_LEFT (1<<3)
#define WPNSTATE_FAMAS_BURST_MODE (1<<4)
#define WPNSTATE_SHIELD_DRAWN (1<<5)

#define MAX_AMMO_TYPES 32
#define MAX_AMMO_SLOTS 32

#define AK47_WEIGHT 25
#define AUG_WEIGHT 25
#define AWP_WEIGHT 30
#define C4_WEIGHT 3
#define DEAGLE_WEIGHT 7
#define ELITE_WEIGHT 5
#define FAMAS_WEIGHT 75
#define FIVESEVEN_WEIGHT 5
#define FLASHBANG_WEIGHT 1
#define G3SG1_WEIGHT 20
#define GALIL_WEIGHT 25
#define GLOCK18_WEIGHT 5
#define HEGRENADE_WEIGHT 2
#define KNIFE_WEIGHT 0
#define M249_WEIGHT 25
#define M3_WEIGHT 20
#define M4A1_WEIGHT 25
#define MAC10_WEIGHT 25
#define MP5NAVY_WEIGHT 25
#define P228_WEIGHT 5
#define P90_WEIGHT 26
#define SCOUT_WEIGHT 30
#define SG550_WEIGHT 20
#define SG552_WEIGHT 25
#define SMOKEGRENADE_WEIGHT 1
#define TMP_WEIGHT 25
#define UMP45_WEIGHT 25
#define USP_WEIGHT 5
#define XM1014_WEIGHT 20

#define MAX_NORMAL_BATTERY 100

#define WEAPON_NOCLIP -1

#define AK47_DEFAULT_GIVE 30
#define AUG_DEFAULT_GIVE 30
#define AWP_DEFAULT_GIVE 10
#define C4_DEFAULT_GIVE 1
#define DEAGLE_DEFAULT_GIVE 7
#define ELITE_DEFAULT_GIVE 30
#define FAMAS_DEFAULT_GIVE 25
#define FIVESEVEN_DEFAULT_GIVE 20
#define FLASHBANG_DEFAULT_GIVE 1
#define G3SG1_DEFAULT_GIVE 20
#define GALIL_DEFAULT_GIVE 35
#define GLOCK18_DEFAULT_GIVE 20
#define HEGRENADE_DEFAULT_GIVE 1
#define M249_DEFAULT_GIVE 100
#define M3_DEFAULT_GIVE 8
#define M4A1_DEFAULT_GIVE 30
#define MAC10_DEFAULT_GIVE 30
#define MP5NAVY_DEFAULT_GIVE 30
#define P228_DEFAULT_GIVE 13
#define P90_DEFAULT_GIVE 50
#define SCOUT_DEFAULT_GIVE 10
#define SG550_DEFAULT_GIVE 30
#define SG552_DEFAULT_GIVE 30
#define MAC10_DEFAULT_GIVE 30
#define SMOKEGRENADE_DEFAULT_GIVE 1
#define TMP_DEFAULT_GIVE 30
#define UMP45_DEFAULT_GIVE 25
#define USP_DEFAULT_GIVE 12
#define XM1014_DEFAULT_GIVE 7

#define AMMO_9MM_GIVE 30
#define AMMO_BUCKSHOT_GIVE 8
#define AMMO_556NATO_GIVE 30
#define AMMO_556NATOBOX_GIVE 30
#define AMMO_762NATO_GIVE 30
#define AMMO_45ACP_GIVE 12
#define AMMO_50AE_GIVE 7
#define AMMO_338MAGNUM_GIVE 10
#define AMMO_57MM_GIVE 50
#define AMMO_357SIG_GIVE 13

#define _9MM_MAX_CARRY MAX_AMMO_9MM
#define BUCKSHOT_MAX_CARRY MAX_AMMO_BUCKSHOT
#define _556NATO_MAX_CARRY MAX_AMMO_556NATO
#define _556NATOBOX_MAX_CARRY MAX_AMMO_556NATOBOX
#define _762NATO_MAX_CARRY MAX_AMMO_762NATO
#define _45ACP_MAX_CARRY MAX_AMMO_45ACP
#define _50AE_MAX_CARRY MAX_AMMO_50AE
#define _338MAGNUM_MAX_CARRY MAX_AMMO_338MAGNUM
#define _57MM_MAX_CARRY MAX_AMMO_57MM
#define _357SIG_MAX_CARRY MAX_AMMO_357SIG

#define HEGRENADE_MAX_CARRY 1
#define FLASHBANG_MAX_CARRY 2
#define SMOKEGRENADE_MAX_CARRY 1
#define C4_MAX_CARRY 1

enum AmmoCostType
{
	AMMO_338MAG_PRICE = 125,
	AMMO_357SIG_PRICE = 50,
	AMMO_45ACP_PRICE = 25,
	AMMO_50AE_PRICE = 40,
	AMMO_556NATO_PRICE = 60,
	AMMO_57MM_PRICE = 50,
	AMMO_762NATO_PRICE = 80,
	AMMO_9MM_PRICE = 20,
	AMMO_BUCKSHOT_PRICE = 65
};

enum WeaponCostType
{
	AK47_PRICE = 2500,
	AWP_PRICE = 4750,
	DEAGLE_PRICE = 650,
	G3SG1_PRICE = 5000,
	SG550_PRICE = 4200,
	GLOCK18_PRICE = 400,
	M249_PRICE = 5750,
	M3_PRICE = 1700,
	M4A1_PRICE = 3100,
	AUG_PRICE = 3500,
	MP5NAVY_PRICE = 1500,
	P228_PRICE = 600,
	P90_PRICE = 2350,
	UMP45_PRICE = 1700,
	MAC10_PRICE = 1400,
	SCOUT_PRICE = 2750,
	SG552_PRICE = 3500,
	TMP_PRICE = 1250,
	USP_PRICE = 500,
	ELITE_PRICE = 800,
	FIVESEVEN_PRICE = 750,
	XM1014_PRICE = 3000,
	GALIL_PRICE = 2000,
	FAMAS_PRICE = 2250,
	SHIELDGUN_PRICE = 2200
};

enum ItemCostType
{
	ASSAULTSUIT_PRICE = 1000,
	FLASHBANG_PRICE = 200,
	HEGRENADE_PRICE = 300,
	SMOKEGRENADE_PRICE = 300,
	KEVLAR_PRICE = 650,
	HELMET_PRICE = 350,
	NVG_PRICE = 1250,
	DEFUSEKIT_PRICE = 200
};

enum ClipSizeType
{
	P228_MAX_CLIP = 13,
	GLOCK18_MAX_CLIP = 20,
	SCOUT_MAX_CLIP = 10,
	XM1014_MAX_CLIP = 7,
	MAC10_MAX_CLIP = 30,
	AUG_MAX_CLIP = 30,
	ELITE_MAX_CLIP = 30,
	FIVESEVEN_MAX_CLIP = 20,
	UMP45_MAX_CLIP = 25,
	SG550_MAX_CLIP = 30,
	GALIL_MAX_CLIP = 35,
	FAMAS_MAX_CLIP = 25,
	USP_MAX_CLIP = 12,
	AWP_MAX_CLIP = 10,
	MP5N_MAX_CLIP = 30,
	M249_MAX_CLIP = 100,
	M3_MAX_CLIP = 8,
	M4A1_MAX_CLIP = 30,
	TMP_MAX_CLIP = 30,
	G3SG1_MAX_CLIP = 20,
	DEAGLE_MAX_CLIP = 7,
	SG552_MAX_CLIP = 30,
	AK47_MAX_CLIP = 30,
	P90_MAX_CLIP = 50
};

enum MaxAmmoType
{
	MAX_AMMO_BUCKSHOT = 32,
	MAX_AMMO_9MM = 120,
	MAX_AMMO_556NATO = 90,
	MAX_AMMO_556NATOBOX = 200,
	MAX_AMMO_762NATO = 90,
	MAX_AMMO_45ACP = 100,
	MAX_AMMO_50AE = 35,
	MAX_AMMO_338MAGNUM = 30,
	MAX_AMMO_57MM = 100,
	MAX_AMMO_357SIG = 52
};

enum AmmoType
{
	AMMO_BUCKSHOT,
	AMMO_9MM,
	AMMO_556NATO,
	AMMO_556NATOBOX,
	AMMO_762NATO,
	AMMO_45ACP,
	AMMO_50AE,
	AMMO_338MAGNUM,
	AMMO_57MM,
	AMMO_357SIG,
	AMMO_MAX_TYPES
};


typedef struct
{
	int iSlot;
	int iPosition;
	const char *pszAmmo1;
	int iMaxAmmo1;
	const char *pszAmmo2;
	int iMaxAmmo2;
	const char *pszName;
	int iMaxClip;
	int iId;
	int iFlags;
	int iWeight;
}
ItemInfo;

typedef struct
{
	const char *pszName;
	int iId;
}
AmmoInfo;

typedef enum
{
	USE_OFF,
	USE_ON,
	USE_SET,
	USE_TOGGLE
}
USE_TYPE;

class CBaseEntity 
{
public:
    virtual void padd(void);
public:
	entvars_t *pev;
	CBaseEntity *m_pGoalEnt;
	CBaseEntity *m_pLink;
	void (CBaseEntity::*m_pfnThink)(void);
	void (CBaseEntity::*m_pfnTouch)(CBaseEntity *pOther);
	void (CBaseEntity::*m_pfnUse)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void (CBaseEntity::*m_pfnBlocked)(CBaseEntity *pOther);
	int maxammo_buckshot;
	int ammo_buckshot;
	int maxammo_9mm;
	int ammo_9mm;
	int maxammo_556nato;
	int ammo_556nato;
	int maxammo_556natobox;
	int ammo_556natobox;
	int maxammo_762nato;
	int ammo_762nato;
	int maxammo_45acp;
	int ammo_45acp;
	int maxammo_50ae;
	int ammo_50ae;
	int maxammo_338mag;
	int ammo_338mag;
	int maxammo_57mm;
	int ammo_57mm;
	int maxammo_357sig;
	int ammo_357sig;
	float m_flStartThrow;
	float m_flReleaseThrow;
};

class CBaseDelay : public CBaseEntity
{
public:
	float		m_flDelay;
	int			m_iszKillTarget;
};

class CBaseAnimating : public CBaseDelay
{
public:
	// animation needs
	float				m_flFrameRate;		// computed FPS for current sequence
	float				m_flGroundSpeed;	// computed linear movement rate for current sequence
	float				m_flLastEventCheck;	// last time the event list was checked
	BOOL				m_fSequenceFinished;// flag set when StudioAdvanceFrame moves across a frame boundry
	BOOL				m_fSequenceLoops;	// true if the sequence loops
};

class CBasePlayerItem : public CBaseAnimating
{
    public:	
        void	*m_pPlayer;
        CBasePlayerItem *m_pNext;
        int		m_iId;
};

class CBasePlayerWeapon : public CBasePlayerItem
{
    public:
        int m_iPlayEmptySound;
        int m_fFireOnEmpty;
        float m_flNextPrimaryAttack;
        float m_flNextSecondaryAttack;
        float m_flTimeWeaponIdle;
        int m_iPrimaryAmmoType;
        int m_iSecondaryAmmoType;
        int m_iClip;
        int m_iClientClip;
        int m_iClientWeaponState;
        int m_fInReload;
        int m_fInSpecialReload;
        int m_iDefaultAmmo;
        int m_iShellId;
        int m_fMaxSpeed;
        bool m_bDelayFire;
        int m_iDirection;
        bool m_bSecondarySilencerOn;
        float m_flAccuracy;
        float m_flLastFire;
        int m_iShotsFired;
        vec3_s m_vVecAiming;
        int model_name;
        float m_flGlock18Shoot;
        int m_iGlock18ShotsFired;
        float m_flFamasShoot;
        int m_iFamasShotsFired;
        float m_fBurstSpread;
        int m_iWeaponState;
        float m_flNextReload;
        float m_flDecreaseShotsFired;
        unsigned short m_usFireGlock18;
        unsigned short m_usFireFamas;	
};

enum eWeaponBit : uint8_t
{
	WEAPONBIT_NONE = 0,
	WEAPONBIT_USPSILENCER = 1,
	WEAPONBIT_GLOCKBURST = 2,
	WEAPONBIT_M4A1SILENCER = 4,
	WEAPONBIT_ELITERIGHT = 8,
	WEAPONBIT_FAMASBURST = 16
};

typedef struct
{
	char _0x0000[172];
	int WeaponID; //0x00AC 
	char _0x00B0[8];
	float NextAttack; //0x00B8 
	char _0x00BC[16];
	int Ammo; //0x00CC 
	char _0x00D0[8];
	int Reloading; //0x00D8 
	char _0x00DC[4];
	int MaxAmmo; //0x00E0 
	char _0x00E4[20];
	float SpreadVar; //0x00F8 
	float PredRecoil; //0x00FC 
	int Recoil; //0x0100 
	char _0x0104[16];
	float PistolBurstSpread; //0x0114 
	char _0x0118[4];
	float BurstSpread; //0x011C 
	char _0x0120[8];
	eWeaponBit WeaponBit; //0x0128 
	char _0x0129[31];
}weapon_t; //0x12C