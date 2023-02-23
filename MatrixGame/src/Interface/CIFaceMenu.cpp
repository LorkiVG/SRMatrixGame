// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../stdafx.h"
#include "CIFaceMenu.h"
#include "CInterface.h"
#include "CIFaceStatic.h"
#include "CIFaceImage.h"
#include "CIFaceElement.h"
#include "../MatrixGame.h"
#include "../MatrixSide.hpp"

CInterface* CIFaceMenu::m_MenuGraphics;
CIFaceMenu* g_PopupMenu;


bool CIFaceMenu::LoadMenuGraphics(CBlockPar& bp)
{
DTRACE();

    m_MenuGraphics->Load(bp, IF_POPUP_MENU);
    return true;
}

//Создаёт выпадающий список элементов (модулей) - используется в конструкторе
//width poshitan snaruzhi - max. dlina kakogo-to elementa - да пошёл ты нахуй с такими комментами! тебе за кириллицу отдельно доплачивали чтоль!? (Klaxons)
void CIFaceMenu::CreateMenu(
    EMenuParent parent,
    int elements,
    int width,
    int x,
    int y,
    CIFaceElement* caller,
    SMenuItemText* labels,
    int cur_hull_num, //Корпус робота, который в данный момент выбран в конструкторе
    int model_pylon //Номер оружейного пилона на корпусе, т.к. их там может быть меньше максимального количества
)
{
DTRACE();

    CInterface* main = nullptr, *ifs = g_IFaceList->m_First;
    main = m_MenuGraphics;

    /*
    while(ifs)
    {
        if(ifs->m_strName == IF_BASE)
        {
            main = ifs;
            break;
        }
        ifs = ifs->m_NextInterface;
    }
    if(!main) return;
    */

    m_InterfaceParent = parent;
    m_Caller = caller;

    int h = 0, w = 0, h_clean = 0;

    h += TOPLINE_HEIGHT + BOTTOMLINE_HEIGHT;
    h_clean = (elements * UNIT_HEIGHT);
    h += h_clean;
    width += POINTER_WIDTH;
    w += width + RIGHTLINE_WIDTH + LEFTLINE_WIDTH;

    //!!!!!!!! Соотнесение конкретного модуля с выделенным пунктом меню происходит в CalcSelectedItem !!!!!!!!

    //Определяем текущий индекс выбранного элемента в списке
    //int real_index_pos;
    //if(caller->m_Param1 == MRT_HULL || caller->m_Param1 == MRT_CHASSIS) real_index_pos = caller->m_Param2 - 1; //Корпуса и шасси не имеют пустого слота
    //else real_index_pos = caller->m_Param2;

    int real_index_pos;
    //С учётом возможного недоступного для данного слота оружия в списке меню данного пилона, также считаем отдельный скорректированный индекс visual_index_pos, по которому и будет отрисована текстура
    int visual_index_pos;
    //if(caller->m_Param1 != MRT_WEAPON) visual_index_pos = real_index_pos; //Для всех модулей кроме оружия, фактическая позиция индекса и есть его визуальная позиция в списке

    //Определяем текущую позицию индекса с учётом возможного смещения в списке (блять. сука. как же я нахуй устал от этого кривого ебаного создания меню. как же хдесь всё хуёво и через жопу приходится делать постоянно)
    switch((int)caller->m_Param1)
    {
        case MRT_HULL:
        {
            real_index_pos = caller->m_Param2 - 1;
            visual_index_pos = 0;
            for(int i = 0; i < ROBOT_HULLS_COUNT; ++i)
            {
                if(real_index_pos == i)
                {
                    visual_index_pos += i;
                    break;
                }

                if(!g_Config.m_RobotHullsConsts[i + 1].for_player_side) --visual_index_pos;
            }
            break;
        }
        case MRT_CHASSIS:
        {
            real_index_pos = caller->m_Param2 - 1;
            visual_index_pos = 0;
            for(int i = 0; i < ROBOT_CHASSIS_COUNT; ++i)
            {
                if(real_index_pos == i)
                {
                    visual_index_pos += i;
                    break;
                }

                if(!g_Config.m_RobotChassisConsts[i + 1].for_player_side) --visual_index_pos;
            }
            break;
        }
        case MRT_HEAD:
        {
            real_index_pos = caller->m_Param2;
            visual_index_pos = 0;
            for(int i = 0; i <= ROBOT_HEADS_COUNT; ++i)
            {
                if(real_index_pos == i)
                {
                    visual_index_pos += i;
                    break;
                }

                //Под индексом 0 в меню голов пустой слот, так что его не проверяем
                if(i && !g_Config.m_RobotHeadsConsts[i].for_player_side) --visual_index_pos;
            }
            break;
        }
        case MRT_WEAPON:
        {
            real_index_pos = caller->m_Param2;
            visual_index_pos = 0;
            for(int i = 0; i <= ROBOT_WEAPONS_COUNT; ++i)
            {
                if(real_index_pos == i)
                {
                    visual_index_pos += i;
                    break;
                }

                //Под индексом 0 в меню оружия пустой слот, так что его не проверяем
                if(i && (!g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data[model_pylon].fit_weapon.test(i) || !g_Config.m_RobotWeaponsConsts[i].for_player_side)) --visual_index_pos;
            }
            break;
        }
    }

    //Ramka preparation
    if(m_RamTex)
    {
        CCache::Destroy(m_RamTex);
        m_RamTex = nullptr;
    }

    m_RamTex = CACHE_CREATE_TEXTUREMANAGED();
    //Draw the shit! to the fucking textures, fuck off y'all. KidRock motherfuckers! - да, да, ебать! код на 90% полное говнище, зато будешь из себя крутого пиздюка строить теперь

    D3DLOCKED_RECT lr_src, lr_dest;
    
    CBitmap bm_src, bm_dest;
    
    int tex_height = 512, tex_width = 512;
    const int mod = 8;
    
    if(D3D_OK != m_RamTex->CreateLock(D3DFMT_A8R8G8B8, tex_height, tex_width, 1, lr_dest)) return; 
    
    bm_dest.CreateRGBA(tex_width, tex_height, lr_dest.Pitch, lr_dest.pBits);
    
    CIFaceImage* els = m_MenuGraphics->m_FirstImage;
    while(els)
    {
        if(els->m_strName == IF_POPUP_TOPLEFT)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            //bm_src.SaveInPNG(L"topleft.png");
            bm_dest.MergeWithAlpha(CPoint(1, 0), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_TOPRIGHT)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            //bm_src.SaveInPNG(L"topright.png");
            bm_dest.MergeWithAlpha(CPoint(w - TOPRIGHT_WIDTH+1, 0), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_BOTTOMLEFT)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            //bm_src.SaveInPNG(L"bottomleft.png");
            bm_dest.MergeWithAlpha(CPoint(0, h-BOTTOMLEFT_HEIGHT-mod), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_BOTTOMRIGHT)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            //bm_src.SaveInPNG(L"bottomright.png");
            bm_dest.MergeWithAlpha(CPoint(w - BOTTOMRIGHT_WIDTH - 4, h - BOTTOMRIGHT_HEIGHT - 1 - mod), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_LEFTLINE)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for(int i = 0; i < h_clean-mod; ++i)
            {
                bm_dest.MergeWithAlpha(CPoint(1, TOPLEFT_HEIGHT + i), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_RIGHTLINE)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for(int i = 0; i < h_clean-mod; ++i)
            {
                bm_dest.MergeWithAlpha(CPoint(w - RIGHTLINE_WIDTH + 1, TOPRIGHT_HEIGHT + i), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_TOPLINE)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for(int i = 0; i < width; ++i)
            {
                bm_dest.MergeWithAlpha(CPoint(TOPLEFT_WIDTH + i + 1, 0), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_BOTTOMLINE)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for(int i = 0; i < width; ++i)
            {
                bm_dest.MergeWithAlpha(CPoint(BOTTOMLEFT_WIDTH + i, h - BOTTOMLEFT_HEIGHT - mod), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_SEL)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for(int i = 0; i < elements; ++i)
            {
                for(int j = 5; j < w - 9; ++j)
                {
                    bm_dest.MergeWithAlpha(CPoint(j, 11 + (UNIT_HEIGHT * i)), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
                }
            }
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_SELRIGHT)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for(int i = 0; i < elements; ++i)
            {
                bm_dest.MergeWithAlpha(CPoint(w - 12, 11 + (UNIT_HEIGHT * i)), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_SELMOUSE)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for(int j = 5; j < w - 9; ++j)
            {
                bm_dest.MergeWithAlpha(CPoint(w + j, 0), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_SELRIGHTMOUSE)
        {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            bm_dest.MergeWithAlpha(CPoint((w * 2) - 12, 0), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        //Выбираем текстуру указателя, подходящую по цвету лейблу элемента в данном пункте списка
        else if(els->m_strName == IF_POPUP_POINTER && labels[real_index_pos].color == DEFAULT_LABELS_COLOR)
        {
            m_PointerImage = els;
            //els->m_Image->LockRect(lr_src, 0);
            //CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            //bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            //bm_dest.MergeWithAlpha(CPoint(LEFT_SPACE + 1, pointer_hpos), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            //els->m_Image->UnlockRect();
        }
        else if(els->m_strName == IF_POPUP_POINTER_NOT_AVAILABLE && labels[real_index_pos].color == NOT_ENOUGH_RES_LABELS_COLOR)
        {
            m_PointerImage = els;
        }

        els = els->m_NextImage;
    }

    int text_zone_width = w;
    int text_zone_height = 18;

    m_RamTex->UnlockRect();
    
    if(m_Ramka)
    {
        if(main && main->FindElementByName(m_Ramka->m_strName))
        {
            LIST_DEL(m_Ramka, main->m_FirstElement, main->m_LastElement, m_PrevElement, m_NextElement);
        }
        HDelete(CIFaceStatic, m_Ramka, g_MatrixHeap);
    }

    m_Ramka = HNew(g_MatrixHeap) CIFaceStatic;
    m_Ramka->m_strName = IF_POPUP_RAMKA;
    //if(main->m_xPos > x)
    //{
    x -= Float2Int(main->m_xPos);
    //}
    //if(main->m_yPos > y)
    //{
    y -= Float2Int(main->m_yPos);
    //}
    m_Ramka->m_xPos = x;
    m_Ramka->m_yPos = y;
    m_Ramka->m_zPos = 0.0f;
    m_Ramka->m_xSize = w;
    m_Ramka->m_ySize = h;
    m_Ramka->m_DefState = IFACE_NORMAL;

    m_Ramka->SetStateImage(
        IFACE_NORMAL,
        m_RamTex,
        0,
        0,
        tex_height,
        tex_width
    );

    m_Ramka->ElementGeomInit((void*)m_Ramka);
    main->AddElement(m_Ramka);
    m_Ramka->SetVisibility(true);

    //В данном случае "селектор" - это цветовое выделение выбранного курсором элемента, а не стрелка-указатель на выбранный модуль
    //Selector preparation
    if(m_Selector)
    {
        if(main->FindElementByName(m_Selector->m_strName))
        {
            LIST_DEL(m_Selector, main->m_FirstElement, main->m_LastElement, m_PrevElement, m_NextElement);
        }
        HDelete(CIFaceStatic, m_Selector, g_MatrixHeap);
    }

    m_Selector = HNew(g_MatrixHeap) CIFaceStatic;
    m_Selector->m_strName = IF_POPUP_SELECTOR;
    m_Selector->m_xPos = x;
    m_Selector->m_yPos = y + 11;
    m_Selector->m_zPos = 0;
    m_Selector->m_xSize = w;
    m_Selector->m_ySize = 18.0f;
    m_Selector->m_DefState = IFACE_NORMAL;

    m_Selector->SetStateImage(
        IFACE_NORMAL,
        m_RamTex,
        w,
        0,
        tex_height,
        tex_width);

    m_Selector->ElementGeomInit((void*)m_Selector);
    main->AddElement(m_Selector);
    m_Selector->SetVisibility(false);

    //Catchers
    CIFaceElement* e = main->m_FirstElement;
    while(e)
    {
        if(e->m_nId == POPUP_SELECTOR_CATCHERS_ID)
        {
            CIFaceElement* tmp_e = e->m_NextElement;
            LIST_DEL(e, main->m_FirstElement, main->m_LastElement, m_PrevElement, m_NextElement);
            HDelete(CIFaceStatic, (CIFaceStatic*)e, g_MatrixHeap);
            e = tmp_e;
            continue;
        }
        e = e->m_NextElement;
    }

    int label_el_num = 0;
    for(int i = 0; i < elements; ++i, ++label_el_num)
    {
        CIFaceStatic* catcher = HNew(g_MatrixHeap) CIFaceStatic;
        catcher->m_strName = L"";
        catcher->m_xPos = x;
        catcher->m_yPos = y + 11 + (UNIT_HEIGHT * i);
        catcher->m_zPos = 0;
        catcher->m_xSize = (w - 8);
        catcher->m_ySize = 19.0f;
        catcher->m_DefState = IFACE_NORMAL;

        catcher->SetStateImage(
            IFACE_NORMAL,
            m_RamTex,
            w * 2,
            (11 + (UNIT_HEIGHT * i)),
            tex_height,
            tex_width
        );

        catcher->ElementGeomInit((void*)catcher);
        main->AddElement(catcher);
        catcher->SetVisibility(false);
        catcher->m_nId = POPUP_SELECTOR_CATCHERS_ID;
        catcher->m_iParam = i;

        //Здесь по-уебански перебирается массив с названиями модулей, которые заносятся в список отдельно
        //Добавляем только те названия, которые доступны для выбора игроку
        if(labels)
        {
            if(label_el_num)
            {
                switch((int)caller->m_Param1)
                {
                    case MRT_HULL:
                    {
                        while(!g_Config.m_RobotHullsConsts[label_el_num + 1].for_player_side)
                        {
                            ++label_el_num;
                        }
                        break;
                    }
                    case MRT_CHASSIS:
                    {
                        while(!g_Config.m_RobotChassisConsts[label_el_num + 1].for_player_side)
                        {
                            ++label_el_num;
                        }
                        break;
                    }
                    case MRT_HEAD:
                    {
                        while(!g_Config.m_RobotHeadsConsts[label_el_num].for_player_side)
                        {
                            ++label_el_num;
                        }
                        break;
                    }
                    case MRT_WEAPON:
                    {
                        while(!g_Config.m_RobotHullsConsts[cur_hull_num].weapon_pylon_data[model_pylon].fit_weapon.test(label_el_num) || !g_Config.m_RobotWeaponsConsts[label_el_num].for_player_side)
                        {
                            ++label_el_num;
                        }
                        break;
                    }
                }
            }

            catcher->m_StateImages[IFACE_NORMAL].SetStateLabelParams(LEFT_SPACE + 6, -3, text_zone_width, text_zone_height, 0, 1, 0, 0, 0, CRect(0, 0, text_zone_width, text_zone_height), labels[label_el_num].text, CWStr(L"Font.2Ranger"), labels[label_el_num].color);
            catcher->m_StateImages[IFACE_NORMAL].SetStateText(true);
        }
    }

    //Курсор - это стрелка-указатель на выбранный элемент
    //Pointer preparation
    //Удаляем ранее созданную текстуру стрелки-указателя
    if(m_Pointer)
    {
        if(main && main->FindElementByName(m_Pointer->m_strName))
        {
            LIST_DEL(m_Pointer, main->m_FirstElement, main->m_LastElement, m_PrevElement, m_NextElement);
        }
        HDelete(CIFaceStatic, m_Pointer, g_MatrixHeap);
    }

    //Рендерим на экране новую текстуру стрелки-указателя
    int pointer_hpos = (visual_index_pos * UNIT_HEIGHT) + TOPLINE_HEIGHT;
    if(m_PointerImage && pointer_hpos <= h_clean) //Проверяем наличие пикчи, и чтобы положение указателя находилось в границах менюшки
    {
        m_Pointer = main->CreateStaticFromImage(float(x + LEFT_SPACE), float(y + pointer_hpos), 0, *m_PointerImage);
        m_Pointer->SetVisibility(false);
        m_Pointer->m_strName = IF_POPUP_POINTER_OBJECT;
    }
    else m_Pointer = nullptr;

    //bm_dest.SaveInPNG(L"test_menu.png");
    SETFLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE);
    m_CurMenuPos = -1;
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    
    if(m_RobotConfig) HDelete(SRobotConfig, m_RobotConfig, g_MatrixHeap);

    m_RobotConfig = HNew(g_MatrixHeap) SRobotConfig;
    *m_RobotConfig = ps->m_ConstructPanel->m_Configs[ps->m_ConstructPanel->m_CurrentConfig];
}

void CIFaceMenu::SetSelectorPos(
    const float& x,
    const float& y,
    int pos
)
{
    if(!m_Selector || m_CurMenuPos == pos) return;

    m_CurMenuPos = pos;
    m_Selector->RecalcPos(x, y, false);

    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    if(!ps) return;

    CalcSelectedItem(false);
    //if(m_Caller) ps->m_ConstructPanel->FocusElement(m_Caller);
}

//Игрок выбрал модуль на замену из выпадающего списка модулей, а не щелчком по кнопке модуля
void CIFaceMenu::OnMenuItemPress()
{
    bool rmode = false;
    if(m_Selector && m_Selector->ElementCatch(g_MatrixMap->m_Cursor.GetPos())) CalcSelectedItem(true);
    else rmode = true;
    ResetMenu(rmode);
}

void CIFaceMenu::ResetMenu(bool canceled)
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

    //restore saved robot configuration
    if(m_RobotConfig)
    {
        if(canceled)
        {
            if(m_InterfaceParent == MENU_PARENT_PILON_1)
            {
                ps->m_Constructor->ReplaceRobotModule(MRT_WEAPON, m_RobotConfig->m_Weapon[0].m_nKind, RUK_WEAPON_PYLON_1);
            }
            else if(m_InterfaceParent == MENU_PARENT_PILON_2)
            {
                ps->m_Constructor->ReplaceRobotModule(MRT_WEAPON, m_RobotConfig->m_Weapon[1].m_nKind, RUK_WEAPON_PYLON_2);
            }
            else if(m_InterfaceParent == MENU_PARENT_PILON_3)
            {
                ps->m_Constructor->ReplaceRobotModule(MRT_WEAPON, m_RobotConfig->m_Weapon[2].m_nKind, RUK_WEAPON_PYLON_3);
            }
            else if(m_InterfaceParent == MENU_PARENT_PILON_4)
            {
                ps->m_Constructor->ReplaceRobotModule(MRT_WEAPON, m_RobotConfig->m_Weapon[3].m_nKind, RUK_WEAPON_PYLON_4);
            }
            else if(m_InterfaceParent == MENU_PARENT_PILON_5)
            {
                ps->m_Constructor->ReplaceRobotModule(MRT_WEAPON, m_RobotConfig->m_Weapon[4].m_nKind, RUK_WEAPON_PYLON_5);
            }
            else if(m_InterfaceParent == MENU_PARENT_HEAD)
            {
                ps->m_Constructor->ReplaceRobotModule(MRT_HEAD, m_RobotConfig->m_Head.m_nKind, 0);
            }
            else if(m_InterfaceParent == MENU_PARENT_HULL)
            {
                ps->m_Constructor->ReplaceRobotModule(MRT_HULL, m_RobotConfig->m_Hull.m_Module.m_nKind, 0);
                for(int i = 0; i < RUK_WEAPON_PYLONS_COUNT && m_RobotConfig->m_Weapon[i].m_nKind; ++i)
                {
                    ps->m_Constructor->ReplaceRobotModule(MRT_WEAPON, m_RobotConfig->m_Weapon[i].m_nKind, i);
                }
                ps->m_Constructor->ReplaceRobotModule(MRT_HEAD, m_RobotConfig->m_Head.m_nKind, 0);
            }
            else if(m_InterfaceParent == MENU_PARENT_CHASSIS)
            {
                ps->m_Constructor->ReplaceRobotModule(MRT_CHASSIS, m_RobotConfig->m_Chassis.m_nKind, 0);
            }
            //ps->m_Constructor->ReplaceRobotModule
            //int cfg = ps->m_ConstructPanel->m_CurrentConfig;
            //ps->m_ConstructPanel->m_Configs[cfg] = *m_RobotConfig;
        }

        HDelete(SRobotConfig, m_RobotConfig, g_MatrixHeap);
        m_RobotConfig = nullptr;
    }

    m_Caller = nullptr;
    m_InterfaceParent = MENU_PARENT_UNDEF;
    m_CurMenuPos = -1;
    if(g_IFaceList) RESETFLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE);
}

//Определяет, какой именно модуль будет выделен игроком при наведении на очередной элемент выпадающего меню
void CIFaceMenu::CalcSelectedItem(bool set)
{
    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    if(!ps || m_InterfaceParent == MENU_PARENT_UNDEF) return;

    ERobotModuleType type;
    int kind;

    int const_pylon = 0;
    int model_pylon = 0;
    switch(m_InterfaceParent)
    {
        case MENU_PARENT_HULL:
        {
            const_pylon = 0;
            type = MRT_HULL;

            //Определяем следующий подходящий корпус в выпадающем списке
            //Корректируем индекс в списке корпусов с учётом недоступных для игрока позиций
            kind = -1; //Нумерация элементов в выпадающем списке корпусов всегда начинается с нуля
            for(int i = 0; i < ROBOT_HULLS_COUNT; ++i)
            {
                if(!g_Config.m_RobotHullsConsts[i + 1].for_player_side) continue;
                ++kind;

                if(kind == m_CurMenuPos)
                {
                    kind = i;
                    break;
                }
            }
            ++kind; //Нумерация корпусов в массиве идёт с 1

            break;
        }
        case MENU_PARENT_CHASSIS:
        {
            const_pylon = 0;
            type = MRT_CHASSIS;

            //Определяем следующее подходящее шасси в выпадающем списке
            //Корректируем индекс в списке шасси с учётом недоступных для игрока позиций
            kind = -1; //Нумерация элементов в выпадающем списке шасси всегда начинается с нуля
            for(int i = 0; i < ROBOT_CHASSIS_COUNT; ++i)
            {
                if(!g_Config.m_RobotChassisConsts[i + 1].for_player_side) continue;
                ++kind;

                if(kind == m_CurMenuPos)
                {
                    kind = i;
                    break;
                }
            }
            ++kind; //Нумерация шасси в массиве идёт с 1

            break;
        }
        case MENU_PARENT_HEAD:
        {
            const_pylon = 0;
            type = MRT_HEAD;

            //Определяем следующую подходящую голову в выпадающем списке
            if(!m_CurMenuPos) kind = RUK_EMPTY;
            else
            {
                //Корректируем индекс в списке голов с учётом недоступных для игрока позиций
                kind = 0;
                for(int i = 1; i <= ROBOT_HEADS_COUNT; ++i)
                {
                    if(!g_Config.m_RobotHeadsConsts[i].for_player_side) continue;
                    ++kind;

                    if(kind == m_CurMenuPos)
                    {
                        kind = i;
                        break;
                    }
                }
            }
            break;
        }
        case MENU_PARENT_PILON_1:
        case MENU_PARENT_PILON_2:
        case MENU_PARENT_PILON_3:
        case MENU_PARENT_PILON_4:
        case MENU_PARENT_PILON_5:
        {
            //Сперва определяем фактический номер пилона на модели
            int cur_hull_kind = g_MatrixMap->GetPlayerSide()->m_ConstructPanel->m_Configs[g_MatrixMap->GetPlayerSide()->m_ConstructPanel->m_CurrentConfig].m_Hull.m_Module.m_nKind;
            for(int i = 0; i < g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data.size(); ++i)
            {
                if(g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[i].constructor_slot_num == m_InterfaceParent - 1)
                {
                    model_pylon = i;
                    break;
                }
            }

            const_pylon = m_InterfaceParent - 1;
            type = MRT_WEAPON;

            //Определяем следующее подходящее оружие в выпадающем списке
            if(!m_CurMenuPos) kind = RUK_EMPTY;
            else
            {
                //Корректируем индекс в списке оружия с учётом недоступных для игрока и данного слота позиций
                kind = 0;
                for(int i = 1; i <= ROBOT_WEAPONS_COUNT; ++i)
                {
                    if(!g_Config.m_RobotWeaponsConsts[i].for_player_side || !g_Config.m_RobotHullsConsts[cur_hull_kind].weapon_pylon_data[model_pylon].fit_weapon.test(i)) continue;
                    ++kind;

                    if(kind == m_CurMenuPos)
                    {
                        kind = i;
                        break;
                    }
                }
            }
            break;
        }
    }
    
    if(set) ps->m_Constructor->ReplaceRobotModule(type, (ERobotModuleKind)kind, const_pylon, model_pylon); //Если игрок выбрал новый модуль
    else ps->m_Constructor->TempReplaceRobotModuleFromMenu(type, (ERobotModuleKind)kind, const_pylon, model_pylon); //Если просто перебирает их мышкой
}