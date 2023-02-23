// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixMap.hpp"
#include "MatrixConfig.hpp"
#include "MatrixProgressBar.hpp"
#include "Effects/MatrixEffect.hpp"

class CMatrixFlyer;

enum ERobotState
{
    ROBOT_EMBRYO = 0,          //Объект для нового робота ещё только начал формироваться (модели робота как таковой ещё не существует)
	ROBOT_IN_SPAWN,            //Постройка робота завершена, модель отрендерена, но подъёмник базы ещё опущен - начинается подъём робота к свету
	ROBOT_BASE_MOVEOUT,        //Постройка робота завершена, подъёмник базы поднят - робот делает первые шаги (управлять им в этот момент всё ещё нельзя)
	ROBOT_SUCCESSFULLY_BUILD,  //Робот успешно построен и готов к получению приказов
    ROBOT_CARRYING,            //Робот подобран вертолётом и летит
    ROBOT_FALLING,             //Робот сброшен вертолётом и падает
    ROBOT_DIP,                 //Запущен процесс уничтожения робота (death in progress), управлять им больше нельзя, да и почти все проверки с этого момента начнут его игнорировать
    ROBOT_CAPTURING_BASE       //Робот добрался до точки начала и приступил к непосредственному захвату базы (не завода), с этого момента он может либо захватить базу, либо умереть (управлять им больше невозможно)
};

enum EAnimation
{
    ANIMATION_OFF,
    ANIMATION_STAY,
    ANIMATION_MOVE,
    ANIMATION_BEGINMOVE,
    ANIMATION_ENDMOVE,
    ANIMATION_ROTATE,
    //ANIMATION_ROTATE_LEFT,
    //ANIMATION_ROTATE_RIGHT,
    ANIMATION_MOVE_BACK,
    ANIMATION_BEGINMOVE_BACK,
    ANIMATION_ENDMOVE_BACK
};


#define CARRYING_DISTANCE       20.0f
#define CARRYING_SPEED          0.996

#define KEELWATER_SPAWN_FACTOR  0.01f
#define DUST_SPAWN_FACTOR       0.007f

#define MR_MAX_MODULES		        9

struct SMatrixRobotModule;

struct SWeaponRepairData
{
    CSprite     m_b0;
    CSprite     m_b1;
    CSpriteLine m_Sprites;
    D3DXVECTOR3 m_pos0 = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_pos1 = { 0.0f, 0.0f, 0.0f };

    int m_DeviceStartMatrixId = 0;
    int m_DeviceEndMatrixId = 0;

    dword m_Flags = 0;
    static const dword CAN_BE_DRAWN = SETBIT(0);

    void Release();
    void Update(SMatrixRobotModule* unit);
    void Draw(bool now);

    static SWeaponRepairData* Allocate(ESpriteTextureSort sprite_spot = SPR_REPAIRER_SPOT, ESpriteTextureSort sprite_rect = SPR_REPAIRER_RECT, int dev_start_id = 2, int dev_end_id = 1);
};

struct SChassisData
{
    //Массив указателей на объекты с эффектами реактивных следов от шасси
    std::vector<CMatrixEffectFireStream*> JetStream;

    //Для шасси на воздушной подушке
    float m_DustCount = 0.0f;

    //Для шасси траков
    D3DXVECTOR3 m_LastSolePos;

    //Для шасси пневматики
    D3DXVECTOR2 m_LinkPos;
    int         m_LinkPrevFrame = 0;
};

struct SMatrixRobotModule
{
    ERobotModuleType    m_Type = MRT_EMPTY; // 0 - empty, 1 - шасси, 2 - оружие, 3 - корпус, 4 - голова

	CVectorObjectAnim*  m_Graph = nullptr;
    D3DXMATRIX          m_Matrix;

    SWeaponRepairData*  m_WeaponRepairData = nullptr;
	CVOShadowStencil*   m_ShadowStencil = nullptr;
    ERobotModuleKind    m_Kind = RUK_EMPTY;
    float               m_NextAnimTime = 0.0f;
	float               m_Angle = 0.0f;
    int                 m_LinkMatrix = 0;
    D3DXMATRIX          m_IMatrix;
	bool                m_Invert = false; //Маркер о необходимости инвертирования модели данного модуля (используется при установке оружия на левую сторону корпуса)
    byte                m_ConstructorPylonNum = 0; //Номер слота в корпусе, в котором установлен данный модуль (пока актуально только для оружия, нумерация с 0)

    D3DXVECTOR3         m_Pos = { 0, 0, 0 };
    D3DXVECTOR3         m_Velocity = { 0, 0, 0 };
    float               m_TTL = 0.0f;
    float               m_dp = 0.0f, m_dr = 0.0f, m_dy = 0.0f;
            
    byte                m_SmokeEffect[sizeof(SEffectHandler)] = { 0 };
    
    SMatrixRobotModule() = default;

    SEffectHandler& Smoke() { return *(SEffectHandler*)&m_SmokeEffect; }

    void PrepareForDIP();
};

struct SWalkingChassisData
{
    D3DXVECTOR2 foot;
    D3DXVECTOR2 other_foot; // if relink occurs, this contained new foot (relink coord)
    dword       newlink = 0;
};

class CMatrixRobot : public CMatrixMapStatic
{
    EAnimation m_Animation = ANIMATION_OFF;
    CMatrixBuilding* m_Base = nullptr; //база из который вышел робот

protected:
    //hitpoint
    CMatrixProgressBar m_ProgressBar;
    int         m_ShowHitpointTime = 0;
    float       m_HitPoint = 0.0f;
    float       m_HitPointMax = 0.0f;  // Максимальное кол-во здоровья
    float       m_MaxHitPointInversed = 0.0f; // for normalized calcs

    static      SWalkingChassisData* m_WalkingChassis;

    bool        m_AutoBoom = false; //По умолчанию автоподрыв бомбы у робота всегда выключен

    //dword     m_RobotFlags; // m_ObjectFlags used instead. do not uncomment!

public:
	EShadowType m_ShadowType = SHADOW_STENCIL; // 0-off 1-proj 2-proj with anim 3-stencil
    int         m_ShadowSize = 128; // texture size for proj

    CWStr m_Name = (CWStr)L"ROBOT";
    int m_defHitPoint = Float2Int(m_HitPoint);

    float m_Speed = 0.0f;
    float m_RotSpeed = 0.0f;
    float m_PosX = 0.0f, m_PosY = 0.0f;

	int m_Side = 0;	// 1-8

    int m_CalcBoundsLastTime = -101;   // need for calculation of bound only 10 times per second
    D3DXVECTOR3 m_CalcBoundMin;
    D3DXVECTOR3 m_CalcBoundMax;

	int m_ModulesCount = 0;
    SMatrixRobotModule m_Module[MR_MAX_MODULES];

    SChassisData m_ChassisData;

    int m_TimeWithBase = 0;

	D3DXVECTOR3 m_ChassisForward = { 0, 0, 0 };    //Вектор текущего направления шасси робота (его текущее физическое положение)
    D3DXVECTOR3 m_ChassisCamForward = { 0, 0, 0 }; //Вектор условного направления шасси робота, к которому линкуется положение камеры в ручном контроле (используется для осуществления стрейфов)
	D3DXVECTOR3 m_HullForward = { 0, 0, 0 };       //Вектор текущего направления корпуса робота (его текущее физическое положение)

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//Steering behavior's try
    D3DXVECTOR3 m_Velocity = { 0, 0, 0 }; //вектор скорости, длина равна скорости, направление движения

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

    CVOShadowProj* m_ShadowProj = nullptr;

	ERobotState    m_CurrState = ROBOT_IN_SPAWN;

    float          m_FallingSpeed = 0.0f;

    CMatrixFlyer*  m_CargoFlyer = nullptr;
    float          m_KeelWaterCount = 0.0f;

    int m_MiniMapFlashTime = 0;

    CMatrixRobot() : CMatrixMapStatic()
    {
        m_Core->m_Type = OBJECT_TYPE_ROBOTAI;

        m_ProgressBar.Modify(1000000, 0, PB_ROBOT_WIDTH, 1);

        if(g_PlayerRobotsAutoBoom == 1) m_AutoBoom = true; //Если подключён мод на автоподрыв (и в конфиге выставлено включение подрыва по умолчанию), то по умолчанию он будет включён
    }
    ~CMatrixRobot()
    {
        ModuleClear();
        if(m_ShadowProj)
        {
            HDelete(CVOShadowProj, m_ShadowProj, g_MatrixHeap);
            m_ShadowProj = nullptr;
        }
    }

    void ShowHitpoint() { m_ShowHitpointTime = HITPOINT_SHOW_TIME; }
    //Добавил функцию, чтобы иметь возможность чинить роботов напрямую из логики зданий, Klaxons
    void ModifyHitpoints(int num)
    {
        m_HitPoint = max(min(m_HitPoint + num, m_HitPointMax), 0);
        m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);
    }

    float GetHitPoint() const { return m_HitPoint / 10; }
    float GetMaxHitPoint() { return m_HitPointMax / 10; }
    void  InitMaxHitpoint(float hp) { m_HitPoint = hp; m_HitPointMax = hp; m_MaxHitPointInversed = 1.0f / hp; }

    void MarkCrazy() { SETFLAG(m_ObjectFlags, ROBOT_CRAZY); }
    void UnMarkCrazy() { RESETFLAG(m_ObjectFlags, ROBOT_CRAZY); }
    bool IsCrazy() const { return FLAG(m_ObjectFlags, ROBOT_CRAZY); }

    //Каков тип двигательной части шасси: шагающий, колёсный, парящий (при желании типы можно (но не нужно) комбинировать, если, к примеру, нужно создать робота, шагающего по воде)
    bool IsWalkingChassis() { return g_Config.m_RobotChassisConsts[m_Module[0].m_Kind].is_walking; }
    bool IsRollingChassis() { return g_Config.m_RobotChassisConsts[m_Module[0].m_Kind].is_rolling; }
    bool IsHoveringChassis() { return g_Config.m_RobotChassisConsts[m_Module[0].m_Kind].is_hovering; }

    void MarkInPosition() { SETFLAG(m_ObjectFlags, ROBOT_FLAG_IN_POSITION); }
    void UnMarkInPosition() { RESETFLAG(m_ObjectFlags, ROBOT_FLAG_IN_POSITION); }
    bool IsInPosition() const { return FLAG(m_ObjectFlags, ROBOT_FLAG_IN_POSITION); }

    bool IsMustDie() const { return FLAG(m_ObjectFlags, ROBOT_MUST_DIE_FLAG); }
    void MustDie() { SETFLAG(m_ObjectFlags, ROBOT_MUST_DIE_FLAG); }
    void ResetMustDie() { RESETFLAG(m_ObjectFlags, ROBOT_MUST_DIE_FLAG); }

    void MarkCaptureInformed() { SETFLAG(m_ObjectFlags, ROBOT_CAPTURE_INFORMED); }
    void UnMarkCaptureInformed() { RESETFLAG(m_ObjectFlags, ROBOT_CAPTURE_INFORMED); }
    bool IsCaptureInformed() const { return FLAG(m_ObjectFlags, ROBOT_CAPTURE_INFORMED); }


    void SetBase(CMatrixBuilding* b) { m_Base = b; m_TimeWithBase = 0; }
    CMatrixBuilding* GetBase() const { return m_Base; }

    EAnimation GetAnimation() { return m_Animation; }
    void SwitchAnimation(EAnimation a);

    bool Carry(CMatrixFlyer* cargo, bool quick_connect = false); // nullptr to off
    void ClearSelection(void);

    static void BuildWalkingChassisData(CVectorObject* vo, int chassis_num);
    static void DestroyWalkingChassisData(void);
    void StepLinkWalkingChassis(void);
    void FirstStepLinkWalkingChassis(void);

    float GetChassisHeight(void) const;
    float GetEyeLevel(void) const;
    float GetHullHeight(void) const;

    float Z_From_Pos(void);

    void ApplyNaklon(const D3DXVECTOR3& dir);

	void ModuleInsert(int before_module, ERobotModuleType type, ERobotModuleKind kind);
	void WeaponInsert(int before_module, ERobotModuleType type, ERobotModuleKind kind, int hull_num, int model_pilon_num);
    void ModuleDelete(int module_num);
	void ModuleClear(void);

    void BoundGet(D3DXVECTOR3& bmin, D3DXVECTOR3& bmax);

    void DoAnimation(int cms);

    virtual bool TakingDamage(int weap, const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, int attacker_side = NEUTRAL_SIDE, CMatrixMapStatic* attaker = nullptr) = 0;
    virtual void GetResources(dword need);

	virtual void Tact(int cms);
    virtual void LogicTact(int cms) = 0;

    virtual bool Pick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt)  const;
    bool         PickFull(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt)  const;

	virtual void BeforeDraw(void);
	virtual void Draw(void);
	virtual void DrawShadowStencil(void);
	virtual void DrawShadowProj(void);

    virtual void FreeDynamicResources(void);

    void OnLoad(void) {};

    virtual bool CalcBounds(D3DXVECTOR3& omin, D3DXVECTOR3& omax);
    virtual int  GetSide(void) const { return m_Side; };
    virtual bool NeedRepair(void) const { return m_HitPoint < m_HitPointMax; }
    virtual bool InRect(const CRect& rect)const;

    void OnOutScreen(void) {};

    bool AutoBoomSet()         { return m_AutoBoom; }
    void AutoBoomSet(bool set) { m_AutoBoom = set; }
};

__forceinline bool CMatrixMapStatic::IsRobotAlive(void) const
{
    return IsRobot() && ((CMatrixRobot*)this)->m_CurrState != ROBOT_DIP;
}