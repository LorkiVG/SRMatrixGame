// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#define FLYER_SPECIFICATIONS
#include "MatrixFlyer.hpp"
#include "MatrixMap.hpp"
#include "MatrixObjectBuilding.hpp"
#include "VectorObject.hpp"
#include "MatrixRobot.hpp"
#include "Interface/CInterface.h"
#include "ShadowStencil.hpp"
#include "MatrixFormGame.hpp"
#include "MatrixObjectCannon.hpp"

D3D_VB CMatrixFlyer::m_VB;
int    CMatrixFlyer::m_VB_ref;

CMatrixFlyer::CMatrixFlyer() : CMatrixMapStatic()
{
    m_Core->m_Type = OBJECT_TYPE_FLYER;

    //SetDirectionAngle(1);
    SetDirectionAngle(0);

    InitMaxHitpoint(5000);

    InitBuffers();
    ++m_VB_ref;

    m_ProgressBar.Modify(1000000, 0, PB_FLYER_WIDTH, 1);
}

CMatrixFlyer::~CMatrixFlyer()
{
    UnSelect();
    ReleaseMe();

    CSound::StopPlay(m_Sound);

    if(m_Modules)
    {
        for(int i = 0; i < m_ModulesCount; ++i) m_Modules[i].Release();
        HFree(m_Modules, Base::g_MatrixHeap);
    }

    if(m_Trajectory) HDelete(CTrajectory, m_Trajectory, Base::g_MatrixHeap);

    if(m_Streams)
    {
        for(int i = 0; i < m_StreamsCount; ++i) m_Streams[i].effect->Release();
        HFree(m_Streams, Base::g_MatrixHeap);
    }

    --m_VB_ref;

    if(m_VB_ref <= 0)
    {
        if(IS_VB(m_VB)) DESTROY_VB(m_VB);
    }
}

void CMatrixFlyer::MarkAllBuffersNoNeed(void)
{
    if(IS_VB(m_VB))
    {
        DESTROY_VB(m_VB);
    }
}

void CMatrixFlyer::InitBuffers(void)
{
    if(IS_VB(m_VB)) return;

    CREATE_VB(4 * sizeof(SVOVertex), VO_FVF, m_VB);
    if(!IS_VB(m_VB)) return;

    SVOVertex* v;
    LOCK_VB(m_VB, &v);

    float scale = 37.0f;

    v->v.x = -0.5f * scale; v->v.y = -0.5f * scale; v->v.z = 0.0f;
    v->tu = 0.0f; v->tv = 0.0f;
    v->n.x = 0.0f; v->n.y = 0.0f; v->n.z = 1.0f;
    ++v;

    v->v.x = +0.5f * scale; v->v.y = -0.5f * scale; v->v.z = 0.0f;
    v->tu = 1.0f; v->tv = 0.0f;
    v->n.x = 0.0f; v->n.y = 0.0f; v->n.z = 1.0f;
    ++v;

    v->v.x = +0.5f * scale; v->v.y = +0.5f * scale; v->v.z = 0.0f;
    v->tu = 1.0f; v->tv = 1.0f;
    v->n.x = 0.0f; v->n.y = 0.0f; v->n.z = 1.0f;
    ++v;

    v->v.x = -0.5f * scale; v->v.y = +0.5f * scale; v->v.z = 0.0f;
    v->tu = 0.0f; v->tv = 1.0f;
    v->n.x = 0.0f; v->n.y = 0.0f; v->n.z = 1.0f;

    UNLOCK_VB(m_VB);
}

void SMatrixFlyerUnit::Release(void)
{
DTRACE();

    if(m_Graph)
    {
        UnloadObject(m_Graph, Base::g_MatrixHeap);
        m_Graph = nullptr;
    }
	if(m_ShadowStencil)
    {
        HDelete(CVOShadowStencil, m_ShadowStencil, Base::g_MatrixHeap);
        m_ShadowStencil = nullptr;
    }

    if(m_Type == FLYER_UNIT_WEAPON || m_Type == FLYER_UNIT_WEAPON_HOLLOW)
    {
        if(m_Weapon.m_Weapon) 
        {
            m_Weapon.m_Weapon->Release();
            m_Weapon.m_Weapon = nullptr;
        }
    }
    else if(m_Type == FLYER_UNIT_PROPELLER)
    {
        m_Propeller.m_Tex = nullptr;
    }
}

bool SMatrixFlyerUnit::Tact(CMatrixFlyer* owner, float ms)
{
DTRACE();

    bool ret = false;
    if(m_Type == FLYER_UNIT_WEAPON || m_Type == FLYER_UNIT_WEAPON_HOLLOW)
    {
        if(m_Graph) ret = m_Graph->Tact(Float2Int(ms));
        if(m_Weapon.m_Weapon) m_Weapon.m_Weapon->Tact(ms);
    }
    else if(m_Type == FLYER_UNIT_PROPELLER)
    {
        //Постоянно крутим винт взлетевшего вертолёта
        if(!FLAG(owner->m_Flags, FLYER_IN_SPAWN))
        {
            m_Propeller.m_Angle += (ms * m_Propeller.m_AngleSpeedMax);
            if(m_Propeller.m_Angle > M_PI_MUL(1)) m_Propeller.m_Angle -= M_PI_MUL(2);
            if(m_Propeller.m_Angle < -M_PI_MUL(1)) m_Propeller.m_Angle += M_PI_MUL(2);
        }
        //Раскручиваем винт, после чего взлетаем (сразу после спавна и поднятия подъёмника)
        //Вывод объекта вертолёта на заданную высоту происходит не здесь, но это случается сразу же после снятия маркеров раскрутки винта
        else if(FLAG(owner->m_Flags, FLYER_IN_SPAWN_SPINUP))
        {
            m_Propeller.m_SpinupCountdown -= Float2Int(ms);

            //Раскручиваем винт, постепенно наращивая обороты
            if(m_Propeller.m_SpinupCountdown > 0)
            {
                float k = float(m_Propeller.m_SpinupCountdown) / FLYER_SPINUP_TIME;
                float sc = LERPFLOAT(k, m_Propeller.m_AngleSpeedMax, 0);

                m_Propeller.m_Angle += (ms * sc);
                if(m_Propeller.m_Angle > M_PI_MUL(1)) m_Propeller.m_Angle -= M_PI_MUL(2);
                if(m_Propeller.m_Angle < -M_PI_MUL(1)) m_Propeller.m_Angle += M_PI_MUL(2);
            }
            //Если винт уже достаточно раскручен (таймер раскрутки дошёл до нуля)
            else
            {
                m_Propeller.m_SpinupCountdown = 0;
                RESETFLAG(owner->m_Flags, FLYER_IN_SPAWN_SPINUP);
                RESETFLAG(owner->m_Flags, FLYER_IN_SPAWN);
                owner->m_CurrState = STATE_FLYER_READY_TO_ORDERS;
                owner->SetManualControlLocked(false);
                owner->m_Base->SetSpawningUnit(false);
                owner->m_Base->Close(); //Опускаем подъёмник базы
                //owner->m_TargetAlt = FLYER_ALT_EMPTY;
            }
        }

        if(!FLAG(owner->m_Flags, FLYER_IN_SPAWN) || FLAG(owner->m_Flags, FLYER_IN_SPAWN_SPINUP))
        {
            if(m_Graph)
            {
                if(!m_Graph->IsAnimEnd()) ret = m_Graph->Tact(Float2Int(ms));
            }
        }
    }
    else if(m_Graph) ret = m_Graph->Tact(Float2Int(ms));

    return ret;
}

float CMatrixFlyer::CalcFlyerZInPoint(float x, float y)
{
    float addz = g_MatrixMap->GetZInterpolatedObjRobots(x, y);
    float newz;
    if(addz > FLYER_ALT_EMPTY) newz = addz + FLYER_ALT_MIN;
    else newz = FLYER_ALT_EMPTY + FLYER_ALT_MIN;

    return newz;
}

void CMatrixFlyer::CalcBodyMatrix(void)
{
DTRACE();

    D3DXMATRIX m1, m2, m3;
    D3DXMatrixRotationX(&m1, m_Pitch);
    D3DXMatrixRotationY(&m2, m_Roll);

    memset(&m3, 0, sizeof(m3));
    m3._11 = m_AngleZCos; m3._12 = m_AngleZSin;
    m3._21 = -m_AngleZSin; m3._22 = m_AngleZCos;
    m3._33 = 1;
    m3._44 = 1;

    m_Core->m_Matrix = m1 * m2 * m3;
    *((D3DXVECTOR3*)(&m_Core->m_Matrix._41)) = GetPos();

    D3DXMatrixInverse(&m_Core->m_IMatrix, nullptr, &m_Core->m_Matrix);

    if(m_Modules)
    {
        ASSERT(m_Modules[0].m_Type == FLYER_UNIT_BODY);
        m_Modules[0].m_Matrix = m_Core->m_Matrix;
        m_Modules[0].m_IMatrix = m_Core->m_IMatrix;
    }

    SETFLAG(m_Flags, FLYER_BODY_MATRIX_DONE);
}

//Инициируем объекты моделей вертолётов
void CMatrixFlyer::CalcMatrix(void)
{
DTRACE();

    for(int i = 1; i < m_ModulesCount; ++i)
    {
        D3DXMATRIX rm;
        SMatrixFlyerUnit* u = m_Modules + i;
        if(u->m_Type == FLYER_UNIT_PROPELLER)
        {
            const D3DXMATRIX* m = m_Modules[0].m_Graph->GetMatrixById(u->m_Propeller.m_MatrixID); //Крепление винта

            BuildRotateMatrix(rm, D3DXVECTOR3(0, 0, 0), *(D3DXVECTOR3*)&m->_31, u->m_Propeller.m_Angle);
            *(D3DXVECTOR3*)&rm._41 = *(D3DXVECTOR3*)&m->_41;

            u->m_Matrix = rm * m_Core->m_Matrix;
            if(u->m_Propeller.m_Inversed)
            {
                D3DXMATRIX* mm = &u->m_Matrix;
                mm->_11 = -mm->_11;
                mm->_12 = -mm->_12;
                mm->_13 = -mm->_13;
            }
            else
            {
                BuildRotateMatrix(rm, D3DXVECTOR3(0, 0, 0), *(D3DXVECTOR3*)&m->_31, u->m_Propeller.m_Angle + M_PI_MUL(0.5));
                *(D3DXVECTOR3*)&rm._41 = *(D3DXVECTOR3*)&m->_41;
            }

            u->m_Propeller.m_Matrix = rm * m_Core->m_Matrix;
            /*if(u->m_Propeller.m_Inversed)
            {
                D3DMATRIX* mm = &u->m_Propeller.m_Matrix;
                mm->_11 = -mm->_11;
                mm->_12 = -mm->_12;
                mm->_13 = -mm->_13;
            }*/
            D3DXMatrixInverse(&u->m_IMatrix, nullptr, &u->m_Matrix);
        }
        else if(u->m_Type == FLYER_UNIT_ENGINE)
        {
            //engine
            D3DXVECTOR3 spos, sdir;

            D3DXMatrixRotationX(&rm, u->m_Engine.m_Angle);
            const D3DXMATRIX* m = m_Modules[0].m_Graph->GetMatrixById(u->m_Engine.m_MatrixID); //Крепление движка
            *(D3DXVECTOR3*)&rm._41 = *(D3DXVECTOR3*)&m->_41;
            u->m_Matrix = rm * m_Core->m_Matrix;
            if(u->m_Engine.m_Inversed)
            {
                D3DXMATRIX* mm = &u->m_Matrix;
                mm->_11 = -mm->_11;
                mm->_12 = -mm->_12;
                mm->_13 = -mm->_13;
            }
            D3DXMatrixInverse(&u->m_IMatrix, nullptr, &u->m_Matrix);
        }
        else if(u->m_Type == FLYER_UNIT_WEAPON)
        {
            //weapon 1
            const D3DXMATRIX* m = m_Modules[0].m_Graph->GetMatrixById(u->m_Weapon.m_MatrixID); //Крепление оружия
            D3DXMATRIX rm1;
            D3DXMatrixRotationX(&rm, u->m_Weapon.m_AngleX);
            D3DXMatrixRotationZ(&rm1, u->m_Weapon.m_AngleZ);
            *(D3DXVECTOR3*)&rm1._41 = *(D3DXVECTOR3*)&m->_41;
            u->m_Matrix = rm * rm1 * m_Core->m_Matrix;
            if(u->m_Weapon.m_Inversed)
            {
                D3DXMATRIX* mm = &u->m_Matrix;
                mm->_11 = -mm->_11;
                mm->_12 = -mm->_12;
                mm->_13 = -mm->_13;
            }
            D3DXMatrixInverse(&u->m_IMatrix, nullptr, &u->m_Matrix);

            const D3DXMATRIX* mw = u->m_Graph->GetMatrixByName(L"Fire");
            D3DXVECTOR3 wstart, wdir;
            D3DXVec3TransformCoord(&wstart, (D3DXVECTOR3*)&mw->_41, &u->m_Matrix);
            D3DXVec3TransformNormal(&wdir, (D3DXVECTOR3*)&mw->_21, &u->m_Matrix);

            u->m_Weapon.m_Weapon->Modify(wstart, wdir, D3DXVECTOR3(0, 0, 0));
        }
        else if(u->m_Type == FLYER_UNIT_WEAPON_HOLLOW)
        {
            //weapon 1
            const D3DXMATRIX* mw = m_Modules[u->m_Weapon.m_Module].m_Graph->GetMatrixById(u->m_Weapon.m_MatrixID);

            D3DXVECTOR3 wstart, wdir;
            D3DXVec3TransformCoord(&wstart, (D3DXVECTOR3*)&mw->_41, &m_Modules[u->m_Weapon.m_Module].m_Matrix);
            D3DXVec3TransformNormal(&wdir, (D3DXVECTOR3*)&mw->_21, &m_Modules[u->m_Weapon.m_Module].m_Matrix);

            u->m_Weapon.m_Weapon->Modify(wstart, wdir, D3DXVECTOR3(0, 0, 0));
        }
        else ERROR_S(L"Unknown flyer unit.");
    }

    //Fire streams
    //Выхлоп двигателей
    for(int i = 0; i < m_StreamsCount; ++i)
    {
        SFireStream* s = m_Streams + i;

        //Firestream
        const D3DXMATRIX* m = m_Modules[s->unit].m_Graph->GetMatrixById(s->matrix);
        D3DXVECTOR3 spos, sdir;
        sdir = -*(D3DXVECTOR3*)&m->_21;
        D3DXVec3TransformCoord(&spos, (D3DXVECTOR3*)&m->_41, &m_Modules[s->unit].m_Matrix);
        D3DXVec3TransformNormal(&sdir, &sdir, &m_Modules[s->unit].m_Matrix);
        s->effect->SetPos(spos, spos + sdir * m_StreamLen);
    }
}

//Запрашиваем ресурсы для сборки модели вертолёта
void CMatrixFlyer::GetResources(dword need)
{
    if(need & m_RChange & (MR_Graph))
    {
		m_RChange &= ~MR_Graph;

        //float hp = (float)g_Config.m_FlyerHitPoints[m_FlyerKind];
        //InitMaxHitpoint(hp);

        //Ресурсы вертолётам загружаются из обычной data.txt, а не из отдельного кэша (да и нахера он нужен вообще?)
        CBlockPar* flb = g_MatrixData->BlockPathGet(BLOCK_PATH_FLYERS_CONFIG);
        flb = flb->BlockGetNE(CWStr((int)m_FlyerKind + 1, g_CacheHeap).Get());

        if(flb)
        {
            m_StreamsCount = flb->BlockCount(L"Fire");

            int bcnt = flb->BlockCount();
            m_ModulesCount = bcnt - m_StreamsCount;
            if(m_ModulesCount > 0)
            {
                m_Modules = (SMatrixFlyerUnit*)HAllocClear(sizeof(SMatrixFlyerUnit) * m_ModulesCount, Base::g_MatrixHeap);
            }
            if(m_StreamsCount > 0)
            {
                m_Streams = (SFireStream*)HAlloc(sizeof(SFireStream) * m_StreamsCount, Base::g_MatrixHeap);
            }
            int findex = 0;

            int index = 0;
            int uindex = 0;
            CWStr model(g_CacheHeap), texture(g_CacheHeap);

            CWStr busy(g_CacheHeap);

            for(; bcnt > 0; ++index, --bcnt)
            {
                CBlockPar* bp = flb->BlockGet(index);
                CWStr bn(flb->BlockGetName(index), g_CacheHeap);
                if(bn == L"Fire")
                {
                    m_Streams[findex++].bp = bp;
                    continue;
                }
	            m_Modules[uindex].m_ShadowStencil = nullptr;

                if(bp->ParCount(L"Model") == 0 && bn == L"Weapon")
                {
                    m_Modules[uindex].m_Type = FLYER_UNIT_WEAPON_HOLLOW;

                    int w = WeapName2Weap(bp->ParGet(L"Weapon"));
                    if(w == WEAPON_NONE)
                    {
off_weapon:;
                        m_Modules[uindex].Release();
                        --m_ModulesCount;
                        continue;
                    }

                    m_Modules[uindex].m_Weapon.m_Weapon = (CMatrixEffectWeapon*)CMatrixEffect::CreateWeapon(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 1), (dword)this, nullptr, w);
                    m_Modules[uindex].m_Weapon.m_Weapon->SetOwner(this);
                    m_Modules[uindex].m_Weapon.m_Module = -1;

                    m_Modules[uindex].m_Weapon.m_MatrixID = bp->ParGet(L"Matrix").GetIntPar(1, L",");

                    CWStr unit(g_CacheHeap);
                    unit = bp->ParGet(L"Matrix").GetStrPar(0,L",");

                    EFlyerUnitType seekfor = FLYER_UNIT_BODY;
                    if(unit == L"Weapon") seekfor = FLYER_UNIT_WEAPON;
                    else if(unit == L"Engine") seekfor = FLYER_UNIT_ENGINE;
                    else if(unit == L"Propeller") seekfor = FLYER_UNIT_PROPELLER;

                    for(int k = 0; k < uindex; ++k)
                    {
                        if(m_Modules[k].m_Type == seekfor)
                        {
                            CWStr bt(L"|", g_CacheHeap);
                            bt += unit;
                            bt += L"_";
                            bt += m_Modules[uindex].m_Weapon.m_MatrixID;
                            bt += L"_";
                            bt += k;
                            bt += L"|";
                            
                            if(busy.Find(bt) < 0)
                            {
                                // found
                                m_Modules[uindex].m_Weapon.m_Module = k;
                                busy += bt;
                                break;
                            }
                        }
                    }
                    if(m_Modules[uindex].m_Weapon.m_Module < 0) goto off_weapon;

                    continue;
                }

                m_Modules[uindex].m_Matrix = g_MatrixMap->GetIdentityMatrix();
                m_Modules[uindex].m_IMatrix = g_MatrixMap->GetIdentityMatrix();

                model = bp->ParGet(L"Model");
                texture = bp->ParGet(L"Texture");

	            m_Modules[uindex].m_Graph = LoadObject(model.Get(), Base::g_MatrixHeap, true, texture.Get());

                if(bn == L"Body")
                {
                    m_Modules[uindex].m_Type = FLYER_UNIT_BODY;
                }
                else if(bn == L"Propeller")
                {
                    m_Modules[uindex].m_Type = FLYER_UNIT_PROPELLER;
                    
                    m_Modules[uindex].m_Propeller.m_Inversed = 0;
                    if(bp->ParCount(L"Inversed"))
                    {
                        m_Modules[uindex].m_Propeller.m_Inversed = bp->ParGet(L"Inversed").GetDword();
                    }
                    m_Modules[uindex].m_Propeller.m_SpinupCountdown = FLYER_SPINUP_TIME; // initially collapsed
                    m_Modules[uindex].m_Graph->SetAnimDefault(0);

                    m_Modules[uindex].m_Propeller.m_Angle = 0;
                    if(bp->ParCount(L"Angle"))
                    {
                        m_Modules[uindex].m_Propeller.m_Angle = (float)bp->ParGet(L"Angle").GetDouble();
                    }
                    m_Modules[uindex].m_Propeller.m_AngleSpeedMax = -0.025f;
                    if(bp->ParCount(L"DAngle"))
                    {
                        m_Modules[uindex].m_Propeller.m_AngleSpeedMax = (float)bp->ParGet(L"DAngle").GetDouble();
                    }
                    m_Modules[uindex].m_Propeller.m_MatrixID = bp->ParGet(L"Matrix").GetDword();

                    m_Modules[uindex].m_Propeller.m_Tex = nullptr;
                    if(bp->ParCount(L"TextureProp"))
                    {
                        m_Modules[uindex].m_Propeller.m_Tex = (CTextureManaged*)g_Cache->Get(cc_TextureManaged,bp->ParGet(L"TextureProp").Get());
                    }
                }
                else if(bn == L"Engine")
                {
                    if(m_EngineUnit < 0) m_EngineUnit = uindex;
                    m_Modules[uindex].m_Type = FLYER_UNIT_ENGINE;
                    m_Modules[uindex].m_Engine.m_MatrixID = bp->ParGet(L"Matrix").GetDword();
                    m_Modules[uindex].m_Engine.m_Inversed = bp->ParGet(L"Inversed").GetDword();
                }
                else if(bn == L"Weapon")
                {
                    m_Modules[uindex].m_Type = FLYER_UNIT_WEAPON;
                    m_Modules[uindex].m_Weapon.m_MatrixID = bp->ParGet(L"Matrix").GetDword();
                    m_Modules[uindex].m_Weapon.m_Inversed = bp->ParGet(L"Inversed").GetDword();

                    m_Modules[uindex].m_Weapon.m_HFOV = GRAD2RAD(float(bp->ParGet(L"RotationZ").GetDouble() * 0.5));
                    m_Modules[uindex].m_Weapon.m_UpAngle = GRAD2RAD((float)bp->ParGet(L"RotationX").GetDoublePar(0, L","));
                    m_Modules[uindex].m_Weapon.m_DownAngle = GRAD2RAD((float)bp->ParGet(L"RotationX").GetDoublePar(1, L","));

                    int w = WeapName2Weap(bp->ParGet(L"Weapon"));
                    if(w == WEAPON_NONE)
                    {
                        m_Modules[uindex].Release();
                        --m_ModulesCount;
                        continue;
                    }

                    m_Modules[uindex].m_Weapon.m_Weapon = (CMatrixEffectWeapon*)CMatrixEffect::CreateWeapon(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 1), (dword)this, nullptr, w);
                    m_Modules[uindex].m_Weapon.m_Weapon->SetOwner(this);
                }

                ++uindex;
            }

            busy = L"";

            for(int i = 0; i < m_StreamsCount; ++i)
            {
                CBlockPar* bp = m_Streams[i].bp;

                m_Streams[i].matrix = bp->ParGet(L"Matrix").GetIntPar(1, L",");

                CWStr unit(g_CacheHeap);
                unit = bp->ParGet(L"Matrix").GetStrPar(0, L",");

                EFlyerUnitType seekfor = FLYER_UNIT_BODY;
                if(unit == L"Weapon") seekfor = FLYER_UNIT_WEAPON;
                else if(unit == L"Engine") seekfor = FLYER_UNIT_ENGINE;
                else if(unit == L"Propeller") seekfor = FLYER_UNIT_PROPELLER;

                for(int k = 0; k < m_ModulesCount; ++k)
                {
                    if(m_Modules[k].m_Type == seekfor)
                    {
                        CWStr bt(L"|", g_CacheHeap);
                        bt += unit;
                        bt += L"_";
                        bt += m_Streams[i].matrix;
                        bt += L"_";
                        bt += k;
                        bt += L"|";
                        
                        if(busy.Find(bt) < 0)
                        {
                            // found
                            m_Streams[i].unit = k;
                            busy += bt;
                            break;
                        }
                    }
                }

                m_Streams[i].effect = CMatrixEffect::CreateFireStream(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(10, 0, 0));
            }
        }
        //else {}
    }
    if(need & m_RChange & (MR_Matrix))
    {
        m_RChange &= ~MR_Matrix;

        if(!FLAG(m_Flags, FLYER_BODY_MATRIX_DONE))
        {
            CalcBodyMatrix();
        }

        RESETFLAG(m_Flags, FLYER_BODY_MATRIX_DONE);

        CalcMatrix();
    }

	if(need & m_RChange & MR_ShadowStencil)
    {
		m_RChange &= ~MR_ShadowStencil;

        for(int i = 0; i < m_ModulesCount; ++i)
        {
            if(m_Modules[i].m_Type == FLYER_UNIT_WEAPON_HOLLOW) continue;
            m_Modules[i].m_Graph->BeforeDraw();
            if(g_Config.m_ShowStencilShadows)
            {
	            if(m_Modules[i].m_ShadowStencil == nullptr)
                {
                    m_Modules[i].m_ShadowStencil = HNew(Base::g_MatrixHeap) CVOShadowStencil(Base::g_MatrixHeap);
                }

                bool invert = (m_Modules[i].m_Type == FLYER_UNIT_ENGINE && m_Modules[i].m_Engine.m_Inversed) ||
                    (m_Modules[i].m_Type == FLYER_UNIT_PROPELLER && m_Modules[i].m_Propeller.m_Inversed) ||
                    (m_Modules[i].m_Type == FLYER_UNIT_WEAPON && m_Modules[i].m_Weapon.m_Inversed);

                //D3DXVECTOR3 light(0,0,-1);
                //D3DXVec3TransformNormal(&light, &light, &m_Modules[i].m_IMatrix);
                D3DXVECTOR3 light;
                D3DXVec3TransformNormal(&light, &g_MatrixMap->m_LightMain, &m_Modules[i].m_IMatrix);
                m_Modules[i].m_ShadowStencil->CreateShadowVO(*(m_Modules[i].m_Graph->VO()), m_Modules[i].m_Graph->GetVOFrame(), light, m_Pos.z - g_MatrixMap->m_GroundZBase, invert);
                m_Modules[i].m_ShadowStencil->BeforeRender();
            }
        }
    }

    m_Core->m_TerainColor = 0xFFFFFFFF;
}

void CMatrixFlyer::BeforeDraw(void)
{
DTRACE();

    //SetTarget(D3DXVECTOR2(0, 3000));
    InitBuffers();

    GetResources(MR_Graph | MR_Matrix | MR_ShadowStencil);

    //JoinToGroup();

    if(IsCarryingRobot())
    {
        GetCarryingRobot()->BeforeDraw();
    }

    for(int i = 0; i < m_StreamsCount; ++i)
    {
        m_Streams[i].effect->BeforeDraw();
    }
    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Modules[i].m_Type == FLYER_UNIT_PROPELLER)
        {
            if(m_Modules[i].m_Propeller.m_Tex) m_Modules[i].m_Propeller.m_Tex->Preload();
        }
    }

    if(m_ShowHitpointTime > 0 && m_HitPoint > 0)
    {
        D3DXVECTOR2 p = g_MatrixMap->m_Camera.Project(GetPos(), g_MatrixMap->GetIdentityMatrix());
        m_ProgressBar.Modify(p.x-(PB_FLYER_WIDTH*0.5f), p.y-FLYER_RADIUS*2, m_HitPoint * m_MaxHitPointInversed);
    }
}

void CMatrixFlyer::SetTarget(const D3DXVECTOR2& tgt)
{
    if(g_MatrixMap->GetPlayerSide()->IsArcadeMode())
    {
        m_TgtUpdateCount = 10;
        m_Target = tgt;
        RESETFLAG(m_Flags, MF_TARGETMOVE);
        SETFLAG(m_Flags, MF_TARGETFIRE);
        RESETFLAG(m_Flags, FLYER_MANUAL);
    }
    else
    {
        m_Target = tgt;
        float newz = CalcFlyerZInPoint(tgt.x, tgt.y);
        CalcTrajectory(D3DXVECTOR3(tgt.x, tgt.y, newz));
    }
}

void CMatrixFlyer::Tact(int ms)
{
    DTRACE();

    float fms = float(ms);
	for(int i = 0; i < m_ModulesCount; ++i)
    {
		if(m_Modules[i].Tact(this, fms))
        {
			RChange(MR_ShadowStencil);
			//if(m_ShadowType == SHADOW_PROJ_DYNAMIC) RChange(MR_ShadowProjTex);
		}
	}
}

struct SFlyerTactData
{
    D3DXVECTOR2 hdir;
    D3DXVECTOR2 tdir;

    D3DXVECTOR3 reaction;

    float ms;

    float pow998;
    float pow999;

    float mul;

    float tlen;

    float speedn;
    float speedf;
};

void CMatrixFlyer::LogicTactArcade(SFlyerTactData& td)
{
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD_ALT]) & 0x8000) == 0x8000)
    {
        MoveForward();
        SETFLAG(m_Flags, FLYER_MANUAL);
    }
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD_ALT]) & 0x8000) == 0x8000)
    {
        MoveBackward();
        SETFLAG(m_Flags, FLYER_MANUAL);
    }
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_STRAFE_LEFT]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_STRAFE_LEFT_ALT]) & 0x8000) == 0x8000)
    {
        MoveLeft();
        SETFLAG(m_Flags, FLYER_MANUAL);
    }
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_STRAFE_RIGHT]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_STRAFE_RIGHT_ALT]) & 0x8000) == 0x8000)
    {
        MoveRight();
        SETFLAG(m_Flags, FLYER_MANUAL);
    }
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_UP]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_UP_ALT]) & 0x8000) == 0x8000)
    {
        MoveUp(FlyerSpecifications[m_FlyerKind]);
        //SETFLAG(m_Flags, FLYER_MANUAL);
    }
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_DOWN]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_DOWN_ALT]) & 0x8000) == 0x8000)
    {
        MoveDown(FlyerSpecifications[m_FlyerKind]);
        //SETFLAG(m_Flags, FLYER_MANUAL);
    }

    if(FLAG(m_Flags, FLYER_MANUAL))
    {
        m_DAngle = 0;
        SetTarget(D3DXVECTOR2(m_Pos.x, m_Pos.y));
        SETFLAG(m_Flags, FLYER_MANUAL);
    }

    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_FIRE]) & 0x8000) == 0x8000 && g_IFaceList->m_InFocus != INTERFACE)
    {
        FireBegin();
    }
    else
    {
        FireEnd();
    }

    SetFireTarget(g_MatrixMap->m_TraceStopPos);

    // rotate flyer if its needed
    CPoint mp = g_MatrixMap->m_Cursor.GetPos();
    bool can_rot = false;
    if(mp.x >= 0 && mp.x < g_ScreenX && mp.y >= 0 && mp.y < g_ScreenY)
    {
        if(mp.x < MOUSE_BORDER || mp.x > (g_ScreenX - MOUSE_BORDER) || mp.y < MOUSE_BORDER || mp.y > (g_ScreenY - MOUSE_BORDER)) can_rot = true;
    }

    //Задаём скорость вращения вертолёта в сторону позиции курсора, если нужно, и если курсор не наведён на интерфейс
    //Непосредственно сам поворот задаётся по коду ниже
    if(g_IFaceList->m_InFocus != INTERFACE || can_rot)
    {
        float yfactor = float(g_MatrixMap->m_Cursor.GetPosY()) * g_MatrixMap->m_Camera.GetResYInversed();
        float rarea = (g_ScreenX - g_ScreenX * LERPFLOAT(yfactor, FLYER_MOUSE_DEAD_AREA_TOP, FLYER_MOUSE_DEAD_AREA_BOTTOM)) * 0.5f;
        float swtest = 1.0f / rarea;

        float k;
        k = (rarea - g_MatrixMap->m_Cursor.GetPosX()) * swtest;

        SPlane hp;
        hp.BuildFromPointNormal(hp, GetPos(), D3DXVECTOR3(0, 0, 1));

        float t = 0;
        D3DXVECTOR2 pp = g_MatrixMap->m_Camera.Project(GetPos(), g_MatrixMap->GetIdentityMatrix());

        if(float(g_MatrixMap->m_Cursor.GetPosY()) > pp.y)
        {
            t = (float(g_MatrixMap->m_Cursor.GetPosY()) - pp.y) / float(g_ScreenY - pp.y);
        }

        if(k > 0)
        {
            m_RotZSpeed -= (0.0023f * k * (1 + t) * td.ms);
            if(m_RotZSpeed < -1.2f)
            {
                //m_RotZSpeed *= td.pow998;
                m_RotZSpeed = -1.2f;
            }
            SETFLAG(m_Flags, FLYER_MANUAL);
        }
        else
        {
            k = (g_MatrixMap->m_Cursor.GetPosX() - (g_ScreenX - rarea)) * swtest;

            if(k > 0)
            {
                m_RotZSpeed += (0.0023f * k * (1 + t) * td.ms);
                if(m_RotZSpeed > 1.2f)
                {
                    //m_RotZSpeed *= td.pow998;
                    m_RotZSpeed = 1.2f;
                }

                SETFLAG(m_Flags, FLYER_MANUAL);
            }
        }
    }

    --m_TgtUpdateCount;


    m_StrafeSpeed *= td.pow998;

    if(FLAG(m_Flags, FLYER_ACTION_MOVE_LEFT))
    {
        m_StrafeSpeed -= (float)fabs(m_StrafeSpeed - FlyerSpecifications[m_FlyerKind].strafe_speed) * (1.0f - td.pow998);
        if(m_StrafeSpeed < -FlyerSpecifications[m_FlyerKind].strafe_speed) m_StrafeSpeed = -FlyerSpecifications[m_FlyerKind].strafe_speed;
    }
    if(FLAG(m_Flags, FLYER_ACTION_MOVE_RIGHT))
    {
        m_StrafeSpeed += (float)fabs(m_StrafeSpeed + FlyerSpecifications[m_FlyerKind].strafe_speed) * (1.0f - td.pow998);
        if(m_StrafeSpeed > FlyerSpecifications[m_FlyerKind].strafe_speed) m_StrafeSpeed = FlyerSpecifications[m_FlyerKind].strafe_speed;
    }

    //CDText::T("strife", CStr(m_StrafeSpeed));
    if(FLAG(m_Flags, FLYER_ACTION_MOVE_FORWARD))
    {
        //SetTarget(D3DXVECTOR2(m_Pos.x, m_Pos.y) + hdir * 1000);

        float speedfi = td.speedn * 0.985f + 0.015f;

        if(!FLAG(m_Flags, FLYER_ACTION_ROT_LEFT) && !FLAG(m_Flags, FLYER_ACTION_ROT_RIGHT))
        {
            m_MoveSpeed += speedfi * (FlyerSpecifications[m_FlyerKind].forward_speed / 0.4f) *  0.0012f * td.ms;
        }

        if(m_MoveSpeed > FlyerSpecifications[m_FlyerKind].forward_speed) m_MoveSpeed = FlyerSpecifications[m_FlyerKind].forward_speed;

        m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_MOVE - m_Pitch);
        m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_move);

        m_StreamLen = LERPFLOAT(td.speedn, 5, 15);
    }
    if(FLAG(m_Flags, FLYER_ACTION_MOVE_BACKWARD))
    {
        float speedfi = td.speedn * 0.985f + 0.015f;

        m_MoveSpeed -= speedfi * 0.0010f * td.ms;
        if(m_MoveSpeed < -FlyerSpecifications[m_FlyerKind].reverse_speed) m_MoveSpeed = -FlyerSpecifications[m_FlyerKind].reverse_speed;

        m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_MOVE - m_Pitch);
        m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_move);
    }

    //Вращаем вертолёт (по оси рысканья) в режиме ручного управления
    if(FLAG(m_Flags, FLYER_MANUAL))
    {
        //Устаревшие флаги, больше не выставляются
        if(FLAG(m_Flags, FLYER_ACTION_ROT_LEFT))
        {
            m_RotZSpeed -= (0.0019f * td.ms);
            if(m_RotZSpeed < -0.9f)
            {
                m_RotZSpeed *= td.pow998;
            }
        }
        if(FLAG(m_Flags, FLYER_ACTION_ROT_RIGHT))
        {
            m_RotZSpeed += (0.0019f * td.ms);
            if(m_RotZSpeed > 0.9f)
            {
                m_RotZSpeed *= td.pow998;
            }
        }

        //Задаём здесь угол крена во время вращения на скорости
        float roll_ang = -m_RotZSpeed * td.speedn;
        if(abs(roll_ang) > FlyerSpecifications[m_FlyerKind].max_roll_angle)
        {
            if(roll_ang > 0) roll_ang = FlyerSpecifications[m_FlyerKind].max_roll_angle;
            else roll_ang = -FlyerSpecifications[m_FlyerKind].max_roll_angle;
        }
        m_TargetRollAngle = roll_ang;

        float rspeedf = 1.0f - td.speedn * 0.5f;

        SetDirectionAngle(GetDirectionAngle() + rspeedf * td.mul * m_RotZSpeed);
        m_RotZSpeed *= td.pow998;
    }

    float dp = 0.3f * (float)sqrt(fabs(m_StrafeSpeed / FlyerSpecifications[m_FlyerKind].strafe_speed));
    COPY_SIGN_FLOAT(dp, m_StrafeSpeed);
    m_TargetRollAngle -= dp;

    if(td.speedn > 0.4f && (m_TgtUpdateCount <= 0)) SETFLAG(m_Flags, MF_TARGETMOVE);

    if(td.tlen > FLYER_TARGET_MATCH_RADIUS && !FLAG(m_Flags, MF_TARGETMOVE))
    {
        // move target
        //D3DXVec2Normalize(&hdir, &hdir);
        td.tdir *= (1.0f / td.tlen);
        float cc = D3DXVec2Dot(&td.hdir, &td.tdir);

        if(!FLAG(m_Flags, FLYER_MANUAL))
        {
            float tang = (float)atan2(-td.tdir.x, td.tdir.y);
            m_DAngle = (float)AngleDist(GetDirectionAngle(), tang);
            
            //float matchf = 0.5f - cc * 0.5f;

            m_RotZSpeed += (0.0012f * td.ms);
            if(m_RotZSpeed > fabs(m_DAngle))
            {
                m_RotZSpeed *= td.pow998;
            }

            SetDirectionAngle(GetDirectionAngle() + td.speedf * m_DAngle * td.mul * m_RotZSpeed);
        }

        float speedfi = td.speedn * 0.985f + 0.015f;

        if(!FLAG(m_Flags, FLYER_ACTION_MOVE_BACKWARD))
        {
            m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_MOVE - m_Pitch);
            m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_move);
        }
        m_StreamLen = LERPFLOAT(td.speedn,5, 15);

        if(cc > 0.5f)
        {
            float k = td.tlen / (FLYER_TARGET_MATCH_RADIUS + FLYER_TARGET_MATCH_RADIUS);
            if(k > 1.0f) k = 1.0f;

            m_MoveSpeed += (k * speedfi * 0.0012f * td.ms) * cc;
            if(m_MoveSpeed > FlyerSpecifications[m_FlyerKind].forward_speed) m_MoveSpeed = FlyerSpecifications[m_FlyerKind].forward_speed;
        }
    }
    else if(!FLAG(m_Flags, FLYER_ACTION_MOVE_FORWARD))
    {
        SETFLAG(m_Flags, MF_TARGETMOVE);

        float mul2 = (float)pow(0.999, double(td.ms));
        m_DAngle *= mul2;
        SetDirectionAngle(GetDirectionAngle() + td.speedf * m_DAngle * td.mul * m_RotZSpeed);
        m_RotZSpeed *= mul2;
        m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_BREAK);
        m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_break);
        m_StreamLen = LERPFLOAT(td.speedn, 5, 15);
    }

    if(!FLAG(m_Flags, FLYER_MANUAL))
    {
        m_TargetRollAngle = -m_DAngle * td.speedn;
    }

    //Обновляем текущие координаты вертолёта
    if(!FLAG(m_Flags, FLYER_IN_SPAWN))
    {
        *(D3DXVECTOR2*)&m_Pos += ((D3DXVECTOR2(-m_AngleZCos, -m_AngleZSin) * m_StrafeSpeed) + (D3DXVECTOR2(-m_AngleZSin, m_AngleZCos) * m_MoveSpeed)) * td.ms;
        RChange(MR_Matrix);
    }

    if(!FLAG(m_Flags, FLYER_ACTION_MOVE_FORWARD | FLYER_ACTION_MOVE_BACKWARD))
    {
        m_MoveSpeed *= td.pow999;

        //if((float)fabs(m_MoveSpeed) < FLYER_MIN_SPEED)
        //{
        //    m_MoveSpeed = 0;
        //}
    }

    JoinToGroup();
}

static bool DoCollsion(const D3DXVECTOR3& pos, CMatrixMapStatic* ms, dword user)
{
    SFlyerTactData* td = (SFlyerTactData*)user;

    //static float jj = 1;
    //if(g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_INSERT) { jj += 0.01f; }
    //if(g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_DELETE)  { jj -= 0.01f; }
    //CDText::T("JJ", CStr(jj));

    D3DXVECTOR3 a;
    D3DXVECTOR3 temp = pos - ms->GetGeoCenter();
    D3DXVec3Normalize(&a, &temp);

    td->reaction += a;

    return true;
}

void CMatrixFlyer::CalcCollisionDisplace(SFlyerTactData& td)
{
    td.reaction = D3DXVECTOR3(0, 0, 0);

    if(g_MatrixMap->FindObjects(GetPos(), FLYER_RADIUS * 2, 1.0f, TRACE_FLYER, this, DoCollsion, (dword)&td))
    {
        D3DXVec3Normalize(&td.reaction, &td.reaction);
        m_Pos += td.reaction * td.ms * 0.01f;
    }
    RChange(MR_Matrix);
}

void CMatrixFlyer::LogicTactStrategy(SFlyerTactData& td)
{
    m_StrafeSpeed *= td.pow998;
    if(m_Trajectory == nullptr)
    {
        //CalcCollisionDisplace(td);

        // do nothing

        m_MoveSpeed *= td.pow998;
        *(D3DXVECTOR2 *)&m_Pos += D3DXVECTOR2(-m_AngleZSin, m_AngleZCos) * m_MoveSpeed * td.ms;

        m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_BREAK);
        m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_break);
        m_StreamLen = LERPFLOAT(td.speedn, 5, 15);
    }
    else
    {
        ProceedTrajectory(td);
        JoinToGroup();
    }
    
    /*
    CMatrixSideUnit* pls = g_MatrixMap->GetPlayerSide();
    if(pls->GetArcadedObject() != this)
    {
        if(pls->m_CurGroup && pls->m_CurGroup->FindObject((CMatrixMapStatic*)this))
        {
            if(pls->m_CurGroup->m_Tactics && pls->m_CurGroup->m_Tactics->GetTarget())
            {
                CMatrixMapStatic* ms = pls->m_CurGroup->m_Tactics->GetTarget();
                D3DXVECTOR3 pos = GetPos() - ms->GetGeoCenter();
                float length = D3DXVec3Length(&pos);
                D3DXVec3Normalize(&pos, &pos);

                pos *= (30 * GLOBAL_SCALE_MOVE);

                pos += ms->GetGeoCenter();
                if((TruncFloat(pos.x) - TruncFloat(GetTarget().x) > 3) || (TruncFloat(pos.y) - TruncFloat(GetTarget().y) > 3))
                {
                    SetTarget(D3DXVECTOR2(pos.x, pos.y));
                }

                if(ms->IsBuilding())
                {}
                else if(ms->IsUnit())
                {
                    SetFireTarget(ms->GetGeoCenter());
                    if(length <= 50 * GLOBAL_SCALE_MOVE) FireBegin();
                    else FireEnd();
                }
            }
            else
            {
                SetFireTarget(GetPos(100) - GetPos());
                FireEnd();
            }
        }
    }
    */
}

//Вертолёт принимает приказ
//В данный момент используется только для вертолётов доставщиков
void CMatrixFlyer::ApplyOrder(
    const D3DXVECTOR2& pos,
    int side,
    EFlyerOrder order,
    float ang,
    int place,
    const CPoint& bpos,
    int robot_template
)
{
    RESETFLAG(m_Flags, FLYER_IN_SPAWN);
    SetDeliveryCopter(true);
    m_CurrState = STATE_FLYER_IS_BUSY;

    m_Side = side;
    m_TrajectoryTargetAngle = ang;

    CBlockPar* bp = nullptr;

    //Если вертолёт имеет приказ доставить робота (то есть если это вертолёт, доставляющий подкрепление)
    if(order == FO_GIVE_BOT)
    {
        bp = g_MatrixData->BlockPathGet(BLOCK_PATH_REINFORCEMENTS);

        float dist = bp->ParGet(L"FlightDistance").GetDouble() + FSRND(150);
        float height = bp->ParGet(L"FlightHeightAboveSeaLevel").GetDouble();

        float z = g_MatrixMap->GetZ(pos.x, pos.y);
        D3DXVECTOR3 p(pos.x, pos.y, z + bp->ParGet(L"RobotDropHeight").GetDouble());

        D3DXVECTOR3 dir;
        dir.x = TableSin(ang);
        dir.y = TableCos(ang);
        dir.z = 0;

        D3DXVECTOR3 pts[4];

        pts[0] = p - dir * dist + D3DXVECTOR3(0, 0, height);
        pts[1] = p - dir * 100;
        pts[2] = p + dir * 100;
        pts[3] = p + dir * dist * 2 + D3DXVECTOR3(0, 0, height * 3);

        m_Pos = pts[0];

        m_Trajectory = HNew(Base::g_MatrixHeap) CTrajectory(Base::g_MatrixHeap);
        m_Trajectory->Init1(pts, 4);

        SRobotTemplate bot = SRobotTemplate::m_AIRobotTypeList[REINFORCEMENTS_TEMPLATES][robot_template];
        CMatrixRobotAI* r = bot.CreateRobotByTemplate(m_Pos, PLAYER_SIDE);
        g_MatrixMap->AddObject(r, true);
        r->CreateTextures();
        r->Carry(this, true);

        SObjectCore* core = r->GetCore(DEBUG_CALL_INFO);
        *(D3DXVECTOR3*)&core->m_Matrix._41 = m_Pos;

        r->m_CalcBoundsLastTime = g_MatrixMap->GetTime()-10000;
        D3DXVECTOR3 minv, maxv;
        r->RChange(MR_Matrix);
        r->CalcBounds(minv, maxv);
        core->m_GeoCenter = (minv + maxv) * 0.5f;
        D3DXVECTOR3 temp = minv - maxv;
        core->m_Radius = D3DXVec3Length(&temp);

        core->Release();

        SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
        g_MatrixMap->GetPlayerSide()->PGOrderAttack(g_MatrixMap->GetPlayerSide()->RobotToLogicGroup(r),g_MatrixMap->m_RoadNetwork.m_Place[place].m_Pos /*bpos*/, nullptr);
        RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);

        r->GetEnv()->m_Place = place;
    }

    m_TrajectoryPos = 0;
    m_TrajectoryLen = m_Trajectory->CalcLength();
    m_TrajectoryLenRev = 1.0f / m_TrajectoryLen;

    //m_Target = m_Pos;
    //SETFLAG(m_Flags, MF_TARGETMOVE);
    SetDirectionAngle(0);

    GetResources(MR_Graph | MR_Matrix);

    JoinToGroup();
}

bool CMatrixFlyer::LogicTactOrder(SFlyerTactData& td)
{
    //Обработчик логики для управляемых вертолётов
    if(!IsDeliveryCopter())
    {
        return false;
    }
    //Обработчик логики для неуправляемых вертолётов из группы доставки
    else
    {
        ProceedTrajectory(td);

        if(m_Trajectory == nullptr)
        {
            g_MatrixMap->StaticDelete(this);
            return true;
        }

        D3DXVECTOR3 vmin, vmax;
        if(this->CalcBounds(vmin, vmax)) return false;

        m_Core->m_GeoCenter = (vmin + vmax) * 0.5f;
        D3DXVECTOR3 temp = vmax - vmin;
        m_Core->m_Radius = D3DXVec3Length(&temp) * 0.5f;
        m_Core->m_TerainColor = 0xFFFFFFFF;

        if(m_TrajectoryPos > 0.5f && IsCarryingRobot())
        {
            GetCarryingRobot()->Carry(nullptr);
        }

        return false;
    }
}

void CMatrixFlyer::LogicTact(int tact)
{
DTRACE();

    if(!g_MatrixMap->GetPlayerSide()->FindObjectInSelection(this)) UnSelect();

    SFlyerTactData td;
    td.ms = float(tact);

    // pb
    m_ShowHitpointTime -= tact;
    if(m_ShowHitpointTime <= 0)
    {
        m_ShowHitpointTime = 0;
    }
    m_ProgressBar.Modify(100000.0f, 0);

    if(FLAG(m_Flags, FLYER_IN_SPAWN))
    {
        m_Pos.z = m_Base->GetFloorZ();
        //Подъёмник базы поднялся, перед взлётом вертолёта начинаем раскручивать винт
        if(m_Base->State() == BASE_OPENED && !FLAG(m_Flags, FLYER_IN_SPAWN_SPINUP))
        {
            SETFLAG(m_Flags, FLYER_IN_SPAWN_SPINUP);
            m_CurrState = STATE_FLYER_BASE_TAKE_OFF;
            m_Sound = CSound::Play(m_Sound, S_FLYER_PROPELLER_START, m_Pos);
        }

        RChange(MR_Matrix);
    }
    else m_Sound = CSound::Play(m_Sound, S_FLYER_PROPELLER_CONTINUE, m_Pos);

    td.hdir = D3DXVECTOR2(-m_AngleZSin, m_AngleZCos);
    td.tdir = D3DXVECTOR2(m_Target - D3DXVECTOR2(m_Pos.x, m_Pos.y));

    td.pow998 = pow(0.998, double(td.ms));
    td.pow999 = pow(0.999, double(td.ms));

    td.mul = 1.0f - td.pow998;

    td.tlen = D3DXVec2Length(&td.tdir);

    td.speedn = fabs(m_MoveSpeed) / FlyerSpecifications[m_FlyerKind].forward_speed;
    td.speedf = 1.1f - td.speedn;

    if(IsAblaze())
    {
        int ablaze_effect_num = IsAblaze();

        if(m_Ablaze.effect == nullptr) CMatrixEffect::CreateShleif(&m_Ablaze);

        if(m_Ablaze.effect != nullptr)
        {
            //((CMatrixEffectShleif*)m_Ablaze.effect)->SetTTL(1000);

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

                if(cnt > 0)
                {
                    ((CMatrixEffectShleif*)m_Ablaze.effect)->AddFire(pos + dir * (t + 2), 100, 1500, 30, 2.5f, false, 1.0f / 35.0f, g_Config.m_WeaponsConsts[ablaze_effect_num].close_color_rgb, g_Config.m_WeaponsConsts[ablaze_effect_num].far_color_rgb);
                }

                //Если робот был уничтожен огнём
                if(TakingDamage(ablaze_effect_num, pos, dir, m_LastDelayDamageSide, nullptr)) return;
            }
        }
    }

    //Вертолётам подкрепления лишняя логика ни к чему
    if(!IsDeliveryCopter())
    {
        MoveSelection();

        if(g_MatrixMap->GetPlayerSide()->GetArcadedObject() == this)
        {
            LogicTactArcade(td);
        }
        else
        {
            LogicTactStrategy(td);
        }
    }

    if(LogicTactOrder(td)) return;

    if(m_EngineUnit >= 0)
    {
        float ea = m_Modules[m_EngineUnit].m_Engine.m_Angle;
        float dang = AngleDist(ea, m_TargetEngineAngle);
        ea += dang * td.mul;
        for(int ii = m_EngineUnit; ii < m_ModulesCount; ++ii)
        {
            m_Modules[ii].m_Engine.m_Angle = ea;
        }
    }

    float dang = AngleDist(m_Pitch, m_TargetPitchAngle);
    m_Pitch += dang * td.mul;

    dang = AngleDist(m_Roll, m_TargetRollAngle);
    m_Roll += dang * td.mul;

    //Здесь происходит корректировка высоты полёта
    if(m_CurrState > STATE_FLYER_IN_SPAWN)//!FLAG(m_Flags, FLYER_IN_SPAWN))
    {
        //Сюда необходимо добавить коллизии по Z

        float newz = CalcFlyerZInPoint(m_Pos.x, m_Pos.y);
        m_Pos.z += (newz - m_Pos.z) * td.mul + m_AltitudeCorrection;
        RChange(MR_Matrix);
    }

#ifdef _DEBUG1
    for(float yy = m_Pos.y - 100; yy < m_Pos.y + 100; yy += 10)
    {
        for(float xx = m_Pos.x - 100; xx < m_Pos.x + 100; xx += 10)
        {
            float z = g_MatrixMap->GetZInterpolated(xx, yy);
            float z1 = g_MatrixMap->GetZInterpolated(xx + 10, yy);
            float z2 = g_MatrixMap->GetZInterpolated(xx + 10, yy + 10);
            D3DXVECTOR3 p0(xx, yy, z);
            D3DXVECTOR3 p1(xx + 10, yy, z1);
            D3DXVECTOR3 p2(xx + 10, yy + 10, z2);
            CHelper::Create(1, 0)->Line(p0, p1);
            CHelper::Create(1, 0)->Line(p1, p2);
        }
    }
#endif

    if(IsCarryingRobot())
    {
        GetCarryingRobot()->Tact(tact);
    }

    CalcBodyMatrix();

    if(!FLAG(m_Flags, MF_TARGETFIRE)) // modify weapon direction only if it is not match
    {
        float mul = 1.0f - (float)pow(0.995,double(td.ms));

        // weapon calcs

        int index = 1;
        for(; index < m_ModulesCount; ++index)
        {
            SMatrixFlyerUnit* w = m_Modules + index;
            if(w->m_Type != FLYER_UNIT_WEAPON) continue;

            // current matrix
            const D3DXMATRIX* mw = w->m_Graph->GetMatrixByName(L"Fire");
            const D3DXMATRIX* m = m_Modules[0].m_Graph->GetMatrixById(w->m_Weapon.m_MatrixID);  // крепление оружия
            D3DXMATRIX rm0, rm1;
            D3DXMatrixRotationX(&rm0, w->m_Weapon.m_AngleX);
            D3DXMatrixRotationZ(&rm1, w->m_Weapon.m_AngleZ);
            *(D3DXVECTOR3*)&rm1._41 = *(D3DXVECTOR3*)&m->_41;
            rm0 = rm0 * rm1 * m_Core->m_Matrix;

            // rm0 - current weapon matrix

            D3DXVECTOR3 wstart, wdir;
            D3DXVec3TransformCoord(&wstart, (D3DXVECTOR3*)&mw->_41, &rm0);
            D3DXVec3TransformNormal(&wdir, (D3DXVECTOR3*)&mw->_21, &rm0);


            //D3DXVECTOR3 tgtdir = (m_FireTarget - *(D3DXVECTOR3*)&rm0._41);
            D3DXVECTOR3 tgtdir = (m_FireTarget - wstart);

            //CHelper::Create(1,0)->Line(wstart, wstart + wdir * 1000);

            float curwa = (float)AngleNorm(atan2(-wdir.x, wdir.y) - m_AngleZ);
            float tgtwa = (float)AngleNorm(atan2(-tgtdir.x, tgtdir.y) - m_AngleZ);

            if(tgtwa < -(w->m_Weapon.m_HFOV)) tgtwa = -(w->m_Weapon.m_HFOV);
            else if(tgtwa > (w->m_Weapon.m_HFOV)) tgtwa = (w->m_Weapon.m_HFOV);

            float da = (float)AngleDist(curwa, tgtwa);

            w->m_Weapon.m_AngleZ += da * mul;

            curwa = (float)AngleNorm(atan2(wdir.z, D3DXVec2Length((D3DXVECTOR2*)&wdir)) - m_Pitch);
            tgtwa = (float)AngleNorm(atan2(tgtdir.z, D3DXVec2Length((D3DXVECTOR2*)&tgtdir)) - m_Pitch);

            if(tgtwa < -w->m_Weapon.m_DownAngle) tgtwa = -(w->m_Weapon.m_DownAngle);
            else if(tgtwa > w->m_Weapon.m_UpAngle) tgtwa = w->m_Weapon.m_UpAngle;

            //CDText::T("ca", CStr(tgtwa));

            da = (float)AngleDist(curwa, tgtwa);

            w->m_Weapon.m_AngleX += da * mul;

            //m_Module[FLYER_UNIT_WEAPON1].m_AngleX = M_PI_MUL(-0.3);
        }

        SETFLAG(m_Flags, MF_TARGETFIRE);
    }

    if(td.speedn > FLYER_MAX_FIRE_SPEED)
    {
        FireEnd();
    }

    // TODO : check weapon tact! may be it is no need
    //m_Module[FLYER_UNIT_WEAPON1].m_Weapon->Tact(td.ms);

    RESETFLAG(m_Flags, FLYER_ACTION_MOVE_FORWARD);
    RESETFLAG(m_Flags, FLYER_ACTION_MOVE_BACKWARD);
    RESETFLAG(m_Flags, FLYER_ACTION_MOVE_LEFT);
    RESETFLAG(m_Flags, FLYER_ACTION_MOVE_RIGHT);
    RESETFLAG(m_Flags, FLYER_ACTION_ROT_LEFT);
    RESETFLAG(m_Flags, FLYER_ACTION_ROT_RIGHT);

    RChange(MR_Matrix|MR_ShadowStencil);
}

void CMatrixFlyer::FireBegin(void)
{
DTRACE();

    float speedn = m_MoveSpeed / FlyerSpecifications[m_FlyerKind].forward_speed;

    if(speedn > FLYER_MAX_FIRE_SPEED)
    {
        FireEnd();
        return;
    }

    int index = 1;
    for(; index < m_ModulesCount; ++index)
    {
        SMatrixFlyerUnit* w = m_Modules + index;
        if(w->m_Type != FLYER_UNIT_WEAPON && (w->m_Type != FLYER_UNIT_WEAPON_HOLLOW)) continue;
        if(!w->m_Weapon.m_Weapon->IsFire())
        {
            if(w->m_Weapon.m_Weapon->GetWeaponNum() == WEAPON_MORTAR)
            {
                D3DXVECTOR3 speed(-m_MoveSpeed * m_AngleZSin, m_MoveSpeed * m_AngleZCos, 0);
                w->m_Weapon.m_Weapon->FireBegin(speed * 10 + m_Pos, this);
            }
            else
            {
                D3DXVECTOR3 speed(-m_MoveSpeed * m_AngleZSin, m_MoveSpeed * m_AngleZCos, 0);
                w->m_Weapon.m_Weapon->FireBegin(speed, this);
            }
        }
    }
}

void CMatrixFlyer::FireEnd(void)
{
    DTRACE();

    int index = 1;
    for(; index < m_ModulesCount; ++index)
    {
        SMatrixFlyerUnit* w = m_Modules + index;
        if(w->m_Type != FLYER_UNIT_WEAPON && (w->m_Type != FLYER_UNIT_WEAPON_HOLLOW)) continue;
        if(w->m_Weapon.m_Weapon->IsFire())
        {
            w->m_Weapon.m_Weapon->FireEnd();
        }
    }
}

/*
void CMatrixFlyer::DownToBase(CMatrixBuilding* building)
{
    if(building->m_Kind != BUILDING_BASE) return;

    D3DXVECTOR3 pos = building->GetPlacePos();

    m_BaseLandPos = pos + D3DXVECTOR3(0, 0, FLYER_ALT_MIN);

    SETFLAG(m_Flags, MF_SEEKBASE);
    RESETFLAG(m_Flags, MF_SEEKBASEOK);
    RESETFLAG(m_Flags, MF_SEEKBASEFOUND);
    RESETFLAG(m_Flags, FLYER_MANUAL);

    m_BaseLandAngle = (float)atan2(-building->m_Matrix._21, building->m_Matrix._22);
    //CalcTrajectory(pos + D3DXVECTOR3(0,0,FLYER_ALT_MIN) , *(D3DXVECTOR3 *)&building->m_Matrix._21);
}
*/

void CMatrixFlyer::CalcTrajectory(const D3DXVECTOR3& target)
{
    //CancelTrajectory();
    D3DXVECTOR3 dirto((target - m_Pos));
    D3DXVECTOR3 pts[9];

    if(m_Trajectory)
    {
        SETFLAG(m_Flags, FLYER_BREAKING);
        m_StoreTarget = target;
        return;
    }
    else
    {
        pts[0] = m_Pos;
        pts[1] = m_Pos + dirto * 0.23f; // + D3DXVECTOR3(0, 0, 30);

        {
            float newz = CalcFlyerZInPoint(pts[1].x, pts[1].y);
            pts[1].z = newz;
        }

        pts[2] = m_Pos + dirto * 0.76f;

        {
            float newz = CalcFlyerZInPoint(pts[2].x, pts[2].y);
            pts[2].z = newz;
        }

        pts[3] = target;

        m_Trajectory = HNew(Base::g_MatrixHeap) CTrajectory(Base::g_MatrixHeap);
        //m_Trajectory->Init1(pts, 4);
        m_Trajectory->Init2(pts, 4);

        m_TrajectoryPos = 0;
    }

    m_TrajectoryLen = m_Trajectory->CalcLength();
    m_TrajectoryLenRev = 1.0f / m_TrajectoryLen;

    //m_TrajectoryTargetAngle = GetDirectionAngle();
    m_TrajectoryTargetAngle = (float)atan2(-dirto.x, dirto.y);

#ifdef _DEBUG
    CHelper::DestroyByGroup(123);

    D3DXVECTOR3 pp;
    m_Trajectory->CalcPoint(pp, 0);
    for(float t = 0.01f; t <= 1.0f; t += 0.01f)
    {
        D3DXVECTOR3 ppp;
        m_Trajectory->CalcPoint(ppp, t);
        CHelper::Create(100000, 123)->Line(pp, ppp);

        pp = ppp;
    }
#endif

}

void CMatrixFlyer::CancelTrajectory(void)
{
    if(m_Trajectory)
    {
        HDelete(CTrajectory, m_Trajectory, Base::g_MatrixHeap);
        m_Trajectory = nullptr;

#ifdef _DEBUG
    CHelper::DestroyByGroup(123);
#endif
    }
}

#define MAXDA 0.017f
void CMatrixFlyer::ProceedTrajectory(SFlyerTactData& td)
{
    if(m_Trajectory == nullptr) return; //Для управляемых вертолётов траектория движения со старта не задаётся

    float ptp = m_TrajectoryPos;
    m_TrajectoryPos += td.ms * m_TrajectoryLenRev * 0.09f;

    D3DXVECTOR3 p;
    m_Trajectory->CalcPoint(p, m_TrajectoryPos);
    D3DXVECTOR3 fdir(p - m_Pos);

    float dd = D3DXVec3Length(&fdir);
    m_MoveSpeed = dd / td.ms;

    float a = (float)atan2(-fdir.x, fdir.y);
    float aa = float(AngleDist(a, m_TrajectoryTargetAngle));
    a += KSCALE(m_TrajectoryPos, 0.8f, 0.99f) * aa;

    float da = float(AngleDist(GetDirectionAngle(), a));

    if(fabs(da) < GRAD2RAD(30))
    {
        m_Pos = p;
    }
    else
    {
        m_TrajectoryPos = ptp;
    }

    float mul = (float)(1.0 - pow(0.997, double(td.ms))) * da;
    SetDirectionAngle(GetDirectionAngle() + mul);

    m_TargetEngineAngle = LERPFLOAT(0.5f, ENGINE_ANGLE_STAY, ENGINE_ANGLE_MOVE - m_Pitch);
    m_TargetPitchAngle = LERPFLOAT(0.5f, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_move);

    /*if(m_TrajectoryPos > stopt)
    {
        m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_BREAK);
        m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_break);
    }*/

    //if(m_Pitch < -1.0f) m_Pitch = -1.0f;

    //m_TargetRollAngle = -da * m_MoveSpeed;
    m_TargetRollAngle = 0;

    if(m_TrajectoryPos > 0.98f)
    {
        CancelTrajectory();
    }

    return;

    if(FLAG(m_Flags, FLYER_BREAKING))
    {
        m_MoveSpeed *= td.pow998;
        *(D3DXVECTOR2*)&m_Pos += D3DXVECTOR2(-m_AngleZSin, m_AngleZCos) * m_MoveSpeed * td.ms;
        if(td.speedn < 0.08f)
        {
            RESETFLAG(m_Flags, FLYER_BREAKING);
            CancelTrajectory();
            CalcTrajectory(m_StoreTarget);
        }
        else
        {
            D3DXVECTOR3 dirto((m_StoreTarget - m_Pos));
            m_TrajectoryTargetAngle = (float)atan2(-dirto.x, dirto.y);

            float da = float(AngleDist(GetDirectionAngle(), m_TrajectoryTargetAngle));

            float mul = (float)(1.0 - pow(0.997, double(td.ms))) * da;
            SetDirectionAngle(GetDirectionAngle() + mul);

            m_TargetRollAngle = -da;

            return;
        }
    }

    float breakt = 1.0f - 3.0f * GLOBAL_SCALE * m_TrajectoryLenRev;
    if(breakt < 0.5f) breakt = 0.5f;
    float stopt = 1.0f - 10.0f * m_TrajectoryLenRev;

    ptp = m_TrajectoryPos;
    m_TrajectoryPos += td.ms * m_TrajectoryLenRev * 0.09f * (1.0f - KSCALE(m_TrajectoryPos,breakt,1.0f));

    m_Trajectory->CalcPoint(p, m_TrajectoryPos);
    fdir = (p-m_Pos);

    dd = D3DXVec3Length(&fdir);
    m_MoveSpeed = dd / td.ms;

    a = (float)atan2(-fdir.x,fdir.y);
    aa = float(AngleDist(a, m_TrajectoryTargetAngle));
    a += KSCALE(m_TrajectoryPos, 0.8f, 0.99f) * aa;

    da = float(AngleDist(GetDirectionAngle(), a));

    if(fabs(da) < GRAD2RAD(30)) m_Pos = p;
    else m_TrajectoryPos = ptp;

    mul = (float)(1.0 - pow(0.997, double(td.ms))) * da;
    SetDirectionAngle(GetDirectionAngle() + mul);

    if(m_TrajectoryPos > breakt)
    {
        m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_BREAK);
        //m_TargetEngineAngle = ENGINE_ANGLE_BREAK;
        m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_break);
    }
    else
    {
        m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_MOVE - m_Pitch);
        m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_move);
    }
    /*if(m_TrajectoryPos > stopt)
    {
        m_TargetEngineAngle = LERPFLOAT(td.speedn, ENGINE_ANGLE_STAY, ENGINE_ANGLE_BREAK);
        m_TargetPitchAngle = LERPFLOAT(td.speedn, FlyerSpecifications[m_FlyerKind].pitch_angle_stay, FlyerSpecifications[m_FlyerKind].pitch_angle_break);
    }*/

    //if(m_Pitch < -1.0f) m_Pitch = -1.0f;

    m_TargetRollAngle = -da * m_MoveSpeed;

    if(m_TrajectoryPos > 0.97f)
    {
        CancelTrajectory();
    }
}

bool CMatrixFlyer::TakingDamage(
    int weap,
    const D3DXVECTOR3& pos,
    const D3DXVECTOR3& dir,
    int attacker_side,
    CMatrixMapStatic* attaker
)
{
DTRACE();

    if(g_Config.m_WeaponsConsts[weap].is_repairer) return false;

    CMatrixEffectWeapon::SoundHit(weap, pos);

    m_HitPoint = max(m_HitPoint - g_Config.m_WeaponsConsts[weap].damage.to_flyers, g_Config.m_WeaponsConsts[weap].non_lethal_threshold.to_flyers);
    if(m_HitPoint >= 0) m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);
    else m_ProgressBar.Modify(0);

    //Проверяем, какие эффекты накладывает попавшее в вертолёт оружие
    if(g_Config.m_WeaponsConsts[weap].extra_effects.size())
    {
        for(int i = 0; i < (int)g_Config.m_WeaponsConsts[weap].extra_effects.size(); ++i)
        {
            int effect = g_Config.m_WeaponsConsts[weap].extra_effects[i].type;
            byte effect_type = g_Config.m_WeaponsConsts[effect].secondary_effect;
            if(effect_type == SECONDARY_EFFECT_ABLAZE)
            {
                if(!g_Config.m_WeaponsConsts[effect].damage.to_flyers) continue;
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
                //MarkShorted(-1);
                //SetShortedTTL(GetShortedTTL() + 500);

                //m_NextTimeShorted = m_Time;

                FireEnd();
            }
        }
    }
    else m_LastDelayDamageSide = 0;

    //Если вертолёт ещё жив, рисуем проходящий по нему урон
    if(m_HitPoint > 0)
    {
        if(g_Config.m_WeaponsConsts[weap].explosive_hit)
        {
            CMatrixEffect::CreateExplosion(pos, ExplosionRobotHit);
            m_Roll += FSRND(0.1f); //Вертолёт слегка отклоняется по продольной оси для имитации попадания из тяжёлого оружия
        }
    }
    //Вертолёт уничтожен
    else
    {
        // dead!!!
        ReleaseMe();
        
        CMatrixEffect::CreateExplosion(*(D3DXVECTOR3*)&m_Core->m_Matrix._41, ExplosionRobotBoom);

        if(FLAG(m_Flags, FLYER_IN_SPAWN))
        {
            m_Base->Close();
        }

        g_MatrixMap->StaticDelete(this);
        return true;
    }
    return false;
}

bool CMatrixFlyer::Pick(const D3DXVECTOR3& start, const D3DXVECTOR3& dir, float* t) const
{
DTRACE();

    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Modules[i].m_Graph)
        {
            if(m_Modules[i].m_Graph->Pick(m_Modules[i].m_Matrix, m_Modules[i].m_IMatrix, start, dir, t)) return true;
        }
    }
    return false;
}

void CMatrixFlyer::Draw(void)
{
    DTRACE();
    dword coltex = (dword)g_MatrixMap->GetSideColorTexture(m_Side)->Tex();

    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Modules[i].m_Type == FLYER_UNIT_WEAPON_HOLLOW) continue;
        ASSERT(m_Modules[i].m_Graph);

        ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &(m_Modules[i].m_Matrix)));

        bool invert = (m_Modules[i].m_Type == FLYER_UNIT_ENGINE && m_Modules[i].m_Engine.m_Inversed != 0) ||
            (m_Modules[i].m_Type == FLYER_UNIT_PROPELLER && m_Modules[i].m_Propeller.m_Inversed != 0) ||
            (m_Modules[i].m_Type == FLYER_UNIT_WEAPON && m_Modules[i].m_Weapon.m_Inversed != 0);

        if(invert)
        {
            g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
            m_Modules[i].m_Graph->Draw(coltex);
            g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        }
        else
        {
            m_Modules[i].m_Graph->Draw(coltex);
        }
    }

    //if(IsCarryingRobot()) GetCarryingRobot()->Draw();

    if(!FLAG(g_MatrixMap->m_Flags, MMFLAG_OBJECTS_DRAWN))
    {
        for(int i = 0; i < m_StreamsCount; ++i)
        {
            m_Streams[i].effect->Draw();
        }
    }
}

void CMatrixFlyer::DrawPropeller(void)
{
    DTRACE();

    if(!IS_VB(m_VB)) return;

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    ASSERT_DX(g_D3DD->SetFVF(VO_FVF));

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);
    //SetAlphaOpSelect(0, D3DTA_CURRENT);
    SetColorOpDisable(1);


    ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(m_VB), 0, sizeof(SVOVertex)));

    for(int i = 1; i < m_ModulesCount; ++i)
    {
        if(m_Modules[i].m_Type != FLYER_UNIT_PROPELLER) continue;
        if(m_Modules[i].m_Propeller.m_Tex == nullptr) continue;
        //if(m_Modules[i].m_Propeller.m_Collapsed) continue;
        if(FLAG(m_Flags, FLYER_IN_SPAWN))
        {
            float k = float(m_Modules[i].m_Propeller.m_SpinupCountdown) / FLYER_SPINUP_TIME;
            dword c = LIC(0xFF000000, 0x00000000, k);
            SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, c));
            continue;
        }

        ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_Modules[i].m_Propeller.m_Matrix));
        ASSERT_DX(g_D3DD->SetTexture(0, m_Modules[i].m_Propeller.m_Tex->Tex()));
        ASSERT_DX(g_D3DD->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2));
    }

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
}

void CMatrixFlyer::DrawShadowStencil(void)
{
    DTRACE();
    
    if(!g_Config.m_ShowStencilShadows) return;

    if(m_Pos.x < 0 || m_Pos.y < 0 || m_Pos.x > (GLOBAL_SCALE * g_MatrixMap->m_Size.x) || m_Pos.y > (GLOBAL_SCALE * g_MatrixMap->m_Size.y)) return;

    for(int i=0; i<m_ModulesCount; ++i)
    {
		if(m_Modules[i].m_ShadowStencil)
        {
            m_Modules[i].m_ShadowStencil->Render(m_Modules[i].m_Matrix);
		}
	}
    if(IsCarryingRobot()) GetCarryingRobot()->DrawShadowStencil();
}

void CMatrixFlyer::FreeDynamicResources(void)
{
    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Modules[i].m_ShadowStencil) m_Modules[i].m_ShadowStencil->DX_Free();
    }
}

bool CMatrixFlyer::CalcBounds(D3DXVECTOR3& minv, D3DXVECTOR3& maxv)
{
    if(!m_ModulesCount) return true;

    //RChange(MR_Matrix);
    GetResources(MR_Matrix | MR_Graph);

    D3DXVECTOR3 bminout, bmaxout;

    minv.x = 1e30f; minv.y = 1e30f; minv.z = 1e30f;
    maxv.x = -1e30f; maxv.y = -1e30f; maxv.z = -1e30f;

    for(int u = 0; u < m_ModulesCount; ++u) // skip basis
    {
        if(m_Modules[u].m_Type == FLYER_UNIT_PROPELLER) continue;
        if(m_Modules[u].m_Type == FLYER_UNIT_WEAPON_HOLLOW) continue;
        int cnt = m_Modules[u].m_Graph->VO()->GetFramesCnt();
		for(int i = 0; i < cnt; ++i)
        {
            m_Modules[u].m_Graph->VO()->GetBound(i, m_Modules[u].m_Matrix, bminout, bmaxout);

            minv.x = min(minv.x, bminout.x);
            minv.y = min(minv.y, bminout.y);
            minv.z = min(minv.z, bminout.z);
            maxv.x = max(maxv.x, bmaxout.x);
            maxv.y = max(maxv.y, bmaxout.y);
            maxv.z = max(maxv.z, bmaxout.z);
		}
	}
    return false;
}

void CMatrixFlyer::Begin(CMatrixBuilding* b)
{
    m_Base = b;
    SETFLAG(m_Flags, FLYER_IN_SPAWN);
    m_CurrState = STATE_FLYER_IN_SPAWN;
    SetManualControlLocked(true);

    m_Pos.x = b->m_Pos.x + FSRND(0.01);
    m_Pos.y = b->m_Pos.y + FSRND(0.01);
    m_Pos.z = b->GetFloorZ();

    m_Side = b->GetSide();

    m_Target = b->m_Pos;
    SETFLAG(m_Flags, MF_TARGETMOVE);

    b->Open(); //Активируем подъёмник базы
    SetDirectionAngle((float)atan2(-b->GetMatrix()._21, b->GetMatrix()._22)); //Задаём начальное направление носа вертолёта

    GetResources(MR_Graph | MR_Matrix);

    JoinToGroup();
}

bool CMatrixFlyer::SelectByGroup()
{
    UnSelect();
    if(CreateSelection())
    {
        SETFLAG(m_Flags, FLYER_SGROUP);
        return true;
    }
    else
    {
        RESETFLAG(m_Flags, FLYER_SGROUP);
        return false;
    }
}

bool CMatrixFlyer::SelectArcade()
{
    UnSelect();
    if(CreateSelection())
    {
        SETFLAG(m_Flags, FLYER_SARCADE);
        return true;
    }
    else
    {
        RESETFLAG(m_Flags, FLYER_SARCADE);
        return false;
    }
}

void CMatrixFlyer::UnSelect()
{
    KillSelection();

    RESETFLAG(m_Flags, FLYER_SGROUP);    
    RESETFLAG(m_Flags, FLYER_SARCADE);
}

bool CMatrixFlyer::CreateSelection()
{
	m_Selection = (CMatrixEffectSelection*)CMatrixEffect::CreateSelection(D3DXVECTOR3(m_Pos.x, m_Pos.y, GetGeoCenter().z/*FLYER_SELECTION_HEIGHT*/), FLYER_SELECTION_SIZE);
    if(!g_MatrixMap->AddEffect(m_Selection))
    {
        m_Selection = nullptr;
        return false;
    }
    return true;
}

void CMatrixFlyer::KillSelection()
{
    if(m_Selection)
    {
        m_Selection->Kill();
        m_Selection = nullptr;
    }
}

void CMatrixFlyer::MoveSelection()
{
    if(m_Selection)
    {
        m_Selection->SetPos(D3DXVECTOR3(m_Pos.x, m_Pos.y, GetGeoCenter().z /* FLYER_SELECTION_HEIGHT */));
    }
}

bool CMatrixFlyer::IsSelected()
{
    if(FLAG(m_Flags, FLYER_SGROUP) || FLAG(m_Flags, FLYER_SARCADE))
    {
        return true;
    }
    return false;
}

void CMatrixFlyer::ReleaseMe()
{
    if(CMatrixFlyer::GetSide() == PLAYER_SIDE)
    {
        CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
        ps->RemoveFromSelection(this);
        int pos = 0;

        //Если данным вертолётом управлял игрок, то обновляем интерфейс, выводим его из аркадного режима
        if(ps->IsArcadeMode() && this == ps->GetArcadedObject() && g_IFaceList)
        {
            CInterface* ifs = g_IFaceList->m_First;
            while(ifs)
            {
                if(ifs->m_strName == IF_MAIN)
                {
                    ifs->m_xPos = float(g_ScreenX - (1024 - 447));
                    ifs->ReCalcElementsPos();
                    break;
                }
                ifs = ifs->m_NextInterface;
            }
        }

        if(ps->GetCurGroup())
        {
            CMatrixGroupObject* go = ps->GetCurGroup()->m_FirstObject;
            while(go)
            {
                ++pos;
                if(go->GetObject() == this) break;
                go = go->m_NextObject;
            }

            if(ps->GetCurGroup()->FindObject(this))
            {
                ps->GetCurGroup()->RemoveObject(this);
            }
        }

        if(GetBase())
        {
            if(m_CurrState == STATE_FLYER_IN_SPAWN || m_CurrState == STATE_FLYER_BASE_TAKE_OFF)
            {
                //Если вертолёт уничтожили, пока тот взлетал с базы, принудительно опускаем подъёмник
                GetBase()->SetSpawningUnit(false);
            }
            GetBase()->Close();
        }

        if(ps->GetCurSelNum() == pos) pos = 0;
        else pos = -1;

        DeleteProgressBarClone(PBC_CLONE1);
        DeleteProgressBarClone(PBC_CLONE2);
    }

    CMatrixMapStatic* objects = CMatrixMapStatic::GetFirstLogic();   
    while(objects)
    {
        if(objects->IsRobot())
        {
            ((CMatrixRobotAI*)objects)->GetEnv()->RemoveFromList(this);
        }
        objects = objects->GetNextLogic();
    }

    //CMatrixSideUnit *my_side = g_MatrixMap->GetSideById(m_Side);

    /*
    if(my_side->m_GroupsList != nullptr)
    {
        my_side->m_GroupsList->RemoveObject(m_Team, m_Group, (CMatrixMapStatic*)this);
    }
    if(my_side->m_CurGroup != nullptr)
    {
        CMatrixTactics* t = my_side->m_CurGroup->GetTactics();
        if(t) t->RemoveObjectFromT(this);
        
        my_side->m_CurGroup->RemoveObject(this);
    }
    */

    if(IsCarryingRobot())
    {
        CMatrixRobot* r = GetCarryingRobot();
        r->Carry(nullptr);
        r->MustDie();
    }
}

void CMatrixFlyer::CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type)
{
    m_ProgressBar.CreateClone(clone_type, x, y, width);
}

void CMatrixFlyer::DeleteProgressBarClone(EPBCoord clone_type)
{
    m_ProgressBar.KillClone(clone_type);
}

void CMatrixFlyer::CreateTextures()
{
    SRenderTexture rt[3];
    rt[0].ts = TEXSIZE_256;
    rt[1].ts = TEXSIZE_64;
    rt[2].ts = TEXSIZE_32;

    if(RenderToTexture(rt,3))
    {
        m_BigTexture = rt[0].tex;
        m_MedTexture = rt[1].tex;
        m_SmallTexture = rt[2].tex;
    }
    else
    {
        m_BigTexture = nullptr;
        m_MedTexture = nullptr;
        m_SmallTexture = nullptr;
    }
}

bool CMatrixFlyer::InRect(const CRect &rect) const
{
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

    for(int i=0; i<m_ModulesCount; ++i)
    {
        if(m_Modules[i].m_Graph)
        {
            d.found = false;
            d.m = m_Modules[i].m_Matrix * t;
            m_Modules[i].m_Graph->EnumFrameVerts(EnumVertsHandler, (dword)&d);
            if (d.found) return true;
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

