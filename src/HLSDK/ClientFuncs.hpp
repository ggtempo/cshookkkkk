#pragma once
#include <Windows.h>
#include <cstdint> 
#include "UserCMD.hpp"

#define MAX_PHYSINFO_STRING 256

typedef struct ref_params_s
{
	// Output
	vec3_s	vieworg;
	vec3_s	viewangles;

	vec3_s	forward;
	vec3_s	right;
	vec3_s   up;

	// Client frametime;
	float	frametime;
	// Client time
	float	time;

	// Misc
	int		intermission;
	int		paused;
	int		spectator;
	int		onground;
	int		waterlevel;

	vec3_s	simvel;
	vec3_s	simorg;

	vec3_s	viewheight;
	float	idealpitch;

	vec3_s	cl_viewangles;

	int		health;
	vec3_s	crosshairangle;
	float	viewsize;

	vec3_s	punchangle;
	int		maxclients;
	int		viewentity;
	int		playernum;
	int		max_entities;
	int		demoplayback;
	int		hardware;

	int		smoothing;

	// Last issued usercmd
	struct usercmd_s *cmd;

	// Movevars
	struct movevars_s *movevars;

	int		viewport[4];		// the viewport coordinates x ,y , width, height

	int		nextView;			// the renderer calls ClientDLL_CalcRefdef() and Renderview
								// so long in cycles until this value is 0 (multiple views)
	int		onlyClientDraw;		// if !=0 nothing is drawn by the engine except clientDraw functions
} ref_params_t;

typedef struct clientdata_s
{
	vec3_t				origin;
	vec3_t				velocity;

	int					viewmodel;
	vec3_t				punchangle;
	int					flags;
	int					waterlevel;
	int					watertype;
	vec3_t				view_ofs;
	float				health;

	int					bInDuck;

	int					weapons; // remove?
	
	int					flTimeStepSound;
	int					flDuckTime;
	int					flSwimTime;
	int					waterjumptime;

	float				maxspeed;

	float				fov;
	int					weaponanim;

	int					m_iId;
	int					ammo_shells;
	int					ammo_nails;
	int					ammo_cells;
	int					ammo_rockets;
	float				m_flNextAttack;
	
	int					tfstate;

	int					pushmsec;

	int					deadflag;

	char				physinfo[ MAX_PHYSINFO_STRING ];

	// For mods
	int					iuser1;
	int					iuser2;
	int					iuser3;
	int					iuser4;
	float				fuser1;
	float				fuser2;
	float				fuser3;
	float				fuser4;
	vec3_t				vuser1;
	vec3_t				vuser2;
	vec3_t				vuser3;
	vec3_t				vuser4;
} clientdata_t;

typedef struct weapon_data_s
{
	int			m_iId;
	int			m_iClip;

	float		m_flNextPrimaryAttack;
	float		m_flNextSecondaryAttack;
	float		m_flTimeWeaponIdle;

	int			m_fInReload;
	int			m_fInSpecialReload;
	float		m_flNextReload;
	float		m_flPumpTime;
	float		m_fReloadTime;

	float		m_fAimedDamage;
	float		m_fNextAimBonus;
	int			m_fInZoom;
	int			m_iWeaponState;

	int			iuser1;
	int			iuser2;
	int			iuser3;
	int			iuser4;
	float		fuser1;
	float		fuser2;
	float		fuser3;
	float		fuser4;
} weapon_data_t;

typedef struct local_state_s
{
	entity_state_t playerstate;
	clientdata_t   client;
	weapon_data_t  weapondata[ 64 ];
} local_state_t;

typedef int qboolean;
typedef void(*HUD_CL_CREATEMOVE_FUNC)(float frametime, usercmd_t *cmd, int active);
typedef void(*HUD_CLIENTMOVE_FUNC)(struct playermove_s *ppmove, qboolean server);
typedef void(*V_CALCREFDEF)(ref_params_s *pparams);
typedef void(*HUD_POSTRUNCMD)( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
typedef int(*HUD_REDRAW)(float, int);

typedef struct
{
	void*	pInitFunc;
	void*	pHudInitFunc;
	void*	pHudVidInitFunc;
	HUD_REDRAW	pHudRedrawFunc;
	void*	pHudUpdateClientDataFunc;
	void*	pHudResetFunc;
	HUD_CLIENTMOVE_FUNC	pClientMove;
	void*	pClientMoveInit;
	void*	pClientTextureType;
	void*	pIN_ActivateMouse;
	void*	pIN_DeactivateMouse;
	void*	pIN_MouseEvent;
	void*	pIN_ClearStates;
	void*	pIN_Accumulate;
	HUD_CL_CREATEMOVE_FUNC	pCL_CreateMove;
	void*	pCL_IsThirdPerson;
	void*	pCL_GetCameraOffsets;
	void*	pFindKey;
	void*	pCamThink;
	V_CALCREFDEF pCalcRefdef;
	void*	pAddEntity;
	void*	pCreateEntities;
	void*	pDrawNormalTriangles;
	void*	pDrawTransparentTriangles;
	void*	pStudioEvent;
	HUD_POSTRUNCMD	pPostRunCmd;
	void*	pShutdown;
	void*	pTxferLocalOverrides;
	void*	pProcessPlayerState;
	void*	pTxferPredictionData;
	void*	pReadDemoBuffer;
	void*	pConnectionlessPacket;
	void*	pGetHullBounds;
	void*	pHudFrame;
	void*	pKeyEvent;
	void*	pTempEntUpdate;
	void*	pGetUserEntity;
	void*	pVoiceStatus;		// Possibly null on old client dlls.
	void*	pDirectorMessage;	// Possibly null on old client dlls.
	void*	pStudioInterface;	// Not used by all clients
	void*	pChatInputPosition;	// Not used by all clients
	void*	pGetPlayerTeam; // Not used by all clients
	void*	pClientFactory;
} cldll_func_t;