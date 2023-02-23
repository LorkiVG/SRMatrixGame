// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../stdafx.h"
#include "CConstructor.h"
#include "CIFaceButton.h"
#include "../MatrixObjectBuilding.hpp"
#include "../MatrixRenderPipeline.hpp"
#include "CInterface.h"
#include "CCounter.h"
#include "CIFaceMenu.h"
#include "CHistory.h"
#include "../Effects/MatrixEffectWeapon.hpp"
#include "../MatrixConfig.hpp"

CConstructor::CConstructor()
{
	m_ViewPosX          = 0; 
	m_ViewPosY          = 0;
	m_ViewWidthX        = 0;
	m_ViewHeightY       = 0;
	m_RobotPosX         = 0;
	m_RobotPosY         = 0;
	m_Side              = 0;
	m_nModuleCnt        = 0;
    m_ShadowType        = SHADOW_OFF;
	m_ShadowSize        = 0;
	m_nPos              = 0;

    m_Robot            = nullptr;
	m_Build            = nullptr;
	m_Base             = nullptr;
	
	m_Robot            = HNew(g_MatrixHeap) CMatrixRobotAI;
	m_Robot->m_PosX    = 0;
	m_Robot->m_PosY    = 0;
	m_Robot->m_Side    = 0; 

    m_Robot->m_ShadowType = SHADOW_OFF;
    m_Robot->m_ShadowSize = 128;

	m_Robot->m_ChassisForward     = D3DXVECTOR3(1, 0, 0);
    m_Robot->m_CurrState   = ROBOT_EMBRYO;

    m_NewBorn              = HNew(g_MatrixHeap) SNewBorn;
}

CConstructor::~CConstructor()
{
    if(m_NewBorn) HDelete(SNewBorn, m_NewBorn, g_MatrixHeap);
    if(m_Robot)
    {
		ASSERT(g_MatrixHeap);
		HDelete(CMatrixRobotAI, m_Robot, g_MatrixHeap);
		m_Robot = nullptr;
	}
}
//Данная функция не используется (вероятно, подразумевала моментальную постройку робота без ожидания)
SNewBorn* CConstructor::ProduceRobot(void*)
{
DTRACE();

	if(!m_Base || m_Base->m_State != BASE_CLOSED) return nullptr;

	if(m_nModuleCnt)
    {
        //m_Base->m_BusyFlag.SetBusy(m_Build); //!SETBUSY
        m_Build = g_MatrixMap->StaticAdd<CMatrixRobotAI>();
		
	    if(m_Head.m_nKind) m_Build->ModuleInsert(0, MRT_HEAD, m_Head.m_nKind);

        auto hull_num = m_Hull.m_Module.m_nKind;
        if(hull_num)
        {
            for(int i = 0; i < g_Config.m_RobotHullsConsts[hull_num].weapon_pylon_data.size(); ++i)
            {
                int constructor_slot_num = g_Config.m_RobotHullsConsts[hull_num].weapon_pylon_data[i].constructor_slot_num;
                if(m_Weapon[constructor_slot_num].m_Module.m_nKind)
                {
                    m_Build->WeaponInsert(0, MRT_WEAPON, m_Weapon[constructor_slot_num].m_Module.m_nKind, hull_num, i);
                }
            }

		    m_Build->ModuleInsert(0, MRT_HULL, hull_num);
	    }

        if(m_Chassis.m_nKind)
        {
            m_Build->ModuleInsert(0, MRT_CHASSIS, m_Chassis.m_nKind);

            //Для "читерного" уровня проходимости шасси коллизию не считаем
            if(g_Config.m_RobotChassisConsts[m_Chassis.m_nKind].passability >= 5)
            {
                m_Build->SetNoObjectsCollision(true);
                m_Build->SetNoObstaclesCollision(true);
            }
        }

        m_Build->m_ShadowType = g_Config.m_RobotShadow;
        m_Build->m_ShadowSize = 128;


		m_Build->m_Side = m_Side;
		m_Build->RobotWeaponInit();
		m_Build->m_PosX = m_Base->m_Pos.x;
		m_Build->m_PosY = m_Base->m_Pos.y;

		m_Build->CalcRobotParams();
        switch(m_Base->m_Angle)
        {
            case 0:
            {
                m_Build->m_ChassisForward = D3DXVECTOR3(0, 1, 0);
                m_Build->m_ChassisCamForward = D3DXVECTOR3(0, 1, 0);
                break;
            }
            case 1:
            {
                m_Build->m_ChassisForward = D3DXVECTOR3(-1, 0, 0);
                m_Build->m_ChassisCamForward = D3DXVECTOR3(-1, 0, 0);
                break;
            }
            case 2:
            {
                m_Build->m_ChassisForward = D3DXVECTOR3(0, -1, 0);
                m_Build->m_ChassisCamForward = D3DXVECTOR3(0, -1, 0);
                break;
            }
            case 3:
            {
                m_Build->m_ChassisForward = D3DXVECTOR3(1, 0, 0);
                m_Build->m_ChassisCamForward = D3DXVECTOR3(1, 0, 0);
                break;
            }
        }

        m_Build->m_HullForward = m_Build->m_ChassisForward;
        
        //robot sozdan
        m_Build->RobotSpawn(m_Base);
        m_Build->JoinToGroup();

        m_NewBorn->m_Robot = m_Build;

        CMatrixSideUnit* si=g_MatrixMap->GetPlayerSide();

        if(si->GetTeam(0)->m_RobotCnt < si->GetTeam(1)->m_RobotCnt && si->GetTeam(0)->m_RobotCnt < si->GetTeam(2)->m_RobotCnt) m_NewBorn->m_Team = 0;
        else if(si->GetTeam(1)->m_RobotCnt < si->GetTeam(0)->m_RobotCnt && si->GetTeam(1)->m_RobotCnt < si->GetTeam(2)->m_RobotCnt) m_NewBorn->m_Team = 1;
        else if(si->GetTeam(2)->m_RobotCnt < si->GetTeam(0)->m_RobotCnt && si->GetTeam(2)->m_RobotCnt < si->GetTeam(1)->m_RobotCnt) m_NewBorn->m_Team = 2;
        else m_NewBorn->m_Team = g_MatrixMap->Rnd(0, 2);

        m_NewBorn->m_Team = 0;

        //Team
        m_Build->SetTeam(m_NewBorn->m_Team);
        ResetConstruction();

        m_Build->CreateTextures();
        return m_NewBorn;
	}

    return nullptr;
}

//Теперь вместо функции выше используется эта, с предварительной постройкой и последующим добавлением робота в очередь постройки
void CConstructor::AddRobotToBuildingQueue(
    void* pObject,
    int team
)
{
    bool crazy_bot = false;
    if(team < 0)
    {
        team = 0;
        crazy_bot = true;
    }

    //if(!m_Base || m_Base->m_State != BASE_CLOSED) return;
    if(!m_Base || m_Base->m_BS.GetItemsCnt() >= BUILDING_QUEUE_LIMIT) return;
	if(m_nModuleCnt)
    {
        //m_Base->m_BusyFlag.SetBusy(m_Build);
        m_Build = HNew(g_MatrixHeap) CMatrixRobotAI;

        if(crazy_bot) m_Build->MarkCrazy();

	    if(m_Head.m_nKind) m_Build->ModuleInsert(0, MRT_HEAD, m_Head.m_nKind);

        auto hull_num = m_Hull.m_Module.m_nKind;
        if(hull_num)
        {
            for(int i = 0; i < g_Config.m_RobotHullsConsts[hull_num].weapon_pylon_data.size(); ++i)
            {
                int constructor_slot_num = g_Config.m_RobotHullsConsts[hull_num].weapon_pylon_data[i].constructor_slot_num;
                if(m_Weapon[constructor_slot_num].m_Module.m_nKind)
                {
                    m_Build->WeaponInsert(0, MRT_WEAPON, m_Weapon[constructor_slot_num].m_Module.m_nKind, hull_num, i);
                }
            }

		    m_Build->ModuleInsert(0, MRT_HULL, (ERobotModuleKind)hull_num);
	    }

        if(m_Chassis.m_nKind)
        {
            m_Build->ModuleInsert(0, MRT_CHASSIS, m_Chassis.m_nKind);

            //Для "читерного" уровня проходимости шасси коллизию не считаем
            if(g_Config.m_RobotChassisConsts[m_Chassis.m_nKind].passability >= 5)
            {
                m_Build->SetNoObjectsCollision(true);
                m_Build->SetNoObstaclesCollision(true);
            }
        }

        m_Build->m_ShadowType = g_Config.m_RobotShadow;
        m_Build->m_ShadowSize = 128;

		m_Build->m_Side = m_Side;
		m_Build->RobotWeaponInit();

		m_Build->m_PosX = m_Base->m_Pos.x;
		m_Build->m_PosY = m_Base->m_Pos.y;

		m_Build->CalcRobotParams();
        switch(m_Base->m_Angle)
        {
            case 0:
            {
                m_Build->m_ChassisForward = D3DXVECTOR3(0, 1, 0);
                m_Build->m_ChassisCamForward = D3DXVECTOR3(0, 1, 0);
                break;
            }
            case 1:
            {
                m_Build->m_ChassisForward = D3DXVECTOR3(-1, 0, 0);
                m_Build->m_ChassisCamForward = D3DXVECTOR3(-1, 0, 0);
                break;
            }
            case 2:
            {
                m_Build->m_ChassisForward = D3DXVECTOR3(0, -1, 0);
                m_Build->m_ChassisCamForward = D3DXVECTOR3(0, -1, 0);
                break;
            }
            case 3:
            {
                m_Build->m_ChassisForward = D3DXVECTOR3(1, 0, 0);
                m_Build->m_ChassisCamForward = D3DXVECTOR3(1, 0, 0);
                break;
            }
        }

        m_Build->m_HullForward = m_Build->m_ChassisForward;
        
        if(m_Base->m_Side == PLAYER_SIDE)
        {
            CMatrixSideUnit* si = g_MatrixMap->GetPlayerSide();
            int cfg_num = si->m_ConstructPanel->m_CurrentConfig;
        }

        m_Build->SetTeam(team);

        //robot sozdan

//#ifdef _DEBUG
//        RESETFLAG(g_Flags, SETBIT(22));
//#endif

        m_Build->GetResources(MR_Graph);

        //if(!FLAG(g_Flags, SETBIT(22))) while(true);

        if(m_Base->m_Side == PLAYER_SIDE) m_Build->CreateTextures();
        m_Build->SetBase(m_Base);
        GetConstructionName((CMatrixRobotAI*)m_Build);        
        m_Base->m_BS.AddItem(m_Build);
	}
}

//Функция запуска постройки робота для стороны игрока
void __stdcall CConstructor::RemoteBuild(void* pObj)
{
DTRACE();

    if(m_Base->m_Side != PLAYER_SIDE) return;

    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
    g_ConfigHistory->AddConfig(&player_side->m_ConstructPanel->m_Configs[cfg_num]);

    for(int i = 0; i < g_IFaceList->m_RCountControl->GetCounter(); ++i)
    {
        AddRobotToBuildingQueue(pObj);
    }

    int res[MAX_RESOURCES];
    GetConstructionPrice(res);
    player_side->AddResourceAmount(TITAN, -res[TITAN] * g_IFaceList->m_RCountControl->GetCounter());
    player_side->AddResourceAmount(ELECTRONICS, -res[ELECTRONICS] * g_IFaceList->m_RCountControl->GetCounter());
    player_side->AddResourceAmount(ENERGY, -res[ENERGY] * g_IFaceList->m_RCountControl->GetCounter());
    player_side->AddResourceAmount(PLASMA, -res[PLASMA] * g_IFaceList->m_RCountControl->GetCounter());

    if(player_side->m_ConstructPanel) player_side->m_ConstructPanel->ResetGroupNClose();

    g_IFaceList->m_RCountControl->Reset();
    g_IFaceList->m_RCountControl->CheckUp();
}

void CConstructor::BeforeRender(void)
{
	//static float za = 0;
    //SinCos(za, &m_Robot->m_ChassisForward.x, &m_Robot->m_ChassisForward.y);
    m_Robot->m_HullForward = m_Robot->m_ChassisForward;
    m_Robot->m_ChassisForward.z = 0;
	//za += 0.001f;
    //if(za > M_PI_MUL(2)) za -= M_PI_MUL(2);

    m_Robot->RChange(MR_Matrix | MR_Graph);
    m_Robot->BeforeDraw();
}

void CConstructor::Render(void)
{
    D3DXMATRIX matWorld, matView, matProj, RotMat;
	D3DVIEWPORT9 ViewPort;

	ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));

	ViewPort.X      = (dword)m_ViewPosX;
	ViewPort.Y      = (dword)m_ViewPosY;
	ViewPort.Width  = m_ViewWidthX;
	ViewPort.Height = m_ViewHeightY;

	ViewPort.MinZ   = 0.0f;
	ViewPort.MaxZ   = 1.0f;
//View robot
	
	ASSERT_DX(g_D3DD->SetViewport(&ViewPort));

    D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r  = 1.0f;
	light.Diffuse.g  = 1.0f;
	light.Diffuse.b  = 1.0f;
	light.Ambient.r  = 0.0f;
	light.Ambient.g  = 0.0f;
	light.Ambient.b  = 0.0f;
	light.Specular.r = 1.0f;
	light.Specular.g = 1.0f;
	light.Specular.b = 1.0f;

    //static float za = 0;
    //if(g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_N) {za -= 0.1f; g_MatrixMap->m_KeyDown = false;}
    //if(g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_M) {za += 0.1f; g_MatrixMap->m_KeyDown = false;}
    //float s, c;
    //SinCos(za, &s, &c);

	//light.Direction	= D3DXVECTOR3(s, c, 0);
    light.Direction	= D3DXVECTOR3(-0.82242596f, 0.56887215f, 0);
    //D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, (D3DXVECTOR3*)&light.Direction);
	ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, 0xFF808080));

    
    if(FLAG(g_Flags,GFLAG_STENCILAVAILABLE)) //Не убирать фигурные скобки, либо удалить точки с запятой после ASSERT_DX
    {
	    ASSERT_DX(g_D3DD->Clear(0, nullptr, D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0));
    }
    else
    {
	    ASSERT_DX(g_D3DD->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0));
    }

    float h = m_Robot->GetChassisHeight();

	D3DXMatrixIdentity(&matWorld);
    D3DXMatrixLookAtLH(&matView,
        &D3DXVECTOR3(80, -30, h + 5),
        &D3DXVECTOR3(0.0f, 0.0f, h),
        &D3DXVECTOR3(0.0f, 0.0f, 1.0f)
    );
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, float(m_ViewWidthX)/float(m_ViewHeightY) /* float(g_ScreenX) / float(g_ScreenX) */ /* 1.0f */, 1.0f, 300.0f);
	ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &matWorld));
	ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW, &matView));
	ASSERT_DX(g_D3DD->SetTransform(D3DTS_PROJECTION, &matProj));

	//Render
    D3DXMATRIX imatView;
    D3DXMatrixInverse(&imatView, nullptr, &matView);

    m_Robot->SetInterfaceDraw(true);
    g_MatrixMap->m_Camera.SetDrawNowParams(imatView,  D3DXVECTOR3(80, -30, h + 5));
    CVectorObject::DrawBegin();
	if(m_nModuleCnt) m_Robot->Draw();
	CVectorObject::DrawEnd();

//Return old
    g_MatrixMap->m_Camera.RestoreCameraParams();

	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r  = GetColorR(g_MatrixMap->m_LightMainColorObj);
	light.Diffuse.g  = GetColorG(g_MatrixMap->m_LightMainColorObj);
	light.Diffuse.b  = GetColorB(g_MatrixMap->m_LightMainColorObj);
	light.Ambient.r  = 0.0f;
	light.Ambient.g  = 0.0f;
	light.Ambient.b  = 0.0f;
	light.Specular.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
	light.Specular.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
	light.Specular.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
	light.Direction	= g_MatrixMap->m_LightMain;
	ASSERT_DX(g_D3DD->SetLight(0,&light));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_AmbientColorObj));
}

void __stdcall CConstructor::RemoteOperateModule(void* pObj)
{
	CIFaceButton* pButton = (CIFaceButton*)pObj;
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    
    if(!pButton || !player_side) return;

    CIFaceElement* super_el = player_side->m_ConstructPanel->m_FocusedElement;

    ERobotModuleType type = ERobotModuleType(Float2Int(pButton->m_Param1));
    ERobotModuleKind kind = ERobotModuleKind(Float2Int(pButton->m_Param2));
    int pylon = 0;

    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
    
    if(pButton->m_strName == IF_BASE_WEAPON_PYLON_1) pylon = RUK_WEAPON_PYLON_1;
    else if(pButton->m_strName == IF_BASE_WEAPON_PYLON_2) pylon = RUK_WEAPON_PYLON_2;
    else if(pButton->m_strName == IF_BASE_WEAPON_PYLON_3) pylon = RUK_WEAPON_PYLON_3;
    else if(pButton->m_strName == IF_BASE_WEAPON_PYLON_4) pylon = RUK_WEAPON_PYLON_4;
    else if(pButton->m_strName == IF_BASE_WEAPON_PYLON_5) pylon = RUK_WEAPON_PYLON_5;

    int cur_hull_num = player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Module.m_nKind;
    int model_pylon = 0;
    switch(type)
    {
        case MRT_HULL:
        {
            int next = cur_hull_num;
            do
            {
                if(next == ROBOT_HULLS_COUNT) next = 1;
                else ++next;

            } while(!g_Config.m_RobotHullsConsts[next].for_player_side);
            
            kind = (ERobotModuleKind)next;
            break;
        }
        case MRT_CHASSIS:
        {
            int next = player_side->m_ConstructPanel->m_Configs[cfg_num].m_Chassis.m_nKind;
            do
            {
                if(next == ROBOT_CHASSIS_COUNT) next = 1;
                else ++next;

            } while(!g_Config.m_RobotChassisConsts[next].for_player_side);
            
            kind = (ERobotModuleKind)next;
            break;
        }
        case MRT_HEAD:
        {
            int next = player_side->m_ConstructPanel->m_Configs[cfg_num].m_Head.m_nKind;
            do
            {
                if(next == ROBOT_HEADS_COUNT) next = 0;
                else ++next;

            } while(next && !g_Config.m_RobotHeadsConsts[next].for_player_side);
            
            kind = (ERobotModuleKind)next;
            break;
        }
        case MRT_WEAPON:
        {
            //Сперва необходимо определить реальный номер оружейного пилона на модели, т.к. их там может быть меньше максимального числа
            for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data.size(); ++i)
            {
                if(g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data[i].constructor_slot_num == pylon)
                {
                    model_pylon = i;
                    break;
                }
            }

            int next = player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pylon].m_nKind;
            do
            {
                if(next == ROBOT_WEAPONS_COUNT) next = 0;
                else ++next;

            } while(next && (!g_Config.m_RobotWeaponsConsts[next].for_player_side || !g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data[model_pylon].fit_weapon.test(next)));

            kind = (ERobotModuleKind)next;
            break;
        }
    }

    ReplaceRobotModule(type, kind, pylon, model_pylon);
}

//Меняет модули робота в конструкторе при переключении их игроком (раньше функция называлась SuperDjeans)
void CConstructor::ReplaceRobotModule(
    ERobotModuleType type,
    ERobotModuleKind kind,
    int pylon,
    int model_pylon,
    bool ld_from_history
)
{
    if(g_IFaceList && g_IFaceList->m_RCountControl) g_IFaceList->m_RCountControl->Reset();

    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
    SRobotConfig* old_cfg = nullptr;

    if(type == MRT_HEAD)
    {
        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Head.m_nKind = kind; //Фактическая замена модуля
        CInterface::CopyElements(g_IFaceList->m_Head[kind], g_IFaceList->m_HeadPilon); //Обновление кнопки замены модуля
    }
    else if(type == MRT_HULL)
    {
        //Если чертежей роботов ещё не существует, создаём новый
        if(!ld_from_history)
        {
            old_cfg = HNew(g_MatrixHeap) SRobotConfig;
            memcpy(old_cfg, &player_side->m_ConstructPanel->m_Configs[cfg_num], sizeof(SRobotConfig));

            CInterface::CopyElements(g_IFaceList->m_Head[0], g_IFaceList->m_HeadPilon);
            ReplaceRobotModule(MRT_HEAD, ERobotModuleKind(0), 0);
        }

        player_side->m_ConstructPanel->ResetWeapon();
        g_IFaceList->WeaponPilonsInit();

        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Module.m_nKind = kind;
        CInterface::CopyElements(g_IFaceList->m_Hull[kind - 1], g_IFaceList->m_ArmorPilon);
    }
    else if(type == MRT_CHASSIS)
    {
        //Загружаем номер шасси в текущий конфиг конструктора, и добавляем в иконку слота шасси иконку выбранного шасси
        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Chassis.m_nKind = kind;
        CInterface::CopyElements(g_IFaceList->m_Chassis[kind - 1], g_IFaceList->m_ChassisPilon);
    }
    else if(type == MRT_WEAPON)
    {
        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pylon].m_nKind = kind;

        m_Weapon[pylon].m_Module.m_nType = MRT_WEAPON;
        m_Weapon[pylon].m_Module.m_nKind = kind;
        m_Weapon[pylon].m_ConstructorPylonNum = pylon;

        CInterface::CopyElements(g_IFaceList->m_Weapon[kind], g_IFaceList->m_WeaponPilon[pylon]);
        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_WeaponPilon[pylon]);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_WeaponPilon[pylon]);

        InsertModules();
        GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());

        g_IFaceList->CreateSummPrice();
        g_IFaceList->m_RCountControl->CheckUp();

        return;
    }

    //Меняем модель робота, стыкуем новый модуль
    OperateModule(type, kind);
    //Собираем роботу название (название корпуса + название шасси + название головы и через тире рандомная цифра)
    //Названия модулей задаются в RobotNames в data.txt
    GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());

    if(type == MRT_HEAD)
    {
        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_HeadPilon);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_HeadPilon);
    }
    else if(type == MRT_HULL)
    {
        if(old_cfg)
        {
            //Переносим всё подходящее оружие со старого корпуса на новый после смены корпуса
            for(int i = 0; i < g_Config.m_RobotHullsConsts[kind].weapon_pylon_data.size(); ++i)
            {
                int const_slot = g_Config.m_RobotHullsConsts[kind].weapon_pylon_data[i].constructor_slot_num;
                auto gun = old_cfg->m_Weapon[const_slot].m_nKind;
                if(!gun) continue;
                if(!g_Config.m_RobotHullsConsts[kind].weapon_pylon_data[i].fit_weapon.test(gun)) continue; //Если это оружие не может быть установлено в данный пилон в новом корпусе
                ReplaceRobotModule(MRT_WEAPON, gun, const_slot, i);
            }

            ReplaceRobotModule(MRT_HEAD, old_cfg->m_Head.m_nKind, 0);
            HDelete(SRobotConfig, old_cfg, g_MatrixHeap);
            old_cfg = nullptr;
        }

        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_ArmorPilon);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_ArmorPilon);
    }
    else if(type == MRT_CHASSIS)
    {
        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_ChassisPilon);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_ChassisPilon);
    }

    g_IFaceList->CreateSummPrice();
    g_IFaceList->m_RCountControl->CheckUp();
}

//Также меняет модуль робота в конструкторе, но делает это при быстром просмотре модулей из выпадающего списка, при этом полного переписывания шаблона не происходит (раньше называлась Djeans007)
void CConstructor::TempReplaceRobotModuleFromMenu(
    ERobotModuleType type,
    ERobotModuleKind kind,
    int pylon,
    int model_pylon
)
{
    g_IFaceList->m_RCountControl->Reset();
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;

    if(type == MRT_WEAPON)
    {
        m_Weapon[pylon].m_Module.m_nType = MRT_WEAPON;
        m_Weapon[pylon].m_Module.m_nKind = kind;
        m_Weapon[pylon].m_ConstructorPylonNum = pylon;
       
        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_WEAPON, kind);

        InsertModules();
        GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());
        player_side->m_ConstructPanel->m_FocusedElement = g_IFaceList->m_Weapon[kind];

        g_IFaceList->CreateSummPrice();
        g_IFaceList->m_RCountControl->CheckUp();
        return;
    }

    //Меняем модель робота, стыкуем новый модуль
    OperateModule(type, kind);
    //Обновляем счётчик суммарных ресурсов на постройку робота внизу окна конструктора
    g_IFaceList->CreateSummPrice();
    //Собираем роботу название (название корпуса + название шасси + название головы и через тире рандомная цифра)
    //Названия модулей задаются в RobotNames в data.txt
    GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());
    
    if(type == MRT_HEAD)
    {
        player_side->m_ConstructPanel->m_FocusedElement =  g_IFaceList->m_Head[kind];
        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_HEAD, kind);
    }
    else if(type == MRT_HULL)
    {
        //Временно "примеряем" всё подходящее оружие с текущего корпуса на выделенный в выпадающем меню
        for(int i = 0; i < g_Config.m_RobotHullsConsts[kind].weapon_pylon_data.size(); ++i)
        {
            int const_slot = g_Config.m_RobotHullsConsts[kind].weapon_pylon_data[i].constructor_slot_num;
            auto gun = player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[const_slot].m_nKind;
            if(!gun) continue;
            if(!g_Config.m_RobotHullsConsts[kind].weapon_pylon_data[i].fit_weapon.test(gun)) continue; //Если это оружие не может быть установлено в данный пилон в выделенном корпусе
            TempReplaceRobotModuleFromMenu(MRT_WEAPON, gun, const_slot, i);
        }

        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_HULL, kind);
        player_side->m_ConstructPanel->m_FocusedElement = g_IFaceList->m_Hull[kind - 1];
    }
    else if(type == MRT_CHASSIS)
    {
        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_CHASSIS, kind);
        player_side->m_ConstructPanel->m_FocusedElement = g_IFaceList->m_Chassis[kind - 1];
    }

    g_IFaceList->m_RCountControl->CheckUp();
}


void CConstructor::OperateModule(ERobotModuleType type, ERobotModuleKind kind, int pylon)
{
    switch(type)
    {
        case MRT_HULL:
        {
            ZeroMemory(m_Weapon, RUK_WEAPON_PYLONS_COUNT * sizeof(SWeaponModule));
            m_Hull.m_Module.m_nType = MRT_HULL;
            m_Hull.m_Module.m_nKind = kind;
            m_Hull.m_MaxWeaponsCnt = g_Config.m_RobotHullsConsts[kind].weapon_pylon_data.size();
            break;
        }
        case MRT_CHASSIS:
        {
            m_Chassis.m_nType = MRT_CHASSIS;
            m_Chassis.m_nKind = kind;
            break;
        }
        case MRT_HEAD:
        {
            m_Head.m_nType = MRT_HEAD;
            m_Head.m_nKind = kind;
            break;
        }
        case MRT_WEAPON:
        {
            m_Weapon[pylon].m_Module.m_nType = MRT_WEAPON;
            m_Weapon[pylon].m_Module.m_nKind = kind;
            m_Weapon[pylon].m_ConstructorPylonNum = pylon;
            break;
        }

        default: ERROR_S2(L"OperateModule function: Unknown module type was received, type number ", CWStr(type)); break;
    }
    
    //Выставляет маркер, что текущий вызов данной функции был сделан игроком
    //Только вот нахера это?
    /*
    int we_are = 0;
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
    
    if(player_side && g_MatrixMap->GetPlayerSide()->m_Constructor == this)
    {
        we_are = 1;
    }
    */

    m_nModuleCnt = 0;
    if(m_Hull.m_Module.m_nKind) ++m_nModuleCnt;
    if(m_Chassis.m_nKind) ++m_nModuleCnt;
    if(m_Head.m_nKind) ++m_nModuleCnt;

    for(int nC = 0; nC < RUK_WEAPON_PYLONS_COUNT; ++nC)
    {
        if (m_Weapon[nC].m_Module.m_nKind) ++m_nModuleCnt;
	}

	if(m_nModuleCnt) InsertModules();
}

//Полная пересборка модели робота по выставленному ранее шаблону
void CConstructor::InsertModules()
{
	m_Robot->ModuleClear();
	if(m_Head.m_nKind)
    {
		m_Robot->ModuleInsert(0, MRT_HEAD, m_Head.m_nKind);
	}

    int hull_num = m_Hull.m_Module.m_nKind;
    if(hull_num)
    {
        for(int i = 0; i < g_Config.m_RobotHullsConsts[hull_num].weapon_pylon_data.size(); ++i)
        {
            int constructor_slot_num = g_Config.m_RobotHullsConsts[hull_num].weapon_pylon_data[i].constructor_slot_num;
		    if(m_Weapon[constructor_slot_num].m_Module.m_nKind)
            {
                m_Robot->WeaponInsert(0, MRT_WEAPON, m_Weapon[constructor_slot_num].m_Module.m_nKind, hull_num, i);
		    }
	    }

		m_Robot->ModuleInsert(0, MRT_HULL, m_Hull.m_Module.m_nKind);
	}

	if(m_Chassis.m_nKind)
    {
		m_Robot->ModuleInsert(0, MRT_CHASSIS, m_Chassis.m_nKind);
		D3DXMatrixIdentity(&m_Robot->m_Module[0].m_Matrix);
	}

	m_Robot->RChange(MR_Matrix | MR_Graph);
}

void CConstructor::ResetConstruction()
{
	ZeroMemory(&m_Hull, sizeof(SHullModule));
	ZeroMemory(&m_Head, sizeof(SModule));
	ZeroMemory(m_Weapon, RUK_WEAPON_PYLONS_COUNT * sizeof(SModule));
	ZeroMemory(&m_Chassis, sizeof(SModule));
	
	m_nModuleCnt = 0;
}

//Запускает постройку робота для стороны ИИ по полученному в аргументе из ChooseAndBuildAIRobot шаблону
void CConstructor::BuildRobotByTemplate(const SRobotTemplate& bot)
{
    //Hull
    OperateModule(MRT_HULL, bot.m_Hull.m_Module.m_nKind);
    //Chassis
    OperateModule(MRT_CHASSIS, bot.m_Chassis.m_nKind);
    //Head
    OperateModule(MRT_HEAD, bot.m_Head.m_nKind);
    //Weapons
    //int weapons_count = g_Config.m_RobotHullsConsts[bot.m_Hull.m_Module.m_nKind].weapon_pylon_data.size();
	for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
    {
        OperateModule(MRT_WEAPON, bot.m_Weapon[i].m_Module.m_nKind, i);
	}

    //Запуск робота в производство
    AddRobotToBuildingQueue(nullptr, bot.m_Team);
}

void CConstructor::OperateCurrentConstruction()
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    ResetConstruction();

    int cfg_num = ps->m_ConstructPanel->m_CurrentConfig;
    
    OperateModule(MRT_CHASSIS, ps->m_ConstructPanel->m_Configs[cfg_num].m_Chassis.m_nKind);
    OperateModule(MRT_HULL, ps->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Module.m_nKind);

    for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
    {
        if(ps->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[i].m_nKind)
        {
            OperateModule(MRT_WEAPON, ps->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[i].m_nKind, i);
        }
    }

    OperateModule(MRT_HEAD, ps->m_ConstructPanel->m_Configs[cfg_num].m_Head.m_nKind);
}

void CConstructor::GetConstructionPrice(int* res)
{
    CMatrixRobotAI* robot = m_Robot;
    ZeroMemory(res, sizeof(int) * MAX_RESOURCES);

    //Проверяем возможно установленную в слоте голову на наличие эффекта изменения стоимости
    float cost_mod = 1.0f;
    int cur_head = robot->GetRobotHeadKind();
    if(cur_head)
    {
        CBlockPar* bp = g_Config.m_RobotHeadsConsts[cur_head].effects;
        int effects_count = bp->ParCount();
        if(effects_count)
        {
            for(int i = 0; i < effects_count; ++i)
            {
                if(bp->ParGetName(i) == L"CostModifyPercent")
                {
                    cost_mod += max(bp->ParGet(i).GetDouble() * 0.01f, -1.0f);
                    break;
                }
            }
        }
    }

    for(int i = 0; i < robot->m_ModulesCount; ++i)
    {
        int price[MAX_RESOURCES];
        ZeroMemory(price, sizeof(price));

        switch(robot->m_Module[i].m_Type)
        {
            case MRT_HULL:
            {
                int kind = robot->m_Module[i].m_Kind;
                price[0] = Float2Int(g_Config.m_RobotHullsConsts[kind].cost_titan * cost_mod);
                price[1] = Float2Int(g_Config.m_RobotHullsConsts[kind].cost_electronics * cost_mod);
                price[2] = Float2Int(g_Config.m_RobotHullsConsts[kind].cost_energy * cost_mod);
                price[3] = Float2Int(g_Config.m_RobotHullsConsts[kind].cost_plasma * cost_mod);
                break;
            }
            case MRT_CHASSIS:
            {
                int kind = robot->m_Module[i].m_Kind;
                price[0] = Float2Int(g_Config.m_RobotChassisConsts[kind].cost_titan * cost_mod);
                price[1] = Float2Int(g_Config.m_RobotChassisConsts[kind].cost_electronics * cost_mod);
                price[2] = Float2Int(g_Config.m_RobotChassisConsts[kind].cost_energy * cost_mod);
                price[3] = Float2Int(g_Config.m_RobotChassisConsts[kind].cost_plasma * cost_mod);
                break;
            }
            case MRT_HEAD:
            {
                int kind = robot->m_Module[i].m_Kind;
                price[0] = g_Config.m_RobotHeadsConsts[kind].cost_titan;
                price[1] = g_Config.m_RobotHeadsConsts[kind].cost_electronics;
                price[2] = g_Config.m_RobotHeadsConsts[kind].cost_energy;
                price[3] = g_Config.m_RobotHeadsConsts[kind].cost_plasma;
                break;
            }
            case MRT_WEAPON:
            {
                int kind = robot->m_Module[i].m_Kind;
                price[0] = Float2Int(g_Config.m_RobotWeaponsConsts[kind].cost_titan * cost_mod);
                price[1] = Float2Int(g_Config.m_RobotWeaponsConsts[kind].cost_electronics * cost_mod);
                price[2] = Float2Int(g_Config.m_RobotWeaponsConsts[kind].cost_energy * cost_mod);
                price[3] = Float2Int(g_Config.m_RobotWeaponsConsts[kind].cost_plasma * cost_mod);
                break;
            }
        }

        for(int j = 0; j < MAX_RESOURCES; ++j)
        {
            res[j] += price[j];
        }
    }
}

//Возвращает структуру для подстановки в характеристики робота в конструкторе
int CConstructor::GetConstructionStructure()
{
    CMatrixRobotAI* robot = m_Robot;
    int structure = 0;

    for(int i = 0; i < robot->m_ModulesCount; ++i)
    {
        if(robot->m_Module[i].m_Type == MRT_HULL) structure += g_Config.m_RobotHullsConsts[robot->m_Module[i].m_Kind].structure;
        else if(robot->m_Module[i].m_Type == MRT_CHASSIS) structure += g_Config.m_RobotChassisConsts[robot->m_Module[i].m_Kind].structure;
    }

    float up = 0;
    for(int i = 0; i < robot->m_ModulesCount; ++i)
    {
        if(robot->m_Module[i].m_Type == MRT_HEAD)
        {
            CBlockPar* bp = g_Config.m_RobotHeadsConsts[robot->m_Module[i].m_Kind].effects;
            if(bp)
            {
                structure += 10.0f * bp->ParGetNE(L"AddStructure").GetFloat();
                float hp_factor = bp->ParGetNE(L"AddStructurePercent").GetFloat() * 0.01;
                if(hp_factor) structure += structure * hp_factor;
                structure = max(structure, 10.0f);
            }
            break;
        }
    }

    //return Float2Int((float(structure) + float(structure) * up) * 0.1f);
    return Float2Int(structure / 10.0f); //Используем для каста дабометод, чтобы было соответствие со счётчиком HP в карточке уже построенного робота
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void __stdcall CConstructorPanel::RemoteFocusElement(void* object)
{
    FocusElement((CIFaceElement*)object);
}
void __stdcall CConstructorPanel::RemoteUnFocusElement(void* object)
{
    if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))  UnFocusElement((CIFaceElement*)object);
}

void CConstructorPanel::FocusElement(CIFaceElement* element)
{
    if(m_FocusedElement != element)
    {
        m_FocusedElement = element;

        CIFaceElement* el = m_FocusedElement;

        ERobotModuleType type = MRT_EMPTY;
        ERobotModuleKind kind = RUK_EMPTY;

        int cfg_num = m_CurrentConfig;

        if(el->m_strName == IF_BASE_WEAPON_PYLON_1)
        {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[0].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if(el->m_strName == IF_BASE_WEAPON_PYLON_2)
        {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[1].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if(el->m_strName == IF_BASE_WEAPON_PYLON_3)
        {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[2].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if(el->m_strName == IF_BASE_WEAPON_PYLON_4)
        {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[3].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if(el->m_strName == IF_BASE_WEAPON_PYLON_5)
        {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[4].m_nKind;
            SetLabelsAndPrice(type, kind);        
        }
        else if(el->m_strName == IF_BASE_HEAD_PYLON)
        {
            type = MRT_HEAD;
            kind = m_Configs[cfg_num].m_Head.m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if(el->m_strName == IF_BASE_HULL_PYLON)
        {
            type = MRT_HULL;
            kind = m_Configs[cfg_num].m_Hull.m_Module.m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if(el->m_strName == IF_BASE_CHASSIS_PYLON)
        {
            type = MRT_CHASSIS;
            kind = m_Configs[cfg_num].m_Chassis.m_nKind;
            SetLabelsAndPrice(type, kind);
        }
    }
}

void CConstructorPanel::UnFocusElement(CIFaceElement* element)
{
    if(m_FocusedElement == element)
    {
        g_IFaceList->DeleteItemPrice();
        m_FocusedElement = nullptr;
        m_FocusedLabel = CWStr(L"");
        m_FocusedDescription = CWStr(L"");
    }
}

void CConstructorPanel::ActivateAndSelect()
{
    m_Active = 1;

    g_IFaceList->CreateSummPrice();
    g_MatrixMap->Pause(true);
}

/*
void CConstructorPanel::FocusElement(void* object)
{
    if((void*)m_FocusedElement != object)
    {
        g_IFaceList->DeleteItemPrice();
        m_FocusedElement = (CIFaceElement*)object;
        g_IFaceList->CreateItemPrice();
    }
}
*/

void CConstructorPanel::ResetGroupNClose()
{   
    m_Active = 0; 
    if(FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu)
    {
        g_PopupMenu->ResetMenu(true);
    }
    g_MatrixMap->Pause(false);

}

//Проверочная функция, вызывается конструктором в момент формирования выпадающего меню выбора модулей
//Используется для подсветки красным цветом названий модулей, которые игрок не сможет построить из-за нехватки ресурсов
bool CConstructorPanel::IsEnoughResourcesForThisModule(
    int pilon,
    ERobotModuleType type,
    ERobotModuleKind kind,
    float cost_modify_by_head
)
{
    int returned_res[MAX_RESOURCES] = { 0 }; //Сколько ресурсов станет дополнительно доступно после переключения с текущего (выбранного в конфиге модуля)
    int required_res[MAX_RESOURCES] = { 0 }; //Сколько ресурсов в принципе нужно для постройки нового (полученного функцией) модуля
    int item_res[MAX_RESOURCES] = { 0 }; //Сколько ресурсов стоит выбранный модуль (используется для проверки на дефицитность конкретного ресурса)
    int total_res_needed[MAX_RESOURCES] = { 0 }; //Сколько ресурсов будет необходимо иметь в запасе для постройки нового (полученного функцией)

    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    ps->m_Constructor->GetConstructionPrice(total_res_needed);

    //Проверяем возможно установленную в слоте голову на наличие эффекта изменения стоимости
    float cost_mod = 1.0f; //Считаем последовательно для каждого модуля по отдельности, чтобы случайно не вызвать расхождений в округлении с расчётом визуальной цены в конструкторе
    if(type != MRT_HEAD)
    {
        int cur_head = m_Configs[m_CurrentConfig].m_Head.m_nKind;
        if(cur_head)
        {
            CBlockPar* bp = g_Config.m_RobotHeadsConsts[cur_head].effects;
            int effects_count = bp->ParCount();
            if(effects_count)
            {
                for(int i = 0; i < effects_count; ++i)
                {
                    if(bp->ParGetName(i) == L"CostModifyPercent")
                    {
                        cost_mod += max(bp->ParGet(i).GetDouble() * 0.01f, -1.0f);
                        break;
                    }
                }
            }
        }
    }

    if(type == MRT_HULL)
    {
        //Если в данный момент установлен хоть какой-то корпус
        int cur_hull_num = m_Configs[m_CurrentConfig].m_Hull.m_Module.m_nKind;
        if(cur_hull_num)
        {
            //Сколько ресурсов вернётся в общий пул после замены текущего корпуса из конфига
            returned_res[0] = g_Config.m_RobotHullsConsts[cur_hull_num].cost_titan * cost_mod;
            returned_res[1] = g_Config.m_RobotHullsConsts[cur_hull_num].cost_electronics * cost_mod;
            returned_res[2] = g_Config.m_RobotHullsConsts[cur_hull_num].cost_energy * cost_mod;
            returned_res[3] = g_Config.m_RobotHullsConsts[cur_hull_num].cost_plasma * cost_mod;

            for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data.size(); ++i)
            {
                int const_pylon = g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data[i].constructor_slot_num;
                int weapon = m_Configs[m_CurrentConfig].m_Weapon[const_pylon].m_nKind;
                if(weapon) //Если в данном слоте корпуса в конструкторе установлено хоть какое-то оружие
                {
                    //Считаем стоимость этого оружия
                    returned_res[0] += g_Config.m_RobotWeaponsConsts[weapon].cost_titan * cost_mod;
                    returned_res[1] += g_Config.m_RobotWeaponsConsts[weapon].cost_electronics * cost_mod;
                    returned_res[2] += g_Config.m_RobotWeaponsConsts[weapon].cost_energy * cost_mod;
                    returned_res[3] += g_Config.m_RobotWeaponsConsts[weapon].cost_plasma * cost_mod;
                }
            }
        }

        //Сколько ресурсов стоит сам выбранный корпус
        required_res[0] = g_Config.m_RobotHullsConsts[kind].cost_titan * cost_mod;
        required_res[1] = g_Config.m_RobotHullsConsts[kind].cost_electronics * cost_mod;
        required_res[2] = g_Config.m_RobotHullsConsts[kind].cost_energy * cost_mod;
        required_res[3] = g_Config.m_RobotHullsConsts[kind].cost_plasma * cost_mod;

        //Для проверки, стоит ли проверяемый корпус дефицитного ресурса сам по себе
        item_res[0] = g_Config.m_RobotHullsConsts[kind].cost_titan * cost_mod;
        item_res[1] = g_Config.m_RobotHullsConsts[kind].cost_electronics * cost_mod;
        item_res[2] = g_Config.m_RobotHullsConsts[kind].cost_energy * cost_mod;
        item_res[3] = g_Config.m_RobotHullsConsts[kind].cost_plasma * cost_mod;

        for(int i = 0; i < g_Config.m_RobotHullsConsts[kind].weapon_pylon_data.size(); ++i)
        {
            int const_pylon = g_Config.m_RobotHullsConsts[kind].weapon_pylon_data[i].constructor_slot_num;
            int weapon = m_Configs[m_CurrentConfig].m_Weapon[const_pylon].m_nKind;
            if(weapon) //Если в данном слоте корпуса в конструкторе установлено хоть какое-то оружие
            {
                //Если это оружие нельзя установить в аналогичный слот выбранного корпуса, пропускаем его
                if(!g_Config.m_RobotHullsConsts[kind].weapon_pylon_data[i].fit_weapon.test(weapon)) continue;

                //Суммируем стоимость всего подходящего для переноса оружия к стоимости выбранного корпуса
                required_res[0] += g_Config.m_RobotWeaponsConsts[weapon].cost_titan * cost_mod;
                required_res[1] += g_Config.m_RobotWeaponsConsts[weapon].cost_electronics * cost_mod;
                required_res[2] += g_Config.m_RobotWeaponsConsts[weapon].cost_energy * cost_mod;
                required_res[3] += g_Config.m_RobotWeaponsConsts[weapon].cost_plasma * cost_mod;

                //Также суммируем их цену к фактической цене корпуса
                //В противном случае возможна ситуация, когда при нехватке ресурсов для переключения корпуса вместе с пушками, корпус всё равно подсвечен в списке доступным
                item_res[0] += g_Config.m_RobotWeaponsConsts[weapon].cost_titan * cost_mod;
                item_res[1] += g_Config.m_RobotWeaponsConsts[weapon].cost_electronics * cost_mod;
                item_res[2] += g_Config.m_RobotWeaponsConsts[weapon].cost_energy * cost_mod;
                item_res[3] += g_Config.m_RobotWeaponsConsts[weapon].cost_plasma * cost_mod;
            }
        }
    }
    else if(type == MRT_CHASSIS)
    {
        //Если в данный момент установлено хоть какое-то шасси
        int cur_chassis = m_Configs[m_CurrentConfig].m_Chassis.m_nKind;
        if(cur_chassis)
        {
            returned_res[0] = g_Config.m_RobotChassisConsts[cur_chassis].cost_titan * cost_mod;
            returned_res[1] = g_Config.m_RobotChassisConsts[cur_chassis].cost_electronics * cost_mod;
            returned_res[2] = g_Config.m_RobotChassisConsts[cur_chassis].cost_energy * cost_mod;
            returned_res[3] = g_Config.m_RobotChassisConsts[cur_chassis].cost_plasma * cost_mod;
        }

        required_res[0] = g_Config.m_RobotChassisConsts[kind].cost_titan * cost_mod;
        required_res[1] = g_Config.m_RobotChassisConsts[kind].cost_electronics * cost_mod;
        required_res[2] = g_Config.m_RobotChassisConsts[kind].cost_energy * cost_mod;
        required_res[3] = g_Config.m_RobotChassisConsts[kind].cost_plasma * cost_mod;

        item_res[0] = g_Config.m_RobotChassisConsts[kind].cost_titan * cost_mod;
        item_res[1] = g_Config.m_RobotChassisConsts[kind].cost_electronics * cost_mod;
        item_res[2] = g_Config.m_RobotChassisConsts[kind].cost_energy * cost_mod;
        item_res[3] = g_Config.m_RobotChassisConsts[kind].cost_plasma * cost_mod;
    }
    else if(type == MRT_HEAD)
    {
        //Если в данный момент установлена хоть какая-то голова
        int cur_head = m_Configs[m_CurrentConfig].m_Head.m_nKind;
        if(cur_head)
        {
            returned_res[0] = g_Config.m_RobotHeadsConsts[cur_head].cost_titan;
            returned_res[1] = g_Config.m_RobotHeadsConsts[cur_head].cost_electronics;
            returned_res[2] = g_Config.m_RobotHeadsConsts[cur_head].cost_energy;
            returned_res[3] = g_Config.m_RobotHeadsConsts[cur_head].cost_plasma;
        }

        required_res[0] = g_Config.m_RobotHeadsConsts[kind].cost_titan;
        required_res[1] = g_Config.m_RobotHeadsConsts[kind].cost_electronics;
        required_res[2] = g_Config.m_RobotHeadsConsts[kind].cost_energy;
        required_res[3] = g_Config.m_RobotHeadsConsts[kind].cost_plasma;

        item_res[0] = g_Config.m_RobotHeadsConsts[kind].cost_titan;
        item_res[1] = g_Config.m_RobotHeadsConsts[kind].cost_electronics;
        item_res[2] = g_Config.m_RobotHeadsConsts[kind].cost_energy;
        item_res[3] = g_Config.m_RobotHeadsConsts[kind].cost_plasma;
    }
    else if(type == MRT_WEAPON)
    {
        //Если в данный момент в этом пилоне установлено хоть какое-то оружие
        int cur_weapon = m_Configs[m_CurrentConfig].m_Weapon[pilon].m_nKind;
        if(cur_weapon)
        {
            returned_res[0] = g_Config.m_RobotWeaponsConsts[cur_weapon].cost_titan * cost_mod;
            returned_res[1] = g_Config.m_RobotWeaponsConsts[cur_weapon].cost_electronics * cost_mod;
            returned_res[2] = g_Config.m_RobotWeaponsConsts[cur_weapon].cost_energy * cost_mod;
            returned_res[3] = g_Config.m_RobotWeaponsConsts[cur_weapon].cost_plasma * cost_mod;
        }

        required_res[0] = g_Config.m_RobotWeaponsConsts[kind].cost_titan * cost_mod;
        required_res[1] = g_Config.m_RobotWeaponsConsts[kind].cost_electronics * cost_mod;
        required_res[2] = g_Config.m_RobotWeaponsConsts[kind].cost_energy * cost_mod;
        required_res[3] = g_Config.m_RobotWeaponsConsts[kind].cost_plasma * cost_mod;

        item_res[0] = g_Config.m_RobotWeaponsConsts[kind].cost_titan * cost_mod;
        item_res[1] = g_Config.m_RobotWeaponsConsts[kind].cost_electronics * cost_mod;
        item_res[2] = g_Config.m_RobotWeaponsConsts[kind].cost_energy * cost_mod;
        item_res[3] = g_Config.m_RobotWeaponsConsts[kind].cost_plasma * cost_mod;
    }

    for(int i = 0; i < MAX_RESOURCES; ++i) total_res_needed[i] = total_res_needed[i] - returned_res[i] + required_res[i];

    //for players only
    //if(ps->IsEnoughResources(total_res_needed)) return true;

    //Если количество доступных ресурсов ниже необходимого для постройки количества, и при этом выбираемый предмет хотя бы чего-то стоит
    for(int i = 0; i < MAX_RESOURCES; ++i)
    {
        if(ps->GetResourcesAmount((ERes)i) < total_res_needed[i] && item_res[i]) return false;
    }

    return true;
}

void CConstructorPanel::MakeItemReplacements(
    ERobotModuleType type,
    ERobotModuleKind kind
)
{
    if(type == MRT_EMPTY || kind == RUK_EMPTY) return;
    
    m_FocusedLabel.Replace(CWStr(L"<br>"), CWStr(L"\r\n"));

    CWStr color(L"<Color=247,195,0>", g_CacheHeap);
    if(type == MRT_WEAPON)
    {
        int weapon_type = g_Config.m_RobotWeaponsConsts[kind].weapon_type;

        int wspeed = g_Config.m_WeaponsConsts[weapon_type].shots_delay;
        int damage = g_Config.m_WeaponsConsts[weapon_type].damage.to_robots;

        damage = Float2Int(damage * 1000.0f / wspeed);

        int add_damage = 0;
        for(int i = 0; i < g_Config.m_WeaponsConsts[weapon_type].extra_effects.size(); ++i)
        {
            int effect = g_Config.m_WeaponsConsts[weapon_type].extra_effects[i].type;
            int effect_type = g_Config.m_WeaponsConsts[effect].secondary_effect;
            if(effect_type == SECONDARY_EFFECT_ABLAZE) add_damage += Float2Int(g_Config.m_WeaponsConsts[effect].damage.to_robots * 1000.0f / ABLAZE_LOGIC_PERIOD);
            else if(effect_type == SECONDARY_EFFECT_SHORTED_OUT) add_damage += Float2Int(g_Config.m_WeaponsConsts[effect].damage.to_robots * 1000.0f / SHORTED_OUT_LOGIC_PERIOD);
        }
        
        if(g_Config.m_WeaponsConsts[weapon_type].is_bomb) //Если оружие бомба, то перемножать её урон на скорострельность незачем
        {
            damage = g_Config.m_WeaponsConsts[weapon_type].damage.to_robots;
        }
        
        CWStr c(color, g_CacheHeap);
        CWStr colored_damage = c.Add(CWStr(L"dam</color>")).Replace(CWStr(L"dam"), CWStr(damage / 10));
        c = color;
        CWStr colored_add_damage = c.Add(CWStr(L"dam</color>")).Replace(CWStr(L"dam"), CWStr(add_damage / 10));

        m_FocusedLabel.Replace(CWStr(L"<Damage>"), colored_damage);
        m_FocusedLabel.Replace(CWStr(L"<EffectsDamage>"), colored_add_damage);
    }
    else if(type == MRT_HEAD)
    {
        CBlockPar* bp = g_Config.m_RobotHeadsConsts[kind].effects;
        if(bp)
        {
            int effects_count = bp->ParCount();
            CWStr repl(g_CacheHeap);
            for(int i = 0; i < effects_count; ++i)
            {
                double multiplier = 1.0;
                CWStr effect_name = bp->ParGetName(i);
                if(effect_name == L"AddStructure") repl = L"<AddStructure>";
                else if(effect_name == L"AddStructurePercent") repl = L"<AddStructurePercent>";
                else if(effect_name == L"SelfRepair") { repl = L"<SelfRepair>"; multiplier = 0.1; }
                else if(effect_name == L"SelfRepairPercent") repl = L"<SelfRepairPercent>";
                else if(effect_name == L"WeaponShotsDelay") repl = L"<WeaponShotsDelay>";
                else if(effect_name == L"WeaponOverheat") repl = L"<WeaponOverheat>";
                else if(effect_name == L"ChassisSpeed") repl = L"<ChassisSpeed>";
                else if(effect_name == L"WeaponDistance") repl = L"<WeaponDistance>";
                else if(effect_name == L"MissileTargetCaptureAngle") repl = L"<MissileTargetCaptureAngle>";
                else if(effect_name == L"MissileHomingSpeed") { repl = L"<MissileHomingSpeed>"; multiplier = 100.0; } //Не знаю, в чём должна измеряться данная величина, пусть будут проценты
                else if(effect_name == L"RadarDistance") repl = L"<RadarDistance>";
                else if(effect_name == L"DischargerProtect") repl = L"<DischargerProtect>";
                else if(effect_name == L"AimProtect") repl = L"<AimProtect>";
                else if(effect_name == L"BombProtect") repl = L"<BombProtect>";
                else if(effect_name == L"CostModifyPercent") repl = L"<CostModifyPercent>";
                
                int val = abs(Double2Int(multiplier * bp->ParGet(i).GetDouble()));
                m_FocusedLabel.Replace(repl, color + val + L"</color>");
            }
        }
    }
    else if(type == MRT_HULL)
    {
        int structure = g_Config.m_RobotHullsConsts[kind].structure;
        //int pylons = g_Config.m_RobotHullsConsts[kind].weapon_pylon_data.size();
        //int ext_pilons = g_Config.m_RobotHullsConsts[kind].weapons_info.extra_pylons;

        CWStr c = color;
        CWStr colored_size = c.Add(CWStr(L"a</color>")).Replace(CWStr(L"a"), CWStr(structure / 10));
        //c = color;
        //CWStr colored_pilons = c.Add(CWStr(L"a</color>")).Replace(CWStr(L"a"), CWStr(pylons));
        //c = color;
        //CWStr colored_epilons = c.Add(CWStr(L"a</color>")).Replace(CWStr(L"a"), CWStr(ext_pilons));

        m_FocusedLabel.Replace(CWStr(L"<Structure>"), colored_size);
        //m_FocusedLabel.Replace(CWStr(L"<Pilons>"), colored_pilons);
        //m_FocusedLabel.Replace(CWStr(L"<AddPilons>"), colored_epilons);
    }
    else if(type == MRT_CHASSIS)
    {
        int structure = g_Config.m_RobotChassisConsts[kind].structure;
        CWStr colored_size = color.Add(CWStr(L"size</color>")).Replace(CWStr(L"size"), CWStr(structure / 10));
        m_FocusedLabel.Replace(CWStr(L"<Structure>"), colored_size);
    }
}

//Выставляются описания и цены (только визуал) для выделенного игроком модуля в конструкторе роботов
void CConstructorPanel::SetLabelsAndPrice(ERobotModuleType type, ERobotModuleKind kind)
{
    g_IFaceList->DeleteItemPrice();
    if(!kind)
    {
        m_FocusedLabel = CWStr(L"");
        m_FocusedDescription = CWStr(L"");
        return;
    }

    //Проверяем возможно установленную в слоте голову на наличие эффекта изменения стоимости
    float cost_mod = 1.0f;
    if(type != MRT_HEAD)
    {
        int cur_head = m_Configs[m_CurrentConfig].m_Head.m_nKind;
        if(cur_head)
        {
            CBlockPar* bp = g_Config.m_RobotHeadsConsts[cur_head].effects;
            int effects_count = bp->ParCount();
            if(effects_count)
            {
                for(int i = 0; i < effects_count; ++i)
                {
                    if(bp->ParGetName(i) == L"CostModifyPercent")
                    {
                        cost_mod += max(bp->ParGet(i).GetDouble() * 0.01f, -1.0f);
                        break;
                    }
                }
            }
        }
    }

    switch(type)
    {
        //Здесь идёт передача указателя на данные массива для разбивки по смещениям (для стоимостей), что, имхо, не особо удачная идея, несмотря на то, что ресурсы в нём всегда идут подряд
        case MRT_HULL:
        {
            g_IFaceList->CreateItemPrice(&g_Config.m_RobotHullsConsts[kind].cost_titan, cost_mod);
            m_FocusedLabel = g_Config.m_RobotHullsConsts[kind].chars_description;
            m_FocusedDescription = g_Config.m_RobotHullsConsts[kind].art_description;
            break;
        }
        case MRT_CHASSIS:
        {
            g_IFaceList->CreateItemPrice(&g_Config.m_RobotChassisConsts[kind].cost_titan, cost_mod);
            m_FocusedLabel = g_Config.m_RobotChassisConsts[kind].chars_description;
            m_FocusedDescription = g_Config.m_RobotChassisConsts[kind].art_description;
            break;
        }
        case MRT_HEAD:
        {
            g_IFaceList->CreateItemPrice(&g_Config.m_RobotHeadsConsts[kind].cost_titan, cost_mod);
            m_FocusedLabel = g_Config.m_RobotHeadsConsts[kind].effects_description;
            m_FocusedDescription = g_Config.m_RobotHeadsConsts[kind].art_description;
            break;
        }
        case MRT_WEAPON:
        {
            g_IFaceList->CreateItemPrice(&g_Config.m_RobotWeaponsConsts[kind].cost_titan, cost_mod);
            m_FocusedLabel = g_Config.m_RobotWeaponsConsts[kind].chars_description;
            m_FocusedDescription = g_Config.m_RobotWeaponsConsts[kind].art_description;
            break;
        }
    }

    //Закидывает статы в описания характеристик предметов
    MakeItemReplacements(type, kind);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Определяем цену модуля в конструкторе роботов
void SPrice::SetPrice(ERobotModuleType type, ERobotModuleKind kind)
{
    ZeroMemory(m_Resources, sizeof(m_Resources));
    if(!kind) return;

    switch(type)
    {
        case MRT_HULL:
        {
            m_Resources[TITAN] = g_Config.m_RobotHullsConsts[kind].cost_titan;
            m_Resources[ELECTRONICS] = g_Config.m_RobotHullsConsts[kind].cost_electronics;
            m_Resources[ENERGY] = g_Config.m_RobotHullsConsts[kind].cost_energy;
            m_Resources[PLASMA] = g_Config.m_RobotHullsConsts[kind].cost_plasma;
            return;
        }
        case MRT_CHASSIS:
        {
            m_Resources[TITAN] = g_Config.m_RobotChassisConsts[kind].cost_titan;
            m_Resources[ELECTRONICS] = g_Config.m_RobotChassisConsts[kind].cost_electronics;
            m_Resources[ENERGY] = g_Config.m_RobotChassisConsts[kind].cost_energy;
            m_Resources[PLASMA] = g_Config.m_RobotChassisConsts[kind].cost_plasma;
            return;
        }
        case MRT_HEAD:
        {
            m_Resources[TITAN] = g_Config.m_RobotHeadsConsts[kind].cost_titan;
            m_Resources[ELECTRONICS] = g_Config.m_RobotHeadsConsts[kind].cost_electronics;
            m_Resources[ENERGY] = g_Config.m_RobotHeadsConsts[kind].cost_energy;
            m_Resources[PLASMA] = g_Config.m_RobotHeadsConsts[kind].cost_plasma;
            return;
        }
        case MRT_WEAPON:
        {
            m_Resources[TITAN] = g_Config.m_RobotWeaponsConsts[kind].cost_titan;
            m_Resources[ELECTRONICS] = g_Config.m_RobotWeaponsConsts[kind].cost_electronics;
            m_Resources[ENERGY] = g_Config.m_RobotWeaponsConsts[kind].cost_energy;
            m_Resources[PLASMA] = g_Config.m_RobotWeaponsConsts[kind].cost_plasma;
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Определение для массивов с готовыми конфигами роботов
std::vector<SRobotTemplate> SRobotTemplate::m_AIRobotTypeList[TOTAL_SIDES + 2]; //Два дополнительных места резервируем для загрузки шаблонов спавнера и подкреплений

//Сборка объекта-робота из заранее заготовленного объекта-конфига SRobotTemplate (не из строки-шаблона)
CMatrixRobotAI* SRobotTemplate::CreateRobotByTemplate(
    const D3DXVECTOR3& pos,
    int side_id,
    bool map_prepare_in_process
)
{
    CMatrixRobotAI* robot = HNew(g_MatrixHeap) CMatrixRobotAI;
    CMatrixSideUnit* side = g_MatrixMap->GetSideById(side_id);
    
    if(!side || !robot) return nullptr;

    robot->ModuleClear();
    if(m_Head.m_nKind) robot->ModuleInsert(0, MRT_HEAD, m_Head.m_nKind);

    SWeaponModule weapons[RUK_WEAPON_PYLONS_COUNT];
    //ZeroMemory(weapons, sizeof(weapons));

    if(m_Hull.m_Module.m_nKind)
    {
        //ААА БЛЯТЬ!!! КАК ЖЕ Я НЕНАВИЖУ ЕБУЧИЙ ЗАКРЫТЫЙ РЕДАКТОР КАРТ, КОТОРЫЙ СПАМИТ ЭТУ ПАРАШУ СО СБИТЫМИ НОМЕРАМИ МОДУЛЕЙ! СУКА!!!
        if(map_prepare_in_process) //Если идёт начальная загрузка боя (собираем стартовых роботов)
        {
            //Ебаный костыль, чтобы пофиксить ошибки нумерации корпусов в редакторе карт (наследие даба) - поудалять НАХУЙ!!! все эти проверки, если будет найден подход к патчу редактора и готовым картам
            if(m_Hull.m_Module.m_nKind == 6) m_Hull.m_Module.m_nKind = RUK_HULL_MONOSTACK;
            else m_Hull.m_Module.m_nKind = (ERobotModuleKind)(m_Hull.m_Module.m_nKind + 1);
        }

        int cur_hull_num = m_Hull.m_Module.m_nKind;
        for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data.size(); ++i)
        {
            int constr_pylon_num = g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data[i].constructor_slot_num; //Номер оружия в слоте конструктора

            if(!map_prepare_in_process)
            {
                if(m_Weapon[constr_pylon_num].m_Module.m_nKind)
                {
                    weapons[constr_pylon_num].m_Module.m_nType = MRT_WEAPON;
                    weapons[constr_pylon_num].m_Module.m_nKind = m_Weapon[constr_pylon_num].m_Module.m_nKind;
                    weapons[constr_pylon_num].m_ConstructorPylonNum = constr_pylon_num;

                    robot->WeaponInsert(0, MRT_WEAPON, weapons[constr_pylon_num].m_Module.m_nKind, cur_hull_num, i);
                }
            }
            //ВОТ ВЕСЬ ЭТОТ ПАРАШНЫЙ БЛОК НУЖЕН ТОЛЬКО ИЗ-ЗА ПАРЫ СБИТЫХ НОМЕРОВ ОРУДИЙ В СЛОТАХ В РЕДАКТОРЕ, СУКА!!!
            else //Если идёт начальная загрузка боя (собираем стартовых роботов) - поудалять НАХУЙ!!! все эти проверки, если будет найден подход к патчу редактора и готовым картам
            {
                //Ебаный костыль на загрузке шаблонов после ебаного редактора карт, чтобы пофиксить ошибки нумерации пушек в слотах 2 и 4 корпусов, имеющих слот под миномёт/бомбу
                if(cur_hull_num == 2 && constr_pylon_num == 4) constr_pylon_num = 1;
                else if(cur_hull_num == 4 && constr_pylon_num == 4) constr_pylon_num = 2;

                if(m_Weapon[constr_pylon_num].m_Module.m_nKind)
                {
                    if(cur_hull_num == 2 && constr_pylon_num == 1)
                    {
                        weapons[4].m_Module.m_nType = MRT_WEAPON;
                        weapons[4].m_Module.m_nKind = m_Weapon[1].m_Module.m_nKind;
                        weapons[4].m_ConstructorPylonNum = 4;

                        robot->WeaponInsert(0, MRT_WEAPON, weapons[4].m_Module.m_nKind, cur_hull_num, i);
                    }
                    else if(cur_hull_num == 4 && constr_pylon_num == 2)
                    {
                        weapons[4].m_Module.m_nType = MRT_WEAPON;
                        weapons[4].m_Module.m_nKind = m_Weapon[2].m_Module.m_nKind;
                        weapons[4].m_ConstructorPylonNum = 4;

                        robot->WeaponInsert(0, MRT_WEAPON, weapons[4].m_Module.m_nKind, cur_hull_num, i);
                    }
                    else
                    {
                        weapons[constr_pylon_num].m_Module.m_nType = MRT_WEAPON;
                        weapons[constr_pylon_num].m_Module.m_nKind = m_Weapon[constr_pylon_num].m_Module.m_nKind;
                        weapons[constr_pylon_num].m_ConstructorPylonNum = constr_pylon_num;

                        robot->WeaponInsert(0, MRT_WEAPON, weapons[constr_pylon_num].m_Module.m_nKind, cur_hull_num, i);
                    }
                }
            }
        }

	    robot->ModuleInsert(0, MRT_HULL, m_Hull.m_Module.m_nKind);
    }

    if(m_Chassis.m_nKind)
    {
		robot->ModuleInsert(0, MRT_CHASSIS, m_Chassis.m_nKind);
        D3DXMatrixIdentity(&robot->m_Module[0].m_Matrix);

        //Для "читерного" уровня проходимости шасси коллизию не считаем
        if(g_Config.m_RobotChassisConsts[m_Chassis.m_nKind].passability >= 5)
        {
            robot->SetNoObjectsCollision(true);
            robot->SetNoObstaclesCollision(true);
        }
	}

    robot->m_ShadowType = g_Config.m_RobotShadow;
    robot->m_ShadowSize = 128;

	robot->m_Side = side_id;
	robot->RobotWeaponInit();

    robot->m_PosX = pos.x;
	robot->m_PosY = pos.y;

	robot->CalcRobotParams(this);

    robot->m_ChassisForward = { 0.0f, 1.0f, 0.0f };
    robot->m_ChassisCamForward = { 0.0f, 1.0f, 0.0f };
    robot->m_HullForward = robot->m_ChassisForward;
  
    robot->SetTeam(m_Team);

    //robot sozdan
    GetConstructionName(robot);
    robot->m_CurrState = ROBOT_SUCCESSFULLY_BUILD;

    robot->GetResources(MR_Matrix | MR_Graph);
    //robot->CreateTextures();
    robot->SetBase(nullptr);

    robot->ResetMustDie();

    return robot;
}

//Считывает и заносит в массив список шаблонов роботов для использования ИИ во время загрузки карты
void SRobotTemplate::LoadAIRobotType(
    void* bp,
    int side_num
)
{
    if(bp == nullptr) return;

    int cnt = ((CBlockPar*)bp)->ParCount();
    if(!cnt) return;

    CWStr str(g_MatrixHeap), str2(g_MatrixHeap);
    for(int i = 0; i < cnt; ++i)
    {
        SRobotTemplate bot;
        m_AIRobotTypeList[side_num].push_back(bot);
        if(!m_AIRobotTypeList[side_num][i].CreateRobotTemplateFromPar(((CBlockPar*)bp)->ParGetName(i), ((CBlockPar*)bp)->ParGet(i)))
        {
            ERROR_S4(L"LoadAIRobotType function, loading robot #", CWStr(i).Get(), L" for side_num ", CWStr(side_num));
        }

        m_AIRobotTypeList[side_num][i].CalcStrength();
    }

    //Сортировка шаблонов роботов для ИИ по их силе (параметр силы также заранее определён в шаблоне)
    if(m_AIRobotTypeList[side_num].size() > 1) AIRobotTypeVectorSort(m_AIRobotTypeList[side_num], 0, m_AIRobotTypeList[side_num].size() - 1);
}

//Очищаются списки готовых шаблонов роботов для ИИ
void SRobotTemplate::ClearAIRobotType()
{
    for(int i = 0; i < TOTAL_SIDES + 2; ++i) m_AIRobotTypeList[i].clear();
}

//Быстрая порционная сортировка массива с конфигами для создания роботов
void SRobotTemplate::AIRobotTypeVectorSort(std::vector<SRobotTemplate>& vector, int low, int high)
{
    if(high <= low) return;

    int j = AIRobotTypeVectorPartition(vector, low, high);

    AIRobotTypeVectorSort(vector, low, j - 1);
    AIRobotTypeVectorSort(vector, j + 1, high);
}
int SRobotTemplate::AIRobotTypeVectorPartition(std::vector<SRobotTemplate>& vector, int low, int high)
{
    int i = low;
    int j = high + 1;

    while(true)
    {
        while(vector[++i].m_Strength < vector[low].m_Strength)
        {
            if(i == high) break;
        }

        while(vector[--j].m_Strength > vector[low].m_Strength)
        {
            if(j == low) break;
        }

        if(i >= j) break;

        std::swap(vector[i], vector[j]);
    }

    std::swap(vector[low], vector[j]);

    return j;
}

//Рассчитываем относительную силу робота для расчётов ИИ
void SRobotTemplate::CalcStrength()
{
    m_Strength = 0.0f;

    for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
    {
        if(m_Weapon[i].m_Module.m_nType != MRT_WEAPON) continue;
        m_Strength += g_Config.m_RobotWeaponsConsts[m_Weapon[i].m_Module.m_nKind].strength;
    }
}

float SRobotTemplate::DifWeapon(SRobotTemplate& other)
{
    int t1 = 0, t2 = 0;
    for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
    {
        if(m_Weapon[i].m_Module.m_nType != MRT_WEAPON) continue;
        ++t1;
    }

    for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
    {
        if(other.m_Weapon[i].m_Module.m_nType != MRT_WEAPON) continue;
        ++t2;
    }

    if(t2 > t1) return other.DifWeapon(*this);

    float cnt = 0;
    float cntr = 0;
    for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
    {
        if(m_Weapon[i].m_Module.m_nType != MRT_WEAPON) continue;
        cnt += 1.0f;

        for(int u = 0; u < RUK_WEAPON_PYLONS_COUNT; ++u)
        {
            if(other.m_Weapon[u].m_Module.m_nType != MRT_WEAPON) continue;

            if(m_Weapon[i].m_Module.m_nKind == other.m_Weapon[u].m_Module.m_nKind)
            {
                cntr += 1.0f;
                break;
            }
        }
    }

    return cntr / cnt;
}

//Загрузка шаблона робота из строки-параметра
bool SRobotTemplate::CreateRobotTemplateFromPar(const CWStr& par_name, const CWStr& par_val)
{
    int MIN_PAR_CNT = 2;

    if(par_name.GetCountPar(L",") < MIN_PAR_CNT) return false;

    CWStr str(g_MatrixHeap);

    // Initialization ////////////////////////////////////////////////////////////////////////////////////

    m_Chassis.m_nType = MRT_CHASSIS;
    m_Chassis.m_nKind = RUK_CHASSIS_PNEUMATIC; //По умолчанию первое шасси

    m_Hull.m_Module.m_nType = MRT_HULL;
    m_Hull.m_Module.m_nKind = RUK_HULL_MONOSTACK; //По умолчанию первый корпус

    m_Head.m_nType = MRT_WEAPON;
    m_Head.m_nKind = RUK_EMPTY;

    m_HaveBomb = false;
    m_HaveRepair = false;

    m_Team = 0;

    //Отсутствие оружия в слотах по умолчанию
    for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
    {
        m_Weapon[i].m_Module.m_nType = MRT_EMPTY;
        m_Weapon[i].m_Module.m_nKind = RUK_EMPTY;
    }

    for(int k = 0; k < MAX_RESOURCES; ++k) m_Resources[k] = 0;

    // Chassis ///////////////////////////////////////////////////////////////////////////////////////////
    str = par_name.GetStrPar(0, L",");
    str.Trim();
    for(int i = 1; i <= ROBOT_CHASSIS_COUNT; ++i)
    {
        if(str == g_Config.m_RobotChassisConsts[i].short_designation)
        {
            m_Chassis.m_nKind = (ERobotModuleKind)i;
            break;
        }
    }
    if(!m_Chassis.m_nKind) return false;

    m_Chassis.m_Price.SetPrice(m_Chassis.m_nType, m_Chassis.m_nKind);
    for(int k = 0; k < MAX_RESOURCES; ++k) m_Resources[k] += m_Chassis.m_Price.m_Resources[k];
    
    // Hull /////////////////////////////////////////////////////////////////////////////////////////////
    str = par_name.GetStrPar(1, L",");
    str.Trim();
    for(int i = 1; i <= ROBOT_HULLS_COUNT; ++i)
    {
        if(str == g_Config.m_RobotHullsConsts[i].short_designation)
        {
            m_Hull.m_Module.m_nKind = (ERobotModuleKind)i;
            break;
        }
    }
    if(!m_Hull.m_Module.m_nKind) return false;

    m_Hull.m_MaxWeaponsCnt = g_Config.m_RobotHullsConsts[m_Hull.m_Module.m_nKind].weapon_pylon_data.size();
    m_Hull.m_Module.m_Price.SetPrice(m_Hull.m_Module.m_nType, m_Hull.m_Module.m_nKind);
    for(int k = 0; k < MAX_RESOURCES; ++k) m_Resources[k] += m_Hull.m_Module.m_Price.m_Resources[k];

    // Weapons ///////////////////////////////////////////////////////////////////////////////////////////
    if(par_name.GetCountPar(L",") >= MIN_PAR_CNT + 1)
    {
        str = par_name.GetStrPar(MIN_PAR_CNT, L",");
        str.Trim();
        int cur_hull_kind = m_Hull.m_Module.m_nKind;

        //Перебор пушек из шаблона
        //int guns_count = 0;
        for(int i = 0; i < m_Hull.m_MaxWeaponsCnt; ++i)
        {
            int constr_pylon_num = g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].constructor_slot_num; //Абсолютный номер пилона для конструктора

            CWStr gun_des = str.GetStrPar(constr_pylon_num, L".");
            if(gun_des == L"-") continue; //Если слот оружия пуст
            for(int j = 1; j <= ROBOT_WEAPONS_COUNT; ++j)
            {
                if(gun_des == g_Config.m_RobotWeaponsConsts[j].short_designation)
                {
                    //Если это оружие может быть установлено в данный пилон
                    if(g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].fit_weapon.test(j))
                    {
                        m_Weapon[constr_pylon_num].m_Module.m_nKind = (ERobotModuleKind)j;
                        m_Weapon[constr_pylon_num].m_ConstructorPylonNum = constr_pylon_num;

                        m_HaveRepair = g_Config.m_RobotWeaponsConsts[j].is_module_repairer;
                        m_HaveBomb = g_Config.m_RobotWeaponsConsts[j].is_module_bomb;

                        m_Weapon[constr_pylon_num].m_Module.m_Price.SetPrice(m_Weapon[constr_pylon_num].m_Module.m_nType, m_Weapon[constr_pylon_num].m_Module.m_nKind);
                        for(int j = 0; j < MAX_RESOURCES; ++j) m_Resources[j] += m_Weapon[constr_pylon_num].m_Module.m_Price.m_Resources[j];
                    }

                    break;
                }
            }
        }
    }

    // Head //////////////////////////////////////////////////////////////////////////////////////////////
    if(par_name.GetCountPar(L",") >= MIN_PAR_CNT + 2)
    {
        str = par_name.GetStrPar(MIN_PAR_CNT + 1, L",");
        str.Trim();

        for(int i = 1; i <= ROBOT_HEADS_COUNT; ++i)
        {
            if(str == g_Config.m_RobotHeadsConsts[i].short_designation)
            {
                m_Head.m_nKind = (ERobotModuleKind)i;
                break;
            }
        }

        //А краш ваш нам нафиг не нужон! Даже если параметр оказался невалид
        //if(!m_Head.m_nKind) return false;

        //Меняем стоимость всего шаблона, если голова имеет соответствующий эффект
        CBlockPar* bp = g_Config.m_RobotHeadsConsts[m_Head.m_nKind].effects;
        int effects_count = bp->ParCount();
        if(effects_count)
        {
            for(int i = 0; i < effects_count; ++i)
            {
                if(bp->ParGetName(i) == L"CostModifyPercent")
                {
                    float cost_mod = max(bp->ParGet(i).GetDouble() * 0.01f, -1.0f) + 1.0f;
                    for(int k = 0; k < MAX_RESOURCES; ++k) m_Resources[k] = Float2Int(m_Resources[k] * cost_mod);
                    break;
                }
            }
        }

        m_Head.m_Price.SetPrice(m_Head.m_nType, m_Head.m_nKind);
        for(int k = 0; k < MAX_RESOURCES; ++k) m_Resources[k] += m_Head.m_Price.m_Resources[k];
    }

    //Также считываем значения из параметра
    m_TemplatePriority = max(par_val.GetStrPar(0, L",").GetInt(), 1);

    //Дополнительный параметр HP робота (не зависящий от установленных модулей, записанный вторым в его параметре, сразу после m_TemplatePriority
    m_HitPointsOverride = -1.0f;
    if(par_val.GetCountPar(L",") > 1) m_HitPointsOverride = max(10.0f * par_val.GetFloatPar(1, L","), 10.f);

    return true;
}

//Собирает имя робота из кратких названий корпуса, шасси и головы (именно в такой последовательности),
//после чего добавляет через тире число суммарного урона робота в секунду
void GetConstructionName(CMatrixRobotAI* robot)
{
    //CMatrixRobotAI* robot = m_Robot;

    CBlockPar* bp_tmp = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"RobotNames");

    robot->m_Name = L"";
    for(int i = 0; i < robot->m_ModulesCount; ++i)
    {
        if(robot->m_Module[i].m_Type == MRT_HULL)
        {
            robot->m_Name += g_Config.m_RobotHullsConsts[robot->m_Module[i].m_Kind].robot_part_name;
            break;
        }
    }

    for(int i = 0; i < robot->m_ModulesCount; ++i)
    {
        if(robot->m_Module[i].m_Type == MRT_CHASSIS)
        {
            robot->m_Name += g_Config.m_RobotChassisConsts[robot->m_Module[i].m_Kind].robot_part_name;
            break;
        }
    }

    for(int i = 0; i < robot->m_ModulesCount; ++i)
    {
        if(robot->m_Module[i].m_Type == MRT_HEAD)
        {
            robot->m_Name += g_Config.m_RobotHeadsConsts[robot->m_Module[i].m_Kind].robot_part_name;
            break;
        }
    }

    int dmg = GetConstructionDamage(robot);
    if(dmg) robot->m_Name += L"-" + CWStr(dmg, g_CacheHeap);
}

//Возвращает цифру суммарного урона в секунду для всех пушек для текущего шаблона робота в конструкторе
int GetConstructionDamage(CMatrixRobotAI* robot)
{
    int total_damage = 0;
    float shots_delay_decrease = 0;

    int head_kind = robot->GetRobotHeadKind();
    CBlockPar* bp = g_Config.m_RobotHeadsConsts[head_kind].effects;
    if(bp) shots_delay_decrease = max(bp->ParGetNE(L"WeaponShotsDelay").GetFloat() / 100.0f, -1.0);

    for(int i = 0; i < robot->m_ModulesCount; ++i)
    {
        if(robot->m_Module[i].m_Type == MRT_WEAPON)
        {
            int weapon_num = g_Config.m_RobotWeaponsConsts[robot->m_Module[i].m_Kind].weapon_type;
            if(!g_Config.m_WeaponsConsts[weapon_num].is_bomb && !g_Config.m_WeaponsConsts[weapon_num].is_repairer)
            {
                float shots_delay = g_Config.m_WeaponsConsts[weapon_num].shots_delay;
                shots_delay = Float2Int(shots_delay + shots_delay * shots_delay_decrease);

                float gun_damage = g_Config.m_WeaponsConsts[weapon_num].damage.to_robots;
                gun_damage = Float2Int(gun_damage * 1000.0f / shots_delay);
                total_damage += gun_damage;
            }
        }
    }

    return total_damage / 10;
}