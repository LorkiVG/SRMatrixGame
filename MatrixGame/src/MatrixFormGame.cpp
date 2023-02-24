// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixFormGame.hpp"
#include "MatrixMap.hpp"
#include "MatrixRobot.hpp"
#include "MatrixFlyer.hpp"
#include "Effects/MatrixEffect.hpp"
#include "Interface/CInterface.h"
#include "MatrixMultiSelection.hpp"
#include "MatrixMapStatic.hpp"
#include "Interface/CIFaceMenu.h"
#include "MatrixGameDll.hpp"
#include "MatrixInstantDraw.hpp"
#include "Interface/MatrixHint.hpp"
#include "MatrixObjectCannon.hpp"
#include "Interface/CCounter.h"
#include "Interface/CIFaceButton.h"
#include "Interface/CHistory.h"

#include <time.h>
#include <sys/timeb.h>
#include "stdio.h"

CFormMatrixGame::CFormMatrixGame() :CForm()
{
    DTRACE();
    m_Name = L"MatrixGame";

    m_LastWorldX = 0; m_LastWorldY = 0;
    m_Action = 0;
    ZeroMemory(m_LastScans, sizeof(SKeyScan) * MAX_SCANS);
}

CFormMatrixGame::~CFormMatrixGame()
{
    DTRACE();
}

void CFormMatrixGame::Enter(void)
{
    DTRACE();
    S3D_Default();
    D3DMATERIAL9 mtrl;
    ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    mtrl.Specular.r = 0.5f;
    mtrl.Specular.g = 0.5f;
    mtrl.Specular.b = 0.5f;
    mtrl.Specular.a = 0.5f;
    g_D3DD->SetMaterial(&mtrl);
    g_D3DD->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);

    D3DXVECTOR3 vecDir;
    D3DLIGHT9 light;
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;//D3DLIGHT_POINT;//D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    light.Ambient.r = 0.0f;
    light.Ambient.g = 0.0f;
    light.Ambient.b = 0.0f;
    light.Specular.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Specular.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Specular.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    //light.Range       = 0;
    light.Direction = g_MatrixMap->m_LightMain;
    //	light.Direction=D3DXVECTOR3(250.0f,-50.0f,-250.0f);
    //	D3DXVec3Normalize((D3DXVECTOR3 *)(&(light.Direction)),(D3DXVECTOR3 *)(&(light.Direction)));
    ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->LightEnable(0, TRUE));

    g_MatrixMap->m_Cursor.SetVisible(true);
}

void CFormMatrixGame::Leave(void)
{
    DTRACE();
}

void CFormMatrixGame::Draw(void)
{
    DTRACE();

    if(!FLAG(g_MatrixMap->m_Flags, MMFLAG_VIDEO_RESOURCES_READY)) return;

    CInstDraw::DrawFrameBegin();

    if(FLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE))
    {
        g_MatrixMap->m_DI.T(L"Automatic mode", L"");
    }

    if(FLAG(g_Config.m_DIFlags, DI_DRAWFPS)) g_MatrixMap->m_DI.T(L"FPS", CWStr(g_DrawFPS, Base::g_MatrixHeap).Get());
    if(FLAG(g_Config.m_DIFlags, DI_TMEM))
    {
        g_MatrixMap->m_DI.T(L"Free Texture Mem", CWStr(g_AvailableTexMem, Base::g_MatrixHeap).Get());
    }
    if(FLAG(g_Config.m_DIFlags, DI_TARGETCOORD))
    {
        CWStr txt(Base::g_MatrixHeap);
        txt = Float2Int(g_MatrixMap->m_Camera.GetXYStrategy().x * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        txt += L", ";
        txt += Float2Int(g_MatrixMap->m_Camera.GetXYStrategy().y * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        //txt += L", ";
        //txt += Float2Int((g_MatrixMap->m_Camera.GetTarget().z+g_MatrixMap->m_Camera.GetZRel()) * 10.0f);
        //txt.Insert(txt.GetLen()-1,L".",1);
        g_MatrixMap->m_DI.T(L"Camera target", txt.Get());
    }
    if(FLAG(g_Config.m_DIFlags, DI_FRUSTUMCENTER))
    {
        CWStr txt(Base::g_MatrixHeap);

        txt = Float2Int(g_MatrixMap->m_Camera.GetFrustumCenter().x * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        txt += L", ";
        txt += Float2Int(g_MatrixMap->m_Camera.GetFrustumCenter().y * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        txt += L", ";
        txt += Float2Int(g_MatrixMap->m_Camera.GetFrustumCenter().z * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        g_MatrixMap->m_DI.T(L"Frustum Center", txt.Get());


        //txt = Float2Int(D3DXVec3Length(&(g_MatrixMap->m_Camera.GetFrustumCenter() - (g_MatrixMap->m_Camera.GetTarget() + D3DXVECTOR3(0, 0, g_MatrixMap->m_Camera.GetZRel())))) * 10.0f);
        //txt.Insert(txt.GetLen() - 1, L".", 1);
        //g_MatrixMap->m_DI.T(L"Cam dist", txt.Get());

        //g_MatrixMap->m_DI.T(L"Z rel", CWStr(g_MatrixMap->m_Camera.GetZRel()));
    }

	g_MatrixMap->BeforeDraw();

	//ASSERT_DX(g_D3DD->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0));
    
    if(FLAG(g_Flags, GFLAG_STENCILAVAILABLE))
    {
#if defined _DEBUG || defined EXE_VERSION
        ASSERT_DX(g_D3DD->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL | D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 255), 1.0f, 0));
#else
        ASSERT_DX(g_D3DD->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0));
#endif

        if(CInterface::ClearRects_GetCount()) ASSERT_DX(g_D3DD->Clear(CInterface::ClearRects_GetCount(), CInterface::ClearRects_Get(), D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255, 0, 255), 0.0f, 0));
    }
    else
    {
        ASSERT_DX(g_D3DD->Clear(0, nullptr, D3DCLEAR_ZBUFFER , D3DCOLOR_XRGB(255,0,255), 1.0f, 0));
        if(CInterface::ClearRects_GetCount()) ASSERT_DX(g_D3DD->Clear(CInterface::ClearRects_GetCount(), CInterface::ClearRects_Get(), D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 0, 255), 0.0f, 0));
    }

	ASSERT_DX(g_D3DD->BeginScene());
#ifdef _DEBUG
    if(!FLAG(g_Flags, GFLAG_EXTRAFREERES))
    {
        SETFLAG(g_Flags, GFLAG_RENDERINPROGRESS);
    }
#endif

	g_MatrixMap->Draw();

	ASSERT_DX(g_D3DD->EndScene());

    //SETFLAG(g_Flags, GFLAG_PRESENT_REQUIRED);
    static int current_delay = 0;

    static dword last_time = 0;
    dword ctime = timeGetTime();
    int step1 = (last_time <= ctime) ? (ctime - last_time) : (0xFFFFFFFF - last_time + ctime);
    last_time = ctime;

    float cfps = 1000.0f / float(step1);

    if(cfps > float(g_MaxFPS))
    {
        ++current_delay;
    }
    else
    {
        --current_delay;
        if(current_delay < 0) current_delay = 0;
    }

    Sleep(current_delay);
    /*
    if(FLAG(g_Flags, GFLAG_PRESENT_REQUIRED))
    {
        ASSERT_DX(g_D3DD->Present(nullptr, nullptr, nullptr, nullptr));
        RESETFLAG(g_Flags, GFLAG_PRESENT_REQUIRED);
    }
    */
    ASSERT_DX(g_D3DD->Present(nullptr, nullptr, nullptr, nullptr));

#ifdef _DEBUG
    RESETFLAG(g_Flags, GFLAG_RENDERINPROGRESS);
    RESETFLAG(g_Flags, GFLAG_EXTRAFREERES);
#endif

#ifdef MEM_SPY_ENABLE
    //Вывод на экран счётчика аллокации (только той, что выделил непосредственно дабоменеджер) в заголовке онка игры
    CStr text = "Memory Allocated: " + CStr((int)SMemHeader::fullsize);
    SetWindowTextA(g_Wnd, text);
#endif
}

//Главный тактовый логический исполнитель игры
void CFormMatrixGame::Tact(int step)
{
DTRACE();

    if(g_MatrixMap->CheckLostDevice()) return;

	g_MatrixMap->Tact(step);

    CPoint mp = g_MatrixMap->m_Cursor.GetPos();

    //Перемещение камеры движением мыши в стратегическом режиме
    if(!g_MatrixMap->GetPlayerSide()->IsArcadeMode())
    {
        if(mp.x >= 0 && mp.x < g_ScreenX && mp.y >= 0 && mp.y < g_ScreenY)
        {
            if(mp.x < MOUSE_BORDER) g_MatrixMap->m_Camera.MoveLeft();
            if(mp.x > (g_ScreenX - MOUSE_BORDER)) g_MatrixMap->m_Camera.MoveRight();
            if(mp.y < MOUSE_BORDER) g_MatrixMap->m_Camera.MoveUp();
            if(mp.y > (g_ScreenY - MOUSE_BORDER)) g_MatrixMap->m_Camera.MoveDown();
        }
    }

    if(g_MatrixMap->m_Console.IsActive()) return;

    if(!g_MatrixMap->GetPlayerSide()->IsArcadeMode())
    {
        //Перед движением камеры по клавишам делаем проверку на то, что игрок не находится в режиме конструктора
        //В режиме конструктора стандартные кнопки движения камеры используются под другие бинды
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_LEFT]) & 0x8000) == 0x8000 || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_LEFT_ALT]) & 0x8000) == 0x8000))
        {
            if(!g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) g_MatrixMap->m_Camera.MoveLeft();
        }
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_RIGHT]) & 0x8000) == 0x8000 || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_RIGHT_ALT]) & 0x8000) == 0x8000))
        {
            if(!g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) g_MatrixMap->m_Camera.MoveRight();
        }
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_UP]) & 0x8000) == 0x8000 || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_UP_ALT]) & 0x8000) == 0x8000))
        {
            if(!g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) g_MatrixMap->m_Camera.MoveUp();
        }
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_DOWN]) & 0x8000) == 0x8000 || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_DOWN_ALT]) & 0x8000) == 0x8000))
        {
            if(!g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) g_MatrixMap->m_Camera.MoveDown();
        }
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_LEFT]) & 0x8000) == 0x8000 || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_LEFT_ALT]) & 0x8000) == 0x8000))
        {
            g_MatrixMap->m_Camera.RotLeft();
        }
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_RIGHT]) & 0x8000) == 0x8000 || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_RIGHT_ALT]) & 0x8000) == 0x8000))
        {
            g_MatrixMap->m_Camera.RotRight();
        }
    }

    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_UP]) & 0x8000) == 0x8000)
    {
        g_MatrixMap->m_Camera.RotUp();
    }
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_DOWN]) & 0x8000) == 0x8000)
    {
        g_MatrixMap->m_Camera.RotDown();
    }

    //Создаётся и сохраняется игровой скриншот (не заносится в буфер обмена)
    if(GetAsyncKeyState(g_Config.m_KeyActions[KA_SAVE_SCREENSHOT]))
    {
        CreateDirectory(PathToOutputFiles(FOLDER_NAME_SCREENSHOTS), nullptr);

        // seek files

        int maxn = 0;

        WIN32_FIND_DATA fd;

        int slen = strlen(FILE_NAME_SCREENSHOT);
		
        CStr n(PathToOutputFiles(FOLDER_NAME_SCREENSHOTS), g_CacheHeap);
		
        n += "\\";
        n.Add(FILE_NAME_SCREENSHOT, slen);
        n += "*.*";

        HANDLE ff = FindFirstFile(n.Get(), &fd);
        if(ff != INVALID_HANDLE_VALUE)
        {
            while(true)
            {
                if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    n = fd.cFileName;
                    int idx = n.FindR(FILE_NAME_SCREENSHOT, slen);
                    n.Del(0, idx + slen);
                    idx = n.Find(".", 1);
                    if(idx > 0) n.SetLen(idx);
                    int nn = n.GetInt();
                    if(nn > maxn) maxn = nn;
                }
                if(0 == FindNextFile(ff, &fd)) break;
            }
            FindClose(ff);
            ++maxn;
        }

        n.Set( maxn );
        n.Insert(0, "000", 3);
        n.Del(0, n.Len() - 3);

		/*
		CStr    n("", g_CacheHeap);

		struct _timeb timebuffer;
        time_t time1;
        unsigned short millitm1;

        _ftime64( &timebuffer );

        time1 = timebuffer.time;
        millitm1 = timebuffer.millitm;
        struct tm  tstruct;
        char       buf[80];
        tstruct = *localtime(&time1);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H.%M.%S.", &tstruct);
		n.Set( millitm1 );
        n.Insert(0, "000", 3);
        n.Del(0, n.Len() - 3);

		n.Insert(0, buf, 20);
		n.Insert(0, " ", 1);
        */

        n.Insert(0, FILE_NAME_SCREENSHOT, slen);
        n.Insert(0, "\\", 1);
        n.Insert(0, PathToOutputFiles(FOLDER_NAME_SCREENSHOTS));
        n.Add(".png", 4);

        DeleteFile(n.Get());

        if(!g_D3Dpp.Windowed)
        {
            IDirect3DSurface9* pTargetSurface = nullptr;
            HRESULT hr = D3D_OK;

            if(!g_D3Dpp.MultiSampleType)
            hr = g_D3DD->GetRenderTarget(0, &pTargetSurface);

            if(hr == D3D_OK)
            {
                D3DSURFACE_DESC desc;

                if(!g_D3Dpp.MultiSampleType)
                {
                    hr = pTargetSurface->GetDesc(&desc);
                }
                else
                {
                    desc.Width = g_ScreenX;
                    desc.Height = g_ScreenY;
                    desc.Format = D3DFMT_A8R8G8B8;
                }
                if(hr == D3D_OK)
                {
                    IDirect3DSurface9* pSurface = nullptr;
                    hr = g_D3DD->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pSurface, nullptr);
                    if(hr == D3D_OK)
                    {
                        if(!g_D3Dpp.MultiSampleType)
                        {
                            hr = g_D3DD->GetRenderTargetData(pTargetSurface, pSurface);
                        }
                        else
                        {
                            hr = g_D3DD->GetFrontBufferData(0, pSurface);
                        }
                        if(hr == D3D_OK)
                        {
                            D3DLOCKED_RECT lockrect;
                            hr = pSurface->LockRect(&lockrect, nullptr, 0);
                            if(hr == D3D_OK)
                            {
                                CBitmap bm;
                                bm.CreateRGB(desc.Width, desc.Height);

                                for(UINT y = 0; y < desc.Height; ++y)
                                {
                                    unsigned char* buf_src = (unsigned char*)lockrect.pBits + lockrect.Pitch * y;
                                    unsigned char* buf_des = (unsigned char*)bm.Data() + bm.Pitch() * y;

                                    for(UINT x = 0; x < desc.Width; ++x)
                                    {
                                        //memcpy(buf_des, buf_src, 3);
                                        buf_des[0] = buf_src[2];
                                        buf_des[1] = buf_src[1];
                                        buf_des[2] = buf_src[0];

                                        buf_src += 4;
                                        buf_des += 3;
                                    }
                                }

                                pSurface->UnlockRect();

                                bm.SaveInPNG(CWStr(n).Get());
                                g_MatrixMap->m_DI.T(L"Screen shot has been saved", L"");
                            }
                            else
                            {
                                //LockRect fail
                                //OutputDebugStringA("LockRect fail\n");
                            }
                        }
                        else
                        {
                            // GetRenderTargetData fail
                            //char s[256];
                            //sprintf_s(s, sizeof(s), "GetRenderTargetData fail - 0x%08X, %u, %d\n", hr, hr, hr);
                            //OutputDebugStringA(s);
                        }
                        pSurface->Release();
                    }
                    else
                    {
                        //CreateOffscreenPlainSurface fail
                        //OutputDebugStringA("CreateOffscreenPlainSurface fail\n");
                    }
                }
                else
                {
                    // GetDesc fail
                    //OutputDebugStringA("GetDesc fail\n");
                }
            
                if(pTargetSurface) pTargetSurface->Release();
            }
            else
            {
                //GetRenderTarget fail
                //OutputDebugStringA("GetRenderTarget fail\n");
            }

            return;
        }

        CBitmap bm(g_CacheHeap);
        CBitmap bmout(g_CacheHeap);
        bmout.CreateRGB(g_ScreenX, g_ScreenY);

        HDC hdc = GetDC(g_Wnd);

        bm.WBM_Bitmap(CreateCompatibleBitmap(hdc, g_ScreenX, g_ScreenY));
        bm.WBM_BitmapDC(CreateCompatibleDC(hdc));

        if(!SelectObject(bm.WBM_BitmapDC(), bm.WBM_Bitmap()))
        {
            ReleaseDC(g_Wnd, hdc);
            return;
        }

        BitBlt(bm.WBM_BitmapDC(), 0, 0, g_ScreenX, g_ScreenY, hdc, 0, 0, SRCCOPY);

        ReleaseDC(g_Wnd, hdc);

        bm.WBM_Save(true);

        bmout.Copy(CPoint(0, 0), bm.Size(), bm, CPoint(0, 0));
        bmout.SaveInPNG(CWStr(n).Get());
		
        //HFree(data, g_CacheHeap);

        g_MatrixMap->m_DI.T(L"Screen shot has been saved", L"");
    }
}

static int g_LastPosX;
static int g_LastPosY;


#if (defined _DEBUG) &&  !(defined _RELDEBUG)
static SEffectHandler point(DEBUG_CALL_INFO);
static CMatrixEffectPath *path = 0;
static CMatrixEffectSelection *sel = 0;
static CMatrixEffectRepair *repair = 0;

void selcallback(CMatrixMapStatic *ms, dword param)
{
    if(ms->GetObjectType() == OBJECT_TYPE_MAPOBJECT) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Mesh", 1000);
    else if(ms->IsRobot()) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Robot", 1000);
    else if(ms->IsCannon()) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Cannon", 1000);
    else if(ms->IsBuilding()) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"BuildinCMultiSelection::m_GameSelection->Update(g_MatrixMap->m_Cursor.GetPos(), TRACE_ROBOT | TRACE_FLYER | TRACE_BUILDING, SideSelectionCallBack, g",1000);
    else if(ms->GetObjectType() == OBJECT_TYPE_FLYER) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Flyer", 1000);
    SideSelectionCallBack(ms, param);
}
#else

void selcallback(CMatrixMapStatic *ms, dword param)
{
    SideSelectionCallBack(ms, param);
}

#endif


void CFormMatrixGame::MouseMove(int x, int y)
{
	CMatrixSideUnit* p_side = g_MatrixMap->GetPlayerSide();

    if(g_MatrixMap->IsMouseCam())
    {
        g_MatrixMap->m_Camera.RotateByMouse(x - g_MatrixMap->m_Cursor.GetPosX(), y - g_MatrixMap->m_Cursor.GetPosY());

        CPoint p = g_MatrixMap->m_Cursor.GetPos();
        ClientToScreen(g_Wnd, &p);

        //SetCursorPos(p.x, p.y);

        if(p_side->GetArcadedObject())
        {
            int dx = x - g_MatrixMap->m_Cursor.GetPosX();
            //if(dx < 0) p_side->GetArcadedObject()->AsRobot()->RotateRobotLeft();
            //if(dx > 0) p_side->GetArcadedObject()->AsRobot()->RotateRobotRight();

            //Поворот шасси
            CMatrixRobotAI* robot = p_side->GetArcadedObject()->AsRobot();
            if(dx < 0)
            {
                //robot->RotateRobotLeft(); //Вроде как тут обработчик учитывает ещё и отклонение по оси Z, но я не уверен, что оно до сих пор актуально

                D3DXVECTOR3 dest = D3DXVECTOR3(robot->m_ChassisForward.y, -robot->m_ChassisForward.x, robot->m_ChassisForward.z);
                if(!robot->IsStrafing()) robot->RotateRobotChassis(dest + D3DXVECTOR3(robot->m_PosX, robot->m_PosY, 0.0f), CONNECTED_CAM_ROTATION); //Проверка на стрейф
                else robot->RotateRobotChassis(dest + D3DXVECTOR3(robot->m_PosX, robot->m_PosY, 0.0f), SIMULTANEOUS_CAM_ROTATION);
            }
            if(dx > 0)
            {
                //robot->RotateRobotRight(); //Вроде как тут обработчик учитывает ещё и отклонение по оси Z, но я не уверен, что оно до сих пор актуально

                D3DXVECTOR3 dest = D3DXVECTOR3(-robot->m_ChassisForward.y, robot->m_ChassisForward.x, robot->m_ChassisForward.z);
                if(!robot->IsStrafing()) robot->RotateRobotChassis(dest + D3DXVECTOR3(robot->m_PosX, robot->m_PosY, 0.0f), CONNECTED_CAM_ROTATION); //Проверка на стрейф
                else robot->RotateRobotChassis(dest + D3DXVECTOR3(robot->m_PosX, robot->m_PosY, 0.0f), SIMULTANEOUS_CAM_ROTATION);
            }
        }

        g_MatrixMap->m_Cursor.SetPos(x, y);

        return;
    }

    g_MatrixMap->m_Cursor.SetPos(x, y);
    p_side->OnMouseMove();

    if(CMultiSelection::m_GameSelection)
    {
        SCallback cbs;
        cbs.mp = CPoint(x, y);
        cbs.calls = 0;
        CMultiSelection::m_GameSelection->Update(g_MatrixMap->m_Cursor.GetPos(), TRACE_ROBOT|TRACE_BUILDING, selcallback, (dword)&cbs);
    }

    //Interface
    SETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_ENABLE_DRAG);

	//bool fRBut = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0x8000;

    /*
    { // cell information

        float xx, yy;
        g_MatrixMap->CalcPickVector(CPoint(x, y), vDir);
		g_MatrixMap->UnitPickWorld(g_MatrixMap->GetFrustumCenter(), vDir, &xx, &yy);

        xx /= GLOBAL_SCALE;
        yy /= GLOBAL_SCALE;

        CDText::T("CELL", (CStr((int)xx) + "," + CStr((int)yy)).Get());
    }
    */

#if(defined _DEBUG) && !(defined _RELDEBUG)
    if(point.effect)
    {
        D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
        pos.z += 10.0f;
        ((CMatrixEffectPointLight *)point.effect)->SetPos(pos);
    }
    if(sel)
    {
        D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
        pos.z += 10.0f;
        sel->SetPos(pos);
    }
    if(repair)
    {
        D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
        pos.z += 10.0f;
        repair->UpdateData(pos, D3DXVECTOR3(1, 0, 0));
    }
#endif

}

void CFormMatrixGame::MouseKey(ButtonStatus status, int key, int x, int y)
{
DTRACE();

    if(status == B_WHEEL)
    {
        while(key > 0)
        {
            g_MatrixMap->m_Camera.ZoomInStep();
            --key;
        }
        while(key < 0)
        {
            g_MatrixMap->m_Camera.ZoomOutStep();
            ++key;
        }

        return;
    }

DCP();

    if(status == B_UP && key == VK_MBUTTON)
    {
        g_MatrixMap->MouseCam(false);
        return;
    }
    if(status == B_DOWN && key == VK_MBUTTON)
    {
        g_MatrixMap->MouseCam(true);
        //SetCursorPos(g_ScreenX/2, g_ScreenY/2);

        return;
    }

DCP();

	m_Action = 0;
	
	//bool fCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0x8000;

    /*
    if(fCtrl && down && key == VK_RBUTTON)
    {
		D3DXVECTOR3 vpos, vdir;
        g_MatrixMap->CalcPickVector(CPoint(x, y), vdir);
		g_MatrixMap->UnitPickWorld(g_MatrixMap->GetFrustumCenter(), vdir, &m_LastWorldX, &m_LastWorldY);
		m_Action = 1;
	}
    */

    DCP();

    //Игрок кликнул левой кнопкой мыши (отпустил), после того, как под его курсором уже сформировалась область выделения
    //Область выделения формируется под курсором даже в случае быстрого однократного клика,
    //однако в выделение в таком случае попадает лишь один объект под курсором
    //Все выделения юнитов и зданий по клику мышью происходят тут
    if(status == B_UP && key == VK_LBUTTON)
    {
    DCP();
        //Если ранее была создана область выделения (создаётся она в этой же функции, но ниже)
        if(CMultiSelection::m_GameSelection)
        {
            CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

            SCallback cbs;
            cbs.mp = CPoint(-1, -1);
            cbs.calls = 0;

            //Завершаем процесс расширения анимации области выделения
            CMultiSelection::m_GameSelection->End();
    DCP();
            if(true/*cbs.calls > 0*/)
            {
    DCP();
                //Если в текущей выделяемой группе имеется либо несколько роботов, либо несколько вертолётов, либо как минимум один робот и один вертолёт
                if(ps->GetCurSelGroup()->GetRobotsCnt() > 1 || ps->GetCurSelGroup()->GetFlyersCnt() > 1 || (ps->GetCurSelGroup()->GetRobotsCnt() + ps->GetCurSelGroup()->GetFlyersCnt()) > 1)
                {
                    ps->GetCurSelGroup()->RemoveBuildings();
                    //Если игрок выделяет юнитов с зажатым Shift
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SHIFT]) & 0x8000) == 0x8000 && ps->GetCurGroup())
                    {
                        CMatrixGroupObject* go = ps->GetCurSelGroup()->m_FirstObject;
                        while(go)
                        {
                            if(ps->GetCurGroup()->FindObject(go->GetObject()))
                            {
                                CMatrixGroupObject* go_tmp = go->m_NextObject;
                                ps->RemoveObjectFromSelectedGroup(go->GetObject());
                                go = go_tmp;
                                continue;
                            }
                            go = go->m_NextObject;
                        }
                        ps->AddToCurrentGroup();
                    }
                    else
                    {
                        ps->SetCurGroup(ps->CreateGroupFromCurrent());
                    }
                    if(ps->GetCurGroup() && ps->GetCurGroup()->GetRobotsCnt() && ps->GetCurGroup()->GetFlyersCnt())
                    {
                        ps->GetCurGroup()->SortFlyers();
                    }
                    ps->Select(GROUP, nullptr);
                }
                //Если в текущей выделяемой группе находится всего один робот
                else if(ps->GetCurSelGroup()->GetRobotsCnt() == 1 && !ps->GetCurSelGroup()->GetFlyersCnt())
                {
    DCP();
                    ps->GetCurSelGroup()->RemoveBuildings();

                    //Если игрок выделяет юнитов с зажатым Shift
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SHIFT]) & 0x8000) == 0x8000 && ps->GetCurGroup())
                    {
                        if(ps->GetCurGroup()->FindObject(ps->GetCurSelGroup()->m_FirstObject->GetObject()))
                        {
                            ps->RemoveObjectFromSelectedGroup(ps->GetCurSelGroup()->m_FirstObject->GetObject());
                        }
                        else
                        {
                            ps->AddToCurrentGroup();
                        }

                        if(ps->GetCurGroup() && ps->GetCurGroup()->GetRobotsCnt() && ps->GetCurGroup()->GetFlyersCnt())
                        {
                            ps->GetCurGroup()->SortFlyers();
                        }
                        ps->Select(GROUP, nullptr);
                    }
                    //Если просто кликнул
                    else
                    {
                        ps->SetCurGroup(ps->CreateGroupFromCurrent());
                        ps->Select(ROBOT, nullptr);
                    }
                }
                //Если в текущей выделяемой группе находится всего один вертолёт
                else if(ps->GetCurSelGroup()->GetFlyersCnt() == 1 && !ps->GetCurSelGroup()->GetRobotsCnt())
                {
    DCP();
                    ps->GetCurSelGroup()->RemoveBuildings();

                    //Если игрок выделяет юнитов с зажатым Shift
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SHIFT]) & 0x8000) == 0x8000 && ps->GetCurGroup())
                    {
                        if(ps->GetCurGroup()->FindObject(ps->GetCurSelGroup()->m_FirstObject->GetObject()))
                        {
                            ps->RemoveObjectFromSelectedGroup(ps->GetCurSelGroup()->m_FirstObject->GetObject());
                        }
                        else
                        {
                            ps->AddToCurrentGroup();
                        }
                        if(ps->GetCurGroup() && ps->GetCurGroup()->GetRobotsCnt() && ps->GetCurGroup()->GetFlyersCnt())
                        {
                            ps->GetCurGroup()->SortFlyers();
                        }
                        ps->Select(GROUP, nullptr);
                    }
                    //Если просто кликнул
                    else
                    {
                        ps->SetCurGroup(ps->CreateGroupFromCurrent());
                        ps->Select(FLYER, nullptr);
                    }
                }
                //Если в текущей выделяемой группе находится здание
                else if(ps->GetCurSelGroup()->GetBuildingsCnt() && !ps->GetCurSelGroup()->GetRobotsCnt() && !ps->GetCurSelGroup()->GetFlyersCnt())
                {
    DCP();
                    ps->Select(BUILDING, ps->GetCurSelGroup()->m_FirstObject->GetObject());
                    ps->GroupsUnselectSoft();
                    ps->GetCurSelGroup()->RemoveAll();
                    ps->SetCurGroup(nullptr);
                    ps->Reselect();
                }
            }
        }
        CMultiSelection::m_GameSelection = nullptr;
	}
    DCP();

    //Если игрок навёл мышь на интерфейс
    if(g_IFaceList->m_InFocus == INTERFACE)
    {
    DCP();

        //Ловим левые клики по интерфейсу
		if(status == B_UP && key == VK_LBUTTON)
        {
			g_IFaceList->OnMouseLBUp();
		}
        else if((status == B_DOWN || status == B_DOUBLE) && key == VK_LBUTTON)
        {
            g_IFaceList->OnMouseLBDown();
		}

        //Ловим правые клики по интерфейсу
        if(status == B_UP && key == VK_RBUTTON)
        {
            g_IFaceList->OnMouseRBUp();
        }
        else if((status == B_DOWN || status == B_DOUBLE) && key == VK_RBUTTON)
        {
            g_IFaceList->OnMouseRBDown();
        }
	}
    //Если игрок навёл мышь в случайную область экрана, но не на интерфейс
    else if(g_IFaceList->m_InFocus == UNKNOWN) //or something else
    {
    DCP();
		if(status == B_DOWN && key == VK_RBUTTON)
        {
    DCP();
			g_MatrixMap->GetPlayerSide()->OnRButtonDown(CPoint(x, y));
		}
        else if(status == B_DOWN && key == VK_LBUTTON)
        {
    DCP();
            //Если игрок зажал левую кнопку мыши и расширяет область выделения
            //(если ещё не была создана область выделения, игрок не в режиме прямого управления, игрок не выбрал приказ из панели на интерфейсе и не находится в процессе выбора места для постройки турели)
            if(CMultiSelection::m_GameSelection == nullptr && !g_MatrixMap->GetPlayerSide()->IsArcadeMode() && !IS_PREORDERING_NOSELECT && !(g_MatrixMap->GetPlayerSide()->m_CurrentAction == BUILDING_TURRET))
            {
                int dx = 0, dy = 0;
                if(IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj) && IS_TRACE_UNIT(g_MatrixMap->m_TraceStopObj))
                {
                    dx = 2;
                    dy = 2;
                }
                CMultiSelection::m_GameSelection = CMultiSelection::Begin(CPoint(g_MatrixMap->m_Cursor.GetPos().x - dx, g_MatrixMap->m_Cursor.GetPos().y - dy));
                if(CMultiSelection::m_GameSelection)
                {
                    SCallback cbs;
                    cbs.mp = g_MatrixMap->m_Cursor.GetPos();
                    cbs.calls = 0;

                    CMultiSelection::m_GameSelection->Update(g_MatrixMap->m_Cursor.GetPos(), TRACE_ROBOT | TRACE_BUILDING, selcallback, (dword)&cbs);
                }
            }

            g_MatrixMap->GetPlayerSide()->OnLButtonDown(CPoint(x, y));
		}
        else if(status == B_UP && key == VK_RBUTTON)
        {
    DCP();
			g_MatrixMap->GetPlayerSide()->OnRButtonUp(CPoint(x, y));
        }
        else if(status == B_UP && key == VK_LBUTTON)
        {
    DCP();
    		CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
            ps->OnLButtonUp(CPoint(x, y));
		}
        else if(status == B_DOUBLE && key == VK_LBUTTON)
        {
    DCP();
			g_MatrixMap->GetPlayerSide()->OnLButtonDouble(CPoint(x, y));
		}
        else if(status == B_DOUBLE && key == VK_RBUTTON)
        {
    DCP();
			g_MatrixMap->GetPlayerSide()->OnRButtonDouble(CPoint(x, y));
		}
	}
}

void ExitRequestHandler(void);
void ConfirmCancelHandler(void);
void ResetRequestHandler(void);
void SurrenderRequestHandler(void);

struct STextInfo
{
    const wchar *t1;
    const wchar *t2;
    int time;
};

static STextInfo stuff[] = 
{
    {L"3D Robots game information....", L"", 3000},
    {L"Coding....", L"", 10000},
    {L" Alexander <ZakkeR> Zeberg", L"", 0},
    {L"", L"Engine lead coder", 0},
    {L"", L"MapEditor lead coder"},
    {L"", L"Optimizations"},
    {L" Alexey <Dab> Dubovoy", L"", 0},
    {L"", L"High-AI lead coder", 0},
    {L"", L"MapEditor base coder", 0},
    {L" Alexander <Sub0> Parshin", L"", 0},
    {L"", L"Low-AI lead coder", 0},
    {L"", L"UI lead coder", 0},

    {L"Artwork...", L"", 10000},
    {L" Eugene <Johan> Cherenkov", L"", 0},
    {L"", L"UI", 0},
    {L"", L"Some cool textures", 0},
    {L"", L"Sky", 0},

    {L" Nina <Nina> Vatulich", L"", 0},
    {L"", L"Terrain textures", 0},
    {L"", L"Effects textures", 0},

    {L"Modeling...", L"", 8000},
    {L" Nina <Nina> Vatulich", L"", 0},
    {L"", L"Lot of meshes", 0},

    {L" Alexander <Alexartist> Yazynin", L"", 0},
    {L"", L"Meshes", 0},
    {L"", L"Buildings", 0},

    {L" Ruslan <IronFist> Tchernyi", L"", 0},
    {L"", L"Advanced meshes", 0},

    {L" Sergey <Esk> Simonov", L"", 0},
    {L"", L"Robots", 0},
    {L"", L"Helicopters", 0},
    {L"", L"Some meshes", 0},

    {L"Map design...", L"", 7000},
    {L" Alexander <Alexartist> Yazynin", L"", 0},
    {L" Ruslan <IronFist> Tchernyi", L"", 0},
    {L" Nina <Nina> Vatulich", L"", 0},


    {L"Game balancing...", L"", 7000},
    {L" Alexander <Alexartist> Yazynin", L"", 0},
    {L"", L"Maps", 0},
    {L" Dmitry <Dm> Gusarov", L"", 0},
    {L"", L"Items", 0},
    {L" Nina <Nina> Vatulich", L"", 0},
    {L"", L"Maps", 0},

    {L"Game texts and sounds...", L"", 5000},
    {L" Ilia <Ilik> Plusnin", L"", 0},

    {L"Thats all folks :)", L"", 3000},
    {nullptr, nullptr}
};

//Обработчики нажатия кнопок на клавиатуре
void CFormMatrixGame::Keyboard(bool down, int scan)
{
    DTRACE();

    bool fCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0x8000;

    //Если была активирована дев-консоль, то обработка всех прочих нажатий отрубается
    //С активной консолью все нажатия должен начать перехватывать код консоли в DevConsole.cpp
    if(g_MatrixMap->m_Console.IsActive())
    {
        //g_MatrixMap->m_Console.SetActive(true);
        g_MatrixMap->m_Console.Keyboard(scan, down);
        return;
    }

    if(down)
    {
        memcpy(&m_LastScans[0], &m_LastScans[1], sizeof(SKeyScan) * (MAX_SCANS - 1));
        m_LastScans[MAX_SCANS - 1].scan = scan;
        m_LastScans[MAX_SCANS - 1].time = g_MatrixMap->GetTime();

//Обработчики набора читов
#ifdef CHEATS_ON
//Вызов консоли не срабатывает, нужно откапывать оригинальную функцию вызова
/*
        if (m_LastScans[MAX_SCANS - 6].scan == KEY_D)
            if (m_LastScans[MAX_SCANS - 5].scan == KEY_E)
                if (m_LastScans[MAX_SCANS - 4].scan == KEY_V)
                    if (m_LastScans[MAX_SCANS - 3].scan == KEY_C)
                        if (m_LastScans[MAX_SCANS - 2].scan == KEY_O)
                            if (m_LastScans[MAX_SCANS - 1].scan == KEY_N)
                            //if (m_LastScans[MAX_SCANS - 1].scan == KEY_TILDA)
                            {
                                m_LastScans[MAX_SCANS - 1].scan = 0;
                                g_MatrixMap->m_Console.SetActive(true);
                                return;
                            }
*/
        if (m_LastScans[MAX_SCANS - 7].scan == KEY_S)
            if (m_LastScans[MAX_SCANS - 6].scan == KEY_H)
                if (m_LastScans[MAX_SCANS - 5].scan == KEY_O)
                    if (m_LastScans[MAX_SCANS - 4].scan == KEY_W)
                        if (m_LastScans[MAX_SCANS - 3].scan == KEY_F)
                            if (m_LastScans[MAX_SCANS - 2].scan == KEY_P)
                                if (m_LastScans[MAX_SCANS - 1].scan == KEY_S)
                                {
                                    m_LastScans[MAX_SCANS - 1].scan = 0;
                                    INVERTFLAG(g_Config.m_DIFlags, DI_DRAWFPS);
                                    return;
                                }
        if (m_LastScans[MAX_SCANS - 5].scan == KEY_I)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_N)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_F)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_O)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_SPACE)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;
                            INVERTFLAG(g_Config.m_DIFlags, DI_TMEM|DI_TARGETCOORD|DI_VISOBJ|DI_ACTIVESOUNDS|DI_FRUSTUMCENTER);
                            return;
                        }
        if (m_LastScans[MAX_SCANS - 4].scan == KEY_A)
            if (m_LastScans[MAX_SCANS - 3].scan == KEY_U)
                if (m_LastScans[MAX_SCANS - 2].scan == KEY_T)
                    if (m_LastScans[MAX_SCANS - 1].scan == KEY_O)
                    {
                        m_LastScans[MAX_SCANS - 1].scan = 0;
                        INVERTFLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE);
                        return;
                    }

        if (m_LastScans[MAX_SCANS - 6].scan == KEY_F)
            if (m_LastScans[MAX_SCANS - 5].scan == KEY_L)
                if (m_LastScans[MAX_SCANS - 4].scan == KEY_Y)
                    if (m_LastScans[MAX_SCANS - 3].scan == KEY_C)
                        if (m_LastScans[MAX_SCANS - 2].scan == KEY_A)
                            if (m_LastScans[MAX_SCANS - 1].scan == KEY_M)
                            {
                                m_LastScans[MAX_SCANS - 1].scan = 0;
                                INVERTFLAG(g_MatrixMap->m_Flags, MMFLAG_FLYCAM);
                                return;
                            }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_B)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_A)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_B)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_K)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_I)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;
                            INVERTFLAG(g_Config.m_DIFlags, DI_SIDEINFO);
                            return;
                        }

        if (m_LastScans[MAX_SCANS - 8].scan == KEY_C)
            if (m_LastScans[MAX_SCANS - 7].scan == KEY_R)
                if (m_LastScans[MAX_SCANS - 6].scan == KEY_A)
                    if (m_LastScans[MAX_SCANS - 5].scan == KEY_Z)
                        if (m_LastScans[MAX_SCANS - 4].scan == KEY_Y)
                            if (m_LastScans[MAX_SCANS - 3].scan == KEY_B)
                                if (m_LastScans[MAX_SCANS - 2].scan == KEY_O)
                                    if (m_LastScans[MAX_SCANS - 1].scan == KEY_T)
                                    {
                                        m_LastScans[MAX_SCANS - 1].scan = 0;
                                        g_MatrixMap->GetPlayerSide()->BuildCrazyBot();
                                        return;
                                    }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_H)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_U)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_R)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_R)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_Y)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;
                            if (!g_MatrixMap->ReinforcementsDisabled() && g_MatrixMap->BeforeReinforcementsTime() > 0)
                            {
                                g_MatrixMap->SetReinforcementsTime(1);
                                return;
                            }

                        }

        if (m_LastScans[MAX_SCANS - 8].scan == KEY_M)
            if (m_LastScans[MAX_SCANS - 7].scan == KEY_E)
                if (m_LastScans[MAX_SCANS - 6].scan == KEY_G)
                    if (m_LastScans[MAX_SCANS - 5].scan == KEY_A)
                        if (m_LastScans[MAX_SCANS - 4].scan == KEY_B)
                            if (m_LastScans[MAX_SCANS - 3].scan == KEY_U)
                                if (m_LastScans[MAX_SCANS - 2].scan == KEY_S)
                                    if (m_LastScans[MAX_SCANS - 1].scan == KEY_T)
                                    {
                                        m_LastScans[MAX_SCANS - 1].scan = 0;
                            
                                        //CSound::Play(S_BENTER, SL_INTERFACE);
                                        //CSound::Play(S_REINFORCEMENTS_READY, SL_INTERFACE);
                                        //CSound::Play(S_ROBOT_BUILD_END, SL_ALL);
                                        //CSound::Play(S_TERRON_KILLED, SL_ALL);
                                        //CSound::Play(S_TURRET_BUILD_2, SL_ALL);

                                        if(FLAG(g_MatrixMap->m_Flags, MMFLAG_MEGABUSTALREADY))
                                        //if(0)
                                        {
                                            CMatrixMapStatic* s = CMatrixMapStatic::GetFirstLogic();
                                            for(; s; s = s->GetNextLogic())
                                            {
                                                if(s->GetSide() == PLAYER_SIDE)
                                                {
                                                    if(s->IsRobot() && !s->AsRobot()->IsAutomaticMode())
                                                    {
                                                        s->AsRobot()->MustDie();
                                                    }
                                                    /*
                                                    else if(s->IsCannon())
                                                    {
                                                        s->AsCannon()->InitMaxHitpoint(s->AsCannon()->GetMaxHitPoint() * 20);
                                                    }
                                                    else if(s->IsBuilding())
                                                    {
                                                        s->AsBuilding()->InitMaxHitpoint(s->AsBuilding()->GetMaxHitPoint() * 20);
                                                    }
                                                    */
                                                }
                                            }
                                        }
                                        else
                                        {
                                            CMatrixMapStatic* s = CMatrixMapStatic::GetFirstLogic();
                                            for(; s; s = s->GetNextLogic())
                                            {
                                                if (s->GetSide() == PLAYER_SIDE)
                                                {
                                                    if (s->IsRobot())
                                                    {
                                                        s->AsRobot()->InitMaxHitpoint(s->AsRobot()->GetMaxHitPoint() * 20);
                                            
                                                    }
                                                    else if (s->IsCannon())
                                                    {
                                                        s->AsCannon()->InitMaxHitpoint(s->AsCannon()->GetMaxHitPoint() * 20);
                                                    }
                                                    else if (s->IsBuilding())
                                                    {
                                                        s->AsBuilding()->InitMaxHitpoint(s->AsBuilding()->GetMaxHitPoint() * 20);
                                                    }

                                                }
                                            }
                                            SETFLAG(g_MatrixMap->m_Flags, MMFLAG_MEGABUSTALREADY);
                                        }
                                        return;
                                    }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_S)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_T)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_A)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_T)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_S)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;

                            if (!g_MatrixMap->IsPaused())
                            {
                                SETFLAG(g_MatrixMap->m_Flags, MMFLAG_STAT_DIALOG_D);
                            }
                            return;
                        }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_V)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_I)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_D)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_E)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_O)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;
                            g_MatrixMap->m_DI.T(L"_____________________________", L"",10000);
                            g_MatrixMap->m_DI.T(L"Sim textures", CWStr(g_D3DDCaps.MaxSimultaneousTextures, g_CacheHeap).Get(), 10000);
                            g_MatrixMap->m_DI.T(L"Stencil available", FLAG(g_Flags,GFLAG_STENCILAVAILABLE)?L"Yes":L"No", 10000);

                            // vidmode
	                        D3DDISPLAYMODE d3ddm;

                            CWStr modet(g_CacheHeap);
                            CWStr modev(g_CacheHeap);
                            for (int i = 0; i < 2; ++i)
                            {
                                modet.Set(L"Buffer ");
                                modet += i;
                                modet += L" mode";
	                            ASSERT_DX(g_D3DD->GetDisplayMode(0,&d3ddm));
                                if (d3ddm.Format == D3DFMT_X8R8G8B8)
                                {
                                    modev = L"X8R8G8B8";
                                }
                                else if (d3ddm.Format == D3DFMT_A8R8G8B8)
                                {
                                    modev = L"A8R8G8B8";
                                }
                                else if (d3ddm.Format == D3DFMT_R8G8B8)
                                {
                                    modev = L"R8G8B8";
                                }
                                else if (d3ddm.Format == D3DFMT_R5G6B5)
                                {
                                    modev = L"R5G6B5";
                                }
                                else
                                {
                                    modev.Set(d3ddm.Format);
                                }
                                g_MatrixMap->m_DI.T(modet.Get(), modev.Get(), 10000);

                            }

                            return;
                        }

        if (m_LastScans[MAX_SCANS - 9].scan == KEY_I)
            if (m_LastScans[MAX_SCANS - 8].scan == KEY_A)
                if (m_LastScans[MAX_SCANS - 7].scan == KEY_M)
                    if (m_LastScans[MAX_SCANS - 6].scan == KEY_L)
                        if (m_LastScans[MAX_SCANS - 5].scan == KEY_O)
                            if (m_LastScans[MAX_SCANS - 4].scan == KEY_O)
                                if (m_LastScans[MAX_SCANS - 3].scan == KEY_S)
                                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_E)
                                        if(m_LastScans[MAX_SCANS - 1].scan == KEY_R)
                                        {
                                            g_ExitState = ES_EXIT_AFTER_WIN;
                                            g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_WIN);
                                            return;
                                        }

        if (m_LastScans[MAX_SCANS - 7].scan == KEY_B)
            if (m_LastScans[MAX_SCANS - 6].scan == KEY_U)
                if (m_LastScans[MAX_SCANS - 5].scan == KEY_B)
                    if (m_LastScans[MAX_SCANS - 4].scan == KEY_U)
                        if (m_LastScans[MAX_SCANS - 3].scan == KEY_B)
                            if (m_LastScans[MAX_SCANS - 2].scan == KEY_U)
                                if (m_LastScans[MAX_SCANS - 1].scan == KEY_SPACE)
                                {
                                    m_LastScans[MAX_SCANS - 1].scan = 0;

                                    int delay = 0;
                                    int ctime = 0;
                                    int od = 0;
                                    for (int i = 0; stuff[i].t1 != nullptr; ++i)
                                    {
                                        if (stuff[i].time)
                                        {
                                            od = delay;
                                            ctime = stuff[i].time;
                                            delay += stuff[i].time + 100;
                                        }
                                        g_MatrixMap->m_DI.T(stuff[i].t1, stuff[i].t2, ctime, od, true);
                                    }
                                    return;
                                }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_C)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_R)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_A)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_S)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_H)
                        {
                            _asm xor eax,eax
                            _asm mov [eax],  eax
                            return;
                        }
#endif
    }

    //Горячие клавиши для команд в игровом меню (выйти, рестарт, сдаться и т.д.)
    if(scan == KEY_ENTER && down)
    {
        if(g_MatrixMap->m_DialogModeName && (g_MatrixMap->m_DialogModeHints.Len() > 4 || wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) != 0))
        {
            g_IFaceList->PressHintButton(HINT_OK);
            return;
        }
    } 

    if(scan == KEY_E && down)
    {
        if(g_MatrixMap->m_DialogModeName && wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) == 0)
        {
            if(g_MatrixMap->m_DialogModeHints.Len() > 4)
            {}
            else
            {
                ExitRequestHandler();
                return;
            }
        }
    } 
    if(scan == KEY_S && down)
    {
        if(g_MatrixMap->m_DialogModeName && wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) == 0)
        {
            if(g_MatrixMap->m_DialogModeHints.Len() > 4)
            {}
            else
            {
                SurrenderRequestHandler();
                return;
            }
        }
    } 
    if(scan == KEY_R && down)
    {
        if(g_MatrixMap->m_DialogModeName && wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) == 0)
        {
            if(g_MatrixMap->m_DialogModeHints.Len() > 4)
            {}
            else
            {
                ResetRequestHandler();
                return;
            }
        }
    } 

    if(scan == KEY_ESC && down)
    {
        if(FLAG(g_MatrixMap->m_Flags, MMFLAG_FULLAUTO))
        {
            g_ExitState = ES_EXIT_TO_MAIN_MENU;
            SETFLAG(g_Flags, GFLAG_EXITLOOP);
            return;
        }

        //Если открыт режим конструктора, то просто закрываем его
        if(g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive())
        {
            g_MatrixMap->GetPlayerSide()->m_ConstructPanel->ResetGroupNClose();
            return;
        }
        //Ловится нажатие Esc для захода/выхода в меню, а также закрытия всяких диалоговых окно и т.д.
        else if(g_MatrixMap->m_DialogModeName && !wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU))
        {
            if(g_MatrixMap->m_DialogModeHints.Len() > 4) ConfirmCancelHandler();
            else g_MatrixMap->LeaveDialogMode();
            return;
        }
        //Если открыто окно с приветственным, либо победным/пораженческим текстом, либо со статистикой боя - закрываем его
        else if(FLAG(g_MatrixMap->m_Flags, MMFLAG_DIALOG_MODE))
        {
            if(g_MatrixMap->m_BeforeWinLoseDialogCount == 1 && g_ExitState < ES_EXIT_TO_MAIN_MENU)
            {
                //Если игрок победил
                if(FLAG(g_MatrixMap->m_Flags, MMFLAG_WIN)) g_MatrixMap->OkExitWinHandler();
                //Если игрок проиграл
                else g_MatrixMap->OkExitLoseHandler();
            }
            else if(g_ExitState >= ES_EXIT_TO_MAIN_MENU /*||g_ExitState == ES_EXIT_AFTER_WIN || g_ExitState == ES_EXIT_AFTER_LOSE || g_ExitState == ES_EXIT_AFTER_SURRENDER*/) g_MatrixMap->OkJustExitHandler();
            else g_MatrixMap->OkHandler();
            return;
        }

        if(FLAG(g_MatrixMap->m_Flags, MMFLAG_DIALOG_MODE)) return;

        if(g_MatrixMap->GetPlayerSide()->GetArcadedObject())
        {
            g_IFaceList->ExitArcadeMode();
            return;
        }

//Отладочное завершение боя - победа, либо поражение, по нажатию Left/Right Shift + Esc соответственно
#ifdef _TRACE
        if((GetAsyncKeyState(VK_LSHIFT) & 0x8000) == 0x8000)
        {
            g_MatrixMap->m_BeforeWinLoseDialogCount = 1;
            SETFLAG(g_MatrixMap->m_Flags, MMFLAG_WIN);

            //SETFLAG(g_Flags, GFLAG_EXITLOOP);
            g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_WIN);
            return;
        }
        if((GetAsyncKeyState(VK_RSHIFT) & 0x8000) == 0x8000)
        {
            g_MatrixMap->m_BeforeWinLoseDialogCount = 1;

            //SETFLAG(g_Flags, GFLAG_EXITLOOP);
            g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_LOSE);
            return;
        }
#endif

        g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_MENU);
		return;
	}

    //down - однократное нажатие кнопки (точнее, какая конкретно кнопка была утоплена)
    //scan - запись последовательности нажатых за последнее время кнопок
    g_MatrixMap->m_KeyDown = down;
    g_MatrixMap->m_KeyScan = scan;

    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD_ALT]) & 0x8000) == 0x8000)
    {
        g_MatrixMap->GetPlayerSide()->OnForward(true);
    }
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD_ALT]) & 0x8000) == 0x8000)
    {
        g_MatrixMap->GetPlayerSide()->OnBackward(true);
    }

    if(down)
    {
        //Выставление камеры в позицию по умолчанию
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_CAM_SETDEFAULT]) & 0x8000) == 0x8000)
        {
            g_MatrixMap->m_Camera.ResetAngles();
            return;
        }

        //Стандартный скриншот с занесением изображения в буфер обмена
        //Не пиняйте строго, код взял на stackoverflow
        //К тому же оно не сохраняет эффекты на экране, вообще хз (Klaxons)
        if(GetAsyncKeyState(g_Config.m_KeyActions[KA_TAKE_SCREENSHOT]))//if(scan == KEY_SNAPSHOT)
        {
            int x1, y1, x2, y2, w, h;

            // get screen dimensions
            x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
            y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
            x2 = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            y2 = GetSystemMetrics(SM_CYVIRTUALSCREEN);
            w = x2 - x1;
            h = y2 - y1;

            // copy screen to bitmap
            HDC     hScreen = GetDC(nullptr);
            HDC     hDC = CreateCompatibleDC(hScreen);
            HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
            HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
            BOOL    bRet = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

            // save bitmap to clipboard
            OpenClipboard(nullptr);
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, hBitmap);
            CloseClipboard();

            // clean up
            SelectObject(hDC, old_obj);
            DeleteDC(hDC);
            ReleaseDC(nullptr, hScreen);
            DeleteObject(hBitmap);
            return;
        }

        //Выставляем паузу в игре, если она ещё не включена
        if(GetAsyncKeyState(g_Config.m_KeyActions[KA_GAME_PAUSED]))//if(scan == KEY_PAUSE)
        {
            g_MatrixMap->Pause(!g_MatrixMap->IsPaused());
            return;
        }

        CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
        if(ps->IsRobotArcadeMode())
        {
            CMatrixRobotAI* robot = ps->GetArcadedObject()->AsRobot();

            //Включение/выключение активных орудий
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_SET_GUN_1]) & 0x8000) == 0x8000)
            {
                SRobotWeapon* gun = robot->GetWeaponByPylonNum(0);
                if(gun && gun->GetWeaponNum() != WEAPON_BOMB)
                {
                    if(gun->m_On)
                    {
                        gun->FireEnd(); //Зацикленные орудия не прекращают стрелять сами по себе, пока зажата кнопка огня
                        gun->m_On = false;
                    }
                    else gun->m_On = true;

                    CSound::AddSound(S_WEAPON_SET, robot->GetGeoCenter(), SL_ALL, SEF_SKIP);
                }
                return;
            }
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_SET_GUN_2]) & 0x8000) == 0x8000)
            {
                SRobotWeapon* gun = robot->GetWeaponByPylonNum(1);
                if(gun && gun->GetWeaponNum() != WEAPON_BOMB)
                {
                    if(gun->m_On)
                    {
                        gun->FireEnd(); //Зацикленные орудия не прекращают стрелять сами по себе, пока зажата кнопка огня
                        gun->m_On = false;
                    }
                    else gun->m_On = true;

                    CSound::AddSound(S_WEAPON_SET, robot->GetGeoCenter(), SL_ALL, SEF_SKIP);
                }
                return;
            }
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_SET_GUN_3]) & 0x8000) == 0x8000)
            {
                SRobotWeapon* gun = robot->GetWeaponByPylonNum(2);
                if(gun && gun->GetWeaponNum() != WEAPON_BOMB)
                {
                    if(gun->m_On)
                    {
                        gun->FireEnd(); //Зацикленные орудия не прекращают стрелять сами по себе, пока зажата кнопка огня
                        gun->m_On = false;
                    }
                    else gun->m_On = true;

                    CSound::AddSound(S_WEAPON_SET, robot->GetGeoCenter(), SL_ALL, SEF_SKIP);
                }
                return;
            }
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_SET_GUN_4]) & 0x8000) == 0x8000)
            {
                SRobotWeapon* gun = robot->GetWeaponByPylonNum(3);
                if(gun && gun->GetWeaponNum() != WEAPON_BOMB)
                {
                    if(gun->m_On)
                    {
                        gun->FireEnd(); //Зацикленные орудия не прекращают стрелять сами по себе, пока зажата кнопка огня
                        gun->m_On = false;
                    }
                    else gun->m_On = true;

                    CSound::AddSound(S_WEAPON_SET, robot->GetGeoCenter(), SL_ALL, SEF_SKIP);
                }
                return;
            }
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_SET_GUN_5]) & 0x8000) == 0x8000)
            {
                SRobotWeapon* gun = robot->GetWeaponByPylonNum(4);
                if(gun && gun->GetWeaponNum() != WEAPON_BOMB)
                {
                    if(gun->m_On)
                    {
                        gun->FireEnd(); //Зацикленные орудия не прекращают стрелять сами по себе, пока зажата кнопка огня
                        gun->m_On = false;
                    }
                    else gun->m_On = true;

                    CSound::AddSound(S_WEAPON_SET, robot->GetGeoCenter(), SL_ALL, SEF_SKIP);
                }
                return;
            }

            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BOOM]) & 0x8000) == 0x8000)
            {
                //"E" - Взорвать.
                if(robot->HaveBomb()) robot->BigBoom();
                return;
            }

            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER_ALT]) & 0x8000) == 0x8000)
            {
                //"Esc", "Пробел", "Enter" - выйти из режима ручного управления роботом
                if(!robot->IsManualControlLocked()) g_IFaceList->ExitArcadeMode();
                return;
            }
        }
        else if(ps->IsFlyerArcadeMode())
        {
            CMatrixFlyer* flyer = ps->GetArcadedObject()->AsFlyer();

            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER_ALT]) & 0x8000) == 0x8000)
            {
                //"Esc", "Пробел", "Enter" - выйти из режима ручного управления вертолётом
                if(!flyer->IsManualControlLocked()) g_IFaceList->ExitArcadeMode();
                return;
            }
        }
        else
        {
            //Выбор всех роботов игрока на карте
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ALL_UNITS_SELECT]) & 0x8000) == 0x8000)//if(scan == KEY_F2)
            {
                if(ps->GetCurGroup())
                {
                    ps->SelectedGroupUnselect();
                    ps->GetCurSelGroup()->RemoveAll();
                }

                CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
                int i = 0;
                while(obj)
                {
                    if(obj->GetSide() == PLAYER_SIDE && obj->IsRobotAlive() && !obj->AsRobot()->IsCrazy())
                    {
                        ps->GetCurSelGroup()->AddObject(obj, -4);
                        ++i;
                    }
                    obj = obj->GetNextLogic();
                }

                if(i)
                {
                    ps->CreateGroupFromCurrent();

                    if(ps->GetCurGroup()->GetObjectsCnt() == 1) ps->Select(ROBOT, nullptr);
                    else /*i > 1*/ ps->Select(GROUP, nullptr);
                }

                return;
            }

            if(!FLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE)/*!IS_PREORDERING_NOSELECT*/)
            {
                //Если мы не в режиме приказа
                if(ps->GetCurGroup() && (ps->m_CurrSel == ROBOT_SELECTED || ps->m_CurrSel == FLYER_SELECTED || ps->m_CurrSel == GROUP_SELECTED))
                {
                    //Стратегический режим - выбрана группа
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER_ALT]) & 0x8000) == 0x8000)
                    {
                        //"Esc", "Пробел", "Enter" - войти в режим ручного управления юнитом
                        CMatrixMapStatic* obj = ps->GetCurGroup()->GetObjectByN(ps->GetCurSelNum());
                        if(!obj->IsManualControlLocked())
                        {
                            ps->GetCurSelGroup()->RemoveAll();
                            ps->CreateGroupFromCurrent(obj);
                            g_IFaceList->EnterArcadeMode(false);
                        }
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTOORDER_ATTACK]) & 0x8000) == 0x8000)
                    {
                        //a"U"to attack - Автоматическая программа атаки
                        if(FLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON))
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
                            ps->PGOrderStop(ps->SelGroupToLogicGroup());
                        }
                        else
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
                            SETFLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON);
                            ps->PGOrderAutoAttack(ps->SelGroupToLogicGroup());
                        }
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTOORDER_CAPTURE]) & 0x8000) == 0x8000)
                    {
                        //"C"apture - Автоматическая программа захвата
                        if(FLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON))
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
                            ps->PGOrderStop(ps->SelGroupToLogicGroup());
                        }
                        else
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
                            SETFLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON);
                            ps->PGOrderAutoCapture(ps->SelGroupToLogicGroup());
                        }
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTOORDER_DEFEND]) & 0x8000) == 0x8000)
                    {
                        //"D"efender - Автоматическая программа защиты
                        if(FLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON))
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
                            ps->PGOrderStop(ps->SelGroupToLogicGroup());
                        }
                        else
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
                            SETFLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON);
                            ps->PGOrderAutoDefence(ps->SelGroupToLogicGroup());
                        }
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_MOVE]) & 0x8000) == 0x8000)
                    {
                        //"M"ove - Двигаться
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_MOVE);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_STOP]) & 0x8000) == 0x8000)
                    {
                        //"S"top - Стоять
                        ps->PGOrderStop(ps->SelGroupToLogicGroup());
                        ps->SelectedGroupBreakOrders();
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_CAPTURE]) & 0x8000) == 0x8000)
                    {
                        //Ta"K"e - Захватить
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_CAPTURE);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_PATROL]) & 0x8000) == 0x8000)
                    {
                        //"P"atrol - Патрулировать
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_PATROL);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if(ps->GetCurGroup()->GetBombersCnt() && (GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_EXPLODE]) & 0x8000) == 0x8000)
                    {
                        //"E"xplode - Взорвать
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BOMB);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if(ps->GetCurGroup()->GetRepairsCnt() && (GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_REPAIR]) & 0x8000) == 0x8000)
                    {
                        //"R"epair - Чинить
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_REPAIR);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_ATTACK]) & 0x8000) == 0x8000)
                    {
                        //"A"ttack
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_FIRE);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                }
                else if(ps->m_CurrSel == BUILDING_SELECTED || ps->m_CurrSel == BASE_SELECTED)
                {
                    //Стратегический режим - выбрана база или завод
                    CMatrixBuilding* bld = (CMatrixBuilding*)ps->m_ActiveObject;
                    
                    //Быстрые клавиши для базы и заводов
                    if(!bld->IsBase() || !ps->m_ConstructPanel->IsActive())
                    {
                        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_TURRET]) & 0x8000) == 0x8000)
                        {
                            //"T"urrel - переход в меню выбора турели
                            CPoint pl[MAX_PLACES];
                            bool cant_build_tu = false;

                            if(
                                (
                                  ps->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_LIGHT_CANNON]) ||
                                  ps->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_HEAVY_CANNON]) ||
                                  ps->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_LASER_CANNON]) ||
                                  ps->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_MISSILE_CANNON])
                                ) && (bld->GetPlacesForTurrets(pl) > 0) && !bld->m_BS.IsMaxItems()
                              )
                            {
                                ps->m_ConstructPanel->ResetGroupNClose();
                                SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
                                SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET);

                                CMatrixMapStatic* ms = CMatrixMapStatic::GetFirstLogic();
                                for(; ms; ms = ms->GetNextLogic())
                                {
                                    if(ms == ps->m_ActiveObject && ms->IsBuildingAlive() && ms->AsBuilding()->m_Side == PLAYER_SIDE)
                                    {
                                        ms->AsBuilding()->CreatePlacesShow();
                                        break;
                                    }
                                }
                            }
                        }
                        else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_CALL_REINFORCEMENTS]) & 0x8000) == 0x8000)
                        {
                            //"Н"elp - Вызов подкрепления
                            bld->Reinforcements();
                        }
                    }

                    //Быстрые клавиши только для базы
                    if(bld->IsBase())
                    {
                        //Игрок выбрал базу и находится на её общем экране
                        if(!ps->m_ConstructPanel->IsActive())
                        {
                            //"B"uild - переход в режим конструктора роботов
                            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT]) & 0x8000) == 0x8000)
                            {
                                g_IFaceList->m_RCountControl->Reset();
                                g_IFaceList->m_RCountControl->CheckUp();
                                ps->m_ConstructPanel->ActivateAndSelect();
                            }
                            //"G"athering point - установка точки сбора базы
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_GATHERING_POINT]) & 0x8000) == 0x8000)
                            {
                                bld->SetGatheringPoint(Float2Int(g_MatrixMap->m_TraceStopPos.x), Float2Int(g_MatrixMap->m_TraceStopPos.y));
                                //Первоначальная отрисовка точки сбора (перенесена в функцию общей постоянной отрисовки)
                                //CMatrixEffect::CreateMoveToAnim(2);
                            }
                        }
                        //Игрок находится в режиме конструктора роботов выбранной базы
                        else
                        {
                            //"B"uild (либо "X") - выход из режима конструктора роботов
                            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_CANCEL]) & 0x8000) == 0x8000)
                            {
                                ps->m_ConstructPanel->ResetGroupNClose();
                            }
                            //"Enter" - запуск производства робота
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_START]) & 0x8000) == 0x8000)
                            {
                                if(g_MatrixMap->GetPlayerSide()->m_Constructor->m_BaseConstBuildBut->GetState() != IFACE_DISABLED)
                                {
                                    CSound::Play(S_BUILD_CLICK, SL_INTERFACE);
                                    g_MatrixMap->GetPlayerSide()->m_Constructor->RemoteBuild(nullptr);
                                }
                            }
                            //"Arrow Up" - увеличение числа роботов для постройки
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_QUANTITY_UP]) & 0x8000) == 0x8000)
                            {
                                if(g_IFaceList->m_RCountControl->m_ButtonUp->GetState() != IFACE_DISABLED)
                                {
                                    CSound::Play(S_BCLICK, SL_INTERFACE);
                                    g_IFaceList->m_RCountControl->Up(nullptr);
                                }
                            }
                            //"Arrow Down" - снижение числа роботов для постройки
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_QUANTITY_DOWN]) & 0x8000) == 0x8000)
                            {
                                if(g_IFaceList->m_RCountControl->m_ButtonDown->GetState() != IFACE_DISABLED)
                                {
                                    CSound::Play(S_BCLICK, SL_INTERFACE);
                                    g_IFaceList->m_RCountControl->Down(nullptr);
                                }
                            }
                            //"Arrow Left" - выбор схемы робота для постройки влево
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_CHOOSE_LEFT]) & 0x8000) == 0x8000)
                            {
                                if(g_ConfigHistory->IsPrev())
                                {
                                    CSound::Play(S_BCLICK, SL_INTERFACE);
                                    g_ConfigHistory->PrevConfig(nullptr);
                                }
                            }
                            //"Arrow Right" - выбор схемы робота для постройки вправо
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_CHOOSE_RIGHT]) & 0x8000) == 0x8000)
                            {
                                if(g_ConfigHistory->IsNext())
                                {
                                    CSound::Play(S_BCLICK, SL_INTERFACE);
                                    g_ConfigHistory->NextConfig(nullptr);
                                }
                            }
                            //Были нажаты цифровые клавиши 0-9 (по местной нумерации 2-11)
                            else if(scan >= 2 && scan <= 11 && ps->m_ConstructPanel->m_FocusedElement)
                            {
                                int key = 0;
                                if(scan != 11) key = scan - 1; //Чтобы номера клавиш соответствовали фактическим номерам, а 11 - это 0

                                ERobotModuleType type = MRT_EMPTY;
                                ERobotModuleKind kind = RUK_EMPTY;
                                CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
                                int cur_hull_kind = player_side->m_ConstructPanel->m_Configs[player_side->m_ConstructPanel->m_CurrentConfig].m_Hull.m_Module.m_nKind;
                                int weapons_available = 0;
                                int pilon = -1;

                                if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_WEAPON_PYLON_1)
                                {
                                    pilon = RUK_WEAPON_PYLON_1;
                                    type = MRT_WEAPON;
                                }
                                else if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_WEAPON_PYLON_2)
                                {
                                    pilon = RUK_WEAPON_PYLON_2;
                                    type = MRT_WEAPON;
                                }
                                else if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_WEAPON_PYLON_3)
                                {
                                    pilon = RUK_WEAPON_PYLON_3;
                                    type = MRT_WEAPON;
                                }
                                else if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_WEAPON_PYLON_4)
                                {
                                    pilon = RUK_WEAPON_PYLON_4;
                                    type = MRT_WEAPON;
                                }
                                else if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_WEAPON_PYLON_5)
                                {
                                    pilon = RUK_WEAPON_PYLON_5;
                                    type = MRT_WEAPON;
                                }
                                else if(key <= g_Config.m_RobotHeadsConsts[0].for_player_side && ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_HEAD_PYLON)
                                {
                                    pilon = 0;
                                    type = MRT_HEAD;

                                    //Определяем следующую подходящую голову в выпадающем списке
                                    int head_num;
                                    if(!key) head_num = RUK_EMPTY;
                                    else
                                    {
                                        //Корректируем индекс в списке голов с учётом недоступных для игрока позиций
                                        head_num = 0;
                                        for(int i = 1; i <= ROBOT_HEADS_COUNT; ++i)
                                        {
                                            if(!g_Config.m_RobotHeadsConsts[i].for_player_side) continue;
                                            ++head_num;

                                            if(head_num == key)
                                            {
                                                head_num = i;
                                                break;
                                            }
                                        }
                                    }

                                    kind = (ERobotModuleKind)head_num;
                                }
                                else if(key && key <= g_Config.m_RobotHullsConsts[0].for_player_side && ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_HULL_PYLON)
                                {
                                    pilon = 0;
                                    type = MRT_HULL;

                                    //Определяем следующий подходящий корпус в выпадающем списке
                                    //Корректируем индекс в списке корпусов с учётом недоступных для игрока позиций
                                    int hull_num = 0;
                                    for(int i = 1; i <= ROBOT_HULLS_COUNT; ++i)
                                    {
                                        if(!g_Config.m_RobotHullsConsts[i].for_player_side) continue;
                                        ++hull_num;

                                        if(hull_num == key)
                                        {
                                            hull_num = i;
                                            break;
                                        }
                                    }

                                    kind = (ERobotModuleKind)hull_num;
                                }
                                else if(key && key <= g_Config.m_RobotChassisConsts[0].for_player_side && ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_CHASSIS_PYLON)
                                {
                                    pilon = 0;
                                    type = MRT_CHASSIS;

                                    //Определяем следующее подходящее шасси в выпадающем списке
                                    //Корректируем индекс в списке шасси с учётом недоступных для игрока позиций
                                    int chassis_num = 0;
                                    for(int i = 1; i <= ROBOT_CHASSIS_COUNT; ++i)
                                    {
                                        if(!g_Config.m_RobotChassisConsts[i].for_player_side) continue;
                                        ++chassis_num;

                                        if(chassis_num == key)
                                        {
                                            chassis_num = i;
                                            break;
                                        }
                                    }

                                    kind = (ERobotModuleKind)chassis_num;
                                }

                                if(type == MRT_WEAPON)
                                {
                                    //Необходимо определить реальный номер оружейного пилона на модели, т.к. их там может быть меньше максимального числа
                                    int model_pylon = 0;
                                    for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data.size(); ++i)
                                    {
                                        if(g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].constructor_slot_num == pilon)
                                        {
                                            model_pylon = i;
                                            break;
                                        }
                                    }

                                    //Считаем, сколько типов оружия доступно для данного слота
                                    for(int i = 1; i <= ROBOT_WEAPONS_COUNT /*g_Config.m_RobotWeaponsConsts[0].for_player_side*/; ++i)
                                    {
                                        if(!g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[model_pylon].fit_weapon.test(i)) continue; //Если данное оружие не подходит для второго слота
                                        if(!g_Config.m_RobotWeaponsConsts[i].for_player_side) continue;
                                        ++weapons_available;
                                    }

                                    if(key <= weapons_available)
                                    {
                                        //Определяем следующее подходящее оружие в выпадающем списке
                                        int weapon_num;
                                        if(!key) weapon_num = RUK_EMPTY;
                                        else
                                        {
                                            //Корректируем индекс в списке оружия с учётом недоступных для игрока и данного слота позиций
                                            weapon_num = 0;
                                            for(int i = 1; i <= ROBOT_WEAPONS_COUNT; ++i)
                                            {
                                                if(!g_Config.m_RobotWeaponsConsts[i].for_player_side || !g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[model_pylon].fit_weapon.test(i)) continue;
                                                ++weapon_num;

                                                if(weapon_num == key)
                                                {
                                                    weapon_num = i;
                                                    break;
                                                }
                                            }
                                        }

                                        kind = (ERobotModuleKind)weapon_num;
                                    }
                                    else type = (ERobotModuleType)0;
                                }

                                if(type) ps->m_Constructor->ReplaceRobotModule(type, kind, pilon);
                            }
                        }
                    }
                }
            }
            else
            {
                //Если мы в режиме приказа или постройки пушки
                if(FLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE) && FLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET) && ps->m_CurrentAction != BUILDING_TURRET)
                {
                    //player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[1]) //Подразумевалась проверка на достаток ресурсов хотя бы для первой турели
                    //Меню выбора турелей:
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_TURRET_CANNON]) & 0x8000) == 0x8000)
                    {
                        //"C"annon - Турель
                        g_IFaceList->BeginBuildTurret(1);
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_TURRET_GUN]) & 0x8000) == 0x8000)
                    {
                        //"G"un - Пушка
                        g_IFaceList->BeginBuildTurret(2);
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_TURRET_LASER]) & 0x8000) == 0x8000)
                    {
                        //"L"azer - Лазер
                        g_IFaceList->BeginBuildTurret(3);
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_TURRET_ROCKET]) & 0x8000) == 0x8000)
                    {
                        //"R"ocket - Ракетница
                        g_IFaceList->BeginBuildTurret(4);
                    }
                    //Нафиг не нужно, поскольку и так сбрасывается на X
                    /*
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_TURRET]) & 0x8000) == 0x8000)
                    {
                        //"T"urret - выход из меню выбора турели
                        g_IFaceList->ResetOrderingMode();
                    }
                    */
                }
                if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_CANCEL]) & 0x8000) == 0x8000)
                {
                    //Отмена постройки турели, закрытие меню выбора (в том числе, если уже была выбрана турель)
                    if(ps->m_CurrentAction == BUILDING_TURRET)
                    {
                        ps->m_CannonForBuild.Delete();
                        ps->m_CurrentAction = NOTHING_SPECIAL;
                    }
                    g_IFaceList->ResetOrderingMode();
                }
            }
            //Общее для стратегического режима
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_MINIMAP_ZOOMIN]) & 0x8000) == 0x8000)
            {
                //приблизить карту
                g_MatrixMap->m_Minimap.ButtonZoomIn(nullptr);
            }
            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_MINIMAP_ZOOMOUT]) & 0x8000) == 0x8000)
            {
                //отдалить карту
                g_MatrixMap->m_Minimap.ButtonZoomOut(nullptr);
            }
            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_ROBOT_SWITCH1]) & 0x8000) == 0x8000)
            {
                //","
                CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
                if(ps->GetCurGroup() && ps->GetCurGroup()->GetObjectsCnt() == 1 && ps->GetCurGroup()->m_FirstObject->GetObject()->IsRobotAlive())
                {
                    obj = ps->GetCurGroup()->m_FirstObject->GetObject()->GetPrevLogic();
                }
                int cnt = 0;
                while(true)
                {
                    if(obj)
                    {
                        if(obj->IsRobotAlive() && obj->GetSide() == PLAYER_SIDE)
                        {
                            ps->GetCurSelGroup()->RemoveAll();
                            ps->CreateGroupFromCurrent(obj);
                            ps->Select(ROBOT, obj);
                            g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(obj->GetGeoCenter().x, obj->GetGeoCenter().y));
                            return;
                        }
                        obj = obj->GetPrevLogic();
                    }
                    else
                    {
                        if(cnt > 0)
                        {
                            return;
                        }
                        ++cnt;
                        obj = CMatrixMapStatic::GetLastLogic();
                    }
                }
            }
            else if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_ROBOT_SWITCH2]) & 0x8000) == 0x8000))
            {
                //"." 
                CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
                if(ps->GetCurGroup() && ps->GetCurGroup()->GetObjectsCnt() == 1 && ps->GetCurGroup()->m_FirstObject->GetObject()->IsRobotAlive())
                {
                    obj = ps->GetCurGroup()->m_FirstObject->GetObject()->GetNextLogic();
                }

                int cnt = 0;
                while(true)
                {
                    if(obj)
                    {
                        if(obj->IsRobotAlive() && obj->GetSide() == PLAYER_SIDE)
                        {
                            ps->GetCurSelGroup()->RemoveAll();
                            ps->CreateGroupFromCurrent(obj);
                            ps->Select(ROBOT, obj);
                            g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(obj->GetGeoCenter().x, obj->GetGeoCenter().y));
                            return;
                        }
                        obj = obj->GetNextLogic();
                    }
                    else
                    {
                        if(cnt > 0)
                        {
                            return;
                        }
                        ++cnt;
                        obj = CMatrixMapStatic::GetFirstLogic();
                    }
                }
            }
        }

        //Если были нажаты клавиши цифр от 1 до 9 (в номенклатуре номеров это 2-10)
        if(scan > 1 && scan < 11 && !ps->IsArcadeMode() && !ps->m_ConstructPanel->IsActive())
        {
            if(CMultiSelection::m_GameSelection)
            {
                CMultiSelection::m_GameSelection->End(false);
            }

            if(ps->m_CurrentAction == BUILDING_TURRET)
            {
                ps->m_CannonForBuild.Delete();
                ps->m_CurrentAction = NOTHING_SPECIAL;
            }
            
            g_IFaceList->ResetOrderingMode();

            //Если также игрок зажал Ctrl
            if(fCtrl)
            {
                CMatrixMapStatic* o = CMatrixMapStatic::GetFirstLogic();

                //Если была выбрана группа роботов (либо один), то здесь удаляется, возможно, уже имевшаяся Ctrl-группа с нажатым номером
                while(o)
                {
                    if(o->IsRobot() && ((CMatrixRobotAI*)o)->GetCtrlGroup() == scan)
                    {
                        ((CMatrixRobotAI*)o)->SetCtrlGroup(0);
                    }
                    else if(o->GetObjectType() == OBJECT_TYPE_BUILDING && ((CMatrixBuilding*)o)->GetCtrlGroup() == scan)
                    {
                        ((CMatrixBuilding*)o)->SetCtrlGroup(0);
                    }
                    else if(o->GetObjectType() == OBJECT_TYPE_FLYER && ((CMatrixFlyer*)o)->GetCtrlGroup() == scan)
                    {
                        ((CMatrixFlyer*)o)->SetCtrlGroup(0);
                    }
                    o = o->GetNextLogic();
                }

                //Если игроком была выделена группа юнитов (либо только один), то здесь из них создаётся Ctrl-группа
                if(ps->GetCurGroup())
                {
                    CMatrixGroupObject* go = ps->GetCurGroup()->m_FirstObject;

                    while(go)
                    {
                        if(go->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI)
                        {
                            ((CMatrixRobotAI*)go->GetObject())->SetCtrlGroup(scan);
                        }
                        //В формирование группы были также добавлены вертолёты (отголоски прошлого)
                        else if(go->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER)
                        {
                            ((CMatrixFlyer*)go->GetObject())->SetCtrlGroup(scan);
                        }
                        go = go->m_NextObject;
                    }
                }
                //Записываем в Ctrl-группы также и выделенные строения (не турели)
                else if(ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED)
                {
                    CMatrixBuilding* go = (CMatrixBuilding*)ps->m_ActiveObject;
                    go->SetCtrlGroup(scan);
                }
            }
            //Если Ctrl игроком зажат не был
            else
            {
                //Маркер о возможной выбранности группы юнитов
                bool prev_unselected = false;

                //Если игрок повторно выбрал цифрой уже выделенную Ctrl-группу, то центрируем камеру на данную Ctrl-группу (на первого юнита в группе)
                if(ps->GetCurGroup())
                {
                    if(m_LastScans[MAX_SCANS - 1].scan == scan && m_LastScans[MAX_SCANS - 2].scan == scan && (m_LastScans[MAX_SCANS - 1].time - m_LastScans[MAX_SCANS - 2].time) < DOUBLESCAN_TIME_DELTA)
                    {
                        CMatrixMapStatic* object = nullptr;
                        if(ps->GetCurGroup()->m_FirstObject) object = ps->GetCurGroup()->m_FirstObject->GetObject();

                        if(object && object->GetObjectType() == OBJECT_TYPE_ROBOTAI)
                        {
                            if(((CMatrixRobotAI*)object)->GetCtrlGroup() == scan)
                            {
                                //set camera to group position. out
                                g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(object->GetGeoCenter().x + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 0), object->GetGeoCenter().y + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 1)));
                                return;
                            }
                        }
                        else if(object && object->GetObjectType() == OBJECT_TYPE_FLYER)
                        {
                            if(((CMatrixFlyer*)object)->GetCtrlGroup() == scan)
                            {
                                //set camera to group position. out
                                g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(object->GetGeoCenter().x + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 0), object->GetGeoCenter().y + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 1)));
                                return;
                            }
                        }
                    }
                }
                //Аналогично центрируем камеру на строение из Ctrl-группы
                else
                {
                    //Устанавливаем маркер о том, что в данный момент игроком не выбрана никакая группа юнитов
                    prev_unselected = true;

                    if(ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED)
                    {
                        if(m_LastScans[MAX_SCANS - 1].scan == scan && m_LastScans[MAX_SCANS - 2].scan == scan && (m_LastScans[MAX_SCANS - 1].time - m_LastScans[MAX_SCANS - 2].time) < DOUBLESCAN_TIME_DELTA)
                        {
                            CMatrixBuilding* object = (CMatrixBuilding*)ps->m_ActiveObject;

                            if(((CMatrixBuilding*)object)->GetCtrlGroup() == scan)
                            {
                                //set camera to building position. out
                                g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(object->GetGeoCenter().x + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 0), object->GetGeoCenter().y + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 1)));
                                return;
                            }
                        }
                    }
                }

                //Здесь происходит выделение любой заданной ранее (но в данный момент не выделенной) Ctrl-группы
                CMatrixMapStatic* o = CMatrixMapStatic::GetFirstLogic();
                while(o)
                {
                    if(o->GetSide() == PLAYER_SIDE)
                    {
                        //Возвращаем (выбираем) группу с роботом-ами
                        if(o->IsRobotAlive() && o->AsRobot()->GetCtrlGroup() == scan)
                        {
                            if(!prev_unselected)
                            {
                                prev_unselected = true;
                                ps->SelectedGroupUnselect();
                                ps->GetCurSelGroup()->RemoveAll();
                            }
                            ps->GetCurSelGroup()->AddObject(o, -4);
                        }
                        //Возвращаем (выбираем) группу с вортолётом-ами
                        else if(o->IsFlyerControllable() && o->AsFlyer()->GetCtrlGroup() == scan)
                        {
                            if(!prev_unselected)
                            {
                                prev_unselected = true;
                                ps->SelectedGroupUnselect();
                                ps->GetCurSelGroup()->RemoveAll();
                            }
                            ps->GetCurSelGroup()->AddObject(o, -4);
                        }
                        //Возвращаем (выбираем) группу со зданием
                        else if(o->GetObjectType() == OBJECT_TYPE_BUILDING && o->AsBuilding()->GetCtrlGroup() == scan)
                        {
                            //С селектами и анселектами строений тут беда, поэтому постоянно нужно "прожимать" миллион функций
                            if(!prev_unselected) prev_unselected = true;
                            ps->Select(BUILDING, o);
                            ps->GroupsUnselectSoft();
                            ps->GetCurSelGroup()->RemoveAll();
                            ps->SetCurGroup(nullptr);
                            ps->Reselect();
                        }
                    }
                    o = o->GetNextLogic();
                }

                if(ps->GetCurSelGroup()->GetRobotsCnt() > 1 || ps->GetCurSelGroup()->GetFlyersCnt() > 1 || (ps->GetCurSelGroup()->GetFlyersCnt() + ps->GetCurSelGroup()->GetRobotsCnt()) > 1)
                {
                    ps->CreateGroupFromCurrent();
                    if(ps->GetCurGroup() && ps->GetCurGroup()->GetRobotsCnt() && ps->GetCurGroup()->GetFlyersCnt())
                    {
                        ps->GetCurGroup()->SortFlyers();
                    }
                    ps->Select(GROUP, nullptr);
                }
                else if(ps->GetCurSelGroup()->GetRobotsCnt() == 1 && !ps->GetCurSelGroup()->GetFlyersCnt())
                {
                    ps->CreateGroupFromCurrent();
                    ps->Select(ROBOT, nullptr);
                }
                else if(ps->GetCurSelGroup()->GetFlyersCnt() == 1 && !ps->GetCurSelGroup()->GetRobotsCnt())
                {
                    ps->CreateGroupFromCurrent();
                    ps->Select(FLYER, nullptr);
                }
            }
        } 
//BUTTON UNPRESSED (обработчики по отжатию указанной кнопки на клавиатуре)
    if(!down)
    {
        if(scan == KEY_LSHIFT)
        {}

#if (defined _DEBUG) &&  !(defined _RELDEBUG)
        if(scan == KEY_SPACE)
        {
            if(sel)
            {
                sel->Kill();
            }
            sel = 0;

            if(point.effect)
            {
                ((CMatrixEffectPointLight *)point.effect)->Kill(1000);
                point.Unconnect();
            }
            if(repair)
            {
                g_MatrixMap->SubEffect(DEBUG_CALL_INFO, repair);
                repair = 0;
            }
        }
#endif
    }
    
//BUTTON PRESSED
#if(defined _DEBUG) && !(defined _RELDEBUG)

        if(scan == KEY_Q)
        {
            g_MatrixMap->DumpLogic();
            g_MatrixMap->m_DI.T(L"LogicDump", L"LogicDump", 1000);
        }
        else if(/*scan == KEY_T*/false)
        {
            SRobotTemplate sb;

            sb.m_Hull.m_Module.m_nKind = RUK_HULL_FULLSTACK;
            if(ROBOT_CHASSIS_COUNT >= RUK_CHASSIS_HOVERCRAFT) sb.m_Chassis.m_nKind = RUK_CHASSIS_HOVERCRAFT;
            else sb.m_Chassis.m_nKind = RUK_CHASSIS_PNEUMATIC;

            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_MISSILES) sb.m_Weapon[RUK_WEAPON_PYLON_1].m_Module.m_nKind = RUK_WEAPON_MISSILES;
            else sb.m_Weapon[RUK_WEAPON_PYLON_1].m_Module.m_nKind = RUK_EMPTY;
            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_MISSILES) sb.m_Weapon[RUK_WEAPON_PYLON_2].m_Module.m_nKind = RUK_WEAPON_MISSILES;
            else sb.m_Weapon[RUK_WEAPON_PYLON_2].m_Module.m_nKind = RUK_EMPTY;
            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_LASER) sb.m_Weapon[RUK_WEAPON_PYLON_3].m_Module.m_nKind = RUK_WEAPON_LASER;
            else sb.m_Weapon[RUK_WEAPON_PYLON_3].m_Module.m_nKind = RUK_EMPTY;
            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_PLASMAGUN) sb.m_Weapon[RUK_WEAPON_PYLON_4].m_Module.m_nKind = RUK_WEAPON_PLASMAGUN;
            else sb.m_Weapon[RUK_WEAPON_PYLON_4].m_Module.m_nKind = RUK_EMPTY;
            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_MORTAR) sb.m_Weapon[RUK_WEAPON_PYLON_5].m_Module.m_nKind = RUK_WEAPON_MORTAR;
            else sb.m_Weapon[RUK_WEAPON_PYLON_5].m_Module.m_nKind = RUK_EMPTY;

            if(ROBOT_HEADS_COUNT >= RUK_HEAD_BLOCKER) sb.m_Head.m_nKind = RUK_HEAD_BLOCKER;
            else sb.m_Head.m_nKind = RUK_EMPTY;

            int side_id = PLAYER_SIDE;
            CMatrixSideUnit* side = g_MatrixMap->GetSideById(side_id);

            if(side->GetRobotsCnt() + side->GetRobotsInQueue() >= side->GetMaxSideRobots()) return;

            sb.m_Team = 0;
            int minr = side->m_Team[sb.m_Team].m_RobotCnt;
            for(int i = 1; i < side->m_TeamCnt; ++i)
            {
                if(side->m_Team[i].m_RobotCnt < minr)
                {
                    minr = side->m_Team[i].m_RobotCnt;
                    sb.m_Team = i;
                }
            }

            int cnt = side->GetRobotsCnt();
            CMatrixMapStatic* mps = CMatrixMapStatic::GetFirstLogic();
            while(mps)
            {
                if(mps->GetSide() == side_id && mps->GetObjectType() == OBJECT_TYPE_BUILDING && mps->AsBuilding()->IsBase())
                {
                    cnt += mps->AsBuilding()->m_BS.GetItemsCnt();
                }
                mps = mps->GetNextLogic();
            }

            if(cnt<side->GetMaxSideRobots())
            {
                CMatrixMapStatic* mps = CMatrixMapStatic::GetFirstLogic();
                while(mps)
                {
                    if(mps->GetSide() == side_id && mps->GetObjectType() == OBJECT_TYPE_BUILDING && mps->AsBuilding()->IsBase())
                    {

                        if(mps->AsBuilding()->m_BS.GetItemsCnt() < 6)
                        {
                            side->m_Constructor->SetBase(mps->AsBuilding());
                            side->m_Constructor->BuildRobotByTemplate(sb);
                            break;
                        }
                    }
                    mps = mps->GetNextLogic();
                }
            }
        }
        else if(/*scan == KEY_Y*/false)
        {
            SRobotTemplate sb;

            sb.m_Hull.m_Module.m_nKind = RUK_HULL_FULLSTACK;
            if(ROBOT_CHASSIS_COUNT >= RUK_CHASSIS_HOVERCRAFT) sb.m_Chassis.m_nKind = RUK_CHASSIS_HOVERCRAFT;
            else sb.m_Chassis.m_nKind = RUK_CHASSIS_PNEUMATIC;

            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_MISSILES) sb.m_Weapon[RUK_WEAPON_PYLON_1].m_Module.m_nKind = RUK_WEAPON_MISSILES;
            else sb.m_Weapon[RUK_WEAPON_PYLON_1].m_Module.m_nKind = RUK_EMPTY;
            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_MISSILES) sb.m_Weapon[RUK_WEAPON_PYLON_2].m_Module.m_nKind = RUK_WEAPON_MISSILES;
            else sb.m_Weapon[RUK_WEAPON_PYLON_2].m_Module.m_nKind = RUK_EMPTY;
            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_LASER) sb.m_Weapon[RUK_WEAPON_PYLON_3].m_Module.m_nKind = RUK_WEAPON_LASER;
            else sb.m_Weapon[RUK_WEAPON_PYLON_3].m_Module.m_nKind = RUK_EMPTY;
            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_PLASMAGUN) sb.m_Weapon[RUK_WEAPON_PYLON_4].m_Module.m_nKind = RUK_WEAPON_PLASMAGUN;
            else sb.m_Weapon[RUK_WEAPON_PYLON_4].m_Module.m_nKind = RUK_EMPTY;
            if(ROBOT_WEAPONS_COUNT >= RUK_WEAPON_MORTAR) sb.m_Weapon[RUK_WEAPON_PYLON_5].m_Module.m_nKind = RUK_WEAPON_MORTAR;
            else sb.m_Weapon[RUK_WEAPON_PYLON_5].m_Module.m_nKind = RUK_EMPTY;

            if(ROBOT_HEADS_COUNT >= RUK_HEAD_BLOCKER) sb.m_Head.m_nKind = RUK_HEAD_BLOCKER;
            else sb.m_Head.m_nKind = RUK_EMPTY;

            int side_id = 2 /*PLAYER_SIDE*/;
            CMatrixSideUnit* side = g_MatrixMap->GetSideById(side_id);

            if(side->GetRobotsCnt() + side->GetRobotsInQueue() >= side->GetMaxSideRobots()) return;

            sb.m_Team = 0;
            int minr = side->m_Team[sb.m_Team].m_RobotCnt;
            for(int i = 1; i < side->m_TeamCnt; ++i)
            {
                if(side->m_Team[i].m_RobotCnt<minr)
                {
                    minr=side->m_Team[i].m_RobotCnt;
                    sb.m_Team = i;
                }
            }

            int cnt = side->GetRobotsCnt();
            CMatrixMapStatic* mps = CMatrixMapStatic::GetFirstLogic();
            while(mps)
            {
                if(mps->GetSide() == side_id && mps->GetObjectType() == OBJECT_TYPE_BUILDING && mps->AsBuilding()->IsBase())
                {
                    cnt += mps->AsBuilding()->m_BS.GetItemsCnt();
                }
                mps = mps->GetNextLogic();
            }

            if(cnt<side->GetMaxSideRobots())
            {
                CMatrixMapStatic* mps = CMatrixMapStatic::GetFirstLogic();
                while(mps)
                {
                    if(mps->GetSide() == side_id && mps->GetObjectType() == OBJECT_TYPE_BUILDING && mps->AsBuilding()->IsBase())
                    {
                    
                        if(mps->AsBuilding()->m_BS.GetItemsCnt()<6)
                        {
                            side->m_Constructor->SetBase(mps->AsBuilding());
                            side->m_Constructor->BuildRobotByTemplate(sb);
                            break;
                        }
                    }
                    mps = mps->GetNextLogic();
                }
            }
        }

        if(scan > 1 && scan < 12) {}

        /*
        if(scan == KEY_R)
        {
            CMatrixSideUnit* s = g_MatrixMap->GetPlayerSide();
            if(s)
            {
                s->AddTitan(1000);
                s->AddElectronics(1000);
                s->AddEnergy(1000);
                s->AddPlasma(1000);
            }
        }

        if(scan == KEY_RBRACKET)
        {
            if(g_IFaceList) g_IFaceList->SlideFocusedInterfaceRight();
        }

        if(scan == KEY_LBRACKET)
        {
            if(g_IFaceList) g_IFaceList->SlideFocusedInterfaceLeft();
        }
        */

        if(scan == KEY_F11)
        {
            D3DResource::Dump(D3DRESTYPE_VB);
            CCache::Dump();
        }
        if(scan == KEY_DELETE)
        {
            if(path) path->Kill();
            path = 0;
        }
        if(scan == KEY_E)
        {
            g_MatrixMap->m_Minimap.AddEvent(g_MatrixMap->m_TraceStopPos.x, g_MatrixMap->m_TraceStopPos.y, 0xFF00FF00, 0xFF000000);
        }
        if(scan == KEY_F)
        {
            g_MatrixMap->ResetReinforcementsTime();
            if(IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj) && g_MatrixMap->m_TraceStopObj->IsBuilding())
            {
                g_MatrixMap->m_TraceStopObj->AsBuilding()->Reinforcements();
            }
        }
#endif
#if (defined _DEBUG) &&  !(defined _RELDEBUG)
        if(scan == KEY_F3)
        {
            static bool prev = false;
            static D3DXVECTOR3 prevp;
            D3DXVECTOR3 newp, p1, p2;

            CHelper::DestroyByGroup(9090);
            if(prev)
            {
                newp = g_MatrixMap->m_TraceStopPos;
                CHelper::Create(10000,9090)->Cone(prevp, prevp + D3DXVECTOR3(0,0,100), 30,10,0xFFFFFFFF,0xFF00FF00, 10);

                p1 = prevp;
                p2 = newp;
                prevp = newp;
            }
            else
            {
                prevp = g_MatrixMap->m_TraceStopPos;
                prev = true;
                CHelper::Create(10000, 9090)->Cone(prevp, prevp + D3DXVECTOR3(0, 0, 100), 30, 10, 0xFFFFFFFF, 0xFF00FF00, 10);
                return;
            }

            D3DXVECTOR3 dir(p2-p1), side;
            float len = D3DXVec3Length(&dir);
            float dd = len / 3;
            dir *= 1.0f / len;
            D3DXVec3Cross(&side, &dir, &D3DXVECTOR3(0, 0, 1));
            D3DXVec3Normalize(&side, &side);

            D3DXVECTOR3 to[128];
            int curi_t = 0;

            for(int idx = 1; idx < 3; ++idx)
            {
                D3DXVECTOR3 p = p1 + dir * float(idx * dd) + side * FSRND(len) * 0.2f;
                to[curi_t++] = p;

                CHelper::Create(10000, 9090)->Cone(p, p + D3DXVECTOR3(0, 0, 90), 30, 35, 0xFFFFFFFF, 0xFFFF0000, 10);
            }

            to[curi_t++] = p2;
            CHelper::Create(10000, 9090)->Cone(p2, p2 + D3DXVECTOR3(0, 0, 90), 30, 35, 0xFFFFFFFF, 0xFFFF0000, 10);
        }
        {
            //static int from = 0;
            //static int to = 100000;
            //static D3DXVECTOR3 ptfrom;

            //bool fShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0x8000;

            if(scan == KEY_F7)
            {
                //ptfrom = g_MatrixMap->m_Camera.GetFrustumCenter();
                //g_MatrixMap->CalcVisTemp(from, to, ptfrom);
                g_MatrixMap->CalcVis();

            }
            //if(scan == KEY_F8)
            //{
            //    if(fShift) --to; else ++to;
            //    if(to < from) to = from;

            //    CDText::T("from", CStr(from));
            //    CDText::T("to", CStr(to));

            //    g_MatrixMap->CalcVisTemp(from, to, ptfrom);
            //}
            //if(scan == KEY_F6)
            //{
            //    if(fShift) --from; else ++from;
            //    if(from > to) from = to;
            //    if(from < 0) from = 0;

            //    CDText::T("from", CStr(from));
            //    CDText::T("to", CStr(to));

            //    g_MatrixMap->CalcVisTemp(from, to, ptfrom);
            //}
        }
        //if(scan == KEY_F8)
        //{
        //    D3DXVECTOR3 p;
        //    g_MatrixMap->TraceLand(&p, g_MatrixMap->m_Camera.GetFrustumCenter(), g_MatrixMap->m_MouseDir);
        //    CHelper::Create(10000,888)->Line(g_MatrixMap->m_Camera.GetFrustumCenter(), p);
        //}
        if(scan == KEY_F6)
        {
            const D3DXVECTOR3 &cam = g_MatrixMap->m_Camera.GetFrustumCenter();
            int gx = TruncFloat(cam.x / (GLOBAL_SCALE * MAP_GROUP_SIZE));
            int gy = TruncFloat(cam.y / (GLOBAL_SCALE * MAP_GROUP_SIZE));

            if(gx >= 0 && gx < g_MatrixMap->m_GroupSize.x && gy >= 0 && gy < g_MatrixMap->m_GroupSize.y)
            {
                SGroupVisibility *gv = g_MatrixMap->m_GroupVis + gx + gy * g_MatrixMap->m_GroupSize.x;

                int level = TruncFloat((cam.z - gv->z_from) / GLOBAL_SCALE);

                if(level >=0 && level < gv->levels_cnt)
                {
                    int n = gv->levels[level];

                    CHelper::DestroyByGroup(345);
                    for(int i = 0; i < n; ++i)
                    {
                        PCMatrixMapGroup g = gv->vis[i];

                        D3DXVECTOR3 f((g->GetPos0().x + g->GetPos1().x) * 0.5f, (g->GetPos0().y + g->GetPos1().y) * 0.5f, 0);
                        f.z = g_MatrixMap->GetZ(f.x, f.y);

                        CHelper::Create(10000, 345)->Cone(f, f + D3DXVECTOR3(0,0,100), 30, 30, 0xFF00FF00, 0xFF0000FF, 20);
                    }
                }
            }
        }

        if(scan == KEY_F5 && IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj))
        {
            if(g_MatrixMap->m_TraceStopObj->IsRobot())
            {
                g_MatrixMap->m_TraceStopObj->AsRobot()->m_PosX = 4110.0f;
                g_MatrixMap->m_TraceStopObj->AsRobot()->m_PosY = 2295.0f;
                g_MatrixMap->m_TraceStopObj->RChange(MR_Matrix);
                g_MatrixMap->m_TraceStopObj->GetResources(MR_Matrix);
                g_MatrixMap->m_TraceStopObj->JoinToGroup();

                CPoint sss(Float2Int(4110.0f / GLOBAL_SCALE_MOVE), Float2Int(2295.0f / GLOBAL_SCALE_MOVE));

                g_MatrixMap->GetPlayerSide()->PGOrderAttack(g_MatrixMap->GetPlayerSide()->RobotToLogicGroup(g_MatrixMap->m_TraceStopObj->AsRobot()), sss, nullptr);
            }

        }
        if(scan == KEY_K && IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj))
        {
            CMatrixMapStatic* f = g_MatrixMap->m_TraceStopObj;
            if(f->GetObjectType() == OBJECT_TYPE_FLYER || f->GetObjectType() == OBJECT_TYPE_ROBOTAI)
            {
                CMatrixMapStatic::SortBegin();
                
                if(f->IsFlyer()) ((CMatrixFlyer*)f)->SetHitpoint(1);

                f->TakingDamage(WEAPON_FLAMETHROWER, f->GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, nullptr);
            }
            else if(f->GetObjectType() == OBJECT_TYPE_MAPOBJECT)
            {
                f->TakingDamage(WEAPON_MORTAR, f->GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, nullptr);
            }
            else if(f->GetObjectType() == OBJECT_TYPE_BUILDING)
            {
                bool killed = f->TakingDamage(WEAPON_BOMB, f->GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, nullptr);
                if(!killed) killed = f->TakingDamage(WEAPON_BOMB, f->GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, nullptr);
                if(!killed) killed = f->TakingDamage(WEAPON_BOMB, f->GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, nullptr);
                if(!killed) killed = f->TakingDamage(WEAPON_BOMB, f->GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, nullptr);
                if(!killed) killed = f->TakingDamage(WEAPON_BOMB, f->GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, nullptr);
            }
        }
        if(scan == KEY_SPACE)
        {
            D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
            pos.z += 10.0f;

            //if(0)
            if(point.effect == nullptr)
            {
                CMatrixEffect::CreatePointLight(&point, pos + D3DXVECTOR3(0, 0, 0), 150, 0xFF0033FF, false);
            }
            
            //if(0)
            if(!sel)
            {
                sel = (CMatrixEffectSelection*)CMatrixEffect::CreateSelection(pos, 20);
                g_MatrixMap->AddEffect(sel);
            }

            //if(0)
            if(!repair)
            {
                repair = (CMatrixEffectRepair*)CMatrixEffect::CreateRepair(pos, D3DXVECTOR3(1, 0, 0), 200, nullptr, (ESpriteTextureSort)g_Config.m_WeaponsConsts[weapon_type].sprite_set[0].sprites_num[0]);
                g_MatrixMap->AddEffect(repair);
            }
		}
#endif
    }
}

void CFormMatrixGame::SystemEvent(ESysEvent se)
{
DTRACE();

    if(se == SYSEV_DEACTIVATING)
    {
        if(FLAG(g_MatrixMap->m_Flags, MMFLAG_VIDEO_RESOURCES_READY))
        {
            g_MatrixMap->ReleasePoolDefaultResources();
        }
    }

    if(se == SYSEV_ACTIVATED)
    {
        if(FLAG(g_Flags, GFLAG_FULLSCREEN))
        {
            RECT r;
            GetWindowRect(g_Wnd, &r);
            ClipCursor(&r);
        }
    }
}