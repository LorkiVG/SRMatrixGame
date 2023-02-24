// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixDebugInfo.hpp"

void CMatrixDebugInfo::Clear()
{
    for(int i = 0; i < m_ItemsCnt; ++i)
    {
        HDelete(CWStr, m_Items[i].key,g_MatrixHeap);
        HDelete(CWStr, m_Items[i].val,g_MatrixHeap);
    }
    m_ItemsCnt = 0;

    ClearFont();
}
void CMatrixDebugInfo::Draw()
{
    if(!m_Font && m_ItemsCnt) InitFont();
#if D3DX_SDK_VERSION >= 21
#else
    m_Font->Begin();
#endif

    try
    {
        int y = m_Pos.y;
        for(int i = 0; i < m_ItemsCnt; ++i)
        {
            if(m_Items[i].bttl > 0) continue;

            CRect r;
            r.left = m_Pos.x;
            r.right = m_Pos.x + DI_KEY_W;
            r.top = y;
            r.bottom = y + DI_KEY_H;

            byte a = 255;
            if(m_Items[i].ttl < 1000) a = (byte)Float2Int(float(m_Items[i].ttl) * 0.255f);

            dword color = (a << 24) | 0xFFFFFF;

            //dword noclip = DT_END_ELLIPSIS;
            //if (m_Items[i].val->GetLen() == 0) noclip = DT_NOCLIP;
#if D3DX_SDK_VERSION >= 21
            m_Font->DrawTextW(nullptr, m_Items[i].key->Get(), m_Items[i].key->GetLen(), &r, DT_NOCLIP|DT_LEFT|DT_VCENTER|DT_SINGLELINE, color);
#else
            m_Font->DrawTextW(m_Items[i].key->Get(), m_Items[i].key->GetLen(), &r, DT_NOCLIP|DT_LEFT|DT_VCENTER|DT_SINGLELINE, color);
#endif

            r.left = r.right + 1;
            r.right = r.left + DI_VAL_W;

#if D3DX_SDK_VERSION >= 21
            m_Font->DrawTextW(nullptr,m_Items[i].val->Get(), m_Items[i].val->GetLen(), &r, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP, color);
#else
            m_Font->DrawTextW(m_Items[i].val->Get(), m_Items[i].val->GetLen(), &r, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP, color);
#endif

            y += DI_KEY_H;
        }
    } catch(...)
    {

    }

#if D3DX_SDK_VERSION >= 21
#else
    m_Font->End();
#endif
}

void CMatrixDebugInfo::T(const wchar *key, const wchar *val, int ttl, int bttl, bool add)
{
    int i = 0;
    if (!add)
    {
        while (i<m_ItemsCnt)
        {
            if (*m_Items[i].key == key)
            {
                m_Items[i].val->Set(val);
                m_Items[i].ttl = ttl;
                m_Items[i].bttl = bttl;
                return;
            }
            ++i;
        }
    }

    if (m_ItemsCnt >= MAX_DEBUG_INFO_ITEMS) return;

    m_Items[m_ItemsCnt].key = HNew(g_MatrixHeap) CWStr(key, g_MatrixHeap);
    m_Items[m_ItemsCnt].val = HNew(g_MatrixHeap) CWStr(val, g_MatrixHeap);
    m_Items[m_ItemsCnt].ttl = ttl;
    m_Items[m_ItemsCnt].bttl = bttl;
    ++m_ItemsCnt;


}

void CMatrixDebugInfo::Tact(int ms)
{
    if (!m_Font && m_ItemsCnt) InitFont();

    int i = 0;
    while(i<m_ItemsCnt)
    {
        if (m_Items[i].bttl > 0)
        {
            m_Items[i].bttl -= ms;
            ++i;
            continue;
        }
        
        m_Items[i].ttl -= ms;
        if (m_Items[i].ttl <= 0)
        {
            --m_ItemsCnt;
            HDelete(CWStr,m_Items[i].key,g_MatrixHeap);
            HDelete(CWStr,m_Items[i].val,g_MatrixHeap);
            if (i < m_ItemsCnt)
            {
                memcpy(m_Items+i,m_Items+i+1,sizeof(SDIItem)*(m_ItemsCnt-i));
            }
            ClearFont();
            continue;
        }
        ++i;
    }
}

void CMatrixDebugInfo::InitFont(void)
{
    ClearFont();

#if D3DX_SDK_VERSION >= 21
    // summer update

    if (D3D_OK != D3DXCreateFontW(g_D3DD,10,0,FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Sans Serif" ,&m_Font))
    {
        m_Font = nullptr;
    }

#else
	HFONT fn=CreateFontA(10,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"MS Sans Serif");
    
    if (D3D_OK != D3DXCreateFont(g_D3DD,fn,&m_Font))
    {
        m_Font = nullptr;
    }
	DeleteObject(fn);
#endif
}