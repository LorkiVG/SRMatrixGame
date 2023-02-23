// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../StdAfx.h"
#include "CIFaceButton.h"
#include "CConstructor.h"
#include "CIFaceMenu.h"
#include "CInterface.h"
#include "MatrixHint.hpp"

int gIndex[] = {
	0,1,
	1,2,
	2,0,
	3,4,
	4,5,
	5,3
};

CIFaceButton::CIFaceButton()
{
	m_Type = IFACE_PUSH_BUTTON;
	m_CurState = IFACE_NORMAL;
	//ZeroMemory(m_StateImages, sizeof(m_StateImages));
}

CIFaceButton::~CIFaceButton()
{
}

bool CIFaceButton::OnMouseMove(CPoint mouse)
{
    if(GetVisibility() && ElementCatch(mouse) && ElementAlpha(mouse))
    {
        if(g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName != m_strName)
        {
            g_IFaceList->m_CurrentHint->Release();
            g_IFaceList->m_CurrentHint = nullptr;
            g_IFaceList->m_CurrentHintControlName = L"";
        }

        if(g_IFaceList->m_CurrentHint == nullptr && m_Hint.HintTemplate != L"")
        {
            if(g_IFaceList->CheckShowHintLogic(m_strName))
            {
                g_IFaceList->AddHintReplacements(m_strName);
                CMatrixHint* hint = CMatrixHint::Build(m_Hint.HintTemplate, m_strName);
                int x = Float2Int(m_PosElInX) + m_Hint.x;
                int y = Float2Int(m_PosElInY) + m_Hint.y;
                g_IFaceList->CorrectCoordinates(g_ScreenX, g_ScreenY, x, y, hint->m_Width, hint->m_Height, m_strName);
                hint->Show(x, y);
                g_IFaceList->m_CurrentHint = hint;
                g_IFaceList->m_CurrentHintControlName = m_strName;
            }
        }

        if(m_Type == IFACE_PUSH_BUTTON)
        {
            if(GetState() == IFACE_NORMAL)
            {
                SetState(IFACE_FOCUSED);
                Action(ON_FOCUS);
                CSound::Play(S_BENTER, SL_INTERFACE);
            }
        }
        else if(m_Type == IFACE_CHECK_BUTTON || m_Type == IFACE_CHECK_BUTTON_SPECIAL)
        {
            if(GetState() == IFACE_NORMAL)
            {
                SetState(IFACE_FOCUSED);
                Action(ON_FOCUS);
                CSound::Play(S_BENTER, SL_INTERFACE);
            }
            else if(GetState() == IFACE_PRESSED_UNFOCUSED)
            {
                SetState(IFACE_PRESSED);
                Action(ON_FOCUS);
                CSound::Play(S_BENTER, SL_INTERFACE);
            }
        }
        else if(m_Type == IFACE_CHECK_PUSH_BUTTON)
        {
            if(GetState() == IFACE_NORMAL)
            {
                SetState(IFACE_FOCUSED);
                Action(ON_FOCUS);
                CSound::Play(S_BENTER, SL_INTERFACE);
            }
        }

        return true;
    }
    else
    {
        if(g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName == m_strName)
        {
            g_IFaceList->m_CurrentHint->Release();
            g_IFaceList->m_CurrentHint = nullptr;
            g_IFaceList->m_CurrentHintControlName = L"";
        }

        return false;
    }

    return false;
}

//Обработчик отжатия левой кнопки мыши, проверяет фокус элемента и вызывает действие, если возможно
//Третий обработчик левого отклика мышью (сбрасываем нажатость, запускаем действие отклика)
void CIFaceButton::OnMouseLBUp()
{
    if(g_IFaceList && g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName == m_strName)
    {
        g_IFaceList->m_CurrentHint->Release();
        g_IFaceList->m_CurrentHint = nullptr;
        g_IFaceList->m_CurrentHintControlName = L"";
    }

    if(m_Type == IFACE_PUSH_BUTTON)
    {
        if(GetState() == IFACE_PRESSED)
        {
            SetState(IFACE_FOCUSED);
            Action(ON_UN_PRESS);
        }
    }
    else if(m_Type == IFACE_CHECK_PUSH_BUTTON)
    {
        if(GetState() == IFACE_PRESSED)
        {
            if(m_DefState == IFACE_NORMAL)
            {
                SetState(IFACE_PRESSED_UNFOCUSED);
                m_DefState = IFACE_PRESSED_UNFOCUSED;
                Action(ON_UN_PRESS);
            }
            /*
            else if(m_DefState == IFACE_PRESSED_UNFOCUSED)
            {
                SetState(IFACE_FOCUSED);
                m_DefState = IFACE_NORMAL;
                Action(ON_UN_PRESS);
            }
            */
        }
    }
}

//Третий обработчик левого клика мышью (задаём нажатость, запускаем действие клика)
bool CIFaceButton::OnMouseLBDown()
{
    if(GetState() == IFACE_DISABLED)
    {
        return true;
    }
	
    if(m_Type == IFACE_PUSH_BUTTON)
    {
		if(GetState() == IFACE_FOCUSED)
        {
			SetState(IFACE_PRESSED);
			Action(ON_PRESS);
            
            if(m_strName == IF_BASE_CONST_BUILD) CSound::Play(S_BUILD_CLICK, SL_INTERFACE);
            else if(m_strName == IF_BASE_CONST_CANCEL) CSound::Play(S_CANCEL_CLICK, SL_INTERFACE);
            else CSound::Play(S_BCLICK, SL_INTERFACE);
			return true;
		}
	}
    else if(m_Type == IFACE_CHECK_BUTTON)
    {
		if(GetState() == IFACE_PRESSED)
        {
			SetState(IFACE_FOCUSED);
			m_DefState = IFACE_NORMAL;
			Action(ON_UN_PRESS);
            CSound::Play(S_BCLICK, SL_INTERFACE);
		}
        else if(GetState() == IFACE_FOCUSED)
        {
			SetState(IFACE_PRESSED);
			m_DefState = IFACE_PRESSED_UNFOCUSED;
			Action(ON_PRESS);
            CSound::Play(S_BCLICK, SL_INTERFACE);
			return true;
		}
    }
    else if(m_Type == IFACE_CHECK_BUTTON_SPECIAL)
    {
		if(GetState() == IFACE_FOCUSED)
        {
			SetState(IFACE_PRESSED);
			m_DefState = IFACE_PRESSED_UNFOCUSED;
			Action(ON_PRESS);
            CSound::Play(S_BCLICK, SL_INTERFACE);
			return true;
		}
    }
    else if(m_Type == IFACE_CHECK_PUSH_BUTTON)
    {
        if(GetState() == IFACE_FOCUSED)
        {
			SetState(IFACE_PRESSED);
			m_DefState = IFACE_NORMAL;
            Action(ON_PRESS);
            if(m_strName.Find(L"conf")) CSound::Play(S_PRESET_CLICK, SL_INTERFACE);
            else CSound::Play(S_BCLICK, SL_INTERFACE);
			return true;
        }
        else if(GetState() == IFACE_PRESSED_UNFOCUSED)
        {
            //SetState(IFACE_PRESSED);
            //m_DefState = IFACE_PRESSED_UNFOCUSED;
            //Action(ON_PRESS);
			return false;
        }
    }
	
    return false;
}

//Обработчик отжатия правой кнопки мыши, проверяет фокус элемента и вызывает действие, если возможно
//Третий обработчик правого отклика мышью (сбрасываем нажатость, запускаем действие отклика)
void CIFaceButton::OnMouseRBUp()
{
    if(m_Type == IFACE_PUSH_BUTTON)
    {
        if(GetState() == IFACE_PRESSED)
        {
            SetState(IFACE_FOCUSED);
            Action(ON_UN_PRESS_RIGHT);
        }
    }
}

//Третий обработчик правого клика мышью (задаём нажатость, запускаем действие клика)
bool CIFaceButton::OnMouseRBDown()
{
    CPoint mouse = g_MatrixMap->m_Cursor.GetPos();
    CMatrixSideUnit* player_side = g_MatrixMap->GetPlayerSide();
    int cur_hull_kind = player_side->m_ConstructPanel->m_Configs[player_side->m_ConstructPanel->m_CurrentConfig].m_Hull.m_Module.m_nKind;

    if(ElementCatch(mouse) /*&& ElementAlpha(mouse)*/)
    {
        //Пока что правый клик добавляется только в моде на автоподрыв бомбы
        if(g_PlayerRobotsAutoBoom && (m_strName == IF_ORDER_BOMB || m_strName == IF_ORDER_BOMB_AUTO || m_strName == IF_MAIN_SELF_BOMB || m_strName == IF_MAIN_SELF_BOMB_AUTO))
        {
            if(GetState() == IFACE_FOCUSED)
            {
                SetState(IFACE_PRESSED);
                Action(ON_PRESS_RIGHT);

                CSound::Play(S_BCLICK, SL_INTERFACE); //Звук простого клика по кнопке
                return true;
            }
        }
        else if(m_strName == IF_BASE_WEAPON_PYLON_1)
        {
            if(g_PopupMenu) //Если нужно заполнить меню выбора конкретного модуля из списка
            {
                //Необходимо определить реальный номер оружейного пилона на модели, т.к. их там может быть меньше максимального числа
                int pylon_num_on_model = 0;
                for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data.size(); ++i)
                {
                    if(g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].constructor_slot_num == RUK_WEAPON_PYLON_1)
                    {
                        pylon_num_on_model = i;
                        break;
                    }
                }

                int add_weapons_count = 1; //Как минимум, в списке будет присутствовать один пустой слот без пушки
                int menu_lenght = WEAPON_MENU_WIDTH;
                for(int i = 1; i <= ROBOT_WEAPONS_COUNT /*g_Config.m_RobotWeaponsConsts[0].for_player_side*/; ++i)
                {
                    if(!g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[pylon_num_on_model].fit_weapon.test(i)) continue; //Если данное оружие не подходит для первого слота
                    if(!g_Config.m_RobotWeaponsConsts[i].for_player_side) continue;

                    menu_lenght = max(g_PopupWeapon[i].text.GetLen() * 7 - 3, menu_lenght);
                    if(!player_side->m_ConstructPanel->IsEnoughResourcesForThisModule(RUK_WEAPON_PYLON_1, MRT_WEAPON, ERobotModuleKind(i)))
                    {
                        g_PopupWeapon[i].color = NOT_ENOUGH_RES_LABELS_COLOR;
                    }
                    else
                    {
                        g_PopupWeapon[i].color = DEFAULT_LABELS_COLOR;
                    }

                    ++add_weapons_count;
                }

                g_PopupMenu->CreateMenu(
                    MENU_PARENT_PILON_1,
                    add_weapons_count,
                    menu_lenght,
                    g_IFaceList->m_BaseX + 242,
                    g_IFaceList->m_BaseY + 155,
                    this,
                    g_PopupWeapon,
                    cur_hull_kind,
                    pylon_num_on_model
                );
            }
        }
        else if(m_strName == IF_BASE_WEAPON_PYLON_2)
        {
            if(g_PopupMenu) //Если нужно заполнить меню выбора конкретного модуля из списка
            {
                //Необходимо определить реальный номер оружейного пилона на модели, т.к. их там может быть меньше максимального числа
                int pylon_num_on_model = 0;
                for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data.size(); ++i)
                {
                    if(g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].constructor_slot_num == RUK_WEAPON_PYLON_2)
                    {
                        pylon_num_on_model = i;
                        break;
                    }
                }

                int add_weapons_count = 1; //Как минимум, в списке будет присутствовать один пустой слот без пушки
                int menu_lenght = WEAPON_MENU_WIDTH;
                for(int i = 1; i <= ROBOT_WEAPONS_COUNT /*g_Config.m_RobotWeaponsConsts[0].for_player_side*/; ++i)
                {
                    if(!g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[pylon_num_on_model].fit_weapon.test(i)) continue; //Если данное оружие не подходит для второго слота
                    if(!g_Config.m_RobotWeaponsConsts[i].for_player_side) continue;

                    menu_lenght = max(g_PopupWeapon[i].text.GetLen() * 7 - 3, menu_lenght);
                    if(!player_side->m_ConstructPanel->IsEnoughResourcesForThisModule(RUK_WEAPON_PYLON_2, MRT_WEAPON, ERobotModuleKind(i)))
                    {
                        g_PopupWeapon[i].color = NOT_ENOUGH_RES_LABELS_COLOR;
                    }
                    else
                    {
                        g_PopupWeapon[i].color = DEFAULT_LABELS_COLOR;
                    }

                    ++add_weapons_count;
                }

                g_PopupMenu->CreateMenu(
                    MENU_PARENT_PILON_2,
                    add_weapons_count,
                    menu_lenght,
                    g_IFaceList->m_BaseX + 389,
                    g_IFaceList->m_BaseY + 155,
                    this,
                    g_PopupWeapon,
                    cur_hull_kind,
                    pylon_num_on_model
                );
            }
        }
        else if(m_strName == IF_BASE_WEAPON_PYLON_3)
        {
            if(g_PopupMenu) //Если нужно заполнить меню выбора конкретного модуля из списка
            {
                //Необходимо определить реальный номер оружейного пилона на модели, т.к. их там может быть меньше максимального числа
                int pylon_num_on_model = 0;
                for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data.size(); ++i)
                {
                    if(g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].constructor_slot_num == RUK_WEAPON_PYLON_3)
                    {
                        pylon_num_on_model = i;
                        break;
                    }
                }

                int add_weapons_count = 1; //Как минимум, в списке будет присутствовать один пустой слот без пушки
                int menu_lenght = WEAPON_MENU_WIDTH;
                for(int i = 1; i <= ROBOT_WEAPONS_COUNT /*g_Config.m_RobotWeaponsConsts[0].for_player_side*/; ++i)
                {
                    if(!g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[pylon_num_on_model].fit_weapon.test(i)) continue; //Если данное оружие не подходит для третьего слота
                    if(!g_Config.m_RobotWeaponsConsts[i].for_player_side) continue;

                    menu_lenght = max(g_PopupWeapon[i].text.GetLen() * 7 - 3, menu_lenght);
                    if(!player_side->m_ConstructPanel->IsEnoughResourcesForThisModule(RUK_WEAPON_PYLON_3, MRT_WEAPON, ERobotModuleKind(i)))
                    {
                        g_PopupWeapon[i].color = NOT_ENOUGH_RES_LABELS_COLOR;
                    }
                    else
                    {
                        g_PopupWeapon[i].color = DEFAULT_LABELS_COLOR;
                    }

                    ++add_weapons_count;
                }

                g_PopupMenu->CreateMenu(
                    MENU_PARENT_PILON_3,
                    add_weapons_count,
                    menu_lenght,
                    g_IFaceList->m_BaseX + 242,
                    g_IFaceList->m_BaseY + 135,
                    this,
                    g_PopupWeapon,
                    cur_hull_kind,
                    pylon_num_on_model
                );
            }
        }
        else if(m_strName == IF_BASE_WEAPON_PYLON_4)
        {
            if(g_PopupMenu) //Если нужно заполнить меню выбора конкретного модуля из списка
            {
                //Необходимо определить реальный номер оружейного пилона на модели, т.к. их там может быть меньше максимального числа
                int pylon_num_on_model = 0;
                for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data.size(); ++i)
                {
                    if(g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].constructor_slot_num == RUK_WEAPON_PYLON_4)
                    {
                        pylon_num_on_model = i;
                        break;
                    }
                }

                int add_weapons_count = 1; //Как минимум, в списке будет присутствовать один пустой слот без пушки
                int menu_lenght = WEAPON_MENU_WIDTH;
                for(int i = 1; i <= ROBOT_WEAPONS_COUNT /*g_Config.m_RobotWeaponsConsts[0].for_player_side*/; ++i)
                {
                    if(!g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[pylon_num_on_model].fit_weapon.test(i)) continue; //Если данное оружие не подходит для четвёртого слота
                    if(!g_Config.m_RobotWeaponsConsts[i].for_player_side) continue;

                    menu_lenght = max(g_PopupWeapon[i].text.GetLen() * 7 - 3, menu_lenght);
                    if(!player_side->m_ConstructPanel->IsEnoughResourcesForThisModule(RUK_WEAPON_PYLON_4, MRT_WEAPON, ERobotModuleKind(i)))
                    {
                        g_PopupWeapon[i].color = NOT_ENOUGH_RES_LABELS_COLOR;
                    }
                    else
                    {
                        g_PopupWeapon[i].color = DEFAULT_LABELS_COLOR;
                    }

                    ++add_weapons_count;
                }

                g_PopupMenu->CreateMenu(
                    MENU_PARENT_PILON_4,
                    add_weapons_count,
                    menu_lenght,
                    g_IFaceList->m_BaseX + 389,
                    g_IFaceList->m_BaseY + 135,
                    this,
                    g_PopupWeapon,
                    cur_hull_kind,
                    pylon_num_on_model
                );
            }
        }
        else if(m_strName == IF_BASE_WEAPON_PYLON_5)
        {
            if(g_PopupMenu) //Если нужно заполнить меню выбора конкретного модуля из списка
            {
                //Необходимо определить реальный номер оружейного пилона на модели, т.к. их там может быть меньше максимального числа
                int pylon_num_on_model = 0;
                for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data.size(); ++i)
                {
                    if(g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].constructor_slot_num == RUK_WEAPON_PYLON_5)
                    {
                        pylon_num_on_model = i;
                        break;
                    }
                }

                int add_weapons_count = 1; //Как минимум, в списке будет присутствовать один пустой слот без пушки
                int menu_lenght = WEAPON_MENU_WIDTH;
                for(int i = 1; i <= ROBOT_WEAPONS_COUNT /*g_Config.m_RobotWeaponsConsts[0].for_player_side*/; ++i)
                {
                    if(!g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[pylon_num_on_model].fit_weapon.test(i)) continue; //Если данное оружие не подходит для пятого слота
                    if(!g_Config.m_RobotWeaponsConsts[i].for_player_side) continue;

                    menu_lenght = max(g_PopupWeapon[i].text.GetLen() * 7 - 3, menu_lenght);
                    if(!player_side->m_ConstructPanel->IsEnoughResourcesForThisModule(RUK_WEAPON_PYLON_5, MRT_WEAPON, ERobotModuleKind(i)))
                    {
                        g_PopupWeapon[i].color = NOT_ENOUGH_RES_LABELS_COLOR;
                    }
                    else
                    {
                        g_PopupWeapon[i].color = DEFAULT_LABELS_COLOR;
                    }

                    ++add_weapons_count;
                }

                g_PopupMenu->CreateMenu(
                    MENU_PARENT_PILON_5,
                    add_weapons_count,
                    menu_lenght,
                    g_IFaceList->m_BaseX + 389,
                    g_IFaceList->m_BaseY + 76,
                    this,
                    g_PopupWeapon,
                    cur_hull_kind,
                    pylon_num_on_model
                );
            }
        }
        else if(m_strName == IF_BASE_HULL_PYLON)
        {
            if(g_PopupMenu) //Если нужно заполнить меню выбора конкретного модуля из списка
            {
                int menu_lenght = HULL_MENU_WIDTH;
                for(int i = 1; i <= ROBOT_HULLS_COUNT /*g_Config.m_RobotHullsConsts[0].for_player_side*/; ++i)
                {
                    if(!g_Config.m_RobotHullsConsts[i].for_player_side) continue;

                    menu_lenght = max(g_PopupHull[i - 1].text.GetLen() * 7 - 3, menu_lenght);
                    if(!player_side->m_ConstructPanel->IsEnoughResourcesForThisModule(1, MRT_HULL, ERobotModuleKind(i)))
                    {
                        g_PopupHull[i - 1].color = NOT_ENOUGH_RES_LABELS_COLOR;
                    }
                    else
                    {
                        g_PopupHull[i - 1].color = DEFAULT_LABELS_COLOR;
                    }
                }

                g_PopupMenu->CreateMenu(
                    MENU_PARENT_HULL,
                    g_Config.m_RobotHullsConsts[0].for_player_side,
                    menu_lenght,
                    g_IFaceList->m_BaseX + 321,
                    g_IFaceList->m_BaseY + 148,
                    this,
                    g_PopupHull
                );
            }
        }
        else if(m_strName == IF_BASE_CHASSIS_PYLON)
        {
            if(g_PopupMenu) //Если нужно заполнить меню выбора конкретного модуля из списка
            {
                int menu_lenght = CHASSIS_MENU_WIDTH;
                for(int i = 1; i <= ROBOT_CHASSIS_COUNT /*g_Config.m_RobotChassisConsts[0].for_player_side*/; ++i)
                {
                    if(!g_Config.m_RobotChassisConsts[i - 1].for_player_side) continue;

                    menu_lenght = max(g_PopupChassis[i - 1].text.GetLen() * 7 - 3, menu_lenght);
                    if(!player_side->m_ConstructPanel->IsEnoughResourcesForThisModule(1, MRT_CHASSIS, ERobotModuleKind(i)))
                    {
                        g_PopupChassis[i - 1].color = NOT_ENOUGH_RES_LABELS_COLOR;
                    }
                    else
                    {
                        g_PopupChassis[i - 1].color = DEFAULT_LABELS_COLOR;
                    }
                }

                g_PopupMenu->CreateMenu(
                    MENU_PARENT_CHASSIS,
                    g_Config.m_RobotChassisConsts[0].for_player_side,
                    menu_lenght,
                    g_IFaceList->m_BaseX + 321,
                    g_IFaceList->m_BaseY + 231,
                    this,
                    g_PopupChassis
                );
            }
        }
        else if(m_strName == IF_BASE_HEAD_PYLON)
        {
            if(g_PopupMenu) //Если нужно заполнить меню выбора конкретного модуля из списка
            {
                int menu_lenght = HEAD_MENU_WIDTH;
                for(int i = 1; i < ROBOT_HEADS_COUNT + 1 /*g_Config.m_RobotHeadsConsts[0].for_player_side + 1*/; ++i)
                {
                    if(!g_Config.m_RobotHeadsConsts[i].for_player_side) continue;

                    menu_lenght = max(g_PopupHead[i].text.GetLen() * 7 - 3, menu_lenght);
                    if(!player_side->m_ConstructPanel->IsEnoughResourcesForThisModule(1, MRT_HEAD, ERobotModuleKind(i)))
                    {
                        g_PopupHead[i].color = NOT_ENOUGH_RES_LABELS_COLOR;
                    }
                    else
                    {
                        g_PopupHead[i].color = DEFAULT_LABELS_COLOR;
                    }
                }

                g_PopupMenu->CreateMenu(
                    MENU_PARENT_HEAD,
                    g_Config.m_RobotHeadsConsts[0].for_player_side + 1,
                    menu_lenght,
                    g_IFaceList->m_BaseX + 315,
                    g_IFaceList->m_BaseY + 76,
                    this,
                    g_PopupHead
                );
            }
        }
        return true;
    }
    else
    {
        if(FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu)
        {
            g_PopupMenu->ResetMenu(true);
        }
        return false;
    }
}