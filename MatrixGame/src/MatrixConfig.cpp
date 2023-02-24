// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "StringConstants.hpp"
#include "MatrixConfig.hpp"
#include "MatrixGameDll.hpp"
#include <vector>

struct SKeyCodes
{
    const wchar* name;
    const wchar* name_alt;
    int code;
};

static SKeyCodes key_codes[] =
{
    { L"LMB",  L"LMouse",  VK_LBUTTON },
    { L"RMB",  L"RMouse",  VK_RBUTTON },
    { L"MMB",  L"MMouse",  VK_MBUTTON },
    { L"X1MB", L"X1Mouse", VK_XBUTTON1 },
    { L"X2MB", L"X2Mouse", VK_XBUTTON2 },

    { L"Esc", L"Escape",   VK_ESCAPE },
    { L"F1",  L"F1",       0x70 },
    { L"F2",  L"F2",       0x71 },
    { L"F3",  L"F3",       0x72 },
    { L"F4",  L"F4",       0x73 },
    { L"F5",  L"F5",       0x74 },
    { L"F6",  L"F6",       0x75 },
    { L"F7",  L"F7",       0x76 },
    { L"F8",  L"F8",       0x77 },
    { L"F9",  L"F9",       0x78 },
    { L"F10", L"F10",      0x79 },
    { L"F11", L"F11",      0x7A },
    { L"F12", L"F12",      0x7B },

    { L"PrintScreen", L"PrtScr",   VK_SNAPSHOT },
    { L"Pause",       L"Break",    VK_PAUSE },

    { L"ScrollLock",  L"ScrLk",    VK_SCROLL },
    { L"CapsLock",    L"CapsLock", VK_CAPITAL },
    { L"NumLock",     L"NumLk",    VK_NUMLOCK },

    { L"~",  L"`",   0xC0 },
    { L"/",  L"?",   0xBF },
    { L"\\", L"|",   VK_OEM_5 },
    { L"<",  L",",   VK_OEM_COMMA },
    { L">",  L".",   VK_OEM_PERIOD },
    { L";",  L":",   0xBA },
    { L"'",  L"\"",  0xDE },
    //Для этих скобок нельзя задавать альтернативу { }, иначе парсер блокпара ломается
    { L"[",  L"[",   VK_OEM_4 },
    { L"]",  L"]",   VK_OEM_6 },
    { L"-",  L"_",   VK_OEM_MINUS },
    { L"+",  L"=",   VK_OEM_PLUS },

    { L"NumPad/", L"Num/", VK_DIVIDE },
    { L"NumPad*", L"Num*", VK_MULTIPLY },
    { L"NumPad-", L"Num-", VK_SUBTRACT },
    { L"NumPad+", L"Num+", VK_ADD },
    //{ L"NumPadEnter", L"NumEnter",  -1 }, //?
    { L"NumPad.", L"Num.", VK_SEPARATOR },
    { L"NumPad0", L"Num0", VK_NUMPAD0 },
    { L"NumPad1", L"Num1", VK_NUMPAD1 },
    { L"NumPad2", L"Num2", VK_NUMPAD2 },
    { L"NumPad3", L"Num3", VK_NUMPAD3 },
    { L"NumPad4", L"Num4", VK_NUMPAD4 },
    { L"NumPad5", L"Num5", VK_NUMPAD5 },
    { L"NumPad6", L"Num6", VK_NUMPAD6 },
    { L"NumPad7", L"Num7", VK_NUMPAD7 },
    { L"NumPad8", L"Num8", VK_NUMPAD8 },
    { L"NumPad9", L"Num9", VK_NUMPAD9 },

    { L"0", L")",    0x30 },
    { L"1", L"!",    0x31 },
    { L"2", L"@",    0x32 },
    { L"3", L"#",    0x33 },
    { L"4", L"$",    0x34 },
    { L"5", L"%",    0x35 },
    { L"6", L"^",    0x36 },
    { L"7", L"&",    0x37 },
    { L"8", L"*",    0x38 },
    { L"9", L"(",    0x39 },

    { L"Left",  L"LeftKey",       VK_LEFT },
    { L"Right", L"RightKey",      VK_RIGHT },
    { L"Up",    L"UpKey",         VK_UP },
    { L"Down",  L"DownKey",       VK_DOWN },

    { L"Backspace", L"Backspace", VK_BACK },
    { L"Tab",       L"Tab",       VK_TAB },
    { L"Enter",     L"Return",    VK_RETURN },
    { L"Space",     L"Space",     VK_SPACE },

    { L"Insert",    L"Ins",       VK_INSERT },
    { L"Delete",    L"Del",       VK_DELETE },
    { L"Home",      L"Home",      VK_HOME },
    { L"End",       L"End",       VK_END },
    { L"PageUp",    L"PgUp",      VK_PRIOR },
    { L"PageDown",  L"PgDn",      VK_NEXT },

    { L"Shift",  L"Shift",        VK_SHIFT },
    { L"LShift", L"LeftShift",    VK_LSHIFT },
    { L"RShift", L"RightShift",   VK_RSHIFT },
    { L"Alt",    L"Alt",          VK_MENU },
    { L"LAlt",   L"LeftAlt",      VK_LMENU },
    { L"RAlt",   L"RightAlt",     VK_RMENU },
    { L"Ctrl",   L"Ctrl",         VK_CONTROL },
    { L"LCtrl",  L"LeftCtrl",     VK_LCONTROL },
    { L"RCtrl",  L"RightCtrl",    VK_RCONTROL },

    { L"LWin",   L"LeftWin",      VK_LWIN },
    { L"RWin",   L"RightWin",     VK_RWIN },
    { L"Menu",   L"Application",  VK_APPS },

    { L"Q", L"q", 0x51 },
    { L"W", L"w", 0x57 },
    { L"E", L"e", 0x45 },
    { L"R", L"r", 0x52 },
    { L"T", L"t", 0x54 },
    { L"Y", L"y", 0x59 },
    { L"U", L"u", 0x55 },
    { L"I", L"i", 0x49 },
    { L"O", L"o", 0x4F },
    { L"P", L"p", 0x50 },

    { L"A", L"a", 0x41 },
    { L"S", L"s", 0x53 },
    { L"D", L"d", 0x44 },
    { L"F", L"f", 0x46 },
    { L"G", L"g", 0x47 },
    { L"H", L"h", 0x48 },
    { L"J", L"j", 0x4A },
    { L"K", L"k", 0x4B },
    { L"L", L"l", 0x4C },

    { L"Z", L"z", 0x5A },
    { L"X", L"x", 0x58 },
    { L"C", L"c", 0x43 },
    { L"V", L"v", 0x56 },
    { L"B", L"b", 0x42 },
    { L"N", L"n", 0x4E },
    { L"M", L"m", 0x4D },

    { nullptr, nullptr, -1}
};

struct SKeyActions
{
    const wchar* name;
    int code;
    int base_key;
};

static SKeyActions key_action_codes[] =
{
    //Управление камерой (работает только в стратегическом режиме)
    { L"CamMoveUp",          		  KA_SCROLL_UP,                  0x57 },
    { L"CamMoveDown",        		  KA_SCROLL_DOWN,                0x53 },
    { L"CamMoveLeft",        		  KA_SCROLL_LEFT,                0x41 },
    { L"CamMoveRight",       		  KA_SCROLL_RIGHT,               0x44 },

    { L"CamMoveUpAlt",       		  KA_SCROLL_UP_ALT,              VK_UP },
    { L"CamMoveDownAlt",     		  KA_SCROLL_DOWN_ALT,            VK_DOWN },
    { L"CamMoveLeftAlt",     		  KA_SCROLL_LEFT_ALT,            VK_LEFT },
    { L"CamMoveRightAlt",    		  KA_SCROLL_RIGHT_ALT,           VK_RIGHT },

    { L"CamRotateUp",        		  KA_ROTATE_UP,                  VK_PRIOR }, //клавиша "PageUp"
    { L"CamRotateDown",      		  KA_ROTATE_DOWN,                VK_NEXT }, //клавиша "PageDown"
    { L"CamRotateLeft",      		  KA_ROTATE_LEFT,                VK_HOME }, //клавиша "Home"
    { L"CamRotateRight",    		  KA_ROTATE_RIGHT,               VK_END }, //клавиша "End"
    { L"CamRotateLeftAlt",   		  KA_ROTATE_LEFT_ALT,            VK_OEM_4 }, //клавиша "["
    { L"CamRotateRightAlt",  		  KA_ROTATE_RIGHT_ALT,           VK_OEM_6 }, //клавиша "]"

    { L"CamDefaultPos",      		  KA_CAM_SETDEFAULT,             VK_OEM_5 },  //клавиша "\"

    //Масштабирование миникарты
    { L"MinimapZoomIn",      		  KA_MINIMAP_ZOOMIN,             VK_OEM_PLUS }, //клавиша "+"
    { L"MinimapZoomOut",     		  KA_MINIMAP_ZOOMOUT,            VK_OEM_MINUS }, //клавиша "-"

    //Меню управления зданиями (в основном базой)
    { L"BaseSetGatheringPoint",        KA_GATHERING_POINT,            0x47 }, //установка точки сбора базы, клавиша "G"

    { L"BaseConstMenuEnter",           KA_BUILD_ROBOT,                0x42 }, //открывает и закрывает меню конструктора робота, клавиша "B"
    //Только из меню конструктора роботов
    { L"BaseConstMenuLunch",           KA_BUILD_ROBOT_START,          VK_RETURN }, //клавиша "Enter"
    { L"BaseConstMenuIncrease",        KA_BUILD_ROBOT_QUANTITY_UP,    VK_UP },
    { L"BaseConstMenuDecrease",        KA_BUILD_ROBOT_QUANTITY_DOWN,  VK_DOWN },
    { L"BaseConstMenuChooseLeft",      KA_BUILD_ROBOT_CHOOSE_LEFT,    VK_LEFT },
    { L"BaseConstMenuChooseRight",     KA_BUILD_ROBOT_CHOOSE_RIGHT,   VK_RIGHT },

    { L"BuildTurrMenuEnter",           KA_BUILD_TURRET,               0x54 }, //открывает и закрывает меню выбора турели, клавиша "T"
    //Только из меню выбора турели
    { L"BuildTurrMenuLightCannon",     KA_TURRET_CANNON,              0x43 }, //выбор лёгкой пушки, клавиша "C"
    { L"BuildTurrMenuHeavyCannon",     KA_TURRET_GUN,                 0x47 }, //выбор тяжёлой пушки, клавиша "G"
    { L"BuildTurrMenuLaserCannon",     KA_TURRET_LASER,               0x4C }, //выбор лазера, клавиша "L"
    { L"BuildTurrMenuMissileCannon",   KA_TURRET_ROCKET,              0x52 }, //выбор ракетницы, клавиша "R"

    { L"CallForReinforcements",        KA_CALL_REINFORCEMENTS,        0x48 }, //вызывает подкрепление, клавиша "H"

    //Выделение/переключение юнитов
    { L"UnitInGroupSelect",            KA_SHIFT,                      VK_SHIFT },
    { L"SetCtrlGroup",                 KA_CTRL,                       VK_CONTROL },
    { L"UnitSelectPrev",               KA_ORDER_ROBOT_SWITCH1,        VK_OEM_COMMA }, //клавиша "<"
    { L"UnitSelectNext",               KA_ORDER_ROBOT_SWITCH2,        VK_OEM_PERIOD }, //клавиша ">"
    { L"UnitSelectAll",                KA_ALL_UNITS_SELECT,           VK_F2 },

    //Меню приказов для юнитов
    { L"UnitSetMoveOrder",             KA_ORDER_MOVE,                 0x46 }, //выбор приказа на движение в указанную точку, клавиша "F"
    { L"UnitSetStopOrder",             KA_ORDER_STOP,                 0x43 }, //приказ на остановку, клавиша "C"
    { L"UnitSetAttackOrder",           KA_ORDER_ATTACK,               0x55 }, //выбор приказа на атаку, клавиша "U"
    { L"UnitSetCaptureOrder",          KA_ORDER_CAPTURE,              0x4B }, //выбор приказа на захват, клавиша "K"
    { L"UnitSetPatrolOrder",           KA_ORDER_PATROL,               0x50 }, //выбор приказа на патрулирование, клавиша "P"
    { L"UnitSetExplodeOrder",          KA_ORDER_EXPLODE,              0x45 }, //выбор приказа на подрыв бомбы (если у робота она есть), клавиша "E"
    { L"UnitSetRepairOrder",           KA_ORDER_REPAIR,               0x52 }, //выбор приказа на ремонт (если есть ремонтник), клавиша "R"

    //Помимо отмены выбранных приказов также используется для закрытия меню постройки турелей и конструктора роботов
    { L"ChoosedOrderCancel",           KA_ORDER_CANCEL,               0x58 }, //клавиша "X"

    //Программы автоматического поведения юнитов
    { L"UnitSetAgressiveBehavior",     KA_AUTOORDER_ATTACK,           0x56 }, //активация программы наступления, клавиша "V"
    { L"UnitSetCaptureBehavior",       KA_AUTOORDER_CAPTURE,          0x42 }, //активация программы захвата, клавиша "B"
    { L"UnitSetDefensiveBehavior",     KA_AUTOORDER_DEFEND,           0x4E }, //активация программы защиты, клавиша "N"

    //Ручное управление юнитом (аркадный режим)
    { L"ManualControlEnter",           KA_UNIT_ENTER,                 VK_RETURN }, //клавиша "Enter" (if not dialog mode!)
    { L"ManualControlEnterAlt",        KA_UNIT_ENTER_ALT,             VK_SPACE }, //клавиша "Space" (if not dialog mode!)
    { L"ManualControlFire",            KA_FIRE,                       VK_LBUTTON },
    { L"ManualControlAutopilot",       KA_AUTO,                       VK_RBUTTON }, //Следование вертолёта за курсором мыши (только для вертолётов)
    { L"ManualControlExplode",         KA_UNIT_BOOM,                  0x46 }, //клавиша "F"

    { L"ManualControlMoveForward",     KA_UNIT_FORWARD,               0x57 }, //W
    { L"ManualControlMoveBackward",    KA_UNIT_BACKWARD,              0x53 }, //S
    { L"ManualControlMoveLeft",        KA_UNIT_STRAFE_LEFT,           0x51 }, //Q
    { L"ManualControlMoveRight",       KA_UNIT_STRAFE_RIGHT,          0x45 }, //E
    { L"ManualControlRotateLeft",      KA_UNIT_ROTATE_LEFT,           0x41 }, //A - используется только для роботов
    { L"ManualControlRotateRight",     KA_UNIT_ROTATE_RIGHT,          0x44 }, //D - используется только для роботов
    { L"ManualControlClimbUp",         KA_UNIT_UP,                    0x51 }, //Q
    { L"ManualControlClimbDown",       KA_UNIT_DOWN,                  0x45 }, //E

    { L"ManualControlMoveForwardAlt",  KA_UNIT_FORWARD_ALT,           VK_UP },
    { L"ManualControlMoveBackwardAlt", KA_UNIT_BACKWARD_ALT,          VK_DOWN },
    { L"ManualControlMoveLeftAlt",     KA_UNIT_STRAFE_LEFT_ALT,       VK_NUMPAD4 },
    { L"ManualControlMoveRightAlt",    KA_UNIT_STRAFE_RIGHT_ALT,      VK_NUMPAD6 },
    { L"ManualControlRotateLeftAlt",   KA_UNIT_ROTATE_LEFT_ALT,       VK_LEFT },
    { L"ManualControlRotateRightAlt",  KA_UNIT_ROTATE_RIGHT_ALT,      VK_RIGHT },
    { L"ManualControlClimbUpAlt",      KA_UNIT_UP_ALT,                VK_NUMPAD8 },
    { L"ManualControlClimbDownAlt",    KA_UNIT_DOWN_ALT,              VK_NUMPAD2 },
    { L"ManualControlSetGun1",         KA_UNIT_SET_GUN_1,             0x31 }, //клавиша 1
    { L"ManualControlSetGun2",         KA_UNIT_SET_GUN_2,             0x32 }, //клавиша 2
    { L"ManualControlSetGun3",         KA_UNIT_SET_GUN_3,             0x33 }, //клавиша 3
    { L"ManualControlSetGun4",         KA_UNIT_SET_GUN_4,             0x34 }, //клавиша 4
    { L"ManualControlSetGun5",         KA_UNIT_SET_GUN_5,             0x35 }, //клавиша 5
    { L"ManualControlRearView",        KA_UNIT_REAR_VIEW,             0x58 }, //клавиша "X"

    //Прочее
    { L"TakeScreenShot",               KA_TAKE_SCREENSHOT,            VK_SNAPSHOT },
    { L"SaveScreenShot",               KA_SAVE_SCREENSHOT,            VK_F9 },
    { L"PauseTheGame",                 KA_GAME_PAUSED,                VK_PAUSE },

    { nullptr,                         -1,                            -1 }
};

//Возвращаем/проверяем наличие кода клавиши привязанной к имени клавиши
static const int KeyName2KeyCode(const CWStr& name)
{
    int i = 0;
    while(key_codes[i].name != nullptr)
    {
        if(key_codes[i].name == name || key_codes[i].name_alt == name) return key_codes[i].code;
        ++i;
    }

    return -1;
}
//Возвращаем/проверяем наличие названия клавиши, соответствующей базовому бинду номера действия
static const wchar* KeyActionNum2KeyName(const int& num)
{
    int i = 0;
    while(key_codes[i].name != nullptr)
    {
        if(key_codes[i].code == key_action_codes[num].base_key) return key_codes[i].name;
        ++i;
    }

    return nullptr;
}
//Возвращаем/проверяем наличие кода действия привязанного к имени действия
static const int KeyActionName2KeyActionCode(const CWStr& name)
{
    int i = 0;
    while(key_action_codes[i].name != nullptr)
    {
        if(key_action_codes[i].name == name) return key_action_codes[i].code;
        ++i;
    }

    return -1;
}

//Возвращаем название клавиши, привязанной (уже после загрузки игры и считывания конфига) к указанному действию (для подстановки в хинты кнопок приказов)
const wchar* CMatrixConfig::KeyActionCode2KeyName(const int& num)
{
    int key_num = g_Config.m_KeyActions[num];
    int i = 0;
    while(key_codes[i].code > -1)
    {
        if(key_codes[i].code == key_num) return key_codes[i].name;
        ++i;
    }
    return L"";
}

void CMatrixConfig::ApplySettings(SRobotsSettings* set)
{
    m_IzvratMS = set->m_IzvratMS;
    m_LandTexturesGloss = set->m_LandTexturesGloss;
    m_ObjTexturesGloss = set->m_ObjTexturesGloss;

    if(set->m_RobotShadow == 0) m_RobotShadow = SHADOW_OFF;
    else if(set->m_RobotShadow == 1) m_RobotShadow = SHADOW_STENCIL;

    m_ShowProjShadows = set->m_ShowProjShadows;
    m_ShowStencilShadows = set->m_ShowStencilShadows;
    m_SkyBox = set->m_SkyBox;
    m_SoftwareCursor = set->m_SoftwareCursor;
		
    m_GammaR.brightness = set->m_Brightness;
    m_GammaR.contrast = set->m_Contrast;
    m_GammaR.gamma = 1.0f;
    m_GammaG = m_GammaR;
    m_GammaB = m_GammaR;
}

void CMatrixConfig::SetDefaults(void)
{
DTRACE();

    g_CamFieldOfView = 60.0f;
    g_MaxFPS = 1000;
    g_MaxViewDistance = 4000.0f;
    g_MaxObjectsPerScreen = 2560;
    g_MaxEffectsCount = 1280;
    g_ShadowsDrawDistance = 1024;
    g_ThinFogDrawDistance = 0.5f;
    g_DenseFogDrawDistance = 0.7f;
    g_PlayerRobotsAutoBoom = 0;
    g_EnableFlyers = false;

    //m_TexTopDownScalefactor = 0;
    //m_TexTopMinSize = 32;
    //m_TexBotDownScalefactor = 0;
    //m_TexBotMinSize = 32;
    m_SoftwareCursor = false;

    m_ObjTexturesGloss = true;
    //m_ObjTextures16 = true;

    m_LandTexturesGloss = true;
    m_DIFlags = 0;
    m_VertexLight = true;

    m_Cursors = nullptr;

    m_ShowStencilShadows = true;
    m_ShowProjShadows = true;
    m_CannonsLogic = true;

    m_IzvratMS = false;

    m_IsManualMissileControl = false;

    //m_PlayerRobotsCnt = 0;
    //m_CompRobotsCnt = 0;

    m_RobotShadow = SHADOW_STENCIL;

    m_GammaR.brightness = 0.5f;
    m_GammaR.contrast = 0.5f;
    m_GammaR.gamma = 1.0f;

    m_GammaG.brightness = 0.5f;
    m_GammaG.contrast = 0.5f;
    m_GammaG.gamma = 1.0f;

    m_GammaB.brightness = 0.5f;
    m_GammaB.contrast = 0.5f;
    m_GammaB.gamma = 1.0f;

    // camera properties

    m_CamParams[CAMERA_STRATEGY].m_CamMouseWheelStep = 0.05f;
    m_CamParams[CAMERA_STRATEGY].m_CamRotSpeedX      = 0.0005f;
    m_CamParams[CAMERA_STRATEGY].m_CamRotSpeedZ      = 0.001f;
    m_CamParams[CAMERA_STRATEGY].m_CamRotAngleMin    = GRAD2RAD(60);
    m_CamParams[CAMERA_STRATEGY].m_CamRotAngleMax    = GRAD2RAD(20);
    m_CamParams[CAMERA_STRATEGY].m_CamDistMin        = 70;
    m_CamParams[CAMERA_STRATEGY].m_CamDistMax        = 250;
    m_CamParams[CAMERA_STRATEGY].m_CamAngleParam     = 0.4f;
    m_CamParams[CAMERA_STRATEGY].m_CamHeight         = 140.0f;

    m_CamParams[CAMERA_ARCADE].m_CamMouseWheelStep   = 0.05f;
    m_CamParams[CAMERA_ARCADE].m_CamRotSpeedX        = 0.0005f;
    m_CamParams[CAMERA_ARCADE].m_CamRotSpeedZ        = 0.001f;
    m_CamParams[CAMERA_ARCADE].m_CamRotAngleMin      = GRAD2RAD(60);
    m_CamParams[CAMERA_ARCADE].m_CamRotAngleMax      = GRAD2RAD(20);
    m_CamParams[CAMERA_ARCADE].m_CamDistMin          = 70;
    m_CamParams[CAMERA_ARCADE].m_CamDistMax          = 250;
    m_CamParams[CAMERA_ARCADE].m_CamAngleParam       = 0.0f;
    m_CamParams[CAMERA_ARCADE].m_CamHeight           = 40.0f;

    m_CamBaseAngleZ      = 0.0f; //GRAD2RAD(38.0f);
    m_CamMoveSpeed       = 1.05f;
    m_CamInRobotForward0 = 10.0f;
    m_CamInRobotForward1 = 30.0f;
   
    int i = 0;
    while(key_action_codes[i].name != nullptr)
    {
        m_KeyActions[key_action_codes[i].code] = key_action_codes[i].base_key;
        ++i;
    }
    
    m_CaptureTimeErase = 750;
    m_CaptureTimePaint = 500;
    m_CaptureTimeRolback = 1500;

    m_SkyBox = 1;
    m_DrawAllObjectsToMinimap = 2;
}


void CMatrixConfig::Clear(void)
{
DTRACE();

    if(m_Cursors)
    {
        for(int i = 0; i < m_CursorsCnt; ++i)
        {
            m_Cursors[i].key.~CWStr();
            m_Cursors[i].val.~CWStr();
        }
        
        HFree(m_Cursors, g_CacheHeap);
    }

    m_WeaponsConsts.clear();
    m_RobotWeaponsConsts.clear();
    m_RobotHullsConsts.clear();
    m_RobotChassisConsts.clear();
    m_RobotHeadsConsts.clear();
    m_TurretsConsts.clear();
}


void CMatrixConfig::ReadParams()
{
DTRACE();

    Clear();

    CBlockPar* cfg_par = g_MatrixData->BlockGet(BLOCK_PATH_MAIN_CONFIG);

    //loading config
    CBlockPar* bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_CURSORS);

    m_CursorsCnt = bp_tmp->ParCount();
    m_Cursors = (SStringPair*)HAlloc(sizeof(SStringPair) * m_CursorsCnt, g_CacheHeap);
    for(int i = 0; i < m_CursorsCnt; ++i)
    {
        new(&m_Cursors[i]) SStringPair();
        m_Cursors[i].key = bp_tmp->ParGetName(i);
        m_Cursors[i].val = bp_tmp->ParGet(i);
    }

    // top size
    //if(g_MatrixCfg->ParCount(CFG_TOP_SIZE)) m_TexTopMinSize = g_MatrixCfg->Par(CFG_TOP_SIZE).GetInt();

    //if(g_MatrixCfg->ParCount(CFG_TOP_SCALE)) m_TexTopDownScalefactor = g_MatrixCfg->Par(CFG_TOP_SCALE).GetInt();

    //if(m_TexTopMinSize < 32) m_TexTopMinSize = 32;

    // bottom size
    //if(g_MatrixCfg->ParCount(CFG_BOT_SIZE)) m_TexBotMinSize = g_MatrixCfg->Par(CFG_BOT_SIZE).GetInt();

    //if(g_MatrixCfg->ParCount(CFG_BOT_SCALE)) m_TexBotDownScalefactor = g_MatrixCfg->Par(CFG_BOT_SCALE).GetInt();

    //if(m_TexBotMinSize < 32) m_TexBotMinSize = 32;
    //if(m_TexBotMinSize < 16) m_TexBotMinSize = 16;

    //if(g_MatrixCfg->ParCount(CFG_LAND_TEXTURES_16)) m_LandTextures16 = g_MatrixCfg->Par(CFG_LAND_TEXTURES_16).GetInt() == 1;

    if(cfg_par->ParCount(CFG_SOFTWARE_CURSOR))
    {
        m_SoftwareCursor = cfg_par->Par(CFG_SOFTWARE_CURSOR).GetInt() == 1;
    }

    if(cfg_par->ParCount(CFG_GLOSS_LAND))
    {
        m_LandTexturesGloss = cfg_par->Par(CFG_GLOSS_LAND).GetInt() == 1;
    }

    if(cfg_par->ParCount(CFG_GLOSS_OBJECT))
    {
        m_ObjTexturesGloss = cfg_par->Par(CFG_GLOSS_OBJECT).GetInt() == 1;
    }
    /*
    if(g_MatrixCfg->ParCount(CFG_OBJECT_TEX_16))
    {
        m_ObjTextures16 = g_MatrixCfg->Par(CFG_OBJECT_TEX_16).GetInt() == 1;
    }
    */

    if(cfg_par->ParCount(CFG_IZVRAT_MS))
    {
        m_IzvratMS = cfg_par->Par(CFG_IZVRAT_MS).GetInt() == 1;
    }

    if(cfg_par->ParCount(CFG_SKY_BOX))
    {
        m_SkyBox = (byte)(cfg_par->Par(CFG_SKY_BOX).GetInt() & 0xFF);
    }

    if(cfg_par->ParCount(CFG_MAX_FPS))
    {
        g_MaxFPS = cfg_par->Par(CFG_MAX_FPS).GetInt();
    }

    if(cfg_par->ParCount(CFG_MAX_VIEW_DISTANCE))
    {
        //Дистанция дальности отрисовки мира вокруг камеры
        g_MaxViewDistance = cfg_par->Par(CFG_MAX_VIEW_DISTANCE).GetFloat();
    }

    if(cfg_par->ParCount(CFG_OBJECTS_PER_SCREEN))
    {
        //Самое максимальное значение 5120 (размер статического массива), устанавливается в константе MAX_OBJECTS_PER_SCREEN
        g_MaxObjectsPerScreen = cfg_par->Par(CFG_OBJECTS_PER_SCREEN).GetInt();
    }

    if(cfg_par->ParCount(CFG_EFFECTS_COUNT))
    {
        //Альтернативно можно регулировать через определение MAX_EFFECTS_COUNT (закомментирована)
        g_MaxEffectsCount = cfg_par->Par(CFG_EFFECTS_COUNT).GetInt();
    }

    if(cfg_par->ParCount(CFG_SHADOWS_DRAW_DISTANCE))
    {
        //Используется в определении DRAW_SHADOWS_DISTANCE_SQ
        g_ShadowsDrawDistance = cfg_par->Par(CFG_SHADOWS_DRAW_DISTANCE).GetInt();
    }

    if(cfg_par->ParCount(CFG_THIN_FOG_DRAW_DISTANCE))
    {
        //Точка удаления от камеры, в которой начинается отрисовка разреженного тумана
        g_ThinFogDrawDistance = cfg_par->Par(CFG_THIN_FOG_DRAW_DISTANCE).GetFloat();
    }

    if(cfg_par->ParCount(CFG_DENSE_FOG_DRAW_DISTANCE))
    {
        //Точка удаления от камеры, в которой начинается отрисовка сплошного тумана
        g_DenseFogDrawDistance = cfg_par->Par(CFG_DENSE_FOG_DRAW_DISTANCE).GetFloat();
    }

    if(cfg_par->ParCount(CFG_PLAYER_ROBOTS_AUTO_BOOM))
    {
        //Проверяем, включена ли опция автоматического подрыва бомбы на роботах игрока в случаях, когда их HP падает до нуля
        g_PlayerRobotsAutoBoom = max(abs(cfg_par->Par(CFG_PLAYER_ROBOTS_AUTO_BOOM).GetBool()), 1);
    }

    if(cfg_par->ParCount(CFG_ENABLE_FLYERS))
    {
        //Проверяем, включена ли опция активации вертолётов в качестве играбельного класса юнитов
        //(В данный момент вертолёты работают в тестовом режиме)
        g_EnableFlyers = cfg_par->Par(CFG_ENABLE_FLYERS).GetBool();
    }
        
    if(cfg_par->ParCount(CFG_OBJECTTOMINIMAP))
    {
        m_DrawAllObjectsToMinimap = (byte)(cfg_par->Par(CFG_OBJECTTOMINIMAP).GetInt() & 0xFF);
    }

    if(cfg_par->ParCount(CFG_DEBUG_INFO))
    {
        m_DIFlags = cfg_par->Par(CFG_DEBUG_INFO).GetHexUnsigned();
    }

    if(cfg_par->ParCount(CFG_VERTEX_LIGHT))
    {
        m_VertexLight = cfg_par->Par(CFG_VERTEX_LIGHT).GetBool();
    }

    if(cfg_par->BlockCount(CFG_GAMMA_RAMP))
    {
        CBlockPar* g = cfg_par->BlockGet(CFG_GAMMA_RAMP);
        m_GammaR.brightness = g->ParGet(L"R").GetFloatPar(0, L",");
        m_GammaR.contrast = g->ParGet(L"R").GetFloatPar(1, L",");
        m_GammaR.gamma = g->ParGet(L"R").GetFloatPar(2, L",");

        m_GammaG.brightness = g->ParGet(L"G").GetFloatPar(0, L",");
        m_GammaG.contrast = g->ParGet(L"G").GetFloatPar(1, L",");
        m_GammaG.gamma = g->ParGet(L"G").GetFloatPar(2, L",");

        m_GammaB.brightness = g->ParGet(L"B").GetFloatPar(0, L",");
        m_GammaB.contrast = g->ParGet(L"B").GetFloatPar(1, L",");
        m_GammaB.gamma = g->ParGet(L"B").GetFloatPar(2, L",");
    }

    ApplyGammaRamp();

    if(cfg_par->ParCount(CFG_ROBOT_SHADOW))
    {
        int sh = cfg_par->Par(CFG_ROBOT_SHADOW).GetInt();

        if(!sh) m_RobotShadow = SHADOW_OFF;
        else if(sh == 1) m_RobotShadow = SHADOW_STENCIL;
    }

    //Загружаем общий массив информации обо всём оружии и доп. эффектах
    bp_tmp = g_MatrixData->BlockPathGet(CONFIG_PATH_WEAPONS_DATA);
    int weapons_count = bp_tmp->BlockCount();
    for(int i = 0; i <= weapons_count; ++i)
    {
        //Пропускаем нулевой элемент
        if(!i)
        {
            SWeaponsConsts t;
            m_WeaponsConsts.push_back(t);
            continue;
        }

        CBlockPar* bp = bp_tmp->BlockGet(i - 1);
        SWeaponsConsts weapon_data;

        weapon_data.type_name = bp_tmp->BlockGetName(i - 1);
        if(!bp->ParGetNE(L"IsSecondaryEffect").GetBool()) //Разбираем основной эффект оружия
        {
            weapon_data.shot_range = bp->ParGetNE(L"ShotRange").GetFloat();
            float shots_per_second = bp->ParGetNE(L"ShotsPerSecond").GetFloat();
            weapon_data.shots_delay = shots_per_second > 0 ? 1000.0f / min(shots_per_second, 1000.0f) : 0.0f;

            const CWStr* effect_type = &bp->ParGetNE(L"EffectType");
            if(*effect_type == L"Cannon")
            {
                weapon_data.primary_effect = EFFECT_CANNON;

                weapon_data.projectile_model_path = bp->ParGet(L"ProjectileModelPath");
                weapon_data.projectile_full_velocity = max(bp->ParGet(L"ProjectileVelocity").GetFloat(), 0.1f);
                weapon_data.projectile_splash_radius = max(bp->ParGet(L"ProjectileSplashRadius").GetFloat(), 0.01f);

                CBlockPar* gun_flash = bp->BlockGetNE(L"GunFlash");
                if(gun_flash)
                {
                    const CWStr* sprites_str = &gun_flash->ParGetNE(L"SpritesInCache");
                    if(!sprites_str->IsEmpty())
                    {
                        SWeaponsConsts::SSpriteSet spr_set;
                        spr_set.sprites_name = sprites_str->GetStrPar(0, L":");
                        spr_set.sprites_count = sprites_str->GetIntPar(1, L":") > 1 ? sprites_str->GetIntPar(1, L":") : 1;
                        weapon_data.sprite_set.push_back(spr_set);

                        weapon_data.sprites_lenght = gun_flash->ParGetNE(L"SpritesLenght").GetInt();
                        weapon_data.sprites_width = gun_flash->ParGetNE(L"SpritesWidth").GetInt();
                    }
                    else weapon_data.sprite_set.push_back({ (CWStr)L"", { }, 0 }); //Для пустого названия номер спрайта SPR_NONE добавится позже, во время загрузки всех спрайтов в память

                    weapon_data.light_radius = gun_flash->ParGetNE(L"LightSpotSize").GetInt();
                    if(weapon_data.light_radius)
                    {
                        weapon_data.light_duration = int(1000.0f * gun_flash->ParGetNE(L"LightSpotDuration").GetFloat());

                        const CWStr* par = &gun_flash->ParGetNE(L"LightSpotColor");
                        if(par->IsEmpty()) weapon_data.hex_BGRA_light_color = 0xFFFFFFFF;
                        else weapon_data.hex_BGRA_light_color = RGBAStringToABGRColorHEX(par);
                    }
                }
                else weapon_data.sprite_set.push_back({ (CWStr)L"", { }, 0 }); //Для пустого названия номер спрайта SPR_NONE добавится позже, во время загрузки всех спрайтов в память

                CBlockPar* contrail = bp->BlockGetNE(L"Contrail");
                if(contrail)
                {
                    weapon_data.sprite_set.push_back({ contrail->ParGet(L"SpritesInCache"), { }, 1 }); //У инверсионного следа всегда только один спрайт
                    weapon_data.contrail_width = contrail->ParGet(L"SpritesWidth").GetInt();
                    weapon_data.contrail_duration = int(1000.0f * contrail->ParGet(L"SpritesDuration").GetFloat());

                    const CWStr* par = &contrail->ParGetNE(L"SpritesColor");
                    if(par->IsEmpty()) weapon_data.hex_BGRA_sprites_color = 0xFFFFFFFF;
                    else weapon_data.hex_BGRA_sprites_color = RGBAStringToABGRColorHEX(par);
                }
                else weapon_data.sprite_set.push_back({ (CWStr)L"", { }, 0 }); //Для пустого названия номер спрайта SPR_NONE добавится позже, во время загрузки всех спрайтов в память
            }
            else if(*effect_type == L"RocketLauncher")
            {
                weapon_data.primary_effect = EFFECT_ROCKET_LAUNCHER;

                weapon_data.projectile_model_path = bp->ParGet(L"MissileModelPath");
                weapon_data.projectile_start_velocity = max(bp->ParGetNE(L"MissileInitialVelocity").GetFloat(), 0.1f);
                weapon_data.projectile_full_velocity = max(bp->ParGet(L"MissileMarchingVelocity").GetFloat(), 0.1f);
                weapon_data.projectile_full_velocity_reach = max(int(1000.0f * bp->ParGetNE(L"MissileMarchingVelocityReachTime").GetFloat()), 0);
                weapon_data.projectile_acceleration_coef = max(bp->ParGet(L"MissileAccelerationCoef").GetFloat(), 0.0f);
                float angle = GRAD2RAD(min(max(bp->ParGetNE(L"MissileTargetCaptureAngle").GetFloat() / 2.0f, 0.0f), 180.0f));
                weapon_data.projectile_target_capture_angle_cos = cos(angle);
                weapon_data.projectile_target_capture_angle_sin = sin(angle);
                weapon_data.projectile_homing_speed = bp->ParGetNE(L"MissileHomingSpeed").GetFloat();
                weapon_data.projectile_splash_radius = max(bp->ParGet(L"MissileSplashRadius").GetFloat(), 0.01f);
                weapon_data.projectile_max_lifetime = max(int(1000.0f * bp->ParGetNE(L"MissileMaxLifetime").GetFloat()), 0);

                CBlockPar* gun_flash = bp->BlockGetNE(L"GunFlash");
                if(gun_flash)
                {
                    const CWStr* sprites_str = &gun_flash->ParGetNE(L"SpritesInCache");
                    if(!sprites_str->IsEmpty())
                    {
                        SWeaponsConsts::SSpriteSet spr_set;
                        spr_set.sprites_name = sprites_str->GetStrPar(0, L":");
                        spr_set.sprites_count = sprites_str->GetIntPar(1, L":") > 1 ? sprites_str->GetIntPar(1, L":") : 1;
                        weapon_data.sprite_set.push_back(spr_set);

                        weapon_data.sprites_lenght = gun_flash->ParGetNE(L"SpritesLenght").GetInt();
                        weapon_data.sprites_width = gun_flash->ParGetNE(L"SpritesWidth").GetInt();
                    }
                    else weapon_data.sprite_set.push_back({ (CWStr)L"", { }, 0 }); //Для пустого названия номер спрайта SPR_NONE добавится позже, во время загрузки всех спрайтов в память

                    weapon_data.light_radius = gun_flash->ParGetNE(L"LightSpotSize").GetInt();
                    if(weapon_data.light_radius)
                    {
                        weapon_data.light_duration = int(1000.0f * gun_flash->ParGetNE(L"LightSpotDuration").GetFloat());

                        const CWStr* par = &gun_flash->ParGetNE(L"LightSpotColor");
                        if(par->IsEmpty()) weapon_data.hex_BGRA_light_color = 0xFFFFFFFF;
                        else weapon_data.hex_BGRA_light_color = RGBAStringToABGRColorHEX(par);
                    }
                }
                else weapon_data.sprite_set.push_back({ (CWStr)L"", { }, 0 }); //Для пустого названия номер спрайта SPR_NONE добавится позже, во время загрузки всех спрайтов в память

                CBlockPar* contrail = bp->BlockGetNE(L"Contrail");
                if(contrail)
                {
                    weapon_data.contrail_duration = 1; //В данном случае служит маркером необходимости отрисовки следа
                    weapon_data.contrail_fire_effect_starts = max(int(1000.0f * contrail->ParGet(L"FireEffectStartsAt").GetFloat()), 0);

                    const CWStr* rgb_string = &contrail->ParGet(L"CloseToHearthColor");
                    for(int j = 0; j < 3 /* rgb_string.GetCountPar(L",") */; ++j)
                    {
                        if(!j) weapon_data.close_color_rgb.red = rgb_string->GetFloatPar(j, L",") / 255.0f;
                        else if(j == 1) weapon_data.close_color_rgb.green = rgb_string->GetFloatPar(j, L",") / 255.0f;
                        else weapon_data.close_color_rgb.blue = rgb_string->GetFloatPar(j, L",") / 255.0f;
                    }

                    rgb_string = &contrail->ParGet(L"FarFromHearthColor");
                    for(int j = 0; j < 3 /* rgb_string.GetCountPar(L",") */; ++j)
                    {
                        if(!j) weapon_data.far_color_rgb.red = rgb_string->GetFloatPar(j, L",") / 255.0f;
                        else if(j == 1) weapon_data.far_color_rgb.green = rgb_string->GetFloatPar(j, L",") / 255.0f;
                        else weapon_data.far_color_rgb.blue = rgb_string->GetFloatPar(j, L",") / 255.0f;
                    }
                }
            }
            else if(*effect_type == L"Mortar")
            {
                weapon_data.primary_effect = EFFECT_MORTAR;
            }
            else if(*effect_type == L"Repairer")
            {
                weapon_data.primary_effect = EFFECT_REPAIRER;
                weapon_data.is_repairer = true;

                const CWStr* sprites_str = &bp->ParGetNE(L"SpritesInCache");
                if(!sprites_str->IsEmpty())
                {
                    for(int i = 0; i < sprites_str->GetCountPar(L","); ++i)
                    {
                        SWeaponsConsts::SSpriteSet spr_set;
                        spr_set.sprites_name = sprites_str->GetStrPar(i, L",");
                        spr_set.sprites_count = 1;

                        weapon_data.sprite_set.push_back(spr_set);
                    }
                }
            }
            else if(*effect_type == L"Bomb")
            {
                weapon_data.primary_effect = EFFECT_BOMB;
                weapon_data.is_bomb = true;
            }
            //Пока что не разобранные все прочие первичные эффекты
            else
            {
                const CWStr* sprites_str = &bp->ParGetNE(L"SpritesInCache");
                if(!sprites_str->IsEmpty())
                {
                    for(int i = 0; i < sprites_str->GetCountPar(L","); ++i)
                    {
                        SWeaponsConsts::SSpriteSet spr_set;
                        spr_set.sprites_name = sprites_str->GetStrPar(i, L",");
                        spr_set.sprites_count = 1;

                        weapon_data.sprite_set.push_back(spr_set);
                    }
                }
            }
        }
        else //Разбираем второстепенный эффект, накладываемый другими эффектами оружия
        {
            ++m_WeaponsConsts[0].secondary_effect; //Запоминаем общее число второстепенных эффектов

            //Определяем основной тип данного эффекта
            const CWStr* effect_type = &bp->ParGetNE(L"EffectType");
            if(!effect_type->IsEmpty())
            {
                if(*effect_type == L"Ablaze") weapon_data.secondary_effect = SECONDARY_EFFECT_ABLAZE;
                else if(*effect_type == L"ShortedOut") weapon_data.secondary_effect = SECONDARY_EFFECT_SHORTED_OUT;
            }
            //else ERROR_S2(L"Effect type is undefined for block: ", weapon_data.type_name);

            weapon_data.effect_priority = max(bp->ParGetNE(L"EffectPriority").GetInt(), 0);

            if(weapon_data.secondary_effect == SECONDARY_EFFECT_ABLAZE)
            {
                CBlockPar* particle_flame = bp->BlockGet(L"ParticleFlame");
                if(particle_flame)
                {
                    const CWStr* rgb_string = &particle_flame->ParGet(L"CloseToHearthColor");
                    for(int j = 0; j < 3 /* rgb_string.GetCountPar(L",") */; ++j)
                    {
                        if(!j) weapon_data.close_color_rgb.red = rgb_string->GetFloatPar(j, L",") / 255.0f;
                        else if(j == 1) weapon_data.close_color_rgb.green = rgb_string->GetFloatPar(j, L",") / 255.0f;
                        else weapon_data.close_color_rgb.blue = rgb_string->GetFloatPar(j, L",") / 255.0f;
                    }

                    rgb_string = &particle_flame->ParGet(L"FarFromHearthColor");
                    for(int j = 0; j < 3 /* rgb_string.GetCountPar(L",") */; ++j)
                    {
                        if(!j) weapon_data.far_color_rgb.red = rgb_string->GetFloatPar(j, L",") / 255.0f;
                        else if(j == 1) weapon_data.far_color_rgb.green = rgb_string->GetFloatPar(j, L",") / 255.0f;
                        else weapon_data.far_color_rgb.blue = rgb_string->GetFloatPar(j, L",") / 255.0f;
                    }
                }
                //else ERROR_S2(L"Block ParticleFlame for SECONDARY_EFFECT_ABLAZE effect not found for: ", weapon_data.type_name);

                CBlockPar* anim_flame = bp->BlockGet(L"AnimatedFlame");
                if(anim_flame)
                {
                    const CWStr* sprites = &anim_flame->ParGet(L"SpritesInCache");

                    SWeaponsConsts::SSpriteSet spr_set;
                    spr_set.sprites_name = sprites->GetStrPar(0, L":");
                    if(sprites->GetCountPar(L":") > 1) spr_set.sprites_count = min(sprites->GetIntPar(1, L":"), 255);
                    else spr_set.sprites_count = 1;
                    weapon_data.sprite_set.push_back(spr_set);
                }
                //else ERROR_S2(L"Block AnimatedFlame for SECONDARY_EFFECT_ABLAZE effect not found for: ", weapon_data.type_name);
            }
            else if(weapon_data.secondary_effect == SECONDARY_EFFECT_SHORTED_OUT)
            {
                CBlockPar* stun_anim = bp->BlockGet(L"StunAnimation");
                if(stun_anim)
                {
                    const CWStr* par = &stun_anim->ParGet(L"SpritesInCache");

                    SWeaponsConsts::SSpriteSet spr_set;
                    spr_set.sprites_name = par->GetStrPar(0, L":");
                    //if(par.GetCountPar(L":") > 1) spr_set.sprites_count = min(par.GetIntPar(1, L":"), 255);
                    //else
                    spr_set.sprites_count = 1;
                    weapon_data.sprite_set.push_back(spr_set);

                    par = &stun_anim->ParGetNE(L"SpritesColor");
                    if(par->IsEmpty()) weapon_data.hex_BGRA_sprites_color = 0xFFFFFFFF;
                    else weapon_data.hex_BGRA_sprites_color = RGBAStringToABGRColorHEX(par);
                }
                //else ERROR_S2(L"Block StunAnimation for SECONDARY_EFFECT_SHORTED_OUT effect not found for: ", weapon_data.type_name);
            }
        }

        CBlockPar* map_obj_ign = bp->BlockGetNE(L"MapObjectsIgnition");
        if(map_obj_ign)
        {
            weapon_data.map_objects_ignition.is_present = true;

            weapon_data.map_objects_ignition.priority = max(map_obj_ign->ParGetNE(L"Priority").GetInt(), 0);

            weapon_data.map_objects_ignition.duration_per_hit = int(1000.0f * map_obj_ign->ParGet(L"DurationPerHit").GetFloat());
            weapon_data.map_objects_ignition.burning_starts_at = int(1000.0f * map_obj_ign->ParGet(L"BurningStartsAt").GetFloat());
            weapon_data.map_objects_ignition.max_duration = int(1000.0f * map_obj_ign->ParGet(L"MaxDuration").GetFloat());

            const CWStr* sprites = &map_obj_ign->ParGet(L"AnimationSprites");
            weapon_data.map_objects_ignition.sprites_name = sprites->GetStrPar(0, L":");
            if(sprites->GetCountPar(L":") > 1) weapon_data.map_objects_ignition.sprites_count = min(sprites->GetIntPar(1, L":"), 255);
            else weapon_data.map_objects_ignition.sprites_count = 1;

            weapon_data.map_objects_ignition.burning_sound_name = map_obj_ign->ParGetNE(L"BurningSound");
        }

        weapon_data.shot_sound_name = bp->ParGetNE(L"ShotSound");
        weapon_data.shot_sound_looped = bp->ParGetNE(L"ShotSoundLooped").GetBool();
        weapon_data.hit_sound_name = bp->ParGetNE(L"HitSound");
        //weapon_data.hit_sound_looped = bp->ParGetNE(L"HitSoundLooped").GetBool(); //Бесконечные страдания?
        weapon_data.explosive_hit = bp->ParGetNE(L"ExplosiveHit").GetBool();

        CBlockPar* damage = bp->BlockGetNE(L"Damage");
        if(damage)
        {
            const CWStr* par = &damage->ParGetNE(L"Robots");
            weapon_data.damage.to_robots = par->GetStrPar(0, L"until").GetFloat() * 10.0f;
            weapon_data.non_lethal_threshold.to_robots = par->GetCountPar(L"until") > 1 ? par->GetStrPar(1, L"until").GetFloat() * 10.0f : 0.0f;
            par = &damage->ParGetNE(L"Helicopters");
            weapon_data.damage.to_flyers = par->GetStrPar(0, L"until").GetFloat() * 10.0f;
            weapon_data.non_lethal_threshold.to_flyers = par->GetCountPar(L"until") > 1 ? par->GetStrPar(1, L"until").GetFloat() * 10.0f : 0.0f;
            par = &damage->ParGetNE(L"Turrets");
            weapon_data.damage.to_turrets = par->GetStrPar(0, L"until").GetFloat() * 10.0f;
            weapon_data.non_lethal_threshold.to_turrets = par->GetCountPar(L"until") > 1 ? par->GetStrPar(1, L"until").GetFloat() * 10.0f : 0.0f;
            par = &damage->ParGetNE(L"Buildings");
            weapon_data.damage.to_buildings = par->GetStrPar(0, L"until").GetFloat() * 10.0f;
            weapon_data.non_lethal_threshold.to_buildings = par->GetCountPar(L"until") > 1 ? par->GetStrPar(1, L"until").GetFloat() * 10.0f : 0.0f;
            par = &damage->ParGetNE(L"MapObjects");
            weapon_data.damage.to_objects = par->GetStrPar(0, L"until").GetFloat() * 10.0f;
            weapon_data.non_lethal_threshold.to_objects = par->GetCountPar(L"until") > 1 ? par->GetStrPar(1, L"until").GetFloat() * 10.0f : 0.0f;
        }

        m_WeaponsConsts.push_back(weapon_data);
        //m_WeaponsConsts.insert(m_WeaponsConsts.begin() + i, weapon_data);
    }

    //Сопоставляем оружие и дополнительные эффекты, которое оно накладывает
    for(int i = 1; i <= weapons_count; ++i)
    {
        CBlockPar* bp = bp_tmp->BlockGet(i - 1);
        if(m_WeaponsConsts[i].secondary_effect) continue;

        //Загружаем дополнительные накладываемые оружием эффекты
        for(int j = 0; j < bp->BlockCount(); ++j)
        {
            CWStr block_name = bp->BlockGetName(j);
            if(block_name.Find(L"Effect") == 0) //Если первые 6 символов равны Effect
            {
                CBlockPar* effect_block = bp->BlockGet(j);
                SWeaponsConsts::SWeaponExtraEffect effect;

                effect.type = WeapName2Weap(block_name.Del(0, 6)); //Удаляем из строки начальный Effect
                byte effect_type = m_WeaponsConsts[effect.type].secondary_effect;

                if(effect_type == SECONDARY_EFFECT_ABLAZE)
                {
                    effect.duration_per_hit = int(1000.0f * effect_block->ParGet(L"DurationPerHit").GetFloat());
                    effect.max_duration = int(1000.0f * effect_block->ParGet(L"MaxDuration").GetFloat());
                }
                else if(effect_type == SECONDARY_EFFECT_SHORTED_OUT)
                {
                    effect.duration_per_hit = int(1000.0f * effect_block->ParGet(L"DurationPerHit").GetFloat());
                    effect.max_duration = int(1000.0f * effect_block->ParGet(L"MaxDuration").GetFloat());
                }

                m_WeaponsConsts[i].extra_effects.push_back(effect);
            }
        }
    }

    //Загружаем информацию о модулях роботов:
    //Оружие (перебираем раньше всех, чтобы уже иметь готовый массив пушек к началу заполнения корпусов)
    bp_tmp = g_MatrixData->BlockPathGet(MODULES_CONFIG_PATH_WEAPONS);
    m_RobotWeaponsCount = bp_tmp->ParGet(L"WeaponsCount").GetInt();
    for(int i = 0; i <= m_RobotWeaponsCount; ++i)
    {
        SRobotWeaponsConsts t;
        m_RobotWeaponsConsts.push_back(t);
        if(!i)
        {
            m_RobotWeaponsConsts[0].name = bp_tmp->ParGet(L"TypeLabel"); //Название типа "оружие:"
            m_RobotWeaponsConsts[0].for_player_side = 0; //Инициализация счётчика
            continue;
        }

        CBlockPar* bp = bp_tmp->BlockGet(CWStr(i));

        if(bp->ParGetNE(L"NotForPlayer").GetBool()) m_RobotWeaponsConsts[i].for_player_side = 0; //Блокирует данное оружие от выбора в конструкторе
        else
        {
            m_RobotWeaponsConsts[i].for_player_side = 1;
            ++m_RobotWeaponsConsts[0].for_player_side;
        }

        m_RobotWeaponsConsts[i].name = bp->ParGet(L"Name");

        m_RobotWeaponsConsts[i].short_designation = bp->ParGet(L"ShortDesignation");

        m_RobotWeaponsConsts[i].chars_description = bp->ParGet(L"CharsDescription");
        m_RobotWeaponsConsts[i].art_description = bp->ParGet(L"ArtDescription");

        m_RobotWeaponsConsts[i].constructor_button_name = bp->ParGet(L"ConstructorButton");
        m_RobotWeaponsConsts[i].constructor_image_name = bp->ParGet(L"ConstructorImage");
        m_RobotWeaponsConsts[i].constructor_label_name = bp->ParGet(L"ConstructorLabel");
        m_RobotWeaponsConsts[i].status_panel_image = bp->ParGet(L"StatusPanelImage");
        m_RobotWeaponsConsts[i].model_path = bp->ParGet(L"ModelPathInCache");

        m_RobotWeaponsConsts[i].strength = bp->ParGet(L"Strength").GetFloat();

        m_RobotWeaponsConsts[i].weapon_type = WeapName2Weap(bp->ParGet(L"WeaponType"));

        //Дублируем эти маркеры в инфу о модуле, чтобы ускорить некоторые проверки в коде
        m_RobotWeaponsConsts[i].is_module_bomb = m_WeaponsConsts[m_RobotWeaponsConsts[i].weapon_type].is_bomb;
        m_RobotWeaponsConsts[i].is_module_repairer = m_WeaponsConsts[m_RobotWeaponsConsts[i].weapon_type].is_repairer;

        if(m_RobotWeaponsConsts[i].is_module_repairer)
        {
            m_RobotWeaponsConsts[i].dev_start_id = bp->ParGet(L"RepairerDeviceStart_MatrixId").GetInt();
            m_RobotWeaponsConsts[i].dev_end_id = bp->ParGet(L"RepairerDeviceEnd_MatrixId").GetInt();
        }

        CBlockPar* overheat = bp->BlockGetNE(L"Overheat");
        if(overheat)
        {
            m_RobotWeaponsConsts[i].heating_speed = Float2Int(min(max(overheat->ParGetNE(L"HeatingPercentPerShot").GetFloat() * 10, 0.0f), 1000.0f));
            m_RobotWeaponsConsts[i].cooling_speed = Float2Int(min(max(overheat->ParGetNE(L"CoolingPercentAfterDelay").GetFloat() * 10, 0.0f), 1000.0f));
            m_RobotWeaponsConsts[i].cooling_delay = Float2Int(max(overheat->ParGetNE(L"CoolingDelay").GetFloat() * 1000.0f, 0.0f));
        }
        else
        {
            m_RobotWeaponsConsts[i].heating_speed = 0;
            m_RobotWeaponsConsts[i].cooling_speed = 0;
            m_RobotWeaponsConsts[i].cooling_delay = 0;
        }

        m_RobotWeaponsConsts[i].module_hit_effect = WeapName2Weap(bp->ParNE(L"ModuleHitEffect"));

        //Цена в ресурсах
        m_RobotWeaponsConsts[i].cost_titan = bp->BlockGet(L"Cost")->ParNE(L"Titan").GetInt();
        m_RobotWeaponsConsts[i].cost_electronics = bp->BlockGet(L"Cost")->ParNE(L"Electronics").GetInt();
        m_RobotWeaponsConsts[i].cost_energy = bp->BlockGet(L"Cost")->ParNE(L"Energy").GetInt();
        m_RobotWeaponsConsts[i].cost_plasma = bp->BlockGet(L"Cost")->ParNE(L"Plasma").GetInt();

        //В процессе заполнения сортируем массив так, чтобы доступные игроку пушки оказались первыми в списке
        //(добавил это, т.к. не разобрался в уебанском алгоритме заполнения раскрываемых менюшек в конструкторе), upd: разобрался
        //if(m_RobotWeaponsConsts[i].for_player_side) std::swap(m_RobotWeaponsConsts[i], m_WeaponsConsts[m_RobotWeaponsConsts[0].for_player_side]);
    }

    //Корпуса
    bp_tmp = g_MatrixData->BlockPathGet(MODULES_CONFIG_PATH_HULLS);
    m_RobotHullsCount = bp_tmp->ParGet(L"HullsCount").GetInt();
    for(int i = 0; i <= m_RobotHullsCount; ++i)
    {
        SRobotHullsConsts t;
        m_RobotHullsConsts.push_back(t);

        if(!i)
        {
            m_RobotHullsConsts[0].name = bp_tmp->ParGet(L"TypeLabel"); //Название типа "корпус:"
            m_RobotHullsConsts[0].for_player_side = 0; //Инициализация счётчика
            continue;
        }

        CBlockPar* bp = bp_tmp->BlockGet(CWStr(i));
        if(bp->ParGetNE(L"NotForPlayer").GetBool()) m_RobotHullsConsts[i].for_player_side = 0; //Блокирует данный модуль от выбора в конструкторе
        else
        {
            m_RobotHullsConsts[i].for_player_side = 1;
            ++m_RobotHullsConsts[0].for_player_side;
        }

        m_RobotHullsConsts[i].name = bp->ParGet(L"Name");
        m_RobotHullsConsts[i].robot_part_name = bp->ParGet(L"RobotPartName");
        m_RobotHullsConsts[i].short_designation = bp->ParGet(L"ShortDesignation");

        m_RobotHullsConsts[i].chars_description = bp->ParGet(L"CharsDescription");
        m_RobotHullsConsts[i].art_description = bp->ParGet(L"ArtDescription");

        //Заполняем информацию о пилонах под оружие
        CBlockPar* weapons_bp = bp->BlockPathGet(L"Characteristics.WeaponPylons");
        m_RobotHullsConsts[i].constructor_weapon_slots_used.reset(); //Инициализация
        int j = 1;
        wchar_t buf[32];
        swprintf(buf, 32, L"Pylon%i_MatrixId", j);
        int weapon_matrix = weapons_bp->ParGetNE(buf).GetInt();

        if(!weapon_matrix) //Если слотов под оружие, по каким-то причинам, вообще нет - всё равно добавляем один элемент для порядку
        {
            SRobotHullsWeaponPylonData weap_data;
            ZeroMemory(&weap_data, sizeof(SRobotHullsWeaponPylonData));
            m_RobotHullsConsts[i].weapon_pylon_data.push_back(weap_data);
        }
        else while(weapon_matrix)
        {
            SRobotHullsWeaponPylonData weap_data;
            weap_data.id = weapon_matrix;
            swprintf(buf, 32, L"Pylon%i_InvertedModel", j);
            weap_data.inverted_model = weapons_bp->ParGetNE(buf).GetBool();
            swprintf(buf, 32, L"Pylon%i_FitWeapon", j);
            const CWStr* fit_weapon = &weapons_bp->ParGetNE(buf);
            if(!fit_weapon->IsEmpty())
            {
                weap_data.fit_weapon.reset(); //Инициализация

                for(int k = 0; k < fit_weapon->GetCountPar(L","); ++k)
                {
                    CWStr* short_des = &fit_weapon->GetStrPar(k, L",").Trim();
                    for(int p = 1; p <= ROBOT_WEAPONS_COUNT; ++p)
                    {
                        //Выставляем биты в true под номерами всех подходящих для данного пилона пушек
                        if(*short_des == m_RobotWeaponsConsts[p].short_designation)
                        {
                            weap_data.fit_weapon.set(p);
                            break;
                        }
                    }
                }
            }
            else weap_data.fit_weapon.set(); //По умолчанию подходящим считается любое оружие

            swprintf(buf, 32, L"Pylon%i_ConstructorSlot", j);
            weap_data.constructor_slot_num = weapons_bp->ParGet(buf).GetInt() - 1; //Какой слот конструктора должен соответствовать этому пилону
            m_RobotHullsConsts[i].constructor_weapon_slots_used.set(weap_data.constructor_slot_num);

            m_RobotHullsConsts[i].weapon_pylon_data.push_back(weap_data);

            swprintf(buf, 32, L"Pylon%i_MatrixId", ++j);
            weapon_matrix = weapons_bp->ParGetNE(buf).GetInt();
        }

        m_RobotHullsConsts[i].constructor_button_name = bp->ParGet(L"ConstructorButton");
        m_RobotHullsConsts[i].constructor_image_name = bp->ParGet(L"ConstructorImage");
        m_RobotHullsConsts[i].constructor_label_name = bp->ParGet(L"ConstructorLabel");
        m_RobotHullsConsts[i].hull_sound_name = bp->ParGet(L"MainSound");
        m_RobotHullsConsts[i].model_path = bp->ParGet(L"ModelPathInCache");

        m_RobotHullsConsts[i].structure = max(10.0f * bp->BlockGet(L"Characteristics")->ParGet(L"Structure").GetFloat(), 10.0f);
        m_RobotHullsConsts[i].rotation_speed = bp->BlockGet(L"Characteristics")->ParGet(L"RotationSpeed").GetFloat();

        m_RobotHullsConsts[i].module_hit_effect = WeapName2Weap(bp->ParNE(L"ModuleHitEffect"));

        //Цена в ресурсах
        m_RobotHullsConsts[i].cost_titan = bp->BlockGet(L"Cost")->ParNE(L"Titan").GetInt();
        m_RobotHullsConsts[i].cost_electronics = bp->BlockGet(L"Cost")->ParNE(L"Electronics").GetInt();
        m_RobotHullsConsts[i].cost_energy = bp->BlockGet(L"Cost")->ParNE(L"Energy").GetInt();
        m_RobotHullsConsts[i].cost_plasma = bp->BlockGet(L"Cost")->ParNE(L"Plasma").GetInt();

        //В процессе заполнения сортируем массив так, чтобы доступные игроку модули оказались первыми в списке
        //(добавил это, т.к. не разобрался в уебанском алгоритме заполнения раскрываемых менюшек в конструкторе), upd: разобрался
        //if(m_RobotHullsConsts[i].for_player_side) std::swap(m_RobotHullsConsts[i], m_RobotHullsConsts[m_RobotHullsConsts[0].for_player_side]);
    }

    //Шасси
    bp_tmp = g_MatrixData->BlockPathGet(MODULES_CONFIG_PATH_CHASSIS);
    m_RobotChassisCount = bp_tmp->ParGet(L"ChassisCount").GetInt();
    for(int i = 0; i <= m_RobotChassisCount; ++i)
    {
        SRobotChassisConsts t;
        m_RobotChassisConsts.push_back(t);
        if(!i)
        {
            m_RobotChassisConsts[0].name = bp_tmp->ParGet(L"TypeLabel"); //Название типа "шасси:"
            m_RobotChassisConsts[0].for_player_side = 0; //Инициализация счётчика
            continue;
        }

        CBlockPar* bp = bp_tmp->BlockGet(CWStr(i));

        if(bp->ParGetNE(L"NotForPlayer").GetBool()) m_RobotChassisConsts[i].for_player_side = 0; //Блокирует данный модуль от выбора в конструкторе
        else
        {
            m_RobotChassisConsts[i].for_player_side = 1;
            ++m_RobotChassisConsts[0].for_player_side;
        }

        m_RobotChassisConsts[i].name = bp->ParGet(L"Name");
        m_RobotChassisConsts[i].robot_part_name = bp->ParGet(L"RobotPartName");
        m_RobotChassisConsts[i].short_designation = bp->ParGet(L"ShortDesignation");

        m_RobotChassisConsts[i].chars_description = bp->ParGet(L"CharsDescription");
        m_RobotChassisConsts[i].art_description = bp->ParGet(L"ArtDescription");

        m_RobotChassisConsts[i].constructor_button_name = bp->ParGet(L"ConstructorButton");
        m_RobotChassisConsts[i].constructor_image_name = bp->ParGet(L"ConstructorImage");
        m_RobotChassisConsts[i].constructor_label_name = bp->ParGet(L"ConstructorLabel");
        m_RobotChassisConsts[i].arcade_enter_sound_name = bp->ParGet(L"ArcadeEnterSound");
        //Считываем из конфига вариации для звуков выдвижения робота (скобки, чтобы не сохранять временные переменные)
        {
            int j = 1;
            wchar_t buf[32];
            swprintf(buf, 32, L"MoveOutSound%i", j);
            const CWStr* move_sound = &bp->ParGetNE(buf);
            while(!move_sound->IsEmpty())
            {
                m_RobotChassisConsts[i].move_out_sound_name.push_back(*move_sound);

                swprintf(buf, 32, L"MoveOutSound%i", ++j);
                move_sound = &bp->ParGetNE(buf);
            }
        }

        m_RobotChassisConsts[i].model_path = bp->ParGet(L"ModelPathInCache");

        CBlockPar* chars = bp->BlockGet(L"Characteristics");
        m_RobotChassisConsts[i].structure = max(10.0f * chars->ParGet(L"Structure").GetFloat(), 10.0f);
        m_RobotChassisConsts[i].move_speed = chars->ParGet(L"MoveSpeed").GetFloat();
        const CWStr* strafe_speed = &chars->ParGetNE(L"StrafeSpeed");
        if(!strafe_speed->IsEmpty()) m_RobotChassisConsts[i].strafe_speed = strafe_speed->GetFloat();
        else m_RobotChassisConsts[i].strafe_speed = m_RobotChassisConsts[i].move_speed;
        m_RobotChassisConsts[i].move_uphill_factor = chars->ParGet(L"MoveUphillFactor").GetFloat();
        m_RobotChassisConsts[i].move_downhill_factor = chars->ParGet(L"MoveDownhillFactor").GetFloat();
        m_RobotChassisConsts[i].move_in_water_factor = chars->ParGet(L"MoveInWaterFactor").GetFloat();
        m_RobotChassisConsts[i].rotation_speed = chars->ParGet(L"RotationSpeed").GetFloat();

        m_RobotChassisConsts[i].anim_move_speed = 1.0f;
        m_RobotChassisConsts[i].anim_move_speed = chars->ParGetNE(L"AnimationSpeed").GetFloat();

        const CWStr* passability = &chars->ParGetNE(L"Passability");
        if(*passability == L"Worst") m_RobotChassisConsts[i].passability = 0;
        else if(*passability == L"Bad") m_RobotChassisConsts[i].passability = 1;
        else if(*passability == L"Average") m_RobotChassisConsts[i].passability = 2;
        else if(*passability == L"Good") m_RobotChassisConsts[i].passability = 3;
        else if(*passability == L"Best") m_RobotChassisConsts[i].passability = 4;
        else if(*passability == L"Cheat") m_RobotChassisConsts[i].passability = 5; //Полностью отключает коллизию с любыми объектами
        else m_RobotChassisConsts[i].passability = 0;

        //Разделение на отдельные маркеры позволяет сделать, например, ходячего робота, который может ходить по воде
        //Однако ввиду бесполезности данного действия, непосредственно из конфига этого сделать сейчас нельзя
        m_RobotChassisConsts[i].is_walking = false;
        m_RobotChassisConsts[i].is_rolling = false;
        m_RobotChassisConsts[i].is_hovering = false;
        const CWStr* chassis_type = &chars->ParGetNE(L"ChassisType");
        if(*chassis_type == L"Walking") m_RobotChassisConsts[i].is_walking = true;
        else if(*chassis_type == L"Rolling") m_RobotChassisConsts[i].is_rolling = true;
        else if(*chassis_type == L"Hovering") m_RobotChassisConsts[i].is_hovering = true;
        else m_RobotChassisConsts[i].is_walking = true;

        if(m_RobotChassisConsts[i].is_walking || m_RobotChassisConsts[i].is_rolling)
        {
            CBlockPar* block_ground_trace = chars->BlockGetNE(L"GroundTrace");
            if(block_ground_trace)
            {
                m_RobotChassisConsts[i].ground_trace.texture_path = block_ground_trace->ParGet(L"TexturePath");
                m_RobotChassisConsts[i].ground_trace.texture_scale = block_ground_trace->ParGetNE(L"TextureScale").GetFloat();

                m_RobotChassisConsts[i].ground_trace.trace_redraw_distance = block_ground_trace->ParGetNE(L"TraceRedrawDistance").GetInt();
                m_RobotChassisConsts[i].ground_trace.trace_duration = int(block_ground_trace->ParGet(L"TraceDuration").GetFloat() * 1000); //Конвертируем секунды в такты

                //Для шагающих шасси также запоминаем матрицы, к которым будут линковаться следы шагов
                if(m_RobotChassisConsts[i].is_walking)
                {
                    int j = 1;
                    wchar_t buf[32];
                    swprintf(buf, 32, L"Foot%i_MatrixId", j);
                    int foot_matrix = chars->ParGetNE(buf).GetInt();
                    while(foot_matrix)
                    {
                        SRobotChassisTraceMatrix trace_matrix;
                        trace_matrix.matrix_id = foot_matrix;
                        m_RobotChassisConsts[i].trace_matrix.push_back(trace_matrix);

                        swprintf(buf, 32, L"Foot%i_MatrixId", ++j);
                        foot_matrix = chars->ParGetNE(buf).GetInt();
                    }
                }
            }
        }

        m_RobotChassisConsts[i].is_dust_trace = chars->ParGetNE(L"IsDustTrace").GetBool();
        m_RobotChassisConsts[i].speed_dependent_anim = chars->ParGetNE(L"SpeedDependentAnimation").GetBool();

        //Для шасси, которым нужно пририсовать реактивные следы (как на "Антигравах"), загружаем дополнительный блок
        CBlockPar* block_jet_streams = chars->BlockGetNE(L"JetStreams");
        if(block_jet_streams)
        {
            int j = 1;
            wchar_t buf[32];
            swprintf(buf, 32, L"Stream%i_MatrixId", j);
            int jet_matrix = block_jet_streams->ParGetNE(buf).GetInt();
            while(jet_matrix)
            {
                SRobotChassisJetStream jet_stream;
                jet_stream.matrix_id = jet_matrix;
                swprintf(buf, 32, L"Stream%i_Sprites", j);
                const CWStr* sprites = &block_jet_streams->ParGet(buf);
                jet_stream.sprites_name = sprites->GetStrPar(0, L":");
                if(sprites->GetCountPar(L":") > 1) jet_stream.sprites_count = min(sprites->GetIntPar(1, L":"), 255);
                else jet_stream.sprites_count = 1;
                swprintf(buf, 32, L"Stream%i_Length", j);
                jet_stream.length = block_jet_streams->ParGet(buf).GetFloat();
                m_RobotChassisConsts[i].jet_stream.push_back(jet_stream);

                swprintf(buf, 32, L"Stream%i_MatrixId", ++j);
                jet_matrix = block_jet_streams->ParGetNE(buf).GetInt();
            }
        }

        m_RobotChassisConsts[i].module_hit_effect = WeapName2Weap(bp->ParNE(L"ModuleHitEffect"));

        //Цена в ресурсах
        m_RobotChassisConsts[i].cost_titan = bp->BlockGet(L"Cost")->ParNE(L"Titan").GetInt();
        m_RobotChassisConsts[i].cost_electronics = bp->BlockGet(L"Cost")->ParNE(L"Electronics").GetInt();
        m_RobotChassisConsts[i].cost_energy = bp->BlockGet(L"Cost")->ParNE(L"Energy").GetInt();
        m_RobotChassisConsts[i].cost_plasma = bp->BlockGet(L"Cost")->ParNE(L"Plasma").GetInt();

        //В процессе заполнения сортируем массив так, чтобы доступные игроку модули оказались первыми в списке
        //(добавил это, т.к. не разобрался в уебанском алгоритме заполнения раскрываемых менюшек в конструкторе), upd: разобрался
        //if(m_RobotChassisConsts[i].for_player_side) std::swap(m_RobotChassisConsts[i], m_RobotChassisConsts[m_RobotChassisConsts[0].for_player_side]);
    }

    //Головы
    bp_tmp = g_MatrixData->BlockPathGet(MODULES_CONFIG_PATH_HEADS);
    m_RobotHeadsCount = bp_tmp->ParGet(L"HeadsCount").GetInt();
    for(int i = 0; i <= m_RobotHeadsCount; ++i)
    {
        SRobotHeadsConsts t;
        m_RobotHeadsConsts.push_back(t);
        if(!i)
        {
            m_RobotHeadsConsts[0].name = bp_tmp->ParGet(L"TypeLabel"); //Название типа "модуль:"
            m_RobotHeadsConsts[0].for_player_side = 0; //Инициализация счётчика
            continue;
        }

        CBlockPar* bp = bp_tmp->BlockGet(CWStr(i));

        if(bp->ParGetNE(L"NotForPlayer").GetBool()) m_RobotHeadsConsts[i].for_player_side = 0; //Блокирует данный модуль от выбора в конструкторе
        else
        {
            m_RobotHeadsConsts[i].for_player_side = 1;
            ++m_RobotHeadsConsts[0].for_player_side;
        }

        m_RobotHeadsConsts[i].name = bp->ParGet(L"Name");
        m_RobotHeadsConsts[i].robot_part_name = bp->ParGet(L"RobotPartName");
        m_RobotHeadsConsts[i].short_designation = bp->ParGet(L"ShortDesignation");
        
        m_RobotHeadsConsts[i].effects = bp->BlockGet(L"Effects");
        m_RobotHeadsConsts[i].effects_description = bp->ParGet(L"EffectsDescription");
        m_RobotHeadsConsts[i].art_description = bp->ParGet(L"ArtDescription");

        m_RobotHeadsConsts[i].constructor_button_name = bp->ParGet(L"ConstructorButton");
        m_RobotHeadsConsts[i].constructor_image_name = bp->ParGet(L"ConstructorImage");
        m_RobotHeadsConsts[i].constructor_label_name = bp->ParGet(L"ConstructorLabel");
        m_RobotHeadsConsts[i].model_path = bp->ParGet(L"ModelPathInCache");

        m_RobotHeadsConsts[i].module_hit_effect = WeapName2Weap(bp->ParNE(L"ModuleHitEffect"));

        //Цена в ресурсах
        m_RobotHeadsConsts[i].cost_titan = bp->BlockGet(L"Cost")->ParNE(L"Titan").GetInt();
        m_RobotHeadsConsts[i].cost_electronics = bp->BlockGet(L"Cost")->ParNE(L"Electronics").GetInt();
        m_RobotHeadsConsts[i].cost_energy = bp->BlockGet(L"Cost")->ParNE(L"Energy").GetInt();
        m_RobotHeadsConsts[i].cost_plasma = bp->BlockGet(L"Cost")->ParNE(L"Plasma").GetInt();

        //В процессе заполнения сортируем массив так, чтобы доступные игроку модули оказались первыми в списке
        //(добавил это, т.к. не разобрался в уебанском алгоритме заполнения раскрываемых менюшек в конструкторе), upd: разобрался
        //if(m_RobotHeadsConsts[i].for_player_side) std::swap(m_RobotHeadsConsts[i], m_RobotHeadsConsts[m_RobotHeadsConsts[0].for_player_side]);
    }

    //Загружаем информацию о характеристиках турелей:
    bp_tmp = g_MatrixData->BlockPathGet(CONFIG_PATH_TURRETS);
    //m_TurretTypesCount = bp_tmp->ParGet(L"TurretsCount").GetInt();
    for(int i = 0; i <= TURRET_KINDS_TOTAL; ++i)
    {
        STurretsConsts t;
        m_TurretsConsts.push_back(t);

        if(!i) //Элемент между концом списка с оружием для роботов и информацией о турелях отводим под общую информацию
        {
            m_TurretsConsts[0].name = bp_tmp->ParGet(L"BuildTurretButtonHint"); //Описание для кнопки выбора турели
            m_TurretsConsts[0].for_player_side = 0; //Инициализация счётчика
            continue;
        }

        CBlockPar* bp = bp_tmp->BlockGet(CWStr(i));
        if(bp->ParGetNE(L"NotForPlayer").GetBool()) m_TurretsConsts[i].for_player_side = 0; //Блокирует данную турель от постройки игроком (пока не реализовано)
        else
        {
            m_TurretsConsts[i].for_player_side = 1;
            ++m_TurretsConsts[0].for_player_side;
        }

        m_TurretsConsts[i].name = bp->ParGet(L"Name");
        m_TurretsConsts[i].chars_description = bp->ParGet(L"CharsDescription");

        m_TurretsConsts[i].structure = max(10.0f * bp->ParGet(L"Structure").GetFloat(), 10.0f);
        float rot_speed = bp->ParGet(L"RotationAnglePerSecond").GetFloat();
        m_TurretsConsts[i].rotation_speed = GRAD2RAD(rot_speed > 0 ? rot_speed / (1000 / LOGIC_TACT_PERIOD) : 0.0f);
        m_TurretsConsts[i].highest_vertical_angle = GRAD2RAD(bp->ParGet(L"HighestVerticalBarrelAngle").GetFloat());
        m_TurretsConsts[i].lowest_vertical_angle = GRAD2RAD(bp->ParGet(L"LowestVerticalBarrelAngle").GetFloat());
        m_TurretsConsts[i].seek_target_range = bp->ParGet(L"SeekTargetRange").GetFloat();
        m_TurretsConsts[i].strength = bp->ParGet(L"Strength").GetFloat();

        //Записываем число стволов и их матрицы
        {
            int j = 1;
            wchar_t buf[32];
            swprintf(buf, 32, L"Gun%i_MatrixId", j);
            int barrel_matrix = bp->ParGetNE(buf).GetInt();
            while(barrel_matrix)
            {
                STurretsConsts::STurretGun barrel;
                barrel.matrix_id = barrel_matrix;
                swprintf(buf, 32, L"Gun%i_WeaponType", j);
                barrel.weapon_type = WeapName2Weap(bp->ParGet(buf));
                m_TurretsConsts[i].guns.push_back(barrel);

                swprintf(buf, 32, L"Gun%i_MatrixId", ++j);
                barrel_matrix = bp->ParGetNE(buf).GetInt();
            }
        }

        //Рассинхрон для поочерёдной стрельбы пушек
        if(m_TurretsConsts[i].guns.size() > 1) m_TurretsConsts[i].guns_async_time = int(max(1000.0f * bp->ParGetNE(L"GunsAsyncDelay").GetFloat(), 0.0f));
        else m_TurretsConsts[i].guns_async_time = 0;

        //Урон, который нанесут детали турели при попадании
        m_TurretsConsts[i].mount_part_hit_effect = WeapName2Weap(bp->ParNE(L"MountPartHitEffect"));
        m_TurretsConsts[i].gun_part_hit_effect = WeapName2Weap(bp->ParNE(L"GunPartHitEffect"));

        //Цена в ресурсах
        m_TurretsConsts[i].cost_titan = bp->BlockGet(L"Cost")->ParNE(L"Titan").GetInt();
        m_TurretsConsts[i].cost_electronics = bp->BlockGet(L"Cost")->ParNE(L"Electronics").GetInt();
        m_TurretsConsts[i].cost_energy = bp->BlockGet(L"Cost")->ParNE(L"Energy").GetInt();
        m_TurretsConsts[i].cost_plasma = bp->BlockGet(L"Cost")->ParNE(L"Plasma").GetInt();

        m_TurretsConsts[i].model_path = bp->ParGet(L"ModelPathInCache");

        //Готовим строку с параметрами турели (показатель урона считается от урона по роботам)
        m_TurretsConsts[i].chars_description.Replace(CWStr(L"<Structure>"), CWStr(m_TurretsConsts[i].structure / 10));
        CWStr str_damage = (CWStr)L"";
        for(int j = 0; j < (int)m_TurretsConsts[i].guns.size(); ++j)
        {
            int gun_type = m_TurretsConsts[i].guns[j].weapon_type;
            if(j) str_damage += L"+";
            int shot_delay = m_WeaponsConsts[gun_type].shots_delay;//max(m_WeaponsConsts[gun_type].shots_delay, m_TurretsConsts[i].guns_async_time * 2); //Рассинхроны между выстрелами могут замедлять скорострельность турели, если их значение выше половины значения задержки скорострельности (не могут, т.к. в этом случае просто отменятся)
            str_damage += CWStr(int( (m_WeaponsConsts[gun_type].damage.to_robots / 10.0f) * (1.0f / (shot_delay / 1000.0f)) ));
        }
        m_TurretsConsts[i].chars_description.Replace(CWStr(L"<DamagePerSecond>"), str_damage);
    }

    //Загружаем характеристики зданий
    int cnt = 1;
    for(int i = 0; i < BUILDING_TYPES_COUNT; ++i, ++cnt)
    {
        m_BuildingsHitPoints[i] = max(10.0f * g_MatrixData->ParPathGet(L"BuildingsConfig." + CWStr(cnt) + L".Structure").GetFloat(), 10.0f);
    }

    // timings
    m_ReinforcementsTime = Float2Int(1000.0f * g_MatrixData->BlockPathGet(BLOCK_PATH_REINFORCEMENTS)->ParGet(L"ReinforcementsReadyTimer").GetFloat()); //Стандартное время до готовности вызова подкрепления в секундах

    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_TIMINGS);

    // resources
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_TIMINGS)->BlockGet(PAR_SOURCE_TIMINGS_RESOURCES);
    m_Timings[RESOURCE_TITAN] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_TITAN).GetInt();
    m_Timings[RESOURCE_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ENERGY).GetInt();
    m_Timings[RESOURCE_PLASMA] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_PLASMA).GetInt();
    m_Timings[RESOURCE_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ELECTRONICS).GetInt();
    m_Timings[RESOURCE_BASE] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_BASE).GetInt();
    // units
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_TIMINGS)->BlockGet(PAR_SOURCE_TIMINGS_UNITS);
    m_Timings[UNIT_ROBOT] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ROBOT).GetInt();
    m_Timings[UNIT_FLYER] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_FLYER).GetInt();
    m_Timings[UNIT_TURRET] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_TURRET).GetInt();

    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_TIMINGS)->BlockGet(PAR_SOURCE_TIMINGS_CAPTURE);
    m_CaptureTimeErase = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ERASE).GetInt();
    m_CaptureTimePaint = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_PAINT).GetInt();
    m_CaptureTimeRolback = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ROLLBACK).GetInt();

    //Common chars
    m_RobotRadarRadius = g_MatrixData->BlockGet(BLOCK_PATH_MAIN_CONFIG)->ParGet(PAR_SOURCE_RADAR_ROBOT_RADIUS).GetFloat();
    m_FlyerRadarRadius = g_MatrixData->BlockGet(BLOCK_PATH_MAIN_CONFIG)->ParGet(PAR_SOURCE_RADAR_FLYER_RADIUS).GetFloat();

    // camera properties
    bp_tmp = g_MatrixData->BlockGetNE(PAR_SOURCE_CAMERA);
    if(bp_tmp)
    {
        int cnt = bp_tmp->ParCount();
        for(int i = 0; i < cnt; ++i)
        {
            if(bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_BASEANGLEZ) m_CamBaseAngleZ = GRAD2RAD(bp_tmp->ParGet(i).GetFloat());
            else if(bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_ARCADE_FWD0) m_CamInRobotForward0 = bp_tmp->ParGet(i).GetFloat();
            else if(bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_ARCADE_FWD1) m_CamInRobotForward1 = bp_tmp->ParGet(i).GetFloat();
            else if(bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_MOVESPEED) m_CamMoveSpeed = bp_tmp->ParGet(i).GetFloat();
            else if(bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_FOV) g_CamFieldOfView = bp_tmp->ParGet(i).GetFloat();
        }

        cnt = bp_tmp->BlockCount();
        for(int i = 0; i < cnt; ++i)
        {
            int index = -1;
            if(bp_tmp->BlockGetName(i) == PAR_SOURCE_CAMERA_STRATEGY) index = CAMERA_STRATEGY;
            else if(bp_tmp->BlockGetName(i) == PAR_SOURCE_CAMERA_ARCADE) index = CAMERA_ARCADE;
            if(index >= 0)
            {
                CBlockPar* bp_c = bp_tmp->BlockGet(i);

                int cnt2 = bp_c->ParCount();
                for(int j = 0; j < cnt2; ++j)
                {
                    if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ROTSPEEDX) m_CamParams[index].m_CamRotSpeedX = bp_c->ParGet(j).GetFloat();
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ROTSPEEDZ) m_CamParams[index].m_CamRotSpeedZ = bp_c->ParGet(j).GetFloat();
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_WHEELSTEP) m_CamParams[index].m_CamMouseWheelStep = bp_c->ParGet(j).GetFloat();
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ROTANGLEMIN) m_CamParams[index].m_CamRotAngleMin = GRAD2RAD(min(94.0f, bp_c->ParGet(j).GetFloat()));
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ROTANGLEMAX) m_CamParams[index].m_CamRotAngleMax = GRAD2RAD(bp_c->ParGet(j).GetFloat());
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_DISTMIN) m_CamParams[index].m_CamDistMin = bp_c->ParGet(j).GetFloat();
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_DISTMAX) m_CamParams[index].m_CamDistMax = bp_c->ParGet(j).GetFloat();
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_DISTPARAM) m_CamParams[index].m_CamDistParam = bp_c->ParGet(j).GetFloat();
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ANGLEPARAM) m_CamParams[index].m_CamAngleParam = bp_c->ParGet(j).GetFloat();
                    else if(bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_HEIGHT) m_CamParams[index].m_CamHeight = bp_c->ParGet(j).GetFloat();
                }
            }
        }
    }

    //Перебираем блок AssignKeys в RobotsCFG.txt, ищем соответствия имён параметров прописанным игровым действиям
    CBlockPar robotsCFG(true, g_MatrixHeap);
    wchar robotsCFG_Path[MAX_PATH];
    SHGetSpecialFolderPathW(0, robotsCFG_Path, CSIDL_PERSONAL, true);
    wcscat(robotsCFG_Path, L"\\SpaceRangersHD\\RobotsCFG.txt");

    //Если обнаружили искомый конфиг, то загружаем его во избежание потери прочих, возможно, содержащихся в нём данных
    CWStr temp_path = (CWStr)L"";
    if(CFile::FileExist(temp_path, robotsCFG_Path)) robotsCFG.LoadFromTextFile(robotsCFG_Path);
    
    //Если активен мод на автоподрыв роботов с бомбой при нулевом здоровье
    if(g_PlayerRobotsAutoBoom)
    {
        const CWStr* default_set = &robotsCFG.ParGetNE(L"AutoDetonationDefault");
        if(!default_set->IsEmpty())
        {
            if(!default_set->GetBool()) g_PlayerRobotsAutoBoom = 2;
        }
        else robotsCFG.ParSetAdd(L"AutoDetonationDefault", L"1");
    }

    //Число параметров должно соответствовать числу прописанных биндов, всё что выше - отбрасывается
    CBlockPar* assignKeys = robotsCFG.BlockGetAdd(CFG_ASSIGN_KEYS);
    int i = 0;
    for(int i = 0; i < KA_LAST; ++i)
    {
        CWStr par = assignKeys->ParGetNE(key_action_codes[i].name);
        if(par != nullptr)
        {
            //Присваиваем прописанную в параметре клавишу определённому игровому действию
            int akn = key_action_codes[i].code;
            //Если указанная за корректным именем параметра клавиша невалидна
            if(akn < 0 || akn >= KA_LAST) continue;

            int kk = KeyName2KeyCode(par);
            //Если в качестве названия клавиши вписан невалид
            if(kk == -1)
            {
                ERROR_S3(L"Planetary battle Warning! Key binded to \"", CWStr(key_action_codes[i].name), L"\" was not found! Reassign is unavailable.");
                continue;
            }

            m_KeyActions[akn] = kk;
        }
        else assignKeys->ParAdd(key_action_codes[i].name, KeyActionNum2KeyName(i));
    }

    //Записываем готовый блок с параметрами-биндами в RobotsCFG.txt
    robotsCFG.SaveInTextFile(robotsCFG_Path);

    //Вариант для опциональной подгрузки того же блока AssignKeys, но из data.txt
    /*
    if(cfg_par->BlockCount(CFG_ASSIGN_KEYS) != 0)
    {
        CBlockPar* ak = cfg_par->BlockGet(CFG_ASSIGN_KEYS);
        int n = ak->ParCount();
        for(int i = 0; i < n; ++i)
        {
            int akn = KeyActionName2KeyActionCode(ak->ParGetName(i));

            if(akn < 0 || akn >= KA_LAST) continue;

            int kk = KeyName2KeyCode(ak->ParGet(i));
            m_KeyActions[akn] = kk;
        }
    }
    */
}

static void GenRamp(word* out, SGammaVals& vals)
{
    const float brk = 1;
    const float cok = 1;

    float contrast = (vals.contrast - 0.5f) * cok;
    float brightness = (vals.brightness - 0.5f) * brk;

    float x = (contrast + 0.5f) * 0.45f;
    float y = 0.45f - x;
    float mu = (-x) / (0.5f - x);

    y += mu * (0.5f - y);
    mu = (0.5f - y) / 128.0f;

    for(int i = 0; i < 256; ++i)
    {
        float f = (float)pow((float)y, 1.0f/vals.gamma) + brightness;
        y += mu;

        int v = Float2Int(f * 65535.0f);
        if(v < 0) v = 0;
        if(v > 65535) v = 65535;
        out[i] = (word)v;
    }
}

void CMatrixConfig::ApplyGammaRamp(void)
{
    D3DGAMMARAMP newramp;
    GenRamp(newramp.red, m_GammaR);
    GenRamp(newramp.green, m_GammaG);
    GenRamp(newramp.blue, m_GammaB);

    g_D3DD->SetGammaRamp(0, D3DSGR_CALIBRATE, &newramp);
    //g_D3DD->SetGammaRamp(1, D3DSGR_CALIBRATE, &newramp);
}

CMatrixConfig g_Config;


