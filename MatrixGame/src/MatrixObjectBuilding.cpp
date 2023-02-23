// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixObjectBuilding.hpp"
#include "MatrixShadowManager.hpp"
#include "MatrixRenderPipeline.hpp"
#include "MatrixObject.hpp"
#include "MatrixFlyer.hpp"
#include "Logic/MatrixAIGroup.h"
#include "MatrixRobot.hpp"
#include "MatrixSkinManager.hpp"
#include "MatrixObjectCannon.hpp"
#include "Interface/CInterface.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable:4355)

CMatrixBuilding::CMatrixBuilding() : CMatrixMapStatic(), m_Name(L"FACTORY", g_MatrixHeap), m_BS(this), m_UnderAttackTime(0)
{
DTRACE();

	m_GGraph = nullptr;
    //m_ShadowStencil = nullptr;
	m_ShadowProj = nullptr;

	m_Core->m_Type = OBJECT_TYPE_BUILDING;

    m_Side = 0;
    m_Kind = BUILDING_BASE;

    m_Pos.x = m_Pos.y = 0;
    m_Angle = 0;

    m_ShadowType = SHADOW_STENCIL;
    m_ShadowSize = 128;

    m_BaseFloor = 0.2f;
    m_State = BASE_CLOSING;

	m_GGraph = HNew(g_MatrixHeap) CVectorObjectGroup();
    /*m_Busy = false;*/

    m_Capturer = nullptr;
    m_Capture = nullptr;
    m_InCaptureTime = 0;
    m_CaptureSeekRobotNextTime = 0;

    m_ResourcePeriod = 0;
    m_TurretsHave = 0;

    m_defHitPoint = 0;

    m_TurretsPlacesCnt = 0;

    //m_Places = (int*)HAlloc(sizeof(int) * (MAX_PLACES * 2) + 4, g_MatrixHeap);
    //ZeroMemory(m_Places, sizeof(int) * (MAX_PLACES * 2));

    m_PlacesShow = nullptr;
    m_Selection = nullptr;

    m_LastDelayDamageSide = 0;

    m_CaptureMeNextTime = 0;
    m_ShowHitpointTime = 0;
}

#pragma warning (default:4355)

CMatrixBuilding::~CMatrixBuilding()
{
DTRACE();

    m_BS.ClearStack();
    UnSelect();

    int x = Float2Int(m_Pos.x * INVERT(GLOBAL_SCALE));
    int y = Float2Int(m_Pos.x * INVERT(GLOBAL_SCALE));

    for(int i = x - 4; i < (x + 4); ++i)
    {
        for(int j = x - 4; j < (x + 4); ++j)
        {
            SMatrixMapUnit* mu = g_MatrixMap->UnitGetTest(i, j);
            if(mu == nullptr) continue;
            if(mu->m_MapUnitBase == this) mu->m_MapUnitBase = nullptr;
        }
    }

    ReleaseMe();
    //if(m_Graph) { HDelete(CVectorObjectAnim, m_Graph, g_MatrixHeap); m_Graph = nullptr; }
    //if(m_ShadowStencil) { HDelete(CVOShadowStencil, m_ShadowStencil, g_MatrixHeap); m_ShadowStencil = nullptr; }
    if(m_GGraph) { HDelete(CVectorObjectGroup, m_GGraph, g_MatrixHeap); }
    if(m_ShadowProj) { HDelete(CMatrixShadowProj, m_ShadowProj, g_MatrixHeap); m_ShadowProj = nullptr; }

#ifdef _DEBUG
    if(m_Capture) g_MatrixMap->SubEffect(DEBUG_CALL_INFO, m_Capture);
#else
    if(m_Capture) g_MatrixMap->SubEffect(m_Capture);
#endif

    //if(m_Places) HFree(m_Places, g_MatrixHeap);

    DeletePlacesShow();
}

void CMatrixBuilding::GetResources(dword need)
{
DTRACE();

    if(need & m_RChange & MR_Matrix)
    {
        m_RChange &= ~MR_Matrix;

        // rotate
        switch (m_Angle)
        {
        case 0:
            m_Core->m_Matrix._11 = 1; m_Core->m_Matrix._12 = 0;
            m_Core->m_Matrix._21 = 0; m_Core->m_Matrix._22 = 1;
            break;
        case 1:
            m_Core->m_Matrix._11 = 0; m_Core->m_Matrix._12 = 1;
            m_Core->m_Matrix._21 = -1; m_Core->m_Matrix._22 = 0;
            break;
        case 2:
            m_Core->m_Matrix._11 = -1; m_Core->m_Matrix._12 = 0;
            m_Core->m_Matrix._21 = 0; m_Core->m_Matrix._22 = -1;
            break;
        case 3:
            m_Core->m_Matrix._11 = 0; m_Core->m_Matrix._12 = -1;
            m_Core->m_Matrix._21 = 1; m_Core->m_Matrix._22 = 0;
        }

        m_Core->m_Matrix._13 = 0; m_Core->m_Matrix._14 = 0;
        m_Core->m_Matrix._31 = 0; m_Core->m_Matrix._32 = 0; m_Core->m_Matrix._33 = 1; m_Core->m_Matrix._34 = 0;
        m_Core->m_Matrix._41 = m_Pos.x; m_Core->m_Matrix._42 = m_Pos.y; m_Core->m_Matrix._43 = m_BuildZ; m_Core->m_Matrix._44 = 1;

		m_GGraph->m_GroupToWorldMatrix = &m_Core->m_Matrix;

        D3DXMatrixInverse(&m_Core->m_IMatrix, nullptr, &m_Core->m_Matrix);

		m_GGraph->RChange(VOUF_MATRIX);
		m_GGraph->GetResources(VOUF_MATRIX);
	}

	if(need & m_RChange & MR_Graph)
    {
        m_RChange &= ~MR_Graph;

        if(!m_GGraph->IsAlreadyLoaded())
        {
            float hp = g_Config.m_BuildingsHitPoints[m_Kind];
            InitMaxHitpoint(hp);

            if(m_Kind == BUILDING_BASE) m_GGraph->Load(OBJECT_PATH_BASE_00, CMatrixEffect::GetSingleBrightSpriteTex(SPR_SPOT_LIGHT), CSkinManager::GetSkin, GSP_SIDE);
            else if(m_Kind == BUILDING_TITAN) m_GGraph->Load(OBJECT_PATH_BASE_01, CMatrixEffect::GetSingleBrightSpriteTex(SPR_SPOT_LIGHT), CSkinManager::GetSkin, GSP_SIDE);
            else if(m_Kind == BUILDING_PLASMA) m_GGraph->Load(OBJECT_PATH_BASE_02, CMatrixEffect::GetSingleBrightSpriteTex(SPR_SPOT_LIGHT), CSkinManager::GetSkin, GSP_SIDE);
            else if(m_Kind == BUILDING_ELECTRONIC) m_GGraph->Load(OBJECT_PATH_BASE_03, CMatrixEffect::GetSingleBrightSpriteTex(SPR_SPOT_LIGHT), CSkinManager::GetSkin, GSP_SIDE);
            else if(m_Kind == BUILDING_ENERGY) m_GGraph->Load(OBJECT_PATH_BASE_04, CMatrixEffect::GetSingleBrightSpriteTex(SPR_SPOT_LIGHT), CSkinManager::GetSkin, GSP_SIDE);
            else if(m_Kind == BUILDING_REPAIR) m_GGraph->Load(OBJECT_PATH_BASE_05, CMatrixEffect::GetSingleBrightSpriteTex(SPR_SPOT_LIGHT), CSkinManager::GetSkin, GSP_SIDE);
        }
	}

	if(need & m_RChange & MR_ShadowStencil)
    {
        m_RChange &= ~MR_ShadowStencil;

		if(m_ShadowType != SHADOW_STENCIL) m_GGraph->ShadowStencilOn(false);
        else
        {
            m_GGraph->m_ShadowStencilLight = g_MatrixMap->m_LightMain;
            m_GGraph->m_GroundZ = (m_Kind == BUILDING_BASE) ? g_MatrixMap->m_GroundZBase : g_MatrixMap->m_GroundZ;
            m_GGraph->ShadowStencilOn(true);
            //m_GGraph->RChange(VOUF_SHADOWSTENCIL); //Tact will modifies this flag
            m_GGraph->GetResources(VOUF_MATRIX | VOUF_SHADOWSTENCIL);
		}

        /*
		if(m_ShadowType != SHADOW_STENCIL)
        {
			if(m_ShadowStencil)
            {
                HDelete(CVOShadowStencil, m_ShadowStencil, g_MatrixHeap);
                m_ShadowStencil = nullptr;
            }
		}
        else if(true)
        {
			ASSERT(m_Graph);
			ASSERT(m_Graph->VO());

			if(!m_ShadowStencil) m_ShadowStencil = HNew(g_MatrixHeap) CVOShadowStencil;

			if(!(m_Graph->VO()->EdgeExist())) m_Graph->VO()->EdgeBuild();

			m_ShadowStencil->Build(*(m_Graph->VO()), m_Graph->FrameVO(), g_MatrixMap->m_LightMain, m_Matrix, g_MatrixMap->m_ShadowPlaneCut);
		}
        */
	}

	if(need & m_RChange & MR_ShadowProjGeom)
    {
		m_RChange &= ~MR_ShadowProjGeom;

        // TO DO: see mapobject and robots

        /*
        if(m_ShadowType!=SHADOW_PROJ_STATIC && m_ShadowType!=SHADOW_PROJ_DYNAMIC)
        {
			if(m_ShadowProj)
            {
                HDelete(CVOShadowProj, m_ShadowProj, g_MatrixHeap);
                m_ShadowProj = nullptr;
            }
		}
        else if(1)
        {
			ASSERT(m_Graph);
			ASSERT(m_Graph->VO());

			if(!m_ShadowProj) m_ShadowProj=HNew(g_MatrixHeap) CVOShadowProj(g_MatrixHeap);

			if(m_ShadowType==SHADOW_PROJ_DYNAMIC)
            {
				CTexture * tex=m_ShadowProj->GetTexture();
				m_ShadowProj->SetTexture(nullptr);
				if(tex)
                {
					tex->RefDec();
					ASSERT(tex->Ref()==0);
				}
				ShadowProjBuild(*m_ShadowProj,*(m_Graph->VO()),tex,m_Graph->FrameVO(),m_Matrix,g_MatrixMap->m_LightMain,10,m_ShadowSize, true);
			}
            else
            {
				CTexture * tex=m_ShadowProj->GetTexture();
				m_ShadowProj->SetTexture(nullptr);
				if(tex)
                {
					tex->RefDec();
					if(tex->Ref()<=0) CCache::Destroy(tex); 
				}
				tex=nullptr;
				CMatrixMapStatic * so=g_MatrixMap->m_StaticFirst;
				while(so)
                {
					if( so->GetObjectType()==OBJECT_TYPE_BUILDING &&
                        ((CMatrixBuilding *)so)->m_ShadowProj &&
                        ((CMatrixBuilding *)so)->m_ShadowProj->GetTexture() &&
                        ((CMatrixBuilding *)so)->m_ShadowType==m_ShadowType &&
                        ((CMatrixBuilding *)so)->m_ShadowSize==m_ShadowSize &&
                        ((CMatrixBuilding *)so)->m_Kind==m_Kind)
                    {
                        tex=((CMatrixBuilding *)so)->m_ShadowProj->GetTexture();
                        break;
                    }
                    so = so->m_Next;
				}
                //(CWStr(L"SB_")+CWStr(m_ShadowSize)+CWStr(L"_")+CWStr(m_Kind)).Get()
				ShadowProjBuild(*m_ShadowProj,*(m_Graph->VO()),tex,0,m_Matrix,g_MatrixMap->m_LightMain,10,m_ShadowSize,false);
			}
			if(!(m_ShadowProj->IsProjected()))
            {
				HDelete(CVOShadowProj,m_ShadowProj,g_MatrixHeap);
				m_ShadowProj=nullptr;
			}
		}
        */
	}

	if(need & m_RChange & MR_MiniMap)
    {
		m_RChange &= ~MR_MiniMap;
        g_MatrixMap->m_Minimap.RenderObjectToBackground(this);
	}
}

bool CMatrixBuilding::TakingDamage(
    int weap,
    const D3DXVECTOR3& pos,
    const D3DXVECTOR3&,
    int attacker_side,
    CMatrixMapStatic* attaker
)
{
DTRACE();

    if(m_State == BUILDING_DIP || m_State == BUILDING_DIP_EXPLODED) return true;

    bool friendly_fire = (attacker_side != NEUTRAL_SIDE) && (attacker_side == m_Side);
    float damage_coef = (friendly_fire || m_Side != PLAYER_SIDE) ? 1.0f : g_MatrixMap->m_Difficulty.coef_enemy_damage_to_player_side;
    if(friendly_fire && m_Side == PLAYER_SIDE) damage_coef = damage_coef * g_MatrixMap->m_Difficulty.coef_friendly_fire;

    CMatrixEffectWeapon::SoundHit(weap, pos);

    if(g_Config.m_WeaponsConsts[weap].is_repairer)
    {
        m_HitPoint = min(m_HitPoint + g_Config.m_WeaponsConsts[weap].damage.to_buildings, m_HitPointMax);
        m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);
        return false;
    }

    //m_HitPoint = max(m_HitPoint - damage_coef * (friendly_fire ? g_Config.m_WeaponsConsts[weap].friendly_damage.to_buildings : g_Config.m_WeaponsConsts[weap].damage.to_buildings), g_Config.m_WeaponsConsts[weap].non_lethal_threshold.to_buildings);
    m_HitPoint = max(m_HitPoint - damage_coef * g_Config.m_WeaponsConsts[weap].damage.to_buildings, g_Config.m_WeaponsConsts[weap].non_lethal_threshold.to_buildings);
    if(m_HitPoint >= 0) m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);
    else m_ProgressBar.Modify(0);

    if(m_Side == PLAYER_SIDE && !friendly_fire)
    {
        if(!m_UnderAttackTime)
        {
            int ss = IRND(3);
            if(!ss) CSound::Play(S_SIDE_UNDER_ATTACK_1);
            else if(ss == 1) CSound::Play(S_SIDE_UNDER_ATTACK_2);
            else CSound::Play(S_SIDE_UNDER_ATTACK_3);
        }
        m_UnderAttackTime = UNDER_ATTACK_IDLE_TIME;
    }

    //Проверяем, какие эффекты накладывает попавшее в здание оружие
    if(g_Config.m_WeaponsConsts[weap].extra_effects.size())
    {
        for(int i = 0; i < (int)g_Config.m_WeaponsConsts[weap].extra_effects.size(); ++i)
        {
            int effect = g_Config.m_WeaponsConsts[weap].extra_effects[i].type;
            byte effect_type = g_Config.m_WeaponsConsts[effect].secondary_effect;
            if(effect_type == SECONDARY_EFFECT_ABLAZE)
            {
                if(!g_Config.m_WeaponsConsts[effect].damage.to_buildings) continue;
                int new_priority = g_Config.m_WeaponsConsts[effect].effect_priority;
                if(IsAblaze()) effect = new_priority >= g_Config.m_WeaponsConsts[IsAblaze()].effect_priority ? effect : IsAblaze(); //Если наложенный ранее эффект горения имеет более высокий приоритет, то обновляем эффект по его статам
                MarkAblaze(effect);
                m_LastDelayDamageSide = attacker_side;

                int ttl = GetAblazeTTL();
                ttl = min(max(ttl + g_Config.m_WeaponsConsts[weap].extra_effects[i].duration_per_hit, 0), g_Config.m_WeaponsConsts[weap].extra_effects[i].max_duration);
                SetAblazeTTL(ttl);

                m_NextTimeAblaze = g_MatrixMap->GetTime(); //То есть в первый раз считаем логику получения урона от огня немедленно
            }
        }
    }
    else m_LastDelayDamageSide = 0;

    //Функция выставления маркера смерти здания "death in progress"
    if(m_HitPoint <= 0)
    {
        if(m_Side == PLAYER_SIDE)
        {
            bool bu = FRND(1) < 0.7f;
            if(bu)
            {
                if(m_Kind == BUILDING_BASE) CSound::Play(S_BASE_KILLED);
                else CSound::Play(S_FACTORY_KILLED);
            }
            else CSound::Play(S_BUILDING_KILLED);
        }

        ReleaseMe();
        m_HitPoint = -1;
        m_State = BUILDING_DIP;

        if(attacker_side != NEUTRAL_SIDE && !friendly_fire)
        {
            g_MatrixMap->GetSideById(attacker_side)->IncStatValue(STAT_BUILDING_KILL);
        }

        m_NextExplosionTime = g_MatrixMap->GetTime();
        m_NextExplosionTimeSound = g_MatrixMap->GetTime();
        g_MatrixMap->RemoveEffectSpawnerByObject(this);
        return true;
    }

    return false;
}

void CMatrixBuilding::OnOutScreen(void) {}

//Отрисовываем точку сбора для выбранного игроком здания (если точка сбора для него вообще установлена)
void CMatrixBuilding::ShowGatheringPointTact(int time)
{
    if(m_Side == PLAYER_SIDE)
    {
        if(m_ShowGatheringPointTime - time <= 0)
        {
            //Обновляем время срабатывания таймера
            m_ShowGatheringPointTime += GATHERING_POINT_SHOW_TIME;

            if(g_MatrixMap->GetPlayerSide()->m_ActiveObject == this)
            {
                D3DXVECTOR3 v;
                v.x = m_GatheringPoint.CoordX;
                v.y = m_GatheringPoint.CoordY;
                v.z = g_MatrixMap->GetZ(v.x, v.y);
                CMatrixEffect::CreateMoveToAnim(v, 2);
            }
        }
    }
}

void CMatrixBuilding::Tact(int cms)
{
DTRACE();

    if(m_Side == PLAYER_SIDE)
    {
        //Отрисовываем иконки поступающих ресурсов над зданием
        if(FLAG(m_ObjectFlags, BUILDING_NEW_INCOME))
        {
            RESETFLAG(m_ObjectFlags, BUILDING_NEW_INCOME);
            switch (m_Kind)
            {
            case BUILDING_TITAN:
                CMatrixEffect::CreateBillboardScore(L"t10", D3DXVECTOR3(GetGeoCenter().x, GetGeoCenter().y, GetGeoCenter().z + 40.0f), 0xFFFFFFFF);
                break;
            case BUILDING_PLASMA:
                CMatrixEffect::CreateBillboardScore(L"p10", D3DXVECTOR3(GetGeoCenter().x, GetGeoCenter().y, GetGeoCenter().z + 40.0f), 0xFFFFFFFF);
                break;
            case BUILDING_ELECTRONIC:
                CMatrixEffect::CreateBillboardScore(L"e10", D3DXVECTOR3(GetGeoCenter().x, GetGeoCenter().y, GetGeoCenter().z + 40.0f), 0xFFFFFFFF);
                break;
            case BUILDING_ENERGY:
                CMatrixEffect::CreateBillboardScore(L"b10", D3DXVECTOR3(GetGeoCenter().x, GetGeoCenter().y, GetGeoCenter().z + 40.0f), 0xFFFFFFFF);
                break;
            case BUILDING_BASE:
                {
                    int fu = g_MatrixMap->GetPlayerSide()->GetResourceForceUp();
                    int prihod = RESOURCES_INCOME_BASE * fu / 100;
                    CMatrixEffect::CreateBillboardScore(L"a" + CWStr(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFFFF);

                    /*
                    if(m_BaseRCycle == 0)
                    {
                       CMatrixEffect::CreateBillboardScore(L"t" + CWStr(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFF00);
                    }
                    else if(m_BaseRCycle == 1)
                    {
                       CMatrixEffect::CreateBillboardScore(L"e" + CWStr(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFF00);
                    }
                    else if(m_BaseRCycle == 2)
                    {
                       CMatrixEffect::CreateBillboardScore(L"b" + CWStr(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFF00);
                    }
                    else if(m_BaseRCycle == 3)
                    {
                       CMatrixEffect::CreateBillboardScore(L"p" + CWStr(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFF00);
                    }
                    */
                }
                break;
            }
        }
    }
    
    if(m_Capture)
    {
        m_Capture->UpdateData(m_TrueColor.m_Color, m_TrueColor.m_ColoredCnt);
    }

    //Здесь крутим анимации всех анимированных объектов на здании
	if(m_GGraph)
    {
		if(m_GGraph->Tact(cms))
        {
            if(m_ShadowType == SHADOW_STENCIL)
            {
                RChange(MR_ShadowStencil);
                //m_GGraph->RChange(MR_ShadowStencil);
            }
            else if(m_ShadowType == SHADOW_PROJ_DYNAMIC)
            {
                RChange(MR_ShadowProjTex);
                //m_GGraph->RChange(MR_ShadowProj);
            }
		}
	}

    if(!m_Capture && !IsBase())
    {
        if(m_Core->m_Matrix._22 > 0)
        {
            m_Capture = (CMatrixEffectZahvat*)CMatrixEffect::CreateZahvat(
                D3DXVECTOR3(m_Pos.x + m_Core->m_Matrix._21 * 2.7f, m_Pos.y + m_Core->m_Matrix._22 * 2.7f, m_Core->m_Matrix._43 + 0.8f), 24, GRAD2RAD(102), MAX_ZAHVAT_POINTS);
        }
        else if(m_Core->m_Matrix._21 > 0)
        {
            m_Capture = (CMatrixEffectZahvat*)CMatrixEffect::CreateZahvat(
                D3DXVECTOR3(m_Pos.x + m_Core->m_Matrix._21 * 2.7f, m_Pos.y + m_Core->m_Matrix._22 * 2.7f, m_Core->m_Matrix._43 + 0.8f), 24, GRAD2RAD(12), MAX_ZAHVAT_POINTS);
        }
        else if(m_Core->m_Matrix._21 < 0)
        {
            m_Capture = (CMatrixEffectZahvat*)CMatrixEffect::CreateZahvat(
                D3DXVECTOR3(m_Pos.x + m_Core->m_Matrix._21 * 2.7f, m_Pos.y + m_Core->m_Matrix._22 * 2.7f, m_Core->m_Matrix._43 + 0.8f), 24, GRAD2RAD(192), MAX_ZAHVAT_POINTS);
        }
        else if(m_Core->m_Matrix._22 < 0)
        {
            m_Capture = (CMatrixEffectZahvat*)CMatrixEffect::CreateZahvat(
                D3DXVECTOR3(m_Pos.x + m_Core->m_Matrix._21 * 2.7f, m_Pos.y + m_Core->m_Matrix._22 * 2.7f, m_Core->m_Matrix._43 + 0.8f), 24, GRAD2RAD(-79), MAX_ZAHVAT_POINTS);
        }

        if(!g_MatrixMap->AddEffect(m_Capture))
        { 
            m_Capture = nullptr;
        }
        else
        {
            if(!m_Side)
            {
                m_TrueColor.m_Color = 0;
                m_TrueColor.m_ColoredCnt = 0;
            }
            else
            {
                m_TrueColor.m_Color = (0xFF000000) | g_MatrixMap->GetSideColor(m_Side);
                m_TrueColor.m_ColoredCnt = MAX_ZAHVAT_POINTS;
            }
        }
    }
}

struct SFindRobotForCaptureAny
{
    CMatrixRobotAI* found;
    float           dist2;
};

static bool FindCaptureMe(const D3DXVECTOR2& center, CMatrixMapStatic* ms, dword user)
{
DTRACE();

    CMatrixBuilding* b = (CMatrixBuilding*)user;

    if(ms->AsRobot()->m_Side != b->m_Side) ms->AsRobot()->AddCaptureCandidate(b);
    return true;
}

static bool FindRobotForCaptureAny(const D3DXVECTOR2& center, CMatrixMapStatic* ms, dword user)
{
DTRACE();

    SFindRobotForCaptureAny* data = (SFindRobotForCaptureAny*)user;

    float dist2 = D3DXVec2LengthSq(&(center - *(D3DXVECTOR2*)&ms->GetGeoCenter()));
    if(dist2 < data->dist2)
    {

        data->found = (CMatrixRobotAI*)ms;
        data->dist2 = dist2;
    }

    return true;
}

void CMatrixBuilding::PauseTact(int cms)
{
DTRACE();

    m_ProgressBar.Modify(100000.0f, 0);

    if(m_State != BUILDING_DIP && m_State != BUILDING_DIP_EXPLODED)
    {
        m_ShowHitpointTime -= cms;
        if(m_ShowHitpointTime < 0)
        {
            m_ShowHitpointTime = 0;
        }
    }
}

void CMatrixBuilding::LogicTact(int cms)
{
DTRACE();

    m_ProgressBar.Modify(100000.0f, 0);
    CMatrixMapStatic* obj;

    if(IsAblaze())
    {
        int ablaze_effect_num = IsAblaze();

        while(g_MatrixMap->GetTime() > m_NextTimeAblaze)
        {
            m_NextTimeAblaze += ABLAZE_LOGIC_PERIOD;

            D3DXVECTOR3 dir, pos;
            float t;

            int cnt = 4;
            do
            {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                D3DXVec3Normalize(&dir, &D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x, m_Core->m_Matrix._42 - pos.y, m_Core->m_Matrix._43 - pos.z));

            } while(!Pick(pos, dir, &t) && (--cnt > 0));

            CMatrixEffect::CreateFireAnim(nullptr, pos + dir * (t + 6), 20, 30 + FRND(20), GetAblazeTTL() - Float2Int(FRND(4000)), g_Config.m_WeaponsConsts[ablaze_effect_num].sprite_set[0].sprites_num);
            //CMatrixEffect::CreateSmoke(nullptr, pos + dir * (t + 6) + D3DXVECTOR3(0, 0, 10), 100, 1300, 20, 0x80303030, false, 1.0f / 30.0f);
            if(cnt > 0) CMatrixEffect::CreateFire(nullptr, pos + dir * (t + 2), 100, 2500, 10, 5.0f, false, FIRE_SPEED, g_Config.m_WeaponsConsts[ablaze_effect_num].close_color_rgb, g_Config.m_WeaponsConsts[ablaze_effect_num].far_color_rgb);

            //Если здание было уничтожено огнём
            if(TakingDamage(ablaze_effect_num, pos, dir, m_LastDelayDamageSide, nullptr)) return;
        }
    }

    //Если база не находится в состоянии смерти (death in progress) и не взрывается
    if(m_State != BUILDING_DIP && m_State != BUILDING_DIP_EXPLODED)
    {
        if(m_CaptureMeNextTime < g_MatrixMap->GetTime() && m_Capturer == nullptr && !(IsBase() && m_TurretsHave > 0))
        {
            m_CaptureMeNextTime = g_MatrixMap->GetTime() + 1002;

            CMatrixMapStatic* r = CMatrixMapStatic::GetFirstLogic();   
            for(; r; r = r->GetNextLogic())
            {
                if(r->IsRobot()) r->AsRobot()->UnMarkCaptureInformed();
            }
                
            g_MatrixMap->FindObjects(m_Pos, DISTANCE_CAPTURE_ME, 1, TRACE_ROBOT, m_Capturer, FindCaptureMe, (dword)this);

            // should not be captured...
            r = CMatrixMapStatic::GetFirstLogic();   
            for(; r; r = r->GetNextLogic())
            {
                if(r->IsRobot() && !r->AsRobot()->IsCaptureInformed())
                {
                    r->AsRobot()->RemoveCaptureCandidate(this);
                }
            }
        }

        m_UnderAttackTime -= cms;
        if(m_UnderAttackTime < 0) m_UnderAttackTime = 0;
        
        if(m_ShowHitpointTime > 0)
        {
            m_ShowHitpointTime -= cms;
            if(m_ShowHitpointTime < 0) m_ShowHitpointTime = 0;
        }

        if(m_Kind != BUILDING_BASE)
        {
            if(m_CaptureSeekRobotNextTime < g_MatrixMap->GetTime())
            {
                SFindRobotForCaptureAny data;
                data.found = nullptr;
                data.dist2 = CAPTURE_RADIUS*CAPTURE_RADIUS;

                //CHelper::Create(300, 0)->Line(D3DXVECTOR3(m_Pos.x, m_Pos.y, 0), D3DXVECTOR3(m_Pos.x, m_Pos.y, 1000));

                g_MatrixMap->FindObjects(m_Pos, CAPTURE_RADIUS, 1, TRACE_ROBOT, nullptr, FindRobotForCaptureAny, (dword)&data);

                if(data.found && m_Side != data.found->GetSide())
                {
                    Capture(data.found);

                    int nt = 100;
                    if(nt > g_Config.m_CaptureTimeErase) nt = g_Config.m_CaptureTimeErase;
                    if(nt > g_Config.m_CaptureTimePaint) nt = g_Config.m_CaptureTimePaint;

                    m_CaptureSeekRobotNextTime = g_MatrixMap->GetTime() + nt;
                }
                else
                {
                    m_CaptureSeekRobotNextTime = g_MatrixMap->GetTime() + CAPTURE_SEEK_ROBOT_PERIOD;
                }
            }
            if(m_InCaptureTime > 0)
            {
                m_InCaptureTime -= cms;
                if(m_InCaptureTime <= 0)
                {
                    m_InCaptureTime = 0;
                    m_CaptureNextTimeRollback = g_MatrixMap->GetTime();
                }
            }
            else
            {
                // rollback
                if(m_Side == 0 && m_TrueColor.m_ColoredCnt > 0)
                {
                    while(m_CaptureNextTimeRollback < g_MatrixMap->GetTime() && m_TrueColor.m_ColoredCnt > 0)
                    {
                        m_CaptureNextTimeRollback += g_Config.m_CaptureTimeRolback;
                        --m_TrueColor.m_ColoredCnt;
                    }
                }
                else if(m_Side != 0 && m_TrueColor.m_ColoredCnt < MAX_ZAHVAT_POINTS)
                {
                    while(m_CaptureNextTimeRollback < g_MatrixMap->GetTime() && m_TrueColor.m_ColoredCnt < MAX_ZAHVAT_POINTS)
                    {
                        m_CaptureNextTimeRollback += g_Config.m_CaptureTimeRolback;
                        ++m_TrueColor.m_ColoredCnt;
                    }
                }
            }
        }

        CMatrixSideUnit *su = nullptr;
        if(m_Side) su = g_MatrixMap->GetSideById(m_Side);

        //Если строение не нейтрально
        if(su)
        {
            m_BS.TickTimer(cms);
            m_ResourcePeriod += cms;

            switch(m_Kind)
            {
            case BUILDING_TITAN:
                if(m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_TITAN])
                {
                    //m_ResourceAmount += RESOURCES_INCOME;
                    SETFLAG(m_ObjectFlags, BUILDING_NEW_INCOME);
                    m_ResourcePeriod = 0;
                    su->AddResourceAmount(TITAN, RESOURCES_INCOME);
                }
                break;
            case BUILDING_PLASMA:
                if(m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_PLASMA])
                {
                    //m_ResourceAmount += RESOURCES_INCOME;
                    SETFLAG(m_ObjectFlags, BUILDING_NEW_INCOME);
                    m_ResourcePeriod = 0;
                    su->AddResourceAmount(PLASMA, RESOURCES_INCOME);
                }
                break;
            case BUILDING_ELECTRONIC:
                if(m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_ELECTRONICS])
                {
                    //m_ResourceAmount += RESOURCES_INCOME;
                    SETFLAG(m_ObjectFlags, BUILDING_NEW_INCOME);
                    m_ResourcePeriod = 0;
                    su->AddResourceAmount(ELECTRONICS, RESOURCES_INCOME);
                }
                break;
            case BUILDING_ENERGY:
                if(m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_ENERGY])
                {
                    //m_ResourceAmount += RESOURCES_INCOME;
                    SETFLAG(m_ObjectFlags, BUILDING_NEW_INCOME);
                    m_ResourcePeriod = 0;
                    su->AddResourceAmount(ENERGY, RESOURCES_INCOME);
                }
                break;
            case BUILDING_REPAIR:
                //Ремонтируем всех союзных роботов, находящихся в радиусе 5000 от центра точки захвата рембазы
                obj = CMatrixMapStatic::GetFirstLogic();
                while(obj)
                {
                    if(obj->IsRobotAlive() && obj->GetSide() == GetSide() && obj->NeedRepair())
                    {
                        CMatrixRobotAI *robot = (CMatrixRobotAI*)obj;
                        //D3DXVECTOR3 dist = this->m_Pos - D3DXVECTOR2(robot->m_PosX, robot->m_PosY);
                        D3DXVECTOR3 dist = (m_Pos + (*(D3DXVECTOR2*)&GetMatrix()._21) * 8.0f) - D3DXVECTOR2(robot->m_PosX, robot->m_PosY);
                        dist.z = m_BuildZ - robot->Z_From_Pos();
                        if(D3DXVec3LengthSq(&dist) <= 5000)
                        {
                            robot->ModifyHitpoints(4);
                        }
                    }
                    obj = obj->GetNextLogic();
                }
                break;
            case BUILDING_BASE:
                if(m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_BASE])
                {
                    //m_BaseRCycle++;
                    //if(m_BaseRCycle > 3) m_BaseRCycle = 0;
                    int fu = su->GetResourceForceUp();
                    int ra = RESOURCES_INCOME_BASE * fu / 100;
                    SETFLAG(m_ObjectFlags, BUILDING_NEW_INCOME);
                    m_ResourcePeriod = 0;

                    su->AddResourceAmount(TITAN, ra);
                    su->AddResourceAmount(ELECTRONICS, ra);
                    su->AddResourceAmount(ENERGY, ra);
                    su->AddResourceAmount(PLASMA, ra);
                        
                    //if(m_BaseRCycle == 0)
                    //{
                    //    su->AddTitan(RESOURCES_INCOME * fu / 100);
                    //}
                    //else if(m_BaseRCycle == 1)
                    //{
                    //    su->AddElectronics(RESOURCES_INCOME * fu / 100);
                    //}
                    //else if(m_BaseRCycle == 2)
                    //{
                    //    su->AddEnergy(RESOURCES_INCOME * fu / 100);
                    //}
                    //else if(m_BaseRCycle == 3)
                    //{
                    //    su->AddPlasma(RESOURCES_INCOME * fu / 100);
                    //}
                }
                break;
            }
        }
    }
    //if(m_Kind == BUILDING_BASE) CDText::T("hp", CStr((float)m_HitPoint));

    int downtime = -BUILDING_EXPLOSION_TIME;
    if(m_Kind == BUILDING_BASE)
    {
        downtime -= BUILDING_BASE_EXPLOSION_TIME;

        if((m_State == BUILDING_DIP) && (m_HitPoint < downtime + 100))
        {
            CSound::AddSound(S_EXPLOSION_BUILDING_BOOM4, GetGeoCenter());
            //DCNT("boom");
            CMatrixEffectWeapon *e = (CMatrixEffectWeapon*)CMatrixEffect::CreateWeapon(GetGeoCenter(), D3DXVECTOR3(0, 0, 1), 0, nullptr, WEAPON_BOMB);
            e->SetOwner(this);
            e->FireBegin(D3DXVECTOR3(0, 0, 0), this);
            e->Tact(1);
            e->FireEnd();
            e->Release();
            m_State = BUILDING_DIP_EXPLODED;
        }
    }

    if(m_HitPoint < downtime)
    {
        if(m_Kind != BUILDING_BASE) CSound::AddSound(S_EXPLOSION_BUILDING_BOOM3, GetGeoCenter());

        if(m_GGraph)
        {
            m_ShadowType = SHADOW_OFF;
	        RChange(MR_ShadowStencil|MR_ShadowProjGeom);
            GetResources(MR_ShadowStencil|MR_ShadowProjGeom);

            HDelete(CVectorObjectGroup, m_GGraph, g_MatrixHeap);
            m_GGraph = nullptr;

            // replace geometry
            int n = (int)m_Kind;

            CWStr namet(OBJECT_PATH_BUILDINGS_RUINS, g_MatrixHeap);
            namet += L"b"; namet += n;
            CWStr namev(namet, g_MatrixHeap);
            namev += L".vo";

            CMatrixMapObject* mo = g_MatrixMap->StaticAdd<CMatrixMapObject>(false);
            mo->InitAsBaseRuins(m_Pos, m_Angle, namev, namet, true);

            if(n != 0)
            {
                namev = namet + L"p.vo";
                namet += L"?Trans";
                CMatrixMapObject *momo = g_MatrixMap->StaticAdd<CMatrixMapObject>(false);
                momo->InitAsBaseRuins(m_Pos, m_Angle, namev, namet, false);
            }

            n = IRND(30) + 20;
            for(int i = 0; i < n; ++i)
            {
                CWStr type(L"1,1000,5000,",g_CacheHeap);
                type += (IRND(500) + 700);
                type += L",2400,60,false,0.03,78000000";

                D3DXVECTOR3 dir, pos;
                float t;

                int cnt = 30;
                do
                {
                    pos.x = mo->GetGeoCenter().x + FSRND(mo->GetRadius());
                    pos.y = mo->GetGeoCenter().y + FSRND(mo->GetRadius());
                    pos.z = mo->GetMatrix()._43 + FRND(mo->GetRadius() * 2);
                    D3DXVec3Normalize(&dir, &D3DXVECTOR3(mo->GetMatrix()._41 - pos.x, mo->GetMatrix()._42 - pos.y, mo->GetMatrix()._43 - pos.z));
                }
                while(!mo->PickFull(pos, dir, &t) && (--cnt > 0));

                if(cnt > 0)
                {
                    pos += dir * (t + 2);

                    g_MatrixMap->AddEffectSpawner(pos.x, pos.y, pos.z, Float2Int(FRND(15000) + 5000), type);
                }
            }

            g_MatrixMap->StaticDelete(this);
            return;
        }
        m_HitPoint = -10000000;
    }

    if(m_HitPoint < 0)
    {
        if(m_GGraph)
        {
            m_HitPoint -= cms;
            // explosions
            while(g_MatrixMap->GetTime() > m_NextExplosionTimeSound)
            {
                m_NextExplosionTimeSound += IRND(BUILDING_EXPLOSION_PERIOD_SND_2-BUILDING_EXPLOSION_PERIOD_SND_1) + BUILDING_EXPLOSION_PERIOD_SND_1;
                CSound::AddSound(S_EXPLOSION_BUILDING_BOOM, GetGeoCenter());
            }
            while(g_MatrixMap->GetTime() > m_NextExplosionTime)
            {
                m_NextExplosionTime += BUILDING_EXPLOSION_PERIOD;

                D3DXVECTOR3 dir, pos, pos0;
                float t;

                int cnt = 4;
                do
                {
                    pos0.x = m_Pos.x - m_Core->m_Matrix._21 * 60;
                    pos0.y = m_Pos.y - m_Core->m_Matrix._22 * 60;
                    pos0.z = m_Core->m_Matrix._43;

                    pos = pos0 + D3DXVECTOR3(FSRND(GetRadius()),FSRND(GetRadius()),FRND(2 * GetRadius()));

                    D3DXVec3Normalize(&dir, &(pos0-pos));
                }
                while(!Pick(pos, dir, &t) && (--cnt > 0));
                if(cnt > 0)
                {
                    //CHelper::Create(1, 0)->Line(pos, pos + dir * 10);

                    if(FRND(1) < 0.04f) CMatrixEffect::CreateExplosion(pos + dir * (t + 2), ExplosionBuildingBoom2);
                    else CMatrixEffect::CreateExplosion(pos + dir * (t + 2), ExplosionBuildingBoom);
                }
            }
        }

        return;
    }

	if(m_Kind == BUILDING_BASE)
    {
        float oldf = m_BaseFloor;
        if(m_State == BASE_OPENING)
        {
            m_BaseFloor += BASE_FLOOR_SPEED * cms;
            if(m_BaseFloor > 1.0f)
            {
                m_BaseFloor = 1.0f;
                m_State = BASE_OPENED;

                CSound::AddSound(S_PLATFORM_UP_STOP, GetGeoCenter());
            }
        }
        if(m_State == BASE_CLOSING)
        {
            m_BaseFloor -= BASE_FLOOR_SPEED * cms;
            if(m_BaseFloor < 0.0f)
            {
                m_BaseFloor = 0.0f;
                m_State = BASE_CLOSED;
                CSound::AddSound(S_DOORS_CLOSE_STOP, GetGeoCenter());
            }
        }

        if(m_BaseFloor != oldf)
        {
		    CVectorObjectGroupUnit* gu;

            gu = m_GGraph->GetById(1);
            SETFLAG(gu->m_Flags, VOUF_MATRIX_USE);
            D3DXMatrixTranslation(&(gu->m_Matrix), 0, 0, -(1.0f - m_BaseFloor) * 63.0f - 3.0f);

            gu = m_GGraph->GetById(2);
            SETFLAG(gu->m_Flags, VOUF_MATRIX_USE);
            D3DXMatrixTranslation(&(gu->m_Matrix), min(m_BaseFloor * 2, 1.0f) * 25.0f, 0, 0);

            gu = m_GGraph->GetById(3);
            SETFLAG(gu->m_Flags, VOUF_MATRIX_USE);
            D3DXMatrixTranslation(&(gu->m_Matrix), -min(m_BaseFloor * 2, 1.0f) * 25.0f, 0, 0);

            RChange(MR_Matrix);
        }
	}

    //Играются фоновые звуки для базы и всех видов заводов
    /*
    ESound snd = S_BASE_AMBIENT;
    if(m_Kind == BUILDING_TITAN) snd = S_TITAN_AMBIENT;
    else if(m_Kind == BUILDING_ELECTRONIC) snd = S_ELECTRONIC_AMBIENT;
    else if(m_Kind == BUILDING_ENERGY) snd = S_ENERGY_AMBIENT;
    else if(m_Kind == BUILDING_REPAIR) snd = S_REPAIR_AMBIENT;
    else if(m_Kind == BUILDING_PLASMA) snd = S_PLASMA_AMBIENT;

    float dist2 = D3DXVec3LengthSq(&(g_MatrixMap->m_Camera.GetFrustumCenter() - GetGeoCenter()));
    if(dist2 > CSound::GetSoundMaxDistSQ(snd))
    {
       CSound::StopPlay(m_AmbientSound);
       m_AmbientSound = SOUND_ID_EMPTY;
    }
    else m_AmbientSound = CSound::Play(m_AmbientSound, snd, GetGeoCenter());
    */
}

bool CMatrixBuilding::Pick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt) const
{
DTRACE();

    if(!m_GGraph) return false;
    return m_GGraph->Pick(orig, dir, outt) != nullptr;
}

void CMatrixBuilding::FreeDynamicResources(void)
{
DTRACE();

    if(m_ShadowProj) m_ShadowProj->DX_Free();
    if(m_ShadowType == SHADOW_STENCIL) m_GGraph->ShadowStencil_DX_Free();
}


void CMatrixBuilding::BeforeDraw(void)
{
DTRACE();

    dword sh = (g_Config.m_ShowProjShadows ? (MR_ShadowProjGeom | MR_ShadowProjTex) : 0) | (g_Config.m_ShowStencilShadows ? MR_ShadowStencil : 0);
    GetResources(MR_Matrix | MR_Graph | MR_MiniMap | sh);

    //static float pp = 0.3f;
    //if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_PGUP) {g_MatrixMap->m_KeyDown = false; pp += 0.01f;}
    //if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_PGDN) {g_MatrixMap->m_KeyDown = false; pp -= 0.01f;}
    //if (pp > 1.0f) pp = 1.0f;
    //if (pp < 0) pp = 0;

    if(m_ShowHitpointTime > 0 && m_HitPoint > 0)
    {
        D3DXVECTOR3 pos;
        float r = GetRadius();
        if(m_Kind == BUILDING_BASE)
        {
            pos = GetGeoCenter() - *(D3DXVECTOR3*)&m_Core->m_Matrix._21 * 20;
            r *= 0.7f;
            pos.z = g_MatrixMap->GetZ(pos.x, pos.y) + 90;
        }
        else
        {
            pos = GetGeoCenter();
            pos.z = g_MatrixMap->GetZ(pos.x, pos.y) + 80;
        }

        if(TRACE_STOP_NONE == g_MatrixMap->Trace(nullptr, g_MatrixMap->m_Camera.GetFrustumCenter(), pos, TRACE_LANDSCAPE, nullptr))
        {
            D3DXVECTOR2 p = g_MatrixMap->m_Camera.Project(pos, g_MatrixMap->GetIdentityMatrix());
            m_ProgressBar.Modify(p.x - r, p.y, m_HitPoint * m_MaxHitPointInversed);
        }
    }

    if(m_GGraph) m_GGraph->BeforeDraw(g_Config.m_ShowStencilShadows);
}


void CMatrixBuilding::Draw(void)
{
    DTRACE();

    if(!m_GGraph) return;

    for(int i = 0; i < 4; ++i)
    {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&g_MatrixMap->m_BiasBuildings))));
    }

    //ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_Matrix));
    //m_Graph->DrawSide(map.SideToColor(m_Side), flags);
    dword coltex = (dword)g_MatrixMap->GetSideColorTexture(m_Side)->Tex();
	g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Core->m_TerainColor);
    //m_Graph->Draw(flags);
	m_GGraph->Draw(coltex);

    if(!FLAG(g_MatrixMap->m_Flags,MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS))
    {
        m_GGraph->DrawLights();
    }
}

void CMatrixBuilding::DrawShadowStencil(void)
{
    DTRACE();
	if(m_ShadowType!=SHADOW_STENCIL) return;
	m_GGraph->ShadowStencilDraw();
}

void CMatrixBuilding::DrawShadowProj()
{
//DTRACE();

    /*
	if(m_ShadowType!=SHADOW_PROJ_DYNAMIC || m_ShadowType!=SHADOW_PROJ_STATIC) return;

    m_ShadowProj->Render();

    D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
    m._41 = m_ShadowProj->GetDX();
    m._42 = m_ShadowProj->GetDY();
	ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));
    */
}


float CMatrixBuilding::GetFloorZ(void)
{
    return m_BuildZ + (1.0f - m_BaseFloor) * BASE_FLOOR_Z - 3.0f + 2.7f;
}

void CMatrixBuilding::OnLoad(void)
{
DTRACE();

    m_ProgressBar.Modify(1000000, 0, PB_BUILDING_WIDTH, 1);

    /*
    int* tmp = m_Places;
    
    int b_x = (int)(m_Pos.x / GLOBAL_SCALE_MOVE);
    int b_y = (int)(m_Pos.y / GLOBAL_SCALE_MOVE);

    *tmp = b_x - 10;
    *(++tmp) = b_y - 10;

    *(++tmp) = b_x + 10;
    *(++tmp) = b_y + 10;

    *(++tmp) = b_x - 10;
    *(++tmp) = b_y + 10;

    *(++tmp) = b_x + 10;
    *(++tmp) = b_y - 10;
    */

    m_BuildZ = g_MatrixMap->GetZ(m_Pos.x, m_Pos.y);
    if(m_BuildZ < WATER_LEVEL)
    {
        D3DXVECTOR3 pos(0, 0, WATER_LEVEL);
        g_MatrixMap->Trace(&pos, D3DXVECTOR3(m_Pos.x,m_Pos.y, 1000), D3DXVECTOR3(m_Pos.x,m_Pos.y, -1000), TRACE_ALL, this);
        m_BuildZ = pos.z;
    }

    switch(m_Kind)
    {
        case BUILDING_BASE:
        {
            m_TurretsMax = BASE_TURRETS;
            m_Name = L"BASE";
            break;
        }
        case BUILDING_TITAN:
        {
            m_TurretsMax = TITAN_TURRETS;
            m_Name = L"TITAN";
            break;
        }
        case BUILDING_PLASMA:
        {
            m_TurretsMax = PLASMA_TURRETS;
            m_Name = L"PLASMA";
            break;
        }
        case BUILDING_ELECTRONIC:
        {
            m_TurretsMax = ELECTRONIC_TURRETS;
            m_Name = L"ELECTRONICS";
            break;
        }
        case BUILDING_ENERGY:
        {
            m_TurretsMax = ENERGY_TURRETS;
            m_Name = L"ENERGY";
            break;
        }
        case BUILDING_REPAIR:
        {
            m_TurretsMax = REPAIR_TURRETS;
            m_Name = L"REPAIR";
            break;
        }
    }

    if(m_Kind == BUILDING_BASE)
    {
        int px = Float2Int((m_Pos.x - (GLOBAL_SCALE / 2)) * INVERT(GLOBAL_SCALE)) - 1;
        int py = Float2Int((m_Pos.y - (GLOBAL_SCALE / 2)) * INVERT(GLOBAL_SCALE)) - 1;

        for(int x = 0; x < 3; ++x)
        {
            for(int y = 0; y < 3; ++y)
            {
                SMatrixMapUnit* mu = g_MatrixMap->UnitGet(x + px, y + py);
                mu->m_MapUnitBase = this;
            }
        }
    }

    m_ShowHitpointTime = 0;
    m_defHitPoint = Float2Int(m_HitPoint);
}

bool CMatrixBuilding::CalcBounds(D3DXVECTOR3& omin, D3DXVECTOR3& omax)
{
DTRACE();
    
    if(!(m_GGraph) || !(m_GGraph->m_First)) return true;

	m_GGraph->BoundGetAllFrame(omin,omax);

	return false;

    /*
    SVOFrame *f = m_GGraph->m_First->m_Obj->VO()->FrameGet(0);
    min.x = f->m_MinX;
    min.y = f->m_MinY;
    max.x = f->m_MaxX;
    max.y = f->m_MaxY;

    int cnt = m_GGraph->m_First->m_Obj->VO()->Header()->m_FrameCnt;
    for(int i = 1; i < cnt; ++i)
    {
        SVOFrame *f = m_GGraph->m_First->m_Obj->VO()->FrameGet(i);

        min.x = min(min.x,f->m_MinX);
        min.y = min(min.y,f->m_MinY);
        max.x = max(max.x,f->m_MaxX);
        max.y = max(max.y,f->m_MaxY);
    }

    return false;
    */
}

struct SFindRobotForCapture
{
    CMatrixRobotAI* by;
    CMatrixRobotAI* found;
    float           dist2;
};


static bool FindRobotForCapture(const D3DXVECTOR2& center, CMatrixMapStatic* ms, dword user)
{
    SFindRobotForCapture* data = (SFindRobotForCapture*)user;

    float dist2 = D3DXVec2LengthSq(&(center - *(D3DXVECTOR2*)&ms->GetGeoCenter()));
    if(dist2 < data->dist2)
    {
        if(data->by == data->found)
        {
            data->by = nullptr;
            return false;
        }

        data->found = (CMatrixRobotAI*)ms;
        data->dist2 = dist2;
    }

    return true;
}

//Делает здание нейтральным
void CMatrixBuilding::SetNeutral(void)
{
    m_TrueColor.m_ColoredCnt = 0;
    m_TrueColor.m_Color = 0;
    m_Side = 0;
    m_BS.ClearStack();
    RChange(MR_MiniMap);

    if(GatheringPointIsSet()) ClearGatheringPoint();
}

ECaptureStatus CMatrixBuilding::Capture(CMatrixRobotAI* by)
{
    if(m_InCaptureTime <= 0)
    {
        m_InCaptureTime = g_Config.m_CaptureTimeErase + g_Config.m_CaptureTimePaint;
        m_InCaptureNextTimeErase = g_MatrixMap->GetTime();
        m_InCaptureNextTimePaint = g_MatrixMap->GetTime();
    }

    if(m_InCaptureNextTimeErase >= g_MatrixMap->GetTime() || m_InCaptureNextTimePaint >= g_MatrixMap->GetTime()) return CAPTURE_BUSY;

    SFindRobotForCapture data;
    data.by = by;
    data.found = nullptr;
    data.dist2 = CAPTURE_RADIUS * CAPTURE_RADIUS;
    
    g_MatrixMap->FindObjects(m_Pos, CAPTURE_RADIUS, 1, TRACE_ROBOT, nullptr, FindRobotForCapture, (dword)&data);
 
    if(data.by == nullptr || data.by != data.found) return CAPTURE_TOO_FAR;

    dword captureer_color = 0xFF000000 | g_MatrixMap->GetSideColor(by->GetSide());
    
    m_InCaptureTime = g_Config.m_CaptureTimeErase + g_Config.m_CaptureTimePaint;

    if(!m_Side)
    {
        if(!m_TrueColor.m_Color || m_TrueColor.m_Color == captureer_color)
        {
            m_TrueColor.m_Color = captureer_color;
            while(m_InCaptureNextTimePaint < g_MatrixMap->GetTime())
            {
                m_InCaptureNextTimePaint += g_Config.m_CaptureTimePaint;
                m_InCaptureNextTimeErase = g_MatrixMap->GetTime();

                if(m_TrueColor.m_ColoredCnt == MAX_ZAHVAT_POINTS)
                {
                    int side = by->GetSide();
                    if(side == PLAYER_SIDE) CSound::Play(S_ENEMY_FACTORY_CAPTURED);
                    m_Side = side;
                    m_BS.ClearStack();
                    RChange(MR_MiniMap);
                    return CAPTURE_DONE;
                }
                ++m_TrueColor.m_ColoredCnt;
            }
        }
        else
        {
            while(m_InCaptureNextTimeErase < g_MatrixMap->GetTime())
            {
                m_InCaptureNextTimeErase += g_Config.m_CaptureTimeErase;
                m_InCaptureNextTimePaint = g_MatrixMap->GetTime();

                if(m_TrueColor.m_ColoredCnt == 0)
                {
                    m_TrueColor.m_Color = 0;
                    return CAPTURE_IN_PROGRESS;
                }
                --m_TrueColor.m_ColoredCnt;
            }
        }
    }
    else
    {
        if(m_TrueColor.m_Color == captureer_color)
        {
            while(m_InCaptureNextTimePaint < g_MatrixMap->GetTime())
            {
                m_InCaptureNextTimePaint += g_Config.m_CaptureTimePaint;
                m_InCaptureNextTimeErase = g_MatrixMap->GetTime();

                if(m_TrueColor.m_ColoredCnt == MAX_ZAHVAT_POINTS)
                {
                    //дозахват
                    int side = by->GetSide();
                    //if(side == PLAYER_SIDE) CSound::Play(S_ENEMY_FACTORY_CAPTURED);
                    m_Side = side;
                    RChange(MR_MiniMap);
                    
                    return CAPTURE_DONE;
                }
                ++m_TrueColor.m_ColoredCnt;
            }
        }
        else
        {
            while(m_InCaptureNextTimeErase < g_MatrixMap->GetTime())
            {
                m_InCaptureNextTimeErase += g_Config.m_CaptureTimeErase;
                m_InCaptureNextTimePaint = g_MatrixMap->GetTime() + g_Config.m_CaptureTimePaint;

                if(m_TrueColor.m_ColoredCnt == 0)
                {
                    if(m_Side == PLAYER_SIDE) CSound::Play(S_PLAYER_FACTORY_CAPTURED);

                    //Да, я в курсе, что это завод и точки сбора там быть вообще никак не должно, но мало ли
                    if(GatheringPointIsSet()) ClearGatheringPoint();

                    m_TrueColor.m_Color = 0;
                    m_Side = 0;
                    RChange(MR_MiniMap);
                    m_BS.ClearStack();
                    return CAPTURE_IN_PROGRESS;
                }
                --m_TrueColor.m_ColoredCnt;
            }
        }
    }

    return CAPTURE_IN_PROGRESS;
}

//Функция вызова подкрепления к указанной базе
void CMatrixBuilding::Reinforcements(void)
{
    if(!m_Side) return;
    //Вызов подкрепления заблокирован на данной карте
    if(g_MatrixMap->ReinforcementsDisabled()) return;
    //КД для вызова подкрепления ещё не прошёл
    if(g_MatrixMap->BeforeReinforcementsTime() > 0) return;

    int cx = Float2Int(m_Pos.x * INVERT(GLOBAL_SCALE_MOVE));// - ROBOT_MOVECELLS_PER_SIZE / 2;
    int cy = Float2Int(m_Pos.y * INVERT(GLOBAL_SCALE_MOVE));// - ROBOT_MOVECELLS_PER_SIZE / 2;

    if(!m_Kind)
    {
        switch(m_Angle)
        {
            case 0: cy += 5 + 1; break;
            case 1: cx -= 6 + 1; break;
            case 2: cy -= 6 + 1; break;
            case 3: cx += 5 + 1; break;
        }
    }

    //m_Pos.x * GLOBAL_SACLE_MOVE + ROBOT_MOVECELLS_PER_SIZE * GLOBAL_SCALE_MOVE / 2

    //Выбирается точка сброса подкрепления возле здания
    g_MatrixMap->PlaceFindNear(0, 4, cx, cy, 0, nullptr, nullptr);

    CMatrixSideUnit* su = g_MatrixMap->GetSideById(m_Side);

    int list_count;
    su->BufPrepare();
    int ret = g_MatrixMap->PlaceList(1+2+4+8+16, CPoint(cx, cy), CPoint(cx, cy), 100, false, su->m_PlaceList, &list_count);
    if(!ret) return;
    g_MatrixMap->InitReinforcementsTime();
    CSound::Play(S_REINFORCEMENTS_CALLED);
    //g_MatrixMap->PlaceListGrow(1+2+4+8+16, g_MatrixMap->GetPlayerSide()->m_PlaceList, &listcnt, 2);

    for(int i = 0; i < list_count; ++i)
    {
        g_MatrixMap->m_RoadNetwork.m_Place[su->m_PlaceList[i]].m_Data = 0;
    }

    CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
    while(obj)
    {
        if(obj->IsCannonAlive())
        {
            g_MatrixMap->m_RoadNetwork.m_Place[obj->AsCannon()->m_Place].m_Data = 1;
        }
        else if(obj->IsRobotAlive() && obj->AsRobot()->GetEnv()->m_Place >= 0)
        {
            g_MatrixMap->m_RoadNetwork.m_Place[obj->AsRobot()->GetEnv()->m_Place].m_Data = 1;
        }

        obj = obj->GetNextLogic();
    }

    float angle = FSRND(M_PI);

    CBlockPar* bp = g_MatrixData->BlockPathGet(BLOCK_PATH_REINFORCEMENTS);
    int max_scores = bp->ParGet(L"ReinforcementsPoints").GetInt();
    int cur_scores = 0;

    int pli = 0;
    while(cur_scores < max_scores)
    {
        if(pli >= list_count) break;

        int place = su->m_PlaceList[pli++];
        if(g_MatrixMap->m_RoadNetwork.m_Place[place].m_Data) continue;
        if(g_MatrixMap->m_RoadNetwork.m_Place[place].m_Move & (1+2+4+8+16)) continue;
        const CPoint& pos = g_MatrixMap->m_RoadNetwork.m_Place[place].m_Pos;

        int template_num;

        int cnt = 10;
        int template_scores = 0;
        for(; cnt > 0; --cnt)
        {
            template_num = g_MatrixMap->Rnd(0, SRobotTemplate::m_AIRobotTypeList[REINFORCEMENTS_TEMPLATES].size() - 1); //Выбираем случайный номер шаблона робота из списка
            template_scores = SRobotTemplate::m_AIRobotTypeList[REINFORCEMENTS_TEMPLATES][template_num].m_TemplatePriority; //В случае вызова подкреплений, за ограничитель их суммарной силы отвечает приоритет шаблона
            if(cur_scores + template_scores > max_scores + 30) continue;
            break;
        }
        cur_scores += template_scores;

        CPoint bpos(Float2Int((m_Pos.x + m_Core->m_Matrix._21 * 50) * INVERT(GLOBAL_SCALE_MOVE)) - ROBOT_MOVECELLS_PER_SIZE / 2,
            Float2Int((m_Pos.y + m_Core->m_Matrix._22 * 50) * INVERT(GLOBAL_SCALE_MOVE)) - ROBOT_MOVECELLS_PER_SIZE / 2);
        su->SpawnDeliveryFlyer(D3DXVECTOR2(pos.x * GLOBAL_SCALE_MOVE + ROBOT_MOVECELLS_PER_SIZE * GLOBAL_SCALE_MOVE / 2, pos.y * GLOBAL_SCALE_MOVE + ROBOT_MOVECELLS_PER_SIZE * GLOBAL_SCALE_MOVE / 2),
            FO_GIVE_BOT, angle, place, bpos, template_num, template_scores < 75 ? (EFlyerKind)bp->ParGet(L"FlyersType").GetInt() : (EFlyerKind)bp->ParGet(L"FlyersTypeBest").GetInt(), max(10.0f * bp->ParGet(L"FlyersStructure").GetFloat(), 10.0f));

        //CHelper::Create(100, 0)->Cone(D3DXVECTOR3(pos.x * GLOBAL_SCALE_MOVE, pos.y * GLOBAL_SCALE_MOVE, 0),
        //    D3DXVECTOR3(pos.x * GLOBAL_SCALE_MOVE, pos.y * GLOBAL_SCALE_MOVE, 100), 10, 10, 0xFFFFFFFF, 0, 10);
    }
}

bool CMatrixBuilding::BuildFlyer(EFlyerKind kind)
{
    CMatrixFlyer* fl = g_MatrixMap->StaticAdd<CMatrixFlyer>();

    SetSpawningUnit(true);
    fl->m_FlyerKind = kind;
    fl->SetDeliveryCopter(false);

    //Готовый вертолёт выезжает с базы
    fl->Begin(this);
    return true;
}

//Здание было уничтожено, удаляем его из игровой логики и очищаем все маркеры
void CMatrixBuilding::ReleaseMe(void)
{
DTRACE();

    DeletePlacesShow();

    if(GatheringPointIsSet()) ClearGatheringPoint();

    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    if(GetSide() == PLAYER_SIDE)
    {
        if(ps->m_ActiveObject == this)
        {
            ps->PLDropAllActions();
        }
        ps->RemoveFromSelection(this);
    }

    //for(int c = 1; c <= g_MatrixMap->m_SidesCount; ++c)
    //{
    //    s = g_MatrixMap->GetSideById(c);
    //    if(s->m_Id == PLAYER_SIDE && s->m_ActiveObject == this)
    //    {
    //        s->Select(NOTHING, nullptr);
    //    }
    //    if(s != g_MatrixMap->GetPlayerSide() && s->m_GroupsList)
    //    {
    //        CMatrixGroup* grps = s->m_GroupsList->m_FirstGroup;
    //        while(grps)
    //        {
    //            if(grps->m_Tactics && grps->m_Tactics->GetTarget() == this)
    //            {
    //                grps->DeInstallTactics();
    //            }
    //            grps = grps->m_NextGroup;
    //        }
    //    }
    //    else
    //    {
    //        if(s->m_CurGroup)
    //        {
    //            CMatrixTactics* t = s->m_CurGroup->GetTactics();
    //            if(t && t->GetTarget() == this)
    //            {
    //                s->m_CurGroup->DeInstallTactics();
    //            }
    //        }
    //    }
    //}
    
    //Уничтожаем всех ещё не покинувших базу роботов, а все связанные турели переводим на нейтральную сторону
    CMatrixMapStatic* objects = CMatrixMapStatic::GetFirstLogic();   
    while(objects)
    {
        if(objects->IsRobotAlive())
        {
            if(objects->AsRobot()->GetBase() == this)
            {
                objects->AsRobot()->SetBase(nullptr);
                objects->AsRobot()->MustDie();
            }

            objects->AsRobot()->RemoveCaptureCandidate(this);
            objects->AsRobot()->GetEnv()->RemoveFromList(this);
            if(objects->AsRobot()->GetCaptureBuilding() == this)
            {
                objects->AsRobot()->StopCapture();
            }
        }
        else if(objects->IsCannon() && objects->AsCannon()->m_ParentBuilding == this)
        {
            objects->AsCannon()->m_ParentBuilding = nullptr;
        }
        objects = objects->GetNextLogic();
    }
}


bool CMatrixBuilding::InRect(const CRect& rect) const
{
    D3DXVECTOR3 dir;
    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.top), dir);
    if(Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, nullptr)) return true;

    D3DXMATRIX s;
    SEVH_data d;

    d.m = g_MatrixMap->m_Camera.GetViewMatrix() * g_MatrixMap->m_Camera.GetProjMatrix();
    D3DXMatrixScaling(&s, float(g_ScreenX / 2), float(-g_ScreenY / 2), 1);
    s._41 = s._11;
    s._42 = float(g_ScreenY/2);
    d.m *= s;
    d.found = false;
    d.rect = &rect;

    m_GGraph->EnumFrameVerts_transform_position(EnumVertsHandler, (dword)&d);
    if(d.found) return true;

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.bottom), dir);
    if(Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, nullptr)) return true;

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.right, rect.top), dir);
    if(Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, nullptr)) return true;
    
    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.right, rect.bottom), dir);
    if(Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, nullptr)) return true;

    return false;
}

bool CMatrixBuilding::Select(void)
{
    D3DXVECTOR3 pos;

    pos.x = m_Pos.x - m_Core->m_Matrix._21 * 60;
    pos.y = m_Pos.y - m_Core->m_Matrix._22 * 60;
    pos.z = m_Core->m_Matrix._43 + 5;

    float r = BUILDING_SELECTION_SIZE;

    if(m_Kind == BUILDING_BASE)
    {
        r = BUILDING_SELECTION_SIZE + 24;
        pos.x -= m_Core->m_Matrix._11 * 7;
        pos.y -= m_Core->m_Matrix._12 * 7;

        pos.x += m_Core->m_Matrix._21 * 16;
        pos.y += m_Core->m_Matrix._22 * 16;
    }
    if(m_Kind == BUILDING_ENERGY)
    {
        r = BUILDING_SELECTION_SIZE + 10;
        pos.x -= m_Core->m_Matrix._21 * 13;
        pos.y -= m_Core->m_Matrix._22 * 13;
    }
    if(m_Kind == BUILDING_PLASMA)
    {
        r = BUILDING_SELECTION_SIZE + 15;
        pos.x -= m_Core->m_Matrix._21 * 17;
        pos.y -= m_Core->m_Matrix._22 * 17;
    }
    if(m_Kind == BUILDING_TITAN)
    {
        r = BUILDING_SELECTION_SIZE + 15;
        pos.x -= m_Core->m_Matrix._21 * 17;
        pos.y -= m_Core->m_Matrix._22 * 17;
    }
    if(m_Kind == BUILDING_ELECTRONIC)
    {
        r = BUILDING_SELECTION_SIZE + 17;
        pos.x -= m_Core->m_Matrix._21 * 17;
        pos.y -= m_Core->m_Matrix._22 * 17;
    }

	m_Selection = (CMatrixEffectSelection*)CMatrixEffect::CreateSelection(pos, r);

    if(!g_MatrixMap->AddEffect(m_Selection))
    {
        m_Selection = nullptr;
        return false;
    }
    return true;
}

void CMatrixBuilding::UnSelect(void)
{
    if(m_Selection)
    {
        m_Selection->Kill();
        m_Selection = nullptr;
    }
}

void CMatrixBuilding::CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type)
{
    m_ProgressBar.CreateClone(clone_type, x, y, width);
}

void CMatrixBuilding::DeleteProgressBarClone(EPBCoord clone_type)
{
    m_ProgressBar.KillClone(clone_type);
}

int CMatrixBuilding::GetPlacesForTurrets(CPoint* places)
{
    int cx = Float2Int(m_Pos.x / GLOBAL_SCALE_MOVE);
    int cy = Float2Int(m_Pos.y / GLOBAL_SCALE_MOVE);

    int cnt = 0;
    //int dist[MAX_PLACES];

    SMatrixPlace* place = g_MatrixMap->m_RoadNetwork.m_Place;
    for(int i = 0; i < g_MatrixMap->m_RoadNetwork.m_PlaceCnt; ++i, ++place) place->m_Data = 0;

    CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
    while(obj)
    {
        if(obj->IsCannonAlive())
        {
            if(obj->AsCannon()->m_Place >= 0)
            {
                place = g_MatrixMap->m_RoadNetwork.GetPlace(obj->AsCannon()->m_Place);
                place->m_Data = 1;
            }
        }
        if(obj->IsBuildingAlive())
        {
            CMatrixMapStatic* bi = obj->AsBuilding()->m_BS.GetTopItem();
            while(bi)
            {
                if(bi->IsCannon())
                {
                    place = g_MatrixMap->m_RoadNetwork.GetPlace(bi->AsCannon()->m_Place);
                    place->m_Data = 1;
                }
                bi = bi->m_NextQueueItem;
            }
        }
        obj = obj->GetNextLogic();
    }

    for (int i = 0; i < m_TurretsPlacesCnt; ++i)
    {
        place = g_MatrixMap->m_RoadNetwork.GetPlace(g_MatrixMap->m_RoadNetwork.FindInPL(m_TurretsPlaces[i].m_Coord));
        if(place->m_Data) continue;

        places[cnt] = m_TurretsPlaces[i].m_Coord;
        ++cnt;
    }

    /*
    CRect plr = g_MatrixMap->m_RoadNetwork.CorrectRectPL(CRect(cx - 16 * 2, cy - 16 * 2, cx + 16 * 2, cy + 16 * 2));
    SMatrixPlaceList* plist = g_MatrixMap->m_RoadNetwork.m_PLList + plr.left + plr.top * g_MatrixMap->m_RoadNetwork.m_PLSizeX;
    for(int y = plr.top; y < plr.bottom; ++y, plist += g_MatrixMap->m_RoadNetwork.m_PLSizeX - (plr.right - plr.left))
    {
        for(int x = plr.left; x < plr.right; ++x, ++plist)
        {
            place = g_MatrixMap->m_RoadNetwork.m_Place + plist->m_Sme;
            for(int u = 0; u < plist->m_Cnt; ++u, ++place)
            {
                if(place->m_Data) continue;

                for(i = 0; i < m_TurretsPlaceCnt; ++i)
                {
                    if(m_TurretsPlace[i] == place->m_Pos) break;
                }
                if(i >= m_TurretsPlaceCnt) continue;

                int cd = POW2(place->m_Pos.x - cx) + POW2(place->m_Pos.y - cy);
                if(cd > POW2(40)) continue;

                for(i = 0; i < cnt; ++i) if(cd < dist[i]) break;
                
                if(i >= MAX_PLACES) continue;
                else if(i >= cnt)
                {
                    cnt++;
                }
                else if(cnt < MAX_PLACES)
                {
                    CopyMemory(dist + i + 1, dist + i, (cnt - i) * sizeof(int));
                    CopyMemory(places + i + 1, places + i, (cnt - i) * sizeof(CPoint));
                    ++cnt;
                }
                else
                {
                    CopyMemory(dist + i + 1, dist + i, (cnt - i - 1) * sizeof(int));
                    CopyMemory(places + i + 1, places + i, (cnt - i - 1) * sizeof(CPoint));
                }

                dist[i] = cd;
                places[i] = place->m_Pos;
                places[i].x += ROBOT_MOVECELLS_PER_SIZE / 2;
                places[i].y += ROBOT_MOVECELLS_PER_SIZE / 2;
            }
        }
    }
    */

    return cnt;
}

void CMatrixBuilding::CreatePlacesShow(void)
{
    DeletePlacesShow();

    CPoint pl[MAX_PLACES];
    int cnt = GetPlacesForTurrets(pl);

    m_PlacesShow = (SEffectHandler*)HAlloc(sizeof(SEffectHandler) * MAX_PLACES, g_MatrixHeap);
    for(int i = 0; i < MAX_PLACES; ++i)
    {
#ifdef _DEBUG
        m_PlacesShow[i].SEffectHandler::SEffectHandler(DEBUG_CALL_INFO);
#else
        m_PlacesShow[i].SEffectHandler::SEffectHandler();
#endif
    }

    for(int i = 0; i < cnt; ++i)
    {
        float x = (float)pl[i].x;
        float y = (float)pl[i].y;
        CMatrixEffect::CreateLandscapeSpot(m_PlacesShow + i, D3DXVECTOR2(x * GLOBAL_SCALE_MOVE, y * GLOBAL_SCALE_MOVE), 0, 6, SPOT_TURRET);
    }
}

void CMatrixBuilding::DeletePlacesShow()
{
    if(IsBuildingAlive() && m_PlacesShow)
    {
        for(int i = 0; i < MAX_PLACES; ++i)
        {
#ifdef _DEBUG
            m_PlacesShow[i].Release(DEBUG_CALL_INFO);
#else
            m_PlacesShow[i].Release();
#endif
        }
        HFree(m_PlacesShow, g_MatrixHeap);
        m_PlacesShow = nullptr;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CBuildQueue::TickTimer(int ms)
{
    if(!m_Items /*|| (m_ParentBase->IsBase() && m_ParentBase->m_State != BASE_CLOSED)*/)
    {
        m_Timer = 0;
        return;
    }

    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    m_Timer += ms;

    //На идёт постройка робота
    if(m_Top->IsRobot())
    {
        float x = g_IFaceList->GetMainX() + 283;
        float y = g_IFaceList->GetMainY() + 71;

        m_ProgressBar.Modify(100000.0f, 0);
        if(m_Timer <= g_Config.m_Timings[UNIT_ROBOT])
        {
            m_ProgressBar.Modify(float(m_Timer) / float(g_Config.m_Timings[UNIT_ROBOT]));
        }

        if(ps->m_CurrSel == BASE_SELECTED && ps->m_ActiveObject == m_ParentBase)
        {
            m_ProgressBar.CreateClone(PBC_CLONE1, x, y, 87);
        }
        else
        {
            m_ProgressBar.KillClone(PBC_CLONE1);
        }

        if(m_Timer >= g_Config.m_Timings[UNIT_ROBOT] && m_ParentBase->m_State == BASE_CLOSED)
        {
            if(m_Top->GetSide() == PLAYER_SIDE && !FLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE))
            {
                if(g_MatrixMap->Rnd(0,1))
                {
                    CSound::Play(S_ROBOT_BUILD_END, SL_ALL);
                }
                else
                {
                    CSound::Play(S_ROBOT_BUILD_END_ALT, SL_ALL);
                }
            }
            
            m_Timer = 0;
            m_ProgressBar.KillClone(PBC_CLONE1);

            //produce robot, del from queue
            //STUB:
            if(m_ParentBase->GetSide() == PLAYER_SIDE)
            {
                g_IFaceList->DeleteQueueIcon(1, m_ParentBase);
            }
            //m_ParentBase->m_BusyFlag.SetBusy((CMatrixRobotAI*)m_Top); // do not busy flag while building robot

            m_Top->AsRobot()->RobotSpawn(((CMatrixRobotAI*)m_Top)->GetBase());
            m_Top->AsRobot()->JoinToGroup();

            g_MatrixMap->AddObject(m_Top, true);

            LIST_DEL(m_Top, m_Top, m_Bottom, m_PrevQueueItem, m_NextQueueItem);
            --m_Items;
        }
    }
    //На идёт постройка вертолёта
    else if(m_Top->GetObjectType() == OBJECT_TYPE_FLYER)
    {
        float x = g_IFaceList->GetMainX() + 283;
        float y = g_IFaceList->GetMainY() + 71;

        m_ProgressBar.Modify(100000.0f, 0);
        if(m_Timer <= g_Config.m_Timings[UNIT_FLYER])
        {
            m_ProgressBar.Modify(m_Timer * 1.0f / g_Config.m_Timings[UNIT_FLYER]);    
        }

        if(ps->m_CurrSel == BASE_SELECTED && ps->m_ActiveObject == m_ParentBase)
        {
            m_ProgressBar.CreateClone(PBC_CLONE1, x, y, 87);
        }
        else
        {
            m_ProgressBar.KillClone(PBC_CLONE1);
        }

        if(m_Timer >= g_Config.m_Timings[UNIT_FLYER] && m_ParentBase->m_State == BASE_CLOSED)
        {
            if(m_Top->GetSide() == PLAYER_SIDE)
            {
                if(g_MatrixMap->Rnd(0, 1))
                {
                    CSound::Play(S_FLYER_BUILD_END, SL_ALL);
                }
                else
                {
                    CSound::Play(S_FLYER_BUILD_END_ALT, SL_ALL);
                }
            }
            
            m_Timer = 0;
            m_ProgressBar.KillClone(PBC_CLONE1);
            //produce flyer, del from queue
            //STUB:
            if(m_ParentBase->GetSide() == PLAYER_SIDE)
            {
                g_IFaceList->DeleteQueueIcon(1, m_ParentBase);
            }

            g_MatrixMap->AddObject(m_Top, true);

            m_Top->AddLT();
            ((CMatrixFlyer*)m_Top)->Begin(m_ParentBase);

            LIST_DEL(m_Top, m_Top, m_Bottom, m_PrevQueueItem, m_NextQueueItem);
            --m_Items;
        }
    }
    //На идёт постройка турели
    else if(m_Top->IsCannon())
    {
        float x = g_IFaceList->GetMainX() + 283;
        float y = g_IFaceList->GetMainY() + 71;
        float percent_done = float(m_Timer) / float(g_Config.m_Timings[UNIT_TURRET]);
        m_ProgressBar.Modify(100000.0f, 0);
        m_ProgressBar.Modify(percent_done);

        //((CMatrixCannon*)m_Top)->SetPBOutOfScreen();
        m_Top->AsCannon()->SetHitPoint(m_Top->AsCannon()->GetMaxHitPoint() * percent_done);
        
        if((ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED) && ps->m_ActiveObject == m_ParentBase)
        {
            m_ProgressBar.CreateClone(PBC_CLONE1, x, y, 87);
        }
        else
        {
            m_ProgressBar.KillClone(PBC_CLONE1);
        }

        if(m_Timer >= g_Config.m_Timings[UNIT_TURRET])
        {
            if(m_Top->GetSide() == PLAYER_SIDE)
            {
                if(m_Top->AsCannon()->m_TurretKind == TURRET_LIGHT_CANNON)
                {
                    CSound::Play(S_TURRET_BUILD_0, SL_ALL);
                }
                else if(m_Top->AsCannon()->m_TurretKind == TURRET_HEAVY_CANNON)
                {
                    CSound::Play(S_TURRET_BUILD_1, SL_ALL);
                }
                else if(m_Top->AsCannon()->m_TurretKind == TURRET_LASER_CANNON)
                {
                    CSound::Play(S_TURRET_BUILD_2, SL_ALL);
                }
                else if(m_Top->AsCannon()->m_TurretKind == TURRET_MISSILE_CANNON)
                {
                    CSound::Play(S_TURRET_BUILD_3, SL_ALL);
                }
            }
            
            m_Timer = 0;
            m_ProgressBar.KillClone(PBC_CLONE1);

            for(int i = 0; i < MAX_PLACES; ++i)
            {
                if(m_ParentBase->m_TurretsPlaces[i].m_Coord.x == Float2Int(m_Top->AsCannon()->m_Pos.x / GLOBAL_SCALE_MOVE) && m_ParentBase->m_TurretsPlaces[i].m_Coord.y == Float2Int(m_Top->AsCannon()->m_Pos.y / GLOBAL_SCALE_MOVE))
                {
                    m_ParentBase->m_TurretsPlaces[i].m_CannonType = m_Top->AsCannon()->m_TurretKind;
                }
            }
            
            //STUB:
            if(m_ParentBase->GetSide() == PLAYER_SIDE)
            {
                g_IFaceList->DeleteQueueIcon(1, m_ParentBase);
                if(g_MatrixMap->GetPlayerSide()->m_ActiveObject == m_ParentBase)
                {
                    g_IFaceList->CreateDynamicTurrets(m_ParentBase);
                }
            }

            g_MatrixMap->m_Minimap.AddEvent(m_Top->GetGeoCenter().x, m_Top->GetGeoCenter().y, 0xffffff00, 0xffffff00);
            m_Top->AsCannon()->m_CurrState = CANNON_IDLE;
            int ss = m_Top->GetSide();
            if(ss != 0) g_MatrixMap->GetSideById(ss)->IncStatValue(STAT_TURRET_BUILD);

            m_Top->ResetInvulnerability();
            LIST_DEL(m_Top, m_Top, m_Bottom, m_PrevQueueItem, m_NextQueueItem);

            --m_Items;
        }
    }

    /*
    if(m_Top)
    {
        CMatrixMapStatic* o = m_Top->m_NextQueueItem;
        while(o)
        {
            if(o->GetObjectType() == OBJECT_TYPE_CANNON)
            {
                ((CMatrixCannon*)o)->SetTerainColor(0xFF00FF00);
            }
            o = o->m_NextQueueItem;
        }
    }
    */
}

void CBuildQueue::AddItem(CMatrixMapStatic* item)
{ 
    if(item && m_Items < BUILDING_QUEUE_LIMIT)
    {
        LIST_ADD(item, m_Top, m_Bottom, m_PrevQueueItem, m_NextQueueItem); 
        ++m_Items;
        //STUB:
        if(item->GetSide() == PLAYER_SIDE)
        {
            g_IFaceList->CreateQueueIcon(m_Items, m_ParentBase, item);
        }
    } 
}

int CBuildQueue::DeleteItem(int no)
{
    if(m_Items)
    {
        if(no == 1)
        {
            m_Timer = 0;
            m_ProgressBar.KillClone(PBC_CLONE1);
        }
        //STUB:
        if(m_ParentBase->GetSide() == PLAYER_SIDE)
        {
            g_IFaceList->DeleteQueueIcon(no, m_ParentBase);
        }
        
        CMatrixMapStatic* items = m_Top;
        int i = 0;
        while(items)
        {
            ++i;
            if(i == no)
            {
                LIST_DEL(items, m_Top, m_Bottom, m_PrevQueueItem, m_NextQueueItem);
                if(items->IsRobot())
                {
                    ReturnRobotResources(items->AsRobot());
                    HDelete(CMatrixRobotAI, (CMatrixRobotAI*)items, g_MatrixHeap);
                }
                else if(items->IsCannon())
                {
                    ReturnTurretResources(items->AsCannon());
                    items->UnjoinGroup();
                    g_MatrixMap->StaticDelete(items);
                }
                else if(items->IsFlyer())
                {
                    HDelete(CMatrixFlyer, (CMatrixFlyer*)items, g_MatrixHeap);
                }

                --m_Items;
                return m_Items;
            }
            items = items->m_NextQueueItem;           
        }
    }
    return 0;
}

CBuildQueue::~CBuildQueue()
{
    if(m_Items)
    {
        CMatrixMapStatic* items = m_Top;

        while(items)
        {
            if(items->m_NextQueueItem)
            {
			    items = items->m_NextQueueItem;
            }
            else
            {
                if(items->IsRobot())
                {
                    HDelete(CMatrixRobotAI, (CMatrixRobotAI*)items, g_MatrixHeap);
                }
                else if(items->IsFlyer())
                {
                    HDelete(CMatrixFlyer, (CMatrixFlyer*)items, g_MatrixHeap);
                }
                else if(items->IsCannon())
                {
                    items->TakingDamage(WEAPON_INSTANT_DEATH, D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), 0, nullptr);
                }
			    items = nullptr;
			    m_Top = nullptr;
			    m_Bottom = nullptr;

		    }

            if(items && items->m_PrevQueueItem)
            {
                if(items->m_PrevQueueItem->IsRobot())
                {
                    HDelete(CMatrixRobotAI, (CMatrixRobotAI*)items->m_PrevQueueItem, g_MatrixHeap);
                }
                else if(items->m_PrevQueueItem->IsFlyer())
                {
                    HDelete(CMatrixFlyer, (CMatrixFlyer*)items->m_PrevQueueItem, g_MatrixHeap);
                }
                else if(items->m_PrevQueueItem->IsCannon())
                {
                    items->m_PrevQueueItem->TakingDamage(WEAPON_INSTANT_DEATH, D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), 0, nullptr);
                }
            }
        }
    }
}

void CBuildQueue::DeleteItem(CMatrixMapStatic* item)
{
    if(m_Items){
        CMatrixMapStatic* i = m_Top;
        int cnt = 0;
        while(i){
            cnt++;
            if(i == item){
                DeleteItem(cnt);
                break;
            }
            i = i->m_NextQueueItem;
        }
    }
}

int CBuildQueue::GetRobotsCnt() const
{
    CMatrixMapStatic* objects = m_Top;
    int robots = 0;
    while(objects)
    {
        if(objects->GetObjectType() == OBJECT_TYPE_ROBOTAI) ++robots;
        objects = objects->m_NextQueueItem;
    }

    return robots;
}

void CBuildQueue::ClearStack()
{
    while(DeleteItem(1)) {}
    if(m_ParentBase->m_Side == PLAYER_SIDE)
    {
        CInterface* ifs = g_IFaceList->m_First;
        while(ifs)
        {
            if(ifs->m_strName == IF_MAIN)
            {
                CIFaceElement* els = ifs->m_FirstElement;
                while(els)
                {
                    if(IS_QUEUE_ICON(els->m_nId) && els->m_iParam == int(m_ParentBase))
                    {
                        els = ifs->DelElement(els);
                        continue;
                    }
                    els = els->m_NextElement;
                }
                break;
            }
            ifs = ifs->m_NextInterface;
        }
    }
}

void CBuildQueue::ReturnRobotResources(CMatrixRobotAI* robot)
{
    CMatrixSideUnit* s = g_MatrixMap->GetSideById(robot->GetSide());

    int titan_cost = 0, elec_cost = 0, energy_cost = 0, plasm_cost = 0;

    for(int i = 0; i < robot->m_ModulesCount; ++i)
    {
        int* res_point = nullptr;
        switch(robot->m_Module[i].m_Type)
        {
            case MRT_HULL: res_point = &g_Config.m_RobotHullsConsts[robot->m_Module[i].m_Kind].cost_titan; break;
            case MRT_CHASSIS: res_point = &g_Config.m_RobotChassisConsts[robot->m_Module[i].m_Kind].cost_titan; break;
            case MRT_HEAD: res_point = &g_Config.m_RobotHeadsConsts[robot->m_Module[i].m_Kind].cost_titan; break;
            case MRT_WEAPON: res_point = &g_Config.m_RobotWeaponsConsts[robot->m_Module[i].m_Kind].cost_titan; break;
            
            //default: break;
        }

        if(res_point)
        {
            titan_cost += res_point[0];
            elec_cost += res_point[1];
            energy_cost += res_point[2];
            plasm_cost += res_point[3];
        }
    }

    s->AddResourceAmount(TITAN, titan_cost);
    s->AddResourceAmount(ELECTRONICS, elec_cost);
    s->AddResourceAmount(ENERGY, energy_cost);
    s->AddResourceAmount(PLASMA, plasm_cost);
}

void CBuildQueue::ReturnTurretResources(CMatrixCannon* turret)
{
    CMatrixSideUnit* s = g_MatrixMap->GetSideById(turret->GetSide());

    int kind = turret->m_TurretKind;
    s->AddResourceAmount(TITAN,       g_Config.m_TurretsConsts[kind].cost_titan);
    s->AddResourceAmount(ELECTRONICS, g_Config.m_TurretsConsts[kind].cost_electronics);
    s->AddResourceAmount(ENERGY,      g_Config.m_TurretsConsts[kind].cost_energy);
    s->AddResourceAmount(PLASMA,      g_Config.m_TurretsConsts[kind].cost_plasma);
}

void CBuildQueue::KillBar()
{
    m_ProgressBar.KillClone(PBC_CLONE1);
}