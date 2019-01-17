#pragma once
#include "Vec3.hpp"
#include "Entity.hpp"
#include "StudioStructures.hpp"

// TODO: Own cvar file
typedef struct cvar_s
{
	char	*name;
	char	*string;
	int		flags;
	float	value;
	struct cvar_s *next;
} cvar_t;

typedef struct engine_studio_api_s
{
	// Allocate number*size bytes and zero it
	void			*( *Mem_Calloc )				( int number, size_t size );
	// Check to see if pointer is in the cache
	void			*( *Cache_Check )				( struct cache_user_s *c );
	// Load file into cache ( can be swapped out on demand )
	void			( *LoadCacheFile )				( char *path, struct cache_user_s *cu );
	// Retrieve model pointer for the named model
	struct model_s	*( *Mod_ForName )				( const char *name, int crash_if_missing );
	// Retrieve pointer to studio model data block from a model
	studiohdr_t		*( *Mod_Extradata ) 			( struct model_s *mod );
	// Retrieve indexed model from client side model precache list
	struct model_s	*( *GetModelByIndex )			( int index );
	// Get entity that is set for rendering
	struct cl_entity_s * ( *GetCurrentEntity )		( void );
	// Get referenced player_info_t
	struct player_info_s *( *PlayerInfo )			( int index );
	// Get most recently received player state data from network system
	struct entity_state_s *( *GetPlayerState )		( int index );
	// Get viewentity
	struct cl_entity_s * ( *GetViewEntity )			( void );
	// Get current frame count, and last two timestampes on client
	void			( *GetTimes )					( int *framecount, double *current, double *old );
	// Get a pointer to a cvar by name
	struct cvar_s	*( *GetCvar )					( const char *name );
	// Get current render origin and view vectors ( up, right and vpn )
	void			( *GetViewInfo )				( float *origin, float *upv, float *rightv, float *vpnv );
	// Get sprite model used for applying chrome effect
	struct model_s	*( *GetChromeSprite )			( void );
	// Get model counters so we can incement instrumentation
	void			( *GetModelCounters )			( int **s, int **a );
	// Get software scaling coefficients
	void			( *GetAliasScale )				( float *x, float *y );

	// Get bone, light, alias, and rotation matrices
	float			****( *StudioGetBoneTransform ) ( void );
	float			****( *StudioGetLightTransform )( void );
	float			***( *StudioGetAliasTransform ) ( void );
	float			***( *StudioGetRotationMatrix ) ( void );

	// Set up body part, and get submodel pointers
	void			( *StudioSetupModel )			( int bodypart, void **ppbodypart, void **ppsubmodel );
	// Check if entity's bbox is in the view frustum
	int				( *StudioCheckBBox )			( void );
	// Apply lighting effects to model
	void			( *StudioDynamicLight )			( struct cl_entity_s *ent, struct alight_s *plight );
	void			( *StudioEntityLight )			( struct alight_s *plight );
	void			( *StudioSetupLighting )		( struct alight_s *plighting );

	// Draw mesh vertices
	void			( *StudioDrawPoints )			( void );

	// Draw hulls around bones
	void			( *StudioDrawHulls )			( void );
	// Draw bbox around studio models
	void			( *StudioDrawAbsBBox )			( void );
	// Draws bones
	void			( *StudioDrawBones )			( void );
	// Loads in appropriate texture for model
	void			( *StudioSetupSkin )			( void *ptexturehdr, int index );
	// Sets up for remapped colors
	void			( *StudioSetRemapColors )		( int top, int bottom );
	// Set's player model and returns model pointer
	struct model_s	*( *SetupPlayerModel )			( int index );
	// Fires any events embedded in animation
	void			( *StudioClientEvents )			( void );
	// Retrieve/set forced render effects flags
	int				( *GetForceFaceFlags )			( void );
	void			( *SetForceFaceFlags )			( int flags );
	// Tell engine the value of the studio model header
	void			( *StudioSetHeader )			( void *header );
	// Tell engine which model_t * is being renderered
	void			( *SetRenderModel )				( struct model_s *model );

	// Final state setup and restore for rendering
	void			( *SetupRenderer )				( int rendermode );
	void			( *RestoreRenderer )			( void );

	// Set render origin for applying chrome effect
	void			( *SetChromeOrigin )			( void );

	// True if using D3D/OpenGL
	int				( *IsHardware )					( void );
	
	// Only called by hardware interface
	void			( *GL_StudioDrawShadow )		( void );
	void			( *GL_SetRenderMode )			( int mode );
} engine_studio_api_t;

#define MAXSTUDIOBONES 128
class CStudioModelRenderer
{
public:
	// Construction/Destruction
	CStudioModelRenderer( void );
	virtual ~CStudioModelRenderer( void );

	// Initialization
	virtual void Init( void );

public:  
	// Public Interfaces
	virtual int StudioDrawModel ( int flags );
	virtual int StudioDrawPlayer ( int flags, struct entity_state_s *pplayer );

public:
	// Local interfaces
	//

	// Look up animation data for sequence
	virtual mstudioanim_t *StudioGetAnim ( model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc );

	// Interpolate model position and angles and set up matrices
	virtual void StudioSetUpTransform (int trivial_accept);

	// Set up model bone positions
	virtual void StudioSetupBones ( void );	

	// Find final attachment points
	virtual void StudioCalcAttachments ( void );
	
	// Save bone matrices and names
	virtual void StudioSaveBones( void );

	// Merge cached bones with current bones for model
	virtual void StudioMergeBones ( model_t *m_pSubModel );

	// Determine interpolation fraction
	virtual float StudioEstimateInterpolant( void );

	// Determine current frame for rendering
	virtual float StudioEstimateFrame ( mstudioseqdesc_t *pseqdesc );

	// Apply special effects to transform matrix
	virtual void StudioFxTransform( cl_entity_t *ent, float transform[3][4] );

	// Spherical interpolation of bones
	virtual void StudioSlerpBones ( vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s );

	// Compute bone adjustments ( bone controllers )
	virtual void StudioCalcBoneAdj ( float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen );

	// Get bone quaternions
	virtual void StudioCalcBoneQuaterion ( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q );

	// Get bone positions
	virtual void StudioCalcBonePosition ( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos );

	// Compute rotations
	virtual void StudioCalcRotations ( float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f );

	// Send bones and verts to renderer
	virtual void StudioRenderModel ( void );

	// Finalize rendering
	virtual void StudioRenderFinal (void);
	
	// GL&D3D vs. Software renderer finishing functions
	virtual void StudioRenderFinal_Software ( void );
	virtual void StudioRenderFinal_Hardware ( void );

	// Player specific data
	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend ( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch );

	// Estimate gait frame for player
	virtual void StudioEstimateGait ( entity_state_t *pplayer );

	// Process movement of player
	virtual void StudioProcessGait ( entity_state_t *pplayer );

public:

	// Client clock
	double			m_clTime;				
	// Old Client clock
	double			m_clOldTime;			

	// Do interpolation?
	int				m_fDoInterp;			
	// Do gait estimation?
	int				m_fGaitEstimation;		

	// Current render frame #
	int				m_nFrameCount;

	// Cvars that studio model code needs to reference
	//
	// Use high quality models?
	cvar_t			*m_pCvarHiModels;	
	// Developer debug output desired?
	cvar_t			*m_pCvarDeveloper;
	// Draw entities bone hit boxes, etc?
	cvar_t			*m_pCvarDrawEntities;

	// The entity which we are currently rendering.
	cl_entity_t		*m_pCurrentEntity;		

	// The model for the entity being rendered
	model_t			*m_pRenderModel;

	// Player info for current player, if drawing a player
	player_info_t	*m_pPlayerInfo;

	// The index of the player being drawn
	int				m_nPlayerIndex;

	// The player's gait movement
	float			m_flGaitMovement;

	// Pointer to header block for studio model data
	studiohdr_t		*m_pStudioHeader;
	
	// Pointers to current body part and submodel
	mstudiobodyparts_t *m_pBodyPart;
	mstudiomodel_t	*m_pSubModel;

	// Palette substition for top and bottom of model
	int				m_nTopColor;			
	int				m_nBottomColor;

	//
	// Sprite model used for drawing studio model chrome
	model_t			*m_pChromeSprite;

	// Caching
	// Number of bones in bone cache
	int				m_nCachedBones; 
	// Names of cached bones
	char			m_nCachedBoneNames[ MAXSTUDIOBONES ][ 32 ];
	// Cached bone & light transformation matrices
	float			m_rgCachedBoneTransform [ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float			m_rgCachedLightTransform[ MAXSTUDIOBONES ][ 3 ][ 4 ];

	// Software renderer scale factors
	float			m_fSoftwareXScale, m_fSoftwareYScale;

	// Current view vectors and render origin
	float			m_vUp[ 3 ];
	float			m_vRight[ 3 ];
	float			m_vNormal[ 3 ];

	float			m_vRenderOrigin[ 3 ];
	
	// Model render counters ( from engine )
	int				*m_pStudioModelCount;
	int				*m_pModelsDrawn;

	// Matrices
	// Model to world transformation
	float			(*m_protationmatrix)[ 3 ][ 4 ];	
	// Model to view transformation
	float			(*m_paliastransform)[ 3 ][ 4 ];	

	// Concatenated bone and light transforms
	float			(*m_pbonetransform) [ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float			(*m_plighttransform)[ MAXSTUDIOBONES ][ 3 ][ 4 ];
};