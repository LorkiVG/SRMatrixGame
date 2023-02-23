// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef _DEVICE_STATE_INCLUDE
#define _DEVICE_STATE_INCLUDE

#define DEVSTATE_ALL                (-1)
#define DEVSTATE_RENDERSTATES       SETBIT(0)
#define DEVSTATE_SAMPLERSTATES      SETBIT(1)
#define DEVSTATE_LIGHTS             SETBIT(2)
#define DEVSTATE_MATERIAL           SETBIT(3)

#define RS_COUNT        103
#define SS_COUNT        13
#define LI_COUNT        8

class CDeviceState : public CMain
{
    IDirect3DDevice9* m_Device = nullptr;

    dword             m_RenderStates[RS_COUNT] = { 0 };
    dword             m_SamplerStates[SS_COUNT * 8] = { 0 };
    BOOL /*говнобул, ёпта! майки int как bool юзают, черти!*/ m_LightsEnabled[LI_COUNT] = { false };
    D3DLIGHT9         m_Lights[LI_COUNT];
    D3DMATERIAL9      m_Material;

public:
    CDeviceState(IDirect3DDevice9* dev) : m_Device(dev)
    {
        ZeroMemory(&m_Lights, sizeof(m_Lights));
        ZeroMemory(&m_Material, sizeof(m_Material));
    }

    void StoreState(dword states = DEVSTATE_ALL);
    void RestoreState(dword states = DEVSTATE_ALL);
};

#endif