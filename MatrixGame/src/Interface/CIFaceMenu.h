// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CConstructor.h"

#pragma once
class CInterface;
class CIFaceStatic;
class CIFaceElement;
class CIFaceImage;
struct SRobotConfig;
enum ERobotModuleType;
enum ERobotModuleKind;

#define UNIT_HEIGHT 19
#define TOPLEFT_HEIGHT 18
#define TOPLEFT_WIDTH 13
#define TOPRIGHT_HEIGHT 18
#define TOPRIGHT_WIDTH 18
#define BOTTOMLEFT_HEIGHT 22
#define BOTTOMLEFT_WIDTH 14
#define BOTTOMRIGHT_HEIGHT 21
#define BOTTOMRIGHT_WIDTH 13
#define TOPLINE_HEIGHT 18
#define TOPLINE_WIDTH 1
#define BOTTOMLINE_HEIGHT 22
#define BOTTOMLINE_WIDTH 1
#define RIGHTLINE_HEIGHT 1
#define RIGHTLINE_WIDTH 18
#define LEFTLINE_HEIGHT 1
#define LEFTLINE_WIDTH 13
#define POINTER_WIDTH 7
#define LEFT_SPACE 7

#define DEFAULT_LABELS_COLOR         0xFFF6c000
#define NOT_ENOUGH_RES_LABELS_COLOR  0xFFFF4319
#define SELECTED_LABELS_COLOR        0xFFFFFFFF

//ENGLISH_BUILD (гудбай америка, О!)
/*
#define HULL_MENU_WIDTH         60
#define CHASSIS_MENU_WIDTH      60
#define HEAD_MENU_WIDTH         45
#define WEAPON_MENU_WIDTH       95
*/

#define HULL_MENU_WIDTH         70
#define CHASSIS_MENU_WIDTH      70
#define HEAD_MENU_WIDTH         70
#define WEAPON_MENU_WIDTH       70

enum EMenuParent {
    MENU_PARENT_UNDEF = 0,

    MENU_PARENT_PILON_1 = 1,
    MENU_PARENT_PILON_2 = 2,
    MENU_PARENT_PILON_3 = 3,
    MENU_PARENT_PILON_4 = 4,
    MENU_PARENT_PILON_5 = 5,

    MENU_PARENT_HEAD = 6,
    MENU_PARENT_HULL = 7,
    MENU_PARENT_CHASSIS = 8,
};

struct SMenuItemText {
    CWStr text;
    dword color = 0;

    SMenuItemText(CHeap* heap) : text(heap) { color = DEFAULT_LABELS_COLOR; }
};

//Класс выпадающего списка для выбора модулей в конструкторе (по нажатию ПКМ на модуле)
class CIFaceMenu : public CMain
{
    float m_Width = 0.0f;
    float m_Height = 0.0f;
    int m_ElementsNum = 0;

    bool m_Visible = false;

    CIFaceStatic* m_Ramka = nullptr;
    CIFaceStatic* m_Selector = nullptr;
    CIFaceStatic* m_Pointer = nullptr;
    CIFaceImage* m_PointerImage = nullptr;

    CTextureManaged* m_RamTex = nullptr;

    int m_CurMenuPos = 0;
    EMenuParent m_InterfaceParent = MENU_PARENT_UNDEF;

    CIFaceElement* m_Caller = nullptr;

    SRobotConfig* m_RobotConfig = nullptr;

public:
    static CInterface* m_MenuGraphics;
    static bool LoadMenuGraphics(CBlockPar& bp);
    void CreateMenu(EMenuParent parent, int elements, int width, int x, int y, CIFaceElement* caller, SMenuItemText* labels = nullptr, int cur_hull_num = 0, int model_pylon = 0);

    void SetSelectorPos(const float& x, const float& y, int pos);
    bool Selector()             { return m_Selector != nullptr; }
    CIFaceStatic* GetRamka()    { return m_Ramka; }
    
    EMenuParent GetMenuParent() { return m_InterfaceParent; }
    
    void OnMenuItemPress();
    void CalcSelectedItem(bool set);

    void ResetMenu(bool canceled);

    CIFaceMenu() = default;
    ~CIFaceMenu()
    {
        if(m_RamTex)
        {
            CCache::Destroy(m_RamTex);
            m_RamTex = nullptr;
        }

        if(m_RobotConfig) HDelete(SRobotConfig, m_RobotConfig, g_MatrixHeap);
    }
};

extern CIFaceMenu* g_PopupMenu;