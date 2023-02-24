// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once
#include "MatrixTactics.h"
//Groups
#define DIF_GROUP_R     200
#define REGROUP_PERIOD  100
#define JUST_PERIOD     100

class CMatrixRobotAI;
class CMatrixSideUnit;
class CMatrixGroup;
class CMatrixTactics;


/*
enum Team
{
    TEAM_UNDEF,
    SYSTEM,
    ALPHA,
    BRAVO,
    CHARLIE
};
*/

//
class CMatrixGroupObject : public CMain
{
public:
    int                 m_Team = -1;
    CMatrixGroup*       m_ParentGroup = nullptr;
    CMatrixMapStatic*   m_Object = nullptr;
    CMatrixGroupObject* m_NextObject = nullptr;
    CMatrixGroupObject* m_PrevObject = nullptr;

public:
    void SetObject(CMatrixMapStatic* object) { m_Object = object; }

    CMatrixMapStatic* GetObject()            { return m_Object; }

    CMatrixGroup* GetParentGroup()           { return m_ParentGroup; }
    void SetParentGroup(CMatrixGroup* group) { m_ParentGroup = group; }

    void SetTeam(int team)                   { m_Team = team; }

    int GetTeam()                            { return m_Team; }

    CMatrixGroupObject() = default;
    ~CMatrixGroupObject() = default;
};

class CMatrixGroup : public CMain
{
public:
    int                 m_Team = -1;
    int                 m_ObjectsCnt = 0;
    int                 m_RobotsCnt = 0;
    int                 m_FlyersCnt = 0;
    int                 m_BuildingsCnt = 0;
    //CMatrixTactics*   m_Tactics = nullptr;
    D3DXVECTOR3         m_GroupPosition = { 0.0f, 0.0f, 0.0f };
    int                 m_Id = 0;
    float               m_GroupSpeed = 0.0f;
    int                 m_SimpleTimer = -1;

    CMatrixGroupObject* m_FirstObject = nullptr;
    CMatrixGroupObject* m_LastObject = nullptr;
    CMatrixGroup*       m_NextGroup = nullptr;
    CMatrixGroup*       m_PrevGroup = nullptr;


    CMatrixGroup() = default;
    ~CMatrixGroup();

    void AddObject(CMatrixMapStatic* object, int team);
    void RemoveObject(CMatrixMapStatic* object);
    bool RemoveObject(int num);
    void RemoveAll();
    void RemoveBuildings();
    void SortFlyers();

    CMatrixMapStatic* GetObjectByN(int num);

    int GetObjectsCnt() { return m_ObjectsCnt; }

    int GetRobotsCnt() { return m_RobotsCnt; }
    int GetFlyersCnt() { return m_FlyersCnt; }
    int GetBuildingsCnt() { return m_BuildingsCnt; }

    void SetTeam(int team) { m_Team = team; }
    int GetTeam() { return m_Team; }

    void FindNearObjects(CMatrixGroupObject* object);
    bool FindObject(CMatrixMapStatic* object);

    //void InstallTactics(TacticsType type, CBlockPar* par);
    //void DeInstallTactics();

    void LogicTact(CMatrixSideUnit* side);
    void CalcGroupPosition();
    void CalcGroupSpeed();
    void SetGroupId(int id) { m_Id = id; }
    int GetGroupId() { return m_Id; }
    //CMatrixTactics* GetTactics() { return m_Tactics; }
    D3DXVECTOR3 GetGroupPos() { return m_GroupPosition; }
    float GetGroupSpeed() { return m_GroupSpeed; }
    void SetGroupSpeed(float speed) { m_GroupSpeed = speed; }

    int GetBombersCnt();
    int GetRepairsCnt();
};

//class CMatrixGroupList : public CMain
//{
//    public:
//        int m_GroupsCnt;
//
//        int m_ReGroupPeriod;
//        CMatrixGroup*   m_FirstGroup;
//        CMatrixGroup*   m_LastGroup;
//
//    public:
//        
//        int GetGroupsCnt()                                                  { return m_GroupsCnt; }
//
//        CMatrixGroup* AddNewGroup(int team, int id);
//        void RemoveGroup(CMatrixGroup* group);
//        void RemoveGroup(int team);
//
//        void ReGroup(CMatrixSideUnit* side);
//
//        //void AddRobotToTeam(Team team, CMatrixRobotAI* robot);
//        //void RemoveRobotFromTeam(Team team, CMatrixRobotAI* robot);
//
//        void AddObject(int team, int id, CMatrixMapStatic* object);
//        void RemoveObject(int team, int id, CMatrixMapStatic* object);
//
//        void LogicTact(CMatrixSideUnit* side);
//
//        CMatrixGroupList()                                                  { m_GroupsCnt=0; m_FirstGroup=nullptr; m_LastGroup=nullptr; m_ReGroupPeriod=0; }
//        ~CMatrixGroupList();
//};
