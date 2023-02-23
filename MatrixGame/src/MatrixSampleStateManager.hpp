// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_SAMPLER_INCLUDE
#define MATRIX_SAMPLER_INCLUDE

struct SRobotsSettings;

class CMatrixSampleStateManager
{
public:
	int m_AFDegree;

	void ApplySettings(SRobotsSettings* set);
	HRESULT SetState(dword sampler, D3DSAMPLERSTATETYPE type, dword value);

	CMatrixSampleStateManager(void);
	~CMatrixSampleStateManager(void);

private:
	bool EligibleForAnisotropy( D3DSAMPLERSTATETYPE type );
};

extern CMatrixSampleStateManager g_Sampler;

#endif
