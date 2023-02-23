// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

class CMatrixMapGroup;
typedef CMatrixMapGroup* PCMatrixMapGroup;

//#define SHOW_ASSIGNED_GROUPS    6
//#define SHOW_ASSIGNED_GROUPS    3

#include "Effects/MatrixEffect.hpp"

// Ресурс объекта
#define MR_Graph                SETBIT(0)       // Графика
#define MR_Matrix               SETBIT(1)       // Матрица
#define MR_Pos                  SETBIT(2)       // Положение
#define MR_Rotate               SETBIT(3)       // Ориентация
#define MR_ShadowStencil        SETBIT(4)       // Стенсильные тени
//#define MR_ShadowProj         SETBIT(5)       // Проекционные тени
#define MR_ShadowProjGeom       SETBIT(6)       // Проекционные тени: геометрия
#define MR_ShadowProjTex        SETBIT(7)       // Проекционные тени: текстуры
#define MR_MiniMap              SETBIT(8)       // Присутствие на миникарте

//#define MR_GraphSort         SETBIT(1)       // Сортировка графики

typedef enum
{
    OBJECT_TYPE_EMPTY       = 0,
    OBJECT_TYPE_MAPOBJECT   = 2,
    OBJECT_TYPE_ROBOTAI     = 3,
    OBJECT_TYPE_BUILDING    = 4,
    OBJECT_TYPE_CANNON      = 5,
    OBJECT_TYPE_FLYER       = 6,

} EObjectType;

#define MAX_OBJECTS_PER_SCREEN 5120

#define UNDER_ATTACK_IDLE_TIME 120000

#define OBJECT_STATE_INVISIBLE           SETBIT(0)   // невидимый
#define OBJECT_STATE_INTERFACE           SETBIT(1)   // рисуется в интерфейсе

#define OBJECT_STATE_INVULNERABLE        SETBIT(2)
#define OBJECT_STATE_SHADOW_SPECIAL      SETBIT(3)   // параметры для расчета тени загружаются
#define OBJECT_STATE_TRACE_INVISIBLE     SETBIT(4)   // объект невидим для TRACE_SKIP_INVISIBLE объектов
#define OBJECT_STATE_DIP                 SETBIT(5)   // используется в StaticDelete

// common flags

// only mesh flags
#define OBJECT_STATE_BURNED              SETBIT(10)   // сгоревший (для мешей)
#define OBJECT_STATE_EXPLOSIVE           SETBIT(11)   // ломается со взрывом (для мешей)
#define OBJECT_STATE_NORMALIZENORMALS    SETBIT(12)   // нормализовывать нормали (для мешей)
#define OBJECT_STATE_SPECIAL             SETBIT(13)   // специальный объект: смерть требуется для победы
#define OBJECT_STATE_TERRON_EXPL         SETBIT(14)   // террон взрывается
#define OBJECT_STATE_TERRON_EXPL1        SETBIT(15)   // террон взрывается 1
#define OBJECT_STATE_TERRON_EXPL2        SETBIT(16)   // террон взрывается 2

//Флаги только для роботов
#define ROBOT_FLAG_MOVING_BACK           SETBIT(10)
#define ROBOT_FLAG_COLLISION             SETBIT(11) //if collision, pneumatic chassis does not steps well (я не особо понял, что значит эта хуйня, но явно оно не имеет отношения к общей коллизии)
#define ROBOT_FLAG_SGROUP                SETBIT(12)
#define ROBOT_FLAG_SARCADE               SETBIT(13)
#define ROBOT_FLAG_ON_WATER              SETBIT(14)
#define ROBOT_FLAG_LINKED                SETBIT(15)
#define ROBOT_FLAG_ROT_LEFT              SETBIT(16)
#define ROBOT_FLAG_ROT_RIGHT             SETBIT(17)
#define ROBOT_CRAZY                      SETBIT(18)  // easter egg :) crazy bot
#define ROBOT_FLAG_IN_POSITION           SETBIT(19)  // easter egg (скрытая комната с фото разработчиков на карте с названием terron)
#define ROBOT_MUST_DIE_FLAG              SETBIT(20)
#define ROBOT_CAPTURE_INFORMED           SETBIT(21)
#define ROBOT_FLAG_STRAFING              SETBIT(22)

//Флаги только для вертолётов
#define FLYER_FLAG_DELIVERY_COPTER       SETBIT(10) //Данный вертолёт состоит в группе доставки роботов и потому игрок им управлять не может

//Общие флаги для роботов и вертолётов
#define UNIT_FLAG_DISABLE_MANUAL         SETBIT(23) //Актуально как для робота, так и для вертолёта
#define UNIT_FLAG_NO_OBJECTS_COLLISION   SETBIT(24) //Отключает коллизию с интерактивным объектами (роботы, вертолёты, турели)
#define UNIT_FLAG_NO_OBSTACLES_COLLISION SETBIT(25) //Отключает коллизию со статичными препятствиями (здания, декорации)
#define UNIT_FLAG_REAR_VIEW              SETBIT(26) //Инвертирует направление камеры в аркадном режиме управления

//Флаги только для турелей
#define OBJECT_CANNON_REF_PROTECTION     SETBIT(10)
#define OBJECT_CANNON_REF_PROTECTION_HIT SETBIT(11)

//Флаги только для зданий
#define BUILDING_NEW_INCOME              SETBIT(10)
#define BUILDING_SPAWNING_UNIT           SETBIT(11) // opening for bot spawn
#define BUILDING_CAPTURE_IN_PROGRESS     SETBIT(12)



#define ABLAZE_LOGIC_PERIOD             90 //Частота срабатывания логики горения
#define SHORTED_OUT_LOGIC_PERIOD        50 //Частота срабатывания логики стана

#define HITPOINT_SHOW_TIME  1000

typedef enum
{
    SHADOW_OFF          = 0,
    SHADOW_PROJ_STATIC  = 1,
    SHADOW_PROJ_DYNAMIC = 2,
    SHADOW_STENCIL      = 3

} EShadowType;

enum EWeapon;

#define MAX_GROUPS_PER_OBJECT 36


class CTemporaryLoadData;

enum 
{
    OBJ_RENDER_ORDINAL,
    OBJ_RENDER_ORDINAL_GLOSS,
    OBJ_RENDER_SIDE,
    OBJ_RENDER_SIDE_GLOSS,

    OBJ_RENDER_TYPES_CNT
};

class CMatrixMapStatic;

struct SObjectCore
{
	D3DXMATRIX   m_Matrix;
    D3DXMATRIX   m_IMatrix; //Inverted matrix
    float        m_Radius = 0.0f;
    D3DXVECTOR3  m_GeoCenter = { 0.0f, 0.0f, 0.0f };
    EObjectType  m_Type = OBJECT_TYPE_EMPTY; // 0 - empty, 2 - CMatrixMapObject, 3 - CMatrixRobotAI, 4 - CMatrixBuilding, 5 - CMatrixCannon
    dword        m_TerainColor = 0;
    int          m_Ref = 0;

    CMatrixMapStatic* m_Object = nullptr;

    static SObjectCore* Create(CMatrixMapStatic* obj)
    {
        SObjectCore* c = (SObjectCore*)HAlloc(sizeof(SObjectCore), g_MatrixHeap);

        c->m_Object = obj;
        c->m_Ref = 1;
        D3DXMatrixIdentity(&c->m_Matrix);

        return c;
    }

    void RefInc() { ++m_Ref; }
    void RefDec()
    {
        --m_Ref;
        if(m_Ref <= 0)
        {
            HFree(this, g_MatrixHeap);
        }
    }

    void Release() { RefDec(); }
};

bool FreeObjResources(dword user);

struct SRenderTexture
{
    ETexSize ts = TEXSIZE_512;
    CTextureManaged* tex = nullptr;
};

class CMatrixRobotAI;
class CMatrixCannon;
class CMatrixBuilding;
class CMatrixFlyer;

class CMatrixMapStatic;
typedef CMatrixMapStatic* PCMatrixMapStatic;
class CMatrixMapStatic : public CMain
{
    SRemindCore m_RemindCore;

    int m_ObjectStateTTLAblaze = 0;
    int m_ObjectStateTTLShorted = 0;

    int m_Z;

    static PCMatrixMapStatic objects[MAX_OBJECTS_PER_SCREEN];
    static int               objects_left;
    static int               objects_rite;


    static CMatrixMapStatic* m_FirstVisNew;
    static CMatrixMapStatic* m_LastVisNew;

    static CMatrixMapStatic* m_FirstVisOld;
    static CMatrixMapStatic* m_LastVisOld;

    CMatrixMapStatic* m_NextVis;
    CMatrixMapStatic* m_PrevVis;

protected:
    struct SEVH_data
    {
        D3DXMATRIX   m = {};
        const CRect* rect = nullptr;
        bool         found = false;
    };

    dword        m_ObjectFlags = 0;      //Битовый набор различных флагов
    dword        m_RChange = 0xffffffff; //Какой ресурс объекта изменился

    SObjectCore* m_Core = nullptr;

    void SetShortedTTL(int ttl) { m_ObjectStateTTLShorted = ttl; };
    void SetAblazeTTL(int ttl) { m_ObjectStateTTLAblaze = ttl; };
    int  GetShortedTTL() { return m_ObjectStateTTLShorted; }
    int  GetAblazeTTL() { return m_ObjectStateTTLAblaze; }
    int  IsAblaze() const { return m_AblazeEffectNum; }
    int  IsShorted() const { return m_ShortedOutEffectNum; }
    void MarkAblaze(int effect_num) { m_AblazeEffectNum = effect_num; }
    void MarkShorted(int effect_num) { m_ShortedOutEffectNum = effect_num; }
    void UnmarkAblaze() { m_AblazeEffectNum = 0; }
    void UnmarkShorted() { m_ShortedOutEffectNum = 0; }

    int m_AblazeEffectNum = 0;
    int m_ShortedOutEffectNum = 0;

    static CMatrixMapStatic* m_FirstLogicTemp;
    static CMatrixMapStatic* m_LastLogicTemp;
    CMatrixMapStatic* m_PrevLogicTemp;
    CMatrixMapStatic* m_NextLogicTemp;

    PCMatrixMapGroup m_InGroups[MAX_GROUPS_PER_OBJECT];    // max size of object is MAX_GROUPS_PER_OBJECT groups
    int              m_InCnt;


    float           m_CamDistSq;    // квадрат растояния до камеры. вычисляется только в графическом такте.

    int             m_NearBaseCnt;

    // for visibility
    void WillDraw()
    {
        // TODO : доделать!!!
        return;
        // remove from previous list of visibility
        LIST_DEL(this, m_FirstVisOld, m_LastVisOld, m_PrevVis, m_NextVis);
        // add to new list of visibility
        LIST_ADD(this, m_FirstVisNew, m_LastVisNew, m_PrevVis, m_NextVis);
    }

public:
    CMatrixMapStatic* m_NextQueueItem;
    CMatrixMapStatic* m_PrevQueueItem;
        
    bool IsNotOnMinimap() const { return FLAG(m_RChange, MR_MiniMap); }
    void SetInvulnerability() { SETFLAG(m_ObjectFlags, OBJECT_STATE_INVULNERABLE); }
    void ResetInvulnerability() { RESETFLAG(m_ObjectFlags, OBJECT_STATE_INVULNERABLE); }
    bool IsInvulnerable() const { return FLAG(m_ObjectFlags, OBJECT_STATE_INVULNERABLE); }
    bool IsTraceInvisible() const { return FLAG(m_ObjectFlags, OBJECT_STATE_TRACE_INVISIBLE); }
    bool IsSpecial() const { return FLAG(m_ObjectFlags, OBJECT_STATE_SPECIAL); }

    bool IsManualControlLocked() const { return FLAG(m_ObjectFlags, UNIT_FLAG_DISABLE_MANUAL); }
    void SetManualControlLocked(bool set) { INITFLAG(m_ObjectFlags, UNIT_FLAG_DISABLE_MANUAL, set); }
        
    bool IsNoObjectsCollision() const { return FLAG(m_ObjectFlags, UNIT_FLAG_NO_OBJECTS_COLLISION); }
    void SetNoObjectsCollision(bool set) { INITFLAG(m_ObjectFlags, UNIT_FLAG_NO_OBJECTS_COLLISION, set); }
    bool IsNoObstaclesCollision() const { return FLAG(m_ObjectFlags, UNIT_FLAG_NO_OBSTACLES_COLLISION); }
    void SetNoObstaclesCollision(bool set) { INITFLAG(m_ObjectFlags, UNIT_FLAG_NO_OBSTACLES_COLLISION, set); }

    bool IsRearView() { return FLAG(m_ObjectFlags, UNIT_FLAG_REAR_VIEW); } //Инвертирует направление камеры в аркадном режиме управления

    bool IsDIP() const { return FLAG(m_ObjectFlags, OBJECT_STATE_DIP); }
    void SetDIP() { SETFLAG(m_ObjectFlags, OBJECT_STATE_DIP); }

    static void StaticInit()
    {
        m_FirstLogicTemp = nullptr;
        m_LastLogicTemp = nullptr;
        objects_left = g_MaxObjectsPerScreen >> 1;
        objects_rite = g_MaxObjectsPerScreen >> 1;

        m_FirstVisNew = nullptr;
        m_LastVisNew = nullptr;

        m_FirstVisOld = nullptr;
        m_LastVisOld = nullptr;
    }
#ifdef _DEBUG
    static void ValidateAfterReset()
    {
        if(m_FirstLogicTemp || m_LastLogicTemp) _asm int 3
        if(objects_left != objects_rite) _asm int 3
        if(m_FirstVisNew || m_LastVisNew) _asm int 3
        if(m_FirstVisOld || m_LastVisOld) _asm int 3
    }
#endif

    void SetTerainColor(dword color) { m_Core->m_TerainColor = color; }

    void SetVisible(bool flag) { INITFLAG(m_ObjectFlags, OBJECT_STATE_INVISIBLE, !flag); }
    bool IsVisible() const { return !FLAG(m_ObjectFlags, OBJECT_STATE_INVISIBLE); }
    void SetInterfaceDraw(bool flag) { INITFLAG(m_ObjectFlags, OBJECT_STATE_INTERFACE, flag); }
    bool IsInterfaceDraw() const { return FLAG(m_ObjectFlags, OBJECT_STATE_INTERFACE); }
		
    PCMatrixMapGroup GetGroup(int n)    { return m_InGroups[n]; }
    int GetGroupCnt()               { return m_InCnt; }

    D3DXVECTOR3 m_AdditionalPoint; // to check visibility with shadows

    dword   m_LastVisFrame;
    int     m_IntersectFlagTracer;
    int     m_IntersectFlagFindObjects;

#pragma warning (disable : 4355)
    CMatrixMapStatic() :
        CMain(), m_LastVisFrame(0xFFFFFFFF), m_NearBaseCnt(0), m_IntersectFlagTracer(0xFFFFFFFF), m_IntersectFlagFindObjects(0xFFFFFFFF), m_InCnt(0), m_PrevLogicTemp(nullptr), m_NextLogicTemp(nullptr), m_RemindCore(FreeObjResources, (dword)this)
    {
        m_Core = SObjectCore::Create(this);

        m_Core->m_TerainColor = 0xFFFFFFFF;
        m_Core->m_Type = OBJECT_TYPE_EMPTY;

        memset(m_InGroups, 0, sizeof(m_InGroups));
            
        m_NextQueueItem = nullptr;
        m_PrevQueueItem = nullptr;
            
        m_NextVis = nullptr;
        m_PrevVis = nullptr;

        m_CamDistSq = 10000.0f;
    }
#pragma warning (default : 4355)

    ~CMatrixMapStatic();

    static CMatrixMapStatic* GetFirstLogic() { return m_FirstLogicTemp; }
    static CMatrixMapStatic* GetLastLogic() { return m_LastLogicTemp; }
    CMatrixMapStatic* GetNextLogic() { return m_NextLogicTemp; }
    CMatrixMapStatic* GetPrevLogic() { return m_PrevLogicTemp; }

    bool IsBase() const;
    __forceinline bool IsRobot() const { return GetObjectType() == OBJECT_TYPE_ROBOTAI; };
    bool IsRobotAlive() const;
    __forceinline bool IsFlyer() const { return GetObjectType() == OBJECT_TYPE_FLYER; };
    bool IsFlyerControllable() const;
    __forceinline bool IsBuilding() const { return GetObjectType() == OBJECT_TYPE_BUILDING; };
    bool IsBuildingAlive() const;
    __forceinline bool IsCannon() const { return GetObjectType() == OBJECT_TYPE_CANNON; };
    bool IsCannonAlive() const;
    bool IsActiveCannonAlive() const;
        
    __forceinline bool IsUnit() const { return IsRobot() || IsFlyer() || IsCannon(); }

    __forceinline bool IsAlive() const { return IsRobotAlive() || IsFlyerControllable() || IsCannonAlive() || IsBuildingAlive(); }
    /*{
        if(obj->GetObjectType() == OBJECT_TYPE_ROBOTAI) return obj->AsRobot()->m_CurrState != ROBOT_DIP;// && (obj->AsRobot()->GetSide() != PLAYER_SIDE || !obj->AsRobot()->IsSelected()) && (g_MatrixMap->GetPlayerSide()->GetArcadedObject() != obj);
        else if(obj->GetObjectType() == OBJECT_TYPE_CANNON) return obj->AsCannon()->m_CurrState != CANNON_DIP && obj->AsCannon()->m_CurrState != CANNON_UNDER_CONSTRUCTION;
        else if(obj->GetObjectType() == OBJECT_TYPE_BUILDING) return (obj->AsBuilding()->m_State != BUILDING_DIP) && (obj->AsBuilding()->m_State != BUILDING_DIP_EXPLODED);
        else return false;
    }*/

    bool FitToMask(dword mask);

    __forceinline CMatrixRobotAI* AsRobot() { return (CMatrixRobotAI*)this; }
    __forceinline CMatrixCannon* AsCannon() { return (CMatrixCannon*)this; }
    __forceinline CMatrixBuilding* AsBuilding() { return (CMatrixBuilding*)this; }
    __forceinline CMatrixFlyer* AsFlyer() { return (CMatrixFlyer*)this; }

    __forceinline bool IsNearBase() const { return m_NearBaseCnt != 0; }

    void RecalcTerainColor();
    __forceinline dword GetTerrainColor() const { return m_Core->m_TerainColor; }

    void        Sort(const D3DXMATRIX& sort);
    static void SortBegin();
    static void OnEndOfDraw(); // this must be called before any sorting stuf

    static void SortEndRecalcTerainColor();
    static void SortEndDraw();
    static void SortEndBeforeDraw();
    static void SortEndDrawShadowProj();
    static void SortEndDrawShadowStencil();
    static void SortEndGraphicTact(int step);

    static void CalcDistances();

    static void RemoveFromSorted(CMatrixMapStatic* ms);

    static int GetVisObjCnt();
    static CMatrixMapStatic* GetVisObj(int i);

    __forceinline EObjectType GetObjectType() const { return m_Core->m_Type; }
    __forceinline const D3DXVECTOR3& GetGeoCenter() const { return m_Core->m_GeoCenter; }
    __forceinline const D3DXMATRIX& GetMatrix() const { return m_Core->m_Matrix; }
    __forceinline float GetRadius() const { return m_Core->m_Radius; }

#ifdef _DEBUG
        SObjectCore* GetCore(SDebugCallInfo& dci);
#else
#ifdef _TRACE
    __forceinline SObjectCore* GetCore(SDebugCallInfo&) { m_Core->RefInc(); return m_Core; }
#else
    __forceinline SObjectCore* GetCore() { m_Core->RefInc(); return m_Core; }
#endif
#endif

    // logic temp: list of static objects, у которых временно вызывается логический такт
    __forceinline bool InLT(void)
    {
        return (m_PrevLogicTemp != nullptr) || (m_NextLogicTemp != nullptr) || (this == m_FirstLogicTemp);
    }
    __forceinline void AddLT(void)
    {
        if(!InLT())
        {
            LIST_ADD(this, m_FirstLogicTemp, m_LastLogicTemp, m_PrevLogicTemp, m_NextLogicTemp);
        }
    }
    __forceinline void DelLT(void)
    {
        if(InLT())
        {
            LIST_DEL_CLEAR(this, m_FirstLogicTemp, m_LastLogicTemp, m_PrevLogicTemp, m_NextLogicTemp);
        }
    }

    static void ProceedLogic(int ms);

    __forceinline void RChange(dword zn) { m_RChange |= zn; }
    __forceinline void RNoNeed(dword zn) { m_RChange &= (~zn); }

    void StaticTact(int ms);

    virtual void GetResources(dword need) = 0; //Запрашиваем необходимые ресурсы для объекта

    virtual void Tact(int cms) = 0;
    virtual void LogicTact(int cms) = 0;

    virtual bool Pick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt) const = 0;

    virtual bool TakingDamage(int weap, const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, int attacker_side = 0/*NEUTRAL_SIDE*/, CMatrixMapStatic* attaker = nullptr) = 0;

    virtual void BeforeDraw(void) = 0;
    virtual void Draw(void) = 0;
    virtual void DrawShadowStencil(void) = 0;
    virtual void DrawShadowProj(void) = 0;
    virtual void FreeDynamicResources(void) = 0;

    //void OnLoad(void);
    //void Init(int ids);

    virtual bool CalcBounds(D3DXVECTOR3& omin, D3DXVECTOR3& omax) = 0;
        
    virtual int GetSide(void) const = 0;
    virtual bool NeedRepair(void) const = 0;

    static bool  EnumVertsHandler(const SVOVertex& v, dword data);
    virtual bool InRect(const CRect& rect) const = 0;

    void JoinToGroup(void);
    void UnjoinGroup(void);


    bool RenderToTexture(SRenderTexture* rt, int n, /*float *fff=nullptr,*/ float anglez = GRAD2RAD(30), float anglex = GRAD2RAD(30), float fov = GRAD2RAD(60));

#ifdef SHOW_ASSIGNED_GROUPS
    void ShowGroups(void);
#endif
};

__forceinline  CMatrixMapStatic* CMatrixEffectRepair::GetTarget(void)
{
    if(m_Target) return m_Target->m_Object;
    return nullptr;
}

__forceinline void   CMatrixEffectWeapon::SetOwner(CMatrixMapStatic* ms) { m_Owner = ms->GetCore(DEBUG_CALL_INFO); m_SideStorage = ms->GetSide(); }
__forceinline       CMatrixMapStatic* CMatrixEffectWeapon::GetOwner(void)
{
    if(m_Owner) return m_Owner->m_Object;
    return nullptr;
}

CVectorObjectAnim* LoadObject(const wchar* name, CHeap* heap, bool side = false, const wchar* texname = nullptr);
void UnloadObject(CVectorObjectAnim* o, CHeap* heap);