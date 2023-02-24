// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_MAP_HPP
#define MATRIX_MAP_HPP

#include "common.hpp"
#include "MatrixConfig.hpp"
#include <vector>

#define DRAW_LANDSCAPE_SETKA 1

#define FREE_TIME_PERIOD    (5000) // in ms

#define GLOBAL_SCALE        (20.0f)
#define MOVE_CNT            (2)
#define GLOBAL_SCALE_MOVE   (GLOBAL_SCALE / MOVE_CNT)

#define MAX_ALWAYS_DRAW_OBJ 16

#define MatrixPathMoveMax 256

#define ROBOT_MOVECELLS_PER_SIZE   4  //Размер стороны треугольника робота в ячейках сетки проходимости

#define DRAW_SHADOWS_DISTANCE_SQ    ((g_ShadowsDrawDistance) * (g_ShadowsDrawDistance))

#define TRACE_LANDSCAPE SETBIT(0)
#define TRACE_WATER     SETBIT(1)
#define TRACE_ROBOT     SETBIT(2)
#define TRACE_BUILDING  SETBIT(3)
#define TRACE_OBJECT    SETBIT(4)
#define TRACE_CANNON    SETBIT(5)
#define TRACE_FLYER     SETBIT(6)
#define TRACE_ANYOBJECT (TRACE_OBJECT | TRACE_BUILDING | TRACE_ROBOT | TRACE_CANNON | TRACE_FLYER)

#define TRACE_OBJECTSPHERE          SETBIT(10) //trace object as speres
#define TRACE_SKIP_INVISIBLE        SETBIT(11) //skip objects with flag OBJECT_STATE_TRACE_INVISIBLE while tracing 

#define TRACE_ALL       (((dword)(-1)) & (~TRACE_OBJECTSPHERE))
#define TRACE_NONOBJECT (TRACE_LANDSCAPE|TRACE_WATER)

#define TRACE_STOP_NONE      ((CMatrixMapStatic*)0)
#define TRACE_STOP_LANDSCAPE ((CMatrixMapStatic*)1)
#define TRACE_STOP_WATER     ((CMatrixMapStatic*)2)

#define IS_TRACE_STOP_OBJECT(o) (((unsigned int)o > 100))
#define IS_TRACE_UNIT(o) ((o->GetObjectType() == OBJECT_TYPE_ROBOTAI)||(o->GetObjectType() == OBJECT_TYPE_FLYER))

#define SHADER_PERC     20
#define SHADER_TIME     500

#include "MatrixCamera.hpp"
#include "VectorObject.hpp"
#include "MatrixSide.hpp"
#include "MatrixMapGroup.hpp"
#include "MatrixMapStatic.hpp"
#include "MatrixWater.hpp"
#include "MatrixMapTexture.hpp"
#include "Effects/MatrixEffect.hpp"
#include "StringConstants.hpp"
#include "MatrixMinimap.hpp"
#include "MatrixConfig.hpp"
#include "MatrixCursor.hpp"
#include "MatrixDebugInfo.hpp"
#include "Logic/MatrixRoadNetwork.hpp"
#include "DevConsole.hpp"
#include "MatrixObjectRobot.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixTransition.hpp"

__forceinline bool CMatrixMapStatic::FitToMask(dword mask)
{
    if(IsRobotAlive()) return (mask & TRACE_ROBOT) != 0;
    if(IsCannonAlive()) return (mask & TRACE_CANNON) != 0;
    if(IsBuildingAlive()) return (mask & TRACE_BUILDING) != 0;
    if(GetObjectType() == OBJECT_TYPE_MAPOBJECT) return (mask & TRACE_OBJECT) != 0;
    if(IsFlyer()) return (mask & TRACE_FLYER) != 0;
    return false;
}

typedef bool (*ENUM_OBJECTS)(const D3DXVECTOR3& center, CMatrixMapStatic* o, dword user);
typedef bool (*ENUM_OBJECTS2D)(const D3DXVECTOR2& center, CMatrixMapStatic* o, dword user);

class CMatrixBuilding;

struct SMatrixMapPoint {
	float z = 0.0f;
    float z_land = 0.0f;
    D3DXVECTOR3 n = { 0.0f, 0.0f, 0.0f };
    dword color = 0;
    int   lum_r = 0, lum_g = 0, lum_b = 0;
};

/*
struct SMatrixMapZone {
	int m_BeginX,m_BeginY;			// Начало роста
	int m_CenterX,m_CenterY;		// Центр масс
	bool m_Building;				// В зоне есть здание
	int m_Size;						// Cnt in unit
	int m_Perim;					// Периметр
	CRect m_Rect;					// Bound zone

	int m_NearZoneCnt;				// Кол-во ближайших зон
	int m_NearZone[8];				// Ближайшие зоны
	int m_NearZoneConnectSize[8];	// Длина соединения с ближайшими зонами

	int m_FPLevel;
	int m_FPWt;
	int m_FPWtp;
};
*/

struct SMatrixMapUnit
{ 
private:
    dword m_TypeBits = 0;

public:
    CMatrixBuilding* m_MapUnitBase = nullptr;

    // coefs for z calc (with bridge bridge)
    float a1 = 0.0f, b1 = 0.0f, c1 = 0.0f;
    float a2 = 0.0f, b2 = 0.0f, c2 = 0.0f;

    bool IsWater() const { return FLAG(m_TypeBits, CELLFLAG_WATER); }
    bool IsLand() const { return FLAG(m_TypeBits, CELLFLAG_LAND); }
    void SetType(dword t) { m_TypeBits = t; }
    bool IsFlat() const { return FLAG(m_TypeBits, CELLFLAG_FLAT); }
    bool IsBridge() const { return FLAG(m_TypeBits, CELLFLAG_BRIDGE); }
    bool IsInshore() const { return FLAG(m_TypeBits, CELLFLAG_INSHORE); }
    void ResetInshore() { RESETFLAG(m_TypeBits, CELLFLAG_INSHORE); }
    void ResetFlat() { RESETFLAG(m_TypeBits, CELLFLAG_FLAT); }
    void SetFlat() { SETFLAG(m_TypeBits, CELLFLAG_FLAT); }
    void SetBridge() { SETFLAG(m_TypeBits, CELLFLAG_BRIDGE); }

    bool IsDown() const { return FLAG(m_TypeBits, CELLFLAG_DOWN); }
};

struct SMatrixMapMove {
	int m_Zone = 0;
    dword m_Sphere = 0;
    dword m_Zubchik = 0;

    int m_Find = 0;
    int m_Weight = 0;
	dword m_Stop = 0;	// (1 - нельзя пройти)  1 - Shasi1(Пневматика)  2 - Shasi2(Колеса)  4 - Shasi3(Гусеницы)  8 - Shasi4(Подушка)  16 - Shasi5(Крылья)
                        // <<0-size 1          <<6-size 2             <<12-size 3        <<18-size 4         <<24-size 5

    byte GetType(int nsh) const
    {
        byte rv = 0;

        if(!(this->m_Stop & (1 << nsh))) return 0xff;

        if(this->m_Sphere & ((1 << nsh) << 0)) rv |= 1;
        if(this->m_Sphere & ((1 << nsh) << 8)) rv |= 2;
        if(this->m_Sphere & ((1 << nsh) << 16)) rv |= 4;
        if(this->m_Sphere & ((1 << nsh) << 24)) rv |= 8;

        if(this->m_Zubchik & ((1 << nsh) << 0)) rv |= 16;
        if(this->m_Zubchik & ((1 << nsh) << 8)) rv |= 32;
        if(this->m_Zubchik & ((1 << nsh) << 16)) rv |= 64;
        if(this->m_Zubchik & ((1 << nsh) << 24)) rv |= 128;

        return rv;
    }
};

struct SDifficulty
{
    float coef_enemy_damage_to_player_side = 1.0f;
    float coef_time_before_reinforcements = 1.0f;
	float coef_friendly_fire = 1.0f;
};

struct SGroupVisibility
{
    PCMatrixMapGroup* vis = nullptr;
    int               vis_cnt = 0;
    float             z_from = 0.0f;
    int*              levels = nullptr;
    int               levels_cnt = 0;

    void              Release(void);
};

#define MMFLAG_OBJECTS_DRAWN    SETBIT(0)
#define MMFLAG_FOG_ON           SETBIT(1)
#define MMFLAG_SKY_ON           SETBIT(2)
//#define MMFLAG_EFF_TAKT       SETBIT(3)   // effect takts loop is active. it causes SubEffect to do not delete effect immediately
#define MMFLAG_PAUSE            SETBIT(4)
#define MMFLAG_DIALOG_MODE      SETBIT(5)
#define MMFLAG_MOUSECAM         SETBIT(6)
#define MMFLAG_NEEDRECALCTER    SETBIT(7)
#define MMFLAG_STAT_DIALOG      SETBIT(8)
#define MMFLAG_STAT_DIALOG_D    SETBIT(9)
#define MMFLAG_ENABLE_CAPTURE_FUCKOFF_SOUND     SETBIT(10)
#define MMFLAG_SOUND_BASE_SEL_ENABLED           SETBIT(11)
#define MMFLAG_SOUND_ORDER_ATTACK_DISABLE       SETBIT(12)
#define MMFLAG_SOUND_ORDER_CAPTURE_ENABLED      SETBIT(13)
#define MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS       SETBIT(14)
#define MMFLAG_WIN                              SETBIT(15)
#define MMFLAG_MUSIC_VOL_CHANGING               SETBIT(16)
#define MMFLAG_TRANSITION                       SETBIT(17)
#define MMFLAG_VIDEO_RESOURCES_READY            SETBIT(18)  // VB's, IB's and Non managed textures

#define MMFLAG_MEGABUSTALREADY                  SETBIT(19)  // easter egg
#define MMFLAG_ROBOT_IN_POSITION                SETBIT(20)  // easter egg
#define MMFLAG_SHOWPORTRETS                     SETBIT(21)  // easter egg

#define MMFLAG_DISABLEINSHORE_BUILD             SETBIT(22)  // inshores can be enabled, but cannot be build in WaterBuild function
#define MMFLAG_AUTOMATIC_MODE                   SETBIT(23)
#define MMFLAG_FLYCAM                           SETBIT(24)
#define MMFLAG_FULLAUTO                         SETBIT(25)

#define MMFLAG_TERRON_DEAD                      SETBIT(26)
#define MMFLAG_TERRON_ONMAP                     SETBIT(27)

#define MMFLAG_SPECIAL_BROKEN                   SETBIT(28)


struct SSkyTex
{
    CTextureManaged* tex = nullptr;
    float u0 = 0.0f, v0 = 0.0f, u1 = 0.0f, v1 = 0.0f;
};

//Хранит в себе перечень баз с установленными пунктами сбора
//Необходимо для непрерывного вызова эффекта отрисовки точки сбора при выделении здания
struct SGatheringPointsList {
    std::vector<CMatrixBuilding*> BuildingsList;
};

enum
{
    SKY_FORE,
    SKY_BACK,
    SKY_LEFT,
    SKY_RITE,
};

class CMatrixHint;

class CMatrixMap : public CMain
{
    CBuf m_AllObjects;

protected:
	~CMatrixMap();

private:
    struct SShadowRectVertex
    {
        D3DXVECTOR4 p = { 0.0f, 0.0f, 0.0f, 0.0f };
        dword       color = 0;
    };

    enum EReloadStep
    {
        RS_SIDEAI,
        RS_RESOURCES,
        RS_MAPOBJECTS,
        RS_BUILDINGS,
        RS_CANNONS,
        RS_ROBOTS,
        RS_EFFECTS,
        RS_CAMPOS
    };

public:
    dword m_Flags = 0;

    CMatrixHint* m_PauseHint = nullptr;

    CBuf         m_DialogModeHints;
    const wchar* m_DialogModeName = nullptr;

    int m_TexUnionDim = 0;
    int m_TexUnionSize = 0;

    float m_SkyHeight = 0;

	CPoint m_Size = { 0, 0 };
	CPoint m_SizeMove = { 0, 0 };
	SMatrixMapUnit* m_Module = nullptr;
	SMatrixMapPoint* m_Point = nullptr;
	SMatrixMapMove* m_Move = nullptr;

    SGatheringPointsList m_GatheringPointsList;

    void AddGatheringPoint(CMatrixBuilding* bld)
    {
        m_GatheringPointsList.BuildingsList.push_back(bld);
    }

    bool RemoveGatheringPoint(CMatrixBuilding* bld)
    {
        for(int i = 0; i < m_GatheringPointsList.BuildingsList.size(); ++i)
        {
            if(m_GatheringPointsList.BuildingsList[i] == bld)
            {
                m_GatheringPointsList.BuildingsList.erase(m_GatheringPointsList.BuildingsList.begin() + i);
                return true;
            }
        }

        return false;
    }

    CMatrixRoadNetwork m_RoadNetwork;

    CPoint            m_GroupSize = { 0, 0 };
	CMatrixMapGroup** m_Group = nullptr;
    SGroupVisibility* m_GroupVis = nullptr;

    CDevConsole       m_Console;
    CMatrixDebugInfo  m_DI;

    int   m_BeforeWinCount = 0;

    float m_GroundZ = WATER_LEVEL;
    float m_GroundZBase = WATER_LEVEL - 64.0f;
    float m_GroundZBaseMiddle = 0.0f;
    float m_GroundZBaseMax = 0.0f;
	//D3DXPLANE m_ShadowPlaneCut;
    //D3DXPLANE m_ShadowPlaneCutBase;
	D3DXVECTOR3 m_LightMain = { 250.0f, -50.0f, -250.0f };

    CMatrixWater* m_Water = nullptr;

	int m_IdsCnt = 0;
	CWStr* m_Ids = nullptr;

    CMatrixSideUnit* m_PlayerSide = nullptr;
	CMatrixSideUnit* m_Side = nullptr;
	int m_SidesCount = 0;

    CMatrixEffect*  m_EffectsFirst = nullptr;
    CMatrixEffect*  m_EffectsLast = nullptr;
    CMatrixEffect*  m_EffectsNextTact = nullptr;
    int             m_EffectsCnt = 0;
    //CDWORDMap     m_Effects;

    //Массив точек (объектов) для отрисовки статичных визуальных эффектов на карте (дым, огонь, разряды электричества, звуки)
    CEffectSpawner* m_EffectSpawners = nullptr;
    int             m_EffectSpawnersCnt = 0;

    CMatrixMapStatic* m_NextLogicObject = nullptr;

    CWStr           m_WaterName = (CWStr)L"";

    float m_BiasCannons = 0.0f;
    float m_BiasRobots = 0.0f;
    float m_BiasBuildings = 0.0f;
    float m_BiasTer = 0.0f;
    float m_BiasWater = 0.0f;

    float m_CameraAngle = 0.0f;
    float m_WaterNormalLen = 1.0f;

    float m_Terrain2ObjectInfluence = 0.0f;
    dword m_Terrain2ObjectTargetColor = 0;
    dword m_WaterColor = 0;
    dword m_SkyColor = 0;
    dword m_InshorewaveColor = 0;
    dword m_AmbientColor = 0;
    dword m_AmbientColorObj = 0;
    dword m_LightMainColor = 0;
    dword m_LightMainColorObj = 0;
    dword m_ShadowColor = 0;
	float m_LightMainAngleZ = 0.0f, m_LightMainAngleX = 0.0f;

	int              m_MacrotextureSize = 1;
	CTextureManaged* m_Macrotexture = nullptr;

    CTextureManaged* m_Reflection = nullptr;

    SSkyTex          m_SkyTex[4];
    float            m_SkyAngle = 0.0f;
    float            m_SkyDeltaAngle = 0.0f;

    CTransition m_Transition;

    CMinimap m_Minimap;

    int m_KeyDown = 0;
    int m_KeyScan = 0;

    CMatrixCursor m_Cursor;

    int m_IntersectFlagTracer = 0;
    int m_IntersectFlagFindObjects = 0;

    D3DXVECTOR3       m_MouseDir = { 0.0f, 0.0f, 0.0f }; // world direction to mouse cursor
    // trace stop!
    D3DXVECTOR3       m_TraceStopPos = { 0.0f, 0.0f, 0.0f }; //Последняя координата курсора мыши на поверхности карте, на которой в последний раз находился курсор (может обновиться только при указании существующей поверхности!)
    CMatrixMapStatic* m_TraceStopObj = nullptr; //Последний CMatrixMapStatic, попавший под курсор мыши

    bool IsTraceNonPlayerObj();
    PCMatrixMapStatic m_AD_Obj[MAX_ALWAYS_DRAW_OBJ];  // out of land object // flyer
    int               m_AD_Obj_cnt = 0;
        
    CMatrixCamera     m_Camera;

    CDeviceState*     m_DeviceState = nullptr;

    SDifficulty       m_Difficulty;

protected:
    D3D_VB  m_ShadowVB = nullptr;

    dword     m_NeutralSideColor = 0;
    dword     m_NeutralSideColorMM = 0;
    CTexture* m_NeutralSideColorTexture = nullptr;

    D3DXMATRIX m_Identity;

    float m_minz =  10000.0f;
    float m_maxz = -10000.0f;

    CBuf* m_VisWater = nullptr;

    CMatrixMapGroup** m_VisibleGroups = nullptr;
    int               m_VisibleGroupsCount = 0;

    dword m_CurFrame = 0;

    int   m_Time = 0;
    int   m_PrevTimeCheckStatus = -1500;

    dword m_StartTime = 0;

    // helicopter explosion
    int         m_ShadeTime = 0;
    int         m_ShadeTimeCurrent = 0;
    bool        m_ShadeOn = false;
    D3DXVECTOR3 m_ShadePosFrom = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_ShadePosTo = { 0.0f, 0.0f, 0.0f };
    dword       m_ShadeInterfaceColor = 0;
        
    int m_ReinforcementsTime = 0;
    int m_ReinforcementsPercent = 0;

    float m_StoreCurrentMusicVolume = 1.0f;
    float m_TargetMusicVolume = 1.0f;

public:
    int m_BeforeWinLoseDialogCount = 0;

    CMatrixMap();
	void Clear();

	void IdsClear();
    __forceinline const CWStr& MapName() { return m_Ids[m_IdsCnt - 1]; }
    __forceinline const CWStr& IdsGet(int no) { return m_Ids[no]; }
    __forceinline int IdsGetCount() const { return m_IdsCnt; }

    void ModuleClear();

    void UnitInit(int sx, int sy);
    void PointCalcNormals(int x, int y);

    void CalcVis(); // non realtime function! its updates map data
    //void CalcVisTemp(int from, int to, const D3DXVECTOR3 &ptfrom); // non realtime function! its updates map data

    __forceinline SMatrixMapUnit* UnitGet(int x, int y) { return m_Module + y * m_Size.x + x; }
    __forceinline SMatrixMapUnit* UnitGetTest(int x, int y) { return (x >= 0 && x < m_Size.x && y >= 0 && y < m_Size.y) ? (m_Module + y * m_Size.x + x) : nullptr; }

    __forceinline SMatrixMapMove* MoveGet(int x, int y)
    {
        return m_Move + y * m_SizeMove.x + x;
    }
    __forceinline SMatrixMapMove* MoveGetTest(int x, int y)
    {
        return(x >= 0 && x < m_SizeMove.x && y >= 0 && y < m_SizeMove.y) ? (m_Move + y * m_SizeMove.x + x) : nullptr;
    }

    __forceinline SMatrixMapPoint* PointGet(int x, int y)
    {
        return m_Point + x + y * (m_Size.x + 1);
    }
    __forceinline SMatrixMapPoint* PointGetTest(int x, int y)
    {
        return (x >= 0 && x <= m_Size.x && y >= 0 && y <= m_Size.y) ? (m_Point + x + y * (m_Size.x + 1)) : nullptr;
    }

#if defined _TRACE || defined _DEBUG
    void ResetReinforcementsTime() { m_ReinforcementsTime = 0; };
#endif
    int BeforeReinforcementsTime() const {return m_ReinforcementsTime;};
    float BeforReinforcementsTimeT() const { return 1.0f - float(m_ReinforcementsTime) / (m_Difficulty.coef_time_before_reinforcements * float(g_Config.m_ReinforcementsTime * m_ReinforcementsPercent / 100)); };
    void InitReinforcementsTime() { m_ReinforcementsTime = Float2Int(m_Difficulty.coef_time_before_reinforcements * float(g_Config.m_ReinforcementsTime) * float(m_ReinforcementsPercent) / 100.0f); };
    bool ReinforcementsDisabled() const { return (m_ReinforcementsPercent == 0); }
    //int GetReinforcementsPrc() { return m_ReinforcementsPercent; }
    void SetReinforcementsTime(int time) { m_ReinforcementsTime = time; }

    float GetZLand(double wx, double wy);
    float GetZ(float wx, float wy);
    float GetZInterpolatedLand(float wx, float wy);
    float GetZInterpolatedObj(float wx, float wy);
    float GetZInterpolatedObjRobots(float wx, float wy);
    void  GetNormal(D3DXVECTOR3* out, float wx, float wy, bool check_water = false);
    dword GetColor(float wx, float wy);

    //void CalcMoveSpherePlace(void);

    void ClearGroupVis();
    void GroupClear();
    void GroupBuild(CStorage& stor);

    void RobotPreload();

    const D3DXMATRIX& GetIdentityMatrix() const { return m_Identity; }

    bool CalcVectorToLandscape(const D3DXVECTOR2& pos, D3DXVECTOR2& dir);

    __forceinline void RemoveFromAD(CMatrixMapStatic* ms)
    {
        for(int i = 0; i < m_AD_Obj_cnt; ++i)
        {
            if(m_AD_Obj[i] == ms)
            {
                m_AD_Obj[i] = m_AD_Obj[--m_AD_Obj_cnt];
                return;
            }
        }
    }

    bool UnitPick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, int* ox, int* oy, float* ot = nullptr) { return UnitPick(orig, dir, CRect(0, 0, m_Size.x, m_Size.y), ox, oy, ot); }
    bool UnitPick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, const CRect& ar, int* ox, int* oy, float* ot = nullptr);
    bool PointPick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, int* ox, int* oy) { return PointPick(orig, dir, CRect(0, 0, m_Size.x, m_Size.y), ox, oy); }
    bool PointPick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, const CRect& ar, int* ox, int* oy);
    bool UnitPickGrid(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, int* ox, int* oy);
    bool UnitPickWorld(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* ox, float* oy);

    void RestoreMusicVolume();
    void SetMusicVolume(float vol);


    void StaticClear();
    void StaticDelete(CMatrixMapStatic* ms);
		
    //CMatrixMapStatic* StaticAdd(EObjectType type, bool add_to_logic = true);
        
    __forceinline void AddObject(CMatrixMapStatic* ms, bool add_to_logic)
    {
        m_AllObjects.Expand(sizeof(CMatrixMapStatic*));
        CMatrixMapStatic** e = m_AllObjects.BuffEnd<CMatrixMapStatic*>();
        *(e - 1) = ms;
        if (add_to_logic) ms->AddLT();
    }
    template <class O> __forceinline O* StaticAdd(bool add_to_logic = true)
    {
        O* o = HNew(Base::g_MatrixHeap) O();
        //O* o = new O();
        AddObject(o, add_to_logic && o->GetObjectType() != OBJECT_TYPE_MAPOBJECT);
        return o;
    }

    void AddEffectSpawner(float x, float y, float z, int ttl, const CWStr& type);
    void RemoveEffectSpawnerByObject(CMatrixMapStatic* ms);
    void RemoveEffectSpawnerByTime();

    void MacrotextureClear();
    void MacrotextureInit(const CWStr& path);

    //side funcs
    dword GetSideColor(int id);
    dword GetSideColorMM(int id);
    CTexture* GetSideColorTexture(int id);
    __forceinline CTexture* GetPlayerSideColorTexture(void) { return m_PlayerSide->m_ColorTexture; };
    void ClearSide();

    void LoadSide(CBlockPar& bp);
    //void LoadTactics(CBlockPar& bp);

    CMatrixSideUnit* GetSideById(int id);
    CMatrixSideUnit* GetPlayerSide() { return m_PlayerSide; };

    void WaterClear();
    void WaterInit();

    void BeforeDraw();
    void Draw();

    void MarkNeedRecalcTerainColor() { SETFLAG(m_Flags, MMFLAG_NEEDRECALCTER); }

    bool AddEffect(CMatrixEffect* ef);
#ifdef _DEBUG
    void SubEffect(SDebugCallInfo& from, CMatrixEffect* e);
#else
    void SubEffect(CMatrixEffect* e);
#endif
    //bool IsEffect(CMatrixEffect* e);
    int GetEffectCount() { return m_EffectsCnt; }
    //void SubLowestEffect();

    void Tact(int step);

    //dword Load(CBuf& b, CBlockPar& bp, bool& surface_macro);
    void Restart(); // boo!

    int  ReloadDynamics(CStorage& stor, EReloadStep step, CBuf* robots = nullptr);

    int  PrepareMap(CStorage& stor, const CWStr& mapname);
    void StaticPrepare(int n, bool skip_progress = false);
    void StaticPrepare2(CBuf* robots);

    void InitObjectsLights();            // must be called only after CreatePoolDefaultResources
    void ReleasePoolDefaultResources();
    void CreatePoolDefaultResources(bool loading); // false - restoring resources
    bool CheckLostDevice();  // if true, device is lost

//zakker
    void ShowPortrets();

    void EnterDialogMode(const wchar* hint);
    void LeaveDialogMode();

    static void OkHandler();
    static void OkExitHandler();
    static void OkExitWinHandler();
    static void OkExitLoseHandler();
    static void OkJustExitHandler();
    static void OkSurrenderHandler();
    static void OkResetHandler();

    //void BeginDieSequence(void);
    bool DieSequenceInProgress() { return m_ShadeOn; }


    CTextureManaged* GetReflectionTexture() { return m_Reflection; };
    int   GetTime() const { return m_Time; }
    dword GetStartTime() const { return m_StartTime; }

    bool CatchPoint(const D3DXVECTOR3& from, const D3DXVECTOR3& to);
    bool TraceLand(D3DXVECTOR3* out, const D3DXVECTOR3& start, const D3DXVECTOR3& dir);
    CMatrixMapStatic* Trace(D3DXVECTOR3* out, const D3DXVECTOR3& start, const D3DXVECTOR3& end, dword mask, CMatrixMapStatic* skip = nullptr);
    bool FindObjects(const D3DXVECTOR3& pos, float radius, float oscale, dword mask, CMatrixMapStatic* skip, ENUM_OBJECTS callback, dword user);
    bool FindObjects(const D3DXVECTOR2& pos, float radius, float oscale, dword mask, CMatrixMapStatic* skip, ENUM_OBJECTS2D callback, dword user);

    //CMatrixMapGroup* GetGroupByCell(int x, int y) { return m_Group[(x / MATRIX_MAP_GROUP_SIZE) + (y / MATRIX_MAP_GROUP_SIZE) * m_GroupSize.x ]; }
    __forceinline CMatrixMapGroup* GetGroupByIndex(int x, int y) { return m_Group[x + y * m_GroupSize.x]; }
    __forceinline CMatrixMapGroup* GetGroupByIndex(int i) { return m_Group[i]; }
    __forceinline CMatrixMapGroup* GetGroupByIndexTest(int x, int y) { return (x < 0 || y < 0 || x >= m_GroupSize.x || y >= m_GroupSize.x) ? nullptr : m_Group[x + y * m_GroupSize.x]; }

    float GetGroupMaxZLand(int x, int y);
    float GetGroupMaxZObj(int x, int y);
    float GetGroupMaxZObjRobots(int x, int y);

    void CalcMapGroupVisibility();

    struct SCalcVisRuntime;

    void CheckCandidate(SCalcVisRuntime& cvr, CMatrixMapGroup* g);


    dword GetCurrentFrame() { return m_CurFrame; }

    __forceinline void Pause(bool p)
    {
        if(FLAG(m_Flags, MMFLAG_DIALOG_MODE)) return; // disable pasue/unpause in dialog mode
        INITFLAG(m_Flags, MMFLAG_PAUSE, p);
        if(p) CSound::StopPlayAllSounds();
    }
    __forceinline bool  IsPaused() { return FLAG(m_Flags, MMFLAG_PAUSE); }

    __forceinline void  MouseCam(bool p) { INITFLAG(m_Flags, MMFLAG_MOUSECAM, p); }
    __forceinline bool  IsMouseCam() { return FLAG(m_Flags, MMFLAG_MOUSECAM); }

    // draw functions
    void BeforeDrawLandscape(bool all = false);
    void BeforeDrawLandscapeSurfaces(bool all = false);
    void DrawLandscape(bool all = false);
    void DrawLandscapeSurfaces(bool all = false);
    void DrawObjects();
    void DrawWater();
    void DrawShadowsProjFast();
    void DrawShadows();
    void DrawEffects();
    void DrawSky();


private:
    enum EScanResult
    {
        SR_NONE,
        SR_BREAK,
        SR_FOUND
    };

    EScanResult ScanLandscapeGroup(void* data, int gx, int gy, const D3DXVECTOR3& start, const D3DXVECTOR3& end);
    EScanResult ScanLandscapeGroupForLand(void* data, int gx, int gy, const D3DXVECTOR3& start, const D3DXVECTOR3& end);
};

class CMatrixMapLogic;
class CMatrixTacticsList; 

#include "MatrixLogic.hpp"



__forceinline bool CMatrixMap::AddEffect(CMatrixEffect* ef)
{
#ifdef _DEBUG
    if(ef->GetType() == EFFECT_UNDEFINED)
    {
        //int a = 1;
        ERROR_S(L"Undefined effect!");
    }
#endif

    if(m_EffectsCnt >= g_MaxEffectsCount)
    {
        CMatrixEffect* ef2del = nullptr;
        int pri = ef->Priority();
        for(CMatrixEffect* e = m_EffectsFirst; e != nullptr; e = e->m_Next)
        {
            if(e->IsDIP()) continue;
            int p = e->Priority();
            if(p < MAX_EFFECT_PRIORITY)
            {
                ef2del = e;
                pri = p;
            }
        }

        if(ef2del == nullptr)
        {
            ef->Release();
            return false;
        }
        else
        {
            if(m_EffectsNextTact == ef2del) m_EffectsNextTact = ef2del->m_Next;
            LIST_DEL(ef2del, m_EffectsFirst, m_EffectsLast, m_Prev, m_Next);
            ef2del->Release();
            --m_EffectsCnt;
        }
    }

    LIST_ADD(ef, m_EffectsFirst, m_EffectsLast, m_Prev, m_Next);
    ++m_EffectsCnt;
#ifdef _DEBUG
    CDText::T("E", CStr(m_EffectsCnt));
#endif
    return true;
}


__forceinline CMatrixSideUnit* CMatrixMap::GetSideById(int id)
{
	for(int i = 0; i < m_SidesCount; ++i)
    {
		if(m_Side[i].m_Id == id) return &m_Side[i];
	}

#ifdef _DEBUG
    _asm int 3
#endif

	ERROR_E;
}

__forceinline dword CMatrixMap::GetSideColor(int id)
{
DTRACE();

    if(!id) return m_NeutralSideColor;
    return GetSideById(id)->m_Color;
}

__forceinline dword CMatrixMap::GetSideColorMM(int id)
{
DTRACE();

    if(!id) return m_NeutralSideColorMM;
    return GetSideById(id)->m_ColorMM;
}

__forceinline CTexture * CMatrixMap::GetSideColorTexture(int id)
{
DTRACE();

    if(!id) return m_NeutralSideColorTexture;
    return GetSideById(id)->m_ColorTexture;
}

__forceinline  float CMatrixMap::GetGroupMaxZLand(int x, int y)
{
    if(x < 0 || x >= m_GroupSize.x || y < 0 || y >= m_GroupSize.y) return 0;
    CMatrixMapGroup* g = GetGroupByIndex(x, y);
    if(g == nullptr) return 0;
    float z = GetGroupByIndex(x, y)->GetMaxZLand();
    if(z < 0) return 0;
    return z;
}
__forceinline  float CMatrixMap::GetGroupMaxZObj(int x, int y)
{
    if(x < 0 || x >= m_GroupSize.x || y < 0 || y >= m_GroupSize.y) return 0;
    CMatrixMapGroup* g = GetGroupByIndex(x, y);
    if(g == nullptr) return 0;
    float z = GetGroupByIndex(x, y)->GetMaxZObj();
    if(z < 0) return 0;
    return z;
}
__forceinline float CMatrixMap::GetGroupMaxZObjRobots(int x, int y)
{
    if(x < 0 || x >= m_GroupSize.x || y < 0 || y >= m_GroupSize.y) return 0;
    CMatrixMapGroup* g = GetGroupByIndex(x, y);
    if(g == nullptr) return 0;
    float z = GetGroupByIndex(x, y)->GetMaxZObjRobots();
    if(z < 0) return 0;
    return z;
}

#endif