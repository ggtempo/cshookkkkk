#pragma once
#include "PlayerMove.hpp"

#define CBTEXTURENAMEMAX 17
#define CTEXTURESMAX 1024

#define CHAR_TEX_CONCRETE	'C'	// texture types
#define CHAR_TEX_METAL		'M'
#define CHAR_TEX_DIRT		'D'
#define CHAR_TEX_VENT		'V'
#define CHAR_TEX_GRATE		'G'
#define CHAR_TEX_TILE		'T'
#define CHAR_TEX_SLOSH		'S'
#define CHAR_TEX_WOOD		'W'
#define CHAR_TEX_COMPUTER	'P'
#define CHAR_TEX_GRASS		'X'
#define CHAR_TEX_GLASS		'Y'
#define CHAR_TEX_FLESH		'F'
#define CHAR_TEX_SNOW		'N'

void PM_SwapTextures(int i, int j);
void PM_SortTextures();
void PM_InitTextureTypes(playermove_t* pmove);
char PM_FindTextureType(char *name);