// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_FLYER_INCLUDE
#define MATRIX_FLYER_INCLUDE

#include "MatrixMapStatic.hpp"
#include "MatrixProgressBar.hpp"

class CMatrixEffectSelection;

#define FLYER_ALT_EMPTY     70
#define FLYER_ALT_CARRYING  110
#define FLYER_ALT_MIN       20

#define FLYER_MOUSE_DEAD_AREA_TOP      0.3f            // 0.2 of screen width
#define FLYER_MOUSE_DEAD_AREA_BOTTOM   0.05f            // 0.0 of screen width

#define FLYER_SEEKBASE_MATCH_RADIUS    10
#define FLYER_TARGET_MATCH_RADIUS      50
#define FLYER_FIRETARGET_MATCH_RADIUS  20

#define FLYER_SPINUP_TIME              3000 //в оригинале 3000, время до полного раскручивания винта перед взлётом

//Эти параметры устарели, вместо них теперь массив по разным типам вертушек
//#define FLYER_MIN_SPEED 0.002f
#define FLYER_MAX_SPEED                0.2f
#define FLYER_MAX_BACK_SPEED           0.10f
#define FLYER_MAX_STRAFE_SPEED         0.12f

#define FLYER_MAX_FIRE_SPEED           0.8f
#define FLYER_MAX_CLIMB_SPEED          0.1f

#define FLYER_RADIUS                   15


#define ENGINE_ANGLE_BREAK  GRAD2RAD(179.9)
#define ENGINE_ANGLE_STAY   GRAD2RAD(90)
#define ENGINE_ANGLE_MOVE   GRAD2RAD(0)

//#define FLYER_WEAPON1_HFOV        GRAD2RAD(90)
//#define FLYER_WEAPON1_UP_ANGLE    GRAD2RAD(0)
//#define FLYER_WEAPON1_DOWN_ANGLE  GRAD2RAD(85)

#define FLYER_SELECTION_HEIGHT 2
#define FLYER_SELECTION_SIZE 20

class CMatrixSideUnit;
class CMatrixEffectWeapon;
class CMatrixRobot;
class CMatrixBuilding;
class CMatrixEffectElevatorField;
class CMatrixEffectFireStream;

enum EWeapon;

enum EFlyerUnitType
{
    FLYER_UNIT_BODY = 0,
    FLYER_UNIT_PROPELLER = 1,
    FLYER_UNIT_ENGINE = 2,
    FLYER_UNIT_WEAPON = 3,
    FLYER_UNIT_WEAPON_HOLLOW = 4
};

enum EFlyerKind
{
    FLYER_SPEED = 0,      //Разведчик
    FLYER_ATTACK = 1,     //Ударный
    FLYER_TRANSPORT = 2,  //Транспорт
    FLYER_BOMB = 3,       //Бомбардировщик
};

struct SFlyerSpecifications
{
    float forward_speed;
    float reverse_speed;
    float strafe_speed;
    float climb_speed; //Как вниз так и вверх

    //В радианах
    float pitch_angle_stay; //Угол тангажа во время ожидания на месте
    float pitch_angle_move; //Угол тангажа при движении вертолёта вперед/назад
    float pitch_angle_break; //Угол тангажа при торможении вертолёта
    float max_roll_angle; //Максимально допустимый угол крена во время разворотов вертолёта
};

//Технические характеристики разных моделей вертолётов (костыли, чтобы не дублировать объект по всем юнитам кода)
extern const SFlyerSpecifications FlyerSpecifications[];
#ifdef FLYER_SPECIFICATIONS
const SFlyerSpecifications FlyerSpecifications[] =
{
    { 0.25, 0.15, 0.15, 0.07,   GRAD2RAD(0), GRAD2RAD(-20), GRAD2RAD(70), GRAD2RAD(75) }, //Разведчик
    { 0.20, 0.10, 0.12, 0.05,   GRAD2RAD(0), GRAD2RAD(-18), GRAD2RAD(60), GRAD2RAD(64) }, //Ударный
    { 0.15, 0.07, 0.08, 0.03,   GRAD2RAD(0), GRAD2RAD(-17), GRAD2RAD(40), GRAD2RAD(40) }, //Транспорт
    { 0.17, 0.08, 0.10, 0.04,   GRAD2RAD(0), GRAD2RAD(-17), GRAD2RAD(50), GRAD2RAD(50) }  //Бомбардировщик
};
#endif

class CMatrixFlyer;

struct SMatrixFlyerUnit
{
    EFlyerUnitType      m_Type;
    CVectorObjectAnim*  m_Graph;
    CVOShadowStencil*   m_ShadowStencil;
    D3DXMATRIX          m_Matrix;
    D3DXMATRIX          m_IMatrix;

    union
    {
        struct
        {
            int               m_MatrixID; // matrix id
            D3DMATRIX         m_Matrix;
            CTextureManaged*  m_Tex;

            dword             m_Inversed;
	        float             m_Angle;
            float             m_AngleSpeedMax;

            int m_SpinupCountdown; //Если винт на старте сложен, то перед раскруткой необходимо будет его развернуть
        } m_Propeller;

        struct
        {
            CMatrixEffectWeapon* m_Weapon;
            int m_MatrixID; // matrix id

            union
            {
                struct
                {
                    dword m_Inversed;
	                float m_AngleZ;
	                float m_AngleX;
                    float m_HFOV;
                    float m_UpAngle;
                    float m_DownAngle;
                };

                struct
                {
                    int m_Module;

                };
            };

        } m_Weapon;

        struct
        {
            int   m_MatrixID; // matrix id
            dword m_Inversed;
	        float m_Angle;
        } m_Engine;

    };

    void Release(void);
    bool Tact(CMatrixFlyer* owner, float ms);
};

#define MF_TARGETMOVE               SETBIT(0)
#define MF_TARGETFIRE               SETBIT(1)
//#define MF_SEEKBASE               SETBIT(2)
//#define MF_SEEKBASEOK             SETBIT(3)
//#define MF_SEEKBASEFOUND          SETBIT(4)

#define FLYER_ACTION_MOVE_FORWARD   SETBIT(5)
#define FLYER_ACTION_MOVE_BACKWARD  SETBIT(6)
#define FLYER_ACTION_MOVE_LEFT      SETBIT(7)
#define FLYER_ACTION_MOVE_RIGHT     SETBIT(8)
#define FLYER_ACTION_ROT_LEFT       SETBIT(9)
#define FLYER_ACTION_ROT_RIGHT      SETBIT(10)
#define FLYER_ACTION_CLIMB_UP       SETBIT(11)
#define FLYER_ACTION_CLIMB_DOWN     SETBIT(12)
#define FLYER_MANUAL                SETBIT(13) //Маркер выставления одной из ручных команд управления вертолётом (любой из них)

#define FLYER_BODY_MATRIX_DONE      SETBIT(14)
#define FLYER_IN_SPAWN              SETBIT(15)
#define FLYER_IN_SPAWN_SPINUP       SETBIT(16) //Вертолёт только выкатился с базы и раскручивает лопасти
#define FLYER_BREAKING              SETBIT(17) // тормозит (в стратегическом режиме сход с траектории)
#define FLYER_SGROUP                SETBIT(18)
#define FLYER_SARCADE               SETBIT(19)

enum EFlyerState
{
    STATE_FLYER_IN_SPAWN,
    STATE_FLYER_BASE_TAKE_OFF,
    STATE_FLYER_READY_TO_ORDERS, //Успешно построен и ожидает приказов
    STATE_FLYER_CARRYING_ROBOT,
    STATE_FLYER_IS_BUSY, //Выставляется для вертолётов поддержки
    STATE_FLYER_DIP //Запущен процесс уничтожения вертолёта (death in progress)
};

struct SFlyerTactData;

enum EFlyerOrder
{
    FO_GIVE_BOT,
    FO_FIRE,
};

struct SFireStream
{
    union
    {
        CMatrixEffectFireStream* effect;
        CBlockPar* bp;
    };

    int                      matrix;    // matrix id
    int                      unit;      // unit index
};

class CMatrixFlyer : public CMatrixMapStatic
{
    static D3D_VB       m_VB;
    static int          m_VB_ref;

    int                 m_Side = 0;

    SMatrixFlyerUnit*   m_Modules = nullptr;
    int                 m_ModulesCount = 0;
    int                 m_EngineUnit = -1;

    SFireStream*        m_Streams = nullptr;
    int                 m_StreamsCount = 0;
    float               m_StreamLen = 10;

	D3DXVECTOR2         m_Target = { 200.0f, 280.0f };
    D3DXVECTOR3         m_FireTarget = { 0.0f, 0.0f, 0.0f };

    dword               m_Sound = SOUND_ID_EMPTY;

    SEffectHandler      m_Ablaze;

    //union
    //{
	//float m_TargetAlt = FLYER_ALT_EMPTY;
    CMatrixBuilding* m_Base = nullptr;
    //};

	D3DXVECTOR3 m_Pos = { 200.0f, 280.0f, FLYER_ALT_EMPTY }; //origin

    //Все углы представлены в радианах
    float m_DAngle = 0.0f;
    float m_AngleZ = 0.0f;
    float m_AngleZSin = 0.0f;
    float m_AngleZCos = 0.0f;

    float m_RotZSpeed = 0.0f;
	float m_MoveSpeed = 0.0f;
    float m_StrafeSpeed = 0.0f; // <0 - left, >0 rite

    float m_Pitch = 0.0f; //Угол тангажа, при нормальном положении вертолёта равен 0, наклон вперёд - , наклон назад +
	float m_Roll = 0.0f; //Угол крена, при нормальном положении вертолёта равен 0, крен влево + , крен вправо -

    float m_AltitudeCorrection = 0.0f; //Значение для управляемой корректировки высоты полёта (суммируется с автоматической базовой высотой)

    float m_TargetEngineAngle = ENGINE_ANGLE_STAY;
    float m_TargetPitchAngle = 0.0f;
    float m_TargetRollAngle = 0.0f;

    //float          m_BaseLandAngle;
    //D3DXVECTOR3    m_BaseLandPos;
    CTrajectory* m_Trajectory = nullptr;
    union
    {
        struct // in breaking mode
        {
            D3DXVECTOR3 m_StoreTarget;
        };
        struct
        {
            float m_TrajectoryPos; // [0..1]
            float m_TrajectoryLen; 
            float m_TrajectoryLenRev; 
            float m_TrajectoryTargetAngle;
        };
    };

    int m_TgtUpdateCount = 0;

    int m_NextTimeAblaze = 0;
    int m_LastDelayDamageSide = 0;

    CWStr m_Name = (CWStr)L"FLYER";

    //hitpoint
    CMatrixProgressBar m_ProgressBar;
    int         m_ShowHitpointTime = 0;
    float       m_HitPoint = 0.0f;
	float       m_HitPointMax = 0.0f;  // Максимальное кол-во здоровья
    float       m_MaxHitPointInversed = 0.0f; // for normalized calcs

    CTextureManaged* m_BigTexture = nullptr;
    CTextureManaged* m_MedTexture = nullptr;
    CTextureManaged* m_SmallTexture = nullptr;

    void CalcMatrix(void);
    void CalcBodyMatrix(void);
    void LogicTactArcade(SFlyerTactData& td);
    void LogicTactStrategy(SFlyerTactData& td);
    bool LogicTactOrder(SFlyerTactData& td);

    void CalcCollisionDisplace(SFlyerTactData& td);

    //void CalcTrajectory(const D3DXVECTOR3 &target, const D3DXVECTOR3 &dir);
    void CalcTrajectory(const D3DXVECTOR3& target);
    void ProceedTrajectory(SFlyerTactData& td);
    void CancelTrajectory(void);
    bool IsTrajectoryEnd(void) const { return m_TrajectoryPos >= 0.99f; }

    float CalcFlyerZInPoint(float x, float y);

    CMatrixEffectSelection* m_Selection = nullptr;
    int m_CtrlGroup = 0;

public:
    //carrying
    struct SCarryData
    {
        CMatrixRobot* m_Robot = nullptr;
        D3DXVECTOR3   m_RobotForward = { 0.0f, 0.0f, 0.0f };
        D3DXVECTOR3   m_RobotUp = { 0.0f, 0.0f, 0.0f };
        D3DXVECTOR3   m_RobotUpBack = { 0.0f, 0.0f, 0.0f };
        float         m_RobotAngle = 0.0f;
        float         m_RobotMassFactor = 0.0f;
        CMatrixEffectElevatorField* m_RobotElevatorField = nullptr;
    } m_CarryData;

    dword m_Flags = MF_TARGETFIRE | FLYER_IN_SPAWN;

    friend struct SMatrixFlyerUnit;

    int m_Team = 0;
    int m_Group = 0;

    EFlyerState m_CurrState = STATE_FLYER_IN_SPAWN;

    bool        SelectByGroup();
    bool        SelectArcade();
    void        UnSelect();
    bool        IsSelected();
    CMatrixEffectSelection* GetSelection()  { return m_Selection; }
    bool        CreateSelection();
    void        KillSelection();
    void        MoveSelection();
    CWStr       GetName()                   { return m_Name; }
    void        SetName(const CWStr& name)  { m_Name = name; }
    int         GetCtrlGroup()              { return m_CtrlGroup; } 
    void        SetCtrlGroup(int group)     { m_CtrlGroup = group; }

    CMatrixBuilding* GetBase() const    { return m_Base; }

    bool        IsDeliveryCopter() const    { return FLAG(m_ObjectFlags, FLYER_FLAG_DELIVERY_COPTER); }
    void        SetDeliveryCopter(bool set) { INITFLAG(m_ObjectFlags, FLYER_FLAG_DELIVERY_COPTER, set); }

    void        SetSide(int side) { m_Side = side; }

    EFlyerKind  m_FlyerKind = FLYER_SPEED;

    static void StaticInit()
    {
        m_VB = nullptr;
        m_VB_ref = 0;
    }

    static void MarkAllBuffersNoNeed();
    static void InitBuffers();

    CMatrixFlyer();
    ~CMatrixFlyer();

    void ApplyOrder(const D3DXVECTOR2& pos, int side, EFlyerOrder order, float ang, int place, const CPoint& bpos, int robot_template);

    // carry robot
    bool IsCarryingRobot() const { return m_CarryData.m_Robot != nullptr; }
    CMatrixRobot* GetCarryingRobot() { return m_CarryData.m_Robot; }
    CMatrixFlyer::SCarryData* GetCarryData() { return &m_CarryData; };

    void  ShowHitpoint() { m_ShowHitpointTime = HITPOINT_SHOW_TIME; }
    float GetHitPoint() const { return m_HitPoint; }
    float GetMaxHitPoint() { return m_HitPointMax; }
    void  InitMaxHitpoint(float hp) { m_HitPoint = hp; m_HitPointMax = hp; m_MaxHitPointInversed = 1.0f / hp; }

    void SetHitpoint(float hp) { m_HitPoint = hp; }

    const D3DXVECTOR3& GetPos() const { return m_Pos; } //Возвращает текущие фактические координаты вертолёта по всем трём осям
    const D3DXVECTOR3  GetPos(float ahead_to) const { return GetPos() + D3DXVECTOR3(-m_AngleZSin, m_AngleZCos, 0.0f) * ahead_to; } //Возвращает координатам на удалении, равном значению аргумента, по направлению вперёд ровно от носа вертолёта, можно использовать для расчёта вектора прямолинейного движения
    const D3DXVECTOR3  GetForward(float ahead_to) const { return D3DXVECTOR3(-m_AngleZSin, m_AngleZCos, 0.0f) * ahead_to; } //Возвращает указанную в аргументе прибавку к координатам (не точные координаты, а именно плюс к фактической m_Pos) ровно перед носом вертолёта, по которым считается вектор направления и которые можно использовать, чтобы отдавать команду двигаться строго вперёд

    const D3DXVECTOR2  GetTarget() const { return m_Target; }
    //void             SetAlt(float alt) { m_TargetAlt = alt; }

    //Угол направления носа вертолёта
    float GetDirectionAngle() const { return m_AngleZ; };
    void SetDirectionAngle(float a)
    {
        a = (float)AngleNorm(a);
        if(a != m_AngleZ)
        {
            m_AngleZ = a;
            SinCos(a, &m_AngleZSin, &m_AngleZCos);
        }
    }
    //Для внешних вызовов
    void SetRollAngle(float angle) { m_TargetRollAngle = angle; }

    void SetTarget(const D3DXVECTOR2& tgt);
    void SetFireTarget(const D3DXVECTOR3& tgt)
    {
        //if(!FLAG(m_Flags, MF_SEEKBASE))
        //{
        m_FireTarget = tgt;
        RESETFLAG(m_Flags, MF_TARGETFIRE);
        //}
    };

    float GetSpeed(void) const { return m_MoveSpeed; }
    float GetSpeedNorm(void) const { return m_MoveSpeed / FlyerSpecifications[m_FlyerKind].forward_speed; }

    void MoveForward(void) { SETFLAG(m_Flags, FLYER_ACTION_MOVE_FORWARD); }
    void MoveBackward(void) { SETFLAG(m_Flags, FLYER_ACTION_MOVE_BACKWARD); }
    void MoveLeft(void) { SETFLAG(m_Flags, FLYER_ACTION_MOVE_LEFT); }
    void MoveRight(void) { SETFLAG(m_Flags, FLYER_ACTION_MOVE_RIGHT); }
    void RotLeft(void) { SETFLAG(m_Flags, FLYER_ACTION_ROT_LEFT); }
    void RotRight(void) { SETFLAG(m_Flags, FLYER_ACTION_ROT_RIGHT); }
    void MoveUp(const SFlyerSpecifications& corr) { m_AltitudeCorrection += corr.climb_speed;/*SETFLAG(m_Flags, FLYER_ACTION_CLIMB_UP);*/ }
    void MoveUp() { m_AltitudeCorrection += FLYER_MAX_CLIMB_SPEED; }
    void MoveDown(const SFlyerSpecifications& corr) { m_AltitudeCorrection -= corr.climb_speed;/*SETFLAG(m_Flags, FLYER_ACTION_CLIMB_DOWN);*/ }
    void MoveDown() { m_AltitudeCorrection -= FLYER_MAX_CLIMB_SPEED; }

    void FireBegin(void);
    void FireEnd(void);

    //void DownToBase(CMatrixBuilding* building);

    void Begin(CMatrixBuilding* b);

    void DrawPropeller(void);

    void ReleaseMe();

    void CreateTextures();
    
    CTextureManaged* GetBigTexture()   { return m_BigTexture; }
    CTextureManaged* GetMedTexture()   { return m_MedTexture; }
    CTextureManaged* GetSmallTexture() { return m_SmallTexture; }

    void CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type);
    void DeleteProgressBarClone(EPBCoord clone_type);

    virtual bool TakingDamage(int weap, const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, int attacker_side = 0/*NEUTRAL_SIDE*/, CMatrixMapStatic* attaker = nullptr);
	virtual void GetResources(dword need); // Запрашиваем нужные ресурсы объекта

	virtual void Tact(int cms);
    virtual void LogicTact(int cms);

    virtual bool Pick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt) const;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void DrawShadowStencil(void);
    virtual void DrawShadowProj(void) {};

    virtual void FreeDynamicResources(void);

    virtual void Load(CBuf& buf, CTemporaryLoadData* td) {};

    virtual bool CalcBounds(D3DXVECTOR3& omin, D3DXVECTOR3& omax);
    virtual int GetSide(void) const { return m_Side; };
    virtual bool NeedRepair(void) const { return m_HitPoint < m_HitPointMax; }

    virtual bool InRect(const CRect& rect)const;

    void OnOutScreen(void) {};
};

__forceinline bool CMatrixMapStatic::IsFlyerControllable(void) const
{
    return IsFlyer() && ((CMatrixFlyer*)this)->m_CurrState != STATE_FLYER_DIP && !FLAG(m_ObjectFlags, FLYER_FLAG_DELIVERY_COPTER);
}

#endif