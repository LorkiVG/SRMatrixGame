// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixMap.hpp"
#include "MatrixObject.hpp"
#include "MatrixObjectRobot.hpp"
#include "MatrixObjectBuilding.hpp"
#include "Logic/MatrixEnvironment.h"
#include "Logic/MatrixTactics.h"
#include "Logic/MatrixAIGroup.h"

#if defined _DEBUG || defined _TRACE
#define CHECK_ROBOT_POS() { if(m_MapX < 0 || m_MapX >= g_MatrixMap->m_SizeMove.x || m_MapY < 0 || m_MapY >= g_MatrixMap->m_SizeMove.y) _asm int 3 }
#else
#define CHECK_ROBOT_POS()
#endif

class CMatrixEffectSelection;

#define COLLIDE_FIELD_R			3
#define COLLIDE_BOT_R			18
#define COLLIDE_SPHERE_R        (GLOBAL_SCALE_MOVE / 2.1f)
//5.2f
#define	BASE_DIST			    70
#define MAX_HULL_ANGLE			GRAD2RAD(360)
#define BARREL_TO_SHOT_ANGLE    GRAD2RAD(15)
#define HULL_TO_ENEMY_ANGLE     GRAD2RAD(45)
#define MAX_ORDERS              5
#define ROBOT_FOV               GRAD2RAD(180)
#define GATHER_PERIOD           100
#define ZERO_VELOCITY           0.02f
#define DECELERATION_FORCE      0.9900990099009901f
#define MIN_ROT_DIST            20
#define GET_LOST_MIN            COLLIDE_BOT_R * 5
#define GET_LOST_MAX            COLLIDE_BOT_R * 10
#define ROBOT_SELECTION_HEIGHT  3
#define ROBOT_SELECTION_SIZE    20

#define HULL_SOUND_PERIOD       3000
#define HULL_ROT_TAKTS          30
#define HULL_ROT_S_ANGL         10

#define MAX_CAPTURE_CANDIDATES  4

//Используются в режиме ручного управления
#define UNIT_MOVING_FORWARD     0
#define UNIT_MOVING_BACKWARD    1
#define UNIT_MOVING_LEFT        2
#define UNIT_MOVING_RIGHT       3

//Типы вращения точки привязки камеры при повороте шасси робота
#define NO_CAM_ROTATION         0 //Точка привязки камеры не будет вращаться вместе с шасси
#define CONNECTED_CAM_ROTATION  2 //Точка привязки камеры будет вращаться строго вместе с шасси, синхронизируется с его позицией
#define SIMULTANEOUS_CAM_ROTATION 3 //Точка привязки камеры будет вращаться вместе с шасси, но не синхронизируется с его позицией

struct SRobotWeapon
{
private:
	CMatrixEffectWeapon* m_Weapon = nullptr;

public:
    SMatrixRobotModule* m_Module = nullptr;

    int                 m_Heat = 0;
    int                 m_HeatingSpeed = 0; //heat grows per WEAPON_HEAT_PERIOD
    int                 m_CoolingDelay = 0;
    int                 m_CoolingSpeed = 0;

    bool                m_On = false; //Маркер активности орудия, если false, то оно не будет стрелять при получении роботом приказа открыть огонь (при отсутствии оружия в слоте равно false)

    void ModifyCoolDown(float addk) { m_Weapon->ModifyCoolDown(addk); }
    void ModifyDist(float addk) { m_Weapon->ModifyDist(addk); }
    bool GetWeaponPos(D3DXVECTOR3& pos) { if(m_Weapon) { pos = m_Weapon->GetPos(); return true; } return false; }
    void SetArcadeCoefficient() { if(m_Weapon) m_Weapon->SetArcadeCoefficient(); }
    void SetDefaultCoefficient() { if(m_Weapon) m_Weapon->SetDefaultCoefficient(); }
    bool IsEffectPresent() const { return m_Weapon != nullptr; }
    int  GetWeaponNum() const { return m_Weapon->GetWeaponNum(); }
    float GetWeaponDist() const { return m_Weapon->GetWeaponDist(); }
    void Modify(const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, const D3DXVECTOR3& speed) { m_Weapon->Modify(pos, dir, speed); }
    void UpdateRepair() { if(m_Module->m_WeaponRepairData) m_Module->m_WeaponRepairData->Update(m_Module); }

    void Tact(float t)
    {
        m_Weapon->Tact(t);
        if(m_Module->m_WeaponRepairData) m_Module->m_WeaponRepairData->Update(m_Module);
    }
    bool IsFireWas() const { return m_Weapon->IsFireWas(); }
    
    void SetOwner(CMatrixMapStatic* ms) { m_Weapon->SetOwner(ms); }

    void CreateEffect(dword user, FIRE_END_HANDLER handler, int type, int cooldown = 0)
    {
        m_Weapon = (CMatrixEffectWeapon*)CMatrixEffect::CreateWeapon(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 1), user, handler, type, cooldown);
        if(g_Config.m_WeaponsConsts[type].is_repairer) PrepareRepair();
    }

    void FireBegin(const D3DXVECTOR3& speed, CMatrixMapStatic* skip) { m_Weapon->FireBegin(speed, skip); }

    void FireEnd()
    {
        m_Weapon->FireEnd();
        m_Module->m_Graph->SetAnimLooped(0);
    }

    void Draw(CMatrixRobotAI* robot);
    void Release()
    {
        m_Weapon->Release();
        if(m_Module->m_WeaponRepairData) m_Module->m_WeaponRepairData->Release();
        m_Module->m_WeaponRepairData = nullptr;
    }

    void PrepareRepair();
};

enum OrderType
{
	ROT_EMPTY_ORDER,
	ROT_MOVE_TO,
	ROT_MOVE_TO_BACK,
    ROT_MOVE_RETURN,          // Робот пропускает другого робота затем возвращается в эту позицию
    ROT_STOP_MOVE,
	ROT_FIRE,
	ROT_STOP_FIRE,
    ROT_CAPTURE_BUILDING,
    ROT_STOP_CAPTURE
};

enum OrderPhase
{
    ROP_EMPTY_PHASE,
    ROP_WAITING_FOR_PARAMS,
    ROP_MOVING,
    ROP_FIRING,
    ROP_CAPTURE_MOVING,
    ROP_CAPTURE_IN_POSITION,
    ROP_CAPTURE_SETTING_UP,
    ROP_CAPTURING,
    ROP_GETING_LOST
};

struct SCaptureCandidate
{
    SObjectCore* bcore = nullptr;
    int          tbc = 0; // time before capture
};

struct SOrder 
{
	OrderType          m_OrderType = ROT_EMPTY_ORDER;
    OrderPhase         m_OrderPhase = ROP_EMPTY_PHASE;

	float              m_Param1 = 0.0f, m_Param2 = 0.0f, m_Param3 = 0.0f;
    int                m_Param4 = 0;
	//CMatrixRobotAI*  m_Robot = nullptr;
    //void*            m_Object = nullptr;
    SObjectCore*       m_ObjCore = nullptr;

public:
    OrderType GetOrderType() const   { return m_OrderType; }
    OrderPhase GetOrderPhase() const { return m_OrderPhase; }

    void GetParams(float* p1, float* p2, float* p3, int* p4 = nullptr)
    {
        if(p1 != nullptr) *p1 = m_Param1;
        if(p2 != nullptr) *p2 = m_Param2;
        if(p3 != nullptr) *p3 = m_Param3;
        if(p4 != nullptr) *p4 = m_Param4;
    }

    CMatrixMapStatic* GetStatic()
    {
        if(m_ObjCore)
        {
            if(m_ObjCore->m_Object) return m_ObjCore->m_Object;
            m_ObjCore->Release();
            m_ObjCore = nullptr;
        }
        return nullptr;
    }

    void SetOrder(const OrderType& orderType, const float& p1 = 0, const float& p2 = 0, const float& p3 = 0, int p4 = 0)
    {
        Reset(); 
        m_OrderType = orderType;
        m_Param1 = p1;
        m_Param2 = p2;
        m_Param3 = p3;
        m_Param4 = p4;
    }
    void SetOrder(const OrderType& orderType, CMatrixMapStatic* obj)
    {
        Reset();
        m_OrderType = orderType;
        m_ObjCore = obj ? obj->GetCore(DEBUG_CALL_INFO) : nullptr;
    }
    void SetPhase(const OrderPhase& phase) { m_OrderPhase = phase; }

    void Reset() { memset(this, 0, sizeof(SOrder)); }

    void Release()
    {
        if(m_ObjCore)
        {
            if(m_ObjCore->m_Object && m_ObjCore->m_Object->IsBuilding())
            {
                m_ObjCore->m_Object->AsBuilding()->ResetCaptured();
            }

            m_ObjCore->Release();
            m_ObjCore = nullptr;
        }
    }
};

class CMatrixRobotAI : public CMatrixRobot
{
    CTextureManaged* m_BigTexture = nullptr;
    CTextureManaged* m_MedTexture = nullptr;
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    CTextureManaged* m_SmallTexture = nullptr;   // hm, may be its no needed //may be, who cares :)
#endif

    SEffectHandler      m_Ablaze;

    int                 m_ColsWeight = 0;
    int                 m_ColsWeight2 = 0;
    int                 m_Cols = 0;
    int                 m_Team = -1;
    int                 m_GroupLogic = -1; // dab. В какой логической группе находится робот
    int                 m_Group = 0;
    int                 m_OrdersInPool = 0;
    SOrder              m_OrdersList[MAX_ORDERS];

    SCaptureCandidate   m_CaptureCandidates[MAX_CAPTURE_CANDIDATES];
    int                 m_CaptureCandidatesCnt = 0;

    int                 m_MapX = 0, m_MapY = 0;
    int                 m_ZoneCur = -1;				        // Зона в которой находится робот
    int                 m_DesX = 0, m_DesY = 0;		        // Точка в которую хочет встать робот
    int                 m_ZoneDes = -1;				        // Зона в которую стремится робот
    int                 m_ZonePathNext = -1;			    // Следующая зона в пути 
    int                 m_ZonePathCnt = 0;			        // Количество зон (включительно начальную и конечную) до m_ZoneDes
    int                 m_ZoneNear = -1;				    // Ближайшая зона от той в которой находится робот по направлению к m_ZoneDes
    int                 m_MovePathCnt = 0;			        // Количество точек в пути движения
    int                 m_MovePathCur = 0;			        // Текущая точка в пути движения
    int*                m_ZonePath = nullptr;			    // Список зон до m_ZoneDes
    CPoint              m_MovePath[MatrixPathMoveMax] = {}; // Путь движения
    float               m_MovePathDist = 0.0f;              // Длина расчитанного пути
    float               m_MovePathDistFollow = 0.0f;        // Сколько робот прошёл по пути
    D3DXVECTOR2         m_MoveTestPos = { 0.0f, 0.0f };
    int                 m_MoveTestChange = 0;

    float               m_Strength = 0.0f;                  // Боевая сила робота

    int                 m_NextTimeAblaze = 0;
    int                 m_NextTimeShorted = 0;
    D3DXVECTOR3         m_CollAvoid = { 0.0f, 0.0f, 0.0f };

    float               m_SpeedWaterCorr = 1.0f;            // коррекция скорости при движении по воде
    float               m_SpeedSlopeCorrDown = 1.0f;        // коррекция скорости при движении с горы
    float               m_SpeedSlopeCorrUp = 1.0f;          // коррекция скорости при движении в гору

	float               m_maxSpeed = 0.0f;		             // максимально развиваемая скорость
    float               m_maxStrafeSpeed = 0.0f;		     // максимально развиваемая скорость стрейфа (не должна быть выше m_maxSpeed)
    float               m_maxHullSpeed = 0.0f;               // скорость поворота корпуса
    float               m_maxRotationSpeed = 0.0f;           // скорость вращения робота на месте
    float               m_MaxFireDist = 0.0f;
    float               m_MinFireDist = 0.0f;
    float               m_RepairDist = 0.0f;
    float               m_SyncMul = 0.0f;
    int                 m_CtrlGroup = 0;

    //Маркер, указывающий, что робот был выведен из ручного контроля игроком, никаких приказов на себе не имеет,
    //а потому не трогай его, сука, со своими перенаправлениями в ближайший треугольник сетки!
    bool                    m_AfterManualControl = false;

    //CMatrixMapStatic*     m_FireTarget = nullptr;
    //int                   m_GatherPeriod = 0;
    CInfo                   m_Environment;
    byte                    m_HaveRepair = 0; //0 - нет, 1 - есть хотя бы один ремонтник, 2 - все оружия это ремонтник (не считая бомб)

    D3DXVECTOR3             m_WeaponDir = { 0.0f, 0.0f, 0.0f };
    dword                   m_nLastCollideFrame = 0;
    dword                   m_PrevTurnSign = 1;
    dword                   m_HullRotCnt = 0;
    CMatrixEffectSelection* m_Selection = nullptr;

    int                     m_LastDelayDamageSide = 0;
    bool                    m_NoMoreEverMovingOverride = false; //"Приколачивает робота к месту гвоздями", если true

public:
    SRobotWeapon            m_Weapons[RUK_WEAPON_PYLONS_COUNT]; //Установленное оружие (заполняется всегда от начала без пропусков, так что чем меньше пушек у робота, тем быстрее переборы)
    int                     m_WeaponsCount = 0;                 //Число установленных пушек в конкретном шаблоне заранее неизвестно, их там может быть хоть 0

    dword                   m_SoundChassis = SOUND_ID_EMPTY;

    float                   m_RadarRadius = 0.0f;
    float                   m_DischargerProtect = 0.0f;  // 0 .. 1   (0 - no protection, 1 - full protection)
    float                   m_BombProtect = 0.0f;        // 0 .. 1   (0 - no protection, 1 - full protection)
    float                   m_AimProtect = 0.0f;         // 0 .. 1   (0 - no protection, 1 - full protection)
    float                   m_SelfRepair = 0.0f;         // Единицы HP, которые восстанавливается в секунду
    float                   m_SelfRepairPercent = 0.0f;  // Процент HP, которые восстанавливается в секунду (сюда запоминается конкретное количество HP для восстановления с учётом тактов)
    float                   m_MissileTargetCaptureAngleCos = 1.0f; //Косинус угла, который будет суммирован к косинусу угла захвата цели ракетой при выстреле из ракетного оружия
    float                   m_MissileTargetCaptureAngleSin = 0.0f; //Синус угла, который необходим для суммы косинусов, потому что я геометрию рот ебал
    float                   m_MissileHomingSpeed = 0.0f; //Бонусная скорость выхода самонаводящейся ракеты на цель, суммируется при выстреле ракетой

    float                   m_GroupSpeed = 0.0f;  // скорость робота в группе
    float                   m_ColSpeed = 100.0f;  // скорость робота? если впереди другой робот

    void             CreateTextures();
    CTextureManaged* GetBigTexture() { return m_BigTexture; }
    CTextureManaged* GetMedTexture() { return m_MedTexture; }
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    CTextureManaged* GetSmallTexture() { return m_SmallTexture; }
#endif

    CMatrixRobotAI() : CMatrixRobot() {}
    ~CMatrixRobotAI() { ReleaseMe(); }

    int         GetCols() { return m_Cols; }
    void        IncCols() { ++m_Cols; }
    int         GetColsWeight() { return m_ColsWeight; }
    void        IncColsWeight(int val = 1) { m_ColsWeight += val; }
    void        SetColsWeight(int w) { m_ColsWeight = w; }
    int         GetColsWeight2() { return m_ColsWeight2; }
    void        IncColsWeight2(int val = 1) { m_ColsWeight2 += val; }
    void        SetColsWeight2(int w) { m_ColsWeight2 = w; }
    int         GetMapPosX() const { return m_MapX; }
    int         GetMapPosY() const { return m_MapY; }
    CInfo*      GetEnv() { return &m_Environment; }
    void        SetEnvTarget(CMatrixMapStatic* t) { m_Environment.m_Target = t; }
    int         GetTeam() { return m_Team; }
    void        SetTeam(int t) { m_Team = t; }
    int         GetGroup() { return m_Group; }
    void        SetGroup(int g) { m_Group = g; }
    int*        GetGroupP() { return &m_Group; }
    float       GetMaxSpeed() { return m_maxSpeed; }
    void        SetMaxSpeed(float s) { m_maxSpeed = s; }
    float       GetMaxStrafeSpeed() { return m_maxStrafeSpeed; }
    void        SetMaxStrafeSpeed(float s) { m_maxStrafeSpeed = s; }
    int         GetOrdersInPool() { return m_OrdersInPool; }
    SOrder*     GetOrder(int no) { return m_OrdersList + no; }
    CMatrixEffectSelection* GetSelection() { return m_Selection; }
    //CWStr&    GetName() { return m_Name; }
    const SRobotWeapon& GetWeapon(int i) const { return m_Weapons[i]; }
    bool        IsHomingMissilesEquipped(); //Проверяет, имеются ли у робота любые ракеты с системой наведения
    float       GetMaxFireDist() { return m_MaxFireDist; }
    float       GetMinFireDist() { return m_MinFireDist; }
    float       GetRepairDist() { return m_RepairDist; }
    int         GetGroupLogic() { return m_GroupLogic; }
    void        SetGroupLogic(int gl) { m_GroupLogic = gl; }
    int         GetRegion() { return g_MatrixMap->GetRegion(m_MapX, m_MapY); }
    int         GetCtrlGroup() { return m_CtrlGroup; }
    void        SetCtrlGroup(int group) { m_CtrlGroup = group; }

    ERobotModuleKind GetRobotHeadKind() { for(int i = 0; i < m_ModulesCount; ++i) { if(m_Module[i].m_Type == MRT_HEAD) return m_Module[i].m_Kind; } return RUK_EMPTY; }

    void        MapPosCalc() { g_MatrixMap->PlaceGet(m_Module[0].m_Kind - 1, m_PosX - 20.0f, m_PosY - 20.0f, &m_MapX, &m_MapY); }

    SRobotWeapon* GetWeaponByPylonNum(byte pylon) //Ищем и возвращаем соответствующее указанному слоту оружие, либо nullptr
    {
        for(int i = 0; i < m_WeaponsCount; ++i)
        {
            if(m_Weapons[i].IsEffectPresent() && m_Weapons[i].m_Module->m_ConstructorPylonNum == pylon) return &m_Weapons[i];
        }

        return nullptr;
    }

    void SetWeaponToArcadedCoeff();
    void SetWeaponToDefaultCoeff();

    float GetStrength() { return m_Strength; } // Сила робота
    void CalcStrength();                       // Расчитываем силу робота

    bool PLIsInPlace() const;

    void CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type);
    void DeleteProgressBarClone(EPBCoord clone_type);

    bool SelectByGroup();
    bool SelectArcade();
    void UnSelect();
    bool IsSelected();
    bool CreateSelection();
    void KillSelection();
    void MoveSelection();

    //Маркер, указывающий, что робот был выведен из ручного контроля игроком, никаких приказов на себе не имеет,
    //а потому не трогай его, сука, со своими перенаправлениями в ближайший треугольник сетки!
    bool IsAfterManual() { return m_AfterManualControl; }
    void SetAfterManual(bool value) { m_AfterManualControl = value; }

    void PlayHullSound();
    bool CheckFireDist(const D3DXVECTOR3 &point);

    bool IsAutomaticMode() const { return m_CurrState == ROBOT_IN_SPAWN || m_CurrState == ROBOT_BASE_MOVEOUT || m_CurrState == ROBOT_CAPTURING_BASE; }
    bool CanBreakOrder()
    {
        if(m_Side != PLAYER_SIDE || FLAG(g_MatrixMap->m_Flags, MMFLAG_FULLAUTO))
        {
            CMatrixBuilding* cf = GetCaptureBuilding();
            if(cf)
            {
                return false; //DO NOT BREAK CAPTURING!!!!!!!!!!!!!!!!!!!!!!!! NEVER!!!!!!!!!!
                /*
                if(cf->IsBase()) return false;
                if(cf->GetSide() != robot->GetSide())
                {
                    if((float(cf->m_TrueColor.m_ColoredCnt) / MAX_ZAHVAT_POINTS) > (1.0 - (robot->AsRobot()->GetHitPoint() * 1.1f) / robot->AsRobot()->GetMaxHitPoint())) return false;
                }
                */
            }
        }

        return !IsAutomaticMode() && ((m_Side != PLAYER_SIDE) || (g_MatrixMap->GetPlayerSide()->GetArcadedObject() != this));
    }

    void        OBBToAABBCollision(int nHeight, int nWidth);
    D3DXVECTOR3 LineToAABBIntersection(const D3DXVECTOR2& s, const D3DXVECTOR2& e, const D3DXVECTOR2& vLu, const D3DXVECTOR2& vLd, const D3DXVECTOR2& vRu, const D3DXVECTOR2& vRd, bool revers_x, bool revers_y);
    D3DXVECTOR3 CornerLineToAABBIntersection(const D3DXVECTOR2& s, const D3DXVECTOR2& e, const D3DXVECTOR2& vLu, const D3DXVECTOR2& vLd, const D3DXVECTOR2& vRu, const D3DXVECTOR2& vRd);
    D3DXVECTOR3 SphereRobotToAABBObstacleCollision(D3DXVECTOR3& corr, const D3DXVECTOR3& vel);
    //D3DXVECTOR3 SphereToAABBIntersection(const D3DXVECTOR2 &pos,float r, const D3DXVECTOR2 &vLu,const D3DXVECTOR2 &vLd,const D3DXVECTOR2 &vRu,const D3DXVECTOR2 &vRd, bool revers_x, bool revers_y);
    D3DXVECTOR3 RobotToObjectCollision(const D3DXVECTOR3& vel, int ms);
    void        WallAvoid(const D3DXVECTOR3& obstacle, const D3DXVECTOR3& dest);
    static bool SphereToAABBCheck(const D3DXVECTOR2& sphere_p, const D3DXVECTOR2& vMin, const D3DXVECTOR2& vMax, float& d, float& dx, float& dy);
    D3DXVECTOR3 SphereToAABB(const D3DXVECTOR2& pos, const SMatrixMapMove* smm, const CPoint& cell, byte corner); //, bool revers_x, bool revers_y);

    void ZoneCurFind();		                    // Найти зону в которой находится робот (Out: m_ZoneCur)
	void ZonePathCalc();	                    // Рассчитать путь до m_ZoneDes (In: m_ZoneCur, m_ZoneDes) (Out: m_ZonePathCnt, m_ZonePath)
	void ZoneMoveCalc();                        // Рассчитать путь движения до ближайшей зоны (In: m_ZoneCur, m_ZoneNear) (Out: m_MovePathCnt, m_MovePathCur, m_MovePath)
    float CalcPathLength();
    //void ZoneMoveCalcTo();	                // Рассчитать путь в нутрии текущей зоны до точки назначения (In: m_DesX, m_DesY) (Out: m_MovePathCnt, m_MovePathCur, m_MovePath)
	void MoveByMovePath(int ms);	            // Двигаться по пути движения
    //void MoveToRndBuilding();


    void CalcRobotParams(struct SRobotTemplate* robot_template = nullptr); // Вычисляет массу, скорость, силу, эффекты от модуля и прочие параметры робота
    bool Seek(const D3DXVECTOR3& dest, bool& rotate, bool end_path, byte moving_direction = UNIT_MOVING_FORWARD, bool cam_rotation = true); // поиск вектора смещения робота
    void SetHullTargetDirection(const D3DXVECTOR3& direction);  // поворот башни
    bool RotateRobotChassis(const D3DXVECTOR3& dest, byte rotate_cam_link = CONNECTED_CAM_ROTATION, float* rotateangle = nullptr);      // поворот робота
	void Decelerate();                              // замедление
    void RobotWeaponInit();                         // инициализация оружия
    void HitTo(CMatrixMapStatic* hit, const D3DXVECTOR3& pos);

    bool IsStrafing() { return FLAG(m_ObjectFlags, ROBOT_FLAG_STRAFING); } //Маркер выполнения "сложного" стрейфа для роботов, передвигающихся по земле (не на парящих шасси)
    //void RotateRobotLeft() { SETFLAG(m_ObjectFlags, ROBOT_FLAG_ROT_LEFT); }
    //void RotateRobotRight() { SETFLAG(m_ObjectFlags, ROBOT_FLAG_ROT_RIGHT); }


//Orders queue processing
    SOrder* AllocPlaceForOrderOnTop();  // make sure that order will bi initialized after this call
    //void AddOrderToEnd(const SOrder &order);

    void RemoveOrderFromTop() { RemoveOrder(0); }

    void RemoveOrder(int pos);
    void RemoveOrder(OrderType order);

    void ProcessOrdersList();

    bool const HaveBomb()
    {
        for(int nC = 0; nC < m_WeaponsCount; ++nC)
        {
            if(m_Weapons[nC].IsEffectPresent() && m_Weapons[nC].GetWeaponNum() == WEAPON_BOMB) return true;
        }
        return false;
    }
    byte const HaveRepair() { return m_HaveRepair; } //0 - нет, 1 - есть хотя бы один ремонтник, 2 - все оружия это ремонтник (не считая бомб)

    //High orders
    void MoveToHigh(int mx, int my);

    //Orders
    void MoveTo(int mx, int my);
    void MoveToBack(int mx, int my);
    void MoveReturn(int mx, int my);
    void StopMoving();

    void OrderFire(const D3DXVECTOR3& fire_pos, int type = 0); // type 0 - fire 2 - repair
    void StopFire();
    void BigBoom();//int nc = -1); //Использование аргумента более неактуально

    void CaptureBuilding(CMatrixBuilding* building);
    CMatrixBuilding* GetCaptureBuilding();
    void StopCapture();

    void TactCaptureCandidate(int ms);
    void AddCaptureCandidate(CMatrixBuilding* b);
    void RemoveCaptureCandidate(CMatrixBuilding* b);
    void ClearCaptureCandidates();

    bool FindOrder(OrderType findOrder, CMatrixMapStatic* obj);
    int  FindOrderLikeThat(OrderType order) const;
    int  FindOrderLikeThat(OrderType order, OrderPhase phase);

    void BreakAllOrders();
    void SetNoMoreEverMovingOverride(bool set) { m_NoMoreEverMovingOverride = set; }
    bool IsNoMoreEverMovingOverride() { return m_NoMoreEverMovingOverride; }
    void UpdateOrder_MoveTo(int mx, int my);
    bool GetMoveToCoords(CPoint& pos);
    bool GetReturnCoords(CPoint& pos);

    void LowLevelStopFire();
    void LowLevelStop();
    bool FindWeapon(int type);
    bool FindRepairWeapon();
    bool FindBombWeapon();

    void LowLevelMove(int ms, const D3DXVECTOR3& dest, bool robot_coll, bool obst_coll, bool end_path = true, byte moving_direction = UNIT_MOVING_FORWARD, bool cam_rotation = true);
    void LowLevelDecelerate(int ms, bool robot_coll, bool obst_coll);

    void ReleaseMe();
    void GatherInfo(int ms);

    void GetLost(const D3DXVECTOR3& v);

    void RobotSpawn(CMatrixBuilding* pBase);    // spawn робота		
    void DIPTact(float ms);                     // death in progress tact

	virtual void LogicTact(int cms);
    void PauseTact(int cms);

#ifdef _DEBUG
    virtual void Draw();
#endif
    virtual bool TakingDamage(int weap, const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, int attacker_side = NEUTRAL_SIDE, CMatrixMapStatic* attaker = nullptr);

    friend class CMatrixRobot;
};

__forceinline void SRobotWeapon::Draw(CMatrixRobotAI* robot)
{
    if(m_Module->m_WeaponRepairData) m_Module->m_WeaponRepairData->Draw(robot->IsInterfaceDraw());
}

inline SMatrixPlace* GetPlacePtr(int no)
{
    if(no < 0) return nullptr;
    return g_MatrixMap->m_RoadNetwork.GetPlace(no);
}

__forceinline bool CMatrixRobotAI::PLIsInPlace() const
{
    CPoint ptp;

    if(FindOrderLikeThat(ROT_MOVE_TO)) return false;
    if(FindOrderLikeThat(ROT_MOVE_RETURN)) return false;

    if(m_Environment.m_Place >= 0)
    {
        ptp = GetPlacePtr(m_Environment.m_Place)->m_Pos;
    }
    else if(m_Environment.m_PlaceAdd.x >= 0)
    {
        ptp = m_Environment.m_PlaceAdd;
    }
    else return false;

    return (GetMapPosX() == ptp.x) && (GetMapPosY() == ptp.y);
}