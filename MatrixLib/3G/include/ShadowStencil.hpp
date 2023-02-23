// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef SHADOW_STENCIL_INCLUDE
#define SHADOW_STENCIL_INCLUDE

#include "VectorObject.hpp"

struct SVOShadowStencilVertex
{
    D3DXVECTOR3 v = { 0.0f, 0.0f, 0.0f };
    static const dword FVF = D3DFVF_XYZ;
};

class CVOShadowStencil : public CMain
{
    dword  m_DirtyDX = 1;
    D3D_VB m_VB = nullptr;
    D3D_IB m_IB = nullptr;

    int    m_IBSize = 0; // in bytes
    int    m_VBSize = 0;

    static CVOShadowStencil* m_First;
    static CVOShadowStencil* m_Last;

    CVOShadowStencil* m_Prev = nullptr;
    CVOShadowStencil* m_Next = nullptr;

    CVectorObject* m_vo = nullptr;

    int m_FrameFor = -1;

    struct SSSFrameData
    {
        SVOShadowStencilVertex* m_preVerts = nullptr;
        int                     m_preVertsAllocated = 0;
        int                     m_preVertsSize = 0;

        word*                   m_preInds = nullptr;
        int                     m_preIndsAllocated = 0;
        int                     m_preIndsSize = 0;

        SVONormal               m_light;
        float                   m_len = 0.0f;
    };

    CHeap* m_Heap = nullptr;

    // calculated for

    SSSFrameData*   m_Frames = nullptr;
    int             m_FramesCnt = 0;

public:
    static void StaticInit(void)
    {
        m_First = nullptr;
        m_Last = nullptr;
    }
    static void BeforeRenderAll(void) { ASSERT_DX(g_D3DD->SetFVF(SVOShadowStencilVertex::FVF)); }

    static void MarkAllBuffersNoNeed(void);

    CVOShadowStencil(CHeap* heap) : CMain(), m_Heap(heap) { LIST_ADD(this, m_First, m_Last, m_Prev, m_Next); }
    ~CVOShadowStencil()
    {
    DTRACE();

        if(m_Frames)
        {
            for(int i = 0; i < m_FramesCnt; ++i)
            {
                if(m_Frames[i].m_preVerts) HFree(m_Frames[i].m_preVerts, m_Heap);
                if(m_Frames[i].m_preInds) HFree(m_Frames[i].m_preInds, m_Heap);
            }
            HFree(m_Frames, m_Heap);
        }

        DX_Free();
        LIST_DEL(this, m_First, m_Last, m_Prev, m_Next);
    }

    bool IsReady(void) const { return m_Frames != nullptr; }
    void DX_Prepare(void);
    void DX_Free(void)
    {
        if(IS_VB(m_VB)) DESTROY_VB(m_VB);
        if(IS_VB(m_IB)) DESTROY_VB(m_IB);

        m_DirtyDX = true;
    }

    void CreateShadowVO(CVectorObject& obj, int frame, const D3DXVECTOR3& vLight, float length, bool obj_horizontal_inversion);

    void BeforeRender(void) { DX_Prepare(); }
	void Render(const D3DXMATRIX& obj_map);
};

#endif