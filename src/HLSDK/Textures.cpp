#include "Textures.hpp"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <memory>

#include <Windows.h>

int pm_gcTextures = 0;
char pm_grgszTextureName[1024][17];
char pm_grgchTextureType[1024];

void PM_SwapTextures(int i, int j)
{
	char chTemp;
	char szTemp[CBTEXTURENAMEMAX];

	std::strcpy(szTemp, pm_grgszTextureName[i]);
	chTemp = pm_grgchTextureType[i];

	std::strcpy(pm_grgszTextureName[i], pm_grgszTextureName[j]);
	pm_grgchTextureType[i] = pm_grgchTextureType[j];

	std::strcpy(pm_grgszTextureName[j], szTemp);
	pm_grgchTextureType[j] = chTemp;
}

void PM_SortTextures()
{
	// Bubble sort, yuck, but this only occurs at startup and it's only 512 elements...
	int i, j;

	for (i = 0; i < pm_gcTextures; ++i)
	{
		for (j = i + 1; j < pm_gcTextures; j++)
		{
			if (_stricmp(pm_grgszTextureName[i], pm_grgszTextureName[j]) > 0)
			{
				// Swap
				PM_SwapTextures(i, j);
			}
		}
	}
}

char *memfgets( byte *pMemFile, int fileSize, int &filePos, char *pBuffer, int bufferSize )
{
	// Bullet-proofing
	if ( !pMemFile || !pBuffer )
		return NULL;

	if ( filePos >= fileSize )
		return NULL;

	int i = filePos;
	int last = fileSize;

	// fgets always NULL terminates, so only read bufferSize-1 characters
	if ( last - filePos > (bufferSize-1) )
		last = filePos + (bufferSize-1);

	int stop = 0;

	// Stop at the next newline (inclusive) or end of buffer
	while ( i < last && !stop )
	{
		if ( pMemFile[i] == '\n' )
			stop = 1;
		i++;
	}


	// If we actually advanced the pointer, copy it over
	if ( i != filePos )
	{
		// We read in size bytes
		int size = i - filePos;
		// copy it out
		memcpy( pBuffer, pMemFile + filePos, sizeof(byte)*size );
		
		// If the buffer isn't full, terminate (this is always true)
		if ( size < bufferSize )
			pBuffer[size] = 0;

		// Update file pointer
		filePos = i;
		return pBuffer;
	}

	// No data read, bail
	return NULL;
}

void PM_InitTextureTypes(playermove_t* pmove)
{
    static int bTextureTypeInit = false;
    if (bTextureTypeInit)
		return;

    bTextureTypeInit = true;

	char buffer[512];
	int i, j;
	uint8_t *pMemFile;
	int fileSize, filePos;    

	std::memset(&(pm_grgszTextureName[0][0]), 0, sizeof(pm_grgszTextureName));
	std::memset(pm_grgchTextureType, 0, sizeof(pm_grgchTextureType));

	pm_gcTextures = 0;
	std::memset(buffer, 0, sizeof(buffer));

	fileSize = pmove->COM_FileSize((char*)"sound/materials.txt");
	pMemFile = pmove->COM_LoadFile((char*)"sound/materials.txt", 5, NULL);

	if (!pMemFile)
		return;

	filePos = 0;

	// for each line in the file...
	while (memfgets(pMemFile, fileSize, filePos, buffer, sizeof(buffer) - 1) != NULL && (pm_gcTextures < CTEXTURESMAX))
	{
		// skip whitespace
		i = 0;
		while (buffer[i] && isspace(buffer[i]))
			++i;

		if (!buffer[i])
			continue;

		// skip comment lines
		if (buffer[i] == '/' || !isalpha(buffer[i]))
			continue;

		// get texture type
		pm_grgchTextureType[pm_gcTextures] = toupper(buffer[i++]);

		// skip whitespace
		while (buffer[i] && isspace(buffer[i]))
			++i;

		if (!buffer[i])
			continue;

		// get sentence name
		j = i;
		while (buffer[j] && !isspace(buffer[j]))
			j++;

		if (!buffer[j])
			continue;

		// null-terminate name and save in sentences array
		j = std::min(j, CBTEXTURENAMEMAX - 1 + i);
		buffer[j] = '\0';

		std::strcpy(&(pm_grgszTextureName[pm_gcTextures++][0]), &(buffer[i]));
	}

	// Must use engine to free since we are in a .dll
	pmove->COM_FreeFile(pMemFile);

	PM_SortTextures();
	bTextureTypeInit = true;
}

char PM_FindTextureType(char *name)
{
	int left, right, pivot;
	int val;

	//assert(pm_shared_initialized);

	left = 0;
	right = pm_gcTextures - 1;

	while (left <= right)
	{
		pivot = (left + right) / 2;

		val = _strnicmp(name, pm_grgszTextureName[pivot], CBTEXTURENAMEMAX - 1);

		if (val == 0)
		{
			return pm_grgchTextureType[pivot];
		}
		else if (val > 0)
		{
			left = pivot + 1;
		}
		else if (val < 0)
		{
			right = pivot - 1;
		}
	}

	return CHAR_TEX_CONCRETE;
}