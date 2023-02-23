// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../stdafx.h"
#include "CIFaceButton.h"
#include "CIFaceStatic.h"
#include "CInterface.h"
#include "CIFaceImage.h"
#include "../MatrixObjectBuilding.hpp"
#include "../MatrixMinimap.hpp"
#include "../MatrixGameDll.hpp"
#include "../MatrixFlyer.hpp"
#include "../MatrixObjectCannon.hpp"
#include "CAnimation.h"
#include "../MatrixSoundManager.hpp"
#include "CIFaceMenu.h"
#include "CCounter.h"
#include "CHistory.h"
#include "MatrixHint.hpp"
#include "../MatrixFormGame.hpp"

CIFaceList* g_IFaceList = nullptr;

SMenuItemText* g_PopupHull;
SMenuItemText* g_PopupChassis;
SMenuItemText* g_PopupHead;
SMenuItemText* g_PopupWeapon;

//Constructor destructor
CInterface::CInterface():name(g_MatrixHeap), m_strName(g_MatrixHeap), item_label1(g_MatrixHeap), item_label2(g_MatrixHeap), rcname(g_MatrixHeap)
{
    factory_res_income = -1;
    base_res_income = -1;
    btype = -1;
    prev_titan   = -1;
    prev_energy  = -1;
    prev_plasma  = -1;
    prev_electro = -1;

    blazer_cnt = -1;
    keller_cnt = -1;
    terron_cnt = -1;

    pilon1 = -1;
    pilon2 = -1;
    pilon3 = -1;
    pilon4 = -1;
    pilon5 = -1;

    pilon_ch = -1;
    pilon_he = -1;
    pilon_hu = -1;

    lives = -1;
    max_lives = -1;

    cur_sel = ESelection(-1);

    titan_summ = -1;
    electronics_summ = -1;
    energy_summ = -1;
    plasma_summ = -1;

    titan_unit = -1;
    electronics_unit = -1;
    energy_unit = -1;
    plasma_unit = -1;

    weight = -1;
    speed = -1;
    structure = -1;
    damage = -1;

    wght = -1;
    spd = -1;

    turmax = -1;
    turhave = -1;

    robots = -1;
    max_robots = -1;


    titan_color = 0xFFF6c000;
    electronics_color = 0xFFF6c000;
    energy_color = 0xFFF6c000;
    plasm_color = 0xFFF6c000;

    titan_unit_color = 0xFFF6c000;
    electronics_unit_color = 0xFFF6c000;
    energy_unit_color = 0xFFF6c000;
    plasm_unit_color = 0xFFF6c000;

//////////////////


    m_InterfaceFlags = 0;
    m_VisibleAlpha = IS_NOT_VISIBLE;
	m_xPos = 0;
	m_yPos = 0;
	m_zPos = 0;

	m_FirstElement = nullptr;
	m_LastElement = nullptr;
    m_FirstImage = nullptr;
    m_LastImage = nullptr;

    m_strName = L"";
	
    //m_Vertices = nullptr;
	m_PrevInterface = nullptr;
	m_NextInterface = nullptr;

	m_nTotalElements = 0;

	m_nId = 0;
	m_AlwaysOnTop = FALSE;

    ZeroMemory(&m_Slider, sizeof(SSlide));
}

CInterface::~CInterface()
{
	DTRACE();
    CIFaceImage* images = m_FirstImage; 
	ASSERT(g_MatrixHeap);

	//List dest

	CIFaceElement* ptmpElement = m_FirstElement;
    while (m_FirstElement) DelElement(m_FirstElement);

    while(images != nullptr)
    {
		if(images->m_NextImage) images = images->m_NextImage;
		else
        {
			HDelete(CIFaceImage, images, g_MatrixHeap);
			images = nullptr;
			m_FirstImage = nullptr;
			m_LastImage = nullptr;
		}

		if(images) HDelete(CIFaceImage, images->m_PrevImage, g_MatrixHeap);
	}
}

//Main routines
bool CInterface::Load(CBlockPar& bp, const wchar* name)
{
    bool need2save = false;
	int nElementNum = 0;
	CWStr tmpStr;
	CBlockPar* pbp1 = nullptr, *pbp2 = nullptr;
	void* fn = nullptr, *cl = nullptr;

//Loading interface file
	pbp1 = bp.BlockGet(name);

//Interface members initialisation
	m_strName = name;
    m_nTotalElements = pbp1->BlockCount();//pbp1->Par(L"eTotal").GetInt();
	m_nId = pbp1->Par(L"id").GetInt();
	m_xPos = pbp1->Par(L"xPos").GetFloat();
	m_yPos = pbp1->Par(L"yPos").GetFloat();
	m_zPos = pbp1->Par(L"zPos").GetFloat();

    if(m_xPos)
    {
        int width = 1024 - Float2Int(m_xPos);
        m_xPos = float(g_ScreenX - width);
    }
    if(m_yPos)
    {
        int height = 768 - Float2Int(m_yPos);
        m_yPos = float(g_ScreenY - height);
    }

    if(m_strName == IF_RADAR)
    {
        g_IFaceList->m_IFRadarPosX = Float2Int(m_xPos);
        g_IFaceList->m_IFRadarPosY = Float2Int(m_yPos);
    }
	
    CWStr labels_text(g_CacheHeap);
    CBlockPar* labels_file = nullptr;
   
    labels_file = pbp1->BlockGetNE(L"LabelsText");
    if(labels_file)
    {
        labels_file = labels_file->BlockGetNE(name);
    }
    
    if(pbp1->Par(L"OnTop").GetInt())
    {
		m_AlwaysOnTop = TRUE;
		m_VisibleAlpha = 255;
	}
    else m_AlwaysOnTop = FALSE;
	
	int Const = 0;
	Const = pbp1->Par(L"ConstPresent").GetInt();

    if(Const)
    {
		g_MatrixMap->GetPlayerSide()->m_Constructor->SetRenderProps(
	    			pbp1->Par(L"ConstX").GetFloat() + m_xPos, 
					pbp1->Par(L"ConstY").GetFloat() + m_yPos,
					pbp1->Par(L"ConstWidth").GetInt(), 
					pbp1->Par(L"ConstHeight").GetInt()
		);
    }

    CIFaceElement* if_elem = nullptr;
    for(int nC = 0; nC < m_nTotalElements; ++nC)
    {
		if_elem = nullptr;
        pbp2 = pbp1->BlockGet(nC);
		tmpStr = pbp1->BlockGetName(nC);

		if(tmpStr == L"Button")
        {
            CIFaceButton* pButton = HNew(g_MatrixHeap) CIFaceButton;
            if_elem = (CIFaceElement*)pButton;
			
			pButton->m_nId = pbp2->Par(L"id").GetInt();
			pButton->m_strName = pbp2->Par(L"Name");
            
            CWStr hint_par(L"", g_CacheHeap);
            hint_par = pbp2->ParNE(L"Hint");
            if(hint_par != L"")
            {
                pButton->m_Hint.HintTemplate = hint_par.GetStrPar(0, L",");
                pButton->m_Hint.x = hint_par.GetIntPar(1, L",");
                pButton->m_Hint.y = hint_par.GetIntPar(2, L",");
            }
            
			pButton->m_Type = (IFaceElementType)pbp2->Par(L"type").GetInt();
			pButton->m_nGroup = pbp2->Par(L"group").GetInt();
			pButton->m_xPos = pbp2->Par(L"xPos").GetFloat();
			pButton->m_yPos = pbp2->Par(L"yPos").GetFloat();
			pButton->m_zPos = pbp2->Par(L"zPos").GetFloat();
			pButton->m_xSize = pbp2->Par(L"xSize").GetFloat();
			pButton->m_ySize = pbp2->Par(L"ySize").GetFloat();
			pButton->m_Param1 = pbp2->Par(L"Param1").GetFloat();
			pButton->m_Param2 = pbp2->Par(L"Param2").GetFloat();
			pButton->m_DefState = (IFaceElementState)pbp2->Par(L"dState").GetInt();

			//FSET(pButton->m_Action, m_Constructor, CConstructor::OperateModule);

            //Задаются действия кнопкам в конструкторе роботов
			if(Const)
            {
                if(
                    (pButton->m_Param1 && pButton->m_Param2)       || 
                    pButton->m_strName == IF_BASE_HULL_PYLON       || 
                    pButton->m_strName == IF_BASE_CHASSIS_PYLON    || 
                    pButton->m_strName == IF_BASE_HEAD_PYLON       || 
                    pButton->m_strName == IF_BASE_WEAPON_PYLON_1   || 
                    pButton->m_strName == IF_BASE_WEAPON_PYLON_2   || 
                    pButton->m_strName == IF_BASE_WEAPON_PYLON_3   ||
                    pButton->m_strName == IF_BASE_WEAPON_PYLON_4   ||
                    pButton->m_strName == IF_BASE_WEAPON_PYLON_5   ||
                    pButton->m_strName == IF_BASE_HEAD_PYLON_EMPTY || 
                    pButton->m_strName == IF_BASE_WEAPON_PYLON_EMPTY
                  )
                {
					FSET(ON_UN_PRESS, pButton, cl, fn, g_MatrixMap->GetPlayerSide()->m_Constructor, CConstructor::RemoteOperateModule);
                    FSET(ON_FOCUS, pButton, cl, fn, g_MatrixMap->GetPlayerSide()->m_ConstructPanel, CConstructorPanel::RemoteFocusElement);
                    FSET(ON_UN_FOCUS, pButton, cl, fn, g_MatrixMap->GetPlayerSide()->m_ConstructPanel, CConstructorPanel::RemoteUnFocusElement);
                    pButton->m_nId = POPUP_REACTION_ELEMENT_ID;
				}
                else if(pButton->m_strName == IF_BASE_CONST_BUILD)
                {
					FSET(ON_UN_PRESS, pButton, cl, fn, g_MatrixMap->GetPlayerSide()->m_Constructor, CConstructor::RemoteBuild);
                    g_MatrixMap->GetPlayerSide()->m_Constructor->m_BaseConstBuildBut = pButton;
                }
                else if(pButton->m_strName == IF_BASE_CONST_CANCEL)
                {
                    FSET(ON_UN_PRESS, pButton, cl, fn, g_MatrixMap->GetPlayerSide()->m_Constructor, CIFaceList::PlayerAction);
                }
			}

            //Задаются действия кнопкам масштабирования миникарты
            if(pButton->m_strName == IF_MAP_ZOOM_IN)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, &g_MatrixMap->m_Minimap, CMinimap::ButtonZoomIn);
            }
            else if(pButton->m_strName == IF_MAP_ZOOM_OUT)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, &g_MatrixMap->m_Minimap, CMinimap::ButtonZoomOut);
            }
            //Задаются действия кнопкам в панели управления базой/юнитом
            else if(pButton->m_strName == IF_BUILD_ROBOT)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_MatrixMap->GetPlayerSide(), CMatrixSideUnit::PlayerAction);
            }
            else if(pButton->m_strName == IF_AORDER_ATTACK_ON)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_AORDER_ATTACK_OFF)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_AORDER_PROTECT_ON)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_AORDER_PROTECT_OFF)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_AORDER_CAPTURE_ON)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_AORDER_CAPTURE_OFF)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_ORDER_FIRE)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_ORDER_CAPTURE)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_ORDER_PATROL)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_ORDER_MOVE)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_ORDER_REPAIR)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            //Для кнопки бомбы прописываем два события, на левый и на правый клики мышью
            else if(pButton->m_strName == IF_ORDER_BOMB)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
                FSET(ON_UN_PRESS_RIGHT, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAltAction);
            }
            else if(pButton->m_strName == IF_ORDER_BOMB_AUTO)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
                FSET(ON_UN_PRESS_RIGHT, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAltAction);
            }
            else if(pButton->m_strName == IF_ORDER_CANCEL)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_ORDER_STOP)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_ENTER_ARCADE_MODE)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_EXIT_ARCADE_MODE)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_MAIN_SELF_BOMB)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
                FSET(ON_UN_PRESS_RIGHT, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAltAction);
            }
            else if(pButton->m_strName == IF_MAIN_SELF_BOMB_AUTO)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
                FSET(ON_UN_PRESS_RIGHT, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAltAction);
            }
            else if(pButton->m_strName == IF_BUILD_TURRET)
            {
                g_IFaceList->m_BuildCa = pButton;
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BUILD_FLYER)
            {
                //g_IFaceList->m_BuildHe = pButton;
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_CALL_FROM_HELL)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BUILD_REPAIR)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_SHOWROBOTS_BUTT)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CMinimap::ShowPlayerBots);
            }
            //Задаются действия кнопкам постройки конкретных турелей
            else if(pButton->m_strName == IF_BUILD_TUR1)
            {
                g_IFaceList->m_Turrets[0] = pButton;
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_BUILD_TUR2)
            {
                g_IFaceList->m_Turrets[1] = pButton;
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_BUILD_TUR3)
            {
                g_IFaceList->m_Turrets[2] = pButton;
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_BUILD_TUR4)
            {
                g_IFaceList->m_Turrets[3] = pButton;
                FSET(ON_UN_PRESS,pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            //Задаются действия кнопкам постройки конкретных вертолётов
            else if(pButton->m_strName == IF_BUILD_FLYER_1)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_BUILD_FLYER_2)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_BUILD_FLYER_3)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_BUILD_FLYER_4)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if(pButton->m_strName == IF_MAIN_MENU_BUTTON)
            {
                FSET(ON_UN_PRESS,pButton, cl, fn, g_IFaceList, CIFaceList::PlayerAction);
            }

            else if(pButton->m_strName == IF_BASE_UP)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList->m_RCountControl, CIFaceCounter::Up);
                g_IFaceList->m_RCountControl->SetButtonUp(pButton);
            }
            else if(pButton->m_strName == IF_BASE_DOWN)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_IFaceList->m_RCountControl, CIFaceCounter::Down);
                g_IFaceList->m_RCountControl->SetButtonDown(pButton);
            }
            else if(pButton->m_strName == IF_BASE_HISTORY_RIGHT)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_ConfigHistory, CHistory::NextConfig);
            }
            else if(pButton->m_strName == IF_BASE_HISTORY_LEFT)
            {
                FSET(ON_UN_PRESS, pButton, cl, fn, g_ConfigHistory, CHistory::PrevConfig);
            }

            //Кнопки выбора корпусов в конструкторе
            for(int i = 1; i <= ROBOT_HULLS_COUNT; ++i)
            {
                if(pButton->m_strName == g_Config.m_RobotHullsConsts[i].constructor_button_name)
                {
                    g_IFaceList->m_Hull[i - 1] = pButton; //-1, т.к. у корпуса не может быть пустого слота
                    pButton->m_Param2 = i;
                }
            }

            //Кнопки выбора шасси в конструкторе
            for(int i = 1; i <= ROBOT_CHASSIS_COUNT; ++i)
            {
                if(pButton->m_strName == g_Config.m_RobotChassisConsts[i].constructor_button_name)
                {
                    g_IFaceList->m_Chassis[i - 1] = pButton; //-1, т.к. у шасси не может быть пустого слота
                    pButton->m_Param2 = i;
                }
            }

            //Кнопки выбора голов в конструкторе
            for(int i = 1; i <= ROBOT_HEADS_COUNT; ++i)
            {
                if(pButton->m_strName == g_Config.m_RobotHeadsConsts[i].constructor_button_name)
                {
                    g_IFaceList->m_Head[i] = pButton;
                    pButton->m_Param2 = i;
                }
            }

            //Кнопки выбора оружия в конструкторе
            for(int i = 1; i <= ROBOT_WEAPONS_COUNT; ++i)
            {
                if(pButton->m_strName == g_Config.m_RobotWeaponsConsts[i].constructor_button_name)
                {
                    g_IFaceList->m_Weapon[i] = pButton;
                    pButton->m_Param2 = i;
                }
            }

            if(pButton->m_strName == IF_BASE_CHASSIS_PYLON)
            {
                g_IFaceList->m_ChassisPilon = pButton;
            }

            if(pButton->m_strName == IF_BASE_HULL_PYLON)
            {
                g_IFaceList->m_ArmorPilon = pButton;
            }

            if(pButton->m_strName == IF_BASE_HEAD_PYLON)
            {
                g_IFaceList->m_HeadPilon = pButton;
            }
            
            if(pButton->m_strName == IF_BASE_HEAD_PYLON_EMPTY)
            {
                g_IFaceList->m_Head[0] = pButton;
            }
            
            if(pButton->m_strName == IF_BASE_WEAPON_PYLON_EMPTY)
            {
                g_IFaceList->m_Weapon[0] = pButton;
            }

            if(pButton->m_strName == IF_BASE_WEAPON_PYLON_1)
            {
                g_IFaceList->m_WeaponPilon[0] = pButton;
            }
            if(pButton->m_strName == IF_BASE_WEAPON_PYLON_2)
            {
                g_IFaceList->m_WeaponPilon[1] = pButton;
            }
            if(pButton->m_strName == IF_BASE_WEAPON_PYLON_3)
            {
                g_IFaceList->m_WeaponPilon[2] = pButton;
            }
            if(pButton->m_strName == IF_BASE_WEAPON_PYLON_4)
            {
                g_IFaceList->m_WeaponPilon[3] = pButton;
            }
            if(pButton->m_strName == IF_BASE_WEAPON_PYLON_5)
            {
                g_IFaceList->m_WeaponPilon[4] = pButton;
            }

            pButton->SetStateImage(
                IFACE_NORMAL, 
                (CTextureManaged*)g_Cache->Get(cc_TextureManaged,
                    g_CacheData->ParPathGet(pbp2->Par(L"sNormal")).Get()),
                    pbp2->Par(L"sNormalX").GetDouble(),
                    pbp2->Par(L"sNormalY").GetDouble(),
                    pbp2->Par(L"sNormalWidth").GetDouble(),
                    pbp2->Par(L"sNormalHeight").GetDouble());

            pButton->SetStateImage(
                IFACE_FOCUSED,
                (CTextureManaged*)g_Cache->Get(cc_TextureManaged,
                    g_CacheData->ParPathGet(pbp2->Par(L"sFocused")).Get()),
                    pbp2->Par(L"sFocusedX").GetDouble(),
                    pbp2->Par(L"sFocusedY").GetDouble(),
                    pbp2->Par(L"sFocusedWidth").GetDouble(),
                    pbp2->Par(L"sFocusedHeight").GetDouble());

            pButton->SetStateImage(
                IFACE_PRESSED, 
                (CTextureManaged*)g_Cache->Get(cc_TextureManaged,
                    g_CacheData->ParPathGet(pbp2->Par(L"sPressed")).Get()),
                    pbp2->Par(L"sPressedX").GetDouble(),
                    pbp2->Par(L"sPressedY").GetDouble(),
                    pbp2->Par(L"sPressedWidth").GetDouble(),
                    pbp2->Par(L"sPressedHeight").GetDouble());

            pButton->SetStateImage(
                IFACE_DISABLED, 
                (CTextureManaged*)g_Cache->Get(cc_TextureManaged,
                    g_CacheData->ParPathGet(pbp2->Par(L"sDisabled")).Get()),
                    pbp2->Par(L"sDisabledX").GetDouble(),
                    pbp2->Par(L"sDisabledY").GetDouble(),
                    pbp2->Par(L"sDisabledWidth").GetDouble(),
                    pbp2->Par(L"sDisabledHeight").GetDouble());

            if(pButton->m_Type == IFACE_CHECK_BUTTON || pButton->m_Type == IFACE_CHECK_BUTTON_SPECIAL || pButton->m_Type == IFACE_CHECK_PUSH_BUTTON)
            {
                pButton->SetStateImage(
                    IFACE_PRESSED_UNFOCUSED, 
                    (CTextureManaged*)g_Cache->Get(cc_TextureManaged,
                        g_CacheData->ParPathGet(pbp2->Par(L"sPressedUnFocused")).Get()),
                        pbp2->Par(L"sPressedUnFocusedX").GetDouble(),
                        pbp2->Par(L"sPressedUnFocusedY").GetDouble(),
                        pbp2->Par(L"sPressedUnFocusedWidth").GetDouble(),
                        pbp2->Par(L"sPressedUnFocusedHeight").GetDouble());
            }
            
            //Animation
            CBlockPar* animation = nullptr;
            animation = pbp2->BlockGetNE(L"Animation");
            if(animation)
            {
                CWStr par = animation->Par(L"Frames");
                if(par.GetLen())
                {
                    int frames_cnt = par.GetIntPar(0, L",");
                    int period = par.GetIntPar(1, L",");
                    int width = par.GetIntPar(2, L",");
                    int height = par.GetIntPar(3, L",");

                    pButton->m_Animation = HNew(g_MatrixHeap) CAnimation(frames_cnt, period);
                    SFrame frame;
                    //frame.name = pButton->m_strName;
                    frame.height = height;
                    frame.width = width;
                    frame.pos_x = pButton->m_xPos + 1;
                    frame.pos_y = pButton->m_yPos;
                    //frame.pos_z = pButton->m_zPos;
                    frame.tex = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, g_CacheData->ParPathGet(pbp2->Par(L"sNormal")).Get());
                    frame.tex_width = pbp2->Par(L"sNormalWidth").GetFloat();
                    frame.tex_height = pbp2->Par(L"sNormalHeight").GetFloat();
                    frame.ipos_x = m_xPos;
                    frame.ipos_y = m_yPos;

                    for(int i = 0; i < frames_cnt * 2; i += 2)
                    {
                        int x = par.GetIntPar(3 + 1 + i, L",");
                        int y = par.GetIntPar(3 + 1 + i + 1, L",");
                        frame.tex_pos_x = x;
                        frame.tex_pos_y = y;
                        //Load Next Frame here
                        pButton->m_Animation->LoadNextFrame(&frame);
                    }
                }
            }
            
            //Initialising button geometry
			pButton->ElementGeomInit((void*)pButton);
            pButton->SetState(pButton->m_DefState);

            //Add Element(Button) to the list, Generate polygons and add them to pVertices
			AddElement(pButton);

			++nElementNum;
		}
        else if(tmpStr == L"Static")
        {
            CIFaceStatic* pStatic = HNew(g_MatrixHeap) CIFaceStatic;
            if_elem = (CIFaceElement*)pStatic;

			pStatic->m_strName = pbp2->Par(L"Name");
            
            CWStr hint_par(L"", g_CacheHeap);
            hint_par = pbp2->ParNE(L"Hint");
            if(hint_par != L"")
            {
                pStatic->m_Hint.HintTemplate = hint_par.GetStrPar(0, L",");
                pStatic->m_Hint.x = hint_par.GetIntPar(1, L",");
                pStatic->m_Hint.y = hint_par.GetIntPar(2, L",");
            }
            
            pStatic->m_xPos = (float)pbp2->Par(L"xPos").GetDouble();
			pStatic->m_yPos = (float)pbp2->Par(L"yPos").GetDouble();
			pStatic->m_zPos = (float)pbp2->Par(L"zPos").GetDouble();
			pStatic->m_xSize = (float)pbp2->Par(L"xSize").GetDouble();
			pStatic->m_ySize = (float)pbp2->Par(L"ySize").GetDouble();
			pStatic->m_DefState = (IFaceElementState)pbp2->Par(L"dState").GetInt();

            //pStatic->m_Hint. = pbp2->ParNE(L"Hint");

            if(m_strName == IF_TOP && pStatic->m_strName == IF_TOP_PANEL1)
            {
                g_MatrixMap->m_DI.SetStartPos(CPoint(10, Float2Int(m_yPos + pStatic->m_yPos + pStatic->m_ySize)));
            }
            else if(pStatic->m_strName == IF_MAP_PANEL)
            {
                g_MatrixMap->m_Minimap.SetOutParams(Float2Int(m_xPos) + 13, Float2Int(m_yPos) + 51, 145, 145, D3DXVECTOR2(g_MatrixMap->m_Size.x * GLOBAL_SCALE * 0.5f, g_MatrixMap->m_Size.y * GLOBAL_SCALE * 0.5f) ,1.0f, 0xFFFFFFFF);

                //FSET(ON_UN_PRESS,pStatic, cl, fn, &g_MatrixMap->m_Minimap, CMinimap::ButtonClick);
                pStatic->m_iParam = IF_MAP_PANELI;
            }
            else if(pStatic->m_strName == IF_RADAR_PN)
            {
                pStatic->m_iParam = IF_RADAR_PNI;
            }
            else if(pStatic->m_strName == IF_BASE_ZERO)
            {
                g_IFaceList->m_RCountControl->SetImage(0, pStatic);
            }
            else if(pStatic->m_strName == IF_BASE_ONE)
            {
                g_IFaceList->m_RCountControl->SetImage(1, pStatic);
            }
            else if(pStatic->m_strName == IF_BASE_TWO)
            {
                g_IFaceList->m_RCountControl->SetImage(2, pStatic);
            }
            else if(pStatic->m_strName == IF_BASE_THREE)
            {
                g_IFaceList->m_RCountControl->SetImage(3, pStatic);
            }
            else if(pStatic->m_strName == IF_BASE_FOUR)
            {
                g_IFaceList->m_RCountControl->SetImage(4, pStatic);
            }
            else if(pStatic->m_strName == IF_BASE_FIVE)
            {
                g_IFaceList->m_RCountControl->SetImage(5, pStatic);
            }
            else if(pStatic->m_strName == IF_BASE_SIX)
            {
                g_IFaceList->m_RCountControl->SetImage(6, pStatic);
            }
            else if(pStatic->m_strName == IF_TITAN_PLANT)
            {
                FSET(ON_PRESS, pStatic, cl, fn, g_IFaceList, CIFaceList::JumpToBuilding);
            }
            else if(pStatic->m_strName == IF_PLASMA_PLANT)
            {
                FSET(ON_PRESS, pStatic, cl, fn, g_IFaceList, CIFaceList::JumpToBuilding);
            }
            else if(pStatic->m_strName == IF_ELECTRO_PLANT)
            {
                FSET(ON_PRESS, pStatic, cl, fn, g_IFaceList, CIFaceList::JumpToBuilding);
            }
            else if(pStatic->m_strName == IF_ENERGY_PLANT)
            {
                FSET(ON_PRESS, pStatic, cl, fn, g_IFaceList, CIFaceList::JumpToBuilding);
            }
            else if(pStatic->m_strName == IF_BASE_PLANT)
            {
                FSET(ON_PRESS, pStatic, cl, fn, g_IFaceList, CIFaceList::JumpToBuilding);
            }

            pStatic->SetStateImage(
                IFACE_NORMAL,
                (CTextureManaged*)g_Cache->Get(cc_TextureManaged, g_CacheData->ParPathGet(pbp2->Par(L"sNormal")).Get()),
                (float)pbp2->Par(L"sNormalX").GetDouble(),
                (float)pbp2->Par(L"sNormalY").GetDouble(),
                (float)pbp2->Par(L"sNormalWidth").GetDouble(),
                (float)pbp2->Par(L"sNormalHeight").GetDouble());


			pStatic->ElementGeomInit((void*)pStatic);

            // only Labels free statics have ClearRect
            if(pbp2->BlockGetNE(IF_LABELS) == nullptr)
            {
                if(!pbp2->ParCount(L"ClearRect"))
                {
                    //pStatic->GenerateClearRect();

                    //CWStr   rect(g_CacheHeap);
                    //rect = pStatic->m_ClearRect.left; rect += L",";
                    //rect += pStatic->m_ClearRect.top; rect += L",";
                    //rect += pStatic->m_ClearRect.right; rect += L",";
                    //rect += pStatic->m_ClearRect.bottom;

                    //pbp2->ParAdd(L"ClearRect", rect);

                    //need2save = true;
                }
                else
                {
                    pStatic->SetClearRect();
                    
                    CWStr rect(pbp2->Par(L"ClearRect"), g_CacheHeap);

                    // element position relative
                    pStatic->m_ClearRect.left = rect.GetIntPar(0, L",");
                    pStatic->m_ClearRect.top = rect.GetIntPar(1, L",");
                    pStatic->m_ClearRect.right = rect.GetIntPar(2, L",");
                    pStatic->m_ClearRect.bottom = rect.GetIntPar(3, L",");
                }
            }

			AddElement(pStatic);
			++nElementNum;
		}
        else if(tmpStr == L"Image")
        {
			CIFaceImage* image = HNew(g_MatrixHeap) CIFaceImage;
			image->m_strName = pbp2->Par(L"Name");

            image->m_Image = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, g_CacheData->ParPathGet(pbp2->Par(L"TextureFile")).Get());

            image->m_xTexPos   = pbp2->Par(L"TexPosX").GetDouble();
            image->m_yTexPos   = pbp2->Par(L"TexPosY").GetDouble();
            image->m_TexWidth  = pbp2->Par(L"TextureWidth").GetDouble();
            image->m_TexHeight = pbp2->Par(L"TextureHeight").GetDouble();
            image->m_Width     = pbp2->Par(L"Width").GetDouble();
            image->m_Height    = pbp2->Par(L"Height").GetDouble();


            LIST_ADD(image, m_FirstImage, m_LastImage, m_PrevImage, m_NextImage);
            ++nElementNum;

            if(image->m_strName == IF_GROUP_GLOW)
            {
                CIFaceStatic* st = CreateStaticFromImage(0, 0, 0, *image);
                st->m_nId = GROUP_SELECTOR_ID;
            }
		}

        if(labels_file && if_elem)
        {
            CBlockPar* labels = nullptr;
            labels = pbp2->BlockGetNE(IF_LABELS);
            if(labels)
            {
                int labels_cnt = labels->BlockCount();
                for(int bl_cnt = 0; bl_cnt < labels_cnt; ++bl_cnt)
                {
                    if(labels->BlockGetName(bl_cnt).GetLen())
                    {
                        CBlockPar* label_block = labels->BlockGet(bl_cnt);
                        CWStr par = label_block->Par(IF_LABEL_PARAMS);
                        if(par.GetLen())
                        {
                            int x = par.GetIntPar(0, L",");
                            int y = par.GetIntPar(1, L",");

                            int sme_x = par.GetIntPar(2, L",");
                            int sme_y = par.GetIntPar(3, L",");

                            int align_x = par.GetIntPar(4, L",");
                            int align_y = par.GetIntPar(5, L",");

                            int perenos = par.GetIntPar(6, L",");

                            int clip_sx = par.GetIntPar(7, L",");
                            int clip_sy = par.GetIntPar(8, L",");
                            int clip_ex = par.GetIntPar(9, L",");
                            int clip_ey = par.GetIntPar(10, L",");

                            const CWStr& state = label_block->Par(L"State");
                            const CWStr& font = label_block->Par(L"Font");

                            const CWStr& color_par = label_block->Par(L"Color");

                            dword color = 0;

                            color |= color_par.GetIntPar(0, L",") << 24;
                            color |= color_par.GetIntPar(1, L",") << 16;
                            color |= color_par.GetIntPar(2, L",") << 8;
                            color |= color_par.GetIntPar(3, L",");
                                
                            IFaceElementState st;
                            if(state == IF_STATE_NORMAL)
                            {
                                st = IFACE_NORMAL;
                            }
                            else if(state == IF_STATE_FOCUSED)
                            {
                                st = IFACE_FOCUSED;
                            }
                            else if(state == IF_STATE_PRESSED)
                            {
                                st = IFACE_PRESSED;
                            }
                            else if(state == IF_STATE_DISABLED)
                            {
                                st = IFACE_DISABLED;
                            }
                            else if(state == IF_STATE_PRESSED_UNFOCUSED)
                            {
                                st = IFACE_PRESSED_UNFOCUSED;
                            }

                            //Подгружаем всякие тексты и текстуры с текстами для разделов конструктора
                            if(labels->BlockGetName(bl_cnt) == IF_STATE_STATIC_LABEL)
                            {
                                //В этом месте формируются всякие строки вида "ihu1text_sNormal"
                                //После чего они грузят текст из условно рандомных мест датника iface.txt, что убого
                                //Нужно будет переделать
                                CWStr t_code;
                                t_code.Add(if_elem->m_strName).Add(L"_").Add(state); //В state сюда попадают варианты: sFocused, sPressed, sDisabled и sNormal
                                CWStr text = label_block->ParNE(L"ModuleType"); //Указатель, что текст нужно подгружать не по стандартному хуйпоймикакому пути, а из заранее загруженного массива
                                if(text == L"") text = labels_file->ParNE(t_code);
                                else
                                {
                                    //Заносим названия модулей в пункты выпадающих списков-панелей в конструкторе роботов
                                    if(text == L"Hull")
                                    {
                                        for(int i = 1; i <= ROBOT_HULLS_COUNT; ++i)
                                        {
                                            if(if_elem->m_strName == g_Config.m_RobotHullsConsts[i].constructor_label_name)
                                            {
                                                text = g_Config.m_RobotHullsConsts[i].name;
                                                g_PopupHull[i - 1].text = text; //-1, т.к. у корпусов не может быть пустого слота
                                                break;
                                            }
                                            else if(i == ROBOT_HULLS_COUNT) text = g_Config.m_RobotHullsConsts[0].name;
                                        }
                                    }
                                    else if(text == L"Chassis")
                                    {
                                        for(int i = 1; i <= ROBOT_CHASSIS_COUNT; ++i)
                                        {
                                            if(if_elem->m_strName == g_Config.m_RobotChassisConsts[i].constructor_label_name)
                                            {
                                                text = g_Config.m_RobotChassisConsts[i].name;
                                                g_PopupChassis[i - 1].text = text; //-1, т.к. у шасси не может быть пустого слота
                                                break;
                                            }
                                            else if(i == ROBOT_CHASSIS_COUNT) text = g_Config.m_RobotChassisConsts[0].name;
                                        }
                                    }
                                    else if(text == L"Head")
                                    {
                                        for(int i = 1; i <= ROBOT_HEADS_COUNT; ++i)
                                        {
                                            if(if_elem->m_strName == g_Config.m_RobotHeadsConsts[i].constructor_label_name)
                                            {
                                                text = g_Config.m_RobotHeadsConsts[i].name;
                                                g_PopupHead[i].text = text;
                                                break;
                                            }
                                            else if(i == ROBOT_HEADS_COUNT) text = g_Config.m_RobotHeadsConsts[0].name;
                                        }
                                    }
                                    else if(text == L"Weapon")
                                    {
                                        for(int i = 1; i <= ROBOT_WEAPONS_COUNT; ++i)
                                        {
                                            if(if_elem->m_strName == g_Config.m_RobotWeaponsConsts[i].constructor_label_name)
                                            {
                                                text = g_Config.m_RobotWeaponsConsts[i].name;
                                                g_PopupWeapon[i].text = text;
                                                break;
                                            }
                                            else if(i == ROBOT_WEAPONS_COUNT) text = g_Config.m_RobotWeaponsConsts[0].name;
                                        }
                                    }
                                }

                                if(if_elem->m_strName == IF_MAIN_MENU_BUTTON  || if_elem->m_strName == IF_BASE_CONST_BUILD || if_elem->m_strName == IF_BASE_CONST_CANCEL || if_elem->m_strName == IF_ENTER_ARCADE_MODE || if_elem->m_strName == IF_EXIT_ARCADE_MODE)
                                {
                                    if_elem->m_StateImages[st].SetStateLabelParams(x - 1, y - 1, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize), align_x, align_y, perenos, sme_x, sme_y, CRect(clip_sx, clip_sy, clip_ex, clip_ey), text, font, 0xFF000000);
                                    if_elem->m_StateImages[st].SetStateText(false);
                                }

                                //Загружаем отрендеренные текстуры с названиями типов модулей и конкретных модулей на их места
                                //(да, это, блять, не текст, а заведомый пререндер на пустую альфа-пикчу в ресурсах!)
                                if_elem->m_StateImages[st].SetStateLabelParams(x, y, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize), align_x, align_y, perenos, sme_x, sme_y, CRect(clip_sx, clip_sy, clip_ex, clip_ey), text, font, color);
                                if_elem->m_StateImages[st].SetStateText(false);
                                
                                if(if_elem->m_strName == IF_ENTER_ARCADE_MODE)
                                {
                                    text = labels_file->Par(L"inro_part2");

                                    if_elem->m_StateImages[st].SetStateLabelParams(x - 1, y + 8, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize), align_x, align_y, perenos, sme_x, sme_y, CRect(clip_sx, clip_sy, clip_ex, clip_ey), text, font, 0xFF000000);
                                    if_elem->m_StateImages[st].SetStateText(false);

                                    if_elem->m_StateImages[st].SetStateLabelParams(x, y + 9, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize), align_x, align_y, perenos, sme_x, sme_y, CRect(clip_sx, clip_sy, clip_ex, clip_ey), text, font, color);
                                    if_elem->m_StateImages[st].SetStateText(false);
                                }
                            }
                            else if(labels->BlockGetName(bl_cnt) == IF_STATE_DYNAMIC_LABEL)
                            {
                                if_elem->m_StateImages[st].SetStateLabelParams(x, y, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize), align_x, align_y, perenos, sme_x, sme_y, CRect(clip_sx, clip_sy, clip_ex, clip_ey), CWStr(L""), font, color);
                            }
                        }
                    }
                }
            }
        }
	}

    if(m_strName == IF_MAIN)
    {
        g_IFaceList->CreateGroupSelection(this);
        g_IFaceList->CreateOrdersGlow(this);
    }

    SortElementsByZ();

    return need2save;
}

CIFaceElement* CInterface::DelElement(CIFaceElement* pElement)
{
DTRACE();

#if defined _TRACE || defined _DEBUG

    CIFaceElement *first = m_FirstElement;
    for(; first; first = first->m_NextElement)
    {
        if (first == pElement) break;
    }
    if(first == nullptr)
    {
        _asm int 3
    }
#endif

DCP();

    CIFaceElement* next = pElement->m_NextElement;
DCP();


    LIST_DEL(pElement, m_FirstElement, m_LastElement, m_PrevElement, m_NextElement);
DCP();

    HDelete(CIFaceElement, pElement, g_MatrixHeap);
DCP();

    return next;

}

bool CInterface::AddElement(CIFaceElement* pElement)
{
DTRACE();

    D3DXVECTOR3 dp(pElement->m_xPos + m_xPos, m_yPos + pElement->m_yPos, pElement->m_zPos);

    int nC = 0;
	while(nC < MAX_STATES && pElement->m_StateImages[nC].Set)
    {
        for(int i = 0; i < 4; ++i)
        {
			pElement->m_StateImages[nC].m_Geom[i].p.x += dp.x;
            pElement->m_StateImages[nC].m_Geom[i].p.y += dp.y;
            pElement->m_StateImages[nC].m_Geom[i].p.z += dp.z;
            
            pElement->m_PosElInX = dp.x;
            pElement->m_PosElInY = dp.y;
		}
    	++nC;
	}

	LIST_ADD(pElement, m_FirstElement, m_LastElement, m_PrevElement, m_NextElement);
    pElement->SetVisibility(false);
	return TRUE;
}

void CInterface::BeforeRender(void)
{
DTRACE();

#ifdef _DEBUG
    try
    {
#endif
DCP();
	    if(m_VisibleAlpha == IS_VISIBLE || m_AlwaysOnTop)
        {
DCP();
            CIFaceElement* pObjectsList = m_FirstElement;
DCP();
		    while(pObjectsList != nullptr)
            {
                //if(pObjectsList->m_strName == IF_POPUP_RAMKA) ASSERT(1);
DCP();
                if(pObjectsList->GetVisibility())
                {
DCP();
				    pObjectsList->BeforeRender();
DCP();
                }
DCP();
			    pObjectsList = pObjectsList->m_NextElement;
DCP();
		    }
            if(g_MatrixMap->GetPlayerSide()->m_CurrSel == BASE_SELECTED && g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive())
            {
DCP();
                g_MatrixMap->GetPlayerSide()->m_Constructor->BeforeRender();
            }
DCP();
	    }
#ifdef _DEBUG
    }
    catch (...)
    {
        _asm int 3

    }
#endif
}

void CInterface::Render()
{
DTRACE();

	if(m_VisibleAlpha == IS_VISIBLE || m_AlwaysOnTop)
    {
        CIFaceElement* pObjectsList = m_FirstElement;

		while(pObjectsList != nullptr)
        {
            if(pObjectsList->GetVisibility())
            {
				pObjectsList->Render(pObjectsList->m_VisibleAlpha);
                if(pObjectsList->m_strName == IF_BASE_CONSTRUCTION_RIGHT)
                {
                    if(g_MatrixMap->GetPlayerSide()->m_CurrSel == BASE_SELECTED && g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()){
                        g_MatrixMap->GetPlayerSide()->m_Constructor->Render();
                    }
                }
            }
			pObjectsList = pObjectsList->m_NextElement;
		}
	}
}

bool CInterface::OnMouseMove(CPoint mouse)
{
	DTRACE();

    if(g_MatrixMap->IsPaused())
    {
        if(m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU)
        {
            return false;
        }
    }
    bool bCatch = false;
    bool bCatchStatic = false;
    bool MiniMapFocused = false;
    bool static_have_hint = false;
    g_IFaceList->m_FocusedElement = nullptr;
    CWStr static_name(L"", g_CacheHeap);

	if(m_VisibleAlpha)
    {
        CIFaceElement* pObjectsList = m_FirstElement;
        while(pObjectsList != nullptr)
        {
            if(pObjectsList->GetVisibility() || pObjectsList->m_nId == POPUP_SELECTOR_CATCHERS_ID)
            {
                if((pObjectsList->m_Type == IFACE_STATIC || pObjectsList->m_Type == IFACE_DYNAMIC_STATIC))
                { 
                    if((pObjectsList->m_strName != IF_POPUP_SELECTOR && pObjectsList->m_nId != POPUP_SELECTOR_CATCHERS_ID) && (pObjectsList->OnMouseMove(mouse)/* || (pObjectsList->m_strName == IF_POPUP_RAMKA && pObjectsList->ElementCatch(mouse))*/))
                    {
                        g_IFaceList->m_FocusedElement = pObjectsList;
                        bCatchStatic = true;
                        static_name = pObjectsList->m_strName;
                        if(pObjectsList->m_Hint.HintTemplate != L"")
                        {
                            static_have_hint = true;
                        }
                    }
                    else
                    {
                        if(pObjectsList->m_nId == POPUP_SELECTOR_CATCHERS_ID && pObjectsList->ElementCatch(mouse))
                        {
                            //Выделяем конкретный элемент в выпадающем списке модулей конструктора
                            if(FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu && g_PopupMenu->Selector())
                            {
                                g_IFaceList->m_FocusedElement = pObjectsList;
                                bCatchStatic = true;
                                g_PopupMenu->SetSelectorPos(pObjectsList->m_xPos, pObjectsList->m_yPos, pObjectsList->m_iParam);
                            }
                        }
                    }
                }
                else
                {
                    if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && pObjectsList->OnMouseMove(mouse))
                    {
                        g_IFaceList->m_FocusedElement = pObjectsList;
                        bCatch = true;
                    }
                    else
                    {
                        pObjectsList->Reset();
                    }
                }
            }
			pObjectsList = pObjectsList->m_NextElement;
		}
	}

    //Клики мышью по миникарте, центрующие экран на указанную на миникарте точку
    if(!bCatch && FLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN) && FLAG(g_IFaceList->m_IfListFlags, MINIMAP_ENABLE_DRAG))
    {
        //Клик левой кнопкой мыши по миникарте (если не выбран приказа с панели управления роботом)
        if(!IS_PREORDERING_NOSELECT && (GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0x8000)
        {
            g_MatrixMap->m_Minimap.ButtonClick(nullptr);
            RESETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_ENABLE_DRAG);
        }
        //Клик правой кнопкой мыши по миникарте (если приказ с панели управления роботом был выбран (курсор изменил вид))
        else if(IS_PREORDERING_NOSELECT && (GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0x8000)
        {
            g_MatrixMap->m_Minimap.ButtonClick(nullptr);
            RESETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_ENABLE_DRAG);
        }
    }
    /*if(!bCatch && bCatchStatic)
    {
        if(g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName != static_name)
        {
            g_IFaceList->m_CurrentHint->Release(); 
            g_IFaceList->m_CurrentHint = nullptr; 
            g_IFaceList->m_CurrentHintControlName = L"";
        }
    }*/
    return (bCatch || bCatchStatic);
}

void CInterface::Reset()
{
    CIFaceElement* objects = m_FirstElement;
    while(objects)
    {
        objects->Reset();
        objects = objects->m_NextElement;
    }
}

void CInterface::SetAlpha(BYTE alpha)
{
    CIFaceElement* objects = m_FirstElement;
    while(objects)
    {
        objects->m_VisibleAlpha = alpha;
        objects = objects->m_NextElement;
    }
}

//Второй обработчик левого клика мышью по элементу интерфейса
bool CInterface::OnMouseLBDown()
{
    if(g_MatrixMap->IsPaused())
    {
        if(m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU)
        {
            return false;
        }
    }

    if(FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && !(g_PopupMenu && g_PopupMenu->GetRamka() && g_PopupMenu->GetRamka()->ElementCatch(g_MatrixMap->m_Cursor.GetPos())))
    {
        if(g_PopupMenu) g_PopupMenu->ResetMenu(true);
        return false;
    }

    if(FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu /* && (g_IFaceList->m_FocusedElement && g_IFaceList->m_FocusedElement->m_nId == POPUP_SELECTOR_CATCHERS_ID)*/)
    {
        g_PopupMenu->OnMenuItemPress();
        return true;
    }

	if(m_VisibleAlpha)
    {
        CIFaceElement* pObjectsList = m_LastElement;
		while(pObjectsList != nullptr)
        {
            if(pObjectsList->GetVisibility())
            {
                if(pObjectsList->ElementCatch(g_MatrixMap->m_Cursor.GetPos()))
                {
                    if(pObjectsList->m_nId == PERSONAL_ICON_ID)
                    {
                        pObjectsList->OnMouseLBDown();
                    }
                    else if(IS_GROUP_ICON(pObjectsList->m_nId))
                    {
                        CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

                        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SHIFT]) & 0x8000) == 0x8000)
                        {
                            CMatrixMapStatic* o = ps->GetCurGroup()->GetObjectByN(pObjectsList->m_nId - GROUP_ICONS_ID);
                            ps->RemoveObjectFromSelectedGroup(o);
                        }
                        else
                        {
                            if(g_MatrixMap->GetPlayerSide()->GetCurSelNum() == pObjectsList->m_nId - GROUP_ICONS_ID)
                            {
                                if(ps->GetCurSelObject() && ps->GetCurSelObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI)
                                {
                                    ps->CreateGroupFromCurrent(ps->GetCurSelObject());
                                    ps->Select(ROBOT, nullptr);
                                }
                                else if(ps->GetCurSelObject() && ps->GetCurSelObject()->GetObjectType() == OBJECT_TYPE_FLYER)
                                {
                                    ps->CreateGroupFromCurrent(ps->GetCurSelObject());
                                    ps->Select(FLYER, nullptr);
                                }
                            }
                            else
                            {
                                g_MatrixMap->GetPlayerSide()->SetCurSelNum(pObjectsList->m_nId - GROUP_ICONS_ID);
                            }
                        }
                        return true;
                    }
                    else if(IS_QUEUE_ICON(pObjectsList->m_nId))
                    {
                        CIFaceElement* ne = pObjectsList->m_NextElement;
                        CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
                        ((CMatrixBuilding*)ps->m_ActiveObject)->m_BS.DeleteItem((pObjectsList->m_nId - QUEUE_ICON) + 1);
                        pObjectsList = ne;
                        return true;
                    }
                    else if(pObjectsList->m_strName == IF_MAP_PANEL)
                    {
                        if(IS_PREORDERING_NOSELECT)
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN);
                            g_MatrixMap->GetPlayerSide()->OnLButtonDown(CPoint(0, 0));
                        }
                        else
                        {
                            SETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN);
                        }
                    }
                    else if(pObjectsList->ElementAlpha(g_MatrixMap->m_Cursor.GetPos()) && pObjectsList->OnMouseLBDown())
                    {
				        if(pObjectsList->m_Type == IFACE_CHECK_BUTTON || pObjectsList->m_Type == IFACE_CHECK_BUTTON_SPECIAL)
                        {
					        pObjectsList->CheckGroupReset(m_FirstElement, pObjectsList);
				        }
				        return true;
                    }
                }
            }
			pObjectsList = pObjectsList->m_PrevElement;
		}
	}

	return false;
}

//Второй обработчик левого отклика мышью по элементу интерфейса
void CInterface::OnMouseLBUp()
{
    //Если включён режим паузы, игнорим все отклики мыши, кроме откликов по миникарте и прочим некоторым штукам (не вполне уверен каким именно)
    if(g_MatrixMap->IsPaused())
    {
        if(m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU)
        {
            return;
        }
    }

    //Если ранее игрок зажал LMB на миникарте, то сейчас у него включён режим перетягивания камеры по ней
    if(FLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN))
    {
        //Отключаем этот режим
        RESETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN);
    }

    //Ищем, по какому именно видимому (активному) элементу интерфейса на экране был сделан отклик мыши и запускаем его обработчик
    if(m_VisibleAlpha)
    {
        CIFaceElement* pObjectsList = m_LastElement;
        while(pObjectsList != nullptr)
        {
            if(pObjectsList->GetVisibility() && pObjectsList->ElementCatch(g_MatrixMap->m_Cursor.GetPos()) && pObjectsList->ElementAlpha(g_MatrixMap->m_Cursor.GetPos()))
            {
                pObjectsList->OnMouseLBUp(); //Вызываем последний третий обработчик отклика с идентичным названием

                if(pObjectsList->m_Type == IFACE_CHECK_PUSH_BUTTON)
                {
                    pObjectsList->CheckGroupReset(m_FirstElement, pObjectsList);
                }

                return;
            }
            pObjectsList = pObjectsList->m_PrevElement;
        }
    }
}

//Второй обработчик правого клика мышью по элементу интерфейса
bool CInterface::OnMouseRBDown()
{
    if(g_MatrixMap->IsPaused())
    {
        if(m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU)
        {
            return false;
        }
    }

    if(m_VisibleAlpha)
    {
        CIFaceElement* pObjectsList = m_LastElement;
		while(pObjectsList != nullptr)
        {
            if(pObjectsList->GetVisibility())
            {
                if(pObjectsList->ElementCatch(g_MatrixMap->m_Cursor.GetPos()))
                {
                    //Выставление маркера движения для робота, если игрок кликнул правой кнопкой по миникарте
                    if(pObjectsList->m_strName == IF_MAP_PANEL)
                    {
                        g_MatrixMap->GetPlayerSide()->OnRButtonDown(CPoint(0, 0));
                        if(IS_PREORDERING_NOSELECT)
                        {
                            SETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN);
                        }
                    }
                    //Правый клик по кнопке интерфейса
                    else if(pObjectsList->ElementAlpha(g_MatrixMap->m_Cursor.GetPos()) && pObjectsList->OnMouseRBDown())
                    {
                        /*
                        if(pObjectsList->m_Type == IFACE_CHECK_BUTTON || pObjectsList->m_Type == IFACE_CHECK_BUTTON_SPECIAL)
                        {
                            pObjectsList->CheckGroupReset(m_FirstElement, pObjectsList);
                        }
                        */
                        
                        //Для правого клика учитываем только конкретные кнопки, в данном случае кнопки подрыва бомбы
                        if(pObjectsList->m_strName == IF_ORDER_BOMB || pObjectsList->m_strName == IF_ORDER_BOMB_AUTO) return true;
                        else if(pObjectsList->m_strName == IF_MAIN_SELF_BOMB || pObjectsList->m_strName == IF_MAIN_SELF_BOMB_AUTO) return true;
                    }
                    /*
                    else
                    {
                        //other elements
                        pObjectsList->OnMouseRBDown();
                    }
                    */
                }
            }
            pObjectsList = pObjectsList->m_PrevElement;
        }
    }

    return false;
}

//Второй обработчик правого отклика мышью по элементу интерфейса
void CInterface::OnMouseRBUp()
{
    //Если включён режим паузы, игнорим все отклики мыши, кроме откликов по миникарте и прочим некоторым штукам (не вполне уверен каким именно)
    /*
    if(g_MatrixMap->IsPaused())
    {
        if(m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU)
        {
            return;
        }
    }
    */

    //Ищем, по какому именно видимому (активному) элементу интерфейса на экране был сделан отклик мыши и запускаем его обработчик
    if(m_VisibleAlpha)
    {
        CIFaceElement* pObjectsList = m_LastElement;
        while(pObjectsList != nullptr)
        {
            if(pObjectsList->GetVisibility() && pObjectsList->ElementCatch(g_MatrixMap->m_Cursor.GetPos()) && pObjectsList->ElementAlpha(g_MatrixMap->m_Cursor.GetPos()))
            {
                //Для правого клика учитываем только конкретные кнопки, в данном случае кнопки подрыва бомбы
                if(pObjectsList->m_strName == IF_ORDER_BOMB || pObjectsList->m_strName == IF_ORDER_BOMB_AUTO) pObjectsList->OnMouseRBUp(); //Вызываем последний обработчик отклика
                else if(pObjectsList->m_strName == IF_MAIN_SELF_BOMB || pObjectsList->m_strName == IF_MAIN_SELF_BOMB_AUTO) pObjectsList->OnMouseRBUp(); //Вызываем последний обработчик отклика

                if(pObjectsList->m_Type == IFACE_CHECK_PUSH_BUTTON)
                {
                    pObjectsList->CheckGroupReset(m_FirstElement, pObjectsList);
                }

                return;
            }

            pObjectsList = pObjectsList->m_PrevElement;
        }
    }
}

void CInterface::Init(void)
{
    int nC = 0;
	CIFaceElement* pElement = m_FirstElement;
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

	if(m_AlwaysOnTop)
    {
        m_VisibleAlpha = IS_VISIBLE;
        
        if(m_strName == IF_MINI_MAP)
        {
            while(pElement)
            {
                if(player_side->IsArcadeMode())
                {
                    pElement->SetVisibility(false);
                }
                else
                {
                    pElement->SetVisibility(true);
                }
                if(pElement->m_strName == IF_MAP_ZOOM_IN)
                {
                    if(g_MatrixMap->m_Minimap.GetCurrentScale() == MINIMAP_MAX_SCALE)
                    {
                        pElement->SetState(IFACE_DISABLED);
                    }
                    else if(g_MatrixMap->m_Minimap.GetCurrentScale() < MINIMAP_MAX_SCALE && pElement->GetState() == IFACE_DISABLED)
                    {
                        pElement->SetState(IFACE_NORMAL);
                    }
                }
                else if(pElement->m_strName == IF_MAP_ZOOM_OUT)
                {
                    if(g_MatrixMap->m_Minimap.GetCurrentScale() == MINIMAP_MIN_SCALE)
                    {
                        pElement->SetState(IFACE_DISABLED);
                    }
                    else if(g_MatrixMap->m_Minimap.GetCurrentScale() > MINIMAP_MIN_SCALE && pElement->GetState() == IFACE_DISABLED)
                    {
                        pElement->SetState(IFACE_NORMAL);
                    }
                }
                else if(pElement->m_strName == IF_SHOWROBOTS_BUTT)
                {
                    if((!player_side->GetSideRobots() || !player_side->GetMaxSideRobots()))
                    {
                        pElement->SetState(IFACE_DISABLED);
                    }
                    else if(pElement->GetState() == IFACE_DISABLED && !(!player_side->GetSideRobots() || !player_side->GetMaxSideRobots()))
                    {
                        pElement->SetState(IFACE_NORMAL);
                    }
                }

                pElement = pElement->m_NextElement;   
            }
        }
        else if(m_strName == IF_MAIN)
        {
            CPoint pl[MAX_PLACES];

            int objects_cnt = 0;
            int robots = 0;

            bool rsel = false;
            bool gsel = false;            
            bool ordering = FLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
            bool singlem = FLAG(g_IFaceList->m_IfListFlags, SINGLE_MODE) || player_side->IsRobotArcadeMode();
            bool bld_tu = ordering && FLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET);
            bool bld_he = ordering && FLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_FLYER);
            bool bld_re = ordering && FLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_REPAIR);

            CMatrixGroup* work_group = player_side->GetCurGroup();
            CMatrixRobotAI* sel_bot = nullptr;
            if(work_group && work_group->GetObjectsCnt())
            {
                objects_cnt = work_group->GetObjectsCnt();
                robots = work_group->GetRobotsCnt();
                gsel = true;
                rsel = ((objects_cnt == 1) && (work_group->m_FirstObject->GetObject()->IsRobotAlive()));
                sel_bot = (CMatrixRobotAI*)work_group->m_FirstObject->GetObject();
            }

            bool bombers = false;
            bool auto_bombers = false;
            bool repairers = false;
            bool heliors = false;

            bool stop = false;
            bool move = false;
            bool fire = false;
            bool capt = false;
            bool bomb = false;
            bool patrol = false;
            bool repair = false;
            bool getup = false;
            bool drop = false;
            
            bool bomber_sel = false;
            bool repairer_sel = false;
            bool heli_sel = false;
            bool robot_sel = false;
            bool new_name = false;
            bool new_lives = false;
            bool new_speed = false;
            bool new_weight = false;
            bool new_turmax = false;
            bool new_turhave = false;
            bool cant_build_turret = false;

            if(
                !player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_LIGHT_CANNON]) &&
                !player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_HEAVY_CANNON]) &&
                !player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_LASER_CANNON]) &&
                !player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_MISSILE_CANNON])
              ) cant_build_turret = true;

            CMatrixRobotAI* cur_r = nullptr;
            CMatrixFlyer* cur_f = nullptr;

            if(player_side->IsRobotArcadeMode())
            {
                cur_r = (CMatrixRobotAI*)player_side->GetArcadedObject();
                robot_sel = true;
                if(!sel_bot) sel_bot = player_side->GetArcadedObject()->AsRobot();
            }

            if(!sel_bot && singlem)
            {
                singlem = false;
                RESETFLAG(g_IFaceList->m_IfListFlags, SINGLE_MODE);
            }

            if(work_group)
            {
                CMatrixGroupObject* go = work_group->m_FirstObject;

                if(gsel)
                {
                    for(int i = 0; i < player_side->GetCurSelNum() && go; ++i) go = go->m_NextObject;

                    if(go)
                    {
                        CMatrixRobotAI* r = (CMatrixRobotAI*)go->GetObject();
                        if(go->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI)
                        {
                            robot_sel = true;
                            cur_r = (CMatrixRobotAI*)go->GetObject();
                            if(r->FindBombWeapon()) bomber_sel = true;
                            if(r->FindRepairWeapon()) repairer_sel = true;
                        }
                    }
                }
            }

            if(work_group)
            {
                g_MatrixMap->GetSideById(PLAYER_SIDE)->ShowOrderState();
                if(gsel)
                {
                    int bombers_cnt = 0;
                    int repairers_cnt = 0;  

                    CMatrixGroupObject *objs = work_group->m_FirstObject;
                    while(objs)
                    {
                        CMatrixRobotAI *robot = objs->GetObject()->AsRobot();
                        if(objs->GetObject()->IsRobot())
                        {
                            if(robot->FindBombWeapon())
                            {
                                ++bombers_cnt;
                                if(robot->AutoBoomSet()) auto_bombers = true;
                            }
                            if(robot->FindRepairWeapon()) ++repairers_cnt;

                            int bot_order = g_MatrixMap->GetSideById(robot->GetSide())->m_PlayerGroup[robot->GetGroupLogic()].Order();
                            if(robot->GetGroupLogic() >= 0 && bot_order == mpo_MoveTo)
                            {//if(robot->FindOrderLikeThat(ROT_MOVE_TO)){
                                move = true;
                            }
                            if(robot->GetGroupLogic() >= 0 && bot_order == mpo_Capture)
                            {//if(robot->FindOrderLikeThat(ROT_CAPTURE_BUILDING)){
                                capt = true;
                            }
                            if(robot->GetGroupLogic() >= 0 && bot_order == mpo_Attack)
                            {//if(robot->FindOrderLikeThat(ROBOT_FIRE)){
                                fire = true;
                            }
                            if(robot->GetGroupLogic() >= 0 && bot_order == mpo_Stop)
                            {//if(!robot->GetOrdersInPool()){
                                stop = true;
                            }
                            if(robot->GetGroupLogic() >= 0 && bot_order == mpo_Patrol)
                            {//if(!robot->GetOrdersInPool()){
                                patrol = true;
                            }
                            if(robot->GetGroupLogic() >= 0 && bot_order == mpo_Bomb)
                            {//if(robot->FindOrderLikeThat(ROT_MOVE_TO)){
                                bomb = true;
                            }
                            if(robot->GetGroupLogic() >= 0 && bot_order == mpo_Repair)
                            {//if(robot->FindOrderLikeThat(ROT_MOVE_TO)){
                                repair = true;
                            }
                        }
                        objs = objs->m_NextObject;
                    }

                    if(bombers_cnt == objects_cnt) bombers = true;
                    if(repairers_cnt == objects_cnt) repairers = true;
                }
            }

            while(pElement)
            {
//Always visible elements////////////////////////////////////////////////////////////////////////////////////
                if(pElement->m_strName == IF_NAME_LABEL)
                {
                    if(gsel || player_side->IsArcadeMode())
                    {
                        if((rsel || robot_sel) && cur_r)
                        {
                            if(name != cur_r->m_Name)
                            {
                                name = cur_r->m_Name;
                                new_name = true;
                            }
                        }
                    }
                    else if(player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED)
                    {
                        CBlockPar* bp_tmp = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Buildings");
                        if(player_side->m_ActiveObject && player_side->m_ActiveObject->AsBuilding()->IsBase() && name != bp_tmp->ParGet(L"Base_Name"))
                        {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Base_Name");
                        }
                        else if(player_side->m_ActiveObject && player_side->m_ActiveObject->AsBuilding()->m_Kind == BUILDING_TITAN && name != bp_tmp->ParGet(L"Titan_Name"))
                        {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Titan_Name");
                        }
                        else if(player_side->m_ActiveObject && player_side->m_ActiveObject->AsBuilding()->m_Kind == BUILDING_ELECTRONIC && name != bp_tmp->ParGet(L"Electronics_Name"))
                        {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Electronics_Name");
                        }
                        else if(player_side->m_ActiveObject && player_side->m_ActiveObject->AsBuilding()->m_Kind == BUILDING_ENERGY && name != bp_tmp->ParGet(L"Energy_Name"))
                        {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Energy_Name");
                        }
                        else if(player_side->m_ActiveObject && player_side->m_ActiveObject->AsBuilding()->m_Kind == BUILDING_PLASMA && name != bp_tmp->ParGet(L"Plasma_Name"))
                        {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Plasma_Name");
                        }
                    }
                }
                else if(pElement->m_strName == IF_LIVES_LABEL)
                {
                    if(gsel || player_side->IsArcadeMode())
                    {
                        if((rsel || robot_sel) && cur_r)
                        {
                            if(lives != cur_r->GetHitPoint())
                            {
                                lives = cur_r->GetHitPoint();
                                max_lives = cur_r->GetMaxHitPoint();
                                new_lives = true;
                            }
                        }
                    }
                    else if(player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED)
                    {
                        if(lives != player_side->m_ActiveObject->AsBuilding()->GetHitPoint())
                        {
                            lives = player_side->m_ActiveObject->AsBuilding()->GetHitPoint();
                            max_lives = player_side->m_ActiveObject->AsBuilding()->GetMaxHitPoint();
                            new_lives = true;
                        }
                    }
                }
                else if(pElement->m_strName == IF_TURRETS_MAX)
                {
                    if(player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED)
                    {
                        if(player_side->m_ActiveObject->AsBuilding()->m_TurretsMax != turmax)
                        {
                            turmax = player_side->m_ActiveObject->AsBuilding()->m_TurretsMax;
                            new_turmax = true;
                        }
                    }
                }
                else if(pElement->m_strName == IF_TURRETS_HAVE)
                {
                    if(player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED)
                    {
                        CMatrixBuilding* bld = (CMatrixBuilding*)player_side->m_ActiveObject;
                        if(bld->GetPlacesForTurrets(pl) != turhave)
                        {
                            turhave = bld->GetPlacesForTurrets(pl);
                            new_turhave = true;
                        }
                    }
                }

//Invisible by default elements///////////////////////////////////////////////////////////////////////////////
                if(pElement->m_Type == IFACE_DYNAMIC_STATIC && IS_GROUP_ICON(pElement->m_nId))
                {
                    pElement->SetVisibility(true);
                }
                else if(pElement->m_nId == PERSONAL_ICON_ID)
                {
                    pElement->SetVisibility(true);
                }
                else if(pElement->m_nId == DYNAMIC_TURRET)
                {
                    pElement->SetVisibility(true);
                }
                else
                {
                    pElement->SetVisibility(false);                
                }


                if(pElement->m_strName == IF_MAIN_PANEL1)
                {
                    pElement->SetVisibility(true);
                }
                else if(pElement->m_strName == IF_MAIN_PANEL2)
                {
                    pElement->SetVisibility(true);
                }

                if(pElement->m_strName == IF_NAME_LABEL && player_side->m_CurrSel != NOTHING_SELECTED)
                {
                    if(new_name)
                    {
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = name;
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                    pElement->SetVisibility(true);
                }
                else if(player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED)
                {
                    CMatrixBuilding* bld = player_side->m_ActiveObject->AsBuilding();
                    CBlockPar* bp_tmp = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Buildings");
                    int income = player_side->GetIncomePerTime(int(bld->m_Kind), 60000);                    
                    if(pElement->m_strName == IF_BUILDING_OPIS)
                    {
                        if(!bld->m_BS.GetItemsCnt())
                        {
                            pElement->SetVisibility(true);
                        }
                        if(btype != int(bld->m_Kind))
                        {
                            btype = int(bld->m_Kind);
                            if(bld->m_Kind == BUILDING_BASE)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Base_Descr");
                            }
                            else if(bld->m_Kind == BUILDING_TITAN)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Titan_Descr");
                            }
                            else if(bld->m_Kind == BUILDING_ELECTRONIC)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Electronics_Descr");
                            }
                            else if(bld->m_Kind == BUILDING_ENERGY)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Energy_Descr");
                            }
                            else if(bld->m_Kind == BUILDING_PLASMA)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Plasma_Descr");
                            }
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                    }
                    else if(pElement->m_strName == IF_BASE_RES_INC && bld->m_Kind == BUILDING_BASE)
                    {
                        if(!bld->m_BS.GetItemsCnt())
                        {
                            pElement->SetVisibility(true);
                        }
                        
                        if(income != base_res_income)
                        {
                            base_res_income = income;
                            CWStr suck(bp_tmp->ParGet(L"ResPer"), g_CacheHeap);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = suck.Replace(CWStr(L"<resources>", g_CacheHeap), L"<Color=247,195,0>" + CWStr(base_res_income, g_CacheHeap) + L"</Color>");
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                    }
                    else if(pElement->m_strName == IF_FACTORY_RES_INC && bld->m_Kind != BUILDING_BASE)
                    {
                        if(!bld->m_BS.GetItemsCnt())
                        {
                            pElement->SetVisibility(true);
                        }
                        if(income != factory_res_income)
                        {
                            factory_res_income = income;
                            CWStr suck(bp_tmp->ParGet(L"ResPer"), g_CacheHeap);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = suck.Replace(CWStr(L"<resources>", g_CacheHeap), L"<Color=247,195,0>" + CWStr(factory_res_income, g_CacheHeap) + L"</Color>");
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                    }
                }
                if(pElement->m_strName == IF_LIVES_LABEL && player_side->m_CurrSel != NOTHING_SELECTED)
                {
                    if(new_lives)
                    {
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(Float2Int(lives)).Add(L"/").Add(CWStr(Float2Int(max_lives)));
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                    pElement->SetVisibility(true);
                }


                if(singlem)
                {
                    if(pElement->m_strName == IF_MANUAL_BG)
                    {
                        if(!sel_bot->IsManualControlLocked())
                        {
                            pElement->SetVisibility(true);
                        }
                    }
                    else if(pElement->m_strName == IF_ENTER_ARCADE_MODE)
                    {
                        if(!player_side->IsArcadeMode() && !sel_bot->IsManualControlLocked())
                        {
                            pElement->SetVisibility(true);
                        }
                    }
                    else if(pElement->m_nId == DYNAMIC_WEAPON_ON_ID)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_strName == IF_EXIT_ARCADE_MODE)
                    {
                        if(player_side->IsArcadeMode())
                        {
                            pElement->SetVisibility(true);
                        }
                    }
                    else if(pElement->m_strName == IF_OVER_HEAT && sel_bot)
                    {
                        pElement->SetVisibility(true);
                        SMatrixRobotModule* module = &sel_bot->m_Module[Float2Int(pElement->m_Param1)];
                        
                        for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
                        {
                            if(sel_bot->GetWeapon(i).m_Module == module)
                            {
                                pElement->m_VisibleAlpha = byte(sel_bot->GetWeapon(i).m_Heat * 0.25f);                        
                            }
                        }
                    }
                    else if(pElement->m_strName == IF_MAIN_WEAPONSLOTS)
                    {
                        pElement->SetVisibility(true);
                    }
                }

                if(player_side->IsArcadeMode())
                {
                    if(bombers)
                    {
                        if(!auto_bombers)
                        {
                            if(pElement->m_strName == IF_MAIN_SELF_BOMB) pElement->SetVisibility(true);
                        }
                        else
                        {
                            if(pElement->m_strName == IF_MAIN_SELF_BOMB_AUTO) pElement->SetVisibility(true);
                        }
                    }
                }

                if(work_group)
                {
                    if(pElement->m_Type == IFACE_DYNAMIC_STATIC && IS_SELECTION(pElement->m_nId))
                    {
                        int n = pElement->m_nId - GROUP_SELECTION_ID;
                        if(n < work_group->GetObjectsCnt())
                        {
                            CMatrixGroupObject* so = work_group->m_FirstObject;
                            int i;
                            for(i = 0; i < n && so; ++i)
                            {
                                so = so->m_NextObject;
                            }

                            if(so)
                            {
                                if(so->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI)
                                {
                                    if(i == player_side->GetCurSelNum())
                                    {
                                        ((CMatrixRobotAI*)so->GetObject())->CreateProgressBarClone(m_xPos + 68, m_yPos + 179, 68, PBC_CLONE2);
                                    }

                                    if(!singlem)
                                    {
                                        ((CMatrixRobotAI*)so->GetObject())->CreateProgressBarClone(pElement->m_xPos + m_xPos, pElement->m_yPos + m_yPos + 36, 46, PBC_CLONE1);
                                    }
                                    else
                                    {
                                        ((CMatrixRobotAI*)so->GetObject())->DeleteProgressBarClone(PBC_CLONE1);
                                    }
                                }
                            }

                            if(!singlem) pElement->SetVisibility(true);
                        }
                        if(singlem) pElement->SetVisibility(false);
                    }
                }
                else if(player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED)
                {
                    CMatrixBuilding* bld = (CMatrixBuilding*)player_side->m_ActiveObject;
                    bld->CreateProgressBarClone(m_xPos + 68, m_yPos + 179, 68, PBC_CLONE2);
                    
                    if(bld->m_Kind == BUILDING_TITAN && pElement->m_strName == IF_TITAN_PLANT)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_PLASMA && pElement->m_strName == IF_PLASMA_PLANT)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_ELECTRONIC && pElement->m_strName == IF_ELECTRO_PLANT)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_ENERGY && pElement->m_strName == IF_ENERGY_PLANT)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_REPAIR && (pElement->m_strName == IF_REPAIR_PLANT || (pElement->m_strName == IF_BUILD_REPAIR && !bld_tu && !bld_re)))
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_BASE && pElement->m_strName == IF_BASE_PLANT)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_BS.GetItemsCnt() && (pElement->m_strName == IF_QUEUE_ICON || pElement->m_strName == IF_QUEUE_OTHER || (IS_QUEUE_ICON(pElement->m_nId) && pElement->m_iParam == int(bld))))
                    {
                        pElement->SetVisibility(true);
                    }
                    else  if(pElement->m_strName == IF_BASE_LINE)
                    {
                        pElement->SetVisibility(true);
                    }
                    //Кнопка открытия конструктора роботов
                    else if(pElement->m_strName == IF_BUILD_ROBOT && !bld_tu && !bld_he)
                    {
                        //Делаем видимой, если выбрана строительная база
                        if(bld->m_Kind == BUILDING_BASE)
                        {
                            pElement->SetVisibility(true);
                        }
                    }
                    //Кнопка выбора типа турели для постройки
                    else if(pElement->m_strName == IF_BUILD_TURRET && !bld_tu && !bld_he)
                    {
                        //Делаем видимой, если выбрано любое здание
                        pElement->SetVisibility(true);

                        //Проверяем лимит пушек для данного здания и блокируем кнопку, если лимит превышен
                        if(!bld->GetPlacesForTurrets(pl) || cant_build_turret || bld->m_BS.IsMaxItems())
                        {
                            pElement->SetState(IFACE_DISABLED);
                        }
                        else if(bld->GetPlacesForTurrets(pl) && !cant_build_turret && !bld->m_BS.IsMaxItems() && pElement->GetState() == IFACE_DISABLED)
                        {
                            pElement->SetState(IFACE_NORMAL);
                        }
                    }
                    //Кнопка выбора типа вертолёта для постройки
                    else if(g_EnableFlyers && pElement->m_strName == IF_BUILD_FLYER && !bld_tu && !bld_he)
                    {
                        //Делаем видимой, если выбрана база
                        if(bld->m_Kind == BUILDING_BASE)
                        {
                            pElement->SetVisibility(true);
                        }
                    }
                    /*
                    else if(pElement->m_strName == IF_BUILD_REPAIR && !bld_tu && !bld_he && !bld_re)
                    {
                        if(bld->m_Kind == BUILDING_BASE)
                        {
                            pElement->SetVisibility(true);
                        }
                    }
                    */
                    else if(pElement->m_strName == IF_CALL_FROM_HELL && !bld_tu && !bld_he)
                    {
                        pElement->SetVisibility(true);
                        if(g_MatrixMap->ReinforcementsDisabled() || g_MatrixMap->BeforeReinforcementsTime() > 0)
                        {
                            pElement->SetState(IFACE_DISABLED);
                        }
                        else if(pElement->GetState() == IFACE_DISABLED)
                        {
                            pElement->SetState(IFACE_NORMAL);
                        }
                    }
                    else if(bld->m_TurretsMax == 1 && pElement->m_strName == IF_PODL1)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_TurretsMax == 2 && pElement->m_strName == IF_PODL2)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_TurretsMax == 3 && pElement->m_strName == IF_PODL3)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_TurretsMax == 4 && pElement->m_strName == IF_PODL4)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->IsBase() && !bld->m_BS.GetItemsCnt() && pElement->m_strName == IF_MB_RES)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_TITAN && !bld->m_BS.GetItemsCnt() && pElement->m_strName == IF_TF_RES)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_ELECTRONIC && !bld->m_BS.GetItemsCnt() && pElement->m_strName == IF_ELF_RES)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_ENERGY && !bld->m_BS.GetItemsCnt() && pElement->m_strName == IF_ENF_RES)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(bld->m_Kind == BUILDING_PLASMA && !bld->m_BS.GetItemsCnt() && pElement->m_strName == IF_PF_RES)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_strName == IF_ZAGLUSHKA1)
                    {
                        pElement->SetVisibility(true);
                    }
                }

                //Делаем видимой панель приказов робота
                if(!ordering && !bld_tu && !bld_he)
                {
                    if(gsel)
                    {
                        if(pElement->m_strName == IF_ORDER_STOP)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(pElement->m_strName == IF_ORDER_MOVE)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(pElement->m_strName == IF_ORDER_PATROL)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(pElement->m_strName == IF_ORDER_FIRE)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(FLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON) && pElement->m_strName == IF_AORDER_ATTACK_ON)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(!FLAG(g_IFaceList->m_IfListFlags, AUTO_ATTACK_ON) && pElement->m_strName == IF_AORDER_ATTACK_OFF)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(FLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON) && pElement->m_strName == IF_AORDER_PROTECT_ON)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(!FLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON) && pElement->m_strName == IF_AORDER_PROTECT_OFF)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(player_side->GetCurGroup()->GetRobotsCnt() && FLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON) && pElement->m_strName == IF_AORDER_CAPTURE_ON)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(player_side->GetCurGroup()->GetRobotsCnt() && !FLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON) && pElement->m_strName == IF_AORDER_CAPTURE_OFF)
                        {
                            pElement->SetVisibility(true);
                        }
                        else if(pElement->m_nId == GROUP_SELECTOR_ID && !singlem)
                        {
                            pElement->SetVisibility(true);
                        }
                        
                        if(IS_ORDER_GLOW(pElement->m_nId))
                        {
                            if(pElement->m_nId - ORDERS_GLOW_ID == 0 && stop) pElement->SetVisibility(true);
                            if(pElement->m_nId - ORDERS_GLOW_ID == 1 && move && !capt && !getup && !drop && !bomb && !repair) pElement->SetVisibility(true);
                            if(pElement->m_nId - ORDERS_GLOW_ID == 2 && patrol) pElement->SetVisibility(true);
                            if(pElement->m_nId - ORDERS_GLOW_ID == 3 && fire) pElement->SetVisibility(true);
                            if(pElement->m_nId - ORDERS_GLOW_ID == 4 && (capt || getup || drop)) pElement->SetVisibility(true);
                            if(pElement->m_nId - ORDERS_GLOW_ID == 5 && (bomber_sel || repairer_sel) && (bomb || repair)) pElement->SetVisibility(true);
                        }

                        if(pElement->m_strName == IF_MAIN_PROG)
                        {
                            pElement->SetVisibility(true);
                        }
                        
                        if(pElement->m_strName == IF_ORDER_CAPTURE)
                        {
                            pElement->SetVisibility(true);
                        }
                        
                        //Только для роботов с бомбой
                        if(bomber_sel)
                        {
                            if(pElement->m_strName == IF_ORDER_BOMB)
                            {
                                if(!g_PlayerRobotsAutoBoom)
                                {
                                    pElement->SetVisibility(true);
                                    //Просто поставить continue здесь нельзя - будет виснуть
                                    goto skipAutoBoom;
                                }

                                //Перебор группы юнитов, выбранной игроком
                                CMatrixGroupObject* obj = work_group->m_FirstObject;
                                while(obj)
                                {
                                    CMatrixMapStatic* unit = obj->GetObject();
                                    //Если хоть у одного робота из группы ещё не выставлен автоподрыв
                                    if(unit->AsRobot()->HaveBomb() && !unit->AsRobot()->AutoBoomSet())
                                    {
                                        pElement->SetVisibility(true);
                                        break;
                                    }
                                    obj = obj->m_NextObject;
                                }
                            }
                            else if(g_PlayerRobotsAutoBoom && pElement->m_strName == IF_ORDER_BOMB_AUTO)
                            {
                                bool set_vis = true;

                                //Перебор группы юнитов, выбранной игроком
                                CMatrixGroupObject* obj = work_group->m_FirstObject;
                                while(obj)
                                {
                                    CMatrixMapStatic* unit = obj->GetObject();
                                    //Если хоть у одного робота из группы ещё не выставлен автоподрыв, то эту кнопку не отображаем
                                    if(unit->AsRobot()->HaveBomb() && !unit->AsRobot()->AutoBoomSet())
                                    {
                                        set_vis = false;
                                        break;
                                    }
                                    //obj = obj->GetNextLogic();
                                    obj = obj->m_NextObject;
                                }

                                if(set_vis) pElement->SetVisibility(true);
                            }
                        }

                        skipAutoBoom:

                        //Только для роботов с ремонтником
                        if(repairer_sel)
                        {
                            if(pElement->m_strName == IF_ORDER_REPAIR)
                            {
                                pElement->SetVisibility(true);
                            }
                        }
                    }
                }
                else
                {
                    if(pElement->m_strName == IF_ORDER_CANCEL)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_strName == IF_ZAGLUSHKA1)
                    {
                        pElement->SetVisibility(true);
                    }

                    if(gsel)
                    {
                        if(pElement->m_nId == GROUP_SELECTOR_ID && !singlem)
                        {
                           pElement->SetVisibility(true);
                        }
                    }

                    if(player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED)
                    {
                        CMatrixBuilding* bld = (CMatrixBuilding*)player_side->m_ActiveObject;
                        
                        //Нажали кнопку выбора типа турели для постройки
                        if(bld_tu && !(player_side->m_CurrentAction == BUILDING_TURRET))
                        {
                            if(pElement->m_strName == IF_BUILD_TUR1)
                            {
                                pElement->SetVisibility(true);
                                bool enough_resources = player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_LIGHT_CANNON]);
                                if(!bld->GetPlacesForTurrets(pl) || !enough_resources)
                                {
                                    pElement->SetState(IFACE_DISABLED);
                                }
                                else if(bld->GetPlacesForTurrets(pl) && pElement->GetState() == IFACE_DISABLED)
                                {
                                    pElement->SetState(IFACE_NORMAL);
                                }
                            }
                            else if(pElement->m_strName == IF_BUILD_TUR2)
                            {
                                pElement->SetVisibility(true);
                                bool enough_resources = player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_HEAVY_CANNON]);
                                if(!bld->GetPlacesForTurrets(pl) || !enough_resources)
                                {
                                    pElement->SetState(IFACE_DISABLED);
                                }
                                else if(bld->GetPlacesForTurrets(pl) && pElement->GetState() == IFACE_DISABLED)
                                {
                                    pElement->SetState(IFACE_NORMAL);
                                }
                            }
                            else if(pElement->m_strName == IF_BUILD_TUR3)
                            {
                                pElement->SetVisibility(true);
                                bool enough_resources = player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_LASER_CANNON]);
                                if(!bld->GetPlacesForTurrets(pl) || !enough_resources)
                                {
                                    pElement->SetState(IFACE_DISABLED);
                                }
                                else if(bld->GetPlacesForTurrets(pl) && pElement->GetState() == IFACE_DISABLED)
                                {
                                    pElement->SetState(IFACE_NORMAL);
                                }
                            }
                            else if(pElement->m_strName == IF_BUILD_TUR4)
                            {
                                pElement->SetVisibility(true);
                                bool enough_resources = player_side->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[TURRET_MISSILE_CANNON]);
                                if(!bld->GetPlacesForTurrets(pl) || !enough_resources)
                                {
                                    pElement->SetState(IFACE_DISABLED);
                                }
                                else if(bld->GetPlacesForTurrets(pl) && pElement->GetState() == IFACE_DISABLED)
                                {
                                    pElement->SetState(IFACE_NORMAL);
                                }
                            }
                        }
                        //Нажали кнопку выбора типа вертолёта для постройки
                        else if(g_EnableFlyers && bld_he && player_side->m_CurrSel == BASE_SELECTED)
                        {
                            if(pElement->m_strName == IF_BUILD_FLYER_1)
                            {
                                pElement->SetVisibility(true);
                            }
                            else if(pElement->m_strName == IF_BUILD_FLYER_2)
                            {
                                pElement->SetVisibility(true);
                            }
                            else if(pElement->m_strName == IF_BUILD_FLYER_3)
                            {
                                pElement->SetVisibility(true);
                            }
                            else if(pElement->m_strName == IF_BUILD_FLYER_4)
                            {
                                pElement->SetVisibility(true);
                            }
                        }
                    }
                }
                pElement = pElement->m_NextElement;
            }
///////////////////EOF MAIN////////////////////////////////////////////////////////////////////////////////////////////
        }
        else if(m_strName == IF_RADAR)
        {
            while(pElement)
            {
                if(player_side->IsArcadeMode())
                {
                    pElement->SetVisibility(true);
                }
                else
                {
                    pElement->SetVisibility(false);
                }
                pElement = pElement->m_NextElement;
            }
        }
        else if(m_strName == IF_TOP)
        {
            while(pElement)
            {
                pElement->SetVisibility(true);
                if(pElement->m_strName == IF_TITAN_LABEL)
                {
                    if(prev_titan != player_side->GetResourcesAmount(TITAN))
                    {
                        prev_titan = player_side->GetResourcesAmount(TITAN);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption.Set(prev_titan);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if(pElement->m_strName == IF_ELECTRO_LABEL)
                {
                    if(prev_electro != player_side->GetResourcesAmount(ELECTRONICS))
                    {
                        prev_electro = player_side->GetResourcesAmount(ELECTRONICS);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption.Set(prev_electro);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if(pElement->m_strName == IF_ENERGY_LABEL)
                {
                    if(prev_energy != player_side->GetResourcesAmount(ENERGY))
                    {
                        prev_energy = player_side->GetResourcesAmount(ENERGY);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption.Set(prev_energy);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if(pElement->m_strName == IF_PLASMA_LABEL)
                {
                    if(prev_plasma != player_side->GetResourcesAmount(PLASMA))
                    {
                        prev_plasma = player_side->GetResourcesAmount(PLASMA);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption.Set(prev_plasma);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if(pElement->m_strName == IF_RVALUE_LABEL)
                {
                    if(/*количество роботов изменилось*/robots != player_side->GetSideRobots() || /*максимальное количество изменилось*/max_robots != player_side->GetMaxSideRobots())
                    {
                        robots = player_side->GetSideRobots();
                        max_robots = player_side->GetMaxSideRobots();
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption.Set(robots);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption.Add(L"/").Add(max_robots);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                pElement = pElement->m_NextElement;
            }
        }
        return;
    }
    
    m_VisibleAlpha = IS_NOT_VISIBLE;

    if((m_strName == IF_BASE) && (player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED) && player_side->m_ActiveObject && player_side->m_ActiveObject->GetObjectType() == OBJECT_TYPE_BUILDING)
    {
        CMatrixBuilding* building = (CMatrixBuilding*)player_side->m_ActiveObject;
        int total_res[MAX_RESOURCES];
        player_side->m_Constructor->GetConstructionPrice(total_res);
        for(int i = 0; i < MAX_RESOURCES; ++i)
        {
            if(g_IFaceList->m_RCountControl->GetCounter())
            {
                total_res[i] *= g_IFaceList->m_RCountControl->GetCounter();
            }
        }

        bool build_flag = (building->m_TurretsHave < building->m_TurretsMax);
        m_VisibleAlpha = IS_VISIBLE;
        int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
        
        while(pElement)
        {
            pElement->SetVisibility(false);
            if(building->IsBase())
            {
                bool bld = building->m_BS.GetItemsCnt() < BUILDING_QUEUE_LIMIT;
                std::bitset<RUK_WEAPON_PYLONS_COUNT> weapon_slots_used = g_Config.m_RobotHullsConsts[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Module.m_nKind].constructor_weapon_slots_used;
                CIFaceElement* foc_el = player_side->m_ConstructPanel->m_FocusedElement;

                if(player_side->m_ConstructPanel->IsActive())
                {
                    if((pElement->m_strName == L"counthz" || pElement->m_strName == IF_BASE_CONSTRUCTION_LEFT || pElement->m_strName == IF_BASE_CONSTRUCTION_RIGHT || pElement->m_strName == IF_BASE_CONSTRUCTION_FOOT))
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_strName == IF_BASE_RCNAME)
                    {
                        if(rcname != player_side->m_Constructor->GetRenderBot()->m_Name)
                        {
                            rcname = player_side->m_Constructor->GetRenderBot()->m_Name;
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = rcname;
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                        pElement->SetVisibility(true);
                    }
                    else if((pElement->m_strName == IF_BASE_WARNING1 || pElement->m_strName == IF_BASE_WARNING_LABEL))
                    {
                        CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
                        if(ps->GetRobotsCnt() + ps->GetRobotsInQueue() >= ps->GetMaxSideRobots()) pElement->SetVisibility(true);
                    }
                    else if(IS_DYNAMIC_WARNING(pElement->m_nId))
                    {
                        int res[MAX_RESOURCES];
                        player_side->m_Constructor->GetConstructionPrice(res);
                        for(int i = 0; i < MAX_RESOURCES; ++i)
                        {
                            if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                        }

                        dword dw = pElement->m_nId - DYNAMIC_WARNING;
                        if(dw < 4)
                        {
                            ERes r = ERes(dw);
                            if(player_side->GetResourcesAmount(r) < res[r])
                            {
                                pElement->SetVisibility(true);
                            }
                        }
                    }
                    else if(pElement->m_strName == IF_BASE_CONST_CANCEL)
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_strName == IF_BASE_CONST_BUILD)
                    {
                        int res[4];
                        player_side->m_Constructor->GetConstructionPrice(res);

                        bool men = false;
                        for(int i = 0; !men && i < MAX_RESOURCES; ++i)
                        {
                            men |= player_side->GetResourcesAmount(ERes(i)) < res[i];
                        }

                        if(!bld || men || player_side->GetRobotsCnt() + player_side->GetRobotsInQueue() >= player_side->GetMaxSideRobots())
                        {
                            pElement->m_DefState = IFACE_DISABLED;
                            pElement->m_CurState = IFACE_DISABLED;
                            g_IFaceList->m_RCountControl->Disable();
                        }
                        else if(pElement->m_CurState == IFACE_DISABLED)
                        {
                            pElement->m_DefState = IFACE_NORMAL;
                            pElement->m_CurState = IFACE_NORMAL;
                            g_IFaceList->m_RCountControl->Enable();
                        }
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_strName == IF_BASE_ITEM_LABEL1)
                    {
                        if(item_label1 != player_side->m_ConstructPanel->m_FocusedLabel)
                        {
                            item_label1 = player_side->m_ConstructPanel->m_FocusedLabel;
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = item_label1;
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);

                        }
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_strName == IF_BASE_ITEM_LABEL2)
                    {
                        if(item_label2 != player_side->m_ConstructPanel->m_FocusedDescription)
                        {
                            item_label2 = player_side->m_ConstructPanel->m_FocusedDescription;
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = item_label2;
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                        pElement->SetVisibility(true);
                    }
                    //Корпус
                    else if(pElement->m_strName == IF_BASE_HULL_PYLON)
                    {
                        pElement->SetVisibility(true);
                        if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
                        {
                            int res[MAX_RESOURCES];
                            player_side->m_Constructor->GetConstructionPrice(res);
                            for(int i = 0; i < MAX_RESOURCES; ++i)
                            {
                                if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                            }

                            bool its_critical = false;
                            if(pElement->m_Param2)
                            {
                                int kind = Float2Int(pElement->m_Param2);
                                if(player_side->GetResourcesAmount(TITAN) < res[TITAN] && g_Config.m_RobotHullsConsts[kind].cost_titan) its_critical = true;
                                if(player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] && g_Config.m_RobotHullsConsts[kind].cost_electronics) its_critical = true;
                                if(player_side->GetResourcesAmount(ENERGY) < res[ENERGY] && g_Config.m_RobotHullsConsts[kind].cost_energy) its_critical = true;
                                if(player_side->GetResourcesAmount(PLASMA) < res[PLASMA] && g_Config.m_RobotHullsConsts[kind].cost_plasma) its_critical = true;
                            }

                            if(its_critical)
                            {
                                g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                            }
                            else
                            {
                                g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                            }
                        }
                    }
                    //Шасси
                    else if(pElement->m_strName == IF_BASE_CHASSIS_PYLON)
                    {
                        pElement->SetVisibility(true);
                        if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
                        {
                            int res[MAX_RESOURCES];
                            player_side->m_Constructor->GetConstructionPrice(res);
                            for(int i = 0; i < MAX_RESOURCES; ++i)
                            {
                                if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                            }

                            bool its_critical = false;
                            if(pElement->m_Param2)
                            {
                                int kind = Float2Int(pElement->m_Param2);
                                if(player_side->GetResourcesAmount(TITAN) < res[TITAN] && g_Config.m_RobotChassisConsts[kind].cost_titan) its_critical = true;
                                if(player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] && g_Config.m_RobotChassisConsts[kind].cost_electronics) its_critical = true;
                                if(player_side->GetResourcesAmount(ENERGY) < res[ENERGY] && g_Config.m_RobotChassisConsts[kind].cost_energy) its_critical = true;
                                if(player_side->GetResourcesAmount(PLASMA) < res[PLASMA] && g_Config.m_RobotChassisConsts[kind].cost_plasma) its_critical = true;
                            }

                            if(its_critical)
                            {
                                g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                            }
                            else
                            {
                                g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                            }
                        }
                    }
                    //Голова
                    else if(pElement->m_strName == IF_BASE_HEAD_PYLON)
                    {
                        pElement->SetVisibility(true);
                        if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
                        {
                            int res[MAX_RESOURCES];
                            player_side->m_Constructor->GetConstructionPrice(res);
                            for(int i = 0; i < MAX_RESOURCES; ++i)
                            {
                                if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                            }

                            bool its_critical = false;
                            if(pElement->m_Param2)
                            {
                                int kind = Float2Int(pElement->m_Param2);
                                if(player_side->GetResourcesAmount(TITAN) < res[TITAN] && g_Config.m_RobotHeadsConsts[kind].cost_titan) its_critical = true;
                                if(player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] && g_Config.m_RobotHeadsConsts[kind].cost_electronics) its_critical = true;
                                if(player_side->GetResourcesAmount(ENERGY) < res[ENERGY] && g_Config.m_RobotHeadsConsts[kind].cost_energy) its_critical = true;
                                if(player_side->GetResourcesAmount(PLASMA) < res[PLASMA] && g_Config.m_RobotHeadsConsts[kind].cost_plasma) its_critical = true;
                            }

                            if(its_critical)
                            {
                                g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                            }
                            else
                            {
                                g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                            }
                        }
                    }
                    //Создание кнопок выбора оружия
                    else if(pElement->m_strName == IF_BASE_WEAPON_PYLON_1)
                    {
                        if(weapon_slots_used.test(RUK_WEAPON_PYLON_1))
                        {
                            pElement->SetVisibility(true);
                            if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
                            {
                                int res[MAX_RESOURCES];
                                player_side->m_Constructor->GetConstructionPrice(res);
                                for(int i = 0; i < MAX_RESOURCES; ++i)
                                {
                                    if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                                }

                                bool its_critical = false;
                                if(pElement->m_Param2)
                                {
                                    int kind = Float2Int(pElement->m_Param2);
                                    if(player_side->GetResourcesAmount(TITAN) < res[TITAN] && g_Config.m_RobotWeaponsConsts[kind].cost_titan) its_critical = true;
                                    if(player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] && g_Config.m_RobotWeaponsConsts[kind].cost_electronics) its_critical = true;
                                    if(player_side->GetResourcesAmount(ENERGY) < res[ENERGY] && g_Config.m_RobotWeaponsConsts[kind].cost_energy) its_critical = true;
                                    if(player_side->GetResourcesAmount(PLASMA) < res[PLASMA] && g_Config.m_RobotWeaponsConsts[kind].cost_plasma) its_critical = true;
                                }

                                if(its_critical)
                                {
                                    g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                                }
                                else
                                {
                                    g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                                }
                            }
                        }
                    }
                    else if(pElement->m_strName == IF_BASE_WEAPON_PYLON_2)
                    {
                        if(weapon_slots_used.test(RUK_WEAPON_PYLON_2))
                        {
                            pElement->SetVisibility(true);
                            if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
                            {
                                int res[MAX_RESOURCES];
                                player_side->m_Constructor->GetConstructionPrice(res);
                                for(int i = 0; i < MAX_RESOURCES; ++i)
                                {
                                    if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                                }

                                bool its_critical = false;
                                if(pElement->m_Param2)
                                {
                                    int kind = Float2Int(pElement->m_Param2);
                                    if(player_side->GetResourcesAmount(TITAN) < res[TITAN] && g_Config.m_RobotWeaponsConsts[kind].cost_titan) its_critical = true;
                                    if(player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] && g_Config.m_RobotWeaponsConsts[kind].cost_electronics) its_critical = true;
                                    if(player_side->GetResourcesAmount(ENERGY) < res[ENERGY] && g_Config.m_RobotWeaponsConsts[kind].cost_energy) its_critical = true;
                                    if(player_side->GetResourcesAmount(PLASMA) < res[PLASMA] && g_Config.m_RobotWeaponsConsts[kind].cost_plasma) its_critical = true;
                                }

                                if(its_critical)
                                {
                                    g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                                }
                                else
                                {
                                    g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                                }
                            }
                        }
                    }
                    else if(pElement->m_strName == IF_BASE_WEAPON_PYLON_3)
                    {
                        if(weapon_slots_used.test(RUK_WEAPON_PYLON_3))
                        {
                            pElement->SetVisibility(true);
                            if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
                            {
                                int res[MAX_RESOURCES];
                                player_side->m_Constructor->GetConstructionPrice(res);
                                for(int i = 0; i < MAX_RESOURCES; ++i)
                                {
                                    if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                                }

                                bool its_critical = false;
                                if(pElement->m_Param2)
                                {
                                    int kind = Float2Int(pElement->m_Param2);
                                    if(player_side->GetResourcesAmount(TITAN) < res[TITAN] && g_Config.m_RobotWeaponsConsts[kind].cost_titan) its_critical = true;
                                    if(player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] && g_Config.m_RobotWeaponsConsts[kind].cost_electronics) its_critical = true;
                                    if(player_side->GetResourcesAmount(ENERGY) < res[ENERGY] && g_Config.m_RobotWeaponsConsts[kind].cost_energy) its_critical = true;
                                    if(player_side->GetResourcesAmount(PLASMA) < res[PLASMA] && g_Config.m_RobotWeaponsConsts[kind].cost_plasma) its_critical = true;
                                }

                                if(its_critical)
                                {
                                    g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                                }
                                else
                                {
                                    g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                                }
                            }
                        }
                    }
                    else if(pElement->m_strName == IF_BASE_WEAPON_PYLON_4)
                    {
                        if(weapon_slots_used.test(RUK_WEAPON_PYLON_4))
                        {
                            pElement->SetVisibility(true);
                            if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
                            {
                                int res[MAX_RESOURCES];
                                player_side->m_Constructor->GetConstructionPrice(res);
                                for(int i = 0; i < MAX_RESOURCES; ++i)
                                {
                                    if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                                }

                                bool its_critical = false;
                                if(pElement->m_Param2)
                                {
                                    int kind = Float2Int(pElement->m_Param2);
                                    if(player_side->GetResourcesAmount(TITAN) < res[TITAN] && g_Config.m_RobotWeaponsConsts[kind].cost_titan) its_critical = true;
                                    if(player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] && g_Config.m_RobotWeaponsConsts[kind].cost_electronics) its_critical = true;
                                    if(player_side->GetResourcesAmount(ENERGY) < res[ENERGY] && g_Config.m_RobotWeaponsConsts[kind].cost_energy) its_critical = true;
                                    if(player_side->GetResourcesAmount(PLASMA) < res[PLASMA] && g_Config.m_RobotWeaponsConsts[kind].cost_plasma) its_critical = true;
                                }

                                if(its_critical)
                                {
                                    g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                                }
                                else
                                {
                                    g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                                }
                            }
                        }
                    }
                    else if(pElement->m_strName == IF_BASE_WEAPON_PYLON_5)
                    {
                        if(weapon_slots_used.test(RUK_WEAPON_PYLON_5))
                        {
                            pElement->SetVisibility(true);
                            if(!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
                            {
                                int res[MAX_RESOURCES];
                                player_side->m_Constructor->GetConstructionPrice(res);
                                for(int i = 0; i < MAX_RESOURCES; ++i)
                                {
                                    if(g_IFaceList->m_RCountControl->GetCounter()) res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                                }

                                bool its_critical = false;
                                if(pElement->m_Param2)
                                {
                                    int kind = Float2Int(pElement->m_Param2);
                                    if(player_side->GetResourcesAmount(TITAN) < res[TITAN] && g_Config.m_RobotWeaponsConsts[kind].cost_titan) its_critical = true;
                                    if(player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] && g_Config.m_RobotWeaponsConsts[kind].cost_electronics) its_critical = true;
                                    if(player_side->GetResourcesAmount(ENERGY) < res[ENERGY] && g_Config.m_RobotWeaponsConsts[kind].cost_energy) its_critical = true;
                                    if(player_side->GetResourcesAmount(PLASMA) < res[PLASMA] && g_Config.m_RobotWeaponsConsts[kind].cost_plasma) its_critical = true;
                                }

                                if(its_critical)
                                {
                                    g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                                }
                                else
                                {
                                    g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                                }
                            }
                        }
                    }
                    //Создание кнопки переключения шаблона робота в меню конструктора вправо
                    else if(pElement->m_strName == IF_BASE_HISTORY_RIGHT)
                    {
                        if(!g_ConfigHistory->IsNext())
                        {
                            pElement->SetState(IFACE_DISABLED);
                        }
                        else if(g_ConfigHistory->IsNext() && pElement->GetState() == IFACE_DISABLED)
                        {
                            pElement->SetState(IFACE_NORMAL);
                        }
                        pElement->SetVisibility(true);
                    }
                    //Создание кнопки переключения шаблона робота в меню конструктора влево
                    else if(pElement->m_strName == IF_BASE_HISTORY_LEFT)
                    {
                        if(!g_ConfigHistory->IsPrev())
                        {
                            pElement->SetState(IFACE_DISABLED);
                        }
                        else if(g_ConfigHistory->IsPrev() && pElement->GetState() == IFACE_DISABLED)
                        {
                            pElement->SetState(IFACE_NORMAL);
                        }
                        pElement->SetVisibility(true);
                    }
                    //Создание кнопки увеличение числа роботов для постройки
                    else if(pElement->m_strName == IF_BASE_UP)
                    {
                        pElement->SetVisibility(true);
                    }
                    //Создание кнопки уменьшение числа роботов для постройки
                    else if(pElement->m_strName == IF_BASE_DOWN)
                    {
                        pElement->SetVisibility(true);
                    }
                    /*
                    else if((pElement->m_strName == IF_BASE_CONFIG1 || pElement->m_strName == IF_BASE_CONFIG2 || pElement->m_strName == IF_BASE_CONFIG3 || pElement->m_strName == IF_BASE_CONFIG4 || pElement->m_strName == IF_BASE_CONFIG5))
                    {
                        pElement->SetVisibility(true);
                    }
                    */
                    else if(pElement->m_Type == IFACE_DYNAMIC_STATIC && (pElement->m_strName == IF_BASE_TITAN_IMAGE || pElement->m_strName == IF_BASE_ELECTRONICS_IMAGE || pElement->m_strName == IF_BASE_ENERGY_IMAGE || pElement->m_strName == IF_BASE_PLASMA_IMAGE))
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_Type == IFACE_DYNAMIC_STATIC && (pElement->m_strName == IF_BASE_TITAN_SUMM || pElement->m_strName == IF_BASE_ELECTRONICS_SUMM || pElement->m_strName == IF_BASE_ENERGY_SUMM || pElement->m_strName == IF_BASE_PLASMA_SUMM))
                    {
                        pElement->SetVisibility(true);
                    }
                    else if(pElement->m_strName == IF_BASE_SUMM_PANEL)
                    {
                        pElement->SetVisibility(true);

                        bool tit_color_upd = false;
                        if((player_side->GetResourcesAmount(TITAN) < titan_summ) && titan_color != 0xFFFF0000)
                        {
                            titan_color = 0xFFFF0000;
                            tit_color_upd = true;
                        }
                        else if((player_side->GetResourcesAmount(TITAN) >= titan_summ) && titan_color != 0xFFF6c000)
                        {
                            titan_color = 0xFFF6c000;
                            tit_color_upd = true;
                        }

                        if(tit_color_upd && titan_summ)
                        {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_Configs[cfg_num].m_titX + 25) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(titan_summ);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        bool elec_color_upd = false;
                        if((player_side->GetResourcesAmount(ELECTRONICS) < electronics_summ) && electronics_color != 0xFFFF0000)
                        {
                            electronics_color = 0xFFFF0000;
                            elec_color_upd = true;
                        }
                        else if((player_side->GetResourcesAmount(ELECTRONICS) >= electronics_summ) && electronics_color != 0xFFF6c000)
                        {
                            electronics_color = 0xFFF6c000;
                            elec_color_upd = true;
                        }

                        if(elec_color_upd && electronics_summ)
                        {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = electronics_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_Configs[cfg_num].m_elecX + 25) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(electronics_summ);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        bool ener_color_upd = false;
                        if((player_side->GetResourcesAmount(ENERGY) < energy_summ) && energy_color != 0xFFFF0000)
                        {
                            energy_color = 0xFFFF0000;
                            ener_color_upd = true;
                        }
                        else if((player_side->GetResourcesAmount(ENERGY) >= energy_summ) && energy_color != 0xFFF6c000)
                        {
                            energy_color = 0xFFF6c000;
                            ener_color_upd = true;
                        }

                        if(ener_color_upd && energy_summ)
                        {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = energy_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_Configs[cfg_num].m_enerX + 19) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(energy_summ);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        bool plas_color_upd = false;
                        if((player_side->GetResourcesAmount(PLASMA) < plasma_summ) && plasm_color != 0xFFFF0000)
                        {
                            plasm_color = 0xFFFF0000;
                            plas_color_upd = true;
                        }
                        else if((player_side->GetResourcesAmount(PLASMA) > plasma_summ) && plasm_color != 0xFFF6c000)
                        {
                            plasm_color = 0xFFF6c000;
                            plas_color_upd = true;
                        }

                        if(plas_color_upd && plasma_summ)
                        {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = plasm_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_Configs[cfg_num].m_plasX + 24) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(plasma_summ);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        if(titan_summ != total_res[TITAN] || electronics_summ != total_res[ELECTRONICS] || energy_summ != total_res[ENERGY] || plasma_summ != total_res[PLASMA])
                        {
                            titan_summ = total_res[TITAN];
                            electronics_summ = total_res[ELECTRONICS];
                            energy_summ = total_res[ENERGY];
                            plasma_summ = total_res[PLASMA];

                            if(titan_summ != 0)
                            {
                                if(player_side->GetResourcesAmount(TITAN) < titan_summ)
                                {
                                    titan_color = 0xFFFF0000;
                                }
                                pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_Configs[cfg_num].m_titX + 25) - Float2Int(pElement->m_xPos);
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(titan_summ);
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                            }

                            if(electronics_summ != 0)
                            {
                                if(player_side->GetResourcesAmount(ELECTRONICS) < electronics_summ)
                                {
                                    electronics_color = 0xFFFF0000;
                                }

                                pElement->m_StateImages[IFACE_NORMAL].m_Color = electronics_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_Configs[cfg_num].m_elecX + 25) - Float2Int(pElement->m_xPos);
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(electronics_summ);
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                            }

                            if(energy_summ != 0)
                            {
                                if(player_side->GetResourcesAmount(ENERGY) < energy_summ)
                                {
                                    energy_color = 0xFFFF0000;
                                }
                                pElement->m_StateImages[IFACE_NORMAL].m_Color = energy_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_Configs[cfg_num].m_enerX + 19) - Float2Int(pElement->m_xPos);
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(energy_summ);
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                            }

                            if(plasma_summ != 0)
                            {
                                if(player_side->GetResourcesAmount(PLASMA) < plasma_summ)
                                {
                                    plasm_color = 0xFFFF0000;
                                }
                                pElement->m_StateImages[IFACE_NORMAL].m_Color = plasm_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_Configs[cfg_num].m_plasX + 24) - Float2Int(pElement->m_xPos);
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(plasma_summ);
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                            }
                        }
                    }
                    else if(foc_el && pElement->m_strName == IF_BASE_UNIT_PANEL)
                    {
                        int* unit_res = nullptr;
                        int kind = Float2Int(foc_el->m_Param2);
                        if(kind)pElement->SetVisibility(true);

                        //Здесь задаются цифры стоимости в ресурсах для конкретных модулей, выводятся в панели конструктора слева
                        //(при этом картинки ресурсов напротив цифр выставляются в другом методе, потому что это охуеть как дабоудобно!)
                        float cost_mod = 1.0f;
                        //Проверяем возможно установленную в слоте голову на наличие эффекта изменения стоимости
                        int cur_head = g_MatrixMap->GetPlayerSide()->m_ConstructPanel->m_Configs[g_MatrixMap->GetPlayerSide()->m_ConstructPanel->m_CurrentConfig].m_Head.m_nKind;
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

                        ERobotModuleType module_type = (ERobotModuleType)(int)foc_el->m_Param1;
                        if(module_type == MRT_HULL) unit_res = &g_Config.m_RobotHullsConsts[kind].cost_titan;
                        //И снова перебор по смещению - фу
                        else if(module_type == MRT_CHASSIS) unit_res = &g_Config.m_RobotChassisConsts[kind].cost_titan;
                        else if(module_type == MRT_HEAD)
                        {
                            cost_mod = 1.0f; //Головы никогда не меняют свою стоимость
                            unit_res = &g_Config.m_RobotHeadsConsts[kind].cost_titan;
                        }
                        else if(module_type == MRT_WEAPON) unit_res = &g_Config.m_RobotWeaponsConsts[kind].cost_titan;

                        bool tit_color_upd = false;
                        if((player_side->GetResourcesAmount(TITAN) < total_res[TITAN]) && titan_unit_color != 0xFFFF0000)
                        {
                            titan_unit_color = 0xFFFF0000;
                            tit_color_upd = true;
                        }
                        else if((player_side->GetResourcesAmount(TITAN) >= total_res[TITAN]) && titan_unit_color != 0xFFF6c000)
                        {
                            titan_unit_color = 0xFFF6c000;
                            tit_color_upd = true;
                        }

                        if(tit_color_upd)
                        {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_TitanResCountX + 25) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(titan_unit);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        bool elec_color_upd = false;
                        if((player_side->GetResourcesAmount(ELECTRONICS) < total_res[ELECTRONICS]) && electronics_unit_color != 0xFFFF0000)
                        {
                            electronics_unit_color = 0xFFFF0000;
                            elec_color_upd = true;
                        }
                        else if((player_side->GetResourcesAmount(ELECTRONICS) >= total_res[ELECTRONICS]) && electronics_unit_color != 0xFFF6c000)
                        {
                            electronics_unit_color = 0xFFF6c000;
                            elec_color_upd = true;
                        }

                        if(elec_color_upd)
                        {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = electronics_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_ElectronicsResCountX + 25) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(electronics_unit);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        bool ener_color_upd = false;
                        if((player_side->GetResourcesAmount(ENERGY) < total_res[ENERGY]) && energy_unit_color != 0xFFFF0000)
                        {
                            energy_unit_color = 0xFFFF0000;
                            ener_color_upd = true;
                        }
                        else if((player_side->GetResourcesAmount(ENERGY) >= total_res[ENERGY]) && energy_unit_color != 0xFFF6c000)
                        {
                            energy_unit_color = 0xFFF6c000;
                            ener_color_upd = true;
                        }

                        if(ener_color_upd)
                        {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = energy_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_EnergyResCountX + 19) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(energy_unit);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        bool plas_color_upd = false;
                        if((player_side->GetResourcesAmount(PLASMA) < total_res[PLASMA]) && plasm_unit_color != 0xFFFF0000)
                        {
                            plasm_unit_color = 0xFFFF0000;
                            plas_color_upd = true;
                        }
                        else if((player_side->GetResourcesAmount(PLASMA) > total_res[PLASMA]) && plasm_unit_color != 0xFFF6c000)
                        {
                            plasm_unit_color = 0xFFF6c000;
                            plas_color_upd = true;
                        }

                        if(plas_color_upd)
                        {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = plasm_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_PlasmaResCountX + 24) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(plasma_unit);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        if(titan_unit != unit_res[TITAN] || electronics_unit != unit_res[ELECTRONICS] || energy_unit != unit_res[ENERGY] || plasma_unit != unit_res[PLASMA])
                        {
                            titan_unit = unit_res[TITAN];
                            electronics_unit = unit_res[ELECTRONICS];
                            energy_unit = unit_res[ENERGY];
                            plasma_unit = unit_res[PLASMA];

                            if(titan_unit)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_unit_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_TitanResCountX + 25) - Float2Int(pElement->m_xPos);
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(int(titan_unit * cost_mod));
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                            }
                            else
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_unit_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = 0;
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(L"");
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                            }

                            if(electronics_unit)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Color = electronics_unit_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_ElectronicsResCountX + 25) - Float2Int(pElement->m_xPos);
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(int(electronics_unit * cost_mod));
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                            }

                            if(energy_unit)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Color = energy_unit_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_EnergyResCountX + 19) - Float2Int(pElement->m_xPos);
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(int(energy_unit * cost_mod));
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                            }

                            if(plasma_unit)
                            {
                                pElement->m_StateImages[IFACE_NORMAL].m_Color = plasm_unit_color;
                                pElement->m_StateImages[IFACE_NORMAL].m_SmeX = (player_side->m_ConstructPanel->m_PlasmaResCountX + 24) - Float2Int(pElement->m_xPos);
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(int(plasma_unit * cost_mod));
                                pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                            }
                        }

                    }
                    else if(pElement->m_strName == IF_BASE_STRUCTURE)
                    {
                        pElement->SetVisibility(true);
                        if(structure != player_side->m_Constructor->GetConstructionStructure())
                        {
                            structure = player_side->m_Constructor->GetConstructionStructure();
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(structure);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                    }
                    else if(pElement->m_strName == IF_BASE_DAMAGE)
                    {
                        pElement->SetVisibility(true);

                        int wep = 0;
                        for(int cnt = 0; cnt < RUK_WEAPON_PYLONS_COUNT; ++cnt)
                        {
                            if(player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[cnt].m_nKind != 0)
                            {
                                ++wep;
                                break;
                            }
                        }

                        if(!wep)
                        {
                            damage = 0;
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(damage);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                        else if(damage != GetConstructionDamage(player_side->m_Constructor->GetRenderBot()))
                        {
                            damage = GetConstructionDamage(player_side->m_Constructor->GetRenderBot());
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(damage);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                    }
                    else if(pElement == g_IFaceList->m_RCountControl->GetImage()) pElement->SetVisibility(true);
                    else if(foc_el)
                    {
                        if(pElement->m_strName == IF_BASE_ITEM_PRICE && foc_el->m_Param2 != 0)
                        {
                            pElement->SetVisibility(true);
                        }

                        //Включается отображение картинки, названия типа модуля и названия модулей робота (всё это - текстуры) рядом с картинкой в режиме конструктора
                        ERobotModuleType module_type = (ERobotModuleType)(int)foc_el->m_Param1;
                        //Корпуса
                        if(module_type == MRT_HULL)
                        {
                            for(int i = 1; i <= ROBOT_HULLS_COUNT; ++i)
                            {
                                if(foc_el->m_Param2 == i)
                                {
                                    if(pElement->m_strName == g_Config.m_RobotHullsConsts[i].constructor_image_name || pElement->m_strName == IF_BASE_IHU_TEXT || pElement->m_strName == g_Config.m_RobotHullsConsts[i].constructor_label_name)
                                    {
                                        pElement->SetVisibility(true);
                                    }
                                }
                            }
                        }
                        
                        //Шасси
                        if(module_type == MRT_CHASSIS)
                        {
                            for(int i = 1; i <= ROBOT_CHASSIS_COUNT; ++i)
                            {
                                if(foc_el->m_Param2 == i)
                                {
                                    if(pElement->m_strName == g_Config.m_RobotChassisConsts[i].constructor_image_name || pElement->m_strName == IF_BASE_ICH_TEXT || pElement->m_strName == g_Config.m_RobotChassisConsts[i].constructor_label_name)
                                    {
                                        pElement->SetVisibility(true);
                                    }
                                }
                            }
                        }

                        //Головы
                        if(module_type == MRT_HEAD)
                        {
                            for(int i = 1; i <= ROBOT_HEADS_COUNT; ++i)
                            {
                                if(foc_el->m_Param2 == i)
                                {
                                    if(pElement->m_strName == g_Config.m_RobotHeadsConsts[i].constructor_image_name || pElement->m_strName == IF_BASE_IHE_TEXT || pElement->m_strName == g_Config.m_RobotHeadsConsts[i].constructor_label_name)
                                    {
                                        pElement->SetVisibility(true);
                                    }
                                }
                            }
                        }

                        //Оружие
                        if(module_type == MRT_WEAPON)
                        {
                            for(int i = 1; i <= ROBOT_WEAPONS_COUNT; ++i)
                            {
                                if(foc_el->m_Param2 == i)
                                {
                                    if(pElement->m_strName == g_Config.m_RobotWeaponsConsts[i].constructor_image_name || pElement->m_strName == IF_BASE_IW_TEXT || pElement->m_strName == g_Config.m_RobotWeaponsConsts[i].constructor_label_name)
                                    {
                                        pElement->SetVisibility(true);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            pElement = pElement->m_NextElement;
        }
    }
    else if(m_strName == IF_HINTS)
    {
        m_VisibleAlpha = IS_VISIBLE;
        CIFaceElement* e = m_FirstElement;
        while(e)
        {
            //e->SetVisibility(true);
            e = e->m_NextElement;
        }
    }
    else if(m_strName == IF_POPUP_MENU && FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
    {
        m_VisibleAlpha = IS_VISIBLE;
        CIFaceElement* pElement = m_FirstElement;
        while(pElement)
        {
            if(pElement->m_strName == IF_POPUP_RAMKA)
            {
                pElement->SetVisibility(true);
            }
            else if((pElement->m_strName == IF_POPUP_SELECTOR/* && g_IFaceList->m_FocusedElement && g_IFaceList->m_FocusedElement->m_nId == POPUP_SELECTOR_CATCHERS_ID*/))
            {
                pElement->SetVisibility(true);
            }
            else if(pElement->m_nId == POPUP_SELECTOR_CATCHERS_ID)
            {
                pElement->SetVisibility(true);
            }
            else if(pElement->m_strName == IF_POPUP_POINTER_OBJECT)
            {
                pElement->SetVisibility(true);
            }
            pElement = pElement->m_NextElement;
        }
    }
}

void CInterface::SortElementsByZ()
{
    CIFaceElement* elements = nullptr, *el_cur = nullptr, *el_plus_one = nullptr, *el_prev = nullptr, *el_next = nullptr;

    int sorting;
    do
    {
        elements = m_FirstElement;
        sorting = 0;
        while(elements && elements->m_NextElement)
        {
            if(elements->m_zPos < elements->m_NextElement->m_zPos)
            {
                sorting = 1;
                
                el_cur = elements;
                el_plus_one = elements->m_NextElement;

                if(el_plus_one->m_NextElement == nullptr)
                {
                    m_LastElement = el_cur;
                    el_cur->m_NextElement = nullptr;
                }
                else
                {
                    el_cur->m_NextElement = el_plus_one->m_NextElement;
                    el_plus_one->m_NextElement->m_PrevElement = el_cur;
                }

                
                if(elements->m_PrevElement == nullptr)
                {
                    m_FirstElement = el_plus_one;
                    el_plus_one->m_PrevElement = nullptr;
                }
                else
                {
                    el_plus_one->m_PrevElement = elements->m_PrevElement;
                    elements->m_PrevElement->m_NextElement = el_plus_one;
                }
                
                el_plus_one->m_NextElement = el_cur;
                el_cur->m_PrevElement = el_plus_one;
            }
            
            elements = elements->m_NextElement;
        }
    }
    while(sorting);
}

void CInterface::CopyElements(CIFaceElement* el_src, CIFaceElement* el_dest)
{
    for(int state = 0; state < MAX_STATES; ++state)
    {
        el_dest->m_StateImages[state].pImage = el_src->m_StateImages[state].pImage;
        el_dest->m_StateImages[state].xTexPos = el_src->m_StateImages[state].xTexPos;
        el_dest->m_StateImages[state].yTexPos = el_src->m_StateImages[state].yTexPos;

        for(int cnt = 0; cnt < 4; ++cnt)
        {
            el_dest->m_StateImages[state].m_Geom[cnt].tu = el_src->m_StateImages[state].m_Geom[cnt].tu;
            el_dest->m_StateImages[state].m_Geom[cnt].tv = el_src->m_StateImages[state].m_Geom[cnt].tv;
        }
    }

    el_dest->m_Param1 = el_src->m_Param1;
    el_dest->m_Param2 = el_src->m_Param2;
    memcpy(el_dest->m_Actions, el_src->m_Actions, sizeof(SAction) * USER_ACTIONS_TOTAL);
    //el_dest->m_Actions = el_src->m_Actions;
}

CIFaceImage* CInterface::FindImageByName(CWStr name)
{
    CIFaceImage* images = m_FirstImage;
    while(images)
    {
        if(images->m_strName == name) return images;
        images = images->m_NextImage;
    }
    return nullptr;
}

CIFaceStatic* CInterface::CreateStaticFromImage(float x, float y, float z, const CIFaceImage& image, bool full_size)
{
DTRACE();

    CIFaceStatic* stat = HNew(g_MatrixHeap) CIFaceStatic;

    stat->m_strName = image.m_strName;
	stat->m_xPos = x;
	stat->m_yPos = y;
	stat->m_zPos = z;
    stat->m_xSize = image.m_Width;
    stat->m_ySize = image.m_Height;
	stat->m_DefState = IFACE_NORMAL;

	stat->SetStateImage(
		IFACE_NORMAL, 
        image.m_Image,
        image.m_xTexPos,
		image.m_yTexPos,
        image.m_TexWidth,
		image.m_TexHeight
    );

    stat->ElementGeomInit((void*)stat, full_size);
    stat->m_Type = IFACE_DYNAMIC_STATIC;

	AddElement(stat);
    return stat;
}

void CInterface::LogicTact(int ms)
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    if(g_IFaceList->m_FocusedInterface == this)
    {
        if(FLAG(m_InterfaceFlags, (INTERFACE_SLIDE_LEFT | INTERFACE_SLIDE_RIGHT)))
        {
            SlideStep();
        }
        //if(!ps->IsArcadeMode() && (GetAsyncKeyState(g_MatrixMap->m_Config.m_KeyActions[KA_UNIT_ROTATE_LEFT]) & 0x8000)==0x8000){
        //    MoveLeft();
        //    ReCalcElementsPos();
        //}
        //if(!ps->IsArcadeMode() && (GetAsyncKeyState(g_MatrixMap->m_Config.m_KeyActions[KA_UNIT_ROTATE_RIGHT]) & 0x8000)==0x8000){
        //    MoveRight();
        //    ReCalcElementsPos();
        //}
        //if(!ps->IsArcadeMode() && (GetAsyncKeyState(g_MatrixMap->m_Config.m_KeyActions[KA_UNIT_FORWARD]) & 0x8000)==0x8000){
        //    MoveUp();
        //    ReCalcElementsPos();
        //}
        //if(!ps->IsArcadeMode() && (GetAsyncKeyState(g_MatrixMap->m_Config.m_KeyActions[KA_UNIT_BACKWARD]) & 0x8000)==0x8000){
        //    MoveDown();
        //    ReCalcElementsPos();
        //}
    }

    CIFaceElement* els = m_FirstElement;

    while(els)
    {
        els->LogicTact(ms);
        els = els->m_NextElement; 
    }
}


void CInterface::MoveLeft()
{
    m_xPos -= 5.0f;
}

void CInterface::MoveRight()
{
    m_xPos += 5.0f;
}

void CInterface::MoveUp()
{
    m_yPos -= 5.0f;
}

void CInterface::MoveDown()
{
    m_yPos += 5.0f;
}

void CInterface::ReCalcElementsPos()
{
DTRACE();

    CIFaceElement* pElement = m_FirstElement;

    if(m_strName == IF_MAIN)
    {
        g_IFaceList->SetMainPos(m_xPos, m_yPos);
    }
    while(pElement)
    {
        pElement->RecalcPos(m_xPos, m_yPos);        
        pElement = pElement->m_NextElement;
    }
}

void CInterface::BeginSlide(float to_x, float to_y)
{
    ZeroMemory(&m_Slider, sizeof(SSlide));
    if(to_x != m_xPos)
    {
        RESETFLAG(m_InterfaceFlags,(INTERFACE_SLIDE_LEFT|INTERFACE_SLIDE_RIGHT));
        if(to_x < m_xPos)
        {
            SETFLAG(m_InterfaceFlags, INTERFACE_SLIDE_LEFT);
            m_Slider.startX = m_xPos;
            m_Slider.startY = m_yPos;
            m_Slider.stopX = to_x;
            m_Slider.stopY = to_y;
            m_Slider.startLength = m_xPos - to_x;
        }
        else
        {
            SETFLAG(m_InterfaceFlags, INTERFACE_SLIDE_RIGHT);
            m_Slider.startX = m_xPos;
            m_Slider.startY = m_yPos;
            m_Slider.stopX = to_x;
            m_Slider.stopY = to_y;
            m_Slider.startLength = to_x - m_xPos;

        }
    }
    //else if(to_y != m_yPos){}
}

void CInterface::SlideStep()
{
    if(FLAG(m_InterfaceFlags, INTERFACE_SLIDE_LEFT))
    {
        if(m_Slider.step > 1.0f)
        {
            RESETFLAG(m_InterfaceFlags, INTERFACE_SLIDE_LEFT);
            ZeroMemory(&m_Slider, sizeof(SSlide));
            return;
        }
    }
    else if(FLAG(m_InterfaceFlags, INTERFACE_SLIDE_RIGHT))
    {
        if(m_Slider.step > SLIDE_FUNC_PARAM)
        {
            RESETFLAG(m_InterfaceFlags, INTERFACE_SLIDE_RIGHT);
            ZeroMemory(&m_Slider, sizeof(SSlide));
            return;
        }
        
        float speed = 0;
        float x = m_Slider.step;
        float b = SLIDE_FUNC_PARAM;
        float c = 0;

        speed = (-(x * x)) + (b * x) + c;
        if(speed < ((-(SLIDE_STEP_SIZE*SLIDE_STEP_SIZE)) + (b * SLIDE_STEP_SIZE) + c))
        {
            speed = 0;
        }


        m_Slider.step += SLIDE_STEP_SIZE;
        float half_way = (m_Slider.startX + (m_Slider.startLength * 0.5f));

        //if(m_xPos >= half_way)
        //{
        //    m_xPos = LERPFLOAT(speed, m_Slider.stopX, half_way);
        //}
        //else
        //{
        //    m_xPos = LERPFLOAT(speed, m_Slider.startX, half_way);
        //}

        m_xPos += LERPFLOAT(speed, 0, SLIDE_MAX_SPEED);
        ReCalcElementsPos();
    }
}

bool CInterface::FindElementByName(const CWStr& name)
{
    CIFaceElement* elements = m_FirstElement;
    while(elements)
    {
        if(elements->m_strName == name) return true;

        elements = elements->m_NextElement;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
CIFaceList::CIFaceList():m_CurrentHintControlName(g_MatrixHeap)
{
    m_First = nullptr;
	m_Last = nullptr;
    m_IfListFlags = 0;
    m_FocusedInterface = nullptr;
    m_FocusedElement = nullptr;

    ZeroMemory(m_WeaponPilon, sizeof(m_WeaponPilon));
    ZeroMemory(m_Turrets, sizeof(m_Turrets));

    m_ChassisPilon = nullptr;
    m_ArmorPilon = nullptr;
    m_HeadPilon = nullptr;
    m_BuildCa = nullptr;

    CInterface::m_ClearRects = HNew(g_MatrixHeap) CBuf(g_MatrixHeap);

    m_RCountControl = HNew(g_MatrixHeap) CIFaceCounter;
    m_CurrentHint = nullptr;

    m_DynamicTY = 153;
    m_DynamicTX[0] = 280;
    m_DynamicTX[1] = 262;
    m_DynamicTX[2] = 304;
    m_DynamicTX[3] = 242;
    m_DynamicTX[4] = 279;
    m_DynamicTX[5] = 316;
    m_DynamicTX[6] = 231;
    m_DynamicTX[7] = 265;
    m_DynamicTX[8] = 299;
    m_DynamicTX[9] = 333;

    m_DWeaponX[0] = 243;
    m_DWeaponY[0] = 106;
    m_DWeaponX[1] = 283;
    m_DWeaponY[1] = 106;
    m_DWeaponX[2] = 243;
    m_DWeaponY[2] = 65;
    m_DWeaponX[3] = 283;
    m_DWeaponY[3] = 65;
    m_DWeaponX[4] = 323;
    m_DWeaponY[4] = 65;

    //Используем костыль для выделения памяти под эти массивы чуть позже
    m_Hull = new CIFaceElement * [ROBOT_HULLS_COUNT];
    ZeroMemory(m_Hull, sizeof(*m_Hull) * ROBOT_HULLS_COUNT);

    m_Chassis = new CIFaceElement * [ROBOT_CHASSIS_COUNT];
    ZeroMemory(m_Chassis, sizeof(*m_Chassis) * ROBOT_CHASSIS_COUNT);

    m_Head = new CIFaceElement * [ROBOT_HEADS_COUNT + 1]; //+1 для пустого слота, который тоже кнопка
    ZeroMemory(m_Head, sizeof(*m_Head) * (ROBOT_HEADS_COUNT + 1));

    m_Weapon = new CIFaceElement * [ROBOT_WEAPONS_COUNT + 1]; //+1 для пустого слота, который тоже кнопка
    ZeroMemory(m_Weapon, sizeof(*m_Weapon) * (ROBOT_WEAPONS_COUNT + 1));

    //Необходимо для соотнесения номеров слотов пушек на правой панели выбранного робота с Id матриц в его корпусе
    m_WeaponPylonNumsForPicsInMenu = new byte [ROBOT_HULLS_COUNT + 1][RUK_WEAPON_PYLONS_COUNT];
    for(int i = 1; i <= ROBOT_HULLS_COUNT; ++i)
    {
        ZeroMemory(m_WeaponPylonNumsForPicsInMenu[i], sizeof(byte) * RUK_WEAPON_PYLONS_COUNT); //Инициализация

        for(int j = 0; j < g_Config.m_RobotHullsConsts[i].weapon_pylon_data.size(); ++j)
        {
            m_WeaponPylonNumsForPicsInMenu[i][g_Config.m_RobotHullsConsts[i].weapon_pylon_data[j].constructor_slot_num] = g_Config.m_RobotHullsConsts[i].weapon_pylon_data[j].id;
        }
    }
}

CIFaceList::~CIFaceList()
{
    if(m_RCountControl) HDelete(CIFaceCounter, m_RCountControl, g_MatrixHeap);

    while(m_First != nullptr)
    {
		if(m_First->m_NextInterface) m_First = m_First->m_NextInterface;
		else
        {
			HDelete(CInterface, m_First, g_MatrixHeap);
			m_First = nullptr;
			m_Last = nullptr;
		}

		if(m_First) HDelete(CInterface, m_First->m_PrevInterface, g_MatrixHeap);
	}

    if(CInterface::m_ClearRects) HDelete(CBuf, CInterface::m_ClearRects, g_MatrixHeap);
    if(g_IFaceList->m_CurrentHint) g_IFaceList->m_CurrentHint->Release();
    g_IFaceList->m_CurrentHint = nullptr;

    //Делаем очистку, т.к. ранее костылём объявили new массивы из конструктора
    if(m_Hull)
    {
        delete[] m_Hull;
        m_Hull = nullptr;
    }
    if(m_Chassis)
    {
        delete[] m_Chassis;
        m_Chassis = nullptr;
    }
    if(m_Head)
    {
        delete[] m_Head;
        m_Head = nullptr;
    }
    if(m_Weapon)
    {
        delete[] m_Weapon;
        m_Weapon = nullptr;
    }
    if(m_WeaponPylonNumsForPicsInMenu)
    {
        delete[] m_WeaponPylonNumsForPicsInMenu;
        m_WeaponPylonNumsForPicsInMenu = nullptr;
    }
}

void CIFaceList::ShowInterface()
{
	DTRACE();
	CInterface* pIFace = m_First;
	while(pIFace)
    {
        pIFace->Init();
        if(m_CurrentHint)
        {
            CIFaceElement* els = pIFace->m_FirstElement;
            while(els)
            {
                if(els->m_strName == m_CurrentHintControlName)
                {
                    if(!els->GetVisibility())
                    {
                        m_CurrentHint->Release();
                        m_CurrentHint = nullptr;
                        m_CurrentHintControlName = L"";
                    }
                    break;
                }
                els = els->m_NextElement;
            }
        }
		pIFace = pIFace->m_NextInterface;
	}
    if(g_PopupMenu)
    {
        g_PopupMenu->m_MenuGraphics->Init();
    }
}

void CIFaceList::BeforeRender(void)
{
	DTRACE();

    CInterface::ClearRects_Clear();

    DCP();

	CInterface *pIFace = m_First;
	while(pIFace)
    {
    DCP();
		if(pIFace->m_VisibleAlpha)
        {
    DCP();

			pIFace->BeforeRender();
		}
    DCP();
		pIFace = pIFace->m_NextInterface;
	}
    DCP();
    if(g_PopupMenu)
    {
    DCP();
        g_PopupMenu->m_MenuGraphics->BeforeRender();
    }
    DCP();

}

void CIFaceList::Render()
{
	DTRACE();
	CInterface *pIFace = m_First;
	while(pIFace)
    {
		if(pIFace->m_VisibleAlpha)
        {
			pIFace->Render();
		}
		pIFace = pIFace->m_NextInterface;
	}
    if(g_PopupMenu)
    {
        g_PopupMenu->m_MenuGraphics->Render();
    }
}

//Обработчик движения курсора по элементам интерфейса
bool CIFaceList::OnMouseMove(CPoint mouse)
{
    DTRACE();

    //Проверяем, на какой элемент интерфейса был наведён курсор
    bool bCatch = false;
    CInterface* pIFace = m_First;
    while(pIFace)
    {
        if(pIFace->m_VisibleAlpha && pIFace->OnMouseMove(mouse))
        {
            bCatch = true;
            m_FocusedInterface = pIFace;
        }

        pIFace = pIFace->m_NextInterface;
    }

    if(g_PopupMenu)
    {
        if(g_PopupMenu->m_MenuGraphics->m_VisibleAlpha && g_PopupMenu->m_MenuGraphics->OnMouseMove(mouse))
        {
            bCatch = true;
            m_FocusedInterface = g_PopupMenu->m_MenuGraphics;
        }
    }

    return bCatch;
}

//Первый обработчик левого клика мышью по элементу интерфейса
bool CIFaceList::OnMouseLBDown()
{
	bool bCatch = false;
    CInterface* pIFace = m_First;
	while(pIFace)
    {
		if(pIFace->m_VisibleAlpha)
        {
            bCatch = pIFace->OnMouseLBDown();
		}
		pIFace = pIFace->m_NextInterface;
	}

    if(g_PopupMenu)
    {
        bCatch = g_PopupMenu->m_MenuGraphics->OnMouseLBDown();
    }
	
    return bCatch;
}

//Первый обработчик правого клика мышью по элементу интерфейса
bool CIFaceList::OnMouseRBDown()
{
    if(FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu)
    {
        g_PopupMenu->ResetMenu(true);
        return false;
    }

	bool bCatch = false;
	CInterface* pIFace = m_First;
	while(pIFace)
    {
		if(pIFace->m_VisibleAlpha)
        {
            bCatch = pIFace->OnMouseRBDown();
		}
		pIFace = pIFace->m_NextInterface;
	}

	return bCatch;
}

//Первый обработчик левого отклика мышью по элементу интерфейса
void CIFaceList::OnMouseLBUp()
{
    CInterface* pIFace = m_First;
	while(pIFace)
    {
		if(pIFace->m_VisibleAlpha)
        {
			pIFace->OnMouseLBUp();
		}
		pIFace = pIFace->m_NextInterface;
	}

    if(g_PopupMenu)
    {
        g_PopupMenu->m_MenuGraphics->OnMouseLBUp();
    }
}

//Первый обработчик правого отклика мышью
void CIFaceList::OnMouseRBUp()
{
    CInterface* pIFace = m_First;
    while(pIFace)
    {
        if(pIFace->m_VisibleAlpha)
        {
            pIFace->OnMouseRBUp();
        }
        pIFace = pIFace->m_NextInterface;
    }
}

void CIFaceList::LogicTact(int ms)
{
    g_IFaceList->m_InFocus = UNKNOWN;
    if(g_IFaceList->OnMouseMove(g_MatrixMap->m_Cursor.GetPos()))
    {
		g_IFaceList->m_InFocus = INTERFACE;
	}

    CInterface* i = m_First;
    while(i)
    {
        i->LogicTact(ms);
        i = i->m_NextInterface;
    }
    if(g_PopupMenu)
    {
        g_PopupMenu->m_MenuGraphics->LogicTact(ms);
    }
    ShowInterface();

    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    //Cursor logic
    //Стратегический режим
    if(!ps->IsArcadeMode())
    {
        CPoint mp = g_MatrixMap->m_Cursor.GetPos();

        if(
            (mp.x >= 0 && mp.x < g_ScreenX && mp.y >= 0 && mp.y < g_ScreenY) &&
            (mp.x < MOUSE_BORDER || mp.x > (g_ScreenX - MOUSE_BORDER) || mp.y < MOUSE_BORDER || mp.y > (g_ScreenY - MOUSE_BORDER))
          )
        {
            g_MatrixMap->m_Cursor.Select(CURSOR_STAR);
        }
        else
        {
            //Приказ "Атака"
            if(FLAG(g_IFaceList->m_IfListFlags, PREORDER_FIRE | PREORDER_BOMB))
            {
                //Под прицелом находится вражеский объект
                if(g_MatrixMap->IsTraceNonPlayerObj())
                {
                    //Устанавливаем курсор CROSS_RED
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_RED);
                }
                else
                {
                    //Устанавливаем курсор CROSS_BLUE
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_BLUE);
                }
            }
            //Приказ захватывать
            else if(FLAG(g_IFaceList->m_IfListFlags, PREORDER_CAPTURE))
            {
                //Под прицелом находится не игроковское здание
                if(
                     IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj) &&
                     g_MatrixMap->m_TraceStopObj->GetObjectType() == OBJECT_TYPE_BUILDING &&
                     g_MatrixMap->m_TraceStopObj->GetSide() != PLAYER_SIDE
                  )
                {
                    //Устанавливаем курсор CROSS_RED
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_RED);
                }
                else
                {
                    //Устанавливаем курсор CROSS_BLUE
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_BLUE);
                }
            }
            ///Приказ патруль или идти
            else if(FLAG(g_IFaceList->m_IfListFlags, PREORDER_MOVE | PREORDER_PATROL | PREORDER_REPAIR))
            {
                //Устанавливаем курсор CROSS_BLUE
                g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_BLUE);
            }
            else if(g_IFaceList->m_InFocus == INTERFACE)
            {
                //Устанавливаем курсор ARROW, либо стрелку для миникарты
                if(m_FocusedInterface->m_strName != IF_MINI_MAP) g_MatrixMap->m_Cursor.Select(CURSOR_ARROW);
                else g_MatrixMap->m_Cursor.Select(CURSOR_ARROW_MINI_MAP);
            }
            else
            {
                g_MatrixMap->m_Cursor.Select(CURSOR_ARROW);
            }
        }
    }
    //Аркадный режим
    else if(ps->GetArcadedObject()->IsRobotAlive())
    {
        //Если курсор наведён на интерфейс
        if(g_IFaceList->m_InFocus == INTERFACE)
        {
            //Устанавливаем курсор ARROW, либо стрелку для миникарты
            if(m_FocusedInterface->m_strName != IF_MINI_MAP) g_MatrixMap->m_Cursor.Select(CURSOR_ARROW);
            else g_MatrixMap->m_Cursor.Select(CURSOR_ARROW_MINI_MAP);
        }
        else
        {
            //Если под курсором что-либо вражеское или нейтральное
            if(g_MatrixMap->IsTraceNonPlayerObj())
            {
                //Если оружие достреливает
                if(((CMatrixRobotAI*)ps->GetArcadedObject())->CheckFireDist(g_MatrixMap->m_TraceStopPos))
                {
                    //Устанавливаем курсор CROSS_RED
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_RED);
                }
                else
                {
                    //Устанавливаем курсор CROSS_YELLOW
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_YELLOW);
                }
            }
            else
            {
                //Устанавливаем курсор CROSS_BLUE
                g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_BLUE);
            }
        }
    }
}

//Добавляем изображения пушек робота (и их текущего перегрева, если нужно) в меню справа
void CIFaceList::CreateWeaponDynamicStatics()
{
DTRACE();

    DeleteWeaponDynamicStatics();
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

    if(!FLAG(m_IfListFlags, SINGLE_MODE) && !player_side->IsArcadeMode()) return;
    if((player_side->GetCurGroup()->m_FirstObject && player_side->GetCurGroup()->m_FirstObject->GetObject()->GetObjectType() != OBJECT_TYPE_ROBOTAI)) return;

    CMatrixRobotAI* bot = nullptr;
    
    if(player_side->IsArcadeMode())
    {
        CMatrixMapStatic* s = player_side->GetArcadedObject();
        if(s && s->GetObjectType() == OBJECT_TYPE_ROBOTAI)
        {
            bot = (CMatrixRobotAI*)s;    
        }
        else return;
    }
    else bot = (CMatrixRobotAI*)player_side->GetCurGroup()->m_FirstObject->GetObject();
    
    CInterface* interfaces = m_First;

    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            int hull_kind = 0;
            for(int i = 0; i < bot->m_ModulesCount; ++i)
            {
                if(bot->m_Module[i].m_Type == MRT_HULL)
                {
                    hull_kind = bot->m_Module[i].m_Kind;
                    break;
                }
            }
            if(!hull_kind) return; //Если не нашли у робота установленный корпус

            CIFaceImage overheat_image = *interfaces->FindImageByName(CWStr(IF_OVER_HEAT));
            for(int i = 0; i < bot->m_ModulesCount; ++i)
            {
                if(bot->m_Module[i].m_Type == MRT_WEAPON)
                {
                    int pos;
                    for(pos = 0; pos < RUK_WEAPON_PYLONS_COUNT; ++pos)
                    {
                        if(g_IFaceList->m_WeaponPylonNumsForPicsInMenu[hull_kind][pos] == bot->m_Module[i].m_LinkMatrix) break;
                    }

                    CWStr name = g_Config.m_RobotWeaponsConsts[bot->m_Module[i].m_Kind].status_panel_image;

                    CIFaceStatic* s = interfaces->CreateStaticFromImage(g_IFaceList->m_DWeaponX[pos], g_IFaceList->m_DWeaponY[pos], 0.0000001f, *interfaces->FindImageByName(name));
                    s->m_nId = DYNAMIC_WEAPON_ON_ID;
                
                    s = interfaces->CreateStaticFromImage(g_IFaceList->m_DWeaponX[pos], g_IFaceList->m_DWeaponY[pos], 0.000001f, overheat_image);
                    s->m_Param1 = i;
                }
            }

            interfaces->SortElementsByZ();
            break;
        }

        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeleteWeaponDynamicStatics()
{
    DTRACE();
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

    if(!(player_side)) return;

    CInterface* interfaces = m_First;

    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            CIFaceElement* elements = interfaces->m_FirstElement;
            while(elements)
            {
                if(elements->m_nId == DYNAMIC_WEAPON_ON_ID || elements->m_strName == IF_OVER_HEAT)
                {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }

        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::CreateItemPrice(int* price, float multiplier)
{
DTRACE();

    DeleteItemPrice();
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    CInterface* interfaces = m_First;

    int res[MAX_RESOURCES] = { price[0], price[1], price[2], price[3] };

    //player_side->m_ConstructPanel->m_FocusedPrice.ResetPrice();
    //memcpy(player_side->m_ConstructPanel->m_FocusedPrice.m_Resources, price.m_Resources, sizeof(player_side->m_ConstructPanel->m_FocusedPrice.m_Resources));
    
    while(interfaces)
    {
        if(interfaces->m_strName == IF_BASE)
        {
            float x = 22, y = 243, z = 0.00001f;
            CIFaceElement* pElement = interfaces->m_FirstElement;

            while(pElement)
            {
                if(pElement->m_strName == IF_BASE_UNIT_PANEL)
                {
                    x = pElement->m_xPos;
                    y = pElement->m_yPos;
                    z = pElement->m_zPos;
                    break;
                }

                pElement = pElement->m_NextElement;
            }

            CIFaceImage titan_image = *interfaces->FindImageByName(CWStr(IF_BASE_TITAN_IMAGE));
            CIFaceImage electronics_image = *interfaces->FindImageByName(CWStr(IF_BASE_ELECTRONICS_IMAGE));
            CIFaceImage energy_image = *interfaces->FindImageByName(CWStr(IF_BASE_ENERGY_IMAGE));
            CIFaceImage plasma_image = *interfaces->FindImageByName(CWStr(IF_BASE_PLASMA_IMAGE));

            int prev_res = -1; //Для корректировки позиции графики ресурса по длине предыдущего счётчика ресурса
            for(int cnt = 0; cnt < MAX_RESOURCES; ++cnt)
            {
                if(res[cnt])
                {
                    res[cnt] *= multiplier;
                    CIFaceStatic* s = nullptr;
                    if(cnt == TITAN)
                    {
                        s = interfaces->CreateStaticFromImage(x, y, z, titan_image);
                        player_side->m_ConstructPanel->m_TitanResCountX = x;
                        prev_res = cnt;
                    }
                    else if(cnt == ELECTRONICS)
                    {
                        if(prev_res > -1)
                        {
                            if(res[prev_res] < 10) x -= 7;
                            //else if(res[prev_res] < 100) x -= 1;
                            else if(res[prev_res] >= 100) x += 6;
                        }
                        s = interfaces->CreateStaticFromImage(x, y, z, electronics_image);
                        player_side->m_ConstructPanel->m_ElectronicsResCountX = x;
                        prev_res = cnt;
                    }
                    else if(cnt == ENERGY)
                    {
                        if(prev_res > -1)
                        {
                            if(res[prev_res] < 10) x -= 8;
                            else if(res[prev_res] < 100) x -= 1;
                            else /*if(res[prev_res] >= 100)*/ x += 7;
                        }
                        s = interfaces->CreateStaticFromImage(x, y, z, energy_image);
                        player_side->m_ConstructPanel->m_EnergyResCountX = x;
                        prev_res = cnt;
                    }
                    else if(cnt == PLASMA)
                    {
                        if(prev_res > -1)
                        {
                            if(res[prev_res] < 10) x -= 10;
                            else if(res[prev_res] < 100) x -= 2;
                            else /*if(res[prev_res] >= 100)*/ x += 4;
                        }
                        s = interfaces->CreateStaticFromImage(x, y, z, plasma_image);
                        player_side->m_ConstructPanel->m_PlasmaResCountX = x;
                        prev_res = cnt;
                    }

                    if(s)
                    {
                        s->SetVisibility(false);
                        x = x + s->m_xSize + 25;
                        s->m_nId = ITEM_PRICE_ID;
                    }
                }
            }

            interfaces->SortElementsByZ();
            break;
        }

        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeleteItemPrice()
{
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    CInterface* interfaces = m_First;
    while(interfaces)
    {
        if(interfaces->m_strName == IF_BASE)
        {
            CIFaceElement* elements = interfaces->m_FirstElement;
            while(elements)
            {
                if(elements->m_Type == IFACE_DYNAMIC_STATIC && elements->m_nId == ITEM_PRICE_ID)
                {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

//Считает и выводит на экран в конструкторе суммарное количество ресурсов, необходимое для постройки робота
void CIFaceList::CreateSummPrice(float multiplier)
{
    DeleteSummPrice();

    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    CInterface* interfaces = m_First;

    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
    int res[MAX_RESOURCES];
    ZeroMemory(res, sizeof(res));

    player_side->m_Constructor->GetConstructionPrice(res);

    int fuck = 0;
    for(int i = 0; i < MAX_RESOURCES; ++i)
    {
        if(res[i])
        {
            if(multiplier) res[i] *= multiplier;
            ++fuck;
        }
    }

    while(interfaces)
    {
        if(interfaces->m_strName == IF_BASE)
        {
            float x = 200, y = 352, z = 0.00001f;

            CIFaceElement* pElement = interfaces->m_FirstElement;

            while(pElement)
            {
                if(pElement->m_strName == IF_BASE_SUMM_PANEL)
                {
                    x = pElement->m_xPos;
                    y = pElement->m_yPos;
                    z = pElement->m_zPos;

                    break;
                }

                pElement = pElement->m_NextElement;
            }

            if(fuck == 3) x += 35;
            else if(fuck == 2) x += 50;

            CIFaceImage titan_image = *interfaces->FindImageByName(CWStr(IF_BASE_TITAN_IMAGE));
            CIFaceImage electronics_image = *interfaces->FindImageByName(CWStr(IF_BASE_ELECTRONICS_IMAGE));
            CIFaceImage energy_image = *interfaces->FindImageByName(CWStr(IF_BASE_ENERGY_IMAGE));
            CIFaceImage plasma_image = *interfaces->FindImageByName(CWStr(IF_BASE_PLASMA_IMAGE));
            CIFaceImage warning_image = *interfaces->FindImageByName(CWStr(IF_BASE_WARNING));

            for(int cnt = 0; cnt < MAX_RESOURCES; ++cnt)
            {
                if(res[cnt] != 0)
                {
                    CIFaceStatic* s = nullptr;
                    int warning_id = DYNAMIC_WARNING;
                    if(cnt == TITAN)
                    {
                        player_side->m_ConstructPanel->m_Configs[cfg_num].m_titX = Float2Int(x) - 2;
                        s = interfaces->CreateStaticFromImage(x, y, z, titan_image);
                        s->m_strName = IF_BASE_TITAN_SUMM;
                    }
                    else if(cnt == ELECTRONICS)
                    {
                        player_side->m_ConstructPanel->m_Configs[cfg_num].m_elecX = Float2Int(x) - 2;
                        s = interfaces->CreateStaticFromImage(x, y, z, electronics_image);
                        s->m_strName = IF_BASE_ELECTRONICS_SUMM;
                        warning_id += 1;
                    }
                    else if(cnt == ENERGY)
                    {
                        player_side->m_ConstructPanel->m_Configs[cfg_num].m_enerX = Float2Int(x) - 2;
                        s = interfaces->CreateStaticFromImage(x, y, z, energy_image);
                        s->m_strName = IF_BASE_ENERGY_SUMM;
                        warning_id += 2;
                    }
                    else if(cnt == PLASMA)
                    {
                        player_side->m_ConstructPanel->m_Configs[cfg_num].m_plasX = Float2Int(x) - 2;
                        s = interfaces->CreateStaticFromImage(x, y, z, plasma_image);
                        s->m_strName = IF_BASE_PLASMA_SUMM;
                        warning_id += 3;
                    }

                    if(s)
                    {
                        s->SetVisibility(false);
                        CIFaceStatic* swarn = interfaces->CreateStaticFromImage(x + s->m_xSize, y + 22, z, warning_image);
                        x = x + s->m_xSize + 31;
                        s->m_nId = SUMM_PRICE_ID;
                        swarn->m_nId = warning_id;
                        swarn->SetVisibility(true);
                    }
                }
            }

            interfaces->SortElementsByZ();
            break;
        }

        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeleteSummPrice()
{
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

    CInterface* interfaces = m_First;
    while(interfaces)
    {
        if(interfaces->m_strName == IF_BASE)
        {
            CIFaceElement* elements = interfaces->m_FirstElement;
            while(elements)
            {
/*                if(elements->m_strName == IF_BASE_SUMM_PANEL){
                    elements->m_StateImages[IFACE_NORMAL].m_Caption = CWStr(L"");
                    elements->m_StateImages[IFACE_NORMAL].SetStateText(true);
                }
                else */if(elements->m_Type == IFACE_DYNAMIC_STATIC && (elements->m_nId == SUMM_PRICE_ID || IS_DYNAMIC_WARNING(elements->m_nId)))
                {

                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::SlideFocusedInterfaceRight()
{
    if(m_FocusedInterface)
    {
        m_FocusedInterface->BeginSlide(m_FocusedInterface->m_xPos + 100, m_FocusedInterface->m_yPos);
    }
}

void CIFaceList::SlideFocusedInterfaceLeft()
{
    if(m_FocusedInterface)
    {
        m_FocusedInterface->BeginSlide(m_FocusedInterface->m_xPos - 100, m_FocusedInterface->m_yPos);
    }
}

//Функция входа в режим ручного контроля (работает как для роботов, так и для вертолётов)
void CIFaceList::EnterArcadeMode(bool pos)
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    if(ps->GetCurGroup() && ps->GetCurGroup()->m_FirstObject)
    {
        //Определяем, какой юнит из текущей выбранной группы пойдёт под прямой контроль игрока (первый)
        CMatrixMapStatic* o = ps->GetCurGroup()->m_FirstObject->GetObject();
        if(pos)
        {
            o = ps->GetCurGroup()->GetObjectByN(ps->GetCurSelNum());
        }

        //
        ps->SetArcadedObject(o);

        //Переставляем все элементы интерфейса под аркадный режим
        CInterface* ifs = g_IFaceList->m_First;
        while(ifs)
        {
            if(ifs->m_strName == IF_MAIN)
            {
                ifs->m_xPos = float(g_ScreenX - (1024 - (447 + 196)));
                ifs->ReCalcElementsPos();
            }
            ifs = ifs->m_NextInterface;
        }
    }
}

//Функция выхода из режима ручного контроля (работает как для роботов, так и для вертолётов)
void CIFaceList::ExitArcadeMode()
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    if(ps->IsArcadeMode())
    {
        CMatrixMapStatic* obj = ps->GetArcadedObject();
        ESelType type = NOTHING;

        if(obj)
        {
            int obj_type = obj->GetObjectType();
            if(obj_type == OBJECT_TYPE_ROBOTAI)
            {
                type = ROBOT;

                CMatrixRobotAI* robot = obj->AsRobot();
                robot->SetAfterManual(true);

                //Если игрок выключил часть орудий, пока находился в ручном контроле, включаем их обратно
                for(int i = 0; i < robot->m_WeaponsCount; ++i)
                {
                    if(robot->m_Weapons[i].IsEffectPresent()) robot->m_Weapons[i].m_On = true;
                }
            }
            else if(obj_type == OBJECT_TYPE_FLYER)
            {
                type = FLYER;
                obj->AsFlyer()->SetRollAngle(0); //Выравниваем возможный крен вертолёта
            }
        }

        //Если игрок включил управление наведением ракет курсором мыши, пока находился в ручном режиме, выключаем его
        if(g_Config.IsManualMissileControl()) g_Config.m_IsManualMissileControl = false;

        ps->SetArcadedObject(nullptr);

        CMatrixGroup* grp = ps->GetCurSelGroup();

        grp->RemoveAll();
        grp->AddObject(obj, -4);

        ps->SetCurGroup(ps->CreateGroupFromCurrent());
        ps->PGOrderStop(ps->SelGroupToLogicGroup());
        ps->SelectedGroupBreakOrders();
        ps->Select(type, obj);

        //Переставляем все элементы интерфейса под стратегический режим
        CInterface* ifs = g_IFaceList->m_First;
        while(ifs)
        {
            if(ifs->m_strName == IF_MAIN)
            {
                ifs->m_xPos = float(g_ScreenX - (1024 - 447));
                ifs->ReCalcElementsPos();
                break;
            }
            ifs = ifs->m_NextInterface;
        }
    }
}

//Выполняет действие по клику ЛКМ или нажатию кнопки на клавиатуре
void __stdcall CIFaceList::PlayerAction(void* object)
{
    if(!object) return;
    
    CIFaceElement* element = (CIFaceElement*)object;
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    //Для отладки
    //SFT(CStr(element->m_strName));

    if(element->m_strName == IF_MAIN_MENU_BUTTON)
    {
        g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_MENU);
    }
    
    //Программа атаки
    //Кнопка включённого режима
    if(element->m_strName == IF_AORDER_ATTACK_ON)
    {
        RESETFLAG(m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);

        ps->PGOrderStop(ps->SelGroupToLogicGroup());
        ps->SelectedGroupBreakOrders();
    }
    //Кнопка выключенного режима
    else if(element->m_strName == IF_AORDER_ATTACK_OFF)
    {
        RESETFLAG(m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
        SETFLAG(m_IfListFlags, AUTO_ATTACK_ON);

        ps->PGOrderAutoAttack(ps->SelGroupToLogicGroup());
    }
    
    //Программа захвата
    //Кнопка включённого режима
    if(element->m_strName == IF_AORDER_CAPTURE_ON)
    {
        RESETFLAG(m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);

        ps->PGOrderStop(ps->SelGroupToLogicGroup());
        ps->SelectedGroupBreakOrders();
    }
    //Кнопка выключенного режима
    else if(element->m_strName == IF_AORDER_CAPTURE_OFF)
    {
        RESETFLAG(m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
        SETFLAG(m_IfListFlags, AUTO_CAPTURE_ON);

        ps->PGOrderAutoCapture(ps->SelGroupToLogicGroup());
    }

    //Программа защиты
    //Кнопка включённого режима
    if(element->m_strName == IF_AORDER_PROTECT_ON)
    {
        RESETFLAG(m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);

        ps->PGOrderStop(ps->SelGroupToLogicGroup());
        ps->SelectedGroupBreakOrders();
    }
    //Кнопка выключенного режима
    else if(element->m_strName == IF_AORDER_PROTECT_OFF)
    {
        RESETFLAG(m_IfListFlags, AUTO_ATTACK_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
        SETFLAG(m_IfListFlags, AUTO_PROTECT_ON);

        ps->PGOrderAutoDefence(ps->SelGroupToLogicGroup());
    }

    //Моментальный подрыв робота с бомбой из режима ручного управления
    if(element->m_strName == IF_MAIN_SELF_BOMB || element->m_strName == IF_MAIN_SELF_BOMB_AUTO)
    {
        if(ps->IsArcadeMode() && ps->GetArcadedObject()->IsRobotAlive())
        {
            ps->GetArcadedObject()->AsRobot()->BigBoom();
        }
    }

    //Выбор приказа или кнопки отмены после выбора приказа
    if(element->m_strName == IF_ORDER_FIRE)
    {
        SETFLAG(m_IfListFlags, PREORDER_FIRE);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if(element->m_strName == IF_ORDER_CAPTURE)
    {
        SETFLAG(m_IfListFlags, PREORDER_CAPTURE);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if(element->m_strName == IF_ORDER_PATROL)
    {
        SETFLAG(m_IfListFlags, PREORDER_PATROL);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if(element->m_strName == IF_ORDER_MOVE)
    {
        SETFLAG(m_IfListFlags, PREORDER_MOVE);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if(element->m_strName == IF_ORDER_REPAIR)
    {
        SETFLAG(m_IfListFlags, PREORDER_REPAIR);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    //В зависимости от того, выбран или не выбран режим автоподрыва, используются две разные кнопки, но действие по ЛКМ у них идентично
    else if(element->m_strName == IF_ORDER_BOMB || element->m_strName == IF_ORDER_BOMB_AUTO)
    {
        SETFLAG(m_IfListFlags, PREORDER_BOMB);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if(element->m_strName == IF_ORDER_CANCEL)
    {
        if(ps->m_CurrentAction == BUILDING_TURRET)
        {
            ps->m_CannonForBuild.Delete();
            ps->m_CurrentAction = NOTHING_SPECIAL;
        }

        ResetOrderingMode();
    }
    else if(element->m_strName == IF_ORDER_STOP)
    {
        ps->PGOrderStop(ps->SelGroupToLogicGroup());
        ps->SelectedGroupBreakOrders();

        /*
        if(ps->m_CurGroup->m_Tactics)
        {
            ps->m_CurGroup->DeInstallTactics();
        }
        else
        {
            ps->SelectedGroupBreakOrders();
        }
        */
    }

    if(element->m_strName == IF_ENTER_ARCADE_MODE)
    {
        EnterArcadeMode();
    }
    else if(element->m_strName == IF_EXIT_ARCADE_MODE)
    {
        ExitArcadeMode();
    }

    if(element->m_strName == IF_BASE_CONST_CANCEL)
    {
        ps->m_ConstructPanel->ResetGroupNClose();
    }

    //Нажали кнопку выбора типа пушки для постройки
    if(element->m_strName == IF_BUILD_TURRET)
    {
        ps->m_ConstructPanel->ResetGroupNClose();
        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET);
        if(ps->m_ActiveObject)
        {
            ((CMatrixBuilding*)ps->m_ActiveObject)->CreatePlacesShow();
        }
    }
    //Нажали кнопку выбора типа вертолёта для постройки
    else if(element->m_strName == IF_BUILD_FLYER)
    {
        ps->m_ConstructPanel->ResetGroupNClose();
        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_FLYER);
    }
    //Нажали кнопку вызова подкрепления (вероятно)
    else if(element->m_strName == IF_CALL_FROM_HELL && (ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED))
    {
        CMatrixBuilding* bld = (CMatrixBuilding*)ps->m_ActiveObject;
        bld->Reinforcements();
    }
    //Нажали кнопку выбора типа ремонта для хз чего
    else if(element->m_strName == IF_BUILD_REPAIR)
    {
        ps->m_ConstructPanel->ResetGroupNClose();
        SETFLAG(m_IfListFlags, ORDERING_MODE);
        SETFLAG(m_IfListFlags, PREORDER_BUILD_REPAIR);       
    }

    if(ps->m_CurrSel == BASE_SELECTED  || ps->m_CurrSel == BUILDING_SELECTED)
    {
        CMatrixBuilding* base = (CMatrixBuilding*)ps->m_ActiveObject;

        //Нажали кнопку постройки конкретной турели
        if(element->m_strName == IF_BUILD_TUR1)
        {
            /*
            if(((CMatrixBuilding*)ps->m_ActiveObject)->HaveMaxTurrets())
            {
                //sound
                CSound::Play(S_CANTBE, SL_INTERFACE);
                //hint
            }
            else
            {
            */
            BeginBuildTurret(1);
            //}
        }
        else if(element->m_strName == IF_BUILD_TUR2)
        {
            if(0/*((CMatrixBuilding*)ps->m_ActiveObject)->HaveMaxTurrets()*/)
            {
                //sound
                CSound::Play(S_CANTBE, SL_INTERFACE);
                //hint
            }
            else
            {
                BeginBuildTurret(2);                
            }
        }
        else if(element->m_strName == IF_BUILD_TUR3)
        {
            if(0/*((CMatrixBuilding*)ps->m_ActiveObject)->HaveMaxTurrets()*/)
            {
                //sound
                CSound::Play(S_CANTBE, SL_INTERFACE);
                //hint
            }
            else
            {
                BeginBuildTurret(3);                
            }
        }
        else if(element->m_strName == IF_BUILD_TUR4)
        {
            if(0/*((CMatrixBuilding*)ps->m_ActiveObject)->HaveMaxTurrets()*/)
            {
                //sound
                CSound::Play(S_CANTBE, SL_INTERFACE);
                //hint
            }
            else
            {
                BeginBuildTurret(4);
            }
        }

        //Нажали кнопку постройки конкретного вертолёта
        if(ps->m_CurrSel == BASE_SELECTED)
        {
            if(element->m_strName == IF_BUILD_FLYER_1)
            {
                //CMatrixFlyer* fl = g_MatrixMap->StaticAdd<CMatrixFlyer>();
                //fl->m_FlyerKind = FLYER_SPEED;
                //fl->SetDeliveryCopter(false);
                //base->m_BS.AddItem(fl);
                base->BuildFlyer(FLYER_SPEED);
            }
            else if(element->m_strName == IF_BUILD_FLYER_2)
            {
                //CMatrixFlyer* fl = g_MatrixMap->StaticAdd<CMatrixFlyer>();
                //fl->m_FlyerKind = FLYER_TRANSPORT;
                //fl->SetDeliveryCopter(false);
                //base->m_BS.AddItem(fl);
                base->BuildFlyer(FLYER_TRANSPORT);
            }
            else if(element->m_strName == IF_BUILD_FLYER_3)
            {
                //CMatrixFlyer* fl = g_MatrixMap->StaticAdd<CMatrixFlyer>();
                //fl->m_FlyerKind = FLYER_BOMB;
                //fl->SetDeliveryCopter(false);
                //base->m_BS.AddItem(fl);
                base->BuildFlyer(FLYER_BOMB);
            }
            else if(element->m_strName == IF_BUILD_FLYER_4)
            {
                //CMatrixFlyer* fl = g_MatrixMap->StaticAdd<CMatrixFlyer>();
                //fl->m_FlyerKind = FLYER_ATTACK;
                //fl->SetDeliveryCopter(false);
                //base->m_BS.AddItem(fl);
                base->BuildFlyer(FLYER_ATTACK);
            }
        }
    }

    int* cfg_num = &ps->m_ConstructPanel->m_CurrentConfig;
}

//Выполняет альтернативное действие при получении того же маркера кнопки, что и в PlayerAction()
void __stdcall CIFaceList::PlayerAltAction(void* object)
{
    if(!object) return;

    CIFaceElement* element = (CIFaceElement*)object;
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    //Кнопка бомбы, если режим автоподрыва выключен
    if(element->m_strName == IF_ORDER_BOMB)
    {
        ps->PGAutoBoomSet(g_MatrixMap->GetPlayerSide()->GetCurGroup(), true);
    }
    //Кнопка бомбы, если режим автоподрыва включён
    else if(element->m_strName == IF_ORDER_BOMB_AUTO)
    {
        ps->PGAutoBoomSet(g_MatrixMap->GetPlayerSide()->GetCurGroup(), false);
    }
    //Кнопка моментального подрыва бомбы в режиме ручного управления, если режим автоподрыва для данного юнита выключен
    else if(element->m_strName == IF_MAIN_SELF_BOMB)
    {
        g_MatrixMap->GetPlayerSide()->GetArcadedObject()->AsRobot()->AutoBoomSet(true);
    }
    //Кнопка моментального подрыва бомбы в режиме ручного управления, если режим автоподрыва для данного юнита включён
    else if(element->m_strName == IF_MAIN_SELF_BOMB_AUTO)
    {
        g_MatrixMap->GetPlayerSide()->GetArcadedObject()->AsRobot()->AutoBoomSet(false);
    }

    //Это здесь как-то нужно для корректного вывода списка замены деталей в конструкторе - не удалять
    int* cfg_num = &ps->m_ConstructPanel->m_CurrentConfig;
}

void CIFaceList::CreateGroupSelection(CInterface* iface)
{
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    int sel_objs = 9;

    float x = 225, y = 49, z = 0.000001f;
    CIFaceImage ramka_image = *iface->FindImageByName(CWStr(IF_GROUP_RAMKA));
    for(int i = 0; i < sel_objs; ++i)
    {
        float pos = (i + 1.0f) / 3.0f;

        if(pos <= 1)
        {
            x = (float)(225 + 48 * i);
            y = 49;
        }
        else if(pos > 1 && pos <= 2)
        {
            x = (float)((225 + 48 * i) - 48 * 3);
            y = 49 * 2;
        }
        else if(pos > 2)
        {
            x = (float)((225 + 48 * i) - 48 * 6);
            y = 49 * 3;
        }
        
        CIFaceStatic* s = iface->CreateStaticFromImage(x, y, z, ramka_image);
        if(s)
        {
            s->SetVisibility(false);
            s->m_nId = GROUP_SELECTION_ID+i;
        }
    }
}

void CIFaceList::DeleteGroupSelection()
{
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

    CMatrixMapStatic* so = CMatrixMapStatic::GetFirstLogic();

    while(so)
    {
        if(so->IsRobot())
        {
            so->AsRobot()->DeleteProgressBarClone(PBC_CLONE1);
        }
        else if(so->IsFlyer())
        {
            so->AsFlyer()->DeleteProgressBarClone(PBC_CLONE1);
        }
        so = so->GetNextLogic();
    }
    
    CInterface* interfaces = m_First;
    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            CIFaceElement* elements = interfaces->m_FirstElement;
            while(elements)
            {
                if(elements->m_Type == IFACE_DYNAMIC_STATIC && elements->m_nId == GROUP_SELECTION_ID)
                {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }

            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}
void CIFaceList::DeleteProgressBars(CMatrixMapStatic* from)
{
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    if(!player_side->GetCurGroup()) return;

    CMatrixGroupObject* gos = player_side->GetCurGroup()->m_FirstObject;

    if(!from)
    {
        CMatrixMapStatic* so = CMatrixMapStatic::GetFirstLogic();

        while(so)
        {
            if(so->GetObjectType() == OBJECT_TYPE_ROBOTAI)
            {
                ((CMatrixRobotAI*)so)->DeleteProgressBarClone(PBC_CLONE1);
            }
            else if(so->GetObjectType() == OBJECT_TYPE_FLYER)
            {
                ((CMatrixFlyer*)so)->DeleteProgressBarClone(PBC_CLONE1);
            }
            
            so = so->GetNextLogic();
        }

    }
    else
    {
        while(gos)
        {
            if(gos->GetObject() == from) break;
            gos = gos->m_NextObject;
        }

        while(gos)
        {
            if(gos->GetObject()->IsRobot())
            {
                gos->GetObject()->AsRobot()->DeleteProgressBarClone(PBC_CLONE1);
            }
            else if(gos->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER)
            {
                ((CMatrixFlyer*)gos->GetObject())->DeleteProgressBarClone(PBC_CLONE1);
            }
            gos = gos->m_NextObject;
        }
    }
}

void CIFaceList::CreateGroupIcons()
{
DTRACE();

    DeleteGroupIcons();
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    if(!player_side->GetCurGroup()) return;

    int sel_objs = player_side->GetCurGroup()->GetRobotsCnt() + player_side->GetCurGroup()->GetFlyersCnt();
    if(!sel_objs) return;

    CInterface* interfaces = m_First;
    CMatrixGroupObject* so = player_side->GetCurGroup()->m_FirstObject;

    CIFaceImage* image = HNew(g_MatrixHeap) CIFaceImage;

    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            float x = 225, y = 49, z = 0;
            int pos = 0;
            for(int i = 0; i < sel_objs; ++i)
            {
                if(i < 3) y = 49;
                else if(i < 6) y = 49 * 2;
                else if(i < 9) y = 49 * 3;

                x = (float)((225 + 48 * pos));
                ++pos;

                if(x > (225 + 48 * 2))
                {
                    pos = 1;
                    x = 225;
                }

                CTextureManaged* tex = nullptr;
                bool robot = false, flyer = false;
                float xmed = 0, ymed = 0;

                if(so)
                {
                    if(so->GetObject()->IsRobotAlive())
                    {
                        tex = so->GetObject()->AsRobot()->GetMedTexture();
                        robot = true;
                    }
                }
                
                if(tex)
                {
                    image->m_Image = tex;
                    image->m_Height = 36;
                    image->m_Width = 47;
                    image->m_NextImage = nullptr;
                    image->m_PrevImage = nullptr;
                    image->m_strName = L"";
                    image->m_Type = IFACE_IMAGE;

                    CIFaceStatic* s = nullptr;
                    if(robot)
                    {
                        image->m_TexHeight = 64;
                        image->m_TexWidth = 64;
                        image->m_xTexPos = 0;
                        image->m_yTexPos = 0;
                        s = interfaces->CreateStaticFromImage(x, y, z, *image, true);
                    }

                    if(s)
                    {
                        s->SetVisibility(true);
                        s->m_nId = GROUP_ICONS_ID + i;
                    }
                }
                so = so->m_NextObject;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }

    HDelete(CIFaceImage, image, g_MatrixHeap);
}

void CIFaceList::DeleteGroupIcons()
{
DTRACE();

    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

    CMatrixMapStatic* so = CMatrixMapStatic::GetFirstLogic();

    CInterface* interfaces = m_First;
    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            CIFaceElement* elements = interfaces->m_FirstElement;
            while(elements)
            {
                if(elements->m_Type == IFACE_DYNAMIC_STATIC && IS_GROUP_ICON(elements->m_nId))
                {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

//Создаёт иконки юнитов в выбранной группе для группового списка
void CIFaceList::CreatePersonal()
{
DTRACE();

    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    if(!player_side->GetCurGroup()) return;
    
    int selected = player_side->GetCurSelNum();

    CMatrixGroup* group = player_side->GetCurGroup();
    
    CMatrixGroupObject* go = group->m_FirstObject;
    for(int i = 0; i < selected && go; ++i) go = go->m_NextObject;
    
    if(!go) return;

    CInterface* interfaces = g_IFaceList->m_First;
    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            CTextureManaged* tex = nullptr;
            float xbig = 0, ybig = 0;
            bool flyer = false;
            bool robot = false;

            CIFaceImage* image = HNew(g_MatrixHeap) CIFaceImage;
            
            if(go->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI)
            {
                tex = ((CMatrixRobotAI*)go->GetObject())->GetBigTexture();
                robot = true;
            }
            else if(go->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER)
            {
                CIFaceImage* img;

                int flyer_kind = ((CMatrixFlyer*)go->GetObject())->m_FlyerKind;
                if(flyer_kind == FLYER_SPEED) img = interfaces->FindImageByName(CWStr(IF_FLYER_1_IMG));
                else if(flyer_kind == FLYER_TRANSPORT) img = interfaces->FindImageByName(CWStr(IF_FLYER_2_IMG));
                else if(flyer_kind == FLYER_BOMB) img = interfaces->FindImageByName(CWStr(IF_FLYER_3_IMG));
                else /*if(flyer_kind == FLYER_ATTACK)*/ img = interfaces->FindImageByName(CWStr(IF_FLYER_4_IMG));

                tex = img->m_Image;
                xbig = img->m_xTexPos;
                ybig = img->m_yTexPos;

                flyer = true;
            }
            
            if(tex)
            {
                image->m_Image = tex;
                image->m_Height = 114;
                image->m_Width = 114;
                image->m_NextImage = nullptr;
                image->m_PrevImage = nullptr;
                image->m_strName = L"";
                image->m_Type = IFACE_IMAGE;
                
                CIFaceStatic* s = nullptr;
                if(robot)
                {
                    image->m_TexHeight = 256;
                    image->m_TexWidth = 256;
                    image->m_xTexPos = 0;
                    image->m_yTexPos = 0;
                    s = interfaces->CreateStaticFromImage(81, 61, 0.0000001f, *image, true);
                }
                else if(flyer)
                {
                    image->m_TexHeight = 512;
                    image->m_TexWidth = 512;
                    image->m_xTexPos = xbig;
                    image->m_yTexPos = ybig;
                    s = interfaces->CreateStaticFromImage(81, 61, 0.0000001f, *image, false);
                }
                
                if(s)
                {
                    void *cl, *fn;
                    FSET(ON_PRESS, s, cl, fn, g_IFaceList, CIFaceList::JumpToRobot);
                    s->SetVisibility(true);
                    s->m_nId = PERSONAL_ICON_ID;
                }
            }

            interfaces->SortElementsByZ();
            HDelete(CIFaceImage, image, g_MatrixHeap);
            break;
        }

        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeletePersonal()
{
DTRACE();

    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

//icon

    CInterface* interfaces = m_First;
    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            CIFaceElement* elements = interfaces->m_FirstElement;
            while(elements)
            {
                if(elements->m_Type == IFACE_DYNAMIC_STATIC && elements->m_nId == PERSONAL_ICON_ID)
                {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }

            interfaces->SortElementsByZ();
            break;
        }

        interfaces = interfaces->m_NextInterface;
    }

//progress bar
    CMatrixMapStatic* s = CMatrixMapStatic::GetFirstLogic();

    while(s)
    {
        if(s->IsRobot())
        {
            s->AsRobot()->DeleteProgressBarClone(PBC_CLONE2);
        }
        else if(s->GetObjectType() == OBJECT_TYPE_FLYER)
        {
            ((CMatrixFlyer*)s)->DeleteProgressBarClone(PBC_CLONE2);
        }
        else if(s->IsBuilding())
        {
            s->AsBuilding()->DeleteProgressBarClone(PBC_CLONE2);
        }
        
        s = s->GetNextLogic();
    }
}


void CIFaceList::CreateOrdersGlow(CInterface *iface)
{
    DTRACE();
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    int orders = 6;

    float x = 419, y = 47, z = 0.0000001f;
    for(int i = 0; i < orders; ++i)
    {
        float pos = (i+1.0f) / 3.0f;
        
        if(pos <= 1)
        {
            x = (float)(419 + 49 * i);
            y = 47;
        }
        else if(pos > 1 && pos <= 2)
        {
            x = (float)((419 + 49 * i) - 49 * 3);
            y = 47+49;
        }
        
        CIFaceStatic* s = iface->CreateStaticFromImage(x, y, z, *iface->FindImageByName(CWStr(IF_ORDER_GLOW)));
        if(s)
        {
            s->SetVisibility(false);
            s->m_nId = ORDERS_GLOW_ID + i;
        }
    }
}

void CIFaceList::ResetOrderingMode()
{
    DTRACE();
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    if(ps->m_ActiveObject && ps->m_ActiveObject->IsBuilding())
    {
        ((CMatrixBuilding*)ps->m_ActiveObject)->DeletePlacesShow();
    }

    RESETFLAG(m_IfListFlags, ORDERING_MODE);
    m_IfListFlags &= 0x0000ffff;
}

void CIFaceList::CreateQueueIcon(int num, CMatrixBuilding* base, CMatrixMapStatic* object)
{
    DTRACE();
    if(!object) return;

    CInterface* ifs = m_First;
    while(ifs)
    {
        if(ifs->m_strName == IF_MAIN)
        {
            CIFaceStatic* s = nullptr;
            CTextureManaged* tex_med = nullptr, *tex_small = nullptr;
            bool flyer = false;
            bool robot = false;
            bool turret = false;
            float xmed = 0;
            float ymed = 0;
            float xsmall = 0;
            float ysmall = 0;

            if(object->IsRobot())
            {
                tex_med = ((CMatrixRobotAI*)object)->GetMedTexture();
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
                tex_small = ((CMatrixRobotAI*)object)->GetSmallTexture();
#else
                tex_small = ((CMatrixRobotAI*)object)->GetMedTexture();
#endif
                robot = true;
            }
            else if(object->IsFlyer())
            {
                if(((CMatrixFlyer*)object)->m_FlyerKind == FLYER_SPEED)
                {
                    tex_med = ifs->FindImageByName(CWStr(IF_FLYER_1_ICON_BIG))->m_Image;
                    tex_small = ifs->FindImageByName(CWStr(IF_FLYER_1_ICON_SMALL))->m_Image;

                    xmed = ifs->FindImageByName(CWStr(IF_FLYER_1_ICON_BIG))->m_xTexPos;
                    ymed = ifs->FindImageByName(CWStr(IF_FLYER_1_ICON_BIG))->m_yTexPos;
                    xsmall = ifs->FindImageByName(CWStr(IF_FLYER_1_ICON_SMALL))->m_xTexPos;
                    ysmall = ifs->FindImageByName(CWStr(IF_FLYER_1_ICON_SMALL))->m_yTexPos;
                }
                else if(((CMatrixFlyer*)object)->m_FlyerKind == FLYER_TRANSPORT)
                {
                    tex_med = ifs->FindImageByName(CWStr(IF_FLYER_2_ICON_BIG))->m_Image;
                    tex_small = ifs->FindImageByName(CWStr(IF_FLYER_2_ICON_SMALL))->m_Image;

                    xmed = ifs->FindImageByName(CWStr(IF_FLYER_2_ICON_BIG))->m_xTexPos;
                    ymed = ifs->FindImageByName(CWStr(IF_FLYER_2_ICON_BIG))->m_yTexPos;
                    xsmall = ifs->FindImageByName(CWStr(IF_FLYER_2_ICON_SMALL))->m_xTexPos;
                    ysmall = ifs->FindImageByName(CWStr(IF_FLYER_2_ICON_SMALL))->m_yTexPos;
                }
                else if(((CMatrixFlyer*)object)->m_FlyerKind == FLYER_BOMB)
                {
                    tex_med = ifs->FindImageByName(CWStr(IF_FLYER_3_ICON_BIG))->m_Image;
                    tex_small = ifs->FindImageByName(CWStr(IF_FLYER_3_ICON_SMALL))->m_Image;

                    xmed = ifs->FindImageByName(CWStr(IF_FLYER_3_ICON_BIG))->m_xTexPos;
                    ymed = ifs->FindImageByName(CWStr(IF_FLYER_3_ICON_BIG))->m_yTexPos;
                    xsmall = ifs->FindImageByName(CWStr(IF_FLYER_3_ICON_SMALL))->m_xTexPos;
                    ysmall = ifs->FindImageByName(CWStr(IF_FLYER_3_ICON_SMALL))->m_yTexPos;
                }
                else if(((CMatrixFlyer*)object)->m_FlyerKind == FLYER_ATTACK)
                {
                    tex_med = ifs->FindImageByName(CWStr(IF_FLYER_4_ICON_BIG))->m_Image;
                    tex_small = ifs->FindImageByName(CWStr(IF_FLYER_4_ICON_SMALL))->m_Image;

                    xmed = ifs->FindImageByName(CWStr(IF_FLYER_4_ICON_BIG))->m_xTexPos;
                    ymed = ifs->FindImageByName(CWStr(IF_FLYER_4_ICON_BIG))->m_yTexPos;
                    xsmall = ifs->FindImageByName(CWStr(IF_FLYER_4_ICON_SMALL))->m_xTexPos;
                    ysmall = ifs->FindImageByName(CWStr(IF_FLYER_4_ICON_SMALL))->m_yTexPos;
                }

                flyer = true;
            }
            else if(object->IsCannon())
            {
                if(((CMatrixCannon*)object)->m_TurretKind == TURRET_LIGHT_CANNON)
                {
                    tex_med = ifs->FindImageByName(CWStr(IF_TURRET_MED1))->m_Image;
                    tex_small = ifs->FindImageByName(CWStr(IF_TURRET_SMALL1))->m_Image;

                    xmed = ifs->FindImageByName(CWStr(IF_TURRET_MED1))->m_xTexPos;
                    ymed = ifs->FindImageByName(CWStr(IF_TURRET_MED1))->m_yTexPos;
                    xsmall = ifs->FindImageByName(CWStr(IF_TURRET_SMALL1))->m_xTexPos;
                    ysmall = ifs->FindImageByName(CWStr(IF_TURRET_SMALL1))->m_yTexPos;
                }
                else if(((CMatrixCannon*)object)->m_TurretKind == TURRET_HEAVY_CANNON)
                {
                    tex_med = ifs->FindImageByName(CWStr(IF_TURRET_MED2))->m_Image;
                    tex_small = ifs->FindImageByName(CWStr(IF_TURRET_SMALL2))->m_Image;

                    xmed = ifs->FindImageByName(CWStr(IF_TURRET_MED2))->m_xTexPos;
                    ymed = ifs->FindImageByName(CWStr(IF_TURRET_MED2))->m_yTexPos;
                    xsmall = ifs->FindImageByName(CWStr(IF_TURRET_SMALL2))->m_xTexPos;
                    ysmall = ifs->FindImageByName(CWStr(IF_TURRET_SMALL2))->m_yTexPos;
                }
                else if(((CMatrixCannon*)object)->m_TurretKind == TURRET_LASER_CANNON)
                {
                    tex_med = ifs->FindImageByName(CWStr(IF_TURRET_MED3))->m_Image;
                    tex_small = ifs->FindImageByName(CWStr(IF_TURRET_SMALL3))->m_Image;

                    xmed = ifs->FindImageByName(CWStr(IF_TURRET_MED3))->m_xTexPos;
                    ymed = ifs->FindImageByName(CWStr(IF_TURRET_MED3))->m_yTexPos;
                    xsmall = ifs->FindImageByName(CWStr(IF_TURRET_SMALL3))->m_xTexPos;
                    ysmall = ifs->FindImageByName(CWStr(IF_TURRET_SMALL3))->m_yTexPos;
                }
                else if(((CMatrixCannon*)object)->m_TurretKind == TURRET_MISSILE_CANNON)
                {
                    tex_med = ifs->FindImageByName(CWStr(IF_TURRET_MED4))->m_Image;
                    tex_small = ifs->FindImageByName(CWStr(IF_TURRET_SMALL4))->m_Image;

                    xmed = ifs->FindImageByName(CWStr(IF_TURRET_MED4))->m_xTexPos;
                    ymed = ifs->FindImageByName(CWStr(IF_TURRET_MED4))->m_yTexPos;
                    xsmall = ifs->FindImageByName(CWStr(IF_TURRET_SMALL4))->m_xTexPos;
                    ysmall = ifs->FindImageByName(CWStr(IF_TURRET_SMALL4))->m_yTexPos;
                }
                turret = true;
            }
    
            CIFaceImage* image = nullptr;
            if(num == 1)
            {
                if(tex_med)
                {
                    image = HNew(g_MatrixHeap) CIFaceImage;
                    image->m_Image = tex_med;
                    image->m_Height = 42;
                    image->m_Width = 42;
                    image->m_NextImage = nullptr;
                    image->m_PrevImage = nullptr;
                    image->m_strName = L"";
                    image->m_Type = IFACE_IMAGE;

                    if(flyer || turret)
                    {
                        image->m_TexHeight = 512;
                        image->m_TexWidth = 512;
                        image->m_xTexPos = xmed;
                        image->m_yTexPos = ymed;
                        s = ifs->CreateStaticFromImage(232, 55, 0, *image, false);
                    }
                    else if(robot)
                    {
                        image->m_xTexPos = 0;
                        image->m_yTexPos = 0;
                        image->m_TexHeight = 64;
                        image->m_TexWidth = 64;
                        s = ifs->CreateStaticFromImage(232, 55, 0, *image, true);
                    }
                    //else if(turret) {}

                    if(s)
                    {
                        s->m_nId = QUEUE_ICON + (num - 1);
                    }

                }
            }
            else
            {
                if(tex_small)
                {
                    image = HNew(g_MatrixHeap) CIFaceImage;
                    image->m_Image = tex_small;
                    image->m_Height = 25;
                    image->m_Width = 25;
                    image->m_NextImage = nullptr;
                    image->m_PrevImage = nullptr;
                    image->m_strName = L"";
                    image->m_Type = IFACE_IMAGE;

                    if(flyer || turret)
                    {
                        image->m_TexHeight = 512;
                        image->m_TexWidth = 512;
                        image->m_xTexPos = xsmall;
                        image->m_yTexPos = ysmall;
                        s = ifs->CreateStaticFromImage(225+(((float)num-2)*31), 105, 0, *image, false);
                    }
                    else if(robot)
                    {
                        image->m_TexHeight = 32;
                        image->m_TexWidth = 32;
                        image->m_xTexPos = 0;
                        image->m_yTexPos = 0;
                        s = ifs->CreateStaticFromImage(225+(((float)num-2)*31), 105, 0, *image, true);
                    }/*else if(turret){
                    }*/

                    if(s)
                    {
                        s->m_nId = QUEUE_ICON+(num-1);
                    }
                }
            }
            if(s)
            {
                s->m_iParam = int(base);
                s->SetVisibility(false);
            }
            
            if(image)
            {
                HDelete(CIFaceImage, image, g_MatrixHeap);
            }

            return;
        }
        ifs = ifs->m_NextInterface;
    }
}


void CIFaceList::DeleteQueueIcon(int num, CMatrixBuilding* base)
{
DTRACE();
    
    if(!base) return;

    CInterface* ifs = m_First;
    while(ifs)
    {
        if(ifs->m_strName == IF_MAIN)
        {

            CIFaceElement* els = ifs->m_FirstElement;
            while(els)
            {
                if(IS_QUEUE_ICON(els->m_nId) && els->m_iParam == (int)base)
                {
                    if(num == 1 && els->m_nId == QUEUE_ICON + 1)
                    {
                        els = ifs->DelElement(els);
                        continue;
                    }
                    else if(num == 1 && els->m_nId == QUEUE_ICON)
                    {
                        els = ifs->DelElement(els);
                        continue;
                    }
                    else if(els->m_nId-QUEUE_ICON > (num - 1))
                    {
                        --els->m_nId;
                        els->RecalcPos(els->m_xPos - 31, els->m_yPos, false);
                    }
                    else if(els->m_nId-QUEUE_ICON == (num-1))
                    {
                        els = ifs->DelElement(els);
                        continue;
                    }
                }
                els = els->m_NextElement;
            }   

            if(num == 1 && base->m_BS.GetItemsCnt() > 1 && base->m_BS.GetTopItem()->m_NextQueueItem)
            {
                CreateQueueIcon(1, base, base->m_BS.GetTopItem()->m_NextQueueItem);
            }
            return;
        }
        ifs = ifs->m_NextInterface;
    }
}

//Заносим базовый (убогий) шаблон робота в конструктор
void CIFaceList::ConstructorButtonsInit()
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide(); //Если включён режим автобоя, например
    if(!ps) return;

    ps->m_Constructor->ReplaceRobotModule(MRT_CHASSIS, RUK_CHASSIS_PNEUMATIC, 0);
    ps->m_Constructor->ReplaceRobotModule(MRT_HULL, RUK_HULL_MONOSTACK, 0);
    ps->m_Constructor->ReplaceRobotModule(MRT_HEAD, RUK_EMPTY, 0);
    WeaponPilonsInit();
    if(g_Config.m_RobotHullsConsts[RUK_HULL_MONOSTACK].weapon_pylon_data.size()) //Если у корпуса в принципе предусмотрены пушки
    {
        //Выбираем первое подходящее для первого слота оружие
        int i = RUK_EMPTY;
        for(int i = 1; i <= g_Config.m_RobotWeaponsConsts[0].for_player_side; ++i)
        {
            if(g_Config.m_RobotHullsConsts[RUK_HULL_MONOSTACK].weapon_pylon_data[0].fit_weapon.test(i)) break;
        }
        ps->m_Constructor->ReplaceRobotModule(MRT_WEAPON, (ERobotModuleKind)i, 0, 0);
    }
    m_RCountControl->ManageButtons();
    GetConstructionName(ps->m_Constructor->GetRenderBot());
}

void CIFaceList::WeaponPilonsInit()
{
    for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT; ++i)
    {
        CInterface::CopyElements(m_Weapon[0], m_WeaponPilon[i]);
    }
}

void CIFaceList::CreateElementRamka(CIFaceElement* element, dword color)
{
    if(!element) return;

    for(int i = 0; i < MAX_STATES; ++i)
    {
        if(element->m_StateImages[i].pImage)
        {
            element->m_StateImages[i].pImage->Ramka(Float2Int(element->m_StateImages[i].xTexPos), Float2Int(element->m_StateImages[i].yTexPos), Float2Int(element->m_xSize), Float2Int(element->m_ySize), color);
        }
    }
}

void CIFaceList::CreateHintButton(int x, int y, EHintButton type, DialogButtonHandler handler)
{
    EnableMainMenuButton(type);
    void* fn = nullptr, *cl = nullptr;
    CIFaceElement* els = m_Hints->m_FirstElement;
    while(els)
    {
        if(type == HINT_OK && els->m_strName == IF_HINTS_OK)
        {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if(type == HINT_CANCEL && els->m_strName == IF_HINTS_CANCEL)
        {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if(type == HINT_CANCEL_MENU && els->m_strName == IF_HINTS_CANCEL_MENU)
        {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if(type == HINT_CONTINUE && els->m_strName == IF_HINTS_CONTINUE)
        {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if(type == HINT_SURRENDER && els->m_strName == IF_HINTS_SURRENDER)
        {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if(type == HINT_EXIT && els->m_strName == IF_HINTS_EXIT)
        {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if(type == HINT_RESET && els->m_strName == IF_HINTS_RESET)
        {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if(type == HINT_HELP && els->m_strName == IF_HINTS_HELP)
        {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::HideHintButtons()
{
    CIFaceElement* els = m_Hints->m_FirstElement;
    while(els)
    {
        els->SetVisibility(false);
        els = els->m_NextElement;
    }
}

void CIFaceList::HideHintButton(EHintButton butt)
{
    CIFaceElement* els = m_Hints->m_FirstElement;

    CWStr sname(g_CacheHeap);
    HintButtonId2Name(butt, sname);

    while(els)
    {
        if(els->m_strName == sname)
        {
            els->SetVisibility(false);
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::DisableMainMenuButton(EHintButton butt)
{
    CWStr sname;
    HintButtonId2Name(butt, sname);

    CIFaceElement* els = m_Hints->m_FirstElement;
    while(els)
    {
        if(els->m_strName == sname)
        {
            els->SetState(IFACE_DISABLED);
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::EnableMainMenuButton(EHintButton butt)
{

    CWStr sname;
    HintButtonId2Name(butt, sname);

    CIFaceElement* els = m_Hints->m_FirstElement;
    while(els)
    {
        if(els->m_strName == sname)
        {
            if(els->GetState() == IFACE_DISABLED) els->SetState(IFACE_NORMAL);
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::PressHintButton(EHintButton butt)
{
    CWStr sname;
    HintButtonId2Name(butt, sname);

    CIFaceElement* els = m_Hints->m_FirstElement;
    while(els)
    {
        if(els->m_strName == sname)
        {
            els->Action(ON_UN_PRESS);
            break;
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::HintButtonId2Name(EHintButton butt, CWStr &sname)
{
    switch(butt)
    {
        case HINT_OK:
            sname = IF_HINTS_OK;
            break;
        case HINT_CANCEL:
            sname = IF_HINTS_CANCEL;
            break;
        case HINT_CANCEL_MENU:
            sname = IF_HINTS_CANCEL_MENU;
            break;
        case HINT_CONTINUE:
            sname = IF_HINTS_CONTINUE;
            break;
        case HINT_SURRENDER:
            sname = IF_HINTS_SURRENDER;
            break;
        case HINT_EXIT:
            sname = IF_HINTS_EXIT;
            break;
        case HINT_RESET:
            sname = IF_HINTS_RESET;
            break;
        case HINT_HELP:
            sname = IF_HINTS_HELP;
            break;
    }
}


bool CIFaceList::CorrectCoordinates(
    int screen_width,
    int screen_height,
    int& posx,
    int& posy,
    int width,
    int height,
    const CWStr& element_name
)
{
    if(
        element_name == IF_BUILD_ROBOT ||
        element_name == IF_BUILD_TURRET ||
        element_name == IF_BUILD_FLYER ||
        element_name == IF_ORDER_STOP ||
        element_name == IF_ORDER_MOVE ||
        element_name == IF_ORDER_PATROL ||
        element_name == IF_ORDER_FIRE ||
        element_name == IF_ORDER_CAPTURE ||
        element_name == IF_ORDER_CANCEL ||
        element_name == IF_ORDER_REPAIR ||
        element_name == IF_ORDER_BOMB ||
        element_name == IF_ORDER_BOMB_AUTO ||
        element_name == IF_EXIT_ARCADE_MODE ||
        element_name == IF_ENTER_ARCADE_MODE ||
        element_name == IF_AORDER_CAPTURE_OFF ||
        element_name == IF_AORDER_CAPTURE_ON ||
        element_name == IF_AORDER_PROTECT_OFF ||
        element_name == IF_AORDER_PROTECT_ON ||
        element_name == IF_AORDER_ATTACK_OFF ||
        element_name == IF_AORDER_ATTACK_ON ||
        element_name == IF_BUILD_TUR1 ||
        element_name == IF_BUILD_TUR2 ||
        element_name == IF_BUILD_TUR3 ||
        element_name == IF_BUILD_TUR4 ||
        element_name == IF_BUILD_FLYER_1 ||
        element_name == IF_BUILD_FLYER_2 ||
        element_name == IF_BUILD_FLYER_3 ||
        element_name == IF_BUILD_FLYER_4 ||
        element_name == IF_MAIN_SELF_BOMB ||
        element_name == IF_MAIN_SELF_BOMB_AUTO ||
        element_name == IF_CALL_FROM_HELL
    )
    {
        int needx = 0;
        if(element_name == IF_EXIT_ARCADE_MODE || element_name == IF_MAIN_SELF_BOMB || element_name == IF_MAIN_SELF_BOMB_AUTO)
        {
            needx = Float2Int(m_MainX) + 354 - width;
        }
        else
        {
            needx = Float2Int(m_MainX) + 554 - width;
        }

        int needy = Float2Int(m_MainY) + 28 - height;

        if(posx > needx) posx -= (posx - needx);
        if(posx < needx) posx += (needx - posx);
        if(posy > needy) posy -= (posy - needy);
        if(posy < needy) posy += (needy - posy);
        
    }
    else if(element_name == IF_BASE_HISTORY_RIGHT || element_name == IF_BASE_HISTORY_LEFT || element_name == IF_BASE_COUNTHZ)
    {
        posx -= Float2Int(width / 2.0f);
        posy -= height;
    }

    bool corx = false;
    if(posx + width + HINT_OTSTUP > screen_width)
    {
        corx = true;
        int val = posx + width + HINT_OTSTUP - screen_width;
        posx -= val;
    }
    
    if(corx == false)
    {
        if(posx < 0) posx = HINT_OTSTUP;
    }
    //Это значит, что hint нельзя показывать, т.к. он вылазит со всех щелей
    else return false;

    bool cory = false;
    if(posy + height + HINT_OTSTUP > screen_height)
    {
        cory = true;
        int val = posy + height + HINT_OTSTUP - screen_height;
        posy -= val;
    }
    
    if(cory == false)
    {
        if(posy < 0) posy = HINT_OTSTUP;
    }
    //Это значит, что hint нельзя показывать, т.к. он вылазит со всех щелей
    else return false;

    return true;
}

void CIFaceList::AddHintReplacements(const CWStr& element_name)
{
    CBlockPar* repl = g_MatrixData->BlockGet(PAR_REPLACE);
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    int turret_kind = 0;
    if(element_name == L"thz")
    {
        int base_i, fa_i;
        ps->GetResourceIncome(base_i, fa_i, TITAN);
        repl->ParSetAdd(L"_titan_income", CWStr(base_i + fa_i, g_CacheHeap));
    }
    else if(element_name == L"enhz1" || element_name == L"enhz2")
    {
        int base_i, fa_i;
        ps->GetResourceIncome(base_i, fa_i, ENERGY);
        repl->ParSetAdd(L"_energy_income", CWStr(base_i + fa_i, g_CacheHeap));
    }
    else if(element_name == L"elhz")
    {
        int base_i, fa_i;
        ps->GetResourceIncome(base_i, fa_i, ELECTRONICS);
        repl->ParSetAdd(L"_electronics_income", CWStr(base_i + fa_i, g_CacheHeap));
    }
    else if(element_name == L"phz")
    {
        int base_i, fa_i;
        ps->GetResourceIncome(base_i, fa_i, PLASMA);
        repl->ParSetAdd(L"_plasma_income", CWStr(base_i + fa_i, g_CacheHeap));
    }
    else if(element_name == L"rvhz")
    {
        repl->ParSetAdd(L"_total_robots", CWStr(ps->GetRobotsCnt(), g_CacheHeap));
        repl->ParSetAdd(L"_max_robots", CWStr(ps->GetMaxSideRobots(), g_CacheHeap));
    }

    else if(element_name == IF_ORDER_STOP) //Кнопка приказа "Остановиться"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_ORDER_STOP));
    }
    else if(element_name == IF_ORDER_MOVE) //Кнопка приказа "Двигаться в точку"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_ORDER_MOVE));
    }
    else if(element_name == IF_ORDER_PATROL) //Кнопка приказа "Патрулировать"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_ORDER_PATROL));
    }
    else if(element_name == IF_ORDER_FIRE) //Кнопка приказа "Атаковать цель"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_ORDER_ATTACK));
    }
    else if(element_name == IF_ORDER_CAPTURE) //Кнопка приказа "Захватить цель"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_ORDER_CAPTURE));
    }
    else if(element_name == IF_ORDER_REPAIR) //Кнопка приказа "Починить цель"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_ORDER_REPAIR));
    }
    else if(element_name == IF_ORDER_BOMB || element_name == IF_ORDER_BOMB_AUTO) //Кнопка приказа "Взорвать бомбу"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_ORDER_EXPLODE));
    }
    else if(element_name == IF_MAIN_SELF_BOMB || element_name == IF_MAIN_SELF_BOMB_AUTO) //Кнопка приказа "Взорвать бомбу" из ручного управления
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_UNIT_BOOM));
    }
    else if(element_name == IF_AORDER_ATTACK_ON || element_name == IF_AORDER_ATTACK_OFF) //Кнопка приказа "Программа атаки"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_AUTOORDER_ATTACK));
    }
    else if(element_name == IF_AORDER_CAPTURE_ON || element_name == IF_AORDER_CAPTURE_OFF) //Кнопка приказа "Программа захвата"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_AUTOORDER_CAPTURE));
    }
    else if(element_name == IF_AORDER_PROTECT_ON || element_name == IF_AORDER_PROTECT_OFF) //Кнопка приказа "Программа защиты"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_AUTOORDER_DEFEND));
    }

    else if(element_name == IF_BUILD_ROBOT) //Кнопка "Конструктор роботов"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_BUILD_ROBOT));
    }
    /*
    else if(element_name == IF_BUILD_FLYER) //Кнопка "Вертолёты"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_BUILD_FLYER));
    }
    */
    else if(element_name == IF_BUILD_TURRET) //Кнопка приказа "Защитные турели"
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_BUILD_TURRET));
        repl->ParSetAdd(L"_build_turret", g_Config.m_TurretsConsts[0].name);
    }
    else if(element_name == IF_CALL_FROM_HELL) //Кнопка вызова подкрепления
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_CALL_REINFORCEMENTS));

        repl->ParSetAdd(L"_ch_cant", L"");
        repl->ParSetAdd(L"_ch_can", L"");
        repl->ParSetAdd(L"_ch_time_min", L"");
        repl->ParSetAdd(L"_ch_time_sec", L"");

        if(g_MatrixMap->ReinforcementsDisabled()) repl->ParSetAdd(L"_ch_cant", L"1");
        else
        {
            if(g_MatrixMap->BeforeReinforcementsTime())
            {
                int milliseconds = g_MatrixMap->BeforeReinforcementsTime();
                int minutes = milliseconds / 60000;
                int seconds = milliseconds / 1000 - minutes * 60;
                repl->ParSetAdd(L"_ch_time_min", CWStr((minutes > 0 ? minutes : 0)));
                repl->ParSetAdd(L"_ch_time_sec", CWStr((seconds > 0 ? seconds : 0)));
            }
            else repl->ParSetAdd(L"_ch_can", L"1");
        }
    }
    else if(element_name == IF_ORDER_CANCEL) //Кнопка выхода из текущего меню выбора приказа/постройки
    {
        repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_ORDER_CANCEL));
    }

    else if(element_name == IF_BUILD_TUR1) turret_kind = TURRET_LIGHT_CANNON;
    else if(element_name == IF_BUILD_TUR2) turret_kind = TURRET_HEAVY_CANNON;
    else if(element_name == IF_BUILD_TUR3) turret_kind = TURRET_LASER_CANNON;
    else if(element_name == IF_BUILD_TUR4) turret_kind = TURRET_MISSILE_CANNON;

    if(turret_kind)
    {
        repl->ParSetAdd(L"_turret_name", g_Config.m_TurretsConsts[turret_kind].name);
        switch(turret_kind) //Название хоткия для постройки турели
        {
            case TURRET_LIGHT_CANNON: repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_TURRET_CANNON)); break;
            case TURRET_HEAVY_CANNON: repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_TURRET_GUN)); break;
            case TURRET_LASER_CANNON: repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_TURRET_LASER)); break;
            case TURRET_MISSILE_CANNON: repl->ParSetAdd(L"_key_name", g_Config.KeyActionCode2KeyName(KA_TURRET_ROCKET)); break;
        }
        repl->ParSetAdd(L"_turret_params", g_Config.m_TurretsConsts[turret_kind].chars_description);

        //Строка с необходимыми ресурсами
        if(g_Config.m_TurretsConsts[turret_kind].cost_titan) repl->ParSetAdd(L"_turret_res1", CWStr(g_Config.m_TurretsConsts[turret_kind].cost_titan, g_CacheHeap));
        else repl->ParSetAdd(L"_turret_res1", L"");
        if(g_Config.m_TurretsConsts[turret_kind].cost_electronics) repl->ParSetAdd(L"_turret_res2", CWStr(g_Config.m_TurretsConsts[turret_kind].cost_electronics, g_CacheHeap));
        else repl->ParSetAdd(L"_turret_res2", L"");
        if(g_Config.m_TurretsConsts[turret_kind].cost_energy) repl->ParSetAdd(L"_turret_res3", CWStr(g_Config.m_TurretsConsts[turret_kind].cost_energy, g_CacheHeap));
        else repl->ParSetAdd(L"_turret_res3", L"");
        if(g_Config.m_TurretsConsts[turret_kind].cost_plasma) repl->ParSetAdd(L"_turret_res4", CWStr(g_Config.m_TurretsConsts[turret_kind].cost_plasma, g_CacheHeap));
        else repl->ParSetAdd(L"_turret_res4", L"");
    }
}

bool CIFaceList::CheckShowHintLogic(const CWStr& element_name)
{
    if(element_name == IF_BASE_COUNTHZ)
    {
        if(FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) return false;
    }

    return true;
}

void __stdcall CIFaceList::JumpToBuilding(void* o)
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    if(ps->m_CurrSel == BUILDING_SELECTED || ps->m_CurrSel == BASE_SELECTED && ps->m_ActiveObject && ps->m_ActiveObject->IsBuilding())
    {
        D3DXVECTOR2 tgt(ps->m_ActiveObject->GetGeoCenter().x, ps->m_ActiveObject->GetGeoCenter().y);
        g_MatrixMap->m_Camera.SetXYStrategy(tgt);
    }
}

void __stdcall CIFaceList::JumpToRobot(void *o)
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    if(!ps->GetCurGroup()) return;

    D3DXVECTOR3 vec=ps->GetCurGroup()->GetObjectByN(ps->GetCurSelNum())->GetGeoCenter();
    g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(vec.x, vec.y));
}

void CIFaceList::CreateDynamicTurrets(CMatrixBuilding* building)
{
    DeleteDynamicTurrets();
    if(!building) return;

    int tur_sheme = building->m_TurretsMax;

    CInterface* interfaces = m_First;

    CIFaceImage* image = nullptr/*HNew(g_MatrixHeap) CIFaceImage*/;
    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            for(int i = 0; i < tur_sheme; ++i)
            {
                image = nullptr;
                if(building->m_TurretsPlaces[i].m_CannonType == 1)
                {
                    image = interfaces->FindImageByName(CWStr(IF_BT1_ICON, g_CacheHeap));
                }
                else if(building->m_TurretsPlaces[i].m_CannonType == 2)
                {
                    image = interfaces->FindImageByName(CWStr(IF_BT2_ICON, g_CacheHeap));
                }
                else if(building->m_TurretsPlaces[i].m_CannonType == 3)
                {
                    image = interfaces->FindImageByName(CWStr(IF_BT3_ICON, g_CacheHeap));
                }
                else if(building->m_TurretsPlaces[i].m_CannonType == 4)
                {
                    image = interfaces->FindImageByName(CWStr(IF_BT4_ICON, g_CacheHeap));
                }

                if(image)
                {
                    float x = 0;
                    if(tur_sheme == 1)
                    {
                        x = (float)g_IFaceList->m_DynamicTX[i];
                    }
                    else if(tur_sheme == 2)
                    {
                        x = (float)g_IFaceList->m_DynamicTX[1 + i];
                    }
                    else if(tur_sheme == 3)
                    {
                        x = (float)g_IFaceList->m_DynamicTX[3 + i];
                    }
                    else if(tur_sheme == 4)
                    {
                        x = (float)g_IFaceList->m_DynamicTX[6 + i];
                    }

                    CIFaceStatic* s = interfaces->CreateStaticFromImage(x, (float)g_IFaceList->m_DynamicTY, 0.000001f, *image);
                    
                    s->SetVisibility(true);
                    s->m_nId = DYNAMIC_TURRET;
                }
            }

            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeleteDynamicTurrets()
{
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();

//icon

    CInterface* interfaces = m_First;
    while(interfaces)
    {
        if(interfaces->m_strName == IF_MAIN)
        {
            CIFaceElement* elements = interfaces->m_FirstElement;
            while(elements)
            {
                if(elements->m_Type == IFACE_DYNAMIC_STATIC && elements->m_nId == DYNAMIC_TURRET)
                {
                    elements = interfaces->DelElement(elements);
                    continue;
                }

                elements = elements->m_NextElement;
            }

            interfaces->SortElementsByZ();
            break;
        }

        interfaces = interfaces->m_NextInterface;
    }

}

void CIFaceList::BeginBuildTurret(int no)
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    if(!ps->IsEnoughResourcesForTurret(&g_Config.m_TurretsConsts[no])) return;

    ps->m_CannonForBuild.Delete();
    CMatrixCannon* cannon = HNew(g_MatrixHeap) CMatrixCannon;
    cannon->m_Pos.x = g_MatrixMap->m_TraceStopPos.x;
    cannon->m_Pos.y = g_MatrixMap->m_TraceStopPos.y;
    cannon->SetSide(PLAYER_SIDE);
    cannon->UnitInit(no);
    cannon->m_Angle = 0;

    cannon->m_ShadowType = SHADOW_OFF;
    cannon->m_ShadowSize = 128;

    cannon->GetResources(MR_Matrix | MR_Graph);

    ps->m_CannonForBuild.m_Cannon = cannon;
    ps->m_CannonForBuild.m_ParentBuilding = (CMatrixBuilding*)ps->m_ActiveObject;
    ps->m_CurrentAction = BUILDING_TURRET;
    g_MatrixMap->m_Cursor.SetPos(g_MatrixMap->m_Cursor.GetPosX(), (int)GetMainY() - 40);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SStateImages::SetStateLabelParams(
    int x,       int y,
    int bound_x, int bound_y,
    int xAlign,  int yAlign,
    int perenos,
    int smeX,    int smeY,
    CRect clipRect,
    CWStr t,
    CWStr font,
    dword color
)
{
    m_Caption = t;
    m_x = x;
    m_y = y;
    m_boundX = bound_x;
    m_boundY = bound_y;
    m_xAlign = xAlign;
    m_yAlign = yAlign;
    m_Perenos = perenos;
    m_SmeX = smeX;
    m_SmeY = smeY;
    m_ClipRect = clipRect;
    m_Font = font;
    m_Color = color;
}

void SStateImages::SetStateText(bool copy)
{
    CTextureManaged* texture = pImage;
    D3DLOCKED_RECT lr;

    if(g_RangersInterface)
    {
        SMGDRangersInterfaceText it;
        g_RangersInterface->m_RangersText(
            (wchar*)m_Caption.Get(),
            (wchar*)m_Font.Get(),
            m_Color,
            m_boundX, m_boundY,
            m_xAlign, m_yAlign,
            m_Perenos,
            m_SmeX, m_SmeY,
            &m_ClipRect,
            &it
        );

        texture->LockRect(lr, 0);

        CBitmap bmsrc(g_CacheHeap);
        bmsrc.CreateRGBA(it.m_SizeX, it.m_SizeY, it.m_Pitch, it.m_Buf);
        CBitmap bmdes(g_CacheHeap);
        bmdes.CreateRGBA(Float2Int(TexWidth), Float2Int(TexHeight), lr.Pitch, lr.pBits);
        if(copy) bmdes.Copy(CPoint(Float2Int(xTexPos) + m_x, Float2Int(yTexPos) + m_y), CPoint(m_boundX, m_boundY), bmsrc, CPoint(0, 0));
        else bmdes.MergeWithAlpha(CPoint(Float2Int(xTexPos) + m_x, Float2Int(yTexPos) + m_y), CPoint(m_boundX, m_boundY), bmsrc, CPoint(0, 0));
        g_RangersInterface->m_RangersTextClear(&it);

        texture->UnlockRect();
        texture->Unload();
    }
}

CBuf* CInterface::m_ClearRects;

#ifdef _DEBUG
void t_pause(void)    { g_MatrixMap->Pause(true); }
void t_unpause(void)    { g_MatrixMap->Pause(false); }
#endif
