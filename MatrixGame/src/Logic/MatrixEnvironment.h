// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixRule.h"

#pragma once
class CMatrixRobotAI;
class CMatrixMapStatic;

class CEnemy: public CMain {
public:
    dword             m_EnemyKind = ENEMY_UNDEF;
    CMatrixMapStatic* m_Enemy = nullptr;

    CEnemy*           m_NextEnemy = nullptr;
    CEnemy*           m_PrevEnemy = nullptr;

    int               m_DelSlowly = 0;

public:
    CEnemy() = default;
    ~CEnemy() = default;

    void SetEnemy(CMatrixMapStatic* ms) { m_Enemy = ms; }
    CMatrixMapStatic* GetEnemy()        { return m_Enemy; }

    void SetKind(dword kind)            { m_EnemyKind |= kind; }
    dword GetKind()                     { return m_EnemyKind; }

    void ClassifyEnemy(CMatrixMapStatic* relTo);
};

class CInfo : public CMain {
public:
    int                 m_EnemyCnt = 0;

    dword               m_EnemyKind = 0;
    CMatrixMapStatic*   m_Target = nullptr;
    CMatrixMapStatic*   m_TargetAttack = nullptr;
    CMatrixMapStatic*   m_TargetLast = nullptr;     // Последняя цель, переменная нужна для определения, когда изменилась цель
    int                 m_TargetChange = 0;         // Время когда цель сменилась
    int                 m_TargetChangeRepair = 0;   // Время когда цель сменилась
    float               m_TargetAngle = 0.0f;

    int                 m_Place = -1;               // Место в котором стоит или в которое идет робот
    CPoint              m_PlaceAdd = { -1, -1 };    // Дополнительное место, назначенное игроком
    int                 m_PlaceNotFound = -1000000; // Место не найдено. Нужно заново искать через определённое время
    bool                m_OrderNoBreak = false;     // Выполняется приказ который нельзя прервать (bool variable)
    int                 m_LastFire = 0;             // Время, когда робот последний раз выстрелил
    int                 m_LastHitTarget = 0;        // Время, когда робот последний раз попал в цель
    int                 m_LastHitEnemy = 0;         // Время, когда робот последний раз попал во врага
    int                 m_LastHitFriendly = 0;      // Время, когда робот последний раз попал в дружественный объект

    int                 m_BadPlaceCnt = 0;
    int                 m_BadPlace[8] = { 0 };      // Список плохих мест, куда становится не стоит

    int                 m_BadCoordCnt = 0;
    CPoint              m_BadCoord[16];             // Список плохих координат, когда робот пропускает другого то эти координаты пропускаются

    int                 m_IgnoreCnt = 0;            // Список целей, на которые не обращаем внимания
    CMatrixMapStatic*   m_Ignore[16] = { nullptr };
    int                 m_IgnoreTime[16] = { 0 };

    CEnemy*             m_FirstEnemy = nullptr;
    CEnemy*             m_LastEnemy = nullptr;

public:
    CInfo() = default;
    ~CInfo() { Clear(); }
    
    void Clear();

    int TargetType() { if(m_Target == nullptr) return 0; if(SearchEnemy(m_Target)) return 1; return 2; }

    void RemoveAllBuilding(CMatrixMapStatic* skip = nullptr);
    void RemoveAllSlowely();
    
    void RemoveFromList(CMatrixMapStatic* ms);
    void RemoveFromList(CEnemy* enemy);
    void RemoveFromListSlowly(CMatrixMapStatic* ms);
    void AddToList(CMatrixMapStatic* ms);
    void AddToListSlowly(CMatrixMapStatic* ms);
    CEnemy* SearchEnemy(CMatrixMapStatic* ms);
    CMatrixMapStatic* GetEnemyByKind(dword kind);    
    int GetEnemyCnt() { return m_EnemyCnt; }
    void KillEnemyByKind(dword kind) { m_EnemyKind = kind; }

    void AddBadPlace(int place);
    bool IsBadPlace(int place);

    void AddBadCoord(CPoint& coord);
    bool IsBadCoord(CPoint& coord);

    void AddIgnore(CMatrixMapStatic* ms);
    bool IsIgnore(CMatrixMapStatic* ms);
    void DelIgnore(CMatrixMapStatic* ms);

    void Reset();
};