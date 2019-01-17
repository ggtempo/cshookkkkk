#pragma once
#include <Windows.h>
#include <cstdint> 
#include "UserCMD.hpp"

typedef int qboolean;
typedef void(*HUD_CL_CREATEMOVE_FUNC)(float frametime, usercmd_t *cmd, int active);
typedef void(*HUD_CLIENTMOVE_FUNC)(struct playermove_s *ppmove, qboolean server);

typedef struct
{
	void*	pInitFunc;
	void*	pHudInitFunc;
	void*	pHudVidInitFunc;
	void*	pHudRedrawFunc;
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
	void*	pCalcRefdef;
	void*	pAddEntity;
	void*	pCreateEntities;
	void*	pDrawNormalTriangles;
	void*	pDrawTransparentTriangles;
	void*	pStudioEvent;
	void*	pPostRunCmd;
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