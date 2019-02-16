#pragma once
#include <cstdint>
#include "Vec3.hpp"
#include "UserCMD.hpp"

typedef int qboolean;

#define	MAX_PHYSENTS 600 		  // Must have room for all entities in the world.
#define MAX_MOVEENTS 64
#define	MAX_CLIP_PLANES	5
#define MAX_PHYSINFO_STRING 256

#define PM_NORMAL			0x00000000
#define PM_STUDIO_IGNORE	0x00000001		// Skip studio models
#define PM_STUDIO_BOX		0x00000002		// Use boxes for non-complex studio models (even in traceline)
#define PM_GLASS_IGNORE		0x00000004		// Ignore entities with non-normal rendermode
#define PM_WORLD_ONLY		0x00000008		// Only trace against the world

// Values for flags parameter of PM_TraceLine
#define PM_TRACELINE_PHYSENTSONLY	0
#define PM_TRACELINE_ANYVISIBLE		1

typedef struct physent_s
{
	char			name[32];             // Name of model, or "player" or "world".
	int				player;
	vec3_t			origin;               // Model's origin in world coordinates.
	struct model_s	*model;		          // only for bsp models
	struct model_s	*studiomodel;         // SOLID_BBOX, but studio clip intersections.
	vec3_t			mins, maxs;	          // only for non-bsp models
	int				info;		          // For client or server to use to identify (index into edicts or cl_entities)
	vec3_t			angles;               // rotated entities need this info for hull testing to work.

	int				solid;				  // Triggers and func_door type WATER brushes are SOLID_NOT
	int				skin;                 // BSP Contents for such things like fun_door water brushes.
	int				rendermode;			  // So we can ignore glass

	// Complex collision detection.
	float			frame;
	int				sequence;
	uint8_t			controller[4];
	uint8_t			blending[2];

	int				movetype;
	int				takedamage;
	int				blooddecal;
	int				team;
	int				classnumber;

	// For mods
	int				iuser1;
	int				iuser2;
	int				iuser3;
	int				iuser4;
	float			fuser1;
	float			fuser2;
	float			fuser3;
	float			fuser4;
	vec3_t			vuser1;
	vec3_t			vuser2;
	vec3_t			vuser3;
	vec3_t			vuser4;
} physent_t;

typedef struct
{
	vec3_t	normal;
	float	dist;
} pmplane_t;

typedef struct pmtrace_s pmtrace_t;

struct pmtrace_s
{
	qboolean	allsolid;	      // if true, plane is not valid
	qboolean	startsolid;	      // if true, the initial point was in a solid area
	qboolean	inopen, inwater;  // End point is in empty space or in water
	float		fraction;		  // time completed, 1.0 = didn't hit anything
	vec3_t		endpos;			  // final position
	pmplane_t	plane;		      // surface normal at impact
	int			ent;			  // entity at impact
	vec3_t      deltavelocity;    // Change in player's velocity caused by impact.  
								  // Only run on server.
	int         hitgroup;
};

#define	MOVETYPE_NONE			0		// never moves
//#define	MOVETYPE_ANGLENOCLIP	1
//#define	MOVETYPE_ANGLECLIP		2
#define	MOVETYPE_WALK			3		// Player only - moving on the ground
#define	MOVETYPE_STEP			4		// gravity, special edge handling -- monsters use this
#define	MOVETYPE_FLY			5		// No gravity, but still collides with stuff
#define	MOVETYPE_TOSS			6		// gravity/collisions
#define	MOVETYPE_PUSH			7		// no clip to world, push and crush
#define	MOVETYPE_NOCLIP			8		// No gravity, no collisions, still do velocity/avelocity
#define	MOVETYPE_FLYMISSILE		9		// extra size to monsters
#define	MOVETYPE_BOUNCE			10		// Just like Toss, but reflect velocity when contacting surfaces
#define MOVETYPE_BOUNCEMISSILE	11		// bounce w/o gravity
#define MOVETYPE_FOLLOW			12		// track movement of aiment
#define	MOVETYPE_PUSHSTEP		13		// BSP model that needs physics/world collisions (uses nearest hull for world collision)

typedef struct movevars_s
{
	float gravity;			// Gravity for map
	float stopspeed;		// Deceleration when not moving
	float maxspeed;			// Max allowed speed
	float spectatormaxspeed;
	float accelerate;		// Acceleration factor
	float airaccelerate;		// Same for when in open air
	float wateraccelerate;		// Same for when in water
	float friction;
	float edgefriction;		// Extra friction near dropofs
	float waterfriction;		// Less in water
	float entgravity;		// 1.0
	float bounce;			// Wall bounce value. 1.0
	float stepsize;			// sv_stepsize;
	float maxvelocity;		// maximum server velocity.
	float zmax;			// Max z-buffer range (for GL)
	float waveHeight;		// Water wave height (for GL)
	qboolean footsteps;		// Play footstep sounds
	char skyName[32];		// Name of the sky map
	float rollangle;
	float rollspeed;
	float skycolor_r;		// Sky color
	float skycolor_g;
	float skycolor_b;
	float skyvec_x;			// Sky vector
	float skyvec_y;
	float skyvec_z;

} movevars_t;

typedef struct playermove_s
{
	int				player_index;  // So we don't try to run the PM_CheckStuck nudging too quickly.
	qboolean		server;        // For debugging, are we running physics code on server side?

	qboolean		multiplayer;   // 1 == multiplayer server
	float			time;          // realtime on host, for reckoning duck timing
	float			frametime;	   // Duration of this frame

	vec3_t			forward, right, up; // Vectors for angles
	// player state
	vec3_t			origin;        // Movement origin.
	vec3_t			angles;        // Movement view angles.
	vec3_t			oldangles;     // Angles before movement view angles were looked at.
	vec3_t			velocity;      // Current movement direction.
	vec3_t			movedir;       // For waterjumping, a forced forward velocity so we can fly over lip of ledge.
	vec3_t			basevelocity;  // Velocity of the conveyor we are standing, e.g.

	// For ducking/dead
	vec3_t			view_ofs;      // Our eye position.
	float			flDuckTime;    // Time we started duck
	qboolean		bInDuck;       // In process of ducking or ducked already?

	// For walking/falling
	int				flTimeStepSound;  // Next time we can play a step sound
	int				iStepLeft;

	float			flFallVelocity;
	vec3_t			punchangle;

	float			flSwimTime;

	float			flNextPrimaryAttack;

	int				effects;		// MUZZLE FLASH, e.g.

	int				flags;         // FL_ONGROUND, FL_DUCKING, etc.
	int				usehull;       // 0 = regular player hull, 1 = ducked player hull, 2 = point hull
	float			gravity;       // Our current gravity and friction.
	float			friction;
	int				oldbuttons;    // Buttons last usercmd
	float			waterjumptime; // Amount of time left in jumping out of water cycle.
	qboolean		dead;          // Are we a dead player?
	int				deadflag;
	int				spectator;     // Should we use spectator physics model?
	int				movetype;      // Our movement type, NOCLIP, WALK, FLY

	int				onground;
	int				waterlevel;
	int				watertype;
	int				oldwaterlevel;

	char			sztexturename[256];
	char			chtexturetype;

	float			maxspeed;
	float			clientmaxspeed; // Player specific maxspeed

	// For mods
	int				iuser1;
	int				iuser2;
	int				iuser3;
	int				iuser4;
	float			fuser1;
	float			fuser2;
	float			fuser3;
	float			fuser4;
	vec3_t			vuser1;
	vec3_t			vuser2;
	vec3_t			vuser3;
	vec3_t			vuser4;
	// world state
	// Number of entities to clip against.
	int				numphysent;
	physent_t		physents[MAX_PHYSENTS];
	// Number of momvement entities (ladders)
	int				nummoveent;
	// just a list of ladders
	physent_t		moveents[MAX_MOVEENTS];

	// All things being rendered, for tracing against things you don't actually collide with
	int				numvisent;
	physent_t		visents[MAX_PHYSENTS];

	// input to run through physics.
	usercmd_t		cmd;

	// Trace results for objects we collided with.
	int				numtouch;
	pmtrace_t		touchindex[MAX_PHYSENTS];

	char			physinfo[MAX_PHYSINFO_STRING]; // Physics info string

	struct movevars_s *movevars;
	vec3_t player_mins[4];
	vec3_t player_maxs[4];

	// Common functions
	const char		*(*PM_Info_ValueForKey) (const char *s, const char *key);
	void(*PM_Particle)(float *origin, int color, float life, int zpos, int zvel);
	int(*PM_TestPlayerPosition) (float *pos, pmtrace_t *ptrace);
	void(*Con_NPrintf)(int idx, char *fmt, ...);
	void(*Con_DPrintf)(char *fmt, ...);
	void(*Con_Printf)(char *fmt, ...);
	double(*Sys_FloatTime)(void);
	void(*PM_StuckTouch)(int hitent, pmtrace_t *ptraceresult);
	int(*PM_PointContents) (float *p, int *truecontents /*filled in if this is non-null*/);
	int(*PM_TruePointContents) (float *p);
	int(*PM_HullPointContents) (struct hull_s *hull, int num, float *p);
	pmtrace_t(*PM_PlayerTrace) (float *start, float *end, int traceFlags, int ignore_pe);
	struct pmtrace_s *(*PM_TraceLine)(float *start, float *end, int flags, int usehulll, int ignore_pe);
	int32_t(*RandomLong)(int32_t lLow, int32_t lHigh);
	float(*RandomFloat)(float flLow, float flHigh);
	int(*PM_GetModelType)(struct model_s *mod);
	void(*PM_GetModelBounds)(struct model_s *mod, float *mins, float *maxs);
	void			*(*PM_HullForBsp)(physent_t *pe, float *offset);
	float(*PM_TraceModel)(physent_t *pEnt, float *start, float *end, /*trace_t*/void* *trace);
	int(*COM_FileSize)(char *filename);
	uint8_t			*(*COM_LoadFile) (char *path, int usehunk, int *pLength);
	void(*COM_FreeFile) (void *buffer);
	char			*(*memfgets)(uint8_t *pMemFile, int fileSize, int *pFilePos, char *pBuffer, int bufferSize);

	// Functions
	// Run functions for this frame?
	qboolean		runfuncs;
	void(*PM_PlaySound) (int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch);
	const char		*(*PM_TraceTexture) (int ground, float *vstart, float *vend);
	void(*PM_PlaybackEventFull) (int flags, int clientindex, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);

	pmtrace_t(*PM_PlayerTraceEx) (float *start, float *end, int traceFlags, int(*pfnIgnore)(physent_t *pe));
	int(*PM_TestPlayerPositionEx) (float *pos, pmtrace_t *ptrace, int(*pfnIgnore)(physent_t *pe));
	struct pmtrace_s *(*PM_TraceLineEx)(float *start, float *end, int flags, int usehulll, int(*pfnIgnore)(physent_t *pe));
} playermove_t;