// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixMap.hpp"
#include "MatrixShadowManager.hpp"
#include "MatrixObjectCannon.hpp"
#include "MatrixRobot.hpp"
#include "MatrixObjectBuilding.hpp"
#include "MatrixFlyer.hpp"
#include "ShadowStencil.hpp"
#include "Interface/CInterface.h"


float CMatrixCannon::GetSeekRadius(void)
{
    return g_Config.m_TurretsConsts[m_TurretKind].seek_target_range;
}

float CMatrixCannon::GetStrength(void)
{
    return g_Config.m_TurretsConsts[m_TurretKind].strength * (0.4f + 0.6f * (m_HitPoint / m_HitPointMax));
}

void CMatrixCannon::FireHandler(CMatrixMapStatic* hit, const D3DXVECTOR3& pos, dword user, dword flags)
{
    //SObjectCore* oc = (SObjectCore*)user;
    SObjectCore* oc = ((CMatrixCannon*)user)->m_Core;

    if(oc->m_Object && FLAG(flags, FEHF_DAMAGE_ROBOT))
    {
        // попадание!
        // обновим тайминг косой стрельбы
        oc->m_Object->AsCannon()->m_TimeFromFire = CANNON_TIME_FROM_FIRE;
    }

    if(oc->m_Object && oc->m_Object->AsCannon()->IsRefProtect())
    {
        oc->m_Object->AsCannon()->SetRefProtectHit();
    }
    else if(FLAG(flags, FEHF_LASTHIT)) oc->Release();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CMatrixCannon::~CMatrixCannon()
{
DTRACE();

    ReleaseMe();
    ModuleClear();
    if(m_ShadowProj)
    {
        HDelete(CMatrixShadowProj, m_ShadowProj, Base::g_MatrixHeap);
        m_ShadowProj = nullptr;
    }

    for(int i = 0; i < m_TurretWeapon.size(); ++i)
    {
        m_TurretWeapon[i].m_Weapon->Release();
        m_TurretWeapon[i].m_Weapon = nullptr;
    }
    m_TurretWeapon.clear();

    if(m_TargetCore) m_TargetCore->Release();
}

void CMatrixCannon::ModuleClear(void)
{
DTRACE();

    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Module[i].m_Graph)
        {
            UnloadObject(m_Module[i].m_Graph, nullptr);
            m_Module[i].m_Graph = nullptr;
        }

        if(m_CurrState == CANNON_DIP)
        {
            if(m_Module[i].m_Smoke.effect)
            {
                ((CMatrixEffectSmoke*)m_Module[i].m_Smoke.effect)->SetTTL(1000);
                m_Module[i].m_Smoke.Unconnect();
            }
        }
        else if(m_Module[i].m_ShadowStencil)
        {
            HDelete(CVOShadowStencil, m_Module[i].m_ShadowStencil, nullptr);
            m_Module[i].m_ShadowStencil = nullptr;
        }
    }

    ZeroMemory(m_Module, sizeof(m_Module));
    RChange(MR_ShadowStencil | MR_ShadowProjGeom | MR_ShadowProjTex | MR_Graph);
}

void CMatrixCannon::BoundGet(D3DXVECTOR3& bmin, D3DXVECTOR3& bmax)
{
DTRACE();

    bmin = D3DXVECTOR3(1e30f, 1e30f, 1e30f);
    bmax = D3DXVECTOR3(-1e30f, -1e30f, -1e30f);

    D3DXVECTOR3 tmin;
    D3DXVECTOR3 tmax;
    D3DXVECTOR3 v[8];

    for(int i = 0; i < m_ModulesCount; ++i)
    {
        m_Module[i].m_Graph->GetBound(tmin, tmax);

        v[0] = D3DXVECTOR3(tmin.x, tmin.y, tmin.z);
        v[1] = D3DXVECTOR3(tmax.x, tmin.y, tmin.z);
        v[2] = D3DXVECTOR3(tmax.x, tmax.y, tmin.z);
        v[3] = D3DXVECTOR3(tmin.x, tmax.y, tmin.z);
        v[4] = D3DXVECTOR3(tmin.x, tmin.y, tmax.z);
        v[5] = D3DXVECTOR3(tmax.x, tmin.y, tmax.z);
        v[6] = D3DXVECTOR3(tmax.x, tmax.y, tmax.z);
        v[7] = D3DXVECTOR3(tmin.x, tmax.y, tmax.z);

        for(int u = 0; u < 8; ++u)
        {
            D3DXVec3TransformCoord(&(v[u]), &(v[u]), &(m_Module[i].m_Matrix));
            bmin.x = min(bmin.x, v[u].x);
            bmin.y = min(bmin.y, v[u].y);
            bmin.z = min(bmin.z, v[u].z);
            bmax.x = max(bmax.x, v[u].x);
            bmax.y = max(bmax.y, v[u].y);
            bmax.z = max(bmax.z, v[u].z);
        }
    }
}


void CMatrixCannon::GetResources(dword need)
{
    if(m_CurrState == CANNON_DIP) return;

	if(need & m_RChange & (MR_Graph))
    {
		m_RChange &= ~MR_Graph;

        ASSERT(!m_ModulesCount);

        float hp = g_Config.m_TurretsConsts[m_TurretKind].structure;
        InitMaxHitpoint(hp);

        CWStr ctype(m_TurretKind, g_CacheHeap);

        CBlockPar* cache = g_CacheData->BlockPathGet(g_Config.m_TurretsConsts[m_TurretKind].model_path);
        CWStr model_path = cache->ParGet(L"BaseModel");
        //CWStr texture_path = cache->ParGet(L"BaseTexture");

        //Грузим основание турели (круглая блямба, которая втыкается в землю)
        m_Module[0].m_Type = TURRET_PART_BASE;
        m_Module[0].m_Graph = LoadObject(model_path.Add(L".vo"), Base::g_MatrixHeap, true);
        //m_Module[0].m_Graph = LoadObject(model_path.Add(L".vo"), Base::g_MatrixHeap, true, texture_path);

        m_ModulesCount = 0;
        while(true)
        {
            const wchar* nam = m_Module[m_ModulesCount].m_Graph->VO()->GetMatrixNameById(20);
            if(nam == nullptr) break;

            ++m_ModulesCount;

            ECannonUnitType type = TURRET_PART_EMPTY;
            if(*nam == 'T')
            {
                type = TURRET_PART_MOUNT;
                model_path = cache->ParGet(L"MountModel");
                //texture_path = cache->ParGet(L"MountTexture");
            }
            else if(*nam == 'S')
            {
                type = TURRET_PART_GUN;
                model_path = cache->ParGet(L"GunModel");
                //texture_path = cache->ParGet(L"GunTexture");
            }

            if(type == TURRET_PART_EMPTY) ERROR_S2(L"Not enough model parts for turret: ", g_Config.m_TurretsConsts[m_TurretKind].name);

            m_Module[m_ModulesCount].m_Type = type;
            m_Module[m_ModulesCount].m_Graph = LoadObject(model_path.Add(L".vo"), Base::g_MatrixHeap, true);
            //m_Module[m_ModulesCount].m_Graph = LoadObject(model_path.Add(L".vo"), Base::g_MatrixHeap, true, texture_path);

            //Добавляем башню/пушку турели (может содержать сразу несколько матриц (стволов) под выстрелы)
            if(type == TURRET_PART_GUN)
            {
                m_AsyncDelay = g_Config.m_TurretsConsts[m_TurretKind].guns_async_time;

                int n = m_Module[m_ModulesCount].m_Graph->VO()->GetMatrixCount();
                //Определяем число и тип орудий на турели
                int top_range = 0;
                for(int i = 0; i < g_Config.m_TurretsConsts[m_TurretKind].guns.size(); ++i)
                {
                    STurretWeapon gun;
                    m_TurretWeapon.push_back(gun);
                    m_TurretWeapon[i].m_TurretWeaponMatrixId = g_Config.m_TurretsConsts[m_TurretKind].guns[i].matrix_id;
                    m_TurretWeapon[i].m_Weapon = (CMatrixEffectWeapon*)CMatrixEffect::CreateWeapon(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 1), (dword)this, FireHandler, g_Config.m_TurretsConsts[m_TurretKind].guns[i].weapon_type);
                    m_TurretWeapon[i].m_Weapon->SetOwner(this);
                    m_TurretWeapon[i].m_AsyncAwaitTimer = 0;

                    int range = m_TurretWeapon[i].m_Weapon->GetWeaponDist();
                    if(range > top_range) top_range = range;
                }

                m_TurretWeaponsTopRange = top_range;
            }
        }

        ++m_ModulesCount;
	}

    if(need & m_RChange & (MR_Matrix))
    {
		m_RChange &= ~MR_Matrix;
        D3DXMatrixRotationZ(&m_Core->m_Matrix, m_Angle);

        m_Core->m_Matrix._41 = m_Pos.x;
        m_Core->m_Matrix._42 = m_Pos.y;

        int px = TruncFloat(m_Pos.x * INVERT(GLOBAL_SCALE));
        int py = TruncFloat(m_Pos.y * INVERT(GLOBAL_SCALE));

        int cnt = 0;
        float rv = 0;
        SMatrixMapPoint* mp = g_MatrixMap->PointGetTest(px, py); if(mp) { ++cnt; rv += mp->z; }
        mp = g_MatrixMap->PointGetTest(px + 1, py); if(mp) { ++cnt; rv += mp->z; }
        mp = g_MatrixMap->PointGetTest(px, py + 1); if(mp) { ++cnt; rv += mp->z; }
        mp = g_MatrixMap->PointGetTest(px + 1, py + 1); if(mp) { ++cnt; rv += mp->z; }

        if(cnt) m_Core->m_Matrix._43 = rv / cnt + m_AddH;
        else ERROR_S(L"Divide by zero probability in CMatrixCannon::GetResources!");

        D3DXMatrixInverse(&m_Core->m_IMatrix, nullptr, &m_Core->m_Matrix);

        D3DXMATRIX mx, m;
        const D3DXMATRIX* tm;
        D3DXVECTOR3 p(0, 0, 0);

        for(int i = 0; i < m_ModulesCount; ++i)
        {
            D3DXMatrixRotationZ(&m, m_Module[i].m_Angle);
            *(D3DXVECTOR3*)&m._41 = p;

            if(m_Module[i].m_Type == TURRET_PART_GUN)
            {
                D3DXMatrixRotationX(&mx, m_AngleX);
                mx *= m;
                D3DXMatrixMultiply(&m_Module[i].m_Matrix, &mx, &m_Core->m_Matrix);

                for(int w = 0; w < m_TurretWeapon.size(); ++w)
                {
                    const D3DXMATRIX* f = m_Module[i].m_Graph->GetMatrixById(m_TurretWeapon[w].m_TurretWeaponMatrixId);
                    D3DXMATRIX weapm = (*f) * m_Module[i].m_Matrix;

                    m_TurretWeapon[w].m_FireFrom = *(D3DXVECTOR3*)&weapm._41;
                    m_TurretWeapon[w].m_FireDir = *(D3DXVECTOR3*)&weapm._21;
                }
            }
            else m_Module[i].m_Matrix = m * m_Core->m_Matrix;

            D3DXMatrixInverse(&m_Module[i].m_IMatrix, nullptr, &m_Module[i].m_Matrix);

            if(i == (m_ModulesCount - 1))
            {
                m_FireCenter = *(D3DXVECTOR3*)&m_Module[i].m_Matrix._41;
                break;
            }

            ASSERT(m_Module[i].m_Graph);
            tm = m_Module[i].m_Graph->GetMatrixById(20);

            // this only takes into account Z-axis rotation.
            //If you uses other then D3DXMatrixRotationZ rotation, please correct following code
            p.x += tm->_41 * m._11 + tm->_42 * m._21;
            p.y += tm->_41 * m._12 + tm->_42 * m._22;
            p.z += tm->_43;
        }
	}

    /*
	if(need & m_RChange & (MR_Sort))
    {
		m_RChange &= ~MR_Sort;

		D3DXVECTOR3 pz=D3DXVECTOR3(0, 0, 0);
		D3DXVec3TransformCoord(&pz, &pz, &(m_Matrix * g_MatrixMap->GetSortMatrix()));
		m_Z = pz.z;
	}

	if(need & m_RChange & (MR_GraphSort))
    {
		m_RChange &= ~MR_GraphSort;

		for(int i = 0; i < m_ModulesCount; ++i)
        {
			ASSERT(m_Module[i].m_Graph);
			m_Module[i].m_Graph->SortIndexForAlpha(m_Matrix * g_MatrixMap->GetViewMatrix());
		}
	}
    */

	if(need & m_RChange & MR_ShadowStencil)
    {
		m_RChange &= ~MR_ShadowStencil;

		if(m_ShadowType != SHADOW_STENCIL)
        {
			for(int i = 0; i < m_ModulesCount; ++i)
            {
				if(m_Module[i].m_ShadowStencil)
                {
                    HDelete(CVOShadowStencil, m_Module[i].m_ShadowStencil, Base::g_MatrixHeap);
                    m_Module[i].m_ShadowStencil = nullptr;
                }
			}
		}
        else
        {
			for(int i = 0; i < m_ModulesCount; ++i)
            {
				ASSERT(m_Module[i].m_Graph);
				ASSERT(m_Module[i].m_Graph->VO());

				//if(m_Module[i].m_Graph->VO()->EdgeExist()) {
				if(!m_Module[i].m_ShadowStencil) m_Module[i].m_ShadowStencil = HNew(Base::g_MatrixHeap) CVOShadowStencil(Base::g_MatrixHeap);

                //if(!(m_Module[i].m_Graph->VO()->EdgeExist())) m_Module[i].m_Graph->VO()->EdgeBuild();

                //STENCIL
				//m_Module[i].m_ShadowStencil->Build(*(m_Module[i].m_Graph->VO()),m_Module[i].m_Graph->FrameVO(),
                //    g_MatrixMap->m_LightMain,m_Module[i].m_Matrix,IsNearBase()?g_MatrixMap->m_ShadowPlaneCutBase:g_MatrixMap->m_ShadowPlaneCut);

                D3DXVECTOR3 light;
                D3DXVec3TransformNormal(&light, &g_MatrixMap->m_LightMain, &m_Module[i].m_IMatrix);

                //D3DXPLANE cutpl;
                //D3DXPlaneTransform(&cutpl, IsNearBase()?(&g_MatrixMap->m_ShadowPlaneCutBase):(&g_MatrixMap->m_ShadowPlaneCut), &m_Module[i].m_IMatrix);
                float len = (GetRadius()*2) +  m_Module[i].m_Matrix._43 - (IsNearBase() ? g_MatrixMap->m_GroundZBase : g_MatrixMap->m_GroundZ);
                m_Module[i].m_ShadowStencil->CreateShadowVO(*(m_Module[i].m_Graph->VO()),m_Module[i].m_Graph->GetVOFrame(), light, len, false);
			}
		}
	}

    if(need & m_RChange & MR_ShadowProjGeom)
    {
        m_RChange &= ~MR_ShadowProjGeom;

        if(m_ShadowType != SHADOW_PROJ_DYNAMIC)
        {
            if(m_ShadowProj)
            {
                HDelete(CMatrixShadowProj, m_ShadowProj, Base::g_MatrixHeap);
                m_ShadowProj = nullptr;
            }
        }
        else
        {
            if(!m_ShadowProj) m_ShadowProj = HNew(Base::g_MatrixHeap) CMatrixShadowProj(Base::g_MatrixHeap, this);

            byte* buf = (byte*)_alloca(sizeof(CVectorObjectAnim*) * m_ModulesCount + sizeof(int) * m_ModulesCount + sizeof(D3DXMATRIX) * m_ModulesCount);
            CVectorObjectAnim** obj = (CVectorObjectAnim**)buf;
            int* noframe = (int*)(buf + sizeof(CVectorObjectAnim*) * m_ModulesCount);
            D3DXMATRIX* wm = (D3DXMATRIX*)(buf + sizeof(CVectorObjectAnim*) * m_ModulesCount + sizeof(int) * m_ModulesCount);

            for(int i = 0; i < m_ModulesCount; ++i)
            {
                ASSERT(m_Module[i].m_Graph);
                ASSERT(m_Module[i].m_Graph->VO());

                obj[i] = m_Module[i].m_Graph;
                noframe[i] = m_Module[i].m_Graph->GetVOFrame();
                wm[i] = m_Module[i].m_Matrix * m_Core->m_IMatrix;
            }

            ShadowProjBuildGeomList(*m_ShadowProj, m_ModulesCount, obj, noframe, wm, m_Core->m_Matrix, m_Core->m_IMatrix, g_MatrixMap->m_LightMain, int(100 / GLOBAL_SCALE), true);

            if(!(m_ShadowProj->IsProjected()))
            {
                HDelete(CMatrixShadowProj, m_ShadowProj, Base::g_MatrixHeap);
                m_ShadowProj = nullptr;
            }
        }
    }

    if(need & m_RChange & MR_ShadowProjTex)
    {
        m_RChange &= ~MR_ShadowProjTex;

        if(m_ShadowProj != nullptr)
        {
            byte* buf = (byte*)_alloca(sizeof(CVectorObjectAnim*) * m_ModulesCount + sizeof(int) * m_ModulesCount + sizeof(D3DXMATRIX) * m_ModulesCount);
            CVectorObjectAnim** obj = (CVectorObjectAnim**)buf;
            int* noframe = (int*)(buf + sizeof(CVectorObjectAnim*) * m_ModulesCount);
            D3DXMATRIX* wm = (D3DXMATRIX*)(buf + sizeof(CVectorObjectAnim*) * m_ModulesCount + sizeof(int) * m_ModulesCount);

            for(int i = 0; i < m_ModulesCount; ++i)
            {
                ASSERT(m_Module[i].m_Graph);
                ASSERT(m_Module[i].m_Graph->VO());

                obj[i] = m_Module[i].m_Graph;
                noframe[i] = m_Module[i].m_Graph->GetVOFrame();
                wm[i] = m_Module[i].m_Matrix * m_Core->m_IMatrix;
            }

            CTexture* tex = nullptr;

            ShadowProjBuildTextureList(*m_ShadowProj, m_ModulesCount, obj, tex, noframe, wm, m_Core->m_Matrix, m_Core->m_IMatrix, g_MatrixMap->m_LightMain, m_ShadowSize);
		}
	}
}


void CMatrixCannon::Tact(int cms)
{
	for(int i = 0; i < m_ModulesCount; ++i)
    {
		if(m_Module[i].m_Graph)
        {
			if(m_Module[i].m_Graph->Tact(cms))
            {
                if(m_ShadowType == SHADOW_STENCIL) RChange(MR_ShadowStencil);
                else if(m_ShadowType == SHADOW_PROJ_DYNAMIC) RChange(MR_ShadowProjTex);
			}

            if(m_Module[i].m_Graph->IsAnimEnd()) m_Module[i].m_Graph->SetAnimByName(ANIMATION_NAME_IDLE);
		}
	}

	//RChange(MR_Matrix);
}

bool CMatrixCannon::Pick(const D3DXVECTOR3& orig, const D3DXVECTOR3& dir, float* outt) const
{
    if(m_CurrState == CANNON_DIP) return false;
	for(int i = 0; i < m_ModulesCount; ++i)
    {
		if(m_Module[i].m_Graph)
        {
            if(m_Module[i].m_Graph->Pick(m_Module[i].m_Matrix, m_Module[i].m_IMatrix, orig, dir, outt)) return true;
		}
	}
	return false;
}

void CMatrixCannon::FreeDynamicResources(void)
{
    if(m_ShadowProj && (m_ShadowType == SHADOW_PROJ_DYNAMIC))
    {
        HDelete(CMatrixShadowProj, m_ShadowProj, Base::g_MatrixHeap);
        m_ShadowProj = nullptr;
        RChange(MR_ShadowProjGeom);
    }
    else if(m_ShadowType == SHADOW_STENCIL)
    {
        for(int i = 0; i < m_ModulesCount; ++i)
        {
            if(m_Module[i].m_ShadowStencil) m_Module[i].m_ShadowStencil->DX_Free();
        }
    }
}

void CMatrixCannon::BeforeDraw(void)
{
    dword sh = (g_Config.m_ShowProjShadows ? (MR_ShadowProjGeom | MR_ShadowProjTex) : 0) | (g_Config.m_ShowStencilShadows ? MR_ShadowStencil : 0);
    GetResources(MR_Matrix | MR_Graph | sh);

    if(m_ShowHitpointTime > 0 && m_HitPoint > 0 && m_CurrState != CANNON_DIP)
    {
        D3DXVECTOR3 pos(GetGeoCenter());
        if(TRACE_STOP_NONE == g_MatrixMap->Trace(nullptr, g_MatrixMap->m_Camera.GetFrustumCenter(), pos, TRACE_LANDSCAPE, nullptr))
        {
            D3DXVECTOR2 p = g_MatrixMap->m_Camera.Project(pos, g_MatrixMap->GetIdentityMatrix());
            m_ProgressBar.Modify(p.x - (PB_CANNON_WIDTH * 0.5f), p.y - GetRadius(), m_HitPoint * m_MaxHitPointInversed);
        }
    }

    for(int i = 0; i < m_ModulesCount; ++i)
    {
        m_Module[i].m_Graph->BeforeDraw();
        if(m_CurrState != CANNON_DIP && m_Module[i].m_ShadowStencil) m_Module[i].m_ShadowStencil->BeforeRender();
    }

    if(m_CurrState != ROBOT_DIP && m_ShadowProj) m_ShadowProj->BeforeRender();
}

void CMatrixCannon::Draw(void)
{
	//g_D3DD->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
    dword coltex = (dword)g_MatrixMap->GetSideColorTexture(m_Side)->Tex();

    for(int i = 0; i < 4; ++i)
    {
        ASSERT_DX(g_D3DD->SetSamplerState(i,D3DSAMP_MIPMAPLODBIAS, *((LPDWORD) (&g_MatrixMap->m_BiasCannons))));
    }

    if(m_CurrState == CANNON_DIP)
    {
    	g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
        for(int i = 0; i < m_ModulesCount; ++i)
        {
            if(m_Module[i].m_TTL <= 0) continue;
		    ASSERT_DX(g_D3DD->SetTransform( D3DTS_WORLD, &(m_Module[i].m_Matrix) ));
            m_Module[i].m_Graph->Draw(coltex);
	    }
    }
    else
    {
        if(m_CurrState == CANNON_UNDER_CONSTRUCTION) g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF00FF00);
        else g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Core->m_TerainColor);
	    for(int i = 0; i < m_ModulesCount; ++i)
        {
		    ASSERT(m_Module[i].m_Graph);
		    ASSERT_DX(g_D3DD->SetTransform( D3DTS_WORLD, &(m_Module[i].m_Matrix)));
            m_Module[i].m_Graph->Draw(coltex);
	    }
    }
	//g_D3DD->SetRenderState(D3DRS_NORMALIZENORMALS,  FALSE);

    /*
    for(int k = 0; k < 100; ++k)
    {
        D3DXVECTOR3 d(FSRND(1),FSRND(1),FSRND(1));
        D3DXVec3Normalize(&d, &d);
        CHelper::Create(1,0)->Line(m_GeoCenter, m_GeoCenter + d*m_Radius);
    }
    */
}

void CMatrixCannon::DrawShadowStencil(void)
{
    if(m_ShadowType != SHADOW_STENCIL) return;

    for(int i = 0; i < m_ModulesCount; ++i)
    {
		if(m_Module[i].m_ShadowStencil)
        {
			m_Module[i].m_ShadowStencil->Render(m_Module[i].m_Matrix);
		}
	}
}

void CMatrixCannon::DrawShadowProj(void)
{
    /*
    if(!m_ShadowProj) return;
    D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
    m._41 = m_ShadowProj->GetDX();
    m._42 = m_ShadowProj->GetDY();
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));

    m_ShadowProj->Render();
    */
}

void CMatrixCannon::OnLoad()
{
    UnitInit(m_TurretKind);
}


bool CMatrixCannon::CalcBounds(D3DXVECTOR3& minv, D3DXVECTOR3& maxv)
{
    if(!m_ModulesCount) return true;

    //RChange(MR_Matrix);
	GetResources(MR_Matrix | MR_Graph);

	D3DXVECTOR3 bminout, bmaxout;

	minv.x = 1e30f; minv.y = 1e30f; minv.z = 1e30f;
	maxv.x = -1e30f; maxv.y = -1e30f; maxv.z = -1e30f;

    for(int u = 1; u < m_ModulesCount; ++u) // skip basis
    {
        int cnt = m_Module[u].m_Graph->VO()->GetFramesCnt();
		for(int i = 0; i < cnt; ++i)
        {
			m_Module[u].m_Graph->VO()->GetBound(i,m_Module[u].m_Matrix,bminout,bmaxout);

            minv.x = min(minv.x, bminout.x);
            minv.y = min(minv.y, bminout.y);
            minv.z = min(minv.z, bminout.z);
            maxv.x = max(maxv.x, bmaxout.x);
            maxv.y = max(maxv.y, bmaxout.y);
            maxv.z = max(maxv.z, bmaxout.z);
		}
	}

    return false;

    /*
    GetResources(MR_Graph|MR_Matrix);

    SVOFrame* f = m_Module[0].m_Graph->VO()->FrameGet(0);
    minv.x = f->m_MinX;
    minv.y = f->m_MinY;
    maxv.x = f->m_MaxX;
    maxv.y = f->m_MaxY;

    for(int u = 0; u<m_ModulesCount; ++u)
    {
        int cnt = m_Module[u].m_Graph->VO()->Header()->m_FrameCnt;
        for(int i = 1; i < cnt; ++i)
        {
            SVOFrame *f = m_Module[u].m_Graph->VO()->FrameGet(i);

            minv.x = min(minv.x,f->m_MinX);
            minv.y = min(minv.y,f->m_MinY);
            maxv.x = max(maxv.x,f->m_MaxX);
            maxv.y = max(maxv.y,f->m_MaxY);
        }
    }

    return false;
    */
}


void CMatrixCannon::DIPTact(float ms)
{
    D3DXVECTOR3* pos;
    D3DXMATRIX* mat;

    //Разлетаются в разные стороны куски при взрыве турели (основание не отлетает, т.к. изначально лежит на земле и код его воспринимает как уже упавшее)
    bool del = true;
    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Module[i].m_TTL <= 0) continue;
        m_Module[i].m_TTL -= ms;

        pos = &m_Module[i].m_Pos;

        if(m_Module[i].m_TTL <= 0)
        {
            // create explosive
            pos->z += GetRadius();
            CMatrixEffect::CreateExplosion(*pos, ExplosionRobotBoomSmall, true);
            if(m_Module[i].m_Smoke.effect)
            {
                ((CMatrixEffectSmoke*)m_Module[i].m_Smoke.effect)->SetTTL(1000);
                m_Module[i].m_Smoke.Unconnect();
            }

            continue;
        }

        del = false;

        if(IS_ZERO_VECTOR(m_Module[i].m_Velocity)) continue;

        D3DXVECTOR3 oldpos = *pos;

        m_Module[i].m_Velocity.z -= 0.0002f * ms;
        (*pos) += m_Module[i].m_Velocity * float(ms);
        mat = &m_Module[i].m_Matrix;
     
        D3DXVECTOR3 hitpos;
        CMatrixMapStatic* o = g_MatrixMap->Trace(&hitpos, oldpos, *pos, TRACE_ALL, this);
        if(o == TRACE_STOP_WATER)
        {
            // in water
            if(g_MatrixMap->GetZ(hitpos.x, hitpos.y) < WATER_LEVEL)
            {
                m_Module[i].m_TTL = 0;
                CMatrixEffect::CreateKonusSplash(hitpos, D3DXVECTOR3(0, 0, 1), 10, 5, FSRND(M_PI), 1000, true, (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPLASH));
                if(m_Module[i].m_Smoke.effect)
                {
                    ((CMatrixEffectSmoke*)m_Module[i].m_Smoke.effect)->SetTTL(1000);
                    m_Module[i].m_Smoke.Unconnect();
                }
            }
        }
        else if(o == TRACE_STOP_LANDSCAPE)
        {
            //SND:
            m_Module[i].m_Velocity = D3DXVECTOR3(0, 0, 0);
            m_Module[i].m_Pos = hitpos;
        }
        else if(IS_TRACE_STOP_OBJECT(o))
        {
            int hit_effect;
            if(m_Module[i].m_Type == TURRET_PART_GUN) hit_effect = g_Config.m_TurretsConsts[m_TurretKind].gun_part_hit_effect;
            else hit_effect = g_Config.m_TurretsConsts[m_TurretKind].mount_part_hit_effect;

            if(hit_effect != WEAPON_NONE) o->TakingDamage(hit_effect, hitpos, m_Module[i].m_Velocity);
            m_Module[i].m_TTL = 1;
        }

        //if(pos->z < 0) pos->z = 0;

        float time = float(g_MatrixMap->GetTime());

        D3DXMATRIX m0,m1(g_MatrixMap->GetIdentityMatrix()), m2(g_MatrixMap->GetIdentityMatrix());
        const D3DXVECTOR3 &pos1 = m_Module[i].m_Graph->VO()->GetFrameGeoCenter(m_Module[i].m_Graph->GetVOFrame());
        *(D3DXVECTOR3*)&m1._41 = pos1;
        *(D3DXVECTOR3*)&m2._41 = -pos1;
        D3DXMatrixRotationYawPitchRoll(&m0, m_Module[i].m_dy * time, m_Module[i].m_dp * time, m_Module[i].m_dr * time);
        *mat = m2 * m0 * m1;

        mat->_41 = pos->x;
        mat->_42 = pos->y;
        mat->_43 = pos->z;

        if(m_Module[i].m_Smoke.effect) ((CMatrixEffectSmoke*)m_Module[i].m_Smoke.effect)->SetPos(*pos);
    }

    if(del) g_MatrixMap->StaticDelete(this);
}

struct FTData
{
    D3DXVECTOR3* cdir = nullptr;
    float coss = 0.0f;
    int   side = 0;
    float dist_cur = 0.0f;
    float dist_fire = 0.0f;
    CMatrixMapStatic** target = nullptr;
    CMatrixMapStatic* skip = nullptr;
};

static bool FindTarget(const D3DXVECTOR3& center, CMatrixMapStatic* ms, dword user)
{
    FTData* d = (FTData*)user;

    if(ms->GetSide() == d->side) return true;

    D3DXVECTOR3 dir(ms->GetGeoCenter() - center);

    float distc = D3DXVec3LengthSq(&dir);
    if (distc > d->dist_fire && d->dist_cur < d->dist_fire) return true;
    bool match = distc < d->dist_fire && d->dist_cur > d->dist_fire;
    D3DXVec3Normalize(&dir, &dir);

    float dot = D3DXVec3Dot(&dir, d->cdir);

    if(match || dot > d->coss)
    {
        CMatrixMapStatic* cel = g_MatrixMap->Trace(nullptr, center, ms->GetGeoCenter(), TRACE_OBJECTSPHERE | TRACE_ROBOT | TRACE_FLYER | TRACE_LANDSCAPE, d->skip);

        if(cel == ms)
        {
            // ландшафт не помеха. тогда может объект за зданием или за другой батвой?
            cel = g_MatrixMap->Trace(nullptr, center, ms->GetGeoCenter(), TRACE_BUILDING | TRACE_CANNON | TRACE_OBJECT, d->skip);
            if(cel == nullptr)
            {
                d->dist_cur = distc;
                d->coss = dot;
                *d->target = ms;
            }
        }
    }

    return true;
}

void CMatrixCannon::BeginFireAnimation(void)
{
    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Module[i].m_Type == TURRET_PART_GUN)
        {

            if(m_Module[i].m_Graph->SetAnimByNameNoBegin(ANIMATION_NAME_FIRELOOP))
            {
                // not looped
                m_Module[i].m_Graph->SetAnimByName(ANIMATION_NAME_FIRE, 0);
            }
            break;
        }
    }
}

void CMatrixCannon::EndFireAnimation(void)
{
    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Module[i].m_Type == TURRET_PART_GUN)
        {
            m_Module[i].m_Graph->SetAnimLooped(0);
            break;
        }
    }
}

void CMatrixCannon::PauseTact(int tact)
{
    SetPBOutOfScreen();
    if(m_CurrState == CANNON_DIP) return;
    if(m_CurrState != CANNON_UNDER_CONSTRUCTION)
    {
        if(m_ShowHitpointTime > 0)
        {
            m_ShowHitpointTime -= tact;
            if(m_ShowHitpointTime < 0) m_ShowHitpointTime = 0;
        }
    }
    else m_ShowHitpointTime = 1;
}


void CMatrixCannon::LogicTact(int tact)
{
    SetPBOutOfScreen();

    if(m_CurrState == CANNON_DIP)
    {
        DIPTact(float(tact));
        return;
    }

    if(m_MiniMapFlashTime > 0)
    {
        m_MiniMapFlashTime -= tact;
    }

    if(m_ParentBuilding && m_ParentBuilding->m_Side != m_Side)
    {
        SetSide(m_ParentBuilding->m_Side);

        CMatrixMapStatic* ms = CMatrixMapStatic::GetFirstLogic();
        while(ms)
        {
            if(ms->IsRobot())
            {
                if(ms->AsRobot()->GetEnv()->SearchEnemy(this)) ms->AsRobot()->GetEnv()->RemoveFromList(this);
            }
            ms = ms->GetNextLogic();
        }
    }

    if(m_UnderAttackTime > 0)
    {
        m_UnderAttackTime -= tact;
        if(m_UnderAttackTime < 0) m_UnderAttackTime = 0;
    }

    //Если пушка недавно отстрелялась и нужно сбросить таймер рассинхрона орудий
    if(m_IsAsyncCooldown)
    {
        byte guns_left = m_TurretWeapon.size();
        for(int i = 0; i < m_TurretWeapon.size(); ++i)
        {
            m_TurretWeapon[i].m_AsyncAwaitTimer = max(m_TurretWeapon[i].m_AsyncAwaitTimer - tact, 0);
            if(!m_TurretWeapon[i].m_AsyncAwaitTimer) --guns_left;
        }

        if(!guns_left) m_IsAsyncCooldown = false;
    }

    if(m_EndFireAfterAsync)
    {
        m_EndFireAfterAsync = max(m_EndFireAfterAsync - tact, 0);
        if(!m_EndFireAfterAsync) m_NextGunToShot = 0;
    }

    if(m_CurrState != CANNON_UNDER_CONSTRUCTION)
    {
        if(m_ShowHitpointTime > 0)
        {
            m_ShowHitpointTime -= tact;
            if(m_ShowHitpointTime < 0) m_ShowHitpointTime = 0;
        }
    }
    else
    {
        /*
        if(m_ParentBuilding && m_ParentBuilding->m_BS.m_Top == this)
        {
            m_NextTimeAblaze += tact;
            float percent_done = float(m_NextTimeAblaze) / float(g_Config.m_Timings[UNIT_TURRET]);

            if(m_NextTimeAblaze > )
            SetHitPoint(GetMaxHitPoint() * percent_done);
        }
        */

        m_ShowHitpointTime = 1;
        return;
    }

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
                D3DXVECTOR3 temp = { m_Core->m_Matrix._41 - pos.x, m_Core->m_Matrix._42 - pos.y, m_Core->m_Matrix._43 - pos.z };
                D3DXVec3Normalize(&dir, &temp);
                
            } while(!Pick(pos, dir, &t) && (--cnt > 0));

            if(cnt > 0) CMatrixEffect::CreateFire(nullptr, pos + dir * (t + 2), 100, 2500, 10, 2.5f, false, FIRE_SPEED, g_Config.m_WeaponsConsts[ablaze_effect_num].close_color_rgb, g_Config.m_WeaponsConsts[ablaze_effect_num].far_color_rgb);

            //Если турель была уничтожена огнём
            if(TakingDamage(ablaze_effect_num, pos, dir, m_LastDelayDamageSide, nullptr)) return;
        }
    }

    if(IsShorted())
    {
        int shorted_effect_num = IsShorted();
        while(g_MatrixMap->GetTime() > m_NextTimeShorted)
        {
            m_NextTimeShorted += SHORTED_OUT_LOGIC_PERIOD;

            D3DXVECTOR3 d1(GetGeoCenter()), d2(GetGeoCenter()), dir, pos;
            float t;

            int cnt = 4;
            do
            {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                D3DXVECTOR3 temp = { m_Core->m_Matrix._41 - pos.x, m_Core->m_Matrix._42 - pos.y, m_Core->m_Matrix._43 - pos.z };
                D3DXVec3Normalize(&dir, &temp);
                
            } while(!Pick(pos, dir, &t) && (--cnt > 0));
            
            if(cnt > 0) d1 = pos + dir * t;

            do
            {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                D3DXVECTOR3 temp = { m_Core->m_Matrix._41 - pos.x, m_Core->m_Matrix._42 - pos.y, m_Core->m_Matrix._43 - pos.z };
                D3DXVec3Normalize(&dir, &temp);
                
                Pick(pos, dir, &t);
            } while(!Pick(pos, dir, &t) && (--cnt > 0));
            
            if(cnt > 0)
            {
                d2 = pos + dir * t;
                CMatrixEffect::CreateShorted(d1, d2, FRND(400) + 100, g_Config.m_WeaponsConsts[shorted_effect_num].hex_BGRA_sprites_color, g_Config.m_WeaponsConsts[shorted_effect_num].sprite_set[0].sprites_num[0]);
            }

            if(TakingDamage(shorted_effect_num, pos, dir, m_LastDelayDamageSide, nullptr)) return;
        }
        return;
    }

    //Непосредственно логика поведения пушки

    STurretsConsts* props = &g_Config.m_TurretsConsts[m_TurretKind];

    bool its_time = false;
    int delta = m_FireNextThinkTime-g_MatrixMap->GetTime();
    if(delta < 0 || delta > CANNON_FIRE_THINK_PERIOD)
    {
        its_time = true;
        m_FireNextThinkTime = g_MatrixMap->GetTime() + CANNON_FIRE_THINK_PERIOD;
    }

    if(its_time)
    {
        // Seek new target
        // seek side target

        CMatrixMapStatic* tgt = nullptr;

        FTData data;
        //data.dist = props->seek_target_range * props->seek_target_range;
        data.coss = -1;
        data.target = &tgt;
        data.side = m_Side;
        data.skip = this;

        data.dist_fire = POW2(GetFireRadius());
        data.dist_cur = POW2(props->seek_target_range);

        // seek robots and flyers

        data.cdir = &m_TurretWeapon[0].m_FireDir;

        m_TargetDisp = D3DXVECTOR3(0, 0, 0);

        g_MatrixMap->FindObjects(GetGeoCenter(), props->seek_target_range, 1, TRACE_ROBOT | TRACE_FLYER, nullptr, FindTarget, (dword)&data);

        if(tgt)
        {
            if(m_TargetCore) m_TargetCore->Release();
            m_TargetCore = tgt->GetCore(DEBUG_CALL_INFO);
        }
        else
        {
            // цель не найдена (уехала далеко наверное)
            if(m_TargetCore) m_TargetCore->Release();
            m_TargetCore = nullptr;
        }
    }

    if(m_TargetCore == nullptr)
    {

no_target:
        // цели нет. 

        if(m_NullTargetTime > 0)
        {
            // стрелять надо...
            // делаем вид, что стреляем мимо...
            m_TimeFromFire -= tact;
            if(m_TimeFromFire < 0) m_TimeFromFire = 0;

            // продолжаем стрельбу в течении m_NullTargetTime времени
            m_NullTargetTime -= tact;
            if(m_NullTargetTime <= 0)
            {
                for(int i = 0; i < m_TurretWeapon.size(); ++i) m_TurretWeapon[i].m_Weapon->FireEnd();
                EndFireAnimation();
                m_NullTargetTime = 0;
                return;
            }
        }
        else
        {
            // все, стрельба окончена. обновим тайминг косой стрельбы
            m_TimeFromFire = CANNON_TIME_FROM_FIRE;
            m_TargetDisp = D3DXVECTOR3(0, 0, 0);
        }

        //Отрабатываем логику выстрела
        bool fire_was = false;
        for(int i = 0; i < m_TurretWeapon.size(); ++i)
        {
            SETFLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION);
            RESETFLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION_HIT);

            m_TurretWeapon[i].m_Weapon->ResetFireCount();
            m_TurretWeapon[i].m_Weapon->Tact(float(tact));
            fire_was |= m_TurretWeapon[i].m_Weapon->IsFireWas();
            bool hw = m_TurretWeapon[i].m_Weapon->IsHitWas();
            if(hw || FLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION_HIT)) m_TimeFromFire = CANNON_TIME_FROM_FIRE;
            else m_Core->m_Ref += m_TurretWeapon[i].m_Weapon->GetFireCount();

            RESETFLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION);
            RESETFLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION_HIT);
        }

        if(fire_was) BeginFireAnimation();

        return;
    }
    else
    {
        // цель есть
        // доварачиваем дуло

        bool matchz = false;
        bool matchx = false;

        D3DXVECTOR3 tgtpos;
        tgtpos = m_TargetCore->m_GeoCenter + m_TargetDisp;
        //tgtpos.z += FSRND(5);

        RChange(MR_Matrix);
        GetResources(MR_Matrix);

        float mul = 0;
        if(!props->rotation_speed) mul = 1.0f - pow(0.995f, tact);

        D3DXVECTOR2 dir(-(tgtpos.x - m_FireCenter.x), (tgtpos.y - m_FireCenter.y));

        float dang = atan2(dir.x, dir.y);
        float cang = m_Module[1].m_Angle + m_Angle;

        float da = (float)AngleDist(cang, dang);

        if(!props->rotation_speed)
        {
            if(fabs(da) < CANNNON_MIN_DANGLE)
            {
                matchz = true;
                da = 0;
                m_Module[1].m_Angle = dang;
            }
            else
            {
                da *= mul;
                m_Module[1].m_Angle += da;
            }
        }
        else
        {
            if(fabs(da) < props->rotation_speed + 0.001)
            {
                m_Module[1].m_Angle += da;
                matchz = true;
            }
	        else if(da < 0) m_Module[1].m_Angle -= props->rotation_speed;
	        else m_Module[1].m_Angle += props->rotation_speed;
        }

        m_Module[2].m_Angle = m_Module[1].m_Angle;

        RChange(MR_Matrix);
        GetResources(MR_Matrix);

        dir = D3DXVECTOR2(-(tgtpos.x - m_FireCenter.x), (tgtpos.y - m_FireCenter.y));

        dang = atan2((tgtpos.z - m_FireCenter.z), D3DXVec2Length(&dir));
        if(dang > props->highest_vertical_angle) dang = props->highest_vertical_angle;
        else if(dang < props->lowest_vertical_angle) dang = props->lowest_vertical_angle;

        da = (float)AngleDist(m_AngleX, dang);

        if(!props->rotation_speed)
        {
            if(fabs(da) < CANNNON_MIN_DANGLE)
            {
                matchx = true;
                da = 0;
                m_AngleX = dang;
            }
            else
            {
                da *= mul;
                m_AngleX += da;
            }
        }
        else
        {
            if(fabs(da) < props->rotation_speed + 0.001)
            {
                m_AngleX += da;
                matchx = true;
            }
	        else if(da < 0) m_AngleX -= props->rotation_speed;
	        else m_AngleX += props->rotation_speed;
        }

        RChange(MR_Matrix | MR_ShadowStencil | MR_ShadowProjTex);
        GetResources(MR_Matrix);

        // доварачиваем оружие
        for(int i = 0; i < m_TurretWeapon.size(); ++i)
        {
            m_TurretWeapon[i].m_Weapon->Modify(m_TurretWeapon[i].m_FireFrom, m_TurretWeapon[i].m_FireDir, D3DXVECTOR3(0, 0, 0));
        }

        // а проверм-ка, надо ли стрелять...
        if(!g_Config.m_CannonsLogic)
        {
            // логику вообще отрубили
            m_TargetCore->Release();
            m_TargetCore = nullptr;

            goto no_target;
        }

        if(!its_time) goto no_target; // не время палить во все стороны

        if(!matchx || !matchz)
        {
            //Ещё не навелись...
            goto no_target; 
        }

        // проверка попадания цели в зону поражения
        D3DXVECTOR3 temp = m_TargetCore->m_GeoCenter - GetGeoCenter();
        float dq = D3DXVec3LengthSq(&temp);
        float ddq = GetFireRadius();
        if(dq > POW2(ddq))
        {
            // нет, не дострелим
            goto no_target;
        }

        // и все-таки еще разок проверим
        for(int i = 0; i < m_TurretWeapon.size(); ++i)
        {
            D3DXVECTOR3 hp = m_TurretWeapon[i].m_FireFrom + m_TurretWeapon[i].m_FireDir * m_TurretWeapon[i].m_Weapon->GetWeaponDist();
            //CMatrixMapStatic* s = 
            g_MatrixMap->Trace(&hp, m_TurretWeapon[i].m_FireFrom, hp, TRACE_ALL, this);

            float dist = DistOtrezokPoint(m_TurretWeapon[i].m_FireFrom, hp, m_TargetCore->m_GeoCenter);
            if(dist > m_TargetCore->m_Radius * 2)
            {
                // так. все равно промажем.
                goto no_target;
            }
        }
    }

    //цель в зоне обстрела, жмём на гашетку
    /*
    if(m_TargetCore && m_TargetCore->m_Object && m_TargetCore->m_Object->GetObjectType() == OBJECT_TYPE_ROBOTAI)
    {
        CMatrixRobotAI* tgt = (CMatrixRobotAI*)m_TargetCore->m_Object;

        if(!tgt->GetEnv()->SearchEnemy(this)) tgt->GetEnv()->AddToList(this);
    }
    */

    m_NullTargetTime = CANNON_NULL_TARGET_TIME; // типа, чтобы стрелять ещё некоторое время после потери цели...

    for(int i = 0; i < m_TurretWeapon.size(); ++i)
    {
        if(m_NextGunToShot == i && !m_TurretWeapon[i].m_AsyncAwaitTimer)
        {
            m_TurretWeapon[i].m_Weapon->FireBegin(D3DXVECTOR3(0, 0, 0), this);
            m_NextGunToShot = i < m_TurretWeapon.size() - 1 ? i + 1 : 0;

            if(m_AsyncDelay)
            {
                m_TurretWeapon[m_NextGunToShot].m_AsyncAwaitTimer = m_AsyncDelay;
                m_EndFireAfterAsync = m_TurretWeapon[i].m_Weapon->GetShotsDelay() + m_AsyncDelay * 1.1; //Нужно, чтобы сбрасывать маркер очерёдности стрельбы после того, как цель покинула зону поражения
                m_IsAsyncCooldown = true;
            }
        }
    }

    //Отрабатываем логику выстрела
    bool fire_was = false;
    for(int i = 0; i < m_TurretWeapon.size(); ++i)
    {
        SETFLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION);
        RESETFLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION_HIT);

        m_TurretWeapon[i].m_Weapon->ResetFireCount();
        m_TurretWeapon[i].m_Weapon->Tact(float(tact));
        fire_was |= m_TurretWeapon[i].m_Weapon->IsFireWas();
        bool hw = m_TurretWeapon[i].m_Weapon->IsHitWas();
        if(hw || FLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION_HIT)) m_TimeFromFire = CANNON_TIME_FROM_FIRE;
        else m_Core->m_Ref += m_TurretWeapon[i].m_Weapon->GetFireCount();
        RESETFLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION);
        RESETFLAG(m_ObjectFlags, OBJECT_CANNON_REF_PROTECTION_HIT);
    }

    if(fire_was) BeginFireAnimation();

    m_TimeFromFire -= tact;
    if(m_TimeFromFire <= 0)
    {
        // стреляем мимо!
        // корректируем дуло
        m_TargetDisp = D3DXVECTOR3(FSRND(m_TargetCore->m_Radius * 0.5f), FSRND(m_TargetCore->m_Radius * 0.5f), FSRND(m_TargetCore->m_Radius * 0.5f));
        m_TimeFromFire = CANNON_TIME_FROM_FIRE;

        m_NextGunToShot = 0;
    }

    //CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_GeoCenter.x, m_GeoCenter.y, 20), D3DXVECTOR3(m_GeoCenter.x, m_GeoCenter.y, 20) + 100 * D3DXVECTOR3(cos(m_Module[1].m_Angle), sin(m_Module[1].m_Angle), 0));
}

bool CMatrixCannon::TakingDamage(
    int weap,
    const D3DXVECTOR3& pos,
    const D3DXVECTOR3&,
    int attacker_side,
    CMatrixMapStatic* attaker
)
{
    bool friendly_fire = false;
    float damage_coef;

    if(weap == WEAPON_INSTANT_DEATH) goto inst_death;
    if(IsInvulnerable()) return false;
    if(m_CurrState == CANNON_DIP) return true;

    friendly_fire = (attacker_side != NEUTRAL_SIDE) && (attacker_side == m_Side);
    damage_coef = (friendly_fire || m_Side != PLAYER_SIDE) ? 1.0f : g_MatrixMap->m_Difficulty.coef_enemy_damage_to_player_side;
    if(friendly_fire && m_Side == PLAYER_SIDE) damage_coef = damage_coef * g_MatrixMap->m_Difficulty.coef_friendly_fire;

    CMatrixEffectWeapon::SoundHit(weap, pos);

    if(g_Config.m_WeaponsConsts[weap].is_repairer)
    {
        m_HitPoint = min(m_HitPoint + g_Config.m_WeaponsConsts[weap].damage.to_turrets, m_HitPointMax);
        m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);

        return false;
    }

    //m_HitPoint = max(m_HitPoint - damage_coef * friendly_fire ? g_Config.m_WeaponsConsts[weap].friendly_damage.to_turrets : g_Config.m_WeaponsConsts[weap].damage.to_turrets, g_Config.m_WeaponsConsts[weap].non_lethal_threshold.to_turrets);
    m_HitPoint = max(m_HitPoint - damage_coef * g_Config.m_WeaponsConsts[weap].damage.to_turrets, g_Config.m_WeaponsConsts[weap].non_lethal_threshold.to_turrets);
    if(m_HitPoint >= 0) m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);
    else m_ProgressBar.Modify(0);

    if(!friendly_fire) m_MiniMapFlashTime = FLASH_PERIOD;
    if(FLAG(g_MatrixMap->m_Flags, MMFLAG_FLYCAM))
    {
        if(attaker) g_MatrixMap->m_Camera.AddWarPair(this, attaker);
    }

    if(m_Side == PLAYER_SIDE && !friendly_fire)
    {
        if(!m_UnderAttackTime)
        {
            int ss = IRND(3);
            if(!ss) CSound::Play(S_SIDE_UNDER_ATTACK_1);
            else if(ss == 1) CSound::Play(S_SIDE_UNDER_ATTACK_2);
            else if(ss == 2) CSound::Play(S_SIDE_UNDER_ATTACK_3);
        }
        m_UnderAttackTime = UNDER_ATTACK_IDLE_TIME;
    }

    //Проверяем, какие эффекты накладывает попавшее в турель оружие
    if(g_Config.m_WeaponsConsts[weap].extra_effects.size())
    {
        for(int i = 0; i < (int)g_Config.m_WeaponsConsts[weap].extra_effects.size(); ++i)
        {
            int effect = g_Config.m_WeaponsConsts[weap].extra_effects[i].type;
            byte effect_type = g_Config.m_WeaponsConsts[effect].secondary_effect;
            if(effect_type == SECONDARY_EFFECT_ABLAZE)
            {
                if(!g_Config.m_WeaponsConsts[effect].damage.to_turrets) continue;
                int new_priority = g_Config.m_WeaponsConsts[effect].effect_priority;

                if(IsAblaze()) effect = new_priority >= g_Config.m_WeaponsConsts[IsAblaze()].effect_priority ? effect : IsAblaze(); //Если наложенный ранее эффект горения имеет более высокий приоритет, то обновляем эффект по его статам
                MarkAblaze(effect);
                m_LastDelayDamageSide = attacker_side;

                int ttl = GetAblazeTTL();
                ttl = min(max(ttl + g_Config.m_WeaponsConsts[weap].extra_effects[i].duration_per_hit, 0), g_Config.m_WeaponsConsts[weap].extra_effects[i].max_duration);
                SetAblazeTTL(ttl);

                m_NextTimeAblaze = g_MatrixMap->GetTime(); //То есть в первый раз считаем логику получения урона от огня немедленно
            }
            else if(effect_type == SECONDARY_EFFECT_SHORTED_OUT)
            {
                for(int i = 0; i < m_TurretWeapon.size(); ++i) m_TurretWeapon[i].m_Weapon->FireEnd();
                for(int i = 0; i < m_ModulesCount; ++i)
                {
                    if(m_Module[i].m_Type == TURRET_PART_GUN)
                    {
                        m_Module[i].m_Graph->SetAnimLooped(0);
                        break;
                    }
                }

                int new_priority = g_Config.m_WeaponsConsts[effect].effect_priority;
                if(IsShorted()) effect = new_priority >= g_Config.m_WeaponsConsts[IsShorted()].effect_priority ? effect : IsShorted(); //Если наложенный ранее эффект стана имеет более высокий приоритет, то обновляем эффект по его статам
                MarkShorted(effect);
                m_LastDelayDamageSide = attacker_side;

                int ttl = GetShortedTTL();
                float dur_per_hit = g_Config.m_WeaponsConsts[weap].extra_effects[i].duration_per_hit;
                float max_dur = g_Config.m_WeaponsConsts[weap].extra_effects[i].max_duration * 3; //Турели станятся на большее время, т.к. в ваниле они не имели лимита продолжительности стана вовсе
                ttl = (int)min(max(ttl + dur_per_hit, 0), max_dur);
                if(ttl) SetShortedTTL(ttl);

                m_NextTimeShorted = g_MatrixMap->GetTime();
            }
        }
    }
    else m_LastDelayDamageSide = 0;

    if(m_HitPoint > 0)
    {
        if(g_Config.m_WeaponsConsts[weap].explosive_hit) CMatrixEffect::CreateExplosion(pos, ExplosionRobotHit);
    }
    else
    {
        if(attacker_side != NEUTRAL_SIDE && !friendly_fire)
        {
            g_MatrixMap->GetSideById(attacker_side)->IncStatValue(STAT_TURRET_KILL);
        }

inst_death:;
        
        CMatrixEffect::CreateExplosion(*(D3DXVECTOR3*)&m_Core->m_Matrix._41, ExplosionRobotBoom, true);

        m_ShadowType = SHADOW_OFF;
        RChange(MR_ShadowProjGeom | MR_ShadowStencil);
        GetResources(MR_ShadowProjGeom | MR_ShadowStencil);
        m_CurrState = CANNON_DIP;

        ReleaseMe();

        for(int i = 0; i < m_TurretWeapon.size(); ++i)
        {
            m_TurretWeapon[i].m_Weapon->Release();
        }
        m_TurretWeapon.clear();

        ///bool cstay = FRND(1) < 0.5f;

        m_Module[0].m_TTL = FRND(3000) + 2000;
        m_Module[0].m_Pos.x = m_Module[0].m_Matrix._41;
        m_Module[0].m_Pos.y = m_Module[0].m_Matrix._42;
        m_Module[0].m_Pos.z = m_Module[0].m_Matrix._43;

        //if(cstay)
        //{
        m_Module[0].m_dp = 0;
        m_Module[0].m_dy = 0;
        m_Module[0].m_dr = 0;
        m_Module[0].m_Velocity = D3DXVECTOR3(0, 0, 0);
        //}
        //else
        //{
            //m_Module[0].m_dp = FSRND(0.0005f);
            //m_Module[0].m_dy = FSRND(0.0005f);
            //m_Module[0].m_dr = FSRND(0.0005f);
            //m_Module[0].m_Velocity = D3DXVECTOR3(0, 0, 0.1f);
        //}

        m_Module[0].m_Smoke.effect = nullptr;
        CMatrixEffect::CreateSmoke(&m_Module[0].m_Smoke, m_Module[0].m_Pos, m_Module[0].m_TTL + 100000, 1000, 100, 0xFF000000, 1.0f / 30.0f);

        for(int i = 1; i < m_ModulesCount; ++i)
        {
            m_Module[i].m_dp = FSRND(0.005f);
            m_Module[i].m_dy = FSRND(0.005f);
            m_Module[i].m_dr = FSRND(0.005f);
            m_Module[i].m_Velocity = D3DXVECTOR3(FSRND(0.08f), FSRND(0.08f),0.1f);
            m_Module[i].m_TTL = FRND(3000) + 2000;
            m_Module[i].m_Pos.x = m_Module[i].m_Matrix._41;
            m_Module[i].m_Pos.y = m_Module[i].m_Matrix._42;
            m_Module[i].m_Pos.z = m_Module[i].m_Matrix._43;
            m_Module[i].m_Smoke.effect = nullptr;
            CMatrixEffect::CreateSmoke(&m_Module[i].m_Smoke, m_Module[i].m_Pos, m_Module[i].m_TTL + 100000, 1000, 100, 0xFF000000, 1.0f / 30.0f);
        }

        return true;
    }

    return false;
}

void CMatrixCannon::ReleaseMe(void)
{
    CMatrixSideUnit* s = nullptr;
    if(m_ParentBuilding)
    {
        for(int i = 0; i < MAX_PLACES; ++i)
        {
            if(m_ParentBuilding->m_TurretsPlaces[i].m_Coord.x == Float2Int(m_Pos.x / GLOBAL_SCALE_MOVE) && m_ParentBuilding->m_TurretsPlaces[i].m_Coord.y == Float2Int(m_Pos.y / GLOBAL_SCALE_MOVE))
            {
                m_ParentBuilding->m_TurretsPlaces[i].m_CannonType = -1;
            }
        }
        
        if(g_MatrixMap->GetPlayerSide()->m_ActiveObject == m_ParentBuilding)
        {
            g_IFaceList->CreateDynamicTurrets(m_ParentBuilding);
        }
    }

    if(m_ParentBuilding && m_ParentBuilding->m_TurretsHave)
    {
        --m_ParentBuilding->m_TurretsHave;

        int side = m_ParentBuilding->GetSide();
        if(side != NEUTRAL_SIDE)
        {
            CMatrixSideUnit *su = g_MatrixMap->GetSideById(side);
            if(su->m_CurrentAction == BUILDING_TURRET || FLAG(g_IFaceList->m_IfListFlags,PREORDER_BUILD_TURRET))
            {
                m_ParentBuilding->CreatePlacesShow();
                m_ParentBuilding = nullptr;
            }
        }
        CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
    }
    
    CMatrixMapStatic* objects = CMatrixMapStatic::GetFirstLogic();
    while(objects)
    {
        if(objects->IsRobotAlive())
        {
            objects->AsRobot()->GetEnv()->RemoveFromList(this);
        }
        else if(objects->IsBuildingAlive())
        {
            objects->AsBuilding()->m_BS.DeleteItem(this);
        }
        objects = objects->GetNextLogic();
    }
}

bool CMatrixCannon::InRect(const CRect& rect) const
{
    if(m_CurrState == CANNON_DIP) return false;

    D3DXVECTOR3 dir;
    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.top), dir);
    if(Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, nullptr)) return true;

    D3DXMATRIX s,t;
    SEVH_data d;
    
    t = g_MatrixMap->m_Camera.GetViewMatrix() * g_MatrixMap->m_Camera.GetProjMatrix();
    D3DXMatrixScaling(&s, float(g_ScreenX/2),float(-g_ScreenY/2),1);
    s._41 = s._11;
    s._42 = float(g_ScreenY/2);
    t *= s;
    d.rect = &rect;

    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Module[i].m_Graph)
        {
            d.found = false;
            d.m = m_Module[i].m_Matrix * t;
            m_Module[i].m_Graph->EnumFrameVerts(EnumVertsHandler, (dword)&d);
            if(d.found) return true;
        }
    }
    
    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.bottom), dir);
    if(Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, nullptr)) return true;

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.right, rect.top), dir);
    if(Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, nullptr)) return true;
    
    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.right, rect.bottom), dir);
    if(Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, nullptr)) return true;

    return false;
}