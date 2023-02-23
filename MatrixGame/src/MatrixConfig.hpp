// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_CONFIG_INCLUDE
#define MATRIX_CONFIG_INCLUDE

#include "Effects/MatrixEffect.hpp"
#include "MatrixMapStatic.hpp"
#include <vector>
#include <bitset>

#define DAMAGE_PER_TIME     10000
#define SECRET_VALUE        1000000.0f
#define LOGIC_TACT_PERIOD   10 //В миллисекундах, итого имеем 100 логических тактов в секунду

struct SStringPair
{
    CWStr   key;
    CWStr   val;
};

enum ERes
{
    TITAN,
    ELECTRONICS,
    ENERGY,
    PLASMA,

    MAX_RESOURCES,

    ERes_FORCE_DWORD = 0x7FFFFFFF
};

enum ERobotModuleType
{
    MRT_EMPTY = 0,
    MRT_CHASSIS = 1,
    MRT_WEAPON = 2,
    MRT_HULL = 3,
    MRT_HEAD = 4,

    ERobotModuleType_FORCE_DWORD = 0x7FFFFFFF
};

enum ERobotModuleKind
{
    RUK_EMPTY = 0, //Если слот модуля пуст (модуль не выбран)

    //Корпуса
    RUK_HULL_MONOSTACK        = 1,
    RUK_HULL_BIREX            = 2,
    RUK_HULL_DIPLOID          = 3,
    RUK_HULL_PARAGON          = 4,
    RUK_HULL_TRIDENT          = 5,
    RUK_HULL_FULLSTACK        = 6,

    //Шасси
    RUK_CHASSIS_PNEUMATIC      = 1,
    RUK_CHASSIS_WHEEL          = 2,
    RUK_CHASSIS_TRACK          = 3,
    RUK_CHASSIS_HOVERCRAFT     = 4,
    RUK_CHASSIS_ANTIGRAVITY    = 5,

    //Головы
    RUK_HEAD_BLOCKER           = 1,
    RUK_HEAD_DYNAMO            = 2,
    RUK_HEAD_LOCATOR           = 3,
    RUK_HEAD_FIREWALL          = 4,
    RUK_HEAD_REPAIRATOR        = 5, //Оригинально назывался RUK_HEAD_RAPID
    RUK_HEAD_DESIGN            = 6,
    RUK_HEAD_SPEAKER           = 7,

    //Пилоны под оружие
    RUK_WEAPON_PYLON_1         = 0,
    RUK_WEAPON_PYLON_2         = 1,
    RUK_WEAPON_PYLON_3         = 2,
    RUK_WEAPON_PYLON_4         = 3,
    RUK_WEAPON_PYLON_5         = 4,

    RUK_WEAPON_PYLONS_COUNT    = 5,

    //Оружие
    RUK_WEAPON_MACHINEGUN      = 1,
    RUK_WEAPON_CANNON          = 2,
    RUK_WEAPON_MISSILES        = 3,
    RUK_WEAPON_FLAMETHROWER    = 4,
    RUK_WEAPON_MORTAR          = 5,
    RUK_WEAPON_LASER           = 6,
    RUK_WEAPON_BOMB            = 7,
    RUK_WEAPON_PLASMAGUN       = 8,
    RUK_WEAPON_DISCHARGER      = 9,
    RUK_WEAPON_REPAIRER        = 10,
};

#define BUILDING_TYPES_COUNT 6

enum ETurretKind
{
    TURRET_LIGHT_CANNON = 1,
    TURRET_HEAVY_CANNON,
    TURRET_LASER_CANNON,
    TURRET_MISSILE_CANNON,

    TURRET_KINDS_TOTAL = 4
};

enum EKeyAction
{
    KA_SCROLL_LEFT,
    KA_SCROLL_RIGHT,
    KA_SCROLL_UP,
    KA_SCROLL_DOWN,

    KA_SCROLL_LEFT_ALT,
    KA_SCROLL_RIGHT_ALT,
    KA_SCROLL_UP_ALT,
    KA_SCROLL_DOWN_ALT,

    KA_ROTATE_LEFT,
    KA_ROTATE_RIGHT,
    KA_ROTATE_UP,
    KA_ROTATE_DOWN,

    KA_UNIT_FORWARD,
    KA_UNIT_BACKWARD,
    KA_UNIT_STRAFE_LEFT,
    KA_UNIT_STRAFE_RIGHT,
    KA_UNIT_ROTATE_LEFT,
    KA_UNIT_ROTATE_RIGHT,
    KA_UNIT_UP,
    KA_UNIT_DOWN,

    KA_UNIT_FORWARD_ALT,
    KA_UNIT_BACKWARD_ALT,
    KA_UNIT_STRAFE_LEFT_ALT,
    KA_UNIT_STRAFE_RIGHT_ALT,
    KA_UNIT_ROTATE_LEFT_ALT,
    KA_UNIT_ROTATE_RIGHT_ALT,
    KA_UNIT_UP_ALT,
    KA_UNIT_DOWN_ALT,

    KA_UNIT_SET_GUN_1,
    KA_UNIT_SET_GUN_2,
    KA_UNIT_SET_GUN_3,
    KA_UNIT_SET_GUN_4,
    KA_UNIT_SET_GUN_5,
    KA_UNIT_REAR_VIEW,

    KA_FIRE,
    KA_AUTO,

    KA_SHIFT,
    KA_CTRL,
    KA_ALL_UNITS_SELECT,

    KA_ROTATE_LEFT_ALT,
    KA_ROTATE_RIGHT_ALT,

    KA_MINIMAP_ZOOMIN,
    KA_MINIMAP_ZOOMOUT,

    KA_CAM_SETDEFAULT,
    KA_TAKE_SCREENSHOT,
    KA_SAVE_SCREENSHOT,
    KA_GAME_PAUSED,

    KA_AUTOORDER_CAPTURE,
    KA_AUTOORDER_ATTACK,
    KA_AUTOORDER_DEFEND,

    KA_ORDER_MOVE,
    KA_ORDER_STOP,
    KA_ORDER_CAPTURE,
    KA_ORDER_PATROL,
    KA_ORDER_EXPLODE,
    KA_ORDER_REPAIR,
    KA_ORDER_ATTACK,
    KA_ORDER_ROBOT_SWITCH1,
    KA_ORDER_ROBOT_SWITCH2,

    KA_ORDER_CANCEL,

    KA_UNIT_BOOM,
    KA_UNIT_ENTER, //(! if not dialog mode)
    KA_UNIT_ENTER_ALT, //(! if not dialog mode)

    KA_GATHERING_POINT,
    KA_BUILD_ROBOT,
    KA_BUILD_ROBOT_START,
    KA_BUILD_ROBOT_QUANTITY_UP,
    KA_BUILD_ROBOT_QUANTITY_DOWN,
    KA_BUILD_ROBOT_CHOOSE_LEFT,
    KA_BUILD_ROBOT_CHOOSE_RIGHT,
    KA_BUILD_TURRET,
    KA_CALL_REINFORCEMENTS,

    KA_TURRET_CANNON,
    KA_TURRET_GUN,
    KA_TURRET_LASER,
    KA_TURRET_ROCKET,

    KA_LAST
};

enum ETimings
{
    RESOURCE_TITAN,
    RESOURCE_ELECTRONICS,
    RESOURCE_ENERGY,
    RESOURCE_PLASMA,
    RESOURCE_BASE,

    UNIT_ROBOT,
    UNIT_FLYER,
    UNIT_TURRET,

    TIMING_LAST
};

enum ESide
{
    NEUTRAL_SIDE = 0,
    PLAYER_SIDE = 1,
    BLAZER_SIDE = 2,
    KELLER_SIDE = 3,
    TERRON_SIDE = 4,

    TOTAL_SIDES,

    //По нумерации должны идти сразу за последней стороной
    REINFORCEMENTS_TEMPLATES = 5,
    ROBOTS_SPAWNER_TEMPLATES = 6
};

struct SCamParam
{
    float m_CamMouseWheelStep;
    float m_CamRotSpeedX;
    float m_CamRotSpeedZ;
    float m_CamRotAngleMin;
    float m_CamRotAngleMax;
    float m_CamDistMin;
    float m_CamDistMax;
    float m_CamDistParam;
    float m_CamAngleParam;
    float m_CamHeight;
};

struct SGammaVals
{
    float brightness = 0.5f, contrast = 0.5f, gamma = 1.0f;
};

enum
{
    CAMERA_STRATEGY,
    CAMERA_ARCADE,

    CAMERA_PARAM_CNT
};

#define ROBOT_HULLS_COUNT g_Config.m_RobotHullsCount
struct SRobotHullsWeaponPylonData
{
    int id;                      //Id матрицы для установки оружия на модели корпуса
    bool inverted_model;         //Маркер о необходимости инвертировать модель установленного в данный пилон оружия (требуется для оружия на левой стороне корпуса)
    std::bitset<128> fit_weapon; //Какое оружие может быть установлено в данный пилон, до 128 пушек (нумерация с 1)
    byte constructor_slot_num;   //Номер слота в конструкторе, который соответствует данному пилону (нумерация с 0)
};
struct SRobotHullsConsts
{
    byte for_player_side;         //Маркер, означающий, что данный модуль разрешён для использования игроком в конструкторе

    CWStr name;                   //Название модуля
    CWStr robot_part_name;        //Обозначение модуля, из которого собирается название робота (есть только у корпуса, шасси и головы)
    CWStr short_designation;      //Обозначение данного модуля в строке шаблона робота

    CWStr chars_description;      //Описание характеристик корпуса
    CWStr art_description;        //Лоровое (художественное) описание корпуса

    std::bitset<RUK_WEAPON_PYLONS_COUNT> constructor_weapon_slots_used; //Используется для быстрого определения, какие из орудийных слотов в конструкторе должны быть открыты для данного корпуса (нумерация с 0)
    std::vector<SRobotHullsWeaponPylonData> weapon_pylon_data;

    CWStr constructor_button_name;
    CWStr constructor_image_name;
    CWStr constructor_label_name;
    CWStr hull_sound_name;
    ESound hull_sound_num;         //Номер звукового объекта из hull_sound_name в общем массиве m_Sound
    CWStr model_path;

    //Основные характеристики
    float structure;
    float rotation_speed;

    int module_hit_effect = WEAPON_NONE;

    int cost_titan;
    int cost_electronics;
    int cost_energy;
    int cost_plasma;
};

#define ROBOT_CHASSIS_COUNT g_Config.m_RobotChassisCount
struct SRobotChassisTraceMatrix
{
    byte matrix_id;       //Id матрицы на модели, к которой необходимо линковать след шага
    //CWStr matrix_name;  //Название матрицы на модели, к которой необходимо линковать след шага
};
struct SRobotChassisGroundTrace
{
    int trace_num;               //Запоминаем сюда номер элемента в массиве следов m_SpotProperties

    CWStr texture_path;
    float texture_scale;

    dword trace_redraw_distance; //Частота отрисовки текстур на земле (актуально только для колёсных и гусеничных шасси, т.к. шагающее оставляет следы под матрицами на ступнях)
    dword trace_duration;
};
struct SRobotChassisJetStream
{
    byte matrix_id;        //Id матрицы на модели, к которой необходимо прикрепить реактивный след
    //CWStr matrix_name;   //Название матрицы на модели, к которой необходимо прикрепить реактивный след

    CWStr sprites_name;    //Спрайты для анимации
    std::vector<int> sprites_num;
    byte sprites_count;

    float length;          //Длина реактивного следа
};
struct SRobotChassisConsts
{
    byte for_player_side;         //Маркер, означающий, что данный модуль разрешён для использования игроком в конструкторе

    CWStr name;                   //Название шасси
    CWStr robot_part_name;        //Обозначение модуля, из которого собирается название робота (есть только у корпуса, шасси и головы)
    CWStr short_designation;      //Обозначение данного шасси в строке шаблона робота

    CWStr chars_description;      //Описание характеристик шасси
    CWStr art_description;        //Лоровое (художественное) описание шасси

    CWStr constructor_button_name;
    CWStr constructor_image_name;
    CWStr constructor_label_name;
    CWStr arcade_enter_sound_name;
    ESound arcade_enter_sound_num; //Номер звукового объекта из arcade_enter_sound_name в общем массиве m_Sound (играется в момент включения режима прямого управления роботом)
    std::vector<CWStr> move_out_sound_name; //Звуки шасси, используемые во время отдачи роботу приказа на движение или патруль
    CWStr model_path;

    //Основные характеристики
    float structure;
    float move_speed;
    float strafe_speed;           //Скорость стрейфа не должна быть выше move_speed
    float anim_move_speed;        //Скорость прокрутки анимации движения шасси, если такая анимация у шасси есть
    float move_uphill_factor;
    float move_downhill_factor;
    float move_in_water_factor;
    float rotation_speed;
    byte passability;

    bool is_walking;              //Маркер шагающего шасси, под каждой ступней рисуются одиночные следы
    std::vector<SRobotChassisTraceMatrix> trace_matrix;
    bool is_rolling;              //Маркер колёсной/гусеничной техники, означает, что под шасси нужно рисовать непрерывные следы
    SRobotChassisGroundTrace ground_trace;
   
    bool is_hovering;             //Маркер парящего над землёй шасси, робот с данным шасси не может провалиться под воду (под карту) (для "Экроплана" и "Антиграва")
    bool is_dust_trace;           //Шасси будет оставлять под собой пылевой след (для "Экроплана")
    bool speed_dependent_anim;    //Маркер для корректного воспроизведения анимаций движения, скорость воспроизведения которых должна быть привязана к текущей скорости робота

    std::vector<SRobotChassisJetStream> jet_stream; //Количество и данные для реактивных следов, которые нужно добавить на модель шасси (для "Антиграва")

    int module_hit_effect = WEAPON_NONE;

    int cost_titan;
    int cost_electronics;
    int cost_energy;
    int cost_plasma;
};

#define ROBOT_HEADS_COUNT g_Config.m_RobotHeadsCount
struct SRobotHeadsConsts
{
    byte for_player_side;         //Маркер, означающий, что данный модуль разрешён для использования игроком в конструкторе

    CWStr name;                   //Название модуля
    CWStr robot_part_name;        //Обозначение модуля, из которого собирается название робота (есть только у корпуса, шасси и головы)
    CWStr short_designation;      //Обозначение данного модуля в строке шаблона робота

    CBlockPar* effects = nullptr; //Блок с параметрами эффектов (было проще запомнить сюда указатель весь блок, чем менять их подгрузку)

    CWStr effects_description;    //Описание эффектов головы
    CWStr art_description;        //Лоровое (художественное) описание модуля

    CWStr constructor_button_name;
    CWStr constructor_image_name;
    CWStr constructor_label_name;
    CWStr model_path;

    int module_hit_effect = WEAPON_NONE;

    int cost_titan = 0;
    int cost_electronics = 0;
    int cost_energy = 0;
    int cost_plasma = 0;
};

#define ROBOT_WEAPONS_COUNT g_Config.m_RobotWeaponsCount
struct SRobotWeaponsConsts
{
    byte for_player_side;         //Маркер, означающий, что данное оружие разрешено для использования игроком в конструкторе

    CWStr name;                   //Название модуля
    CWStr short_designation;      //Обозначение данного оружия в строке шаблона робота

    CWStr chars_description;      //Описание характеристик оружия
    CWStr art_description;        //Лоровое (художественное) описание оружия

    CWStr constructor_button_name;
    CWStr constructor_image_name;
    CWStr constructor_label_name;
    CWStr status_panel_image;
    CWStr model_path;

    int weapon_type;

    bool is_module_bomb = false;
    bool is_module_repairer = false;

    //Для ремонтника - точки, по которым рисуется линия эффекта свечения
    int dev_start_id = 0;
    int dev_end_id = 0;

    float strength;               //Параметр относительной силы данного орудийного модуля, по которой его опасность оценивают боты

    int heating_speed;
    int cooling_speed;
    int cooling_delay;

    int module_hit_effect = WEAPON_NONE;

    int cost_titan;
    int cost_electronics;
    int cost_energy;
    int cost_plasma;
};

struct STurretsConsts
{
    byte for_player_side;         //Маркер, означающий, что данная турель разрешена для постройки игроком

    CWStr name;                   //Название турели
    CWStr chars_description;      //Описание характеристик оружия
    CWStr model_path;

    float strength;               //Параметр относительной силы турели, по которой её опасность оценивают боты

    float structure;              //Запас HP (параметр для турелей)

    struct STurretGun
    {
        int matrix_id = 0;
        //CWStr matrix_name = (CWStr)L"";
        int weapon_type = WEAPON_NONE;
    };
    std::vector<STurretGun> guns; //Количество стволов турели, в которых записаны их матрицы и соответствующие типы орудий
    int guns_async_time;          //Время рассинхрона между выстрелами турели с несколькими орудиями

    float seek_target_range;      //Дальность предварительного выбора цели (применяется только для турелей)

    float rotation_speed;         //Максимальная скорость поворота турели за такт в радианах (если параметр равен 0, то максимальная)
    float highest_vertical_angle; //Углы вертикального наведения
    float lowest_vertical_angle;

    int mount_part_hit_effect = WEAPON_NONE;
    int gun_part_hit_effect = WEAPON_NONE;

    int cost_titan;
    int cost_electronics;
    int cost_energy;
    int cost_plasma;
};

enum //EPrimaryWeaponEffect
{
    EFFECT_CANNON = 1,
    EFFECT_ROCKET_LAUNCHER,
    EFFECT_MORTAR,
    EFFECT_REPAIRER,
    EFFECT_BOMB
};

enum //ESecondaryWeaponEffect
{
    SECONDARY_EFFECT_ABLAZE = 1,
    SECONDARY_EFFECT_SHORTED_OUT
};

struct SWeaponsConsts
{
    CWStr type_name = (CWStr)L"";
    byte primary_effect = 0;   //Маркер и номер для оружия, являющегося основным эффектом (различные выстрелы)
    byte secondary_effect = 0; //Маркер и номер (SECONDARY_EFFECT_ABLAZE, SECONDARY_EFFECT_SHORTED_OUT и т.д.) для оружия, являющегося несамостоятельным дополнительным эффектом
    int effect_priority = 0;   //Приоритет эффекта имеет смысл, когда необходимо выбрать, какой из эффектов одного типа нужно наложить или при выборе замены уже наложенного эффекта

    bool is_bomb = false;
    bool is_repairer = false;

    //Общий массив с информацией обо всех оружейных эффектах (используются как роботами, так вертолётами и турелями)
    struct SObjectsVaraity
    {
        float to_robots = 0.0f;
        float to_flyers = 0.0f;
        float to_turrets = 0.0f;
        float to_buildings = 0.0f;
        float to_objects = 0.0f;
    };
    SObjectsVaraity damage;               //Обычный урон, наносимый данным оружием по врагам
    //SObjectsVaraity friendly_damage;    //Урон, наносимый данным оружием по дружественным целям (как правило, всё равно перемножается на ситуативный коэффициент, так что особого смысла не имеет)
    SObjectsVaraity non_lethal_threshold; //При нанесении урона данным оружием, HP атакуемого объекта не будет опускаться ниже данного значения (задавать выше 0 только для нелетального оружия / типа урона)

    float shots_delay = 0.0f;              //Задержка в ms между выстрелами
    float shot_range = 0.0f;               //Дальность стрельбы

    //Для горения объектов у каждого оружия есть свой отдельный блок
    struct SMapObjectsIgnition
    {
        bool is_present = false;

        int priority = 0;

        int duration_per_hit = 0; //Какая продолжительность горения набрасывается на объект за каждое попадание
        int burning_starts_at = 0; //Начало горения объекта по общему накрученному таймеру (до этого времени он просто дымится)
        int max_duration = 0;

        CWStr sprites_name = (CWStr)L"";
        std::vector<int> sprites_num;
        byte sprites_count = 0;

        ESound burning_sound_num = S_NONE;
        CWStr burning_sound_name = (CWStr)L"";

    } map_objects_ignition;

    //Используются для всяких эффектов и подробных параметров оружия
    CWStr projectile_model_path = (CWStr)L"";
    float projectile_start_velocity = 0.0f;
    float projectile_full_velocity = 0.0f;
    int   projectile_full_velocity_reach = 0;
    float projectile_acceleration_coef = 0.0f;
    float projectile_target_capture_angle_cos = 0.0f; //Записывается сюда как косинус угла, используется для самонаведения ракеты
    float projectile_target_capture_angle_sin = 0.0f; //Для суммирования косинусов перед запуском ракеты
    float projectile_homing_speed = 0.0f; //Используется для самонаведения ракеты
    float projectile_splash_radius = 0.0f;
    int   projectile_max_lifetime = 0;

    int   sprites_lenght = 0;
    int   sprites_width = 0;

    int   contrail_width = 0;
    int   contrail_duration = 0;
    int   contrail_fire_effect_starts = 0;
    dword hex_BGRA_sprites_color = 0;

    int   light_radius = 0;
    int   light_duration = 0;
    dword hex_BGRA_light_color = 0;

    SFloatRGBColor close_color_rgb = { 0.0f, 0.0f, 0.0f };
    SFloatRGBColor far_color_rgb = { 0.0f, 0.0f, 0.0f };

    //Для примера объявления обычных массивов дефолтным аргументом
    //void A(const int(&arr)[2] = { 2, 2 }) {}

    struct SWeaponExtraEffect {
        int type = -1;

        int duration_per_hit = 0;
        int max_duration = 0;
    };
    std::vector<SWeaponExtraEffect> extra_effects; //Массив с номерами дополнительных эффектов, которые накладывает данное оружие при нанесении урона

    //Спрайты, использующиеся для эффектов и анимаций оружия (могут использоваться отдельные спрайты, либо наборы для спрайтовых анимаций)
    struct SSpriteSet{
        CWStr sprites_name = (CWStr)L"";
        std::vector<int> sprites_num;
        byte sprites_count = 0;
    };
    std::vector<SSpriteSet> sprite_set; //Также используется для эффектов, использующих всего один, либо несколько отдельных спрайтов

    CWStr shot_sound_name = (CWStr)L"";
    ESound shot_sound_num = S_NONE;
    bool shot_sound_looped = false; //Маркер о необходимости непрерывного воспроизведения звука выстрела, пока зажата гашетка, без необходимости разделять его на отдельные выстрелы (как для огнемёта лазера, разрядника и ремонтника)
    CWStr hit_sound_name = (CWStr)L"";
    ESound hit_sound_num = S_NONE;
    bool explosive_hit = false;
};

//Не забывать обновлять, ResetMenu и выборе модулей по клавишам с клавиатуры (//Были нажаты цифровые клавиши 1-9 (по местной нумерации 2-10)), чтобы не крашила при невалиде в шаблоне!

//Не забыть про лимиты установки модулей MR_MAX_MODULES и RUK_WEAPON_PYLONS_COUNT
class CMatrixConfig : public CMain
{
public:
    int m_RobotHullsCount;
    std::vector<SRobotHullsConsts> m_RobotHullsConsts;
    int m_RobotChassisCount;
    std::vector<SRobotChassisConsts> m_RobotChassisConsts;
    int m_RobotHeadsCount;
    std::vector<SRobotHeadsConsts> m_RobotHeadsConsts;
    int m_RobotWeaponsCount;
    std::vector<SRobotWeaponsConsts> m_RobotWeaponsConsts;

    std::vector<STurretsConsts> m_TurretsConsts;

    std::vector<SWeaponsConsts> m_WeaponsConsts;


    SStringPair* m_Cursors = nullptr;
    int          m_CursorsCnt = 0;

    float        m_BuildingsHitPoints[BUILDING_TYPES_COUNT] = { 5000.0f, 3000.0f, 3000.0f, 3000.0f, 3000.0f, 3000.0f }; //Стоковые значения для базы и заводов (в любом случае перезаписываются из конфига во время инициализации игры)

    int          m_ReinforcementsTime = 0;

    //int m_PlayerRobotsCnt;
    //int m_CompRobotsCnt;

    // camera params

    SCamParam m_CamParams[CAMERA_PARAM_CNT];

    float m_CamBaseAngleZ;
    float m_CamMoveSpeed;
    float m_CamInRobotForward0;
    float m_CamInRobotForward1;

    //float m_CamRotAngleMinInFlyer;
    //float m_CamDistMinInFlyer;

    SGammaVals m_GammaR, m_GammaG, m_GammaB;

    // params
    dword m_DIFlags;
    //int m_TexTopMinSize;
    //int m_TexTopDownScalefactor;
    //int m_TexBotMinSize;
    //int m_TexBotDownScalefactor;

    bool m_ShowStencilShadows;
    bool m_ShowProjShadows;
    bool m_CannonsLogic;
    //bool m_LandTextures16;

    bool m_LandTexturesGloss;
    bool m_SoftwareCursor;
    bool m_VertexLight;

    bool m_ObjTexturesGloss;
    //bool m_ObjTextures16;

    bool m_IzvratMS;
    byte m_SkyBox;                  // 0 - none, 1 - dds (low quality), 2 - png (high quality)
    byte m_DrawAllObjectsToMinimap; // 0 - none, 1 - force yes, 2 - auto

    int  m_CaptureTimeErase;
    int  m_CaptureTimePaint;
    int  m_CaptureTimeRolback;

    int  m_KeyActions[KA_LAST];
    bool m_IsManualMissileControl = false; //Маркер активации режима ручного управления наведением ракет для робота под прямым управлением игрока

    int  m_Timings[TIMING_LAST];

    float m_RobotRadarRadius;
    float m_FlyerRadarRadius;

    EShadowType m_RobotShadow;

    void Clear();

    void SetDefaults();
    void ReadParams();
    void ApplySettings(SRobotsSettings* set);

    bool IsManualMissileControl() { return m_IsManualMissileControl; }; //Состояние маркера активации режима ручного управления наведением ракет

    void ApplyGammaRamp(void);

    static const wchar* CMatrixConfig::KeyActionCode2KeyName(const int& num);

    dword RGBAStringToABGRColorHEX(const CWStr* par)
    {
        //Собираем цвет из параметра RGBA в ABGR HEX
        CWStr par_color = par->GetStrPar(0, L":");
        int red = par_color.GetStrPar(0, L",").GetInt();
        int green = par_color.GetStrPar(1, L",").GetInt();
        int blue = par_color.GetStrPar(2, L",").GetInt();
        int alpha = int(255 * par->GetStrPar(1, L":").GetDouble());
        return (blue > 255 ? 255 : blue) | ((green > 255 ? 255 : green) << 8) | ((red > 255 ? 255 : red) << 16) | ((alpha > 255 ? 255 : alpha) << 24);
    }

    //Добавил сюда для удобства, т.к. g_Config видно отовсюду
    float PortionInDiapason(
        float cur,    //Текущее значение
        float a,      //Начало диапазона, в котором находится текущее значение
        float b,      //Конец диапазона, в котором находится текущее значение
        float fromA,  //Минимальное значение диапазона, в который нужно перевести текущее значение
        float toB     //Максимальное значение диапазона, в который нужно перевести текущее значение
    )
    {
        if(a < b)
        {
            if(cur <= a) return fromA;
            else if(cur >= b) return toB;
            else return (cur - a) / (b - a) * (toB - fromA) + fromA;
        }
        else
        {
            if(cur >= a) return fromA;
            else if(cur <= b) return toB;
            else return (a - cur) / (a - b) * (toB - fromA) + fromA;
        }
    }

    CMatrixConfig() : CMain() {};
    ~CMatrixConfig() = default;
};

extern CMatrixConfig g_Config;

__forceinline int WeapName2Weap(const CWStr& weapon_name)
{
    for(int i = 0; i < (int)g_Config.m_WeaponsConsts.size(); ++i)
    {
        if(g_Config.m_WeaponsConsts[i].type_name == weapon_name) return i;
    }
    return WEAPON_NONE;
}

#endif