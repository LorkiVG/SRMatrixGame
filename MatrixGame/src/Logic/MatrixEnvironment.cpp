// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../stdafx.h"
#include "MatrixEnvironment.h"
#include "../MatrixMap.hpp"
#include "../MatrixRobot.hpp"
#include "../MatrixObjectCannon.hpp"

void CEnemy::ClassifyEnemy(CMatrixMapStatic* relTo)
{
    m_EnemyKind = ENEMY_ANY;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfo::Clear(void)
{
DTRACE();

	CEnemy* enemies = m_FirstEnemy;
    while(enemies)
    {
        CEnemy* next = enemies->m_NextEnemy;
        HDelete(CEnemy, enemies, g_MatrixHeap);
        enemies = next;
    }

    m_FirstEnemy = nullptr;
    m_LastEnemy  = nullptr;
}

void CInfo::RemoveAllBuilding(CMatrixMapStatic *skip)
{
    if(m_Target != skip && m_Target && m_Target->GetObjectType() == OBJECT_TYPE_BUILDING) m_Target = nullptr;
    if(m_TargetAttack != skip && m_TargetAttack && m_TargetAttack->GetObjectType() == OBJECT_TYPE_BUILDING) m_TargetAttack = nullptr;

    CEnemy *enemie = m_FirstEnemy;
    while(enemie)
    {
        CEnemy *e2 = enemie;
        enemie = enemie->m_NextEnemy;
        if(e2->GetEnemy() != skip && e2->GetEnemy()->GetObjectType() == OBJECT_TYPE_BUILDING)
        {
            LIST_DEL(e2, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
            HDelete(CEnemy, e2, g_MatrixHeap);
            --m_EnemyCnt;
        }
    }
}

void CInfo::RemoveAllSlowely()
{
    CEnemy *enemie = m_FirstEnemy;
    while(enemie)
    {
        CEnemy *e2 = enemie;
        enemie = enemie->m_NextEnemy;
        if(e2->m_DelSlowly)
        {
            if(e2->GetEnemy() == m_Target) m_Target = nullptr;
            if(e2->GetEnemy() == m_TargetAttack) m_TargetAttack = nullptr;

            LIST_DEL(e2, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
            HDelete(CEnemy, e2, g_MatrixHeap);
            --m_EnemyCnt;
        }
    }
}

void CInfo::RemoveFromList(CMatrixMapStatic *ms)
{
    if(m_Target == ms) m_Target = nullptr;
    if(m_TargetAttack == ms) m_TargetAttack = nullptr;

    CEnemy *enemy = SearchEnemy(ms);
    if(!enemy) return;

    RemoveFromList(enemy);
}

void CInfo::RemoveFromList(CEnemy *enemy)
{
    LIST_DEL(enemy, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
    HDelete(CEnemy, enemy, g_MatrixHeap);
    --m_EnemyCnt;
}

void CInfo::RemoveFromListSlowly(CMatrixMapStatic* ms)
{
    CEnemy *enemy = SearchEnemy(ms);
    if(!enemy)
    {
        if(m_Target == ms) m_Target = nullptr;
        if(m_TargetAttack == ms) m_TargetAttack = nullptr;
        return;
    }

    enemy->m_DelSlowly++;
    if(enemy->m_DelSlowly >= 3)
    {
        if(m_Target == ms) m_Target = nullptr;
        if(m_TargetAttack == ms) m_TargetAttack = nullptr;

        LIST_DEL(enemy, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
        HDelete(CEnemy, enemy, g_MatrixHeap);
        --m_EnemyCnt;
    }
}

void CInfo::AddToList(CMatrixMapStatic *ms)
{
    DelIgnore(ms);

    CEnemy *enemy = HNew(g_MatrixHeap) CEnemy;
    enemy->SetEnemy(ms);
    enemy->ClassifyEnemy(ms);
    LIST_ADD(enemy, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
    ++m_EnemyCnt;
}

void CInfo::AddToListSlowly(CMatrixMapStatic *ms)
{
    DelIgnore(ms);

    CEnemy *enemy = SearchEnemy(ms);
    if(enemy)
    {
        enemy->m_DelSlowly = 0;
        return;
    }

    enemy = HNew(g_MatrixHeap) CEnemy;
    enemy->SetEnemy(ms);
    enemy->ClassifyEnemy(ms);
    LIST_ADD(enemy, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
    ++m_EnemyCnt;
}

void CInfo::Reset()
{
    m_EnemyKind = ENEMY_UNDEF;
    m_Target = nullptr;
    m_TargetAttack = nullptr;
}

CEnemy* CInfo::SearchEnemy(CMatrixMapStatic* ms)
{
    CEnemy* enemie = m_FirstEnemy;
    while(enemie)
    {
        if(enemie->GetEnemy() == ms) return enemie;
        enemie = enemie->m_NextEnemy;
    }
    return nullptr;
}

CMatrixMapStatic* CInfo::GetEnemyByKind(dword kind)
{
    CEnemy* enemies = m_FirstEnemy;
    while(enemies)
    {
        if(enemies->GetKind() == kind) return enemies->GetEnemy();
        enemies = enemies->m_NextEnemy;
    }
    return nullptr;
}
void CInfo::AddBadPlace(int place)
{
    if(IsBadPlace(place)) return;

    if(m_BadPlaceCnt < 8)
    {
        m_BadPlace[m_BadPlaceCnt] = place;
        ++m_BadPlaceCnt;
    }
    else
    {
        memmove(m_BadPlace, m_BadPlace + 1, (8 - 1) * sizeof(int));
        m_BadPlace[m_BadPlaceCnt - 1] = place;
    }
}

bool CInfo::IsBadPlace(int place)
{
    for(int i = 0; i < m_BadPlaceCnt; ++i)
    {
        if(m_BadPlace[i] == place) return true;
    }
    return false;
}

void CInfo::AddBadCoord(const CPoint& coord)
{
    if(IsBadCoord(coord)) return;

    if(m_BadCoordCnt < 16)
    {
        m_BadCoord[m_BadCoordCnt] = coord;
        ++m_BadCoordCnt;
    }
    else
    {
        memmove(m_BadCoord, m_BadCoord + 1, (16 - 1) * sizeof(int));
        m_BadCoord[m_BadCoordCnt - 1] = coord;
    }
}

bool CInfo::IsBadCoord(const CPoint& coord)
{
    for(int i = 0; i < m_BadCoordCnt; ++i)
    {
        if(m_BadCoord[i] == coord) return true;
    }
    return false;
}

void CInfo::AddIgnore(CMatrixMapStatic* ms)
{
    int empty = -1;
    for(int i = 0; i < m_IgnoreCnt; ++i)
    {
        if(m_Ignore[i] == ms)
        {
            m_IgnoreTime[i] = g_MatrixMap->GetTime();
            return;
        }
        else if(empty < 0 && (m_Ignore[i] == nullptr || (g_MatrixMap->GetTime() - m_IgnoreTime[i]) >= 5000))
        {
            empty = i;
        }
    }

    if(empty >= 0)
    {
        m_Ignore[empty] = ms;
        m_IgnoreTime[empty] = g_MatrixMap->GetTime();
    }
    else if(m_IgnoreCnt < 16)
    {
        m_Ignore[m_IgnoreCnt] = ms;
        m_IgnoreTime[m_IgnoreCnt] = g_MatrixMap->GetTime();
        ++m_IgnoreCnt;
    }
    else
    {
        memmove(m_Ignore, m_Ignore + 1, (16 - 1) * sizeof(CMatrixMapStatic*));
        memmove(m_IgnoreTime, m_IgnoreTime + 1, (16 - 1) * sizeof(int));
        m_Ignore[m_IgnoreCnt - 1] = ms;
    }
}

bool CInfo::IsIgnore(CMatrixMapStatic* ms)
{
    for(int i = 0; i < m_IgnoreCnt; ++i)
    {
        if(m_Ignore[i] == ms && (g_MatrixMap->GetTime() - m_IgnoreTime[i]) < 5000)
        {
            return true;
        }
    }
    return false;
}

void CInfo::DelIgnore(CMatrixMapStatic* ms)
{
    for(int i = 0; i < m_IgnoreCnt; ++i)
    {
        if(m_Ignore[i] == ms)
        {
            m_Ignore[i] = nullptr;
        }
    }
}