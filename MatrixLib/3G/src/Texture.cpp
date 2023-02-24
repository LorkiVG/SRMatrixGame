// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "Texture.hpp"

CBaseTexture* CBaseTexture::m_TexturesFirst;
CBaseTexture* CBaseTexture::m_TexturesLast;

void CBaseTexture::OnLostDevice(void)
{
    CBaseTexture* tex = m_TexturesFirst;
    for(; tex; tex = tex->m_TexturesNext)
    {
        if(!FLAG(tex->m_Flags, TF_LOST) && tex->m_Type == cc_Texture) ((CTexture*)tex)->OnLostDevice();
    }
}

void CBaseTexture::OnResetDevice(void)
{
    CBaseTexture* tex = m_TexturesFirst;
    for(; tex; tex = tex->m_TexturesNext)
    {
        if(FLAG(tex->m_Flags, TF_LOST) && tex->m_Type == cc_Texture) ((CTexture*)tex)->OnResetDevice();
    }
}

LPDIRECT3DTEXTURE9 CBaseTexture::LoadTextureFromFile(bool to16, D3DPOOL pool)
{
DTRACE();

    LPDIRECT3DTEXTURE9 ret = nullptr;
    ParseFlags(m_Name);
    if(to16)
    {
        if(FLAG(m_Flags, TF_COMPRESSED)) goto autoload;
        CWStr tn(m_Name, g_CacheHeap);
        CFile::FileExist(tn, tn.Get(), CacheExtsTex);
        int idx = tn.FindR('.');
        if(idx < 0) goto autoload;
        tn.LowerCase(idx + 1);
        if(0 != memcmp(tn.Get() + idx + 1, L"png", sizeof(wchar) * 4)) goto autoload;

        CTextureManaged* tex = CACHE_CREATE_TEXTUREMANAGED();
        CBitmap bm(g_CacheHeap);
        bm.LoadFromPNG(tn.Get());
        tex->LoadFromBitmap(bm, true, FLAG(m_Flags, TF_NOMIPMAP) ? 1 : 0);

#ifdef USE_DX_MANAGED_TEXTURES
        ret = tex->m_Tex;
        tex->m_Tex = nullptr;
#else
        ret = tex->m_TexFrom;
        tex->m_TexFrom = nullptr;
#endif
        CCache::Destroy(tex);
        //ASSERT(pool == D3DPOOL_SYSTEMMEM);
        return ret;
    }

autoload:

    CBuf buf(g_CacheHeap);
    if(FLAG(m_Flags, TF_COMPRESSED))
    {
        LoadFromFile(buf, CacheExtsTex);

        CStorage ccc(g_CacheHeap);
        ccc.Load(buf);

        CDataBuf* b = ccc.GetBuf(L"0", L"0", ST_BYTE);
        buf.Clear();
        buf.BufAdd(b->GetFirst<byte>(0), b->GetArrayLength(0));
    }
    else LoadFromFile(buf, CacheExtsTex);

    if(FAILED(D3DXCreateTextureFromFileInMemoryEx(g_D3DD, buf.Get(), buf.Len(), 0, 0, FLAG(m_Flags, TF_NOMIPMAP) ? 1 : 0, 0, D3DFMT_UNKNOWN, pool, D3DX_FILTER_NONE, D3DX_FILTER_BOX, 0, nullptr, nullptr, &ret)))
    {
        return nullptr;
    }
    return ret;
}

void CBaseTexture::ParseFlags(const CWStr& name)
{
    m_Flags = 0;
    CWStr tstr(g_CacheHeap);
    int cnt = name.GetCountPar(L"?");
    if(cnt > 1)
    {
        for(int i = 1; i < cnt; ++i)
        {
            tstr = name.GetStrPar(i, L"?");
            tstr.Trim();
            if(tstr == L"Trans") SETFLAG(m_Flags, TF_ALPHATEST);
            else if(tstr == L"Alpha") SETFLAG(m_Flags, TF_ALPHABLEND);
        }
    }

    tstr = name.GetStrPar(0, L"?");
    CacheReplaceFileExt(tstr, tstr.Get(), L".txt");

    if(CFile::FileExist(tstr, tstr.Get()))
    {
        bool proceed = true;
        if(tstr.Find(L"pinguin.txt") >= 0 || tstr.Find(L"robotarget.txt") >= 0) proceed = false;

        if(proceed)
        {
            CBlockPar texi(1, g_CacheHeap);
            texi.LoadFromTextFile(tstr);

            tstr = texi.ParGetNE(L"AlphaTest");
            if(!tstr.IsEmpty())
            {
                if(tstr == L"0") RESETFLAG(m_Flags, TF_ALPHATEST);
                else SETFLAG(m_Flags, TF_ALPHATEST);
            }
            tstr = texi.ParGetNE(L"AlphaBlend");
            if(!tstr.IsEmpty())
            {
                if(tstr == L"0") RESETFLAG(m_Flags, TF_ALPHABLEND);
                else SETFLAG(m_Flags, TF_ALPHABLEND);
            }
            tstr = texi.ParGetNE(L"Compressed");
            if(!tstr.IsEmpty())
            {
                if(tstr == L"0") RESETFLAG(m_Flags, TF_COMPRESSED);
                else SETFLAG(m_Flags, TF_COMPRESSED);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CTexture::OnLostDevice(void)
{
    // store texture
    if(m_Tex == nullptr) return;

    D3DSURFACE_DESC desc;
    HRESULT res = m_Tex->GetLevelDesc(0, &desc);
    if(res != D3D_OK)
    {
        // oblom konkretny
        SETFLAG(m_Flags, TF_LOST);
        m_Tex->Release();
        m_Tex = nullptr;
        return;
    }

    if(desc.Pool != D3DPOOL_DEFAULT) return; // do not need any actions

    SETFLAG(m_Flags, TF_LOST);

    IDirect3DTexture9* tex;

    int levels = m_Tex->GetLevelCount();

    res = g_D3DD->CreateTexture(desc.Width, desc.Height, levels, 0, desc.Format, D3DPOOL_SYSTEMMEM, &tex, 0);
    if(res != D3D_OK)
    {
        // oblom konkretny
        m_Tex->Release();
        m_Tex = nullptr;
        return;
    }

    m_Usage = desc.Usage;

    for(int i = 0; i < levels; ++i)
    {
        IDirect3DSurface9* from;
        IDirect3DSurface9* to;

        if(D3D_OK != tex->GetSurfaceLevel(i, &to))
        {
            m_Tex->Release();
            m_Tex = nullptr;
            tex->Release();
            return;
        }

        if(D3D_OK != m_Tex->GetSurfaceLevel(i, &from))
        {
            to->Release();
            m_Tex->Release();
            m_Tex = nullptr;
            tex->Release();
            return;
        }

        if(D3D_OK != D3DXLoadSurfaceFromSurface(to, nullptr, nullptr, from, nullptr, nullptr, D3DX_FILTER_NONE, 0))
        {
            from->Release();
            to->Release();
            m_Tex->Release();
            m_Tex = nullptr;
            tex->Release();
            return;
        }

        from->Release();
        to->Release();
    }

    m_Tex->Release();
    m_Tex = tex;
}

void CTexture::OnResetDevice(void)
{
    if(!FLAG(m_Flags, TF_LOST) || m_Tex == nullptr) return; // not lost

    // restore texture

    D3DSURFACE_DESC desc;
    HRESULT res = m_Tex->GetLevelDesc(0, &desc);
    if(res != D3D_OK)
    {
        // oblom konkretny
        SETFLAG(m_Flags, TF_LOST);
        m_Tex->Release();
        m_Tex = nullptr;
        return;
    }

    IDirect3DTexture9* tex;

    int levels = m_Tex->GetLevelCount();

    res = g_D3DD->CreateTexture(desc.Width, desc.Height, levels, m_Usage, desc.Format, D3DPOOL_DEFAULT, &tex, 0);
    if(res != D3D_OK)
    {
        // oblom konkretny
        m_OOM_counter = 50;
        m_Tex->Release();
        m_Tex = nullptr;
        return;
    }

    for(int i = 0; i < levels; ++i)
    {
        IDirect3DSurface9* from;
        IDirect3DSurface9* to;

        if(D3D_OK != tex->GetSurfaceLevel(i, &to))
        {
            m_OOM_counter = 50;
            m_Tex->Release();
            m_Tex = nullptr;
            tex->Release();
            return;
        }

        if(D3D_OK != m_Tex->GetSurfaceLevel(i, &from))
        {
            m_OOM_counter = 50;
            to->Release();
            m_Tex->Release();
            m_Tex = nullptr;
            tex->Release();
            return;
        }

        m_Tex->AddDirtyRect(nullptr);
        if(D3D_OK != g_D3DD->UpdateTexture(m_Tex, tex))
        {
            m_OOM_counter = 50;
            from->Release();
            to->Release();
            m_Tex->Release();
            m_Tex = nullptr;
            tex->Release();
            return;
        }

        from->Release();
        to->Release();
    }

    m_Tex->Release();
    m_Tex = tex;
    m_OOM_counter = 0;

    RESETFLAG(m_Flags, TF_LOST);
}

void CTexture::LoadFromBitmap(CBitmap& bm)
{
DTRACE();

#ifdef _DEBUG
    ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS));
#endif

    Unload();
    CBuf buf(g_CacheHeap);
    bm.SaveInDDSUncompressed(buf);
    FAILED_DX(D3DXCreateTextureFromFileInMemoryEx(g_D3DD, buf.Get(), buf.Len(), 0, 0, FLAG(m_Flags, TF_NOMIPMAP) ? 1 : 0, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_TRIANGLE | D3DX_FILTER_DITHER, D3DX_FILTER_BOX, 0, nullptr, nullptr, &m_Tex));
}

void CTexture::Unload()
{
DTRACE();

    if(m_Tex)
    {
        m_Tex->Release();
        m_Tex = nullptr;
    }
    m_Flags = 0;
}

void CTexture::Load()
{
DTRACE();

    if(FLAG(m_Flags, TF_LOST)) return; // нехуй тут делать. текстура потеряна блять!

    ASSERT(!IsLoaded());
    //Unload();

    if(g_D3DD->GetAvailableTextureMem() < (2 * 1024 * 1024))
    {
        m_OOM_counter = OOM_TEXTRUE_HIT_COUNTER;
        return;
    }

    m_Tex = LoadTextureFromFile(false);
    if(m_Tex == nullptr)
    {
        m_OOM_counter = OOM_TEXTRUE_HIT_COUNTER;
        return;
    }

    D3DSURFACE_DESC desc;
    m_Tex->GetLevelDesc(0, &desc);
    m_Size.x = desc.Width;
    m_Size.y = desc.Height;
}

void CTexture::MakeSolidTexture(int sx, int sy, dword color)
{
#ifdef _DEBUG
    ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS));
#endif

    Unload();
    D3DLOCKED_RECT lr;
    LPDIRECT3DTEXTURE9 sm_tex;
    ASSERT_DX(g_D3DD->CreateTexture(sx, sy, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &sm_tex, 0));
    ASSERT_DX(sm_tex->LockRect(0, &lr, nullptr, 0/*D3DLOCK_DISCARD_TEMP*/));
    int SizeInMemory = lr.Pitch * sy;
    dword* des = (dword*)lr.pBits;
    while(SizeInMemory > 0)
    {
        *des++ = color;
        SizeInMemory -= 4;
    }
    ASSERT_DX(sm_tex->UnlockRect(0));
    ASSERT_DX(g_D3DD->CreateTexture(sx, sy, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_Tex, 0));
    ASSERT_DX(sm_tex->AddDirtyRect(nullptr));
    ASSERT_DX(g_D3DD->UpdateTexture(sm_tex, m_Tex));
    //m_Tex->SetPriority(0xFFFFFFFF);

    sm_tex->Release();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef USE_DX_MANAGED_TEXTURES

void UnloadTextureManaged(dword user)
{
    CTextureManaged* tex = (CTextureManaged*)user;
    tex->Unload();
}
#endif

//#ifdef _DEBUG
//
////int g_TexManagedCnt = 0;
//
//static CDWORDMap* dm = nullptr;
//
//static void dm_in(dword n)
//{
//    if(dm == nullptr)
//    {
//        dm = HNew(g_CacheHeap) CDWORDMap(g_CacheHeap);
//    }
//
//    dword v;
//
//    if(dm->Get(n, &v))
//    {
//        _asm int 3
//    }
//    dm->Set(n,0);
//}
//
//static void dm_out(dword n)
//{
//    if(dm == nullptr)
//    {
//        dm = HNew(g_CacheHeap) CDWORDMap(g_CacheHeap);
//    }
//
//    dword v;
//
//    if(!dm->Get(n, &v))
//    {
//        _asm int 3
//    }
//    dm->Del(n);
//}
//#endif


void CTextureManaged::Init(bool to16)
{
#ifdef USE_DX_MANAGED_TEXTURES
    if(m_Tex) return;
    m_Tex = LoadTextureFromFile(to16, D3DPOOL_MANAGED);

    D3DSURFACE_DESC desc;
    m_Tex->GetLevelDesc(0, &desc);
    m_Size.x = desc.Width;
    m_Size.y = desc.Height;

#else
    if(m_TexFrom) return;
    m_TexFrom = LoadTextureFromFile(to16, D3DPOOL_SYSTEMMEM);

    D3DSURFACE_DESC desc;
    m_TexFrom->GetLevelDesc(0, &desc);
    m_Format = desc.Format;
    m_Levelcnt = m_TexFrom->GetLevelCount();
    m_Size.x = desc.Width;
    m_Size.y = desc.Height;
#endif
}

void CTextureManaged::Load()
{
    DTRACE();

#ifdef USE_DX_MANAGED_TEXTURES
    if(m_Tex == nullptr)
    {
#ifdef _DEBUG
        if(m_Name.GetLen() == 0) ERROR_E;
#endif
        Init(false);
    }


#else
    ASSERT(m_Tex == nullptr);

    if(m_TexFrom == nullptr)
    {
#ifdef _DEBUG
        if(m_Name.GetLen() == 0) ERROR_E;
#endif
        Init(false);
    }

    if(g_D3DD->GetAvailableTextureMem() < (2 * 1024 * 1024))
    {
        m_OOM_counter = OOM_TEXTRUE_HIT_COUNTER;
        return;
    }

    //if(m_Tex) { m_Tex->Release(); /*--g_TexManagedCnt;*/ }
    if(FAILED(g_D3DD->CreateTexture(GetSizeX(), GetSizeY(), m_Levelcnt, 0, m_Format, D3DPOOL_DEFAULT, &m_Tex, 0)))
    {
        m_Tex = nullptr;
        m_OOM_counter = OOM_TEXTRUE_HIT_COUNTER;
    }
    else
    {
        ASSERT_DX(m_TexFrom->AddDirtyRect(nullptr));
        ASSERT_DX(g_D3DD->UpdateTexture(m_TexFrom, m_Tex));
        //++g_TexManagedCnt;
    }
#endif
}

void CTextureManaged::Unload()
{
#ifndef USE_DX_MANAGED_TEXTURES
    if(m_Tex)
    {
        m_Tex->Release(); /*--g_TexManagedCnt;*/
        m_Tex = nullptr;
    }
#endif
    //CDText::T("texcnt", CStr(g_TexManagedCnt));
}

//void CTextureManaged::LoadFromBitmapAsIs(const CBitmap& bm)
//{
//	Unload();
//
//    if(m_TexFrom) m_TexFrom->Release();
//
//    int lx,ly;
//    D3DLOCKED_RECT lr;
//
//    lx=bm.SizeX();
//    ly=bm.SizeY();
//    m_Size.x=lx;
//    m_Size.y=ly;
//
//    ASSERT((bm.BytePP()==4));
//
//    if(D3D_OK !=(g_D3DD->CreateTexture(lx,ly,1,0,D3DFMT_A8R8G8B8,/*D3DPOOL_MANAGED*/D3DPOOL_SYSTEMMEM,&m_TexFrom,0)))
//    {
//        m_TexFrom = nullptr;
//        return;
//    }
//    m_Format = D3DFMT_A8R8G8B8;
//    m_Levelcnt = 1;
//
//    ASSERT_DX(m_TexFrom->LockRect(0,&lr,nullptr,0));
//
////    m_SizeInMemory+=lr.Pitch*ly;
//
//    byte * sou=(byte *)bm.Data();
//    byte * des=(byte *)lr.pBits;
//    for(int y=0; y<ly; ++y,sou+=bm.Pitch()-lx*4,des+=lr.Pitch-lx*4)
//    {
//        //memcpy(des,sou, lx);
//        for(int x=0; x<lx; ++x,sou+=4,des+=4)
//        {
//            *(dword *)des = *(dword *)sou;
//        }
//
//    }
//
//    ASSERT_DX(m_TexFrom->UnlockRect(0));
//
//}


void CTextureManaged::LoadFromBitmap(int level, const CBitmap& bm, bool convert_to_16bit)
{
    DTRACE();

    //#ifdef _DEBUG
    //        ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS));
    //#endif

    int lx, ly;
    D3DLOCKED_RECT lr;

    lx = bm.SizeX();
    ly = bm.SizeY();

    if(bm.BytePP() == 4)
    {
        if(convert_to_16bit)
        {
#ifdef USE_DX_MANAGED_TEXTURES
            ASSERT_DX(m_Tex->LockRect(level, &lr, nullptr, 0));
#else
            ASSERT_DX(m_TexFrom->LockRect(level, &lr, nullptr, 0));
#endif

            //m_SizeInMemory += lr.Pitch * ly;

            const byte* sou = bm.ByteData();
            byte* des = (byte*)lr.pBits;
            for (int y = 0;y < ly;++y, sou += bm.Pitch() - lx * 4, des += lr.Pitch - lx * 2)
            {
                for (int x = 0;x < lx;++x, sou += 4, des += 2)
                {
                    *(des) = (*(sou + 1) & 0xF0) | (*(sou + 2) >> 4);
                    *(des + 1) = (*(sou + 3) & 0xF0) | (*(sou + 0) >> 4);
                }
            }
        }
        else
        {
#ifdef USE_DX_MANAGED_TEXTURES
            ASSERT_DX(m_Tex->LockRect(level, &lr, nullptr, 0));
#else
            ASSERT_DX(m_TexFrom->LockRect(level, &lr, nullptr, 0));
#endif
            //m_SizeInMemory += lr.Pitch * ly;

            byte* sou = (byte*)bm.Data();
            byte* des = (byte*)lr.pBits;
            for(int y = 0; y < ly; ++y, sou += bm.Pitch() - lx * 4, des += lr.Pitch - lx * 4)
            {
                for(int x = 0; x < lx; ++x, sou += 4, des += 4)
                {
                    *(des) = *(sou + 2);
                    *(des + 1) = *(sou + 1);
                    *(des + 2) = *(sou + 0);
                    *(des + 3) = *(sou + 3);
                }
            }

            // undone
//            _asm
//            {
//loop1:
//                mov     eax, [esi]
//                mov     edx, [esi + 4]
//
//                mov     ebx, eax
//                and     eax, 0xFF00FF00
//                ror     ebx, 16
//                add     edi, 8
//                and     ebx, 0x00FF00FF
//                or      eax, ebx
//                mov     ebx, edx
//                and     edx, 0xFF00FF00
//                ror     ebx, 16
//                add     esi, 8
//                and     ebx, 0x00FF00FF
//                or      edx, ebx
//
//                mov     [edi - 8], eax
//                mov     [edi - 4], edx
//
//                dec     ecx
//                jnz     loop1
//
//
//            }
        }
    }
    else if(bm.BytePP() == 3)
    {
        if(convert_to_16bit)
        {
#ifdef USE_DX_MANAGED_TEXTURES
            ASSERT_DX(m_Tex->LockRect(level, &lr, nullptr, 0));
#else
            ASSERT_DX(m_TexFrom->LockRect(level, &lr, nullptr, 0));
#endif

            //m_SizeInMemory += lr.Pitch * ly;

            byte* sou = (byte*)bm.Data();
            byte* des = (byte*)lr.pBits;
            for(int y = 0; y < ly; ++y, sou += bm.Pitch() - lx * 3, des += lr.Pitch - lx * 2)
            {
                for(int x = 0; x < lx; ++x, sou += 3, des += 2)
                {
                    //*(des) =* (sou + 2);     // r
                    //*(des + 1) =* (sou + 1); // g
                    //*(des + 2) =* (sou + 0); // b
                    //*(des + 3) =* (sou + 3); // a

                    word c = ((*(sou + 0) << 7) & 0x7C00) |
                        ((*(sou + 1) << 2) & 0x03E0) |
                        ((*(sou + 2) >> 3) & 0x001F);

                    *((word*)des) = c;
                }
            }
        }
        else
        {
#ifdef USE_DX_MANAGED_TEXTURES
            ASSERT_DX(m_Tex->LockRect(level, &lr, nullptr, 0));
#else
            ASSERT_DX(m_TexFrom->LockRect(level, &lr, nullptr, 0));
#endif
            //m_SizeInMemory += lr.Pitch * ly;

            byte* sou = (byte*)bm.Data();
            byte* des = (byte*)lr.pBits;
            for(int y = 0; y < ly; ++y, sou += bm.Pitch() - lx * 3, des += lr.Pitch - lx * 4)
            {
                for(int x = 0; x < lx; ++x, sou += 3, des += 4)
                {
                    *(des) = *(sou + 2);
                    *(des + 1) = *(sou + 1);
                    *(des + 2) = *(sou + 0);
                    *(des + 3) = 255;
                }
            }

        }
    }
    else ERROR_E;

#ifdef USE_DX_MANAGED_TEXTURES
    ASSERT_DX(m_Tex->UnlockRect(level));
#else
    ASSERT_DX(m_TexFrom->UnlockRect(level));
#endif

}

void CTextureManaged::LoadFromBitmap(const CBitmap& bm, D3DFORMAT fmt, int levels)
{
#ifdef USE_DX_MANAGED_TEXTURES
    if(m_Tex) m_Tex->Release();
    m_Tex = nullptr;
#else
    Unload();
    if(m_TexFrom) m_TexFrom->Release();
#endif

    CBuf buf(g_CacheHeap);

    bm.SaveInDDSUncompressed(buf);
    //bm.SaveInBMP(buf);
    //((CBitmap*)&bm)->SaveInPNG(buf); // const hack

#ifdef USE_DX_MANAGED_TEXTURES
    D3DXCreateTextureFromFileInMemoryEx(g_D3DD, buf.Get(), buf.Len(), 0, 0, levels, 0, fmt, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_BOX, 0, nullptr, nullptr, &m_Tex);
#else
    D3DXCreateTextureFromFileInMemoryEx(g_D3DD, buf.Get(), buf.Len(), 0, 0, levels, 0, fmt, D3DPOOL_SYSTEMMEM, D3DX_FILTER_NONE, D3DX_FILTER_BOX, 0, nullptr, nullptr, &m_TexFrom);
#endif
    //m_TexFrom->GenerateMipSubLevels(


#ifdef USE_DX_MANAGED_TEXTURES
    if(m_Tex != nullptr)
    {
        D3DSURFACE_DESC desc;
        m_Tex->GetLevelDesc(0, &desc);
        m_Size.x = desc.Width;
        m_Size.y = desc.Height;
    }
#else
    if(m_TexFrom != nullptr)
    {
        m_Levelcnt = m_TexFrom->GetLevelCount();
        D3DSURFACE_DESC desc;
        m_TexFrom->GetLevelDesc(0, &desc);

        m_Format = desc.Format;
        m_Size.x = desc.Width;
        m_Size.y = desc.Height;
    }
#endif
}

void CTextureManaged::LoadFromBitmap(const CBitmap& bm, bool convert_to_16bit, int levelcnt)
{
#ifdef USE_DX_MANAGED_TEXTURES
    if(m_Tex) m_Tex->Release();
    m_Tex = nullptr;
#else
    Unload();
    if(m_TexFrom) m_TexFrom->Release();
#endif

    int lx, ly;
    D3DLOCKED_RECT lr;

    lx = bm.SizeX();
    ly = bm.SizeY();
    m_Size.x = lx;
    m_Size.y = ly;
    //m_SizeInMemory = 0;

    if(bm.BytePP() == 4)
    {
        if(convert_to_16bit)
        {
#ifdef USE_DX_MANAGED_TEXTURES
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, levelcnt, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_Tex, 0)))
            {
                m_Tex = nullptr;
                return;
            }
#else
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, levelcnt, 0, D3DFMT_A4R4G4B4, D3DPOOL_SYSTEMMEM, &m_TexFrom, 0)))
            {
                m_TexFrom = nullptr;
                return;
            }
            m_Format = D3DFMT_A4R4G4B4;
            m_Levelcnt = m_TexFrom->GetLevelCount();
#endif
        }
        else
        {
#ifdef USE_DX_MANAGED_TEXTURES
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, levelcnt, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_Tex, 0)))
            {
                m_Tex = nullptr;
                return;
            }
#else
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, levelcnt, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_TexFrom, 0)))
            {
                m_TexFrom = nullptr;
                return;
            }
            m_Format = D3DFMT_A8R8G8B8;
            m_Levelcnt = m_TexFrom->GetLevelCount();
#endif
        }
    }
    else if(bm.BytePP() == 3)
    {
        if(convert_to_16bit)
        {
#ifdef USE_DX_MANAGED_TEXTURES
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, levelcnt, 0, D3DFMT_X1R5G5B5, D3DPOOL_MANAGED, &m_Tex, 0)))
            {
                m_Tex = nullptr;
                return;
            }
#else
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, levelcnt, 0, D3DFMT_X1R5G5B5, D3DPOOL_SYSTEMMEM, &m_TexFrom, 0)))
            {
                m_TexFrom = nullptr;
                return;
            }
            m_Format = D3DFMT_X1R5G5B5;
            m_Levelcnt = m_TexFrom->GetLevelCount();
#endif
        }
        else
        {
#ifdef USE_DX_MANAGED_TEXTURES
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, levelcnt, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_Tex, 0)))
            {
                m_Tex = nullptr;
                return;
            }
#else
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, levelcnt, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_TexFrom, 0)))
            {
                m_TexFrom = nullptr;
                return;
            }
            m_Format = D3DFMT_A8R8G8B8;
            m_Levelcnt = m_TexFrom->GetLevelCount();
#endif
        }

    }
    else if(bm.BytePP() == 1)
    {
#ifdef USE_DX_MANAGED_TEXTURES
        if(D3D_OK == g_D3DD->CreateTexture(lx, ly, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &m_Tex, 0))
        {
            ASSERT_DX(m_Tex->LockRect(0, &lr, nullptr, 0));
#else
        if(D3D_OK == g_D3DD->CreateTexture(lx, ly, 1, 0, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &m_TexFrom, 0))
        {
            m_Format = D3DFMT_L8;
            ASSERT_DX(m_TexFrom->LockRect(0, &lr, nullptr, 0));
#endif
            //m_SizeInMemory += lr.Pitch * ly;
            byte* sou = (byte*)bm.Data();
            byte* des = (byte*)lr.pBits;
            for(int y = 0; y < ly; ++y, sou += bm.Pitch(), des += lr.Pitch)
            {
                memcpy(des, sou, lx);
            }
#ifndef USE_DX_MANAGED_TEXTURES
            m_Levelcnt = 1;
#endif
            UnlockRect();
#ifdef USE_DX_MANAGED_TEXTURES
        }
        else if(D3D_OK == g_D3DD->CreateTexture(lx, ly, 1, 0, D3DFMT_A8L8, D3DPOOL_MANAGED, &m_Tex, 0))
        {
            ASSERT_DX(m_Tex->LockRect(0, &lr, nullptr, 0));
#else
        }
        else if(D3D_OK == g_D3DD->CreateTexture(lx, ly, 1, 0, D3DFMT_A8L8, D3DPOOL_SYSTEMMEM, &m_TexFrom, 0))
        {
            m_Format = D3DFMT_A8L8;
            ASSERT_DX(m_TexFrom->LockRect(0, &lr, nullptr, 0));
#endif
            //m_SizeInMemory += lr.Pitch * ly;
            byte* sou = (byte*)bm.Data();
            byte* des = (byte*)lr.pBits;
            for(int y = 0; y < ly; ++y, sou += bm.Pitch(), des += lr.Pitch)
            {
                byte* sou1 = sou;
                word* des1 = (word*)des;
                int lx2 = lx;
                while(lx2-- > 0)
                {
                    word out = (0xFF << 8) | (*sou1 << 0);
                    *des1 = out;
                    ++sou1;
                    ++des1;
                }
            }
#ifndef USE_DX_MANAGED_TEXTURES
            m_Levelcnt = 1;
#endif
            UnlockRect();
        }
        else
        {
#ifdef USE_DX_MANAGED_TEXTURES
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &m_Tex, 0)))
            {
                m_Tex = nullptr;
                return;
            }
            ASSERT_DX(m_Tex->LockRect(0, &lr, nullptr, 0));
#else
            if(D3D_OK != (g_D3DD->CreateTexture(lx, ly, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &m_TexFrom, 0)))
            {
                m_TexFrom = nullptr;
                return;
            }
            m_Format = D3DFMT_X8R8G8B8;
            ASSERT_DX(m_TexFrom->LockRect(0, &lr, nullptr, 0));
#endif
            //m_SizeInMemory += lr.Pitch*ly;
            byte* sou = (byte*)bm.Data();
            byte* des = (byte*)lr.pBits;
            for (int y = 0;y < ly;y++, sou += bm.Pitch(), des += lr.Pitch)
            {
                byte* sou1 = sou;
                dword* des1 = (dword*)des;
                int lx2 = lx;
                while (lx2-- > 0)
                {
                    dword out = (0xFF << 24) | (*sou1 << 16) | (*sou1 << 8) | (*sou1 << 0);
                    *des1 = out;
                    ++sou1;
                    ++des1;
                }
            }
#ifndef USE_DX_MANAGED_TEXTURES
            m_Levelcnt = 1;
#endif
            UnlockRect();
        }

        return;
    }
    else ERROR_E;

    LoadFromBitmap(0, bm, convert_to_16bit);

#ifdef USE_DX_MANAGED_TEXTURES
    int lcnt = m_Tex->GetLevelCount();
#else
    int lcnt = m_TexFrom->GetLevelCount();
#endif
    CBitmap bc(g_CacheHeap);

    if(bm.BytePP() == 4) bc.CreateRGBA(lx, ly);
    if(bm.BytePP() == 3) bc.CreateRGB(lx, ly);
    bc.Copy(CPoint(0, 0), bm.Size(), bm, CPoint(0, 0));

    for(int i = 1; i < lcnt; ++i)
    {
        bc.Make2xSmaller();
        LoadFromBitmap(i, bc, convert_to_16bit);
    }
}


void CTextureManaged::Ramka(int x, int y, int w, int h, dword c)
{
    D3DLOCKED_RECT lr;
#ifdef _DEBUG
    ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS));
#endif
    RECT    r;
    r.left = x;
    r.top = y;
    r.right = x + w;
    r.bottom = y + h;
#ifdef USE_DX_MANAGED_TEXTURES
    ASSERT_DX(m_Tex->LockRect(0, &lr, &r, 0));
#else
    ASSERT_DX(m_TexFrom->LockRect(0, &lr, &r, 0));
#endif
    dword* src = (dword*)lr.pBits;

    dword* src_0 = src + 1;
    dword* src_1 = (dword*)(((byte*)src) + lr.Pitch * (h - 1)) + 1;

    --w;
    while (h > 0)
    {
        *src = c;
        *(src + w) = c;
        src = (dword*)(((byte*)src) + lr.Pitch);
        --h;
    }

    while (w > 0)
    {
        *src_0 = c;
        *src_1 = c;
        ++src_0;
        ++src_1;
        --w;
    }

#ifndef USE_DX_MANAGED_TEXTURES
    if (m_Tex)
    {
        m_Tex->Release();
        m_Tex = nullptr;
    }
#endif

    UnlockRect();
}

dword CTextureManaged::GetPixelColor(int x, int y)
{
    D3DLOCKED_RECT lr;
#ifdef _DEBUG
    ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS));
#endif
    RECT    r;
    r.left = x;
    r.top = y;
    r.right = x + 1;
    r.bottom = y + 1;

    if (x < 0 || y < 0 || x >= m_Size.x || y >= m_Size.y)
        return 0;


#ifdef USE_DX_MANAGED_TEXTURES
    ASSERT_DX(m_Tex->LockRect(0, &lr, &r, D3DLOCK_READONLY));
#else
    ASSERT_DX(m_TexFrom->LockRect(0, &lr, &r, D3DLOCK_READONLY));
#endif
    byte* src = (byte*)lr.pBits;

    //dword c = *(dword *)(src + (lr.Pitch * y) + x * 4);
    dword c = *(dword*)(src);

    UnlockRect();
    return c;
}