// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once
#include "VectorObject.hpp"
#include "Effects/MatrixEffect.hpp"
#include "Logic/MatrixRoadNetwork.hpp"
#include "MatrixConfig.hpp"
#include <vector>

//#define MAX_ROBOTS_BUILD      10
#define MAX_ROBOTS              60
#define MAX_FACTORIES           10
#define MAX_LOGIC_GROUP         (MAX_ROBOTS + 1)

#define MAX_TEAM_CNT            3
#define MAX_SELECTION_GROUPS    10

#define HELI_PARAM              3

#define ROBOTS_BY_BASE          3
#define ROBOTS_BY_MAIN          4
#define ROBOTS_BY_FACTORY       1

#define MAIN_ENEMY_RECALC_PERIOD 60000 //Как часто (в тактах) компьютер будет переопределять своего текущего наиболее приоритетного врага (сторону)

//Использовалось в качестве аркадного коэффициента-бонуса к скорости
//Было заменено на переменную g_UnitSpeedArcadeCoef, которую можно выставлять из конфига
//#define SPEED_BOOST             1.1f

#define FRIENDLY_SEARCH_RADIUS  400

class CMatrixEffectWeapon;
class CMatrixMapStatic;
class CConstructor;
class CMatrixEffectSelection;
class CMatrixRobotAI;
class CMatrixRobot;
class CMatrixBuilding;
class CMatrixTactics;
class CMatrixTacticsList;
class CMatrixGroupList;
class CMatrixEffectElevatorField;
class CMatrixGroup;
class CMatrixCannon;
class CMatrixEffectLandscapeSpot;
class CMatrixFlyer;
class CConstructorPanel;

enum EWeapon;
enum Team;
enum EFlyerOrder;

#define MAX_STATISTICS 6

enum EStat
{
    STAT_ROBOT_BUILD,
    STAT_ROBOT_KILL,
    STAT_TURRET_BUILD,
    STAT_TURRET_KILL,
    STAT_BUILDING_KILL,
    STAT_TIME
};

enum ESelection
{
	FLYER_SELECTED = 0,
	ROBOT_SELECTED = 1,
	BUILDING_SELECTED = 2,
    NOTHING_SELECTED = 4,
    BASE_SELECTED = 5,
    GROUP_SELECTED = 6,
    ARCADE_SELECTED = 7
};

enum ESelType
{
	ROBOT = 0,
	FAR_ROBOT = 1,
	FLYER = 2,
	BUILDING = 3,
    NOTHING = 4,
    GROUP = 5,
    ARCADE = 6
};

struct SRobot
{
	CMatrixRobotAI* m_Robot = nullptr;
    D3DXVECTOR3 m_Mark = { 0.0f, 0.0f, 0.0f };
	//bool m_CrossCatched = false;
};

enum ESideStatus
{
    SS_NONE,        // side absent
    SS_ACTIVE,      // active side
    SS_JUST_DEAD,   // just dead side. switch this status to SS_NONE after you get it
    SS_JUST_WIN     // valid only for player side
};

enum EPlayerActions
{
    NOTHING_SPECIAL,
    CAPTURING_ROBOT,
    TRANSPORTING_ROBOT,
    DROPING_ROBOT,
    GETING_IN_ROBOT,
    BUILDING_TURRET
};

struct SCannonForBuild
{
    CMatrixCannon*   m_Cannon = nullptr;
    CMatrixBuilding* m_ParentBuilding = nullptr;
    SEffectHandler   m_ParentSpot;
    int              m_CanBuildFlag = 0;

    SCannonForBuild() : m_ParentSpot() {}
    void Delete();
};

struct SCallback
{
    CPoint mp = { 0, 0 };
    int    calls = 0;
};

enum EMatrixLogicActionType
{
    mlat_None,
    mlat_Defence,
    mlat_Attack,
    mlat_Forward,
    mlat_Retreat,
    mlat_Capture,
    mlat_Intercept
};

#define MLRG_CAPTURE 1
#define MLRG_DEFENCE 2

#define REGION_PATH_MAX_CNT 32

struct SMatrixLogicAction
{
    EMatrixLogicActionType m_Type = mlat_None;
    int m_Region = 0;
    int m_RegionPathCnt = 0;                       // Кол-во регионов в пути
    int m_RegionPath[REGION_PATH_MAX_CNT] = { 0 }; // Путь по регионам
};

struct SMatrixLogicGroup
{
private:
    dword m_Bits = 0;
    //int m_RobotCnt = 0;
    //bool m_War = false; // Группа вступила в бой

public:
    SMatrixLogicAction m_Action;
    int m_Team = 0;
    float m_Strength = 0.0f;

    int RobotsCnt() const { return m_Bits & 0xFFFF; }
    void RobotsCnt(int a) { m_Bits = (m_Bits & 0xFFFF0000) | a; }
    __forceinline void IncRobotsCnt(int cc = 1) { m_Bits += cc; }

    bool IsWar() const { return FLAG(m_Bits, SETBIT(31)); }
    void SetWar(bool w) { INITFLAG(m_Bits, SETBIT(31), w); }
};

enum EMatrixPlayerOrder
{
    mpo_Stop,
    mpo_MoveTo,
    mpo_Capture,
    mpo_Attack,
    mpo_Patrol,
    mpo_Repair,
    mpo_Bomb,
    mpo_AutoCapture,
    mpo_AutoAttack,
    mpo_AutoDefence
};

struct SMatrixPlayerGroup
{
private:
    dword m_Bits = 0;
    //bool m_War = false;           //Группа вступила в бой
    //bool m_ShowPlace = false;     //Отображение точки, на которую направлен приказ
    //bool m_PatrolReturn = false;
    //EMatrixPlayerOrder m_Order = mpo_Stop;

public:
    int m_RobotCnt = 0;
    CPoint m_From = { 0, 0 };
    CPoint m_To = { 0, 0 };
    CMatrixMapStatic* m_Obj = nullptr;
    int m_Region = 0;
    int m_RegionPathCnt = 0;                       //Кол-во регионов в пути
    int m_RegionPath[REGION_PATH_MAX_CNT] = { 0 }; //Путь по регионам
    CMatrixRoadRoute* m_RoadPath = nullptr;

    //Какой тип приказа задан
    EMatrixPlayerOrder Order() const { return EMatrixPlayerOrder(m_Bits & 0xFF); }
    //Задать тип приказа
    void Order(EMatrixPlayerOrder o) { m_Bits = (m_Bits & 0xFFFFFF00) | o; }

    bool IsWar() const { return FLAG(m_Bits, SETBIT(31)); }
    void SetWar(bool w) { INITFLAG(m_Bits, SETBIT(31), w); }

    bool IsShowPlace() const { return FLAG(m_Bits, SETBIT(30)); }
    void SetShowPlace(bool w) { INITFLAG(m_Bits, SETBIT(30), w); }

    bool IsPatrolReturn() const { return FLAG(m_Bits, SETBIT(29)); }
    void SetPatrolReturn(bool w) { INITFLAG(m_Bits, SETBIT(29), w); }
};

struct SMatrixTeam
{
private:
    dword m_Bits = 0;

    //byte m_Move = 0;
    //bool m_War = false;
    //bool m_Stay = false;
    //bool m_WaitUnion = false;
    //bool m_RobotInDesRegion = false;
    //bool m_RegroupOnlyAfterWar = false;

    //bool m_lOk = false;
public:
    int m_RobotCnt = 0;                         // Кол-во роботов в команде
    int m_GroupCnt = 0;

    int m_WaitUnionLast = 0;

    float m_Strength = 0.0f;

    int m_TargetRegion = -1;
    int m_TargetRegionGoal = 0;

    CPoint m_CenterMass = { 0, 0 };
    int m_RadiusMass = 0;
    CRect m_Rect = { 0, 0, 0, 0 };
    CPoint m_Center = { 0, 0 };
    int m_Radius = 0;
    int m_RegionMassPrev = 0;
    int m_RegionMass = 0;
    int m_RegionNearDanger = 0;                 // Самый опасный регион, включая текущий
    int m_RegionFarDanger = 0;
    int m_RegionNearEnemy = 0;
    int m_RegionNearRetreat = 0;
    int m_RegionNearForward = 0;
    int m_RegionNerestBase = 0;

    int m_Brave = 0;
    float m_BraveStrangeCancel = 0.0f;

    int m_ActionCnt = 0;
    SMatrixLogicAction m_ActionList[16];
    SMatrixLogicAction m_Action;
    SMatrixLogicAction m_ActionPrev;
    int m_ActionTime = 0;
    int m_RegionNext = 0;
    CMatrixRoadRoute* m_RoadPath = nullptr;

    int m_RegionListCnt = 0;                    // Список регионов, в которых находятся роботы 
    int m_RegionList[MAX_ROBOTS] = { 0 };       // Регионы
    int m_RegionListRobots[MAX_ROBOTS] = { 0 }; // Кол-во роботов


    byte Move() const { return byte(m_Bits & 0xFF); }
    void Move(byte m) { m_Bits = (m_Bits & 0xFFFFFF00) | m; }
    void OrMove(byte m) { m_Bits |= m; }

    bool IsWar() const { return FLAG(m_Bits, SETBIT(31)); }
    void SetWar(bool w) { INITFLAG(m_Bits, SETBIT(31), w); }

    bool IsStay() const { return FLAG(m_Bits, SETBIT(30)); }
    void SetStay(bool w) { INITFLAG(m_Bits, SETBIT(30), w); }

    bool IsWaitUnion() const { return FLAG(m_Bits, SETBIT(29)); }
    void SetWaitUnion(bool w) { INITFLAG(m_Bits, SETBIT(29), w); }

    bool IsRobotInDesRegion() const { return FLAG(m_Bits, SETBIT(28)); }
    void SetRobotInDesRegion(bool w) { INITFLAG(m_Bits, SETBIT(28), w); }

    bool IsRegroupOnlyAfterWar() const { return FLAG(m_Bits, SETBIT(27)); }
    void SetRegroupOnlyAfterWar(bool w) { INITFLAG(m_Bits, SETBIT(27), w); }

    bool IslOk() const { return FLAG(m_Bits, SETBIT(26)); }
    void SetlOk(bool w) { INITFLAG(m_Bits, SETBIT(26), w); }
};

struct SMatrixLogicRegion
{
    int m_WarEnemyRobotCnt = 0;
    int m_WarEnemyCannonCnt = 0;
    int m_WarEnemyBuildingCnt = 0;
    int m_WarEnemyBaseCnt = 0;

    int m_EnemyRobotCnt = 0;
    int m_EnemyCannonCnt = 0;
    int m_EnemyBuildingCnt = 0;
    int m_EnemyBaseCnt = 0;

    float m_Danger = 0.0f;        // Кофициент опасности
    float m_DangerAdd = 0.0f;     // Выращенная опасность

    int m_NeutralCannonCnt = 0;
    int m_NeutralBuildingCnt = 0;
    int m_NeutralBaseCnt = 0;

    int m_OurRobotCnt = 0;
    int m_OurCannonCnt = 0;
    int m_OurBuildingCnt = 0;
    int m_OurBaseCnt = 0;

    int m_EnemyRobotDist = 0;
    int m_EnemyBuildingDist = 0;
    int m_OurBaseDist = 0;

    dword m_Data = 0;
};

class CMatrixSideUnit : public CMain
{
private:
    ESideStatus m_SideStatus = SS_NONE;
    int         m_RobotsCnt = 0;

    float m_Strength = 0.0f;               //Сила стороны
    int m_WarSide = -1;                    //На какую сторону стараемся напасть
    //float m_WarSideStrangeCancel = 0.0f; //Перерассчитываем сторону с которой воюем, когда сила стороны упадет ниже критической

    SMatrixLogicRegion* m_Region = nullptr;
    int* m_RegionIndex = nullptr;
    int m_LastTactHL = 0;
    int m_LastTactTL = 0;
    int m_LastTeamChange = 0;
    int m_LastTactUnderfire = 0;

    int m_NextWarSideCalcTime = 0;

    //int m_TitanCnt = 300;
    //int m_ElectronicCnt = 300;
    //int m_EnergyCnt = 300;
    //int m_PlasmaCnt = 300;

    int m_BaseResForce = 0;
    int m_Resources[MAX_RESOURCES] = { 0 };
    int m_CurSelNum = 0;

    int m_Statistic[MAX_STATISTICS] = { 0 };

    CMatrixMapStatic* m_Arcaded = nullptr;
    dword             m_ArcadedP_available = 0;
    D3DXVECTOR3       m_ArcadedP_cur = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3       m_ArcadedP_prevrp = { 0.0f, 0.0f, 0.0f };

    D3DXVECTOR3       m_ArcadedP_ppos0 = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3       m_ArcadedP_ppos1 = { 0.0f, 0.0f, 0.0f };
    float             m_ArcadedP_k = 0.0f;

    CMatrixGroup* m_FirstGroup = nullptr;
    CMatrixGroup* m_LastGroup = nullptr;
    CMatrixGroup* m_CurrentGroup = nullptr; //CMatrixGroup* m_CurGroup;

    CMatrixGroup* m_CurSelGroup = nullptr;

public:
    //In map options
    int   m_TimeNextBomb = 60000;
    int   m_TimeLastBomb = 0;

    float m_StrengthMul = 1.0f;
    float m_BraveMul = 0.5f;
    int   m_TeamCnt = 3;
    float m_DangerMul = 1.0f;
    float m_WaitResMul = 1.0f;

    int*  m_PlaceList = nullptr;

    CMatrixMapStatic* m_ActiveObject = nullptr;
    SRobot            m_Robots[MAX_ROBOTS];
    SCannonForBuild   m_CannonForBuild;

    //Отдельные логические группы роботов для доминаторов и игрока
    //Логические группы разбиваются по исполняемым приказам,
    //то есть N роботов с одинаковым приказом будут добавлены в одну логическую группу
    //"Группа игрока" - это группа, которую выделяет сам игрок. Вероятно, она может не совпадать с логической группой,
    //которая была задана автоматически. Но может я не прав и группа игрока всего лишь дублирует отображение логической группы
    SMatrixLogicGroup m_LogicGroup[MAX_LOGIC_GROUP];
    SMatrixPlayerGroup m_PlayerGroup[MAX_LOGIC_GROUP];
    int m_WaitResForBuildRobot = 0;

    SMatrixTeam m_Team[MAX_TEAM_CNT];

    int m_Id = 0;
    CWStr m_Name = (CWStr)L"";

    dword     m_Color = 0;
    dword     m_ColorMM = 0;
    CTexture* m_ColorTexture = nullptr;

    CConstructor* m_Constructor = nullptr;

    int m_BuildRobotLast = -1;
    int m_BuildRobotLast2 = -1;
    int m_BuildRobotLast3 = -1;

    //Player
    CConstructorPanel* m_ConstructPanel = nullptr;
    CWStr m_PlayerName = (CWStr)L"Player1";
    EPlayerActions m_CurrentAction = NOTHING_SPECIAL;
    int m_nCurrRobotPos = -1;
    ESelection m_CurrSel = NOTHING_SELECTED;

    //CBlockPar* m_TacticsPar = nullptr;
    //CMatrixGroupList* m_GroupsList = nullptr;

    CMatrixSideUnit();
    ~CMatrixSideUnit();

    void BufPrepare();
    void InitPlayerSide();

    void ClearLogicGroupsOrderLists();

    void SetStatus(ESideStatus s) { m_SideStatus = s; }
    ESideStatus GetStatus() const { return m_SideStatus; }

    void ClearStatistics() { memset(m_Statistic, 0, sizeof(m_Statistic)); }
    int  GetStatValue(EStat stat) const { return m_Statistic[stat]; }
    void SetStatValue(EStat stat, int v) { m_Statistic[stat] = v; }
    void IncStatValue(EStat stat, int v = 1) { m_Statistic[stat] += v; }

    int GetRobotsInQueue();
    int GetAlphaCnt() { return m_Team[0].m_RobotCnt; }
    int GetBravoCnt() { return m_Team[1].m_RobotCnt; }
    int GetCharlieCnt() { return m_Team[2].m_RobotCnt; }
    int GetRobotsCnt() { return m_RobotsCnt; }

    int  GetResourcesAmount(ERes res) const { return m_Resources[res]; }
    void AddResourceAmount(ERes res, int amount) { m_Resources[res] += amount; if(m_Resources[res] > 9000) m_Resources[res] = 9000; if(m_Resources[res] < 0) m_Resources[res] = 0; }
    void SetResourceAmount(ERes res, int amount) { m_Resources[res] = amount; }
    void SetResourceForceUp(int fu) { m_BaseResForce = fu; }
    int  GetResourceForceUp() { return m_BaseResForce; }
    bool IsEnoughResources(const int* resources) { if(m_Resources[0] >= resources[0] && m_Resources[1] >= resources[1] && m_Resources[2] >= resources[2] && m_Resources[3] >= resources[3]) return true; else return false; }
    bool IsEnoughResourcesForTurret(const STurretsConsts* resources) { if(m_Resources[0] >= resources->cost_titan && m_Resources[1] >= resources->cost_electronics && m_Resources[2] >= resources->cost_energy && m_Resources[3] >= resources->cost_plasma) return true; else return false; }

    void GetResourceIncome(int& base_i, int& fa_i, ERes resource_type);
    int  GetIncomePerTime(int building, int ms);
    void PLDropAllActions();
    SMatrixTeam* GetTeam(int no) { return m_Team + no; }


    int GetSideRobots() { return m_RobotsCnt; }
    int GetMaxSideRobots();

    void LogicTact(int ms);

    CMatrixGroup* GetCurGroup() { return m_CurrentGroup; }
    void SetCurGroup(CMatrixGroup* group);

    void RemoveObjectFromSelectedGroup(CMatrixMapStatic* o);

    int  GetCurSelNum() { return m_CurSelNum; }
    void SetCurSelNum(int i);
    void ResetSelection();
    CMatrixMapStatic* GetCurSelObject();

    CMatrixGroup* CreateGroupFromCurrent();
    void CreateGroupFromCurrent(CMatrixMapStatic* obj);
    void AddToCurrentGroup();
    void SelectedGroupUnselect();
    void GroupsUnselectSoft();
    void SelectedGroupBreakOrders();
    void SelectedGroupMoveTo(const D3DXVECTOR2& pos);
    void SelectedGroupAttack(CMatrixMapStatic* victim);
    void SelectedGroupCapture(CMatrixMapStatic* building);
    void PumpGroups();

    CMatrixGroup* GetFirstGroup() { return m_FirstGroup; }
    CMatrixGroup* GetLastGroup() { return m_LastGroup; }
    CMatrixGroup* GetCurSelGroup() { return m_CurSelGroup; }

    void RemoveFromSelection(CMatrixMapStatic* o);
    bool FindObjectInSelection(CMatrixMapStatic* o);
    void ResetSystemSelection();
    void __stdcall PlayerAction(void* object);

    void RobotStop(void* pObject);
    void Select(ESelType type, CMatrixMapStatic* object);
    void Reselect();
    void ShowOrderState();
    bool MouseToLand(const CPoint& mouse, float* pWorldX, float* pWorldY, int* pMapX, int* pMapY);
    CMatrixMapStatic* MouseToLand();
    void OnRButtonDown(const CPoint& mouse);
    void OnRButtonDouble(const CPoint& mouse);
    void OnLButtonDown(const CPoint& mouse);
    void OnLButtonDouble(const CPoint& mouse);
    void OnRButtonUp(const CPoint& mouse);
    void OnLButtonUp(const CPoint& mouse);
    void OnForward(bool down);
    void OnBackward(bool down);
    //void OnLeft(bool down);
    //void OnRight(bool down);
    void OnMouseMove();

    // Tactics
    //void GiveRandomOrder();

    // Groups
    //CMatrixGroup* GetGroup(int id, int t);

    bool IsArcadeMode() const { return m_Arcaded != nullptr; }
    bool IsRobotArcadeMode();
    bool IsFlyerArcadeMode();
    CMatrixMapStatic* GetArcadedObject() { return m_Arcaded; }
    void SetArcadedObject(CMatrixMapStatic* o);

    D3DXVECTOR3 CorrectWalkingArcadedRobotHullPos(D3DXVECTOR3& p, CMatrixRobot* robot);
    //void SetArcadedRobotArmorP(const D3DXVECTOR3 &p);
    void InterpolateArcadedRobotHullPos(int ms);

    void SpawnDeliveryFlyer(const D3DXVECTOR2& to, EFlyerOrder order, float ang, int place, const CPoint& bpos, int robot_template, enum EFlyerKind flyer_type, float flyer_structure);

    // STUB:
    int IsInPlaces(const CPoint* places, int placescnt, int x, int y);

    // High logic
    void CalcStrength();
    void Regroup();
    void ClearTeam(int team);
    int ClacSpawnTeam(int region, int nsh);
    void EscapeFromBomb();
    void GroupNoTeamRobot();
    void CalcMaxSpeed();
    void TactHL();
    int FindNearRegionWithUTR(int from, int* exclude_list, int exclude_cnt, dword flags); // 1-our 2-netral 4-enemy 8-base 16-building 32-robot 64-cannon
    int CompareRegionForward(int team, int r1, int r2);
    int CompareAction(int team, SMatrixLogicAction* a1, SMatrixLogicAction* a2);
    void BestAction(int team);
    void LiveAction(int team);
    float BuildRobotMinStrange(CMatrixBuilding* base);
    void ChooseAndBuildAIRobot(int cur_side = 0);
    void ChooseAndBuildAICannon();

    // Theam logic
    void TactTL();
    void WarTL(int group);
    void RepairTL(int group);
    void AssignPlace(CMatrixRobotAI* robot, int region, CPoint* target = nullptr, std::vector<SMatrixRegion*>* all_regions = nullptr);
    void AssignPlace(int group, int region);
    void SortRobotList(CMatrixRobotAI** rl, int rlcnt);
    bool CmpOrder(int team, int group) { ASSERT(team >= 0 && team < m_TeamCnt); return m_LogicGroup[group].m_Action.m_Type == m_Team[team].m_Action.m_Type && m_LogicGroup[group].m_Action.m_Region == m_Team[team].m_Action.m_Region; } // Путь не сравнивается
    void CopyOrder(int team, int group) { ASSERT(team >= 0 && team < m_TeamCnt); m_LogicGroup[group].m_Action = m_Team[team].m_Action; }

    bool PlaceInRegion(CMatrixRobotAI* robot, int place, int region);

    void CalcRegionPath(SMatrixLogicAction* ac, int rend, byte mm);

    bool CanMoveNoEnemy(byte mm, int r1, int r2);

    // Player logic
    void SoundCapture(int pg);

    void TactPL(int onlygroup = -1);
    bool FirePL(int group);
    void RepairPL(int group);
    void WarPL(int group);
    int SelGroupToLogicGroup();
    int RobotToLogicGroup(CMatrixRobotAI* robot);
    void PGOrderStop(int no);
    void PGOrderMoveTo(int no, const CPoint& tp);
    void PGOrderCapture(int no, CMatrixBuilding* building);
    void PGOrderAttack(int no, const CPoint& tp, CMatrixMapStatic* target_obj);
    void PGOrderPatrol(int no, const CPoint& tp);
    void PGOrderRepair(int no, CMatrixMapStatic* target_obj);
    void PGOrderBomb(int no, const CPoint& tp, CMatrixMapStatic* target_obj);
    void PGOrderAutoCapture(int no);
    void PGOrderAutoAttack(int no);
    void PGOrderAutoDefence(int no);
    void PGRemoveAllPassive(int no, CMatrixMapStatic* skip);
    void PGAssignPlace(int no, CPoint& center);
    void PGAssignPlacePlayer(int no, const CPoint& center);
    void PGSetPlace(CMatrixRobotAI* robot, const CPoint& p);
    void PGPlaceClear(int no);
    CPoint PGCalcCenterGroup(int no);
    CPoint PGCalcPlaceCenter(int no);
    void PGShowPlace(int no);
    void PGCalcStat();
    void PGFindCaptureBuilding(int no);
    void PGFindAttackTarget(int no);
    void PGFindDefenceTarget(int no);
    void PGCalcRegionPath(SMatrixPlayerGroup* pg, int rend, byte mm);
    void PGAutoBoomSet(CMatrixGroup* work_group, bool set);

    void BuildCrazyBot();

    void DMTeam(int team, EMatrixLogicActionType ot, int state, const wchar* format, ...);
    void DMSide(const wchar* format, ...);
};

void SideSelectionCallBack(CMatrixMapStatic* ms, dword param);