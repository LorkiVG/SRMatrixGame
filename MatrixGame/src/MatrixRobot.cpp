// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixRobot.hpp"
#include "MatrixObjectBuilding.hpp"
#include "Logic/MatrixRule.h"
#include "MatrixObjectCannon.hpp"
#include "MatrixFlyer.hpp"
#include "Interface/CInterface.h"
#include <vector>
#include <algorithm>

void SWeaponRepairData::Release()
{
    m_b0.Release();
    m_b1.Release();
    m_Sprites.Release();
    HFree(this, g_MatrixHeap);
}

void SWeaponRepairData::Draw(bool now)
{
    if(!FLAG(m_Flags, CAN_BE_DRAWN)) return;

    byte a = g_MatrixMap->IsPaused() ? 240 : (byte(FRND(128) + 128));

    m_Sprites.SetAlpha(a);
    m_b0.SetAlpha(a);
    m_b1.SetAlpha(a);

    D3DXVECTOR3 p0, p1;
    D3DXVECTOR3 tocam;
    tocam = (now ? g_MatrixMap->m_Camera.GetDrawNowFC() : g_MatrixMap->m_Camera.GetFrustumCenter()) - m_pos0;
    D3DXVec3Normalize(&tocam, &tocam);
    p0 = m_pos0 + tocam * 3;

    tocam = (now ? g_MatrixMap->m_Camera.GetDrawNowFC() : g_MatrixMap->m_Camera.GetFrustumCenter()) - m_pos1;
    D3DXVec3Normalize(&tocam, &tocam);
    p1 = m_pos1 + tocam * 3;

    m_Sprites.SetPos(p0, p1);
    m_b0.SetPos(p0);
    m_b1.SetPos(p1);

    if(now)
    {
        CVectorObject::DrawEnd();

        m_Sprites.DrawNow(g_MatrixMap->m_Camera.GetDrawNowFC());
        m_b0.DrawNow(g_MatrixMap->m_Camera.GetDrawNowIView());
        m_b1.DrawNow(g_MatrixMap->m_Camera.GetDrawNowIView());
    }
    else
    {
        m_Sprites.AddToDrawQueue();
        m_b0.Sort(g_MatrixMap->m_Camera.GetViewInversedMatrix());
        m_b1.Sort(g_MatrixMap->m_Camera.GetViewInversedMatrix());
    }
}

void SWeaponRepairData::Update(SMatrixRobotModule* unit)
{
    const D3DXMATRIX* m1 = unit->m_Graph->GetMatrixById(m_DeviceStartMatrixId);
    const D3DXMATRIX* m0 = unit->m_Graph->GetMatrixById(m_DeviceEndMatrixId);

    D3DXVec3TransformCoord(&m_pos0, (D3DXVECTOR3*)&m0->_41, &unit->m_Matrix);
    D3DXVec3TransformCoord(&m_pos1, (D3DXVECTOR3*)&m1->_41, &unit->m_Matrix);

    SETFLAG(m_Flags, SWeaponRepairData::CAN_BE_DRAWN);
}

SWeaponRepairData* SWeaponRepairData::Allocate(ESpriteTextureSort sprite_spot, ESpriteTextureSort sprite_rect, int dev_start_id, int dev_end_id)
{
    SWeaponRepairData* r = (SWeaponRepairData*)HAlloc(sizeof(SWeaponRepairData), g_MatrixHeap);

    r->m_DeviceStartMatrixId = dev_start_id;
    r->m_DeviceEndMatrixId = dev_end_id;

    r->m_b0.CSprite::CSprite(TRACE_PARAM_CALL D3DXVECTOR3(-1000, -1000, -1000), 5, 0, 0xFFFFFFFF, CMatrixEffect::GetSingleBrightSpriteTex(sprite_spot));
    r->m_b1.CSprite::CSprite(TRACE_PARAM_CALL D3DXVECTOR3(-1000, -1000, -1000), 10, 0, 0xFFFFFFFF, CMatrixEffect::GetSingleBrightSpriteTex(sprite_spot));
    r->m_Sprites.CSpriteLine::CSpriteLine(TRACE_PARAM_CALL D3DXVECTOR3(-1000, -1000, -1000), D3DXVECTOR3(-1000, -1000, -1000), 10, 0xFFFFFFFF, CMatrixEffect::GetSingleBrightSpriteTex(sprite_rect));
    RESETFLAG(r->m_Flags, SWeaponRepairData::CAN_BE_DRAWN);
    return r;
}

void SRobotWeapon::PrepareRepair()
{
    if(m_Module)
    {
        ASSERT(m_Module->m_WeaponRepairData == nullptr);
        m_Module->m_WeaponRepairData = SWeaponRepairData::Allocate((ESpriteTextureSort)g_Config.m_WeaponsConsts[GetWeaponNum()].sprite_set[0].sprites_num[0], (ESpriteTextureSort)g_Config.m_WeaponsConsts[GetWeaponNum()].sprite_set[1].sprites_num[0], g_Config.m_RobotWeaponsConsts[m_Module->m_Kind].dev_start_id, g_Config.m_RobotWeaponsConsts[m_Module->m_Kind].dev_end_id);
    }
}

void CMatrixRobotAI::DIPTact(float ms)
{
DTRACE();

    D3DXVECTOR3* pos;
    D3DXMATRIX* mat;

    bool del = true;
    for(int i = 0; i < m_ModulesCount; ++i)
    {
        if(m_Module[i].m_TTL <= 0) continue;
        m_Module[i].m_TTL -= ms;

        pos = &m_Module[i].m_Pos;

        if(m_Module[i].m_TTL <= 0)
        {
            //Create explosive
            pos->z += 10;
            CMatrixEffect::CreateExplosion(*pos, ExplosionRobotBoomSmall, true);
            if(m_Module[i].Smoke().effect)
            {
                ((CMatrixEffectSmoke*)m_Module[i].Smoke().effect)->SetTTL(1000);
                m_Module[i].Smoke().Unconnect();
            }
            continue;
        }

        del = false;

        if(IS_ZERO_VECTOR(m_Module[i].m_Velocity)) continue;

        D3DXVECTOR3 oldpos = *pos;

        m_Module[i].m_Velocity.z -= 0.0002f * ms;
        (*pos) += m_Module[i].m_Velocity * ms;
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
                if(m_Module[i].Smoke().effect)
                {
                    ((CMatrixEffectSmoke*)m_Module[i].Smoke().effect)->SetTTL(1000);
                    m_Module[i].Smoke().Unconnect();
                }
            }
        }
        else if(o == TRACE_STOP_LANDSCAPE)
        {
            m_Module[i].m_Velocity = D3DXVECTOR3(0, 0, 0);
            m_Module[i].m_Pos = hitpos;

            SMatrixMapUnit* mu = g_MatrixMap->UnitGetTest(Float2Int(hitpos.x * INVERT(GLOBAL_SCALE)), Float2Int(hitpos.y * INVERT(GLOBAL_SCALE)));
            if(mu == nullptr || mu->m_MapUnitBase != nullptr) m_Module[i].m_TTL = 0;
        }
        else if(IS_TRACE_STOP_OBJECT(o))
        {
            int hit_effect;
            switch(m_Module[i].m_Type)
            {
                case MRT_WEAPON: hit_effect = g_Config.m_RobotWeaponsConsts[m_Module[i].m_Kind].module_hit_effect; break;
                case MRT_HULL: hit_effect = g_Config.m_RobotHullsConsts[m_Module[i].m_Kind].module_hit_effect; break;
                case MRT_HEAD: hit_effect = g_Config.m_RobotHeadsConsts[m_Module[i].m_Kind].module_hit_effect; break;
                case MRT_CHASSIS: hit_effect = g_Config.m_RobotChassisConsts[m_Module[i].m_Kind].module_hit_effect; break;
                default: hit_effect = g_Config.m_RobotWeaponsConsts[m_Module[i].m_Kind].module_hit_effect; break;
            }

            if(hit_effect != WEAPON_NONE) o->TakingDamage(hit_effect, hitpos, m_Module[i].m_Velocity);
            m_Module[i].m_TTL = 1;
        }

        //if(pos->z < 0) pos->z = 0;

        float time = float(g_MatrixMap->GetTime());

        D3DXMATRIX m0, m1(g_MatrixMap->GetIdentityMatrix()), m2(g_MatrixMap->GetIdentityMatrix());
        const D3DXVECTOR3& pos1 = m_Module[i].m_Graph->VO()->GetFrameGeoCenter(m_Module[i].m_Graph->GetVOFrame());
        *(D3DXVECTOR3*)&m1._41 = pos1;
        *(D3DXVECTOR3*)&m2._41 = -pos1;
        D3DXMatrixRotationYawPitchRoll(&m0, m_Module[i].m_dy * time, m_Module[i].m_dp * time, m_Module[i].m_dr * time);
        *mat = m2 * m0 * m1;

        mat->_41 = pos->x;
        mat->_42 = pos->y;
        mat->_43 = pos->z;

        if(m_Module[i].Smoke().effect)
        {
            ((CMatrixEffectSmoke*)m_Module[i].Smoke().effect)->SetPos(*pos);
        }
    }

    if(del) g_MatrixMap->StaticDelete(this);
}

struct RCData
{
    D3DXVECTOR3 norm = { 0.0f, 0.0f, 0.0f };
    CMatrixMapStatic* nearest = nullptr;
    float             neardist2 = 0.0f;
};

static bool CollisionRobots(const D3DXVECTOR3& center, CMatrixMapStatic* ms, dword user)
{
    RCData* d = (RCData*)user;

    D3DXVECTOR3 v(center - ms->GetGeoCenter());
    float dist = D3DXVec3LengthSq(&v);
    if(dist < d->neardist2)
    {
        d->norm = v;
        d->neardist2 = dist;
        d->nearest = ms;
    }
    return true;
}

void CMatrixRobotAI::PauseTact(int cms)
{
    m_ProgressBar.Modify(100000.0f, 0);
    if(m_CurrState == ROBOT_DIP) return;
    if(m_ShowHitpointTime > 0)
    {
        m_ShowHitpointTime -= cms;
        if(m_ShowHitpointTime < 0) m_ShowHitpointTime = 0;
    }
}

void CMatrixRobotAI::LogicTact(int ms)
{
    //В этот блок исполнение перескакивает через goto
    //Некая корректировка анимации двуногого шасси робота
    if(false)
    {
    do_animation:;
        if(fabs(m_Speed) <= 0.01f) SwitchAnimation(ANIMATION_STAY);
        else
        {
            bool mt = FindOrderLikeThat(ROT_MOVE_TO);
            if(!mt) mt = FindOrderLikeThat(ROT_CAPTURE_BUILDING);

            if(mt) SwitchAnimation(ANIMATION_MOVE);
            else if(FindOrderLikeThat(ROT_MOVE_TO_BACK)) SwitchAnimation(ANIMATION_MOVE_BACK);
        }

        if(IsWalkingChassis())
        {
            DoAnimation(ms);
            StepLinkWalkingChassis(); //Обсчитываем шаги и рисуем следы для шагающего шасси
        }

        return;
    }

    //Срабатывает мгновенное уничтожение робота, если он должен умереть, но не был помечен маркером Death in progress (видимо)
    if(IsMustDie() && m_CurrState != ROBOT_DIP)
    {
        TakingDamage(WEAPON_INSTANT_DEATH, GetGeoCenter(), D3DXVECTOR3(0, 0, 0));
        return;
    }

    //Если робот, по каким-либо причинам, завис на базе дольше чем на 10 секунд, то он уничтожается, а база закрывает створки
    if(GetBase())
    {
        m_TimeWithBase += ms;
        if(m_TimeWithBase > 10000)
        {
            GetBase()->Close();
            m_TimeWithBase = 0;
            TakingDamage(WEAPON_INSTANT_DEATH, GetGeoCenter(), D3DXVECTOR3(0, 0, 0));
            return;
        }
    }

    if(m_ColsWeight) m_ColsWeight = max(0, m_ColsWeight - ms);
    if(m_ColsWeight2) m_ColsWeight2 = max(0, m_ColsWeight2 - ms);
    else GetEnv()->m_BadCoordCnt = 0;

    if(!g_MatrixMap->GetPlayerSide()->FindObjectInSelection(this)) UnSelect();

    m_SyncMul = (float)ms / LOGIC_TACT_PERIOD;
    MoveSelection();

    //Обновление точки местоположения робота (но это не точно)
    m_ProgressBar.Modify(100000.0f, 0);

    //Death in progress - протокол "смерти" запущен, робот готовиться к уничтожению
    if(m_CurrState == ROBOT_DIP)
    {
        DIPTact(float(ms));
        return;
    }

    //Тикает таймер времени отрисовки мигающего индикатора атакованного робота на миникарте
    if(m_MiniMapFlashTime > 0) m_MiniMapFlashTime -= ms;

    //Тикает таймер времени отрисовки полоски HP над роботом
    if(m_ShowHitpointTime > 0)
    {
        m_ShowHitpointTime -= ms;
        if(m_ShowHitpointTime < 0) m_ShowHitpointTime = 0;
    }

    //Если робот горит
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
                    D3DXVec3Normalize(&dir, &D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x, m_Core->m_Matrix._42 - pos.y, m_Core->m_Matrix._43 - pos.z));

                } while(!PickFull(pos, dir, &t) && (--cnt > 0));

                if(cnt > 0)
                {
                    ((CMatrixEffectShleif*)m_Ablaze.effect)->AddFire(pos + dir * (t + 2), 100, 1500, 30, 2.5f, false, 1.0f / 35.0f, g_Config.m_WeaponsConsts[ablaze_effect_num].close_color_rgb, g_Config.m_WeaponsConsts[ablaze_effect_num].far_color_rgb);
                }

                //Если робот был уничтожен огнём
                if(TakingDamage(ablaze_effect_num, pos, dir, m_LastDelayDamageSide, nullptr)) return;
            }
        }
    }

    //Если робот вырублен разрядником
    if(IsShorted())
    {
        int shorted_effect_num = IsShorted();
        while(g_MatrixMap->GetTime() > m_NextTimeShorted)
        {
            m_NextTimeShorted += SHORTED_OUT_LOGIC_PERIOD;

            D3DXVECTOR3 d1, d2, dir, pos;
            float t;

            int cnt = 4;
            do
            {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                D3DXVec3Normalize(&dir, &D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x, m_Core->m_Matrix._42 - pos.y, m_Core->m_Matrix._43 - pos.z));
            } while(!Pick(pos, dir, &t) && (--cnt > 0));
            
            if(cnt > 0) d1 = pos + dir * t;

            do
            {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                D3DXVec3Normalize(&dir, &D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x, m_Core->m_Matrix._42 - pos.y, m_Core->m_Matrix._43 - pos.z));
            } while(!Pick(pos, dir, &t) && (--cnt > 0));
            
            if(cnt > 0)
            {
                d2 = pos + dir * t;
                CMatrixEffect::CreateShorted(d1, d2, FRND(400) + 100, g_Config.m_WeaponsConsts[shorted_effect_num].hex_BGRA_sprites_color, g_Config.m_WeaponsConsts[shorted_effect_num].sprite_set[0].sprites_num[0]);
            }

            if(TakingDamage(shorted_effect_num, pos, dir, m_LastDelayDamageSide, nullptr)) return;
        }
    }
    //Саморемонт под станом невозможен
    else
    {
        //Если у робота имеется саморемонт, то накидываем ему недостающие HP отсюда
        if(m_SelfRepair || m_SelfRepairPercent)
        {
            if(m_HitPoint < m_HitPointMax)
            {
                m_HitPoint = min(m_HitPoint + m_SelfRepair + m_SelfRepairPercent, m_HitPointMax);
                m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);

                //Хуй заставишь это всё нормально работать
                //CMatrixEffectWeapon::SelfRepairEffect();

                //CSound::AddSound(S_WEAPON_REPAIR, pos, SL_ALL, SEF_SKIP);
                //if(m_Sound != SOUND_ID_EMPTY)
                //{
                //    m_Sound = CSound::ChangePos(m_Sound, m_SoundType, m_Pos);
                //}
            }
        }
    }

DCP();

    //Если робот является крейзиботом
    if(IsCrazy())
    {
        static int ppx = -1;
        static int ppy = -1;
        static int cnt = 0;
        if(ppx == m_MapX && ppy == m_MapY) ++cnt;
        else
        {
            ppx = m_MapX;
            ppy = m_MapY;
        }

        if(cnt > 100 || (PLIsInPlace() && GetEnv()->GetEnemyCnt() <= 0))
        {
            cnt = 0;
            ppx = m_MapX;
            ppy = m_MapY;
            // find new target :)

            CMatrixMapStatic* target = nullptr;
            CMatrixMapStatic* ms = CMatrixMapStatic::GetFirstLogic();
            for(; ms; ms = ms->GetNextLogic())
            {
                /*
                if(ms->IsBuildingAlive() && !ms->IsBase())
                {
                    b = ms->AsBuilding();
                    if(FRND(1) < 0.1f) break;
                }
                */
                if(ms->IsRobotAlive() && ms != this)
                {
                    target = ms;
                    if(ms->GetSide() == PLAYER_SIDE) break;
                }
            }

            if(target == nullptr)
            {
                MustDie();
                return;
            }

            SObjectCore* c = target->GetCore(DEBUG_CALL_INFO);
            CPoint pos(int(c->m_GeoCenter.x / GLOBAL_SCALE_MOVE), int(c->m_GeoCenter.y / GLOBAL_SCALE_MOVE));
            c->Release();

            SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);

            CMatrixSideUnit* su = g_MatrixMap->GetSideById(m_Side);
            su->PGOrderAttack(su->RobotToLogicGroup(this), pos, target);

            RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
        }
    }
DCP();

    //Если робот был сброшен с вертолёта и в данный момент летит вниз
    if(m_CurrState == ROBOT_FALLING)
    {
DCP();
        float dtime = float(ms) * 0.013f;
        m_FallingSpeed += dtime;
        m_Core->m_Matrix._43 -= m_FallingSpeed * dtime;
        RChange(MR_Matrix | MR_ShadowProjGeom | MR_ShadowProjTex | MR_ShadowStencil);

        float z = Z_From_Pos();
        if(m_Core->m_Matrix._43 < z)
        {
            m_Core->m_Matrix._43 = z;
            m_CurrState = ROBOT_SUCCESSFULLY_BUILD; //Маркер о готовности робота к получению дальнейших приказов
            m_KeelWaterCount = 0;

            m_FallingSpeed = 0;

            CMatrixEffect::CreateDust(nullptr, *(D3DXVECTOR2*)&GetGeoCenter(), D3DXVECTOR2(0, 0), 3000);
            CMatrixEffect::CreateDust(nullptr, *(D3DXVECTOR2*)&GetGeoCenter(), D3DXVECTOR2(0, 0), 3000);
            CMatrixEffect::CreateDust(nullptr, *(D3DXVECTOR2*)&GetGeoCenter(), D3DXVECTOR2(0, 0), 3000);
            CMatrixEffect::CreateDust(nullptr, *(D3DXVECTOR2*)&GetGeoCenter(), D3DXVECTOR2(0, 0), 3000);
            JoinToGroup();

            //Определяем место, в которое упадёт робот для последующей отрисовки следов от колёсного/гусеничного типа шасси
            if(IsRollingChassis()) m_ChassisData.m_LastSolePos = GetGeoCenter();

            //int x0 = TruncFloat(m_PosX * INVERT(GLOBAL_SCALE_MOVE));
            //int y0 = TruncFloat(m_PosY * INVERT(GLOBAL_SCALE_MOVE));

            //AsRobot()->MoveTo(x0, y0);
            AsRobot()->MapPosCalc();

            CSound::AddSound(S_ROBOT_UPAL, GetGeoCenter());
        }
        return;
    }

DCP();

    //Робот болтается в вертолётной "авоське", ожидает сброса
    if(m_CurrState == ROBOT_CARRYING)
    {
DCP();
        RCData data;

        float mul = 1.0 - pow(CARRYING_SPEED, double(ms));
        CMatrixFlyer::SCarryData* carry_data = m_CargoFlyer->GetCarryData();
        if(carry_data->m_RobotElevatorField && m_CargoFlyer->GetCarryData()->m_RobotElevatorField->m_Activated)
        {
            //m_Matrix._43 -= (0.01f * ms);
            carry_data->m_RobotMassFactor += (0.0005f * ms);
            if(carry_data->m_RobotMassFactor > 1.0f)
            {
                carry_data->m_RobotMassFactor = 1.0f;
            }

            //if(FRND(1) < mul * 0.5f) g_MatrixMap->AddEffect(CMatrixEffect::CreateShorted(g_MatrixHeap, m_CargoFlyer->GetPos(), *(D3DXVECTOR3*)&m_Matrix._41, FRND(400) + 100));
        }

        m_FallingSpeed -= float(ms) * 0.013f;
        if(m_FallingSpeed < 0) m_FallingSpeed = 0;
DCP();
        D3DXVECTOR3 move, delta(m_CargoFlyer->GetPos() - D3DXVECTOR3(0, 0, CARRYING_DISTANCE) - m_Core->m_GeoCenter);
        //float x = D3DXVec3Length(&delta);
        move = delta * mul * m_CargoFlyer->GetCarryData()->m_RobotMassFactor;
        move.z -= m_FallingSpeed * float(ms) * 0.013f;
        *(D3DXVECTOR3*)&m_Core->m_Matrix._41 += move;

        float cz = g_MatrixMap->GetZ(m_Core->m_Matrix._41, m_Core->m_Matrix._42);
        if(m_Core->m_Matrix._43 < cz) m_Core->m_Matrix._43 = cz;

        carry_data->m_RobotUp.x += move.x * 0.03f + m_CargoFlyer->GetCarryData()->m_RobotUpBack.x * mul;
        carry_data->m_RobotUp.y += move.y * 0.03f + m_CargoFlyer->GetCarryData()->m_RobotUpBack.y * mul;

        Vec2Truncate(*(D3DXVECTOR2*)&m_CargoFlyer->GetCarryData()->m_RobotUp, 1);

        carry_data->m_RobotUpBack.x -= m_CargoFlyer->GetCarryData()->m_RobotUp.x * 0.08f;
        carry_data->m_RobotUpBack.y -= m_CargoFlyer->GetCarryData()->m_RobotUp.y * 0.08f;;
        carry_data->m_RobotUpBack.z = 0;

        float mul2 = pow(0.999, double(ms));
        (*(D3DXVECTOR2*)&m_CargoFlyer->GetCarryData()->m_RobotUp) *= mul2;

        Vec2Truncate(*(D3DXVECTOR2*)&m_CargoFlyer->GetCarryData()->m_RobotUpBack, 0.4f);

        float da = mul * mul * AngleDist(m_CargoFlyer->GetDirectionAngle(), m_CargoFlyer->GetCarryData()->m_RobotAngle);
        m_CargoFlyer->GetCarryData()->m_RobotAngle -= da;

        *((D3DXVECTOR2*)&m_ChassisForward) = RotatePoint(*(D3DXVECTOR2*)&m_CargoFlyer->GetCarryData()->m_RobotForward, m_CargoFlyer->GetCarryData()->m_RobotAngle);
        m_ChassisForward.z = m_CargoFlyer->GetCarryData()->m_RobotForward.z;

        RChange(MR_Matrix);
        m_CalcBoundsLastTime = g_MatrixMap->GetTime() - 10000;

        D3DXVECTOR3 vmin, vmax;
        if(CalcBounds(vmin, vmax)) ERROR_E;
        m_Core->m_GeoCenter = (vmin + vmax) * 0.5f;

        data.neardist2 = 1e30f;
        data.nearest = nullptr;

        bool hit = g_MatrixMap->FindObjects(m_Core->m_GeoCenter, m_Core->m_Radius * 0.5f, 0.5f, TRACE_ALL, nullptr, CollisionRobots, (dword)&data);

        if(hit)
        {
            float dist = sqrt(data.neardist2);
            if(dist >= 0.000001f) data.norm *= INVERT(dist);
            else D3DXVec3Normalize(&data.norm, &data.norm);

            *(D3DXVECTOR3*)&m_Core->m_Matrix._41 += data.norm * (data.nearest->GetRadius() * 0.5f + m_Core->m_Radius * 0.5f - dist);
        }

        dword tc = g_MatrixMap->GetColor(m_Core->m_GeoCenter.x, m_Core->m_GeoCenter.y);
        //m_Core->m_TerainColor = LIC(tc,g_MatrixMap->m_Terrain2ObjectTargetColor, g_MatrixMap->m_Terrain2ObjectInfluence);
        m_Core->m_TerainColor = 0xFFFFFFFF;

        //m_PosX = m_Matrix._41;
        //m_PosY = m_Matrix._42;
        m_PosX = m_Core->m_GeoCenter.x;
        m_PosY = m_Core->m_GeoCenter.y;

        RChange(MR_Matrix | MR_ShadowProjGeom | MR_ShadowProjTex | MR_ShadowStencil);

        //GetResources(MR_Matrix);
        if(m_CargoFlyer->GetCarryData()->m_RobotElevatorField == nullptr)
        {
            m_CargoFlyer->GetCarryData()->m_RobotElevatorField = (CMatrixEffectElevatorField*)CMatrixEffect::CreateElevatorField(m_CargoFlyer->GetPos(), m_Core->m_GeoCenter, m_Core->m_Radius, m_ChassisForward);
            if(!g_MatrixMap->AddEffect(m_CargoFlyer->GetCarryData()->m_RobotElevatorField)) m_CargoFlyer->GetCarryData()->m_RobotElevatorField = nullptr;
        }
        else m_CargoFlyer->GetCarryData()->m_RobotElevatorField->UpdateData(m_CargoFlyer->GetPos(), *(D3DXVECTOR3*)&m_Core->m_Matrix._41, m_Core->m_Radius, m_ChassisForward);

        return;
    }

    //Если робота вырубило разрядником, прерываем дальнейшее исполнение логики
    if(IsShorted()) return;

DCP();

    //normal...

    float mul = 1.0 - pow(0.996, double(ms));
    D3DXVECTOR3 up;
    g_MatrixMap->GetNormal(&up, m_PosX, m_PosY, true);
    *(D3DXVECTOR3*)&m_Core->m_Matrix._31 = LERPVECTOR(mul, *(D3DXVECTOR3*)&m_Core->m_Matrix._31, up);
    
DCP();
    
    if(m_CurrState != ROBOT_IN_SPAWN)
    {
DCP();
        //Создаём на земле эффекты следов от колёсного/гусеничного типа шасси
        if(IsRollingChassis())
        {
            int kind = m_Module[0].m_Kind;
            float dist_sq = D3DXVec3LengthSq(&(m_ChassisData.m_LastSolePos - m_Core->m_GeoCenter));
            if(dist_sq > g_Config.m_RobotChassisConsts[kind].ground_trace.trace_redraw_distance) //Минимальная дистанция очередной отрисовки текстуры - 100
            {
                m_ChassisData.m_LastSolePos = m_Core->m_GeoCenter;
                int x = TruncFloat(m_Core->m_GeoCenter.x * INVERT(GLOBAL_SCALE));
                int y = TruncFloat(m_Core->m_GeoCenter.y * INVERT(GLOBAL_SCALE));
                SMatrixMapUnit* mu = g_MatrixMap->UnitGetTest(x, y);

                if(mu && mu->IsLand() && !mu->IsBridge())
                {
                    float ang = atan2(-m_ChassisForward.x, m_ChassisForward.y);
                    CMatrixEffect::CreateLandscapeSpot(nullptr, *(D3DXVECTOR2*)&m_Core->m_Matrix._41, ang, g_Config.m_RobotChassisConsts[kind].ground_trace.texture_scale, (ESpotType)g_Config.m_RobotChassisConsts[kind].ground_trace.trace_num);
                }
            }
        }
    }
DCP();

    //Если готовый робот в данный момент собирается покинуть базу
    if(m_CurrState == ROBOT_IN_SPAWN)
    {
 DCP();
        if(GetBase())
        {
            //И база уже готова выпустить его на свет, выставляем роботу маркер выезда с подъёмника базы
            if(GetBase()->m_State == BASE_OPENED)
            {
                m_CurrState = ROBOT_BASE_MOVEOUT;
                SetAfterManual(false);
            }
        }
    }
    //Если ранее робот уже получил маркер выезда с базы, то отдаём необходимые команды
    //Начальный вейпоинт, используемый сразу после выхода с базы, робот получает в функции RobotSpawn
    else if(m_CurrState == ROBOT_BASE_MOVEOUT)
    {
DCP();
        //Робот как бы "выталкивается" с базы, получая команду продвинутся вперёд
        LowLevelMove(ms, m_ChassisForward * 100, true, false);
DCP();
        RChange(MR_Matrix | MR_ShadowProjGeom | MR_ShadowProjTex | MR_ShadowStencil);

        //Если породившая робота база всё ещё существует
        if(GetBase())
        {
            CMatrixBuilding* base = GetBase();
DCP();
            D3DXVECTOR2 dist = D3DXVECTOR2(m_PosX, m_PosY) - base->m_Pos;
            //Если робот уже успел отдалиться от базы на некоторое расстояние
            if(D3DXVec2LengthSq(&dist) >= BASE_DIST * BASE_DIST)
            {
DCP();
                GetLost(m_ChassisForward); //Вот эта строка, вероятно, лишняя, либо используется только для подстраховки нештатных ситуаций (тупо направляет робота вперёд)
                D3DXVec2Normalize(&dist, &dist);
                m_CurrState = ROBOT_SUCCESSFULLY_BUILD;
                SetManualControlLocked(false);
DCP();
                if(!m_OrdersInPool) LowLevelStop();
                g_MatrixMap->m_Minimap.AddEvent(m_Core->m_GeoCenter.x, m_Core->m_GeoCenter.y, 0xff00ff00, 0xff00ff00);

                base->SetSpawningUnit(false);
                base->Close();
                SetBase(nullptr);
DCP();
            }
        }
        //Иначе роботу кранты (типа не успел свалить)
        else
        {
            MustDie();
            return;
        }
DCP();
        goto do_animation;
    }
DCP();

    //////////////// Real-deal logic starts here :))) /////////////////////////////////////////////////////////////////////////	
    if(m_CurrState != ROBOT_SUCCESSFULLY_BUILD)
    {
        RChange(MR_Matrix | MR_ShadowProjGeom | MR_ShadowProjTex | MR_ShadowStencil);
        if(m_CurrState != ROBOT_CAPTURING_BASE) goto do_animation;
    }
DCP();

    //GatherInfo(ms);
    //m_Environment.LogicTact();

    //if(this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject())
    //CDText::T("Velocity", CStr(m_Speed));
DCP();

    if(m_Side == PLAYER_SIDE && this == g_MatrixMap->GetPlayerSide()->GetArcadedObject() && m_CurrState == ROBOT_SUCCESSFULLY_BUILD)
    {
        if(g_IFaceList->m_InFocus != INTERFACE)
        {
            SetHullTargetDirection(g_MatrixMap->m_TraceStopPos);
        }
    }

    //TODO : fire while capture here!

    if(this != g_MatrixMap->GetPlayerSide()->GetArcadedObject())
    {
        if(m_Environment.m_Target && m_Environment.m_Target->IsRobot() && m_Environment.m_Target->AsRobot()->m_CurrState == ROBOT_SUCCESSFULLY_BUILD)
        {
            bool capturing = FindOrderLikeThat(ROT_CAPTURE_BUILDING);

            CMatrixRobotAI* Enemy = (CMatrixRobotAI*)m_Environment.m_Target;
            D3DXVECTOR2 napr(0, 0);
            D3DXVECTOR3 enemy_pos(0, 0, 0);
            enemy_pos = D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0);
            napr = D3DXVECTOR2(Enemy->m_PosX, Enemy->m_PosY) - D3DXVECTOR2(m_PosX, m_PosY);
            D3DXVECTOR2 naprN;
            D3DXVec2Normalize(&naprN, &napr);

            float Cos = m_ChassisForward.x * naprN.x + m_ChassisForward.y * naprN.y;
            float needAngle = float(acos(Cos));
            //SetHullTargetDirection(D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0));
            if(fabs(needAngle) < MAX_HULL_ANGLE)
            {
                SetHullTargetDirection(D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0));
            }
            else if(m_CollAvoid.x == 0 && m_CollAvoid.y == 0 && !capturing)
            {
                RotateRobotChassis(D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0));
                SetHullTargetDirection(D3DXVECTOR3(m_PosX + m_ChassisForward.x, m_PosY + m_ChassisForward.y, 0));
            }

            /*
            Cos = m_ChassisForward.x * m_HullForward.x + m_ChassisForward.y * m_HullForward.y;
            float realAngle = float(acos(Cos));

            if(fabs(needAngle - realAngle) <= HULL_TO_ENEMY_ANGLE)
            {
                if(!Enemy->m_Environment.SearchEnemy(this)) Enemy->m_Environment.AddToList(this);
                m_FireTarget = Enemy;
                Fire(Enemy->m_Core->m_GeoCenter + Enemy->m_Velocity);
            }
            else StopFire();
            */
        }
        else if(m_Environment.m_Target && m_Environment.m_Target->IsActiveCannonAlive())
        {
            bool capturing = FindOrderLikeThat(ROT_CAPTURE_BUILDING);

            CMatrixCannon* Enemy = (CMatrixCannon*)m_Environment.m_Target;
            D3DXVECTOR2 napr(0, 0);
            D3DXVECTOR3 enemy_pos(0, 0, 0);
            enemy_pos = D3DXVECTOR3(Enemy->GetGeoCenter().x, Enemy->GetGeoCenter().y, 0);
            napr = D3DXVECTOR2(enemy_pos.x, enemy_pos.y) - D3DXVECTOR2(m_PosX, m_PosY);
            D3DXVECTOR2 naprN;
            D3DXVec2Normalize(&naprN, &napr);

            float Cos = m_ChassisForward.x * naprN.x + m_ChassisForward.y * naprN.y;
            float needAngle = acos(Cos);
            //SetHullTargetDirection(D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0));
            if(fabs(needAngle) < MAX_HULL_ANGLE)
            {
                SetHullTargetDirection(enemy_pos);
            }
            else if(!m_CollAvoid.x && !m_CollAvoid.y && !capturing)
            {
                RotateRobotChassis(enemy_pos);
                SetHullTargetDirection(D3DXVECTOR3(m_PosX + m_ChassisForward.x, m_PosY + m_ChassisForward.y, 0));
            }

            /*
            Cos = m_ChassisForward.x * m_HullForward.x + m_ChassisForward.y * m_HullForward.y;
            float realAngle = float(acos(Cos));

            if(fabs(needAngle - realAngle) <= HULL_TO_ENEMY_ANGLE)
            {
                m_FireTarget = Enemy;
                Fire(Enemy->GetGeoCenter());
            }
            else topFire();
            */
        }
        else if(m_Environment.m_Target && m_Environment.m_Target->IsBuildingAlive() && !FindOrderLikeThat(ROT_CAPTURE_BUILDING))
        {
            //CMatrixBuilding* Enemy = (CMatrixCannon*)m_Environment.m_Target;
            D3DXVECTOR2 napr(0, 0);
            D3DXVECTOR3 enemy_pos = m_WeaponDir;
            //enemy_pos = D3DXVECTOR3(Enemy->GetGeoCenter().x, Enemy->GetGeoCenter().y, 0);
            napr = D3DXVECTOR2(enemy_pos.x, enemy_pos.y) - D3DXVECTOR2(m_PosX, m_PosY);
            D3DXVECTOR2 naprN;
            D3DXVec2Normalize(&naprN, &napr);

            float Cos = m_ChassisForward.x * naprN.x + m_ChassisForward.y * naprN.y;
            float needAngle = float(acos(Cos));
            if(fabs(needAngle) < MAX_HULL_ANGLE)
            {
                SetHullTargetDirection(enemy_pos);
            }
            else if(m_CollAvoid.x == 0 && m_CollAvoid.y == 0)
            {
                RotateRobotChassis(enemy_pos);
                SetHullTargetDirection(D3DXVECTOR3(m_PosX + m_ChassisForward.x, m_PosY + m_ChassisForward.y, 0));
            }
        }
        else
        {
            if(this != (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject())
            {
                StopFire();
                SetHullTargetDirection(D3DXVECTOR3(m_PosX + m_ChassisForward.x, m_PosY + m_ChassisForward.y, 0));
            }
        }
    }

    //ORDERS PROCESSING////////////////////////////////////////////////////
    if(this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject()/* && !(GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0x8000*/)
    {
        //SetWindowTextA(g_Wnd, "test1"); //Может быть полезно для отладки

        //Стрейф
        byte is_strafing = 0; //Не выставляем этот маркер при полноценных стрейфах парящих шасси, т.к. им никаких дополнительных коррекций не требуется
        if(is_strafing >= 0 && (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_STRAFE_LEFT]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_STRAFE_LEFT_ALT]) & 0x8000) == 0x8000)
        {
            if(IsHoveringChassis())
            {
                D3DXVECTOR3 dest(m_PosX, m_PosY, 0.0f);
                dest += D3DXVECTOR3(m_ChassisForward.y, -m_ChassisForward.x, m_ChassisForward.z) * m_maxStrafeSpeed;
                LowLevelMove(ms, dest, true, true, true, UNIT_MOVING_LEFT);
            }
            else
            {
                D3DXVECTOR3 dest = D3DXVECTOR3(m_ChassisCamForward.y, -m_ChassisCamForward.x, m_ChassisCamForward.z);
                if(!RotateRobotChassis(dest + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), NO_CAM_ROTATION))
                {
                    if(
                        !((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD]) & 0x8000) == 0x8000) && !((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD_ALT]) & 0x8000) == 0x8000) &&
                        !((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD]) & 0x8000) == 0x8000) && !((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD_ALT]) & 0x8000) == 0x8000)
                      )
                    {
                        if(GetAnimation() != ANIMATION_STAY) SwitchAnimation(ANIMATION_STAY); //Здесь должна быть анимация разворота, но её "в природе" не существует
                    }
                }
                else
                {
                    if(GetAnimation() != ANIMATION_MOVE) SwitchAnimation(ANIMATION_MOVE);
                    LowLevelMove(ms, dest * m_maxStrafeSpeed + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), true, true, true, UNIT_MOVING_FORWARD, false);
                }
                is_strafing = 1;
                SETFLAG(m_ObjectFlags, ROBOT_FLAG_STRAFING);
            }
        }
        if(is_strafing <= 0 && (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_STRAFE_RIGHT]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_STRAFE_RIGHT_ALT]) & 0x8000) == 0x8000)
        {
            if(IsHoveringChassis())
            {
                D3DXVECTOR3 dest(m_PosX, m_PosY, 0.0f);
                dest += D3DXVECTOR3(-m_ChassisForward.y, m_ChassisForward.x, m_ChassisForward.z) * m_maxStrafeSpeed;
                LowLevelMove(ms, dest, true, true, true, UNIT_MOVING_RIGHT);
            }
            else
            {
                D3DXVECTOR3 dest = D3DXVECTOR3(-m_ChassisCamForward.y, m_ChassisCamForward.x, m_ChassisCamForward.z);
                if(!RotateRobotChassis(dest + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), NO_CAM_ROTATION))
                {
                    if(
                        !((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD]) & 0x8000) == 0x8000) && !((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD_ALT]) & 0x8000) == 0x8000) &&
                        !((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD]) & 0x8000) == 0x8000) && !((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD_ALT]) & 0x8000) == 0x8000)
                      )
                    {
                        if(GetAnimation() != ANIMATION_STAY) SwitchAnimation(ANIMATION_STAY); //Здесь должна быть анимация разворота, но её "в природе" не существует
                    }
                }
                else
                {
                    if(GetAnimation() != ANIMATION_MOVE) SwitchAnimation(ANIMATION_MOVE);
                    LowLevelMove(ms, dest * m_maxStrafeSpeed + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), true, true, true, UNIT_MOVING_FORWARD, false);
                }
                is_strafing = -1;
                SETFLAG(m_ObjectFlags, ROBOT_FLAG_STRAFING);
            }
        }

        //Возвращаем шасси в исходное положение после стрейфа (только для не парящих шасси)
        if(!is_strafing)
        {
            if(IsStrafing()) RESETFLAG(m_ObjectFlags, ROBOT_FLAG_STRAFING);

            if(m_ChassisCamForward != m_ChassisForward)
            {
                if(GetAnimation() != ANIMATION_STAY) SwitchAnimation(ANIMATION_STAY);
                RotateRobotChassis(m_ChassisCamForward + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), NO_CAM_ROTATION);
            }
        }

        //Поворот шасси (и, как следствие, камеры)
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ROTATE_LEFT]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ROTATE_LEFT_ALT]) & 0x8000) == 0x8000)
        {
            //RotateRobotLeft(); //Вроде как тут обработчик учитывает ещё и отклонение по оси Z, но я не уверен, что оно до сих пор актуально

            D3DXVECTOR3 dest = D3DXVECTOR3(m_ChassisForward.y, -m_ChassisForward.x, m_ChassisForward.z);
            if(!is_strafing) RotateRobotChassis(dest + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), CONNECTED_CAM_ROTATION);
            else RotateRobotChassis(dest + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), SIMULTANEOUS_CAM_ROTATION);
        }
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ROTATE_RIGHT]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ROTATE_RIGHT_ALT]) & 0x8000) == 0x8000)
        {
            //RotateRobotRight(); //Вроде как тут обработчик учитывает ещё и отклонение по оси Z, но я не уверен, что оно до сих пор актуально

            D3DXVECTOR3 dest = D3DXVECTOR3(-m_ChassisForward.y, m_ChassisForward.x, m_ChassisForward.z);
            if(!is_strafing) RotateRobotChassis(dest + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), CONNECTED_CAM_ROTATION);
            else RotateRobotChassis(dest + D3DXVECTOR3(m_PosX, m_PosY, 0.0f), SIMULTANEOUS_CAM_ROTATION);
        }

        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_FIRE]) & 0x8000) == 0x8000)
        {
            if(!FindOrderLikeThat(ROT_FIRE) && g_IFaceList->m_InFocus != INTERFACE)
            {
                OrderFire(g_MatrixMap->m_TraceStopPos);
            }
        }
        else
        {
            if(FindOrderLikeThat(ROT_FIRE))
            {
                StopFire();
            }
        }
    }

    //Переключение на "камеру заднего вида" и обратно
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_REAR_VIEW]) & 0x8000) == 0x8000)
    {
        SETFLAG(m_ObjectFlags, UNIT_FLAG_REAR_VIEW);
    }
    else if(FLAG(m_ObjectFlags, UNIT_FLAG_REAR_VIEW))
    {
        RESETFLAG(m_ObjectFlags, UNIT_FLAG_REAR_VIEW);
    }

    //Вот здесь раньше выполнялся разворот корпуса по RotateRobotLeft(); и RotateRobotRight();
    //Вроде как тут обработчик учитывает ещё и отклонение по оси Z, но я не уверен, что оно до сих пор актуально
    /*
    if(FLAG(m_ObjectFlags, ROBOT_FLAG_ROT_LEFT | ROBOT_FLAG_ROT_RIGHT))
    {
        D3DXVECTOR3 dest;
        D3DXMATRIX rot_mat;

        float ang = 0;
        if(FLAG(m_ObjectFlags, ROBOT_FLAG_ROT_LEFT)) ang -= GRAD2RAD(90.0f);
        if(FLAG(m_ObjectFlags, ROBOT_FLAG_ROT_RIGHT)) ang += GRAD2RAD(90.0f);

        dest = m_ChassisForward * m_maxSpeed;
        D3DXMatrixRotationZ(&rot_mat, ang);
        D3DXVec3TransformCoord(&dest, &dest, &rot_mat);
        dest.x += m_PosX;
        dest.y += m_PosY;

        RotateRobotChassis(dest);
        RESETFLAG(m_ObjectFlags, ROBOT_FLAG_ROT_LEFT | ROBOT_FLAG_ROT_RIGHT);
    }
    */

    if(FLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE) || m_Side != PLAYER_SIDE) TactCaptureCandidate(ms);

    //if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()) ASSERT(1);
    int cnt = 0;
    while(cnt < m_OrdersInPool)
    {
        float f1 = 0, f2 = 0, f3 = 0, x = 0, y = 0;
        D3DXVECTOR3 vvv;
        CMatrixBuilding* building = nullptr;
        int mx = 0, my = 0, d = 0;
        bool StillMoving = false;
        switch(m_OrdersList[0].GetOrderType())
        {
            case ROT_FIRE:
            {
                int type;
                m_OrdersList[0].GetParams(&f1, &f2, &f3, &type);

                if(g_MatrixMap->GetPlayerSide()->GetArcadedObject() == this)
                {
                    m_WeaponDir = g_MatrixMap->m_TraceStopPos;

                    //m_WeaponDir.x = g_MatrixMap->m_TraceStopPos.x;
                    //m_WeaponDir.y = g_MatrixMap->m_TraceStopPos.y;
                    //if(IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj)) m_WeaponDir.z = g_MatrixMap->m_TraceStopObj->GetGeoCenter().z;
                    //else
                    //m_WeaponDir.z = g_MatrixMap->m_TraceStopPos.z;
                }
                else
                {
                    /*
                    D3DXVECTOR2 napr = D3DXVECTOR2(f1, f2) - D3DXVECTOR2(m_PosX, m_PosY);
                    D3DXVECTOR2 naprN;
                    D3DXVec2Normalize(&naprN, &napr);

                    float Cos = m_ChassisForward.x * naprN.x + m_ChassisForward.y * naprN.y;
                    float needAngle = float(acos(Cos));

                    if(fabs(needAngle) <= MAX_HULL_ANGLE) SetHullTargetDirection(D3DXVECTOR3(f1, f2, 0));
                    else RotateRobotChassis(D3DXVECTOR3(f1, f2, 0));
                    */
                    m_WeaponDir.x = f1;
                    m_WeaponDir.y = f2;
                    m_WeaponDir.z = f3;
                }

                for(int nC = 0; nC < m_WeaponsCount; ++nC)
                {
                    if(m_Weapons[nC].m_On && m_Weapons[nC].IsEffectPresent())
                    {
                        int weap_type = m_Weapons[nC].GetWeaponNum();
                        if(g_Config.m_WeaponsConsts[weap_type].is_repairer)
                        {
                            //Тип 2 означает, что выбранная для ведения огня цель дружественна стреляющему
                            if(type == 2) m_Weapons[nC].FireBegin(m_Velocity * (1.0f / LOGIC_TACT_PERIOD), this);
                            else m_Weapons[nC].FireEnd();
                        }
                        else if(weap_type == WEAPON_MORTAR)
                        {
                            if(!type) m_Weapons[nC].FireBegin(D3DXVECTOR3(f1, f2, f3), this);
                            else m_Weapons[nC].FireEnd();
                        }
                        else
                        {
                            if(!type) m_Weapons[nC].FireBegin(m_Velocity * (1.0f / LOGIC_TACT_PERIOD), this);
                            else m_Weapons[nC].FireEnd();
                        }
                    }
                }
                break;
            }
            case ROT_STOP_FIRE:
            {
                LowLevelStopFire();
                RemoveOrderFromTop();
                continue;
            }
            case ROT_CAPTURE_BUILDING:
            {
                building = (CMatrixBuilding*)m_OrdersList[0].GetStatic();
                if(building == nullptr) break;
                building->SetCapturedBy(this);

                int ord_phase = m_OrdersList[0].GetOrderPhase();

                //Подготовительная фаза перед захватом, определяем точку начала захвата базы/завода и выдаём роботу приказ движения в эту точку (приказ движения будет временно вынесен на первое место)
                if(ord_phase == ROP_EMPTY_PHASE)
                {
                    if(fabs((double)m_PosX - building->m_Pos.x) < 2.0f && fabs((double)m_PosY - building->m_Pos.y) < 2.0f)
                    {
                        m_OrdersList[0].SetPhase(ROP_CAPTURE_SETTING_UP);
                        break;
                    }

                    m_OrdersList[0].SetPhase(ROP_CAPTURE_MOVING); //Выставляется фаза для приказа захвата
                    if(building->IsBase())
                    {
                        //Этой строкой мы задаём направление и удалённость точки, в которую будет направлен робот перед захватом базы
                        //(то есть перед тем, как он получит приказ идти к лифту)
                        //Число 85.0f означает дистанцию удаления от лифта базы, на которую робот приблизиться перед началом захвата
                        D3DXVECTOR2 tgtpos(building->m_Pos + (*(D3DXVECTOR2*)&building->GetMatrix()._21) * 85.0f);

                        mx = TruncFloat(tgtpos.x / GLOBAL_SCALE_MOVE - (ROBOT_MOVECELLS_PER_SIZE / 2));
                        my = TruncFloat(tgtpos.y / GLOBAL_SCALE_MOVE - (ROBOT_MOVECELLS_PER_SIZE / 2));
                        g_MatrixMap->PlaceFindNear(m_Module[0].m_Kind - 1, 4, mx, my, this);
                    }
                    else
                    {
                        mx = TruncFloat(building->m_Pos.x / GLOBAL_SCALE_MOVE - (ROBOT_MOVECELLS_PER_SIZE / 2));
                        my = TruncFloat(building->m_Pos.y / GLOBAL_SCALE_MOVE - (ROBOT_MOVECELLS_PER_SIZE / 2));
                        g_MatrixMap->PlaceFindNear(m_Module[0].m_Kind - 1, 4, mx, my, this);
                    }

                    MoveTo(mx, my);
                    m_OrdersList[0].SetPhase(ROP_CAPTURE_MOVING); //Выставляется фаза для приказа движения (который теперь основной)
                    --cnt;
                }
                //Если робот ещё не добрался до точки начала захвата здания, отправляем его туда
                else if(ord_phase == ROP_CAPTURE_MOVING)
                {
                    if(building->IsBase())
                    {
                        D3DXVECTOR3 dist = (building->m_Pos + (*(D3DXVECTOR2*)&building->GetMatrix()._21) * 85.0f) - D3DXVECTOR2(m_PosX, m_PosY);
                        dist.z = 0;

                        //dist += (*(D3DXVECTOR2*)&building->GetMatrix()._21) * 100.0f;
                        if(!FindOrderLikeThat(ROT_MOVE_TO, ROP_CAPTURE_MOVING) && D3DXVec3LengthSq(&dist) <= 45 * 45)//(BASE_DIST + 60) * (BASE_DIST + 60))
                        {
                            m_OrdersList[0].SetPhase(ROP_CAPTURE_IN_POSITION);
                        }
                    }
                    else
                    {
                        D3DXVECTOR3 dist = building->m_Pos - D3DXVECTOR2(m_PosX, m_PosY);
                        dist.z = 0;

                        if(!FindOrderLikeThat(ROT_MOVE_TO, ROP_CAPTURE_MOVING) && D3DXVec3LengthSq(&dist) <= (BASE_DIST) * (BASE_DIST))// (fabs(TruncFloat(m_PosX) - building->m_Pos.x) < 5 && fabs(TruncFloat(m_PosY) - building->m_Pos.y) < 5))
                        {
                            m_OrdersList[0].SetPhase(ROP_CAPTURE_IN_POSITION);
                        }
                    }
                }
                //Робот добрался до точки начала захвата здания, отдаём непосредственный приказ на захват
                else if(ord_phase == ROP_CAPTURE_IN_POSITION)
                {
                    if(building->IsBase())
                    {
                        SetManualControlLocked(true); //Обязательно выставляем маркер запрета ручного контроля в начале этого блока
                        SetBase(building);
                        //this->UnSelect(); //Игрок же всё равно сможет его потом перевыбрать, так что пофиг
                        if(building->m_State != BASE_OPENED && building->m_State != BASE_OPENING)
                        {
                            building->Open();
                            break;
                        }
                        else if(building->m_State != BASE_OPENED) break;

                        LowLevelMove(ms, D3DXVECTOR3(building->m_Pos.x, building->m_Pos.y, 0), true, false);
                    }
                    else LowLevelMove(ms, D3DXVECTOR3(building->m_Pos.x, building->m_Pos.y, 0), true, true);

                    //Робот дошёл до центра точки захвата здания (лифта для базы)
                    if(fabs((double)m_PosX - building->m_Pos.x) < 2.0f && fabs((double)m_PosY - building->m_Pos.y) < 2.0f)
                    {
                        m_OrdersList[0].SetPhase(ROP_CAPTURE_SETTING_UP);

                        //Величайший баг тысячелетия происходит именно здесь:
                        //Если не "прибивать робота к месту" костылём, то время от времени он будет пытаться сбежать с лифта в момент начала разворота
                        //Почему и откуда это происходит, я в душе не ведаю, вероятно, один из приказов MoveTo каким-то хуем умудряется "пробить защиту", причём в принципе любую
                        if(building->IsBase())
                        {
                            SetNoObstaclesCollision(true); //Выставляем, чтобы избежать багов с телепортацией робота за пределы базы
                            //SetNoMoreEverMovingOverride(true); //Чтобы нахер приколотить гвоздями эту суку к лифту! ДА СТОЙ ЖЕ ТЫ НА МЕСТЕ, УЁБИЩЕ!!!
                        }
                        SetNoMoreEverMovingOverride(true);
                        return;
                    }
                }
                //Робот добрался до центра лифта базы, разворачиваем его
                else if(ord_phase == ROP_CAPTURE_SETTING_UP)
                {
                    if(RotateRobotChassis(D3DXVECTOR3(m_PosX + building->GetMatrix()._21, m_PosY + building->GetMatrix()._22, 0)))
                    {
                        m_OrdersList[0].SetPhase(ROP_CAPTURING);
                        return;
                    }
                }
                else if(ord_phase == ROP_CAPTURING)
                {
                    CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
                    if(ps->m_CurrSel == BUILDING_SELECTED || ps->m_CurrSel == BASE_SELECTED)
                    {
                        CMatrixBuilding* bld = ((CMatrixBuilding*)ps->m_ActiveObject);
                        if(bld == building)
                        {
                            ps->Select(NOTHING, nullptr);
                            ps->PLDropAllActions();
                        }
                    }

                    //Если робот добрался до лифта базы и уже выполнил разворот, опускаем лифт, захватываем базу и удаляем самого робота
                    if(building->IsBase())
                    {
                        if(building->m_State != BASE_CLOSED && building->m_State != BASE_CLOSING)
                        {
                            m_CurrState = ROBOT_CAPTURING_BASE;
                            building->Close();
                        }
                        else if(building->m_State == BASE_CLOSED)
                        {
                            if(m_Side == PLAYER_SIDE) CSound::Play(S_ENEMY_BASE_CAPTURED);
                            else if(building->m_Side == PLAYER_SIDE) CSound::Play(S_PLAYER_BASE_CAPTURED);

                            if(building->GatheringPointIsSet()) building->ClearGatheringPoint();

                            building->m_Side = m_Side;
                            building->m_BS.ClearStack();
                            g_MatrixMap->StaticDelete(this);
                            return;
                        }
                    }
                    else
                    {
                        //Завершаем захват, если завод уже захвачен
                        if(building->Capture(this) == CAPTURE_DONE)
                        {
                            SetNoMoreEverMovingOverride(false);
                            RemoveOrderFromTop();
                            continue;
                        }
                    }
                }
                break;
            }
            case ROT_STOP_CAPTURE:
            {
                RemoveOrderFromTop();
                continue; //Крутим цикл приказов дальше
            }
            case ROT_MOVE_TO:
            case ROT_MOVE_TO_BACK:
            {
                if(this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject())
                {
                    if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD_ALT]) & 0x8000) == 0x8000))
                    {
                        D3DXVECTOR3 dest(m_PosX, m_PosY, 0);
                        dest += m_ChassisForward * m_maxSpeed;
                        LowLevelMove(ms, dest, true, true, false);
                    }
                    else if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD_ALT]) & 0x8000) == 0x8000))
                    {
                        m_ChassisCamForward = m_ChassisForward; //Обновляем привязку камеры на случай, если до этого игрок производил стрейф

                        D3DXVECTOR3 dest(m_PosX, m_PosY, 0);
                        dest -= m_ChassisForward * m_maxSpeed;
                        LowLevelMove(ms, dest, true, true, false, UNIT_MOVING_BACKWARD, false);
                    }
                    else StopMoving();

                    MapPosCalc();
                    break;
                }

                int i;
                for(i = 0; i < m_OrdersInPool; ++i)
                {
                    if(m_OrdersList[i].GetOrderType() == ROT_CAPTURE_BUILDING && m_OrdersList[i].GetOrderPhase() == ROP_CAPTURE_IN_POSITION)
                    {
                        CMatrixBuilding* building = (CMatrixBuilding*)m_OrdersList[i].GetStatic();
                        if(building && building->IsBase()) break;
                    }
                }
                if(i < m_OrdersInPool) break;

                //int x0 = TruncFloat(robot_pos.x * INVERT(GLOBAL_SCALE_MOVE)) - COLLIDE_FIELD_R;
                //int y0 = TruncFloat(robot_pos.y * INVERT(GLOBAL_SCALE_MOVE)) - COLLIDE_FIELD_R;

                //g_MatrixMap->PlaceGet(m_Module[0].m_Kind - 1, m_PosX - 20.0f, m_PosY - 20.0f, &m_MapX, &m_MapY);
                MapPosCalc();

                CHECK_ROBOT_POS();
                //if(!(g_MatrixMap->PlaceFindNear(m_Module[0].m_Kind - 1, 4, m_MapX, m_MapY, this))) ERROR_E;

                ZoneCurFind();

                if(m_ZoneCur < 0)
                {
                    if(m_MovePathCnt > 0) MoveByMovePath(ms);
                    break;
                }

                /*
                if(m_ZonePathNext >= 0 && m_ZonePathNext < m_ZonePathCnt)
                {
                    if(m_ZoneCur == m_ZonePath[m_ZonePathNext])
                    {
                        ++m_ZonePathNext;
                        m_MovePathCur = 0;
                        m_MovePathCnt = 0;
                    }
                }
                */
                if(m_ZonePathNext >= 0 && m_ZonePathNext < m_ZonePathCnt)
                {
                    if(m_MovePathDistFollow > m_MovePathDist * 0.7)
                    {
                        for(int i = m_ZonePathNext; i < m_ZonePathCnt; ++i)
                        {
                            if(m_ZonePath[i] == m_ZoneCur)
                            {
                                m_ZonePathNext = i + 1;
                                m_MovePathCur = 0;
                                m_MovePathCnt = 0;
                                break;
                            }
                        }

                        if(i >= m_ZonePathCnt)
                        {
                            m_ZonePathCnt = 0;
                            m_MovePathCur = 0;
                            m_MovePathCnt = 0;
                        }
                    }
                }

                if(m_MovePathCnt > 0)
                {
                    MoveByMovePath(ms);
                    break;
                }
                if(m_ZoneDes < 0)
                {
                    m_ZoneDes = g_MatrixMap->ZoneFindNear(m_Module[0].m_Kind - 1, m_DesX, m_DesY);
                    m_ZonePathCnt = 0;
                }

                if(m_ZonePathCnt <= 0) ZonePathCalc();

                ZoneMoveCalc();
                if(m_MovePathCnt <= 0) StopMoving();

                break;
            }
            case ROT_MOVE_RETURN:
            {
DCP();
                if(!FindOrderLikeThat(ROT_MOVE_TO))
                {
                    m_OrdersList[0].GetParams(&f1, &f2, nullptr, nullptr);
                    if(g_MatrixMap->PlaceIsEmpty(m_Module[0].m_Kind - 1, 4, Float2Int(f1), Float2Int(f2), this))
                    {
                        RemoveOrderFromTop();
                        MoveTo(Float2Int(f1), Float2Int(f2));
                        continue;
                    }
                }
                break;
            }
            case ROT_STOP_MOVE:
            {
DCP();
                LowLevelStop();
                RemoveOrderFromTop();

                SETFLAG(m_ObjectFlags, ROBOT_FLAG_COLLISION);
                RChange(MR_Matrix | MR_ShadowProjTex | MR_ShadowStencil);
                continue;
            }
        }

        //Process next order
        ProcessOrdersList(); //Первый приказ в списке удаляется, остальной список приказов сдвигается на один вверх
        ++cnt;
    }
DCP();

    //{
    //    if(this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject() && !m_Speed)
    //    {
    //        CMatrixMapStatic* statics = CMatrixMapStatic::GetFirstLogic();
    //        
    //        while(statics)
    //        {
    //            if(statics->GetObjectType() == OBJECT_TYPE_BUILDING && ((CMatrixBuilding*)statics)->m_Side != m_Side)
    //            {
    //                CMatrixBuilding* eve_factory = (CMatrixBuilding*)statics;
    //                if(!eve_factory->m_BusyFlag.IsBusy() || (eve_factory->m_BusyFlag.IsBusy() && eve_factory->m_BusyFlag.GetBusyBy() != this))
    //                {
    //                   D3DXVECTOR3 res = m_Core->m_GeoCenter - D3DXVECTOR3(eve_factory->m_Pos.x, eve_factory->m_Pos.y, 0);
    //                   if(D3DXVec3LengthSq(&res) < 800 && eve_factory->Capture(this) == CAPTURE_DONE) eve_factory->m_BusyFlag.Reset();
    //                }
    //            }
    //            statics = statics->GetNextLogic();
    //        }
    //    }
    //}

    {
        for(int nC = 0; nC < m_WeaponsCount; ++nC)
        {
            if(m_Weapons[nC].IsEffectPresent() && (m_Weapons[nC].m_On || g_MatrixMap->GetPlayerSide()->GetArcadedObject() == this))
            {
                //m_WeaponDir.x = g_MatrixMap->m_TraceStopPos.x;
                //m_WeaponDir.y = g_MatrixMap->m_TraceStopPos.y;
                //m_WeaponDir.z = g_MatrixMap->m_TraceStopPos.z;

                //Если оружие перегрелось, прекращаем стрельбу
                if(m_Weapons[nC].m_Heat + m_Weapons[nC].m_HeatingSpeed > WEAPON_MAX_HEAT)
                {
                    m_Weapons[nC].FireEnd();
                    continue;
                }

                float fire_dist = m_Weapons[nC].GetWeaponDist();
                D3DXVECTOR3 vPos(0, 0, 0), vDir(0, 0, 0);
                vDir = m_WeaponDir;

                D3DXMATRIX m = (*m_Weapons[nC].m_Module->m_Graph->GetMatrixById(1)) * m_Weapons[nC].m_Module->m_Matrix;
                D3DXVec3TransformCoord(&vPos, &vPos, &m);

                D3DXVECTOR3 vWeapPos = D3DXVECTOR3(m._21, m._22, m._23);

                vDir.x -= vPos.x;
                vDir.y -= vPos.y;
                vDir.z -= vPos.z;

                D3DXVec3Normalize(&vDir, &vDir);

                //CHECKRIDE
                //Fire distance checkride
                if(this != g_MatrixMap->GetPlayerSide()->GetArcadedObject())
                {
                    if(D3DXVec3LengthSq(&(GetGeoCenter() - m_WeaponDir)) > fire_dist * fire_dist)
                    {
                        m_Weapons[nC].FireEnd();
                        continue;
                    }
                }

                //Особые обработчики для некоторых видов оружия
                byte prim_weap_type = g_Config.m_WeaponsConsts[m_Weapons[nC].GetWeaponNum()].primary_effect;
                if(prim_weap_type == EFFECT_MORTAR)
                {
                    m_Weapons[nC].Modify(vPos, vWeapPos, m_WeaponDir);
                    m_Weapons[nC].Tact(float(ms));

                    if(m_Weapons[nC].IsFireWas())
                    {
                        //FIREINDAHOLE:
                        m_Weapons[nC].m_Heat = min(m_Weapons[nC].m_Heat + m_Weapons[nC].m_HeatingSpeed, WEAPON_MAX_HEAT);

                        if(m_Weapons[nC].m_Module->m_Graph->SetAnimByNameNoBegin(ANIMATION_NAME_FIRELOOP))
                        {
                            // not looped
                            m_Weapons[nC].m_Module->m_Graph->SetAnimByName(ANIMATION_NAME_FIRE, 0);
                        }
                    }
                    continue;
                }
                else if(prim_weap_type == EFFECT_BOMB)
                {
                    //m_Weapons[nC].m_Weapon->Modify(vPos, vDir, m_Velocity * (1.0f / LOGIC_TACT_PERIOD));				
                    //m_Weapons[nC].m_Weapon->Tact(float(ms));
                    continue;
                }
                else if(prim_weap_type == EFFECT_ROCKET_LAUNCHER)
                {
                    //Special homing_missile handler
                    m_Weapons[nC].Modify(vPos, vDir, m_Velocity * (1.0f / LOGIC_TACT_PERIOD));
                    m_Weapons[nC].Tact(float(ms));
                    if(m_Weapons[nC].IsFireWas())
                    {
                        //FIREINDAHOLE:
                        m_Weapons[nC].m_Heat = min(m_Weapons[nC].m_Heat + m_Weapons[nC].m_HeatingSpeed, WEAPON_MAX_HEAT);
                    }
                    continue;
                }

                //Стандартный обработчик для всех прочих видов оружия
                m_Weapons[nC].Modify(vPos, vDir, m_Velocity * (1.0f / LOGIC_TACT_PERIOD));

                //Common weapons handler
                //Angle checkride

                //g_MatrixMap->m_DI.T(L"d", CWStr(fire_dist));
                //g_MatrixMap->m_DI.T(L"d1", CWStr(D3DXVec3Length(&vDir)));
                //g_MatrixMap->m_DI.T(L"d2", CWStr(D3DXVec3Length(&D3DXVECTOR3(vPosNorm.x, vPosNorm.y, vDir.z))));

                //CHelper::Create(1, 0)->Line(vPos, vPos + vDir * fire_dist, 0xffff0000, 0xffff0000);
                //CHelper::Create(1, 0)->Line(vPos, vPos + D3DXVECTOR3(vPosNorm.x, vPosNorm.y, vDir.z) * fire_dist);

                D3DXVECTOR2 dir1, dir2;
                D3DXVec2Normalize(&dir1, (D3DXVECTOR2*)&vWeapPos);
                D3DXVec2Normalize(&dir2, (D3DXVECTOR2*)&vDir);

                //float cos = vPosNorm.x * vDir.x + vPosNorm.y * vDir.y + /*vPosNorm.z*/vDir.z * vDir.z;

                float angle = acos(D3DXVec2Dot(&dir1, &dir2));
                //CDText::T("angle", CStr(angle));
                //CDText::T("cos", CStr(cos));
                if(angle > BARREL_TO_SHOT_ANGLE && !g_Config.m_WeaponsConsts[m_Weapons[nC].GetWeaponNum()].is_repairer)
                {
                    m_Weapons[nC].FireEnd();
                    continue;
                }

                m_Weapons[nC].Tact(float(ms));

                //Если был произведён выстрел
                if(m_Weapons[nC].IsFireWas())
                {
                    //Накручиваем перегрев
                    m_Weapons[nC].m_Heat = min(m_Weapons[nC].m_Heat + m_Weapons[nC].m_HeatingSpeed, WEAPON_MAX_HEAT);

                    //Крутим анимацию стрельбы
                    if(m_Weapons[nC].m_Module->m_Graph->SetAnimByNameNoBegin(ANIMATION_NAME_FIRELOOP))
                    {
                        // not looped
                        m_Weapons[nC].m_Module->m_Graph->SetAnimByName(ANIMATION_NAME_FIRE, 0);
                    }
                }
            }
        }
    }

    for(int cnt = 0; cnt < m_WeaponsCount; ++cnt)
    {
        if(m_Weapons[cnt].IsEffectPresent())
        {
            //Пока оружие нагрето, постоянно накручиваем задержку до следующего охлаждения, иначе сбрасываем её
            if(m_Weapons[cnt].m_Heat)
            {
                m_Weapons[cnt].m_CoolingDelay += ms;

                int period = g_Config.m_RobotWeaponsConsts[m_Weapons[cnt].m_Module->m_Kind].cooling_delay;

                //Когда таймер задержки до охлаждения достиг заданного рубежа, охлаждаем оружие и сбрасываем таймер (ожидаем, когда таймер дойдет до момента охлаждения вновь)
                while(m_Weapons[cnt].m_Heat && m_Weapons[cnt].m_CoolingDelay >= period)
                {
                    m_Weapons[cnt].m_Heat = max(m_Weapons[cnt].m_Heat - m_Weapons[cnt].m_CoolingSpeed, 0);
                    m_Weapons[cnt].m_CoolingDelay -= period;
                }
            }
            else m_Weapons[cnt].m_CoolingDelay = 0;
        }
    }

    goto do_animation;
}



void CMatrixRobotAI::ZoneCurFind()
{
    SMatrixMapMove* smm = g_MatrixMap->MoveGet(m_MapX, m_MapY);
    if(smm && smm->m_Zone >= 0)
    {
        m_ZoneCur = smm->m_Zone;
        return;
    }

    /*
    mu=g_MatrixMap->UnitGet(m_MapX+1,m_MapY+1);
    if(mu && mu->m_Zone[m_Module[0].m_Kind - 1] >= 0) { m_ZoneCur = mu->m_Zone[m_Module[0].m_Kind - 1]; return;	}

    mu=g_MatrixMap->UnitGet(m_MapX+1,m_MapY);
    if(mu && mu->m_Zone[m_Module[0].m_Kind - 1] >= 0) { m_ZoneCur = mu->m_Zone[m_Module[0].m_Kind - 1]; return;	}

    mu=g_MatrixMap->UnitGet(m_MapX,m_MapY+1);
    if(mu && mu->m_Zone[m_Module[0].m_Kind - 1] >= 0) { m_ZoneCur = mu->m_Zone[m_Module[0].m_Kind - 1]; return;	}
    */

    //ERROR_E;
    //m_ZoneCur=g_MatrixMap->ZoneFindNear(m_Module[0].m_Kind-1,m_MapX,m_MapY);

    /*
    //m_MovePathCnt
    int m_MovePathCnt=g_MatrixMap->ZoneMoveFindNear(m_Module[0].m_Kind-1,m_MapX,m_MapY,m_MovePath);
    if(newcnt <= 0)
    {
        SMatrixMapUnit * mu=g_MatrixMap->UnitGet(m_MapX,m_MapY);
        if(mu && mu->m_Zone[m_Module[0].m_Kind-1]>=0) m_ZoneCur=mu->m_Zone[m_Module[0].m_Kind-1];
        else m_ZoneCur=-1;
    }
    else m_ZoneCur = -1;
    */
}

void CMatrixRobotAI::ZonePathCalc()
{
    if(m_ZoneCur < 0) return;
    ASSERT(m_ZoneCur >= 0);

    if(!m_ZonePath) m_ZonePath = (int*)HAlloc(g_MatrixMap->m_RoadNetwork.m_ZoneCnt * sizeof(int), g_MatrixHeap);
    //m_ZonePathCnt = g_MatrixMap->ZoneFindPath(m_Module[0].m_Kind-1, m_ZoneCur, m_ZoneDes, m_ZonePath);

    CMatrixSideUnit* side = g_MatrixMap->GetSideById(GetSide());
    if(GetSide() == PLAYER_SIDE && GetGroupLogic() >= 0 && side->m_PlayerGroup[GetGroupLogic()].m_RoadPath->m_ListCnt > 0)
    {
        m_ZonePathCnt = g_MatrixMap->FindPathInZone(m_Module[0].m_Kind - 1, m_ZoneCur, m_ZoneDes, side->m_PlayerGroup[GetGroupLogic()].m_RoadPath, 0, m_ZonePath, g_TestRobot == this);
    }
    else if(GetSide() != PLAYER_SIDE && GetTeam() >= 0 && side->m_Team[GetTeam()].m_RoadPath->m_ListCnt > 0)
    {
        m_ZonePathCnt = g_MatrixMap->FindPathInZone(m_Module[0].m_Kind - 1, m_ZoneCur, m_ZoneDes, side->m_Team[GetTeam()].m_RoadPath, 0, m_ZonePath, g_TestRobot == this);
    }
    else
    {
        m_ZonePathCnt = g_MatrixMap->FindPathInZone(m_Module[0].m_Kind - 1, m_ZoneCur, m_ZoneDes, nullptr, 0, m_ZonePath, g_TestRobot == this);
    }
    if(m_ZonePathCnt > 0)
    {
        ASSERT(m_ZonePathCnt >= 2);
        m_ZonePathNext = 1;
    }
    else m_ZonePathNext = -1;

    //Если дойти не можем, то меняем команду
    if(GetSide() != PLAYER_SIDE && m_ZoneCur != m_ZoneDes && m_ZonePathCnt <= 0)
    {
        SetTeam(g_MatrixMap->GetSideById(GetSide())->ClacSpawnTeam(GetRegion(), m_Module[0].m_Kind - 1));
        SetGroupLogic(-1);
    }
}

void CMatrixRobotAI::ZoneMoveCalc()
{
    if(m_ZoneCur < 0) return;
    ASSERT(m_ZoneCur >= 0);
    //ASSERT(m_ZoneNear >= 0);

    int other_cnt = 0;
    int other_size[200];
    CPoint* other_path_list[200];
    int other_path_cnt[200];
    CPoint other_des[200];

    m_MoveTestChange = g_MatrixMap->GetTime();

    CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
    while(obj)
    {
        if(obj->IsRobotAlive() && obj != this)
        {
            CMatrixRobotAI* r = (CMatrixRobotAI*)obj;
            CPoint tp;
            if(r->GetMoveToCoords(tp))
            {
                ASSERT(other_cnt < 200);
                other_size[other_cnt] = 4;
                if(/*r->m_MovePath &&*/ r->m_MovePathCur < r->m_MovePathCnt) //Статический массив не может быть null
                {
                    other_path_list[other_cnt] = r->m_MovePath + r->m_MovePathCur;
                    other_path_cnt[other_cnt] = r->m_MovePathCnt - r->m_MovePathCur;
                }
                else
                {
                    other_path_list[other_cnt] = nullptr;
                    other_path_cnt[other_cnt] = 0;
                }
                other_des[other_cnt] = CPoint(r->m_DesX, r->m_DesY);
                ++other_cnt;
            }
            else
            {
                ASSERT(other_cnt < 200);
                other_size[other_cnt] = 4;
                other_path_list[other_cnt] = nullptr;
                other_path_cnt[other_cnt] = 0;
                other_des[other_cnt] = CPoint(r->m_MapX, r->m_MapY);
                ++other_cnt;
            }
        }
        else if(obj->IsCannonAlive())
        {
            ASSERT(other_cnt < 200);
            other_size[other_cnt] = 4;
            other_path_list[other_cnt] = nullptr;
            other_path_cnt[other_cnt] = 0;
            other_des[other_cnt] = CPoint(Float2Int(obj->AsCannon()->m_Pos.x / GLOBAL_SCALE_MOVE) - ROBOT_MOVECELLS_PER_SIZE / 2, Float2Int(obj->AsCannon()->m_Pos.y / GLOBAL_SCALE_MOVE) - ROBOT_MOVECELLS_PER_SIZE / 2);
            ++other_cnt;
        }
        obj = obj->GetNextLogic();
    }

    if(m_ZonePathNext >= 0)
    {
        m_MovePathCnt = g_MatrixMap->FindLocalPath(m_Module[0].m_Kind - 1, 4, m_MapX, m_MapY, m_ZonePath + m_ZonePathNext - 1, m_ZonePathCnt - (m_ZonePathNext - 1), m_DesX, m_DesY, m_MovePath,
            other_cnt, other_size, other_path_list, other_path_cnt, other_des, g_TestRobot == this);
    }
    else
    {
        m_MovePathCnt = g_MatrixMap->FindLocalPath(m_Module[0].m_Kind - 1, 4, m_MapX, m_MapY, &m_ZoneCur, 1, m_DesX, m_DesY, m_MovePath,
            other_cnt, other_size, other_path_list, other_path_cnt, other_des, g_TestRobot == this);
    }

    /*
    int zonesou1 = m_ZoneCur;
    int zonesou2 = m_ZoneCur;
    int zonesou3 = m_ZoneCur;
    if(m_ZonePathNext >= 0 && m_ZonePathNext < m_ZonePathCnt)
    {
        zonesou1 = zonesou2 = zonesou3 = m_ZonePath[m_ZonePathNext];
        if ((m_ZonePathNext + 1) < m_ZonePathCnt) zonesou2 = zonesou3 = m_ZonePath[m_ZonePathNext + 1];
    }
    if((m_ZonePathNext + 2) >= m_ZonePathCnt)
    {
        m_MovePathCnt = g_MatrixMap->ZoneMoveFind(m_Module[0].m_Kind - 1, 4, m_MapX, m_MapY, m_ZoneCur, zonesou1, -1, zonesou3, m_DesX, m_DesY, m_MovePath);
    }
    else if((m_ZonePathNext + 2) < m_ZonePathCnt)//&& m_MovePathCnt < 3)
    {
        zonesou3 = m_ZonePath[m_ZonePathNext + 2];
        m_MovePathCnt = g_MatrixMap->ZoneMoveFind(m_Module[0].m_Kind - 1, 4, m_MapX, m_MapY, m_ZoneCur, zonesou1, zonesou2, zonesou3, m_DesX, m_DesY, m_MovePath);
    }
    */

    m_MovePathCnt = g_MatrixMap->OptimizeMovePath(m_Module[0].m_Kind - 1, 4, m_MovePathCnt, m_MovePath);
    m_MovePathCur = 0;

    m_MovePathDist = 0.0f;
    m_MovePathDistFollow = 0.0f;
    for (int i = 1; i < m_MovePathCnt; ++i)
    {
        m_MovePathDist += GLOBAL_SCALE_MOVE * sqrt(float(m_MovePath[i - 1].Dist2(m_MovePath[i])));
    }
}

/*
void CMatrixRobotAI::ZoneMoveCalcTo()
{
    ASSERT(m_ZoneCur >= 0);
    m_MovePathCnt = g_MatrixMap->ZoneMoveIn(m_Module[0].m_Kind-1, 4, m_MapX, m_MapY, m_DesX, m_DesY, m_MovePath);
    m_MovePathCnt = g_MatrixMap->OptimizeMovePath(m_Module[0].m_Kind-1, m_MovePathCnt, m_MovePath);
    m_MovePathCur = 0;
}
*/

float CMatrixRobotAI::CalcPathLength()
{
    float dist = 0.0f;
    for(int i = m_MovePathCur; i < m_MovePathCnt - 1; ++i)
    {
        dist += sqrt(float(POW2(GLOBAL_SCALE_MOVE * m_MovePath[i].x - GLOBAL_SCALE_MOVE * m_MovePath[i + 1].x) + POW2(GLOBAL_SCALE_MOVE * m_MovePath[i].y - GLOBAL_SCALE_MOVE * m_MovePath[i + 1].y)));
    }
    return dist;
}

void CMatrixRobotAI::MoveByMovePath(int ms)
{
    ASSERT(m_MovePathCnt > 0);
    ASSERT(m_MovePathCur >= 0 || m_MovePathCur < m_MovePathCnt - 1);

    float sou_x = GLOBAL_SCALE_MOVE * m_MovePath[m_MovePathCur].x + GLOBAL_SCALE_MOVE * 2.0f;// + GLOBAL_SCALE;// + 5.0f;
    float sou_y = GLOBAL_SCALE_MOVE * m_MovePath[m_MovePathCur].y + GLOBAL_SCALE_MOVE * 2.0f;// + GLOBAL_SCALE;// + 5.0f;
    float des_x = GLOBAL_SCALE_MOVE * m_MovePath[m_MovePathCur + 1].x + GLOBAL_SCALE_MOVE * 2.0f;// + GLOBAL_SCALE;// + 5.0f;
    float des_y = GLOBAL_SCALE_MOVE * m_MovePath[m_MovePathCur + 1].y + GLOBAL_SCALE_MOVE * 2.0f;// + GLOBAL_SCALE;// + 5.0f;

    //float v_x = des_x - sou_x;
    //float v_y = des_y - sou_y;

    bool globalend = (m_DesX == m_MovePath[m_MovePathCur + 1].x) && (m_DesY == m_MovePath[m_MovePathCur + 1].y);

    LowLevelMove(ms, D3DXVECTOR3(des_x, des_y, 0), true, true, globalend && (m_MovePathCur + 1) == (m_MovePathCnt - 1));

    D3DXVECTOR2 vMe = D3DXVECTOR2(m_PosX, m_PosY) - D3DXVECTOR2(sou_x, sou_y);
    D3DXVECTOR2 vPath = D3DXVECTOR2(des_x, des_y) - D3DXVECTOR2(sou_x, sou_y);

    D3DXVECTOR3 vMeProj = Vec3Projection(D3DXVECTOR3(vPath.x, vPath.y, 0), D3DXVECTOR3(vMe.x, vMe.y, 0));

    float lengthMeProjSq = D3DXVec3LengthSq(&vMeProj);
    float lengthPathSq = D3DXVec2LengthSq(&vPath);

    if (
        (!globalend && (lengthMeProjSq >= lengthPathSq)) ||
        (globalend && (POW2(m_PosX - des_x) + POW2(m_PosY - des_y) < 0.2f))
        )
    {
        ++m_MovePathCur;
        if (m_MovePathCur >= m_MovePathCnt - 1)
        {
            ++m_ZonePathNext;
            if (m_ZonePathNext < m_ZonePathCnt)
            {
                m_MovePathCur = 0;
                m_MovePathCnt = 0;
            }
            else
            {
                m_PosX = des_x;
                m_PosY = des_y;
                StopMoving();
            }
        }
    }

    // Если долго стоим на месте, то перерассчитать маршрут
    if ((POW2(m_MoveTestPos.x - m_PosX) + POW2(m_MoveTestPos.y - m_PosY)) > POW2(5.0f))
    {
        m_MoveTestPos.x = m_PosX;
        m_MoveTestPos.y = m_PosY;
        m_MoveTestChange = g_MatrixMap->GetTime();
    }
    else if ((g_MatrixMap->GetTime() - m_MoveTestChange) > 2000)
    {
        m_ZonePathCnt = 0;
        m_MovePathCur = 0;
        m_MovePathCnt = 0;
    }

    RChange(MR_Matrix | MR_Rotate | MR_Pos | MR_ShadowStencil | MR_ShadowProjGeom | MR_ShadowProjTex);
}

/*
void CMatrixRobotAI::LowOrderStop()
{
    m_LowOrder = 0;
    m_ZoneDes = -1;
    m_ZonePathCnt = 0;
    m_ZonePathNext = -1;
    //m_ZoneNear = -1;
    m_MovePathCnt = 0;
    m_MovePathCur = 0;
}

void CMatrixRobotAI::LowOrderMoveTo(int mx, int my)
{
    LowOrderStop();
    m_DesX = mx;
    m_DesY = my;
    m_LowOrder = 1;
}
*/


/*
void CMatrixRobotAI::MoveToRndBuilding()
{
    CMatrixMapStatic* ms = g_MatrixMap->m_StaticFirstNT;
    while(ms)
    {
        if(ms->GetObjectType() == OBJECT_TYPE_BUILDING && (g_MatrixMap->Rnd(0, 10)<=5))// && ((CMatrixBuilding*)ms)->m_Kind != 0)
        {
        if(((CMatrixBuilding*)ms)->m_Side != m_Side && !((CMatrixBuilding*)ms)->m_Busy)
        {
            CaptureFactory(((CMatrixBuilding*)ms));
        }
        /*
        float x = GLOBAL_SCALE * (((CMatrixBuilding *)ms)->m_MapPos.x);
        float y = GLOBAL_SCALE * (((CMatrixBuilding *)ms)->m_MapPos.y + 3);

        if(((x - m_PosX) * (x - m_PosX) + (y - m_PosY) * (y-m_PosY)) > 100 * 100)
        {
            int mx = int(x / GLOBAL_SCALE);
            int my = int(y / GLOBAL_SCALE);

            g_MatrixMap->PlaceFindNear(m_Module[0].m_Kind - 1, 4, mx, my);

            MoveTo(mx, my);
            break;
        }
        /*
        }
        else if(ms != this && ms->GetObjectType() == OBJECT_TYPE_ROBOTAI && (g_MatrixMap->Rnd(0, 10) <= 5))
        {
        float x = (ms->AsRobot()->m_PosX);
        float y = (ms->AsRobot()->m_PosY + 30);

        if(ms->AsRobot()->m_CurrState == ROBOT_SUCCESSFULLY_BUILD && ((x - m_PosX) * (x - m_PosX) + (y - m_PosY) * (y - m_PosY)) > 100 * 100)
        {
            int mx = int(x / GLOBAL_SCALE);
            int my = int(y / GLOBAL_SCALE);

            g_MatrixMap->PlaceFindNear(m_Module[0].m_Kind - 1, 4, mx, my);

            MoveTo(mx, my);
            break;
        }

        }
        ms = ms->m_NextNT;
    }
}
*/

//Функция получения роботом урона в результате чьей-то атаки (возвращает true, если робот был уничтожен полученным уроном)
bool CMatrixRobotAI::TakingDamage(
    int weap,
    const D3DXVECTOR3& pos,
    const D3DXVECTOR3& dir,
    int attacker_side,
    CMatrixMapStatic* attaker
)
{
    ASSERT(this);
    if(m_CurrState == ROBOT_DIP) return true;

    bool friendly_fire = false;
    if(weap == WEAPON_INSTANT_DEATH) goto inst_death;
    friendly_fire = attacker_side && attacker_side == m_Side;

    float damage_coef = (friendly_fire || m_Side != PLAYER_SIDE) ? 1.0f : g_MatrixMap->m_Difficulty.coef_enemy_damage_to_player_side;
    if(friendly_fire && m_Side == PLAYER_SIDE) damage_coef = damage_coef * g_MatrixMap->m_Difficulty.coef_friendly_fire;

    CMatrixEffectWeapon::SoundHit(weap, pos);

    //Если в робота стреляют хилкой
    if(g_Config.m_WeaponsConsts[weap].is_repairer)
    {
        m_HitPoint = min(m_HitPoint + g_Config.m_WeaponsConsts[weap].damage.to_robots, m_HitPointMax);
        m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);

        return false;
    }

    if(!friendly_fire && attaker != nullptr && attaker->IsCannonAlive() && attaker->AsCannon()->GetSide() != m_Side)
    {
        if(!GetEnv()->SearchEnemy(attaker)) GetEnv()->AddToList(attaker);

        if((!GetEnv()->m_TargetAttack || GetEnv()->m_TargetAttack->IsCannon()) && (g_MatrixMap->GetTime() - GetEnv()->m_LastHitTarget) > 4000 && (g_MatrixMap->GetTime() - GetEnv()->m_TargetChange) > 1000)
        {
            GetEnv()->m_TargetLast = GetEnv()->m_TargetAttack;
            GetEnv()->m_TargetAttack = attaker;
            GetEnv()->m_TargetChange = g_MatrixMap->GetTime();
        }
    }

    //float damage = damage_coef * friendly_fire ? g_Config.m_WeaponsConsts[weap].friendly_damage.to_robots : g_Config.m_WeaponsConsts[weap].damage.to_robots;
    float damage = damage_coef * g_Config.m_WeaponsConsts[weap].damage.to_robots;
    if(weap == WEAPON_BOMB) damage += damage * m_BombProtect;
    m_HitPoint = max(m_HitPoint - damage, g_Config.m_WeaponsConsts[weap].non_lethal_threshold.to_robots);

    if(m_HitPoint >= 0) m_ProgressBar.Modify(m_HitPoint * m_MaxHitPointInversed);
    else m_ProgressBar.Modify(0);

    if(!friendly_fire) m_MiniMapFlashTime = FLASH_PERIOD;

    if(FLAG(g_MatrixMap->m_Flags, MMFLAG_FLYCAM))
    {
        if(attaker) g_MatrixMap->m_Camera.AddWarPair(this, attaker);
    }

    //Проверяем, какие эффекты накладывает попавшее в робота оружие
    if(g_Config.m_WeaponsConsts[weap].extra_effects.size())
    {
        for(int i = 0; i < (int)g_Config.m_WeaponsConsts[weap].extra_effects.size(); ++i)
        {
            SWeaponsConsts::SWeaponExtraEffect* effect = &g_Config.m_WeaponsConsts[weap].extra_effects[i];
            int effect_num = effect->type;
            byte effect_type = g_Config.m_WeaponsConsts[effect_num].secondary_effect;
            if(effect_type == SECONDARY_EFFECT_ABLAZE)
            {
                if(!g_Config.m_WeaponsConsts[effect_num].damage.to_robots) continue;
                int new_priority = g_Config.m_WeaponsConsts[effect_num].effect_priority;

                if(IsAblaze()) effect_num = new_priority >= g_Config.m_WeaponsConsts[IsAblaze()].effect_priority ? effect_num : IsAblaze(); //Если наложенный ранее эффект горения имеет более высокий приоритет, то обновляем эффект по его статам
                MarkAblaze(effect_num);
                m_LastDelayDamageSide = attacker_side;

                int ttl = GetAblazeTTL();
                ttl = min(max(ttl + effect->duration_per_hit, 0), effect->max_duration);
                SetAblazeTTL(ttl);

                m_NextTimeAblaze = g_MatrixMap->GetTime(); //То есть в первый раз считаем логику получения урона от огня немедленно
            }
            else if(effect_type == SECONDARY_EFFECT_SHORTED_OUT)
            {
                LowLevelStopFire();
                SwitchAnimation(ANIMATION_OFF);

                int new_priority = g_Config.m_WeaponsConsts[effect_num].effect_priority;
                if(IsShorted()) effect_num = new_priority >= g_Config.m_WeaponsConsts[IsShorted()].effect_priority ? effect_num : IsShorted(); //Если наложенный ранее эффект стана имеет более высокий приоритет, то обновляем эффект по его статам
                MarkShorted(effect_num);
                m_LastDelayDamageSide = attacker_side;

                int ttl = GetShortedTTL();
                float dur_per_hit = effect->duration_per_hit;
                float max_dur = effect->max_duration;
                ttl = (int)min(max(ttl + (dur_per_hit + dur_per_hit * m_DischargerProtect), 0), max_dur + max_dur * m_DischargerProtect);
                if(ttl) SetShortedTTL(ttl);

                m_NextTimeShorted = g_MatrixMap->GetTime();
            }
        }
    }
    else m_LastDelayDamageSide = 0;

    if(m_HitPoint > 50)
    {
        if(g_Config.m_WeaponsConsts[weap].explosive_hit) CMatrixEffect::CreateExplosion(pos, ExplosionRobotHit);
    }
    //else if(m_HitPoint > 0) {}
    else if(m_HitPoint <= 0)
    {
        if(attacker_side != NEUTRAL_SIDE && !friendly_fire)
        {
            g_MatrixMap->GetSideById(attacker_side)->IncStatValue(STAT_ROBOT_KILL);
        }

    inst_death:

        if(IsInPosition()) g_MatrixMap->ShowPortrets();

        for(int nC = 0; nC < m_WeaponsCount; ++nC)
        {
            if(m_Weapons[nC].IsEffectPresent() && m_Weapons[nC].GetWeaponNum() == WEAPON_BOMB)
            {
                if(GetSide() == PLAYER_SIDE)
                {
                    //Для активации по правому клику мыши по значку подрыва
                    if(AutoBoomSet() && !IsShorted()) BigBoom();//BigBoom(nC);
                }
                else
                {
                    float danger = 0.0f;
                    CMatrixMapStatic* ms = CMatrixMapStatic::GetFirstLogic();
                    for(; ms; ms = ms->GetNextLogic())
                    {
                        if(ms != this && ms->IsRobotAlive())
                        {
                            CMatrixRobotAI* robot = ms->AsRobot();
                            if((POW2(robot->m_PosX - m_PosX) + POW2(robot->m_PosY - m_PosY)) < POW2(m_Weapons[nC].GetWeaponDist() * 1.0f))
                            {
                                if(ms->GetSide() == GetSide()) danger -= robot->GetStrength();
                                else danger += robot->GetStrength();
                            }
                        }
                        else if(ms->IsCannonAlive() && ms->AsCannon()->m_CurrState != CANNON_UNDER_CONSTRUCTION)
                        {
                            CMatrixCannon* cannon = ms->AsCannon();
                            if((POW2(cannon->m_Pos.x - m_PosX) + POW2(cannon->m_Pos.y - m_PosY)) < POW2(m_Weapons[nC].GetWeaponDist() * 1.0f))
                            {
                                if(ms->GetSide() == GetSide()) danger -= cannon->GetStrength();
                                else danger += cannon->GetStrength();
                            }
                        }
                    }

                    if(danger > 0.0f) BigBoom();//BigBoom(nC);
                }
                break;
            }
        }

        ResetMustDie(); // to avoid MUST_DIE flag checking... robot already dying...

        //} else {
DCP();

        ReleaseMe();
DCP();
        CMatrixEffect::CreateExplosion(GetGeoCenter(), ExplosionRobotBoom, true);
        CMatrixEffect::CreateLandscapeSpot(nullptr, *(D3DXVECTOR2*)&GetGeoCenter(), FSRND(M_PI), 4 + FRND(2), SPOT_VORONKA);

DCP();
        ClearSelection();

DCP();
        m_ShadowType = SHADOW_OFF;
        RChange(MR_ShadowProjGeom | MR_ShadowStencil);
        GetResources(MR_ShadowProjGeom | MR_ShadowStencil);
DCP();
        for(int i = 0; i < m_ModulesCount; ++i) m_Module[i].PrepareForDIP();
DCP();

        //Очищаем объекты эффектов реактивных струй для шасси робота
        if(g_Config.m_RobotChassisConsts[m_Module[0].m_Kind].jet_stream.size())
        {
            for(int i = 0; i < g_Config.m_RobotChassisConsts[m_Module[0].m_Kind].jet_stream.size(); ++i)
            {
                m_ChassisData.JetStream[i]->Release();
            }

            m_ChassisData.JetStream.clear();
        }
DCP();

        bool cstay = FRND(1) < 0.5f;
        if(IsManualControlLocked())
        {
            if(GetBase())
            {
                if(m_CurrState == ROBOT_BASE_MOVEOUT || m_CurrState == ROBOT_IN_SPAWN)
                {
                    //Если робота уничтожили, пока тот выходил с базы, в любом случае опускаем подъёмник
                    GetBase()->SetSpawningUnit(false);
                }
                GetBase()->Close();
            }
            cstay = false;
        }

        if(FLAG(m_ObjectFlags, ROBOT_FLAG_ON_WATER)) cstay = false;

DCP();
        SwitchAnimation(ANIMATION_OFF);
        m_CurrState = ROBOT_DIP;

        bool onair = false;

        float z = g_MatrixMap->GetZ(m_Module[0].m_Matrix._41, m_Module[0].m_Matrix._42);
        if(z + GetRadius() < m_Module[0].m_Matrix._43)
        {
            // on air
            onair = true;
        }
DCP();

        m_Module[0].m_TTL = FRND(3000) + 2000;
        m_Module[0].m_Pos.x = m_Module[0].m_Matrix._41;
        m_Module[0].m_Pos.y = m_Module[0].m_Matrix._42;
        m_Module[0].m_Pos.z = m_Module[0].m_Matrix._43;
DCP();

        if(cstay && !onair)
        {
            m_Module[0].m_dp = 0;
            m_Module[0].m_dy = 0;
            m_Module[0].m_dr = 0;
            m_Module[0].m_Velocity = D3DXVECTOR3(0, 0, 0);
        }
        else
        {
            m_Module[0].m_dp = FSRND(0.0005f);
            m_Module[0].m_dy = FSRND(0.0005f);
            m_Module[0].m_dr = FSRND(0.0005f);

            if(onair) m_Module[0].m_Velocity = D3DXVECTOR3(FSRND(0.08f), FSRND(0.08f), FSRND(0.1f));
            else m_Module[0].m_Velocity = D3DXVECTOR3(0, 0, 0.1f);
        }

DCP();
        m_Module[0].Smoke().effect = nullptr;
        CMatrixEffect::CreateSmoke(&m_Module[0].Smoke(), m_Module[0].m_Pos, m_Module[0].m_TTL + 100000, 1600, 80, 0xCF303030, 1.0f / 30.0f);

        for(int i = 1; i < m_ModulesCount; ++i)
        {
            m_Module[i].Smoke().effect = nullptr;
            if(m_Module[i].m_Type == MRT_HULL)
            {
                m_Module[i].m_TTL = 0;
                continue;
            }

            m_Module[i].m_dp = FSRND(0.005f);
            m_Module[i].m_dy = FSRND(0.005f);
            m_Module[i].m_dr = FSRND(0.005f);

            if(onair) m_Module[i].m_Velocity = D3DXVECTOR3(FSRND(0.08f), FSRND(0.08f), FSRND(0.1f));
            else m_Module[i].m_Velocity = D3DXVECTOR3(FSRND(0.08f), FSRND(0.08f), 0.1f);

            m_Module[i].m_TTL = FRND(3000) + 2000;
            m_Module[i].m_Pos.x = m_Module[i].m_Matrix._41;
            m_Module[i].m_Pos.y = m_Module[i].m_Matrix._42;
            m_Module[i].m_Pos.z = m_Module[i].m_Matrix._43;

            CMatrixEffect::CreateSmoke(&m_Module[i].Smoke(), m_Module[i].m_Pos, m_Module[i].m_TTL + 100000, 1600, 80, 0xCF303030, 1.0f / 30.0f);
        }

DCP();
        return true;
    }
    return false;
}


#ifdef _DEBUG
void CMatrixRobotAI::Draw(void)
{
    //if(m_CurrState != ROBOT_DIP)
    // {
    //    CPoint tp;
    //    if(!GetMoveToCoords(tp)) {
    //        tp.x=m_MapX; tp.y=m_MapY;
    //    }

    //    D3DXVECTOR3 v1,v2,v3,v4;
    //    v1.x=tp.x*GLOBAL_SCALE_MOVE; v1.y=tp.y*GLOBAL_SCALE_MOVE; v1.z=g_MatrixMap->GetZ(v1.x,v1.y)+1.0f;
    //    v2.x=(tp.x+4)*GLOBAL_SCALE_MOVE; v2.y=tp.y*GLOBAL_SCALE_MOVE; v2.z=g_MatrixMap->GetZ(v2.x,v2.y)+1.0f;
    //    v3.x=(tp.x+4)*GLOBAL_SCALE_MOVE; v3.y=(tp.y+4)*GLOBAL_SCALE_MOVE; v3.z=g_MatrixMap->GetZ(v3.x,v3.y)+1.0f;
    //    v4.x=(tp.x)*GLOBAL_SCALE_MOVE; v4.y=(tp.y+4)*GLOBAL_SCALE_MOVE; v4.z=g_MatrixMap->GetZ(v4.x,v4.y)+1.0f;

    //    CHelper::Create(1)->Triangle(v1,v2,v3,0x8000ff00);
    //    CHelper::Create(1)->Triangle(v1,v3,v4,0x8000ff00);
    //}

    //if(m_CurrState != ROBOT_DIP) {
       // if(m_ZonePathCnt>0) {
          //  for(int i=1;i<m_ZonePathCnt; ++i)
          // {
             //   D3DXVECTOR3 vfrom,vto;
             //   vfrom.x=GLOBAL_SCALE_MOVE*float(g_MatrixMap->m_RoadNetwork.m_Zone[m_ZonePath[i-1]].m_Center.x)+GLOBAL_SCALE_MOVE/2;
             //   vfrom.y=GLOBAL_SCALE_MOVE*float(g_MatrixMap->m_RoadNetwork.m_Zone[m_ZonePath[i-1]].m_Center.y)+GLOBAL_SCALE_MOVE/2;
             //   vfrom.z=g_MatrixMap->GetZ(vfrom.x,vfrom.y)+50.0f;

             //   vto.x=GLOBAL_SCALE_MOVE*float(g_MatrixMap->m_RoadNetwork.m_Zone[m_ZonePath[i]].m_Center.x)+GLOBAL_SCALE_MOVE/2;
             //   vto.y=GLOBAL_SCALE_MOVE*float(g_MatrixMap->m_RoadNetwork.m_Zone[m_ZonePath[i]].m_Center.y)+GLOBAL_SCALE_MOVE/2;
             //   vto.z=g_MatrixMap->GetZ(vto.x,vto.y)+50.0f;

    //            CHelper::Create(1)->Cone(vfrom,vto,0.5f,0.5f,0xffff0000,0xffff0000,6);
             //   CHelper::Create(1)->Cone(vto+(vfrom-vto)*0.1f,vto,1.5f,0.5f,0xffff0000,0xffffff00,6);
             //   CHelper::Create(1)->Cone(vto,vto-D3DXVECTOR3(0.0f,0.0f,50.0f),0.5f,0.5f,0xffff0000,0xffff0000,6);
          //  }
       // }

       // if(m_MovePathCnt>0) {
          //  for(int i=1;i<m_MovePathCnt; ++i)
          // {
             //   D3DXVECTOR3 vfrom,vto;
             //   vfrom.x=GLOBAL_SCALE_MOVE*m_MovePath[i-1].x+GLOBAL_SCALE_MOVE/2;
             //   vfrom.y=GLOBAL_SCALE_MOVE*m_MovePath[i-1].y+GLOBAL_SCALE_MOVE/2;
             //   vfrom.z=g_MatrixMap->GetZ(vfrom.x,vfrom.y)+2.0f;//+GLOBAL_SCALE_MOVE;

             //   vto.x=GLOBAL_SCALE_MOVE*m_MovePath[i].x+GLOBAL_SCALE_MOVE/2;
             //   vto.y=GLOBAL_SCALE_MOVE*m_MovePath[i].y+GLOBAL_SCALE_MOVE/2;
             //   vto.z=g_MatrixMap->GetZ(vto.x,vto.y)+2.0f;//+GLOBAL_SCALE_MOVE;

             //   CHelper::Create(1)->Cone(vfrom,vto,0.5f,0.5f,0xffffffff,0xffff0000,6);
          //  }
       // }

    //}
    CMatrixRobot::Draw();
}
#endif

void CMatrixRobotAI::RobotSpawn(CMatrixBuilding* pBase)
{
    SetBase(pBase);
    pBase->SetSpawningUnit(true);

    CMatrixSideUnit* side = g_MatrixMap->GetSideById(pBase->m_Side);
    SetManualControlLocked(true);

    if(IsCrazy())
    {
        InitMaxHitpoint(1000000.0f);
        m_Team = 0;

        SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);

        side->AssignPlace(this, g_MatrixMap->GetRegion(CPoint(int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE), int(pBase->m_Pos.y / GLOBAL_SCALE_MOVE))));
        if(GetEnv()->m_Place < 0) side->PGOrderAttack(side->RobotToLogicGroup(this), CPoint(int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE), int(pBase->m_Pos.y / GLOBAL_SCALE_MOVE)), nullptr);
        else side->PGOrderAttack(side->RobotToLogicGroup(this), g_MatrixMap->m_RoadNetwork.GetPlace(GetEnv()->m_Place)->m_Pos, nullptr);

        RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
    }
    else
    {
        //Спавнится робот-доминатор
        if(side->m_Id != PLAYER_SIDE)
        {
            m_Team = side->ClacSpawnTeam(g_MatrixMap->GetRegion(CPoint(Float2Int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE), Float2Int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE))), m_Module[0].m_Kind - 1);
            //int minr = side->m_Team[m_Team].m_RobotCnt;
            //for(int i = 1; i < side->m_TeamCnt; ++i)
            //{
            //    if(side->m_Team[i].m_RobotCnt < minr)
            //    {
            //        minr = side->m_Team[i].m_RobotCnt;
            //        m_Team = i;
            //    }
            //}
        }
        //Спавнится робот игрока
        else
        {
            m_Team = 0;

            SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);

            //Робот получает стартовое место, в которое он направится после выхода с базы
            if(pBase->GatheringPointIsSet())
            {
                std::vector<SMatrixRegion*> all_regions;
                for(int i = 0; i < g_MatrixMap->m_RoadNetwork.m_RegionCnt; ++i) all_regions.push_back(&g_MatrixMap->m_RoadNetwork.m_Region[i]);

                std::sort(all_regions.begin(), all_regions.end(),
                    [&](const SMatrixRegion* a, const SMatrixRegion* b)->bool
                    {
                        const CPoint point = pBase->GetGatheringPoint();
                        return a->m_Center.Dist2(point) < b->m_Center.Dist2(point);
                    }
                );
                side->AssignPlace(this, g_MatrixMap->GetRegion(pBase->GetGatheringPoint()), &pBase->GetGatheringPoint(), &all_regions);
            }
            else
            {
                side->AssignPlace(this, g_MatrixMap->GetRegion(CPoint(int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE), int(pBase->m_Pos.y / GLOBAL_SCALE_MOVE))));
            }

            if(GetEnv()->m_Place < 0) side->PGOrderAttack(side->RobotToLogicGroup(this), CPoint(int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE), int(pBase->m_Pos.y / GLOBAL_SCALE_MOVE)), nullptr);
            else side->PGOrderAttack(side->RobotToLogicGroup(this), g_MatrixMap->m_RoadNetwork.GetPlace(GetEnv()->m_Place)->m_Pos, nullptr);

            RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
        }

        if(m_Side != 0) g_MatrixMap->GetSideById(m_Side)->IncStatValue(STAT_ROBOT_BUILD);
    }

    m_CurrState = ROBOT_IN_SPAWN;
    pBase->Open();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

//Поворачивает корпуса робота в указанном направлении (задаёт направление для дальнейшего разворота, не меняет положение модели корпуса физически, это делается изменением m_HullForward)
void CMatrixRobotAI::SetHullTargetDirection(const D3DXVECTOR3& dest)
{
    D3DXVECTOR3 destDirN(0, 0, 0), destDir(0, 0, 0);

    destDir = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
    if(this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject() && D3DXVec3LengthSq(&destDir) < MIN_ROT_DIST * MIN_ROT_DIST)
    {
        destDirN = m_HullForward;
    }
    else
    {
        destDir.z = 0;
        D3DXVec3Normalize(&destDirN, &destDir);
    }

    float cos3 = m_HullForward.x * destDirN.x + m_HullForward.y * destDirN.y;

    float angle3 = acos(cos3);

    float cos1 = m_HullForward.x * m_ChassisForward.x + m_HullForward.y * m_ChassisForward.y;
    float angle1 = acos(cos1);

    D3DXVECTOR3 vec(m_Module[0].m_Matrix._11, m_Module[0].m_Matrix._12, 0);
    D3DXVec3Normalize(&vec, &vec);
    float cos2 = m_HullForward.x * vec.x + m_HullForward.y * vec.y;

    if(this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject())
    {
        bool dchanged = false;
        if(fabs(angle3) >= GRAD2RAD(HULL_ROT_S_ANGL))
        {
            float cosmos = destDirN.x * (-m_HullForward.y) + destDirN.y * (m_HullForward.x);
            if(cosmos < 0 && m_PrevTurnSign == 1)
            {
                m_PrevTurnSign = 0;
                dchanged = true;
            }
            else if(cosmos > 0 && !m_PrevTurnSign)
            {
                m_PrevTurnSign = 1;
                dchanged = true;
            }
        }

        ++m_HullRotCnt;

        if(dchanged && m_HullRotCnt >= HULL_ROT_TAKTS)
        {
            PlayHullSound();
            m_HullRotCnt = 0;
        }
    }

    if(fabs(angle3) >= GRAD2RAD(160)) D3DXVec3Normalize(&m_HullForward, &(m_HullForward + Vec3Truncate((destDirN - m_HullForward), m_maxHullSpeed * 3)));
    else D3DXVec3Normalize(&m_HullForward, &(m_HullForward + Vec3Truncate((destDirN - m_HullForward), m_maxHullSpeed)));

    if(fabs(angle1) >= MAX_HULL_ANGLE)
    {
        D3DXMATRIX rotMat;
        if(cos2 > 0) D3DXMatrixRotationZ(&rotMat, -(MAX_HULL_ANGLE));
        else D3DXMatrixRotationZ(&rotMat, MAX_HULL_ANGLE);
        D3DXVec3TransformCoord(&m_HullForward, &m_ChassisForward, &rotMat);
    }

    RChange(MR_Matrix | MR_ShadowProjTex | MR_ShadowStencil);
}

//Поворачивает шасси робота в указанном направлении (производит физический поворот модели)
bool CMatrixRobotAI::RotateRobotChassis(const D3DXVECTOR3& dest, byte rotate_cam_link, float* rotate_angle)
{
    RChange(MR_Matrix | MR_ShadowProjTex | MR_ShadowStencil);

    //SwitchAnimation(ANIMATION_ROTATE);
    //m_RotSpeed = m_GroupSpeed;

    D3DXVECTOR3 dest_dir_norm(0, 0, 0), dest_dir(0, 0, 0), forward(0, 0, 0);

    float rot_speed = m_maxRotationSpeed * m_SyncMul;

    dest_dir = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
    dest_dir.z = 0;
    D3DXVec3Normalize(&dest_dir_norm, &dest_dir);
    forward = m_ChassisForward;
    forward.z = 0;
    D3DXVec3Normalize(&forward, &forward);

    float cos1 = (forward.x * dest_dir_norm.x) + (forward.y * dest_dir_norm.y);
    if(cos1 > 1.0f) cos1 = 1.0f;
    else if(cos1 < -1.0f) cos1 = -1.0f;
    float angle1 = acos(cos1);

    if(rotate_angle) *rotate_angle = angle1; //Угол, на который должен быть выполнен разворот (с учётом полученного направления) относительно текущего положения шасси

    D3DXVECTOR3 vec = { forward.y, -forward.x, 0 };
    float rot_dir = (dest_dir_norm.x * vec.x) + (dest_dir_norm.y * vec.y);
    if(fabs(rot_dir) < 0.000001) rot_dir = -((m_ChassisCamForward.x * dest_dir_norm.y) - (m_ChassisCamForward.y * dest_dir_norm.x)); //Чтобы не было неверных переоборотов направо при угле близком к 180 градусам

    //За этот вызов шасси успешно достигло указанного положения (повернулось на нужный угол), завершаем функцию
    if(angle1 <= rot_speed)
    {
        m_ChassisForward = dest_dir_norm;
        if(rotate_cam_link == CONNECTED_CAM_ROTATION) m_ChassisCamForward = dest_dir_norm;
        else if(rotate_cam_link == SIMULTANEOUS_CAM_ROTATION)
        {
            if(rot_dir > 0) //Поворот влево
            {
                D3DXMATRIX rotMat;
                D3DXMatrixRotationZ(&rotMat, -rot_speed);
                D3DXVec3TransformCoord(&m_ChassisCamForward, &m_ChassisCamForward, &rotMat);
            }
            else //Поворот вправо
            {
                D3DXMATRIX rotMat;
                D3DXMatrixRotationZ(&rotMat, rot_speed);
                D3DXVec3TransformCoord(&m_ChassisCamForward, &m_ChassisCamForward, &rotMat);
            }
        }
        m_Velocity = m_ChassisForward * D3DXVec3Length(&m_Velocity);
        return true;
    }

    //Скорости разворота шасси оказалось недостаточно для достижения нужного положения, вращение необходимо будет продолжить повторным вызовом
    if(fabs(angle1 - rot_speed) < 0.001) forward = dest_dir_norm;
    else if(rot_dir > 0) //Поворот влево
    {
        D3DXMATRIX rotMat;
        D3DXMatrixRotationZ(&rotMat, -rot_speed);
        D3DXVec3TransformCoord(&forward, &forward, &rotMat);
    }
    else //Поворот вправо
    {
        D3DXMATRIX rotMat;
        D3DXMatrixRotationZ(&rotMat, rot_speed);
        D3DXVec3TransformCoord(&forward, &forward, &rotMat);
    }

    m_ChassisForward = forward;
    if(rotate_cam_link == CONNECTED_CAM_ROTATION) m_ChassisCamForward = forward;
    else if(rotate_cam_link == SIMULTANEOUS_CAM_ROTATION)
    {
        if(rot_dir > 0) //Поворот влево
        {
            D3DXMATRIX rotMat;
            D3DXMatrixRotationZ(&rotMat, -rot_speed);
            D3DXVec3TransformCoord(&m_ChassisCamForward, &m_ChassisCamForward, &rotMat);
        }
        else //Поворот вправо
        {
            D3DXMATRIX rotMat;
            D3DXMatrixRotationZ(&rotMat, rot_speed);
            D3DXVec3TransformCoord(&m_ChassisCamForward, &m_ChassisCamForward, &rotMat);
        }
    }
    m_Velocity = forward * D3DXVec3Length(&m_Velocity);
    return false;
}


bool CMatrixRobotAI::Seek(
    const D3DXVECTOR3& dest,
    bool& rotate,
    bool end_path,
    byte moving_direction, //Маркер направления движения робота, при движении вперёд, шасси будет автоматически доворачиваться по вектору движения
    bool cam_rotation
)
{
    float rangle = 0.0f;

    rotate = true;
    if(cam_rotation && m_CurrState != ROBOT_BASE_MOVEOUT && moving_direction == UNIT_MOVING_FORWARD)
    {
        if(RotateRobotChassis(dest, CONNECTED_CAM_ROTATION, &rangle))
        {
            rotate = false;
            rangle = 0.0f;
        }
    }
    else rotate = false;

    D3DXVECTOR3 forward = m_ChassisForward;
    forward.z = 0;

    D3DXVec3Normalize(&forward, &forward);

    float slope; //Нужен для определения ускорения/замедления при движении робота по наклонным поверхностям
    switch(moving_direction)
    {
        case UNIT_MOVING_FORWARD:
        {
            //Переусложнённая оригинальная формула, которая в текущем виде бесполезна, однако в 0,0,1 вместо 1 можно, при желании, подставить модификатор гравитации карты
            //slope = D3DXVec3Dot(&D3DXVECTOR3(0, 0, 1), (D3DXVECTOR3*)&m_Core->m_Matrix._21);
            
            slope = m_Core->m_Matrix._23;
            break;
        }
        case UNIT_MOVING_BACKWARD:
        {
            //Переусложнённая оригинальная формула, которая в текущем виде не имеет смысла, однако в 0,0,1 вместо 1 можно, при желании, подставить модификатор гравитации карты
            //slope = D3DXVec3Dot(&D3DXVECTOR3(0, 0, 1), &-(*(D3DXVECTOR3*)&m_Core->m_Matrix._21));
            
            slope = -m_Core->m_Matrix._23;
            forward = -forward;
            break;
        }
        case UNIT_MOVING_LEFT:
        {
            //Переусложнённая оригинальная формула, которая в текущем виде бесполезна, однако в 0,0,1 вместо 1 можно, при желании, подставить модификатор гравитации карты
            //slope = D3DXVec3Dot(&D3DXVECTOR3(0, 0, 1), (D3DXVECTOR3*)&m_Core->m_Matrix._11);

            slope = m_Core->m_Matrix._13;
            forward = D3DXVECTOR3(forward.y, -forward.x, forward.z);
            break;
        }
        case UNIT_MOVING_RIGHT:
        {
            //Переусложнённая оригинальная формула, которая в текущем виде не имеет смысла, однако в 0,0,1 вместо 1 можно, при желании, подставить модификатор гравитации карты
            //slope = D3DXVec3Dot(&D3DXVECTOR3(0, 0, 1), &-(*(D3DXVECTOR3*)&m_Core->m_Matrix._11));

            slope = -m_Core->m_Matrix._13;
            forward = D3DXVECTOR3(-forward.y, forward.x, forward.z);
            break;
        }
    }

    D3DXVECTOR3 destDir = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
    float destLength = D3DXVec3Length(&destDir);

    if(m_CollAvoid.x != 0.0f || m_CollAvoid.y != 0.0f)
    {
        ASSERT(1);
    }

    if(m_GroupSpeed <= 0.001f) m_GroupSpeed = m_maxSpeed;

    float k = FLAG(m_ObjectFlags, ROBOT_FLAG_ON_WATER) ? m_SpeedWaterCorr : 1.0f;

    if(slope >= 0)
    {
        if(slope >= m_SpeedSlopeCorrUp) k = 0;
        else k *= LERPFLOAT(slope / m_SpeedSlopeCorrUp, 1.0, 0.0f);
    }
    else if(slope < 0) k *= LERPFLOAT(-slope, 1.0f, m_SpeedSlopeCorrDown);

    if((destLength - min(m_GroupSpeed, m_ColSpeed)) < 0.001f)
    {
        m_Velocity = destDir * k;
        m_Speed = destLength * k;
    }
    else
    {
        m_Speed = k * min(m_GroupSpeed, m_ColSpeed);
        if(end_path)
        {
            float t = min(1.0f, destLength / 20.0f);
            t *= min(1.0f, (1.0f - rangle / pi_f));
            m_Speed = m_Speed * t;
        }

        m_Velocity = forward * m_Speed;
    }
    return true;
}

//bool CMatrixRobotAI::Seek(const D3DXVECTOR3 &dest, bool end_path)
//{
//	if(m_GroupSpeed == 0)
//        m_GroupSpeed = m_maxSpeed;
//    //D3DXVECTOR3 oldv = D3DXVECTOR3(m_PosX, m_PosY, 0) + m_Velocity;
//    //D3DXVECTOR3 ovel = m_Velocity;
//
//    D3DXVECTOR3 desired_velocity(0,0,0), SteeringN(0,0,0), destDir(0,0,0), destDirN(0,0,0), destRot(0,0,0);
//	bool accelerating = false;
//
//	destDir = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
//    float destLength=D3DXVec3Length(&destDir);
//
//    if(m_CollAvoid.x != 0 || m_CollAvoid.y != 0){
//        destDirN = m_CollAvoid;
//        //CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(m_CollAvoid.x*100.0f, m_CollAvoid.y*100.0f, 50),0xffffffff, 0xffff0000);
//    }else{
//        D3DXVec3Normalize(&destDirN, &destDir);
//}
//
//
//    destRot = D3DXVECTOR3(m_PosX, m_PosY, 0) + destDirN * destLength;
//    if(m_CurrState == ROBOT_BASE_MOVEOUT){
//		desired_velocity = m_ChassisForward;
//		accelerating = true;
//    }else{
//		desired_velocity = destDirN;
//		accelerating = false;
//    }
//	
//    m_GroupSpeed=m_maxSpeed;
//	
//    desired_velocity *= /*m_maxSpeed*/m_GroupSpeed;
//    desired_velocity.z = 0;
//
//    D3DXVECTOR3 steering = desired_velocity - m_Velocity;
//	D3DXVECTOR3 accel = Vec3Truncate(steering, m_maxForce) / m_RobotMass;
//
//	D3DXVec3Normalize(&SteeringN, &(m_Velocity + accel));
//    
//    float rangle=0.0f;
//    
//	m_Velocity.z = 0;
//    if(m_CurrState == ROBOT_BASE_MOVEOUT/* || RotateRobotChassis(destRot)*/){
//        m_Velocity = Vec3Truncate(m_Velocity + accel, /*m_maxSpeed*/m_GroupSpeed);
//    }else{
//        float mspeed=m_GroupSpeed;
//
//        bool rot = false;
//        float len = CalcPathLength();//D3DXVec3Length(&destDir);
//        if(len > COLLIDE_BOT_R*3){
//            if((m_CollAvoid.x == 0 && m_CollAvoid.y == 0))
//            {
//                if(RotateRobotChassis(dest, CONNECTED_CAM_ROTATION, &rangle))
//                {
//                    rot = true;
//                }
//                else
//                {
//                    m_Velocity = Vec3Truncate(m_Velocity + Vec3Truncate((m_ChassisForward - m_Velocity), m_maxForce)/m_RobotMass, m_GroupSpeed);
//                    if(end_path) m_Velocity = Vec3Truncate(m_Velocity, max(0.01f,m_GroupSpeed*min(1.0f,destLength/(GLOBAL_SCALE_MOVE*1.5f))));
//                }
//            }
//        }else if(len > COLLIDE_BOT_R*0.5f){
//            rot = RotateRobotChassis(dest);
//            if(!rot){
//                return false;
//            }
//        }else{
//            rot = true;
//        }
//
//        if(rot){
//            if(!end_path) m_Velocity = Vec3Truncate(m_Velocity + accel, /*m_maxSpeed*/m_GroupSpeed);
//            else {
////if(rangle<-pi || rangle>pi) {
////    ASSERT(1);
////}
////DM(L"Velocity",CWStr().Format(L"mul=<f> Vel=<f>,<f>,<f>",rangle,m_Velocity.x,m_Velocity.y,m_Velocity.z).Get());
//                rangle=1.0f+(fabs(rangle)/pi)*2.0f;
//                m_Velocity = Vec3Truncate(m_Velocity + accel, max(0.1f,m_GroupSpeed*min(1.0f,destLength/(GLOBAL_SCALE_MOVE*2.0f*rangle))));
//            }
//        }
//    }
////
//    //D3DXVECTOR3 newv = D3DXVECTOR3(m_PosX, m_PosY, 0) + m_Velocity;
//
//    //D3DXVECTOR3 dv = newv - oldv;
//    //dv *= m_SyncMul;
//    //m_Velocity = ovel + dv;
//
//
////
//    m_Speed = D3DXVec3Length(&m_Velocity);
//    D3DXVECTOR3 vvv(0,0,0);
//    D3DXVec3Normalize(&vvv, &m_Velocity);
//
//
//    if(!IS_ZERO_VECTOR(vvv)
//    {
//	    m_ChassisForward = vvv;
//      m_ChassisCamForward = vvv;
//    }
//    
//    return true;
//
//}

//Функция базовой (низкоуровневой) логики движения робота (не приказа), обсчитывает подходящий маршрут и все препятствия на пути, включая геометрию коллизий
void CMatrixRobotAI::LowLevelMove(
    int ms,
    const D3DXVECTOR3& dest, //Вектор движения (направление)
    bool robot_coll,         //Маркер учёта коллизии с другими динамическими объектами на карте
    bool obst_coll,          //Маркер учёта коллизии со всякой топографией и статичными декорациями
    bool end_path,
    byte moving_direction,   //Маркер направления движения робота, при движении вперёд, шасси будет автоматически доворачиваться по вектору движения
    bool cam_rotation
)
{
    if(IsNoMoreEverMovingOverride()) return; //Если эта тупая мразь почему-то продолжает ехать, несмотря ни на что

    bool rotate = false;
    bool vel = Seek(dest, rotate, end_path, moving_direction, cam_rotation);

    m_Cols = 0;
    D3DXVECTOR3 r(0, 0, 0), o(0, 0, 0), result_coll(0, 0, 0), tmp_vel(0, 0, 0);

    if(!vel)
    {
        tmp_vel = m_Velocity;
        m_Velocity = D3DXVECTOR3(0, 0, 0);
    }

    D3DXVECTOR3 genetic_mutated_velocity = m_Velocity * m_SyncMul;

    //Коллизия роботов, вертолётов и турелей
    if(robot_coll && !IsNoObjectsCollision())
    {
        r = RobotToObjectCollision(genetic_mutated_velocity, ms);
        result_coll = r;
    }
    //Коллизия здания и декораций
    if(obst_coll && !IsNoObstaclesCollision())
    {
        o = SphereRobotToAABBObstacleCollision(r, genetic_mutated_velocity);
        if(!m_Cols) WallAvoid(o, dest);
        result_coll = r + o;
    }

#ifdef _SUB_BUG    
    if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject())
    {
        g_MatrixMap->m_DI.T(L"pos_x", CWStr(m_PosX));
        g_MatrixMap->m_DI.T(L"pos_y", CWStr(m_PosY));
        g_MatrixMap->m_DI.T(L"geo_x", CWStr(m_Core->m_GeoCenter.x));
        g_MatrixMap->m_DI.T(L"geo_y", CWStr(m_Core->m_GeoCenter.y));
        g_MatrixMap->m_DI.T(L"m_Velocity", CWStr(D3DXVec3Length(&m_Velocity)));
        //g_MatrixMap->m_DI.T(L"genetic_mutated_velocity", CWStr(D3DXVec3Length(&genetic_mutated_velocity)));
        g_MatrixMap->m_DI.T(L"result_coll", CWStr(D3DXVec3Length(&result_coll)));
    }
    CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(m_Velocity.x * 100, m_Velocity.y * 100, 50), 0xffffffff, 0xffffff00);
    //CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(100, 0, 50), 0xffffffff, 0xff00ff00);    
    //CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(0, 100, 50), 0xffffffff, 0xffff00ff);    
    //CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(result_coll.x * 100, result_coll.y * 100, 50), 0xffffffff, 0xffff0000);                    
#endif

    m_MovePathDistFollow += sqrt(float(POW2(genetic_mutated_velocity.x + result_coll.x) + POW2(genetic_mutated_velocity.y + result_coll.y)));

    RChange(MR_Matrix | MR_ShadowProjGeom);

    m_PosX += genetic_mutated_velocity.x + result_coll.x;
    m_PosY += genetic_mutated_velocity.y + result_coll.y;

    if(IsWalkingChassis())
    {
        if(rotate || result_coll.x || result_coll.y || GetColsWeight2() || (end_path && (POW2(dest.x - m_PosX) + POW2(dest.y - m_PosY)) < POW2(GLOBAL_SCALE_MOVE)))
        {
            if(!FLAG(m_ObjectFlags, ROBOT_FLAG_COLLISION))
            {
                SETFLAG(m_ObjectFlags, ROBOT_FLAG_COLLISION);
            }
        }
        else
        {
            if(FLAG(m_ObjectFlags, ROBOT_FLAG_COLLISION))
            {
                RESETFLAG(m_ObjectFlags, ROBOT_FLAG_COLLISION);
                FirstStepLinkWalkingChassis();
            }
        }
    }

    /*
    DM(L"RC", CWStr().Format(L"rotate=<i> result_coll=<i> GetColsWeight2=<i> end_path=<i> ROBOT_FLAG_COLLISION=<i>",
            int(rotate),
            int(result_coll.x != 0 || result_coll.y != 0),
            int(GetColsWeight2()),
            (end_path && (POW2(dest.x - m_PosX) + POW2(dest.y - m_PosY)) < POW2(GLOBAL_SCALE_MOVE)),
            FLAG(m_RobotFlags, ROBOT_FLAG_COLLISION)
            ).Get());

        m_MovePathDistFollow += sqrt(float(POW2(genetic_mutated_velocity.x + result_coll.x) + POW2(genetic_mutated_velocity.y + result_coll.y)));

        m_PosX += genetic_mutated_velocity.x + result_coll.x;
        m_PosY += genetic_mutated_velocity.y + result_coll.y;
    */

    if(!vel) m_Velocity = tmp_vel;

    JoinToGroup();
}

void CMatrixRobotAI::LowLevelDecelerate(int ms, bool robot_coll, bool obst_coll)
{
DTRACE();

    Decelerate();
    m_Cols = 0;
    D3DXVECTOR3 r(0, 0, 0), o(0, 0, 0), result_coll(0, 0, 0);

    D3DXVECTOR3 genetic_mutated_velocity = m_Velocity * m_SyncMul;

    //Коллизия роботов, вертолётов и турелей
    if(robot_coll && !IsNoObjectsCollision())
    {
        r = RobotToObjectCollision(genetic_mutated_velocity, ms);
        result_coll = r;
    }
    //Коллизия здания и декораций
    if(obst_coll && !IsNoObstaclesCollision())
    {
        o = SphereRobotToAABBObstacleCollision(r, genetic_mutated_velocity);
        result_coll = r + o;
    }

    if(result_coll.x || result_coll.y)
    {
        SETFLAG(m_ObjectFlags, ROBOT_FLAG_COLLISION);
    }
    else
    {
        RESETFLAG(m_ObjectFlags, ROBOT_FLAG_COLLISION);
        if(IsWalkingChassis())
        {
            FirstStepLinkWalkingChassis();
        }
    }

    m_PosX += genetic_mutated_velocity.x + result_coll.x;
    m_PosY += genetic_mutated_velocity.y + result_coll.y;
    JoinToGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrixRobotAI::Decelerate()
{
    m_Velocity *= DECELERATION_FORCE;
    m_Speed = D3DXVec3Length(&m_Velocity);
    D3DXVECTOR3 vvv(0, 0, 0);
    D3DXVec3Normalize(&vvv, &m_Velocity);
    if(!IS_ZERO_VECTOR(vvv))
    {
        m_ChassisForward = vvv;
        m_ChassisCamForward = vvv;
    }
}

#if(defined _DEBUG) && !(defined _RELDEBUG)
void HelperT(D3DXVECTOR2 from, D3DXVECTOR2 to)
{
    return;
    CHelper::Create(1, 0)->Line(D3DXVECTOR3(from.x, from.y, 2),
        D3DXVECTOR3(to.x, to.y, 2),
        0xffffff00, 0xffffff00);

    D3DXVECTOR3 n = to - from;
    n.z = 0;
    D3DXVec3Normalize(&n, &n);

    CHelper::Create(1, 0)->Line(D3DXVECTOR3(to.x - n.y * 1.5f, to.y + n.x * 1.5f, 2),
        D3DXVECTOR3(to.x + n.y * 1.5f, to.y - n.x * 1.5f, 2),
        0xffffff00, 0xffffff00);
}
#endif


D3DXVECTOR3 CMatrixRobotAI::SphereRobotToAABBObstacleCollision(D3DXVECTOR3& corr, const D3DXVECTOR3& vel)
{
    D3DXVECTOR2 robot_pos;
    D3DXVECTOR3 vCorrTotal(0, 0, 0);

    robot_pos.x = m_PosX + corr.x + vel.x;
    robot_pos.y = m_PosY + corr.y + vel.y;

    D3DXVECTOR3 oldpos = D3DXVECTOR3(robot_pos.x, robot_pos.y, 0);

    CPoint check_cell;

    int calc_for_x = -COLLIDE_FIELD_R * 2;
    int calc_for_y = -COLLIDE_FIELD_R * 2;
    BYTE corners[COLLIDE_FIELD_R + COLLIDE_FIELD_R][COLLIDE_FIELD_R + COLLIDE_FIELD_R];
    memset(corners, -1, sizeof(corners));

    for(int cnt = 0; cnt < 4; ++cnt)
    {
        //robot_pos.x += vCorrTotal.x;
        //robot_pos.y += vCorrTotal.y;
        //vCorrTotal *= 0.0f;

        int col_cnt = 0;

        int x0 = TruncFloat(robot_pos.x * INVERT(GLOBAL_SCALE_MOVE)) - COLLIDE_FIELD_R;
        int y0 = TruncFloat(robot_pos.y * INVERT(GLOBAL_SCALE_MOVE)) - COLLIDE_FIELD_R;

        int x1 = x0 + (COLLIDE_FIELD_R * 2);
        int y1 = y0 + (COLLIDE_FIELD_R * 2);

        if(x1 < 0) break;
        if(y1 < 0) break;
        if(x0 > g_MatrixMap->m_SizeMove.x) break;
        if(y0 > g_MatrixMap->m_SizeMove.y) break;

        if(x0 < 0) x0 = 0;
        if(y0 < 0) y0 = 0;
        if(x1 > g_MatrixMap->m_SizeMove.x) x1 = g_MatrixMap->m_SizeMove.x;
        if(y1 > g_MatrixMap->m_SizeMove.y) y1 = g_MatrixMap->m_SizeMove.y;

        SMatrixMapMove* smm = g_MatrixMap->MoveGet(x0, y0);

        if(calc_for_x != x0 || calc_for_y != y0)
        {
            // recalc corners
            SMatrixMapMove* smm0 = smm;

            for(int y = y0; y < y1; ++y, smm0 += g_MatrixMap->m_SizeMove.x - (x1 - x0))
            {
                for(int x = x0; x < x1; ++x, ++smm0)
                {
                    corners[x - x0][y - y0] = smm0->GetType(g_Config.m_RobotChassisConsts[m_Module[0].m_Kind].passability);
                }
            }

            calc_for_x = x0;
            calc_for_y = y0;
        }

        for(int y = y0; y < y1; ++y, smm += g_MatrixMap->m_SizeMove.x - (x1 - x0))
        {
            for(int x = x0; x < x1; ++x, ++smm)
            {
                BYTE corner = corners[x - x0][y - y0];
                if(corner == 0xFF) continue;

                D3DXVECTOR3 col = SphereToAABB(robot_pos, smm, CPoint(x, y), corner); //, x >= (x1 - COLLIDE_FIELD_R), y >= (y1 - COLLIDE_FIELD_R));

                if(!IS_ZERO_VECTOR(col))
                {
                    col.z = 0;

                    robot_pos.x += col.x;
                    robot_pos.y += col.y;

                    ++col_cnt;
                }
            }
        }
    }

    //#ifdef _SUB_BUG
    //if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()){
    //    g_MatrixMap->m_DI.T(L"col_cnt", CWStr(col_cnt + m_Cols));
    //}
    //#endif

    //if(m_Cols+col_cnt > 2){
    //    m_Velocity *= 0;
    //    corr *= 0;
    //    vCorrTotal *= 0;
    //    return vCorrTotal;
    //}

    //float ct_len/*_sq*/ = D3DXVec3Length/*Sq*/(&vCorrTotal);
    //float c_len/*_sq*/ = D3DXVec3Length/*Sq*/(&corr);
    //float vel_len/*_sq*/ = D3DXVec3Length/*Sq*/(&m_Velocity);
    //float ctc_len/*_sq*/ = D3DXVec3Length/*Sq*/(&(vCorrTotal + corr));
    //float velc_len/*_sq*/ = D3DXVec3Length/*Sq*/(&(m_Velocity + corr));
    //float velct_len/*_sq*/ = D3DXVec3Length/*Sq*/(&(vCorrTotal + m_Velocity));

    //if(ct_len > vel_len){
    //    D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);    
    //    vCorrTotal *= vel_len;
    //}

    //if(velc_len > ct_len){
    //    D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);    
    //    vCorrTotal *= velc_len;
    //}

    //if(corr.x || corr.y){
    //    if(vel_len > ctc_len){
    //        D3DXVec3Normalize(&m_Velocity, &m_Velocity);
    //        m_Velocity *= ctc_len;
    //    }else if(velc_len > ctc_len){
    //        m_Velocity *= 0;
    //        corr *= 0;
    //        vCorrTotal *= 0;
    //    }else if(velct_len > c_len){
    //        m_Velocity *= 0;
    //        corr *= 0;
    //        vCorrTotal *= 0;
    //    }
    //}

    //if(ct_len > vel_len){
    //    //D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);    
    //    //vCorrTotal *= vel_len;
    //    if(corr.x || corr.y){
    //        if(vel_len > ctc_len){
    //            D3DXVec3Normalize(&m_Velocity, &m_Velocity);
    //            m_Velocity *= ctc_len;
    //        }else if(D3DXVec3Length(&(m_Velocity + corr)) > ct_len){
    //            m_Velocity *= 0;
    //            corr *= 0;
    //            vCorrTotal *= 0;
    //        }
    //    }
    //}

    //if(ct_len > vel_len){
    //    D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);    
    //    vCorrTotal *= vel_len;
    //}
    //if(corr.x || corr.y){
    //    ct_len = D3DXVec3Length/*Sq*/(&vCorrTotal);
    //    D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);
    //    vCorrTotal *= ct_len + c_len;
    //}

    return D3DXVECTOR3(robot_pos.x, robot_pos.y, 0) - oldpos;//vCorrTotal;
}

struct CollisionData
{
    D3DXVECTOR3 result;
    CMatrixRobotAI* robot;
    D3DXVECTOR3 vel;
    bool stop;
    int ms;
    bool far_col;
};

static bool CollisionCallback(const D3DXVECTOR3& fpos, CMatrixMapStatic* pObject, dword user)
{
    CollisionData* data = (CollisionData*)user;

    const int tm = 2;

    if (pObject->IsRobot() && !pObject->AsRobot()->IsAutomaticMode())
    {
        CMatrixRobotAI* pCurrBot = pObject->AsRobot();
        D3DXVECTOR2 my_pos = D3DXVECTOR2(data->robot->m_PosX, data->robot->m_PosY) + D3DXVECTOR2(data->vel.x, data->vel.y);
        D3DXVECTOR2 collide_pos = D3DXVECTOR2(pCurrBot->m_PosX, pCurrBot->m_PosY) /*+ D3DXVECTOR2(pCurrBot->m_Velocity.x, pCurrBot->m_Velocity.y)*/;
        D3DXVECTOR2 vDist = my_pos - collide_pos;
        float dist = D3DXVec2Length(&vDist)/*D3DXVec2LengthSq(&vDist)*/;

        if (dist < (COLLIDE_BOT_R + COLLIDE_BOT_R)/**(COLLIDE_BOT_R + COLLIDE_BOT_R)*/)
        {

            while (true)
            {
                float vd = POW2(data->vel.x) + POW2(data->vel.y);
                if (fabs(vd) < POW2(0.0001f)) break;

                float d = 1.0f / (float)sqrt(vd);
                float vx = data->vel.x * d;
                float vy = data->vel.y * d;

                data->robot->IncColsWeight(data->ms * tm);
                data->robot->IncColsWeight2(data->ms * tm);
                pCurrBot->IncColsWeight(data->ms * tm);
                pCurrBot->IncColsWeight2(data->ms * tm);
                if (data->robot->GetColsWeight2() > 500 * tm) data->robot->SetColsWeight2(500 * tm);

                if (!pCurrBot->IsAutomaticMode())
                {
                    if (data->robot->GetColsWeight2() < 200 * tm) // Если недавно столкнулись
                    {
                        float vd2 = POW2(pCurrBot->m_Velocity.x) + POW2(pCurrBot->m_Velocity.y);
                        if (vd2 > POW2(0.0001f)) // Если робот движется
                        {
                            if ((vx * -vDist.x + vy * -vDist.y) > 0) // И он находится впереди
                            {
                                if (vx * pCurrBot->m_Velocity.x + vy * pCurrBot->m_Velocity.y > 0) // И движется приблизительно в одну сторону
                                {
                                    if ((pCurrBot->m_Velocity.x * vDist.x + pCurrBot->m_Velocity.y * vDist.y) > 0 && dword(pCurrBot) < dword(data->robot)); // Только один из двух роботов может двигаться
                                    else
                                    {
                                        data->stop = true;
                                        data->far_col = true;
                                        data->robot->m_ColSpeed = min(data->robot->m_GroupSpeed, pCurrBot->m_GroupSpeed * 0.5f);
                                    }
                                }
                            }

                        }
                    }
                }

#ifdef _SUB_BUG
                g_MatrixMap->m_DI.T(L"ColsWeight", CWStr().Format(L"<i>    <i>", data->robot->GetColsWeight(), data->robot->GetColsWeight2()).Get(), 1000);
#endif

                if (data->robot->GetColsWeight() < 500 * tm) break;
                data->robot->SetColsWeight(500 * tm);

                d = 1.0f / dist;
                float dx = -vDist.x * d, dy = -vDist.y * d;

                if (!((vx * dx + vy * dy) > cos(90 * ToRad))) break;

                d = POW2(pCurrBot->m_Velocity.x) + POW2(pCurrBot->m_Velocity.y);
                if (d > 0)
                {
                    d = 1.0f / (float)sqrt(d);
                    float dvx = pCurrBot->m_Velocity.x * d;
                    float dvy = pCurrBot->m_Velocity.y * d;

                    if (!((vx * dvx + vy * dvy) < -cos(45 * ToRad)))
                    {
                        data->robot->SetColsWeight(0);
                        break;
                    }
                }

                if (!data->robot->FindOrderLikeThat(ROT_MOVE_RETURN) && data->robot->m_Side == pCurrBot->m_Side)
                {

                    if (!pCurrBot->IsAutomaticMode())
                    {
                        if (!pCurrBot->FindOrderLikeThat(ROT_MOVE_RETURN))
                        {
                            CPoint tp;
                            if (pCurrBot->GetMoveToCoords(tp)) pCurrBot->MoveReturn(tp.x, tp.y);
                            else pCurrBot->MoveReturn(pCurrBot->GetMapPosX(), pCurrBot->GetMapPosY());
                        }

                        int tpx = pCurrBot->GetMapPosX();
                        int tpy = pCurrBot->GetMapPosY();
                        if (g_MatrixMap->PlaceFindNearReturn(pCurrBot->m_Module[0].m_Kind - 1, 4, tpx, tpy, pCurrBot))
                        {
                            pCurrBot->MoveTo(tpx, tpy);
                            pCurrBot->GetEnv()->AddBadCoord(CPoint(tpx, tpy));
                        }
                    }
                }
                data->robot->SetColsWeight(0);

                break;
            }

            /*            if(data->robot->m_CurrState == ROBOT_BASE_MOVEOUT || data->robot->FindOrderLikeThat(ROT_MOVE_TO, ROT_GETING_LOST)){
                            pCurrBot->GetLost(data->robot->m_ChassisForward);
                            float vel_len1 = D3DXVec3LengthSq(&pCurrBot->m_Velocity);
                            float vel_len2 = D3DXVec3LengthSq(&data->robot->m_Velocity);

                            if(vel_len1 > ZERO_VELOCITY*ZERO_VELOCITY && vel_len1 < vel_len2){
                                D3DXVec3Normalize(&data->robot->m_Velocity, &data->robot->m_Velocity);
                                data->robot->m_Velocity *= (float)sqrt(vel_len1);
                            }

                        }*/
            float correction = float(COLLIDE_BOT_R + COLLIDE_BOT_R - /*sqrt*/(dist)) * 0.5f;
            D3DXVec2Normalize(&vDist, &vDist);
            vDist *= correction;
            data->result += D3DXVECTOR3(vDist.x, vDist.y, 0);
            data->robot->IncCols();
        }
        else if(dist < (COLLIDE_BOT_R * 4.0f)/**(COLLIDE_BOT_R + COLLIDE_BOT_R)*/)
        {
            while(true)
            {
                float vd = POW2(data->vel.x) + POW2(data->vel.y);
                if(fabs(vd) < POW2(0.0001f)) break;

                float d = 1.0f / (float)sqrt(vd);
                float vx = data->vel.x * d;
                float vy = data->vel.y * d;

                if(!pCurrBot->IsAutomaticMode())
                {
                    float vd2 = POW2(pCurrBot->m_Velocity.x) + POW2(pCurrBot->m_Velocity.y);
                    if(vd2 > POW2(0.0001f)) // Если робот движется
                    {
                        if((vx * -vDist.x + vy * -vDist.y) > 0) // И он находится впереди
                        {
                            if(vx * pCurrBot->m_Velocity.x + vy * pCurrBot->m_Velocity.y > 0) // И движется приблизительно в одну сторону
                            {
                                if((pCurrBot->m_Velocity.x * vDist.x + pCurrBot->m_Velocity.y * vDist.y) > 0 && dword(pCurrBot) < dword(data->robot)); // Только один из двух роботов может двигаться
                                else
                                {
                                    data->far_col = true;
                                    data->robot->m_ColSpeed = min(data->robot->m_GroupSpeed, pCurrBot->m_GroupSpeed * 0.5f);
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    else if (pObject->GetObjectType() == OBJECT_TYPE_CANNON)
    {
        CMatrixCannon* cannon = ((CMatrixCannon*)pObject);
        D3DXVECTOR2 my_pos = D3DXVECTOR2(data->robot->m_PosX, data->robot->m_PosY) + D3DXVECTOR2(data->vel.x, data->vel.y);
        D3DXVECTOR2 collide_pos = cannon->m_Pos;
        D3DXVECTOR2 vDist = my_pos - collide_pos;
        float dist = D3DXVec2Length(&vDist)/*D3DXVec2LengthSq(&vDist)*/;
        //#ifdef _SUB_BUG
        //                CHelper::Create(1,1)->Cone(D3DXVECTOR3(cannon->m_Pos.x, cannon->m_Pos.y, 5.9f), D3DXVECTOR3(cannon->m_Pos.x, cannon->m_Pos.y, 7),  CANNON_COLLIDE_R,  CANNON_COLLIDE_R, 0xffffffff, 0xffffffff,  12);
        //                g_MatrixMap->m_DI.T(L"CAN_DIST", CWStr(dist));
        //#endif

        if (dist < (COLLIDE_BOT_R + CANNON_COLLIDE_R)/**(COLLIDE_BOT_R + CANNON_COLLIDE_R)*/)
        {
            float correction = float(COLLIDE_BOT_R + CANNON_COLLIDE_R - /*sqrt*/(dist));
            //#ifdef _SUB_BUG
            //                    g_MatrixMap->m_DI.T(L"correction", CWStr(correction));
            //#endif
            D3DXVec2Normalize(&vDist, &vDist);
            vDist *= correction;
            data->result += D3DXVECTOR3(vDist.x, vDist.y, 0);
            data->robot->IncCols();
        }
    }
    return true;
}

D3DXVECTOR3 CMatrixRobotAI::RobotToObjectCollision(const D3DXVECTOR3& vel, int ms)
{
    CollisionData data;
    data.result = D3DXVECTOR3(0, 0, 0);
    data.robot = this;
    data.vel = vel;
    data.stop = false;
    data.ms = ms;
    data.far_col = false;
    g_MatrixMap->FindObjects(D3DXVECTOR3(m_PosX, m_PosY, m_Core->m_GeoCenter.z), COLLIDE_BOT_R * 3, 1, TRACE_ROBOT | TRACE_CANNON, this, CollisionCallback, (dword)&data);

    if (!data.far_col) m_ColSpeed = 100.0f;

    if (data.stop) return -vel;
    else return data.result;
}

void CMatrixRobotAI::WallAvoid(const D3DXVECTOR3& o, const D3DXVECTOR3& dest)
{
    D3DXVECTOR3 prev_coll = m_CollAvoid;
    m_CollAvoid = D3DXVECTOR3(0, 0, 0);
    return;
    if(o.x != 0 || o.y != 0)
    {
        D3DXVECTOR3 true_forward(1, 0, 0);
        float sign1 = m_ChassisForward.x * true_forward.x + m_ChassisForward.y * true_forward.y;
        true_forward = D3DXVECTOR3(0, 1, 0);
        float sign2 = m_ChassisForward.x * true_forward.x + m_ChassisForward.y * true_forward.y;
        //HelperCreate(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 30), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(true_forward.x * 100, true_forward.y * 100, 30), 0xff00ff00, 0xff00ff00);
        //HelperCreate(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(m_ChassisForward.x * 100, m_ChassisForward.y * 100, 50), 0xffffff00, 0xffffff00);
        //if(sign < 0) CDText::T("SIGN", "<<<<");
        //else CDText::T("SIGN", ">>>>");

        if(o.x != 0 && o.y != 0)
        {
            D3DXVECTOR3 o_norm(0, 0, 0);
            D3DXVec3Normalize(&o_norm, &o);

            float cos1 = m_ChassisForward.x * o_norm.x;
            float angle1 = (float)acos(cos1);

            float cos2 = m_ChassisForward.y * o_norm.y;
            float angle2 = (float)acos(cos2);

            if(fabs(angle1) > GRAD2RAD(120))
            {
                //we are perp to Y
                if(o.y < 0) m_CollAvoid = D3DXVECTOR3(0, -1, 0);
                else m_CollAvoid = D3DXVECTOR3(0, 1, 0);
            }
            else if(fabs(angle2) > GRAD2RAD(120))
            {
                //we are perp to X
                if(o.x < 0) m_CollAvoid = D3DXVECTOR3(-1, 0, 0);
                else m_CollAvoid = D3DXVECTOR3(1, 0, 0);
            }
            //if(prev_coll.y == -1){
            //    if(o.x < 0)
            //        m_CollAvoid = D3DXVECTOR3(-1, 0, 0);
            //    else
            //        m_CollAvoid = D3DXVECTOR3(1, 0, 0);
            //}
        }
        else if(o.x != 0/* || ((o.x != 0 && o.y != 0) && fabs(o.x) > fabs(o.y))*/)
        {
            //это условие избыточно, оно зарезервировано для возможных изменений

            if(o.x > 0)
            {
                if(sign2 < 0)
                {
                    m_CollAvoid += D3DXVECTOR3(0, -1, 0);
                }
                else
                {
                    m_CollAvoid += D3DXVECTOR3(0, 1, 0);
                }
            }
            else
            {
                if(sign2 < 0)
                {
                    m_CollAvoid += D3DXVECTOR3(0, -1, 0);
                }
                else
                {
                    m_CollAvoid += D3DXVECTOR3(0, 1, 0);
                }
            }
        }
        else if(o.y != 0/* || ((o.x != 0 && o.y != 0) && fabs(o.y) > fabs(o.x))*/)
        {
            //это условие избыточно, оно зарезервировано для возможных изменений
            if(o.y > 0)
            {
                if(sign1 < 0)
                {
                    m_CollAvoid += D3DXVECTOR3(-1, 0, 0);
                }
                else
                {
                    m_CollAvoid += D3DXVECTOR3(1, 0, 0);
                }
            }
            else
            {
                if(sign1 < 0)
                {
                    m_CollAvoid += D3DXVECTOR3(-1, 0, 0);
                }
                else
                {
                    m_CollAvoid += D3DXVECTOR3(1, 0, 0);
                }
            }
        }
    }

    if(m_CollAvoid.x || m_CollAvoid.y)
    {
        D3DXVECTOR3 dest_n = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
        D3DXVec3Normalize(&dest_n, &dest_n);

        float cos = dest_n.x * m_CollAvoid.x + dest_n.y * m_CollAvoid.y;
        float angle = (float)acos(cos);

        if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject())
        {
            if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ROTATE_LEFT]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ROTATE_RIGHT]) & 0x8000) == 0x8000))
            {
                ZeroMemory(&m_CollAvoid, sizeof(D3DXVECTOR3));
            }
        }

        if(angle >= GRAD2RAD(90))
        {
            ZeroMemory(&m_CollAvoid, sizeof(D3DXVECTOR3));
        }
    }
}


D3DXVECTOR3 CMatrixRobotAI::SphereToAABB(const D3DXVECTOR2& pos, const SMatrixMapMove* smm, const CPoint& cell, byte corner) //, bool revers_x, bool revers_y)
{
    //byte corner = smm->GetType(m_Module[0].m_Kind-1);

    //if(corner != 0xff)
    //{

    float dcol = 0, dsx = 0, dsy = 0;

    D3DXVECTOR2 lu(cell.x * GLOBAL_SCALE_MOVE, cell.y * GLOBAL_SCALE_MOVE);
    D3DXVECTOR2 rd(lu.x + GLOBAL_SCALE_MOVE, lu.y + GLOBAL_SCALE_MOVE);

    if(SphereToAABBCheck(pos, lu, rd, dcol, dsx, dsy))
    {
        //        #ifdef _SUB_BUG
        //        CHelper::Create(1,1)->Cone(D3DXVECTOR3(pos.x, pos.y, 0.9f), D3DXVECTOR3(pos.x, pos.y, 3.0f),  COLLIDE_BOT_R,  COLLIDE_BOT_R, 0xffffffff, 0xffffffff,  12);		
        ////line1(-)top
        //        CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1), 
        //						D3DXVECTOR3(vRu.x, vRu.y, 1),
        //						0xffff0000,0xffff0000);
        //
        ////line2(|)left	
        //        CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1), 
        //						D3DXVECTOR3(vLd.x, vLd.y, 1),
        //						0xffff0000,0xffff0000);
        //
        ////line3(|)right
        //        CHelper::Create(1,0)->Line(D3DXVECTOR3(vRu.x, vRu.y, 1), 
        //						D3DXVECTOR3(vRd.x, vRd.y, 1),
        //						0xffff0000,0xffff0000);
        ////line4(-)bottom
        //        CHelper::Create(1,0)->Line(D3DXVECTOR3(vLd.x, vLd.y, 1), 
        //						D3DXVECTOR3(vRd.x, vRd.y, 1),
        //						0xffff0000,0xffff0000);
        //        #endif

                //place for spherical and triangle corners
        if(corner > 0)
        {
            //#ifdef _SUB_BUG
            //CHelper::Create(1,1)->Cone(D3DXVECTOR3(vLu.x+COLLIDE_SPHERE_R, vLu.y+COLLIDE_SPHERE_R, 1), D3DXVECTOR3(vLu.x+COLLIDE_SPHERE_R, vLu.y+COLLIDE_SPHERE_R, 2),  COLLIDE_SPHERE_R,  COLLIDE_SPHERE_R, 0xffffffff, 0xffffffff,  12);		
            //#endif
            //check for spherical corner or zubchik

            //byte c1 = (corner & 1);
            //byte c2 = (corner & 2);
            //byte c4 = (corner & 4);
            //byte c8 = (corner & 8);

            //byte z1 = (corner & 16);
            //byte z2 = (corner & 32);
            //byte z4 = (corner & 64);
            //byte z8 = (corner & 128);

            byte cr = 0, zb = 0;

            if(pos.x > lu.x + GLOBAL_SCALE_MOVE * 0.5f)
            {
                if(pos.y < lu.y + GLOBAL_SCALE_MOVE * 0.5f)
                {
                    //1
                    if(corner & 16) zb = (corner & 16);
                    else cr = (corner & 1);
                }
                else
                {
                    //2
                    if(corner & 32) zb = (corner & 32);
                    else cr = (corner & 2);
                }
            }
            else
            {
                if(pos.y < lu.y + GLOBAL_SCALE_MOVE * 0.5f)
                {
                    //8
                    if(corner & 128) zb = (corner & 128);
                    else cr = (corner & 8);
                }
                else
                {
                    //4
                    if(corner & 64) zb = (corner & 64);
                    else cr = (corner & 4);
                }
            }

            if(zb)
            {
                if((corner & 16) || (corner & 64))
                {
                    D3DXVECTOR3 v = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(lu.x, lu.y, 0);
                    D3DXVECTOR3 v2 = D3DXVECTOR3(GLOBAL_SCALE_MOVE, GLOBAL_SCALE_MOVE, 0);
                    D3DXVECTOR3 proj = Vec3Projection(v2, v);
                    D3DXVECTOR2 point = lu + D3DXVECTOR2(proj.x, proj.y);

                    D3DXVECTOR2 res = pos - point;
                    float res_len = D3DXVec2Length(&res);

                    if ((point.x >= lu.x && point.x <= rd.x && point.y >= lu.y && point.y <= rd.y) && res_len < COLLIDE_BOT_R)
                    {

                        //      #ifdef _SUB_BUG
                        //      CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1), 
                                            //D3DXVECTOR3(vRd.x, vRd.y, 1),
                                            //0xffff0000,0xffff0000);
                        //      #endif

                        float cor = COLLIDE_BOT_R - res_len;
                        D3DXVec2Normalize(&res, &res);
                        res *= cor;
                        return D3DXVECTOR3(res.x, res.y, -1);
                    }
                }
                else if ((corner & 32) || (corner & 128))
                {
                    D3DXVECTOR3 v = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(rd.x, lu.y, 0);
                    //D3DXVECTOR3 v2 = D3DXVECTOR3(vLd.x, vLd.y, 0) - D3DXVECTOR3(vRu.x, vRu.y, 0);
                    D3DXVECTOR3 v2 = D3DXVECTOR3(-GLOBAL_SCALE_MOVE, GLOBAL_SCALE_MOVE, 0);

                    D3DXVECTOR3 proj = Vec3Projection(v2, v);
                    D3DXVECTOR2 point(rd.x + proj.x, lu.y + proj.y); //= vRu + D3DXVECTOR2(proj.x, );


                    D3DXVECTOR2 res = pos - point;
                    float res_len = D3DXVec2Length(&res);

                    //if((point.x >= vLd.x && point.x <= vRu.x && point.y >= vRu.y && point.y <= vLd.y) && res_len < COLLIDE_BOT_R){
                    if((point.x >= lu.x && point.x <= rd.x && point.y >= lu.y && point.y <= rd.y) && res_len < COLLIDE_BOT_R)
                    {
                        //#ifdef _SUB_BUG
                        //CHelper::Create(1,0)->Line(D3DXVECTOR3(vRu.x, vRu.y, 1), 
                              //D3DXVECTOR3(vLd.x, vLd.y, 1),
                              //0xffff0000, 0xffff0000);
                        //#endif
                        float cor = COLLIDE_BOT_R - res_len;
                        D3DXVec2Normalize(&res, &res);
                        res *= cor;

                        return D3DXVECTOR3(res.x, res.y, -1);
                    }
                }
                return D3DXVECTOR3(0, 0, 0);
            }
            else if(cr)
            {
                D3DXVECTOR3 vDist = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(lu.x + GLOBAL_SCALE_MOVE * 0.5f, lu.y + GLOBAL_SCALE_MOVE * 0.5f, 0);
                float dist = D3DXVec3Length/*Sq*/(&vDist);
                if(dist < (COLLIDE_BOT_R + COLLIDE_SPHERE_R)/**(COLLIDE_BOT_R + COLLIDE_SPHERE_R)*/)
                {
                    float correction = float(COLLIDE_BOT_R + COLLIDE_SPHERE_R - /*sqrt*/(dist));
                    D3DXVec3Normalize(&vDist, &vDist);
                    vDist *= correction;
                    return vDist;
                }
                return D3DXVECTOR3(0, 0, 0);
            }

            if(corner > 15) return D3DXVECTOR3(0, 0, 0);
        }

        D3DXVECTOR2 mind_corner = lu;
        float prev_min_dist = D3DXVec2LengthSq(&(pos - lu));
        int angle = 8;

        float a = D3DXVec2LengthSq(&(pos - D3DXVECTOR2(rd.x, lu.y)));
        if (a < prev_min_dist)
        {
            prev_min_dist = a;
            mind_corner = D3DXVECTOR2(rd.x, lu.y);
            angle = 1;
        }

        a = D3DXVec2LengthSq(&(pos - rd));
        if (a < prev_min_dist)
        {
            prev_min_dist = a;
            mind_corner = rd;
            angle = 2;
        }

        a = D3DXVec2LengthSq(&(pos - D3DXVECTOR2(lu.x, rd.y)));

        if (a < prev_min_dist)
        {
            prev_min_dist = a;
            mind_corner = D3DXVECTOR2(lu.x, rd.y);
            angle = 4;
        }

        bool _GOTCHA_ = false;
        if (angle == 1)
        {
            // r
            // u
            BYTE a1 = 255;
            BYTE a2 = 255;
            if (cell.x < (g_MatrixMap->m_SizeMove.x - 1)) a1 = (smm + 1)->GetType(m_Module[0].m_Kind - 1);
            if (cell.y > 0) a2 = (smm - g_MatrixMap->m_SizeMove.x)->GetType(m_Module[0].m_Kind - 1);

            //byte a1 = g_MatrixMap->GetCellMoveType(m_Module[0].m_Kind-1, int(vLu.x+GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE), int(vLu.y) / int(GLOBAL_SCALE_MOVE));
            //byte a2 = g_MatrixMap->GetCellMoveType(m_Module[0].m_Kind-1, int(vLu.x) / int(GLOBAL_SCALE_MOVE), int(vLu.y-GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE));

            if ((a1 > 15 && a1 != 255) && (a2 > 15 && a2 != 255))
            {
                return D3DXVECTOR3(0, 0, 0);
            }
            else if ((a1 > 15 && a1 != 255) || (a2 > 15 && a2 != 255))
            {
                _GOTCHA_ = true;
            }

        }
        else if (angle == 2)
        {

            // r
            // d
            BYTE a1 = 255;
            BYTE a2 = 255;
            if (cell.x < (g_MatrixMap->m_SizeMove.x - 1)) a1 = (smm + 1)->GetType(m_Module[0].m_Kind - 1);
            if (cell.y < (g_MatrixMap->m_SizeMove.y - 1)) a2 = (smm + g_MatrixMap->m_SizeMove.x)->GetType(m_Module[0].m_Kind - 1);

            //byte a1 = g_MatrixMap->GetCellMoveType(m_Module[0].m_Kind-1, int(vLu.x+GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE), int(vLu.y) / int(GLOBAL_SCALE_MOVE));
            //byte a2 = g_MatrixMap->GetCellMoveType(m_Module[0].m_Kind-1, int(vLu.x) / int(GLOBAL_SCALE_MOVE), int(vLu.y+GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE));

            if ((a1 > 15 && a1 != 255) && (a2 > 15 && a2 != 255))
            {
                return D3DXVECTOR3(0, 0, 0);
            }
            else if ((a1 > 15 && a1 != 255) || (a2 > 15 && a2 != 255))
            {
                _GOTCHA_ = true;
            }
        }
        else if(angle == 4)
        {
            // l
            // d
            byte a1 = 255;
            byte a2 = 255;
            if(cell.x > 0) a1 = (smm - 1)->GetType(m_Module[0].m_Kind - 1);
            if(cell.y < (g_MatrixMap->m_SizeMove.y - 1)) a2 = (smm + g_MatrixMap->m_SizeMove.x)->GetType(m_Module[0].m_Kind - 1);

            //byte a1 = g_MatrixMap->GetCellMoveType(m_Module[0].m_Kind-1, int(vLu.x-GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE), int(vLu.y) / int(GLOBAL_SCALE_MOVE));
            //byte a2 = g_MatrixMap->GetCellMoveType(m_Module[0].m_Kind-1, int(vLu.x) / int(GLOBAL_SCALE_MOVE), int(vLu.y+GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE));

            if((a1 > 15 && a1 != 255) && (a2 > 15 && a2 != 255))
            {
                return D3DXVECTOR3(0, 0, 0);
            }
            else if((a1 > 15 && a1 != 255) || (a2 > 15 && a2 != 255))
            {
                _GOTCHA_ = true;
            }
        }
        else if(angle == 8)
        {
            // l
            // u
            byte a1 = 255;
            byte a2 = 255;
            if(cell.x > 0) a1 = (smm - 1)->GetType(m_Module[0].m_Kind - 1);
            if(cell.y > 0) a2 = (smm - g_MatrixMap->m_SizeMove.x)->GetType(m_Module[0].m_Kind - 1);

            //byte a1 = g_MatrixMap->GetCellMoveType(m_Module[0].m_Kind-1, int(vLu.x-GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE), int(vLu.y) / int(GLOBAL_SCALE_MOVE));
            //byte a2 = g_MatrixMap->GetCellMoveType(m_Module[0].m_Kind-1, int(vLu.x) / int(GLOBAL_SCALE_MOVE), int(vLu.y-GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE));

            if((a1 > 15 && a1 != 255) && (a2 > 15 && a2 != 255))
            {
                return D3DXVECTOR3(0, 0, 0);
            }
            else if((a1 > 15 && a1 != 255) || (a2 > 15 && a2 != 255))
            {
                _GOTCHA_ = true;
            }

        }

        float dx = pos.x - mind_corner.x;
        dx *= dx;

        float dy = pos.y - mind_corner.y;
        dy *= dy;

        D3DXVECTOR3 result(0, 0, 0), result2(0, 0, 0);

        if (dx > dy)
        {
            result = D3DXVECTOR3(pos.x, 0, 0) - D3DXVECTOR3(mind_corner.x, 0, 0);
            result2 = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(mind_corner.x, mind_corner.y, 0);
            D3DXVECTOR3 gotcha;
            D3DXVec3Normalize(&gotcha, &result2);

            float cos1 = gotcha.x * 1 + gotcha.y * 0;
            float cos2 = gotcha.x * 0 + gotcha.y * 1;
            if (_GOTCHA_)
            {
                if (angle == 1 && (cos1 > 0 && cos2 < 0))
                {
                    result = result2;
                }
                else if (angle == 2 && (cos1 > 0 && cos2 > 0))
                {
                    result = result2;
                }
                else if (angle == 4 && (cos1 < 0 && cos2 >0))
                {
                    result = result2;
                }
                else if (angle == 8 && (cos1 < 0 && cos2 < 0))
                {
                    result = result2;
                }
            }
            D3DXVec3Normalize(&result, &result);
            result *= COLLIDE_BOT_R - (float)sqrt(dcol);

        }
        else if (dx < dy)
        {
            result = D3DXVECTOR3(0, pos.y, 0) - D3DXVECTOR3(0, mind_corner.y, 0);
            result2 = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(mind_corner.x, mind_corner.y, 0);
            D3DXVECTOR3 gotcha;
            D3DXVec3Normalize(&gotcha, &result2);
            float cos1 = gotcha.x * 1 + gotcha.y * 0;
            float cos2 = gotcha.x * 0 + gotcha.y * 1;

            if (_GOTCHA_) {
                if (angle == 1 && (cos1 > 0 && cos2 < 0))
                {
                    result = result2;
                }
                else if (angle == 2 && (cos1 > 0 && cos2 > 0))
                {
                    result = result2;
                }
                else if (angle == 4 && (cos1 < 0 && cos2 >0))
                {
                    result = result2;
                }
                else if (angle == 8 && (cos1 < 0 && cos2 < 0))
                {
                    result = result2;
                }
            }
            D3DXVec3Normalize(&result, &result);
            result *= COLLIDE_BOT_R - (float)sqrt(dcol);
        }

        return result;
    }
    //}
    return D3DXVECTOR3(0, 0, 0);
}

bool CMatrixRobotAI::SphereToAABBCheck(const D3DXVECTOR2& p, const D3DXVECTOR2& vMin, const D3DXVECTOR2& vMax, float& d, float& dsx, float& dsy)
{
    float distance = 0;

    if (p.x < vMin.x)
    {
        float dx = p.x - vMin.x;
        distance += dx * dx;
        dsx += dx * dx;
    }
    else if (p.x > vMax.x)
    {
        float dx = p.x - vMax.x;
        distance += dx * dx;
        dsx += dx * dx;
    }

    if (p.y < vMin.y)
    {
        float dy = p.y - vMin.y;
        distance += dy * dy;
        dsy += dy * dy;
    }
    else if (p.y > vMax.y)
    {
        float dy = p.y - vMax.y;
        distance += dy * dy;
        dsy += dy * dy;
    }

    d = distance;
    return distance <= (COLLIDE_BOT_R * COLLIDE_BOT_R);
}
//void CMatrixRobotAI::OBBToAABBCollision(int nHeight, int nWidth)
//{
//		
//	D3DXVECTOR2 curBlock, checkingBlock, point_tmp, s_field(0,0), s_me(0,0);
//	D3DXVECTOR2 botDirFN(m_Module[0].m_Matrix._21, m_Module[0].m_Matrix._22),
//		botDirUN(m_Module[0].m_Matrix._11, m_Module[0].m_Matrix._12),
//		botDirBN, botDirDN, worldDirFN(0, 1), worldDirUN(1, 0), worldDirBN(0, -1),worldDirDN(-1, 0);
//
//	D3DXVec2Normalize(&botDirFN, &botDirFN);
//	D3DXVec2Normalize(&botDirUN, &botDirUN);
//
//	botDirBN = botDirFN * (-1);
//	botDirDN = botDirUN * (-1);
//
//	D3DXVECTOR2 future_pos;
//	D3DXVECTOR2 vLu;
//	D3DXVECTOR2 vRu;
//	D3DXVECTOR2 vLd;
//	D3DXVECTOR2 vRd;
//			
//	D3DXVECTOR3 Velocity(m_Velocity);
//
//	for(int nE = 1; nE <= 1; nE++){
//		D3DXVECTOR3 vRight(0, 0, 0), vRightPrev(0,0, 0);
//		D3DXVECTOR3 vLeft(0, 0, 0), vLeftPrev(0, 0, 0);
//		D3DXVECTOR3 vForward(0, 0, 0), vForwardPrev(0, 0, 0);
//		D3DXVECTOR3 vBackward(0, 0, 0), vBackwardPrev(0, 0, 0);
//		D3DXVECTOR3 vLuCor(0, 0, 0), vLuCorPrev(0, 0, 0);
//		D3DXVECTOR3 vRuCor(0, 0, 0), vRuCorPrev(0, 0, 0);
//		D3DXVECTOR3 vLdCor(0, 0, 0), vLdCorPrev(0, 0, 0);
//		D3DXVECTOR3 vRdCor(0, 0, 0), vRdCorPrev(0, 0, 0);
//			
//		future_pos = D3DXVECTOR2(m_PosX, m_PosY) + D3DXVECTOR2(Velocity.x, Velocity.y);
//		s_me = future_pos + ((botDirFN * (float)(nHeight * int(GLOBAL_SCALE)) / 2) + (botDirUN * (float)(nWidth * int(GLOBAL_SCALE)) / 2));
//	
//		s_field.y = float( (int(future_pos.y) / int(GLOBAL_SCALE)) - COLLIDE_FIELD_R);
//		s_field.x = float( (int(future_pos.x) / int(GLOBAL_SCALE)) - COLLIDE_FIELD_R);
//
//		vLu = s_me;
//		vRu = s_me + botDirDN * (float)(nWidth * int(GLOBAL_SCALE));
//		vLd = s_me + botDirBN * (float)(nHeight * int(GLOBAL_SCALE));
//		vRd = s_me + botDirBN * (float)(nHeight * int(GLOBAL_SCALE)) + botDirDN * (float)(nWidth * int(GLOBAL_SCALE));
//
//
///*
//		for(int i = 0; i < 7; i++) {
//			D3DXVECTOR3 v1=D3DXVECTOR3(s_field.x*GLOBAL_SCALE,s_field.y*GLOBAL_SCALE+GLOBAL_SCALE*i,1);
//			D3DXVECTOR3 v2=D3DXVECTOR3(s_field.x*GLOBAL_SCALE+GLOBAL_SCALE*7,s_field.y*GLOBAL_SCALE+GLOBAL_SCALE*i,1);
//
//			CHelper::Create(1,0)->Line(v1,v2,0xffff0000,0xffff0000);
//
//			v1=D3DXVECTOR3(s_field.x*GLOBAL_SCALE+GLOBAL_SCALE*i,s_field.y*GLOBAL_SCALE,1);
//			v2=D3DXVECTOR3(s_field.x*GLOBAL_SCALE+GLOBAL_SCALE*i,s_field.y*GLOBAL_SCALE+GLOBAL_SCALE*7,1);
//
//			CHelper::Create(1,0)->Line(v1,v2,0xffff0000,0xffff0000);
//		}
//*/
//		for(int outerLoop = 0; outerLoop <= 6; outerLoop++){
//			checkingBlock.y = (s_field.y + outerLoop);
//			for(int innerLoop = 0; innerLoop <= 6; innerLoop++){
//				checkingBlock.x = (s_field.x + innerLoop);
//
//				D3DXVECTOR2 vCellLu, vCellRu, vCellLd, vCellRd;
//				bool bReversX, bReversY;
//				bReversX = innerLoop>=3; 
//				bReversY = outerLoop>=3;
//
//				vCellLu.x = checkingBlock.x*int(GLOBAL_SCALE);
//				vCellLu.y = checkingBlock.y*int(GLOBAL_SCALE);
//
//				vCellRu.x = checkingBlock.x*int(GLOBAL_SCALE) + int(GLOBAL_SCALE);
//				vCellRu.y = checkingBlock.y*int(GLOBAL_SCALE);
//
//				vCellLd.x = checkingBlock.x*int(GLOBAL_SCALE);
//				vCellLd.y = checkingBlock.y*int(GLOBAL_SCALE) + int(GLOBAL_SCALE);
//			
//				vCellRd.x = checkingBlock.x*int(GLOBAL_SCALE) + int(GLOBAL_SCALE);
//				vCellRd.y = checkingBlock.y*int(GLOBAL_SCALE) + int(GLOBAL_SCALE);
//				
//
//				if(PointToAABB(vCellLu, vLu, (int)GLOBAL_SCALE, (int)GLOBAL_SCALE))
//				{
//				//Угол принадлежащий vLu лежит внутри ячейки
//					D3DXVECTOR3 vTmp	= CornerLineToAABBIntersection(vLu, vRu, vCellLu, vCellLd, vCellRu, vCellRd);
//					D3DXVECTOR3 vTmp2	= CornerLineToAABBIntersection(vLu, vLd, vCellLu, vCellLd, vCellRu, vCellRd);
//					if(D3DXVec3LengthSq(&vTmp) < D3DXVec3LengthSq(&vTmp2))
//						vLuCor = vTmp;
//					else
//						vLuCor = vTmp2;
//
//					if(D3DXVec3LengthSq(&vLuCor) > D3DXVec3LengthSq(&vLuCorPrev))
//						vLuCorPrev = vLuCor;
//					
//				}else if(PointToAABB(vCellLu, vRu, (int)GLOBAL_SCALE, (int)GLOBAL_SCALE)){
//				//Угол принадлежащий vRu лежит внутри ячейки
//					D3DXVECTOR3 vTmp	= CornerLineToAABBIntersection(vRu, vLu, vCellLu, vCellLd, vCellRu, vCellRd);
//					D3DXVECTOR3 vTmp2	= CornerLineToAABBIntersection(vRu, vRd, vCellLu, vCellLd, vCellRu, vCellRd);
//
//					if(D3DXVec3LengthSq(&vTmp) < D3DXVec3LengthSq(&vTmp2))
//						vRuCor = vTmp;
//					else
//						vRuCor = vTmp2;
//
//					if(D3DXVec3LengthSq(&vRuCor) > D3DXVec3LengthSq(&vRuCorPrev))
//						vRuCorPrev = vRuCor;
//
//				} else if (PointToAABB(vCellLu, vLd, (int)GLOBAL_SCALE, (int)GLOBAL_SCALE)){
//				//Угол принадлежащий vLd лежит внутри ячейки
//					D3DXVECTOR3 vTmp	= CornerLineToAABBIntersection(vLd, vLu, vCellLu, vCellLd, vCellRu, vCellRd);
//					D3DXVECTOR3 vTmp2	= CornerLineToAABBIntersection(vLd, vRd, vCellLu, vCellLd, vCellRu, vCellRd);
//
//					if(D3DXVec3LengthSq(&vTmp) < D3DXVec3LengthSq(&vTmp2))
//						vLdCor = vTmp;
//					else
//						vLdCor = vTmp2;
//
//					if(D3DXVec3LengthSq(&vLdCor) > D3DXVec3LengthSq(&vLdCorPrev))
//						vLdCorPrev = vLdCor;
//
//				} else if (PointToAABB(vCellLu, vRd, (int)GLOBAL_SCALE, (int)GLOBAL_SCALE)){
//				//Угол принадлежащий vRd лежит внутри ячейки
//					D3DXVECTOR3 vTmp	= CornerLineToAABBIntersection(vRd, vRu, vCellLu, vCellLd, vCellRu, vCellRd);
//					D3DXVECTOR3 vTmp2	= CornerLineToAABBIntersection(vRd, vLd, vCellLu, vCellLd, vCellRu, vCellRd);
//
//					if(D3DXVec3LengthSq(&vTmp) < D3DXVec3LengthSq(&vTmp2))
//						vLuCor = vTmp;
//					else
//						vLuCor = vTmp2;
//
//					if(D3DXVec3LengthSq(&vRdCor) > D3DXVec3LengthSq(&vRdCorPrev))
//						vRdCorPrev = vRdCor;
//
//				}else{
////Right
//					vRight = LineToAABBIntersection(vRu, vRd, vCellLu, vCellLd, vCellRu, vCellRd, bReversX, bReversY);
//					if(D3DXVec3LengthSq(&vRight) > D3DXVec3LengthSq(&vRightPrev)){
//						vRightPrev = vRight;
//					}
//				
////Left
//					vLeft = LineToAABBIntersection(vLu, vLd, vCellLu, vCellLd, vCellRu, vCellRd, bReversX, bReversY);
//					if(D3DXVec3LengthSq(&vLeft) > D3DXVec3LengthSq(&vLeftPrev)){
//						vLeftPrev = vLeft;
//					}
//
//				
////Backward
//					vBackward = LineToAABBIntersection(vLd, vRd, vCellLu, vCellLd, vCellRu, vCellRd, bReversX, bReversY);
//					if(D3DXVec3LengthSq(&vBackward) > D3DXVec3LengthSq(&vBackwardPrev)){
//						vBackwardPrev = vBackward;
//					}
//
//				
////Forward
//					vForward = LineToAABBIntersection(vLu, vRu, vCellLu, vCellLd, vCellRu, vCellRd, bReversX, bReversY);
//					if(D3DXVec3LengthSq(&vForward) > D3DXVec3LengthSq(&vForwardPrev)){
//						vForwardPrev = vForward;
//					}
//				}
//			}
//		}
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), 
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + vRightPrev,
////			0xffff0000,0xffff0000);		
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), 
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + vLeftPrev,
////			0xffff0000,0xffff0000);		
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), 
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + vForwardPrev,
////			0xffff0000,0xffff0000);		
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), 
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + vBackwardPrev,
////			0xffff0000,0xffff0000);		
//
//		Velocity += vRightPrev + vLeftPrev + vForwardPrev + vBackwardPrev + vLuCorPrev + vRuCorPrev + vLdCorPrev +vRdCorPrev;
//
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), 
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + Velocity*10,
////			0xffffffff,0xffff0000);		
//
//	}
//	
//
////	Velocity = Vec3Truncate(Velocity, m_maxSpeed);
//	m_PosX += Velocity.x;
//	m_PosY += Velocity.y;
//
//}

//D3DXVECTOR3 CMatrixRobotAI::LineToAABBIntersection(const D3DXVECTOR2 &s,const D3DXVECTOR2 &e, const D3DXVECTOR2 &vLu,const D3DXVECTOR2 &vLd,const D3DXVECTOR2 &vRu,const D3DXVECTOR2 &vRd, bool revers_x, bool revers_y)
//{
//	if(!g_MatrixMap->PlaceIsEmpty(m_Module[0].m_Kind-1, 1, int(vLu.x / int(GLOBAL_SCALE_MOVE)), int(vLu.y / int(GLOBAL_SCALE_MOVE)))){
//		int line = 0;
//
//		float u1 = 0,u2 = 0,u4 = 0,u8 = 0, t1 = 0, t2 = 0, t4 = 0, t8 = 0;
//			
//		line = 0;
////line1(-)top
//		if(IntersectLine(s, e,	vLu, vRu, &t1, &u1) && t1 >= 0.0f && t1 <= 1.0 && u1 >= 0.0f && u1 <= 1.0f){
//			line |= 1;
//		}
////line2(|)left	
//		if(IntersectLine(s, e,	vLu, vLd, &t2, &u2) && t2 >= 0.0f && t2 <= 1.0 && u2 >= 0.0f && u2 <= 1.0f){
//			line |= 2;
//		}
//
////line3(|)right
//		if(IntersectLine(s, e, vRu, vRd, &t4, &u4) && t4 >= 0.0f && t4 <= 1.0 && u4 >= 0.0f && u4 <= 1.0f){
//			line |= 4;
//		}
//
////line4(-)bottom
//		if(IntersectLine(s, e,	vLd, vRd, &t8, &u8) && t8 >= 0.0f && t8 <= 1.0 && u8 >= 0.0f && u8 <= 1.0f){
//			line |= 8;
//		}
//
//		if(line != 0){
//	/*CHelper::Create(1,0)->Line(D3DXVECTOR3(s.x, s.y, 1), D3DXVECTOR3(e.x, e.y, 1),0xffffffff,0xffffffff);*/
///*
//
//
//	CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1), 
//						 D3DXVECTOR3(vRu.x, vRu.y, 1),
//						 0xffff0000,0xffff0000);
//	CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1), 
//						 D3DXVECTOR3(vLd.x, vLd.y, 1),
//						 0xffff0000,0xffff0000);
//	CHelper::Create(1,0)->Line(D3DXVECTOR3(vRu.x, vRu.y, 1), 
//						 D3DXVECTOR3(vRd.x, vRd.y, 1),
//						 0xffff0000,0xffff0000);
//	CHelper::Create(1,0)->Line(D3DXVECTOR3(vLd.x, vLd.y, 1), 
//						 D3DXVECTOR3(vRd.x, vRd.y, 1),
//						 0xffff0000,0xffff0000);
//*/
//		}
//		D3DXVECTOR2 r, r2;
//		if(line==(2|4)) {
//			if(!revers_y) {
//				r=D3DXVECTOR2(0.0f,min(1.0f-u2,1.0f-u4)*GLOBAL_SCALE_MOVE);
//
////				if((1.0f-u2)<(1.0f-u4)) HelperT(vLd-r,vLd);
////				else HelperT(vRd-r,vRd);
//
//				return D3DXVECTOR3(r.x, r.y, 0);
//			} else {
//				r=D3DXVECTOR2(0.0f,(min(u2,u4))*GLOBAL_SCALE_MOVE);
//
////				if((u2)<(u4)) HelperT(vLu+r,vLu);
////				else HelperT(vRu+r,vRu);
//
//				return D3DXVECTOR3(-r.x, -r.y, 0);
//			}
//
//		} else if(line==(1|8)) {
//			if(!revers_x) {
//				r=D3DXVECTOR2(min(1.0f-u1,1.0f-u8)*GLOBAL_SCALE_MOVE,0.0f);
//
////				if((1.0f-u2)<(1.0f-u4)) HelperT(vRu-r,vRu);
////				else HelperT(vRd-r,vRd);
//
//				return D3DXVECTOR3(r.x, r.y, 0);
//			} else {
//				r=D3DXVECTOR2((min(u1,u8))*GLOBAL_SCALE_MOVE,0.0f);
//
////				if((u2)<(u4)) HelperT(vLu+r,vLu);
////				else HelperT(vLd+r,vLd);
//
//				return D3DXVECTOR3(-r.x, -r.y, 0);
//			}
//		} else if(line == (1|4)) {
//			r=D3DXVECTOR2((1.0f - u1)*GLOBAL_SCALE_MOVE, 0.0f);			
//			r2=D3DXVECTOR2(0.0f, u4*GLOBAL_SCALE_MOVE);
//
//	//		HelperT(vRu - r,vRu+r2);
//	//		HelperT(vRu - r,vRu);
//	//		HelperT(vRu,vRu+r2);
//
//			if((1.0f - u1) < u4){
//				return D3DXVECTOR3(r.x, r.y, 0);
//			}
//			else{
//				return D3DXVECTOR3(-r2.x, -r2.y, 0);
//
//			}
//		} else if(line == (8|4)) {
//			r=D3DXVECTOR2((1.0f - u8)*GLOBAL_SCALE_MOVE, 0.0f);			
//			r2=D3DXVECTOR2(0.0f, (1.0f - u4)*GLOBAL_SCALE_MOVE);
//
//	//		HelperT(vRd - r,vRd-r2);
//	//		HelperT(vRd - r,vRd);
//	//		HelperT(vRd,vRd-r2);
//
//
//			if((1.0f - u8) < (1.0f - u4)){
//				return D3DXVECTOR3(r.x, r.y, 0);
//			}
//			else{
//				return D3DXVECTOR3(r2.x, r2.y, 0);
//
//			}
//		} else if(line == (1|2)) {
//			r=D3DXVECTOR2(u1*GLOBAL_SCALE_MOVE, 0.0f);			
//			r2=D3DXVECTOR2(0.0f, u2*GLOBAL_SCALE_MOVE);
//
//	//		HelperT(vLu, vLu + r);
//	//		HelperT(vLu, vLu + r2);
//	//		HelperT(vLu + r2, vLu + r);
//
//			if(u1 < u2){
//				return D3DXVECTOR3(-r.x, -r.y, 0);
//			}
//			else{
//				return D3DXVECTOR3(-r2.x, -r2.y, 0);
//			}
//		} else if(line == (8|2)) {
//			r=D3DXVECTOR2(u8*GLOBAL_SCALE_MOVE, 0.0f);			
//			r2=D3DXVECTOR2(0.0f, (1.0f - u2)*GLOBAL_SCALE_MOVE);
//
//	//		HelperT(vLd, vLd + r);
//	//		HelperT(vLd, vLd - r2);
//	//		HelperT(vLd - r2,vLd + r);
//
//			if(u8 < (1.0f - u2)){
//				return D3DXVECTOR3(-r.x, -r.y, 0);
//			}
//			else{
//				return D3DXVECTOR3(r2.x, r2.y, 0);
//			}
//		} 
//		else return D3DXVECTOR3(0,0,0);
//	}
//	return D3DXVECTOR3(0,0,0);
//}

//D3DXVECTOR3 CMatrixRobotAI::CornerLineToAABBIntersection(const D3DXVECTOR2 &s,const D3DXVECTOR2 &e, const D3DXVECTOR2 &vLu,const D3DXVECTOR2 &vLd,const D3DXVECTOR2 &vRu,const D3DXVECTOR2 &vRd)
//{
//	if(!g_MatrixMap->PlaceIsEmpty(m_Module[0].m_Kind-1, 1, int(vLu.x / int(GLOBAL_SCALE_MOVE)), int(vLu.y / int(GLOBAL_SCALE_MOVE)))){
//		int line = 0;
//
//		float u1 = 0,u2 = 0,u4 = 0,u8 = 0, t1 = 0, t2 = 0, t4 = 0, t8 = 0;
//			
////line1(-)top
//		if(IntersectLine(s, e,	vLu, vRu, &t1, &u1) && t1 >= 0.0f && t1 <= 1.0 && u1 >= 0.0f && u1 <= 1.0f){
//			line = 1;
//		}
////line2(|)left	
//		if(IntersectLine(s, e,	vLu, vLd, &t2, &u2) && t2 >= 0.0f && t2 <= 1.0 && u2 >= 0.0f && u2 <= 1.0f){
//			line = 2;
//		}
//
////line3(|)right
//		if(IntersectLine(s, e, vRu, vRd, &t4, &u4) && t4 >= 0.0f && t4 <= 1.0 && u4 >= 0.0f && u4 <= 1.0f){
//			line = 4;
//		}
//
////line4(-)bottom
//		if(IntersectLine(s, e,	vLd, vRd, &t8, &u8) && t8 >= 0.0f && t8 <= 1.0 && u8 >= 0.0f && u8 <= 1.0f){
//			line = 8;
//		}
//
//
//		if(line != 0){
////CHelper::Create(1,0)->Line(D3DXVECTOR3(s.x, s.y, 1), D3DXVECTOR3(e.x, e.y, 1),0xffffffff,0xffffffff);
///*
//CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1), 
//						 D3DXVECTOR3(vRu.x, vRu.y, 1),
//						 0xffff0000,0xffff0000);
//CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1), 
//						 D3DXVECTOR3(vLd.x, vLd.y, 1),
//						 0xffff0000,0xffff0000);
//CHelper::Create(1,0)->Line(D3DXVECTOR3(vRu.x, vRu.y, 1), 
//						 D3DXVECTOR3(vRd.x, vRd.y, 1),
//						 0xffff0000,0xffff0000);
//CHelper::Create(1,0)->Line(D3DXVECTOR3(vLd.x, vLd.y, 1), 
//						 D3DXVECTOR3(vRd.x, vRd.y, 1),
//						 0xffff0000,0xffff0000);
//*/
//		}
//
//		if(line == 1){
//			D3DXVECTOR2  napr = e - s;
//			napr *= t1;
////			HelperT(s, s + napr);
//			return D3DXVECTOR3(napr.x, napr.y, 0);
//		} else if(line == 2){
//			D3DXVECTOR2  napr = e - s;
//			napr *= t2;
////			HelperT(s, s + napr);
//			return D3DXVECTOR3(napr.x, napr.y, 0);
//
//		} else if(line == 4){
//			D3DXVECTOR2  napr = e - s;
//			napr *= t4;
////			HelperT(s, s + napr);
//			return D3DXVECTOR3(napr.x, napr.y, 0);
//
//		} else if(line == 8){
//			D3DXVECTOR2  napr = e - s;
//			napr *= t8;
////			HelperT(s, s + napr);
//			return D3DXVECTOR3(napr.x, napr.y, 0);
//		}
//		else return D3DXVECTOR3(0,0,0);
//	}
//	return D3DXVECTOR3(0,0,0);
//}



//void CMatrixRobotAI::Load(CBuf& buf, CTemporaryLoadData*)
//{
//DTRACE();
//
//
//	m_PosX=float(buf.Int());
//	m_PosY=float(buf.Int());
//
//	m_PosX=GLOBAL_SCALE*(m_PosX+buf.Float());
//    m_PosY=GLOBAL_SCALE*(m_PosY+buf.Float());
//
//	m_Side=buf.Byte();
//
//    bool arm = false;
//
//	int cnt = buf.Byte();
//	for(int i = 0; i < cnt; ++i)
//    {
//		ERobotModuleType type = (ERobotModuleType)buf.Byte();
//		ERobotModuleKind kind = (ERobotModuleKind)buf.Byte();
//		ModuleInsert(i,type,kind);
//        float a = buf.Float();
//        
//        if (type == MRT_HULL) arm = true;
//        
//        if (type == MRT_CHASSIS)
//        {
//            m_ChassisForward.z = 0;
//            SinCos(a, &m_ChassisForward.x, &m_ChassisForward.y);
//            (*(dword *)&m_ChassisForward.x) ^= 0x80000000;
//            m_Module[i].m_Angle = 0;
//            m_HullForward = m_ChassisForward;
//        }
//        else
//        {
//            m_Module[i].m_Angle = a;
//        }
//
//        D3DXMatrixIdentity(&m_Module[i].m_Matrix);
//	}
//
//    D3DXMatrixIdentity(&m_Core->m_Matrix);
//
//	m_Core->m_ShadowType = (EShadowType)buf.Byte();
//  //m_ShadowType=0;
//	m_Core->m_ShadowSize=buf.Word();
//
//    m_CurrState = ROBOT_SUCCESSFULLY_BUILD;
//
//    if(!arm) 
//    {
//        g_MatrixMap->StaticDelete(this);
//        return;
//    }
//
//    WeaponSelectMatrix(); //Есть в последней релизной версии, но тут было удалено за ненадобностью
//    RobotWeaponInit();
//	  CalcRobotParams();
//
//    RChange(MR_Graph);
//    GetResources(MR_Graph);
//
//    SwitchAnimation(ANIMATION_STAY);
//}

void robot_weapon_hit(CMatrixMapStatic* hit, const D3DXVECTOR3& pos, dword user, dword flags)
{
    CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
    while(obj)
    {
        if(user == dword(obj))
        {
            if(obj->AsRobot()->m_CurrState != ROBOT_DIP) obj->AsRobot()->HitTo(hit, pos);
            return;
        }
        obj = obj->GetNextLogic();
    }
}

void CMatrixRobotAI::HitTo(CMatrixMapStatic* hit, const D3DXVECTOR3& pos)
{
    if(IS_TRACE_STOP_OBJECT(hit))
    {
        if(GetEnv()->m_Target == hit) GetEnv()->m_LastHitEnemy = GetEnv()->m_LastHitTarget = g_MatrixMap->GetTime();
        else if(hit->GetObjectType() == OBJECT_TYPE_BUILDING);
        else if(hit->GetSide() != GetSide()) GetEnv()->m_LastHitEnemy = g_MatrixMap->GetTime();
        else GetEnv()->m_LastHitFriendly = g_MatrixMap->GetTime();

        /*
        if(GetSide() != PLAYER_SIDE)
        {
            CMatrixSideUnit* side = g_MatrixMap->GetSideById(GetSide());
            side->m_Team[GetTeam()].m_War = true;
        }
        */

        //Если робот стреляет в пушку, и мы в него попали, то робот переключается на нас
        if(hit->GetObjectType() == OBJECT_TYPE_ROBOTAI)
        {
            CMatrixRobotAI* robot = (CMatrixRobotAI*)hit;

            if(robot != this && robot->GetSide() != GetSide() && !robot->m_Environment.SearchEnemy(this)) robot->m_Environment.AddToList(this);

            if(robot->GetEnv()->m_TargetAttack != nullptr && robot->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_CANNON && robot->GetEnv()->SearchEnemy(this) && (g_MatrixMap->GetTime() - GetEnv()->m_TargetChange) > 1000)
            {
                GetEnv()->m_TargetLast = GetEnv()->m_TargetAttack;
                robot->GetEnv()->m_TargetAttack = this;
                GetEnv()->m_TargetChange = g_MatrixMap->GetTime();
            }
        }
    }
}

void CMatrixRobotAI::RobotWeaponInit()
{
    //В принципе, здесь есть возможность уменьшить размер массива m_Weapons[] для роботов, у которых под орудия заняты не все 5 слотов,
    //но единственным преимуществом будет крайне незначительное уменьшение размера объекта робота, а минусом,
    //создание динамического массива с неопределёнными заранее смещениями, что немного замедлит переборы орудий (короче, так себе затея)
    int Weap = 0;
    for(int i = m_ModulesCount - 1; i >= 0; --i) //Расположение орудий в списке модулей обратное, так что делаем перебор с инверсией
    {
        if(m_Module[i].m_Type == MRT_WEAPON)
        {
            int kind = m_Module[i].m_Kind;

            m_Weapons[Weap].m_Module = &m_Module[i];

            m_Weapons[Weap].CreateEffect((dword)this, &robot_weapon_hit, g_Config.m_RobotWeaponsConsts[kind].weapon_type);

            m_Weapons[Weap].m_HeatingSpeed = g_Config.m_RobotWeaponsConsts[kind].heating_speed;
            m_Weapons[Weap].m_CoolingSpeed = g_Config.m_RobotWeaponsConsts[kind].cooling_speed;
            //m_Weapons[Weap].m_CoolingDelay = 0; //Инициализируется в конструкторе
            //m_Weapons[Weap].m_Heat = 0;         //Инициализируется в конструкторе

            m_Weapons[Weap].SetOwner(this);
            m_Weapons[Weap].m_On = true; //По умолчанию выключено

            ++Weap;
        }
    }

    m_WeaponsCount = Weap;
}

void CMatrixRobotAI::CalcRobotParams(SRobotTemplate* robot_template)
{
    m_HaveRepair = 0;

    bool normal_weapon = false;
    float hp = 0;
    for(int nC = 0; nC <= m_ModulesCount; ++nC)
    {
        int type = m_Module[nC].m_Type;
        int kind = m_Module[nC].m_Kind;

        switch(type)
        {
            case MRT_HULL:
            {
                hp += g_Config.m_RobotHullsConsts[kind].structure;
                m_maxHullSpeed = g_Config.m_RobotHullsConsts[kind].rotation_speed;
                break;
            }
            case MRT_CHASSIS:
            {
                hp += g_Config.m_RobotChassisConsts[kind].structure;
                m_maxSpeed = g_Config.m_RobotChassisConsts[kind].move_speed;
                m_maxStrafeSpeed = g_Config.m_RobotChassisConsts[kind].strafe_speed;
                m_SpeedSlopeCorrUp = g_Config.m_RobotChassisConsts[kind].move_uphill_factor;
                m_SpeedSlopeCorrDown = g_Config.m_RobotChassisConsts[kind].move_downhill_factor;
                m_SpeedWaterCorr = g_Config.m_RobotChassisConsts[kind].move_in_water_factor;
                m_maxRotationSpeed = g_Config.m_RobotChassisConsts[kind].rotation_speed;
                break;
            }
            case MRT_WEAPON:
            {
                m_HaveRepair = g_Config.m_RobotWeaponsConsts[kind].is_module_repairer;
                normal_weapon = (g_Config.m_RobotWeaponsConsts[kind].is_module_bomb == false);
                break;
            }
        }
    }

    if(m_HaveRepair && !normal_weapon) m_HaveRepair = 2;

    CBlockPar* bp = nullptr; //Инициализируем на случай, если у робота нет головы
    for(int nC = m_ModulesCount - 1; nC >= 0; --nC)
    {
        if(m_Module[nC].m_Type == MRT_HEAD)
        {
            bp = g_Config.m_RobotHeadsConsts[m_Module[nC].m_Kind].effects;
            break;
        }
    }

    float cool_down = 0;
    float fire_dist = 0;
    float overheat = 0;

    m_DischargerProtect = 0;
    m_BombProtect = 0;
    m_AimProtect = 0;
    m_RadarRadius = g_Config.m_RobotRadarRadius;

    m_SelfRepair = 0;
    m_SelfRepairPercent = 0;

    //Здесь происходит подгрузка модификаторов для модулей (голов) из конфигов
    if(bp)
    {
        hp += 10.0f * bp->ParGetNE(L"AddStructure").GetFloat();
        float hp_factor = bp->ParGetNE(L"AddStructurePercent").GetFloat() * 0.01f;

        if(hp_factor) hp += hp * hp_factor;
        hp = max(hp, 10.0f);

        m_SelfRepair = bp->ParGetNE(L"SelfRepair").GetFloat();
        if(m_SelfRepair) m_SelfRepair = m_SelfRepair / (1000.0f / LOGIC_TACT_PERIOD);

        m_SelfRepairPercent = min(bp->ParGetNE(L"SelfRepairPercent").GetFloat() * 0.01f, 1.0f);
        if(m_SelfRepairPercent) m_SelfRepairPercent = (m_SelfRepairPercent * hp) / (1000.0f / LOGIC_TACT_PERIOD);

        float up = bp->ParGetNE(L"ChassisSpeed").GetFloat() / 100.0f;
        if(up < -1.0f) up = -1.0f;
        m_maxSpeed += m_maxSpeed * up;
        m_maxStrafeSpeed += m_maxStrafeSpeed * up;
        m_maxRotationSpeed += m_maxRotationSpeed * up;

        cool_down = bp->ParGetNE(L"WeaponShotsDelay").GetFloat() / 100.0f;
        if(cool_down < -1.0f) cool_down = -1.0f;

        overheat = bp->ParGetNE(L"WeaponOverheat").GetFloat() / 100.0f;
        if(overheat < -1.0f) overheat = -1.0f;

        fire_dist = bp->ParGetNE(L"WeaponDistance").GetFloat() / 100.0f;
        if(fire_dist < -1.0f) fire_dist = -1.0f;

        //Бонусы для самонаведения ракет
        float angle = GRAD2RAD(min(max(bp->ParGetNE(L"MissileTargetCaptureAngle").GetFloat() / 2.0f, -180.0f), 180.0f));
        if(angle)
        {
            m_MissileTargetCaptureAngleCos = cos(angle);
            m_MissileTargetCaptureAngleSin = sin(angle);
        }
        m_MissileHomingSpeed = bp->ParGetNE(L"MissileHomingSpeed").GetFloat();

        up = bp->ParGetNE(L"RadarDistance").GetFloat() / 100.0f;
        if(up < -1.0f) up = -1.0f;
        m_RadarRadius += m_RadarRadius * up;

        m_AimProtect = bp->ParGetNE(L"AimProtect").GetFloat() / 100.0f;
        if(m_AimProtect < 0) m_AimProtect = 0;
        else if(m_AimProtect > 1) m_AimProtect = 1;

        m_BombProtect = bp->ParGetNE(L"BombProtect").GetFloat() / 100.0f;
        if(m_BombProtect >= 0) m_BombProtect = -m_BombProtect;
        else m_BombProtect = abs(m_BombProtect);
        if(m_BombProtect > 1) m_BombProtect = 1;

        m_DischargerProtect = bp->ParGetNE(L"DischargerProtect").GetFloat() / 100.0f;
        if(m_DischargerProtect >= 0) m_DischargerProtect = -m_DischargerProtect;
        else m_DischargerProtect = abs(m_DischargerProtect);

        if(m_DischargerProtect > 1) m_DischargerProtect = 1;
    }

    if(robot_template != nullptr && robot_template->m_HitPointsOverride) hp = robot_template->m_HitPointsOverride;
    InitMaxHitpoint(hp);

    float tmp_min = 1E30f, tmp_max = -1E30f, r_min = 1e30f;

    int cnt = 0;
    while(cnt < m_WeaponsCount)
    {
        m_Weapons[cnt].ModifyCoolDown(cool_down);

        m_Weapons[cnt].ModifyDist(fire_dist);
        m_Weapons[cnt].m_HeatingSpeed = Float2Int(float(m_Weapons[cnt].m_HeatingSpeed) + float(m_Weapons[cnt].m_HeatingSpeed) * overheat);

        if(m_Weapons[cnt].GetWeaponNum() == WEAPON_BOMB)
        {
            ++cnt;
            continue;
        }

        if(g_Config.m_WeaponsConsts[m_Weapons[cnt].GetWeaponNum()].is_repairer)
        {
            r_min = min(r_min, m_Weapons[cnt].GetWeaponDist());
            ++cnt;
            continue;
        }

        if(m_Weapons[cnt].GetWeaponDist() < tmp_min)
        {
            tmp_min = m_Weapons[cnt].GetWeaponDist();
        }

        if(m_Weapons[cnt].GetWeaponDist() > tmp_max)
        {
            tmp_max = m_Weapons[cnt].GetWeaponDist();
        }

        ++cnt;
    }

    if(tmp_max < 0.0f)
    {
        m_MaxFireDist = m_MinFireDist = 0.0f;
    }
    else
    {
        m_MaxFireDist = tmp_max;
        m_MinFireDist = tmp_min;
    }

    if(r_min > 1e20f) m_RepairDist = 0;
    else m_RepairDist = r_min;
}

void CMatrixRobotAI::GatherInfo(int type)
{
    CalcStrength();

    /*
    m_GatherPeriod += ms;

    CMatrixGroup* my_group = g_MatrixMap->GetSideById(m_Side)->GetGroup(m_Group, m_Team);
    if(my_group) m_GroupSpeed = my_group->GetGroupSpeed();

    if(m_GatherPeriod >= GATHER_PERIOD) m_GatherPeriod = 0;
    else return;
    */

    CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();

    if(!type)
    {
        //Look
        while(obj)
        {
            if(obj->IsRobotAlive() && obj != this && obj->GetSide() != m_Side)
            {
                CMatrixRobotAI* robot = (CMatrixRobotAI*)obj;
                D3DXVECTOR3 enemy_napr = D3DXVECTOR3(robot->m_PosX, robot->m_PosY, 0) - D3DXVECTOR3(m_PosX, m_PosY, 0);

                D3DXVECTOR3 en_norm(0, 0, 0);
                D3DXVec3Normalize(&en_norm, &enemy_napr);
                float cos = m_HullForward.x * en_norm.x + m_HullForward.y * en_norm.y;
                float angle_rad = (float)acos(cos);

                float dist_enemy = D3DXVec3LengthSq(&enemy_napr);

                if(dist_enemy <= POW2(max(robot->m_MaxFireDist, m_MaxFireDist) * 1.1)/*(D3DXVec3LengthSq(&enemy_napr) <= m_MinFireDist*m_MinFireDist) || (D3DXVec3LengthSq(&enemy_napr) <= m_MaxFireDist*m_MaxFireDist && angle_rad <= ROBOT_FOV)*/ && robot->m_CurrState != ROBOT_DIP)
                {
                    if(g_MatrixMap->IsLogicVisible(this, robot, 0.0f))
                    {
                        if(!m_Environment.SearchEnemy(robot) && !m_Environment.IsIgnore(robot))
                        {
                            CMatrixSideUnit* side = g_MatrixMap->GetSideById(GetSide());
                            int listcnt = 0;
                            int dist = 0;
                            CPoint p_from(m_MapX, m_MapY);
                            CPoint p_to(robot->m_MapX, robot->m_MapY);

                            side->BufPrepare();

                            float d = sqrt(float(p_from.Dist2(p_to)));
                            float z = fabs(g_MatrixMap->GetZ(robot->m_PosX, robot->m_PosY) - g_MatrixMap->GetZ(m_PosX, m_PosY));

                            /*
                            if((z / (d * GLOBAL_SCALE_MOVE)) >= tan(BARREL_TO_SHOT_ANGLE)) m_Environment.AddIgnore(robot);
                            else
                            */
                            if(g_MatrixMap->PlaceList(m_Module[0].m_Kind - 1, p_from, p_to, Float2Int(GetMaxFireDist() / GLOBAL_SCALE_MOVE/* + ROBOT_MOVECELLS_PER_SIZE*/), false, side->m_PlaceList, &listcnt, &dist))
                            {
                                if(POW2(dist / 4) < p_from.Dist2(p_to)) m_Environment.AddToList(robot);
                                else m_Environment.AddIgnore(robot);
                            }
                            else m_Environment.AddIgnore(robot);
                        }
                    }
                    /*
                    D3DXVECTOR3 rem;
                    CMatrixMapStatic* trace_res = nullptr;
                    trace_res = g_MatrixMap->Trace(&rem, GetGeoCenter(), robot->GetGeoCenter(), TRACE_ANYOBJECT | TRACE_NONOBJECT | TRACE_OBJECTSPHERE | TRACE_SKIP_INVISIBLE, this);
                    if((IS_TRACE_STOP_OBJECT(trace_res) && trace_res == robot) && !m_Environment.SearchEnemy(robot)) m_Environment.AddToList(obj);
                    else
                    {
                        D3DXVECTOR3 v1 = GetGeoCenter();
                        v1.z += 50.0f;
                        trace_res = g_MatrixMap->Trace(&rem, v1, robot->GetGeoCenter(), TRACE_ANYOBJECT | TRACE_NONOBJECT | TRACE_OBJECTSPHERE | TRACE_SKIP_INVISIBLE, this);
                        if((IS_TRACE_STOP_OBJECT(trace_res) && trace_res == robot) && !m_Environment.SearchEnemy(robot)) m_Environment.AddToList(obj);
                    }
                    */
                }
                else
                {
                    if(/*robot->m_FireTarget != this && */ dist_enemy > POW2(max(robot->m_MaxFireDist, m_MaxFireDist) * 1.4)) m_Environment.RemoveFromListSlowly(obj);
                }
            }
            else if (obj->IsCannonAlive() && obj->AsCannon()->m_CurrState != CANNON_UNDER_CONSTRUCTION && obj->GetSide() != m_Side)
            {
                CMatrixCannon* cannon = (CMatrixCannon*)obj;
                D3DXVECTOR3 enemy_napr = cannon->GetGeoCenter() - D3DXVECTOR3(m_PosX, m_PosY, 0);

                D3DXVECTOR3 en_norm(0, 0, 0);
                D3DXVec3Normalize(&en_norm, &enemy_napr);
                float cos = m_HullForward.x * en_norm.x + m_HullForward.y * en_norm.y;
                float angle_rad = acos(cos);

                float dist_enemy = D3DXVec3LengthSq(&enemy_napr);

                if(dist_enemy <= POW2(max(cannon->GetFireRadius() * 1.01, m_MaxFireDist * 1.1)) /*(D3DXVec3LengthSq(&enemy_napr) <= m_MinFireDist*m_MinFireDist)*//* || (D3DXVec3LengthSq(&enemy_napr) <= m_MaxFireDist*m_MaxFireDist && angle_rad <= ROBOT_FOV) */ && cannon->m_CurrState != CANNON_DIP)
                {
                    if(g_MatrixMap->IsLogicVisible(this, cannon, 0.0f))
                    {
                        if(!m_Environment.SearchEnemy(cannon) && !m_Environment.IsIgnore(cannon))
                        {
                            CMatrixSideUnit* side = g_MatrixMap->GetSideById(GetSide());
                            int listcnt = 0;
                            int dist = 0;
                            CPoint p_from(m_MapX, m_MapY);
                            CPoint p_to(Float2Int(cannon->m_Pos.x / GLOBAL_SCALE_MOVE), Float2Int(cannon->m_Pos.y / GLOBAL_SCALE_MOVE));

                            side->BufPrepare();

                            float d = sqrt(float(p_from.Dist2(p_to)));
                            //float z=fabs(g_MatrixMap->GetZ(cannon->m_Pos.x,cannon->m_Pos.y)-g_MatrixMap->GetZ(m_PosX,m_PosY));
                            float z = fabs(cannon->GetGeoCenter().z - GetGeoCenter().z);

                            if((z / (d * GLOBAL_SCALE_MOVE)) >= tan(BARREL_TO_SHOT_ANGLE))
                            {
                                m_Environment.AddIgnore(cannon);
                            }
                            else if(g_MatrixMap->PlaceList(m_Module[0].m_Kind - 1, p_from, p_to, Float2Int(GetMaxFireDist() / GLOBAL_SCALE_MOVE/*+ROBOT_MOVECELLS_PER_SIZE*/), false, side->m_PlaceList, &listcnt, &dist))
                            {
                                if(POW2(dist / 4) < p_from.Dist2(p_to))
                                {
                                    m_Environment.AddToList(cannon);
                                }
                                else
                                {
                                    m_Environment.AddIgnore(cannon);
                                }
                            }
                            else m_Environment.AddIgnore(cannon);
                        }
                    }
                    /*                D3DXVECTOR3 rem;

                                    CMatrixMapStatic* trace_res = nullptr;
                                    trace_res = g_MatrixMap->Trace(&rem, GetGeoCenter(), cannon->GetGeoCenter(), TRACE_ANYOBJECT|TRACE_NONOBJECT|TRACE_OBJECTSPHERE|TRACE_SKIP_INVISIBLE, this);
                                    if((IS_TRACE_STOP_OBJECT(trace_res) && trace_res == cannon) && !m_Environment.SearchEnemy(cannon)){
                                        m_Environment.AddToList(obj);
                                    } else {
                                        D3DXVECTOR3 v1=GetGeoCenter(); v1.z+=50.0f;
                                        trace_res = g_MatrixMap->Trace(&rem, v1, cannon->GetGeoCenter(), TRACE_ANYOBJECT|TRACE_NONOBJECT|TRACE_OBJECTSPHERE|TRACE_SKIP_INVISIBLE, this);
                                        if((IS_TRACE_STOP_OBJECT(trace_res) && trace_res == cannon) && !m_Environment.SearchEnemy(cannon)){
                                            m_Environment.AddToList(obj);
                                        }
                                    }*/
                }
                else
                {
                    if(cannon->m_TargetCore != this->m_Core && dist_enemy > POW2(max(cannon->GetFireRadius(), m_MaxFireDist) * 1.5)) m_Environment.RemoveFromListSlowly(obj);
                }
            }

            obj = obj->GetNextLogic();
        }
    }
    else if(type == 1)
    {
        int r = GetRegion();

        //Get info about enemies by radio
        obj = CMatrixMapStatic::GetFirstLogic();
        while(obj)
        {
            if(this != obj && obj->GetObjectType() == OBJECT_TYPE_ROBOTAI && obj->GetSide() == GetSide())
            {
                //Если в одной группе
                if(obj->AsRobot()->m_GroupLogic == m_GroupLogic)
                {
                    CEnemy* enemie = obj->AsRobot()->m_Environment.m_FirstEnemy;
                    while(enemie)
                    {
                        if(enemie->GetEnemy()->GetSide() != GetSide())
                        {
                            if(!enemie->m_DelSlowly) m_Environment.AddToListSlowly(enemie->GetEnemy());
                        }
                        enemie = enemie->m_NextEnemy;
                    }
                }
                //Если в одной команде и в одном регионе
                else if(obj->AsRobot()->GetTeam() == GetTeam() && obj->AsRobot()->GetRegion() == r)
                {
                    CEnemy* enemie = obj->AsRobot()->m_Environment.m_FirstEnemy;
                    while(enemie)
                    {
                        if(enemie->GetEnemy()->GetSide() != GetSide())
                        {
                            if(!enemie->m_DelSlowly) m_Environment.AddToListSlowly(enemie->GetEnemy());
                        }
                        enemie = enemie->m_NextEnemy;
                    }
                }
            }
            obj = obj->GetNextLogic();
        }

        //    } else {
        //        m_Environment.RemoveAllSlowely();
    }

    /*    CMatrixGroupObject* gobj = my_group->m_FirstObject;

        while(gobj){
            if(gobj->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
                CMatrixRobotAI* bot = (CMatrixRobotAI*)gobj->GetObject();
                if(bot->m_Environment.GetEnemyCnt() > 0){
                    CEnemy* enemies = bot->m_Environment.m_FirstEnemy;
                    while(enemies){
                        if(!m_Environment.SearchEnemy(enemies->GetEnemy()))
                            m_Environment.AddToList(enemies->GetEnemy());
                        enemies = enemies->m_NextEnemy;
                    }
                }
            }
            gobj = gobj->m_NextObject;
        }*/
        //Classify all enemies

    CEnemy* enemies = m_Environment.m_FirstEnemy;
    while(enemies)
    {
        enemies->ClassifyEnemy(this);
        enemies = enemies->m_NextEnemy;
    }
}

SOrder* CMatrixRobotAI::AllocPlaceForOrderOnTop(void)
{
    if(m_OrdersInPool >= MAX_ORDERS) return nullptr;

    //Бесполезная проверка, т.к. ни на рождении, ни на захвате робота трогать нельзя, а на захвате ещё и ручной контроль лочится вместе с выставлением базы
    /*
    if(GetBase() && !IsManualControlLocked())
    {
        // base capturing
        GetBase()->Close();
        SetBase(nullptr);
    }
    */

    if(m_OrdersInPool > 0) MoveMemory(&m_OrdersList[1], &m_OrdersList[0], sizeof(SOrder) * m_OrdersInPool);

    ++m_OrdersInPool;
    return m_OrdersList;
}

/*
void CMatrixRobotAI::AddOrderToEnd(const SOrder& order)
{
    if(m_OrdersInPool > 0) MoveMemory(&m_OrdersList[0], &m_OrdersList[1], sizeof(SOrder) * m_OrdersInPool - 1);

    m_OrdersList[m_OrdersInPool - 1] = order;
    ++m_OrdersInPool;
}
*/

void CMatrixRobotAI::RemoveOrder(int pos)
{
    if(pos < 0 || pos >= m_OrdersInPool) return;
    m_OrdersList[pos].Release();

    --m_OrdersInPool;
    memcpy(m_OrdersList + pos, m_OrdersList + pos + 1, sizeof(SOrder) * (m_OrdersInPool - pos));
}

void CMatrixRobotAI::RemoveOrder(OrderType order)
{
    for(int i = 0; i < m_OrdersInPool;)
    {
        if(m_OrdersList[i].GetOrderType() == order)
        {
            RemoveOrder(i);
            continue;
        }

        ++i;
    }
}

void CMatrixRobotAI::ProcessOrdersList()
{
    if(m_OrdersInPool <= 1) return;

    SOrder tmp_order;
    tmp_order = m_OrdersList[0];

    MoveMemory(&m_OrdersList[0], &m_OrdersList[1], sizeof(SOrder) * m_OrdersInPool - 1);

    m_OrdersList[m_OrdersInPool - 1] = tmp_order;
}

void CMatrixRobotAI::MoveTo(int mx, int my)
{
    RemoveOrder(ROT_MOVE_TO);
    RemoveOrder(ROT_MOVE_TO_BACK);
    RemoveOrder(ROT_STOP_MOVE);

    SOrder* order = AllocPlaceForOrderOnTop();
    if(order == nullptr) return;

    order->SetOrder(ROT_MOVE_TO, mx, my, 0, 0);

    m_ZoneDes = -1;
    m_ZonePathCnt = 0;
    m_ZonePathNext = -1;
    //m_ZoneNear = -1;
    m_MovePathCnt = 0;
    m_MovePathCur = 0;
    m_DesX = mx;
    m_DesY = my;
}

void CMatrixRobotAI::MoveToBack(int mx, int my)
{
    RemoveOrder(ROT_MOVE_TO);
    RemoveOrder(ROT_MOVE_TO_BACK);
    RemoveOrder(ROT_STOP_MOVE);

    SOrder* order = AllocPlaceForOrderOnTop();
    if(order == nullptr) return;
    order->SetOrder(ROT_MOVE_TO_BACK, (float)mx, (float)my, 0, 0);

    m_ZoneDes = -1;
    m_ZonePathCnt = 0;
    m_ZonePathNext = -1;
    //m_ZoneNear = -1;
    m_MovePathCnt = 0;
    m_MovePathCur = 0;
    m_DesX = mx;
    m_DesY = my;
}


void CMatrixRobotAI::MoveReturn(int mx, int my)
{
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_MOVE_RETURN)
        {
            m_OrdersList[cnt].SetOrder(ROT_MOVE_TO, (float)mx, (float)my, 0, 0);
            return;
        }
    }

    SOrder* order = AllocPlaceForOrderOnTop();
    if(order == nullptr) return;
    order->SetOrder(ROT_MOVE_RETURN, (float)mx, (float)my, 0, 0);
}

//Высокоприоритетный приказ на движение
void CMatrixRobotAI::MoveToHigh(int mx, int my)
{
    StopCapture();
    RemoveOrder(ROT_MOVE_RETURN);
    MoveTo(mx, my);
}

void CMatrixRobotAI::StopMoving(void)
{
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO || m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO_BACK)
        {
            m_OrdersList[cnt].SetOrder(ROT_STOP_MOVE, 0, 0, 0, 0);
        }
    }
    m_ZoneDes = -1;
    m_ZonePathCnt = 0;
    m_ZonePathNext = -1;
    //m_ZoneNear = -1;
    m_MovePathCnt = 0;
    m_MovePathCur = 0;

    //LowLevelStop();
}

void CMatrixRobotAI::OrderFire(const D3DXVECTOR3& fire_pos, int type)
{
    RemoveOrder(ROT_FIRE);
    SOrder* order = AllocPlaceForOrderOnTop();
    if(order == nullptr) return;
    order->SetOrder(ROT_FIRE, fire_pos.x, fire_pos.y, fire_pos.z, type);
}

void CMatrixRobotAI::StopFire()
{
    //m_FireTarget = nullptr;
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_FIRE)
        {
            m_OrdersList[cnt].SetOrder(ROT_STOP_FIRE, 0, 0, 0);
        }
    }
}

void CMatrixRobotAI::BigBoom()
{
    //Старый код, который не учитывает число бомб на роботе
    /*
    if(nc < 0)
    {
        for(nc = 0; nc < m_WeaponsCount; ++nc)
        {
            if(m_Weapons[nc].IsEffectPresent() && m_Weapons[nc].GetWeaponNum() == WEAPON_BOMB) break;
        }
        if(nc >= m_WeaponsCount) return;
    }

    if(nc >= 0)
    {
        m_Weapons[nc].Modify(GetGeoCenter(), D3DXVECTOR3(m_PosX, m_PosY, 0), m_Velocity * (1.0f / LOGIC_TACT_PERIOD));
        m_Weapons[nc].FireBegin(m_Velocity * (1.0f / LOGIC_TACT_PERIOD), this);
        m_Weapons[nc].Tact(1);
    }
    */

    int bombs_count = 0;
    for(int i = 0; i < m_WeaponsCount; ++i)
    {
        if(m_Weapons[i].IsEffectPresent() && m_Weapons[i].GetWeaponNum() == WEAPON_BOMB)
        {
            m_Weapons[i].Modify(GetGeoCenter(), D3DXVECTOR3(m_PosX, m_PosY, 0), m_Velocity * (1.0f / LOGIC_TACT_PERIOD));
            m_Weapons[i].FireBegin(m_Velocity * (1.0f / LOGIC_TACT_PERIOD), this);
            m_Weapons[i].Tact(1);
            ++bombs_count;
        }
    }

    MustDie();
}

struct SSeekCaptureMeB
{
    CMatrixBuilding* b;
    CMatrixRobotAI* r;
    float dist2;
};

void CMatrixRobotAI::CaptureBuilding(CMatrixBuilding* building)
{
    RemoveOrder(ROT_CAPTURE_BUILDING);

    SOrder* capture_order = AllocPlaceForOrderOnTop();
    if(capture_order == nullptr) return;
    capture_order->SetOrder(ROT_CAPTURE_BUILDING, building);
}

CMatrixBuilding* CMatrixRobotAI::GetCaptureBuilding(void)
{
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_CAPTURE_BUILDING)
        {
            return (CMatrixBuilding*)m_OrdersList[cnt].GetStatic();
        }
    }
    return nullptr;
}

void CMatrixRobotAI::StopCapture()
{
    SetNoMoreEverMovingOverride(false);
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_CAPTURE_BUILDING)
        {
            m_OrdersList[cnt].Release();
            CMatrixMapStatic* ms = m_OrdersList[cnt].GetStatic();
            m_OrdersList[cnt].SetOrder(ROT_STOP_CAPTURE, ms);
        }
        else if(m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO && m_OrdersList[cnt].GetOrderPhase() == ROP_CAPTURE_MOVING)
        {
            m_OrdersList[cnt].Release();
            m_OrdersList[cnt].SetOrder(ROT_STOP_MOVE, 0, 0, 0, 0);
        }
    }
}

void CMatrixRobotAI::TactCaptureCandidate(int ms)
{
    CMatrixBuilding* bc = nullptr;
    for(int i = 0; i < m_CaptureCandidatesCnt; )
    {
        m_CaptureCandidates[i].tbc -= ms;
        if(m_CaptureCandidates[i].tbc < 0)
        {
            CMatrixBuilding* b = (CMatrixBuilding*)m_CaptureCandidates[i].bcore->m_Object;
            if(b != nullptr && b->m_Side != m_Side)
            {
                if(b->m_Capturer == nullptr)
                {
                    if(bc)
                    {
                        if(bc->m_TurretsHave > b->m_TurretsHave) bc = b;
                        else if(bc->m_TurretsHave == b->m_TurretsHave)
                        {
                            if(b->m_Side == 0) bc = b; // neutral buildings are more prioritized
                        }
                    }
                    else bc = b;
                }
            }
            else
            {
                m_CaptureCandidates[i].bcore->Release();
                m_CaptureCandidates[i] = m_CaptureCandidates[--m_CaptureCandidatesCnt];
                continue;
            }
        }
        ++i;
    }

    if(bc != nullptr)
    {
        CaptureBuilding(bc);
        ClearCaptureCandidates();
    }
}

void CMatrixRobotAI::ClearCaptureCandidates()
{
    for(int i = 0; i < m_CaptureCandidatesCnt; ++i)
    {
        m_CaptureCandidates[i].bcore->Release();
    }

    m_CaptureCandidatesCnt = 0;
}

void CMatrixRobotAI::AddCaptureCandidate(CMatrixBuilding* b)
{
    if(FindOrderLikeThat(ROT_CAPTURE_BUILDING)) return;

    MarkCaptureInformed();

    if(m_CaptureCandidatesCnt >= MAX_CAPTURE_CANDIDATES) return;

    int i;
    for(i = 0; i < m_CaptureCandidatesCnt; ++i)
    {
        if(m_CaptureCandidates[i].bcore->m_Object == b) return;
    }

    m_CaptureCandidates[i].bcore = b->GetCore(DEBUG_CALL_INFO);
    m_CaptureCandidates[i].tbc = g_MatrixMap->Rnd(100, 5000);
    ++m_CaptureCandidatesCnt;
}

void CMatrixRobotAI::RemoveCaptureCandidate(CMatrixBuilding* b)
{
    for(int i = 0; i < m_CaptureCandidatesCnt; ++i)
    {
        if(m_CaptureCandidates[i].bcore->m_Object == b)
        {
            m_CaptureCandidates[i].bcore->Release();
            m_CaptureCandidates[i] = m_CaptureCandidates[--m_CaptureCandidatesCnt];
            return;
        }
    }
}

bool CMatrixRobotAI::FindOrder(OrderType findOrder, CMatrixMapStatic* obj)
{
DTRACE();

    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == findOrder && m_OrdersList[cnt].GetStatic() == obj) return true;
    }
    return false;
}

void CMatrixRobotAI::BreakAllOrders(void)
{
    LowLevelStop();
    m_Environment.Reset();
    LowLevelStopFire();

    for(int cnt = m_OrdersInPool - 1; cnt >= 0; --cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_CAPTURE_BUILDING)
        {
            CMatrixBuilding* building = (CMatrixBuilding*)m_OrdersList[cnt].GetStatic();
            if(building && building->IsCaptured() && building->m_Capturer == this)
            {
                building->ResetCaptured();
                if(IsManualControlLocked())
                {
                    if(building->IsBase() && building->m_State != BASE_CLOSED) building->Close();
                }
                /*
                else
                {
                    //Тут какой-то пиздец, но этот код ломает игру, хотя и должен бы выглядеть именно так

                    //Если робот уже приступил к непосредственному захвату базы
                    //if(GetOrder(0)->GetStatic()->AsBuilding()->IsBase() && GetOrder(0)->GetOrderPhase() >= ROP_CAPTURE_SETTING_UP)
                    //{
                    //    MustDie();
                    //    return;
                    //}

                    //Если робот уже приступил к захвату базы, то этот код идёт нахуй
                    if(GetOrder(0)->GetOrderPhase() < ROP_CAPTURE_SETTING_UP || !GetOrder(0)->GetStatic()->AsBuilding()->IsBase())
                    {
                        MustDie();
                    }
                    else return;
                }
                */
            }
        }
    }

    while(m_OrdersInPool > 0) RemoveOrder(m_OrdersInPool - 1);
}

int CMatrixRobotAI::FindOrderLikeThat(OrderType order) const
{
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == order) return cnt + 1;
    }
    return 0;
}

int CMatrixRobotAI::FindOrderLikeThat(OrderType order, OrderPhase phase)
{
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == order && m_OrdersList[cnt].GetOrderPhase() == phase) return cnt + 1;
    }
    return 0;
}

void CMatrixRobotAI::UpdateOrder_MoveTo(int mx, int my)
{
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO)
        {
            //g_MatrixMap->PlaceFindNear(m_Module[0].m_Kind - 1, 4, mx, my);
            m_OrdersList[cnt].Release();
            m_OrdersList[cnt].SetOrder(ROT_MOVE_TO, (float)mx, (float)my, 0);
            return;
        }
    }
}

bool CMatrixRobotAI::GetMoveToCoords(CPoint& pos)
{
    float f1, f2;
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO)
        {
            m_OrdersList[cnt].GetParams(&f1, &f2, nullptr);
            pos.x = Float2Int(f1);
            pos.y = Float2Int(f2);
            return true;
        }
    }
    return false;
}

bool CMatrixRobotAI::GetReturnCoords(CPoint& pos)
{
    float f1, f2;
    for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
    {
        if(m_OrdersList[cnt].GetOrderType() == ROT_MOVE_RETURN)
        {
            m_OrdersList[cnt].GetParams(&f1, &f2, nullptr);
            pos.x = Float2Int(f1);
            pos.y = Float2Int(f2);
            return true;
        }
    }
    return false;
}

void CMatrixRobotAI::LowLevelStopFire(void)
{
    for(int nC = 0; nC < m_WeaponsCount; ++nC)
    {
        if(m_Weapons[nC].IsEffectPresent())
        {
            m_Weapons[nC].FireEnd();
        }
    }
}

void CMatrixRobotAI::LowLevelStop(void)
{
    m_Speed = 0;
    m_Velocity = D3DXVECTOR3(0, 0, 0);
}

bool CMatrixRobotAI::FindWeapon(int type)
{
    for(int nC = 0; nC < m_WeaponsCount; ++nC)
    {
        if(m_Weapons[nC].IsEffectPresent() && m_Weapons[nC].GetWeaponNum() == type) return true;
    }
    return false;
}
bool CMatrixRobotAI::FindRepairWeapon()
{
    for(int nC = 0; nC < m_WeaponsCount; ++nC)
    {
        if(m_Weapons[nC].IsEffectPresent() && g_Config.m_WeaponsConsts[m_Weapons[nC].GetWeaponNum()].is_repairer) return true;
    }
    return false;
}
bool CMatrixRobotAI::FindBombWeapon()
{
    for(int nC = 0; nC < m_WeaponsCount; ++nC)
    {
        if(m_Weapons[nC].IsEffectPresent() && g_Config.m_WeaponsConsts[m_Weapons[nC].GetWeaponNum()].is_bomb) return true;
    }
    return false;
}

void CMatrixRobotAI::ReleaseMe(void)
{
    UnSelect();
    BreakAllOrders();
    ClearCaptureCandidates();

    for(int nC = 0; nC < m_WeaponsCount; ++nC)
    {
        if(m_Weapons[nC].IsEffectPresent())
        {
            m_Weapons[nC].FireEnd();
            m_Weapons[nC].Release();
        }
    }
    m_WeaponsCount = 0;

    if(GetSide() == PLAYER_SIDE)
    {
        CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();

        ps->RemoveFromSelection(this);

        int pos = 0;

        //Если данным роботом управлял игрок, то обновляем интерфейс, выводим его из аркадного режима
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
                if(!ps->GetCurGroup()->GetObjectsCnt())
                {
                    ps->PLDropAllActions();
                }
            }

        }

        if(ps->GetCurSelNum() == pos) pos = 0;
        else pos = -1;

        DeleteProgressBarClone(PBC_CLONE1);
        DeleteProgressBarClone(PBC_CLONE2);
        if(g_IFaceList)
        {
            g_IFaceList->DeleteProgressBars(this);
            if(ps->m_CurrSel == GROUP_SELECTED)
            {
                g_IFaceList->CreateGroupIcons();
            }
            ps->SetCurSelNum(pos);
        }
    }

    CMatrixMapStatic* objects = CMatrixMapStatic::GetFirstLogic();
    while(objects)
    {
        if(objects->IsRobotAlive() && objects->AsRobot() != this)
        {
            objects->AsRobot()->m_Environment.RemoveFromList(this);
        }
        objects = objects->GetNextLogic();
    }

    m_Environment.Clear();

    if(m_CurrState == ROBOT_CARRYING)
    {
        Carry(nullptr);
    }

    if(m_BigTexture)
    {
        g_Cache->Destroy(m_BigTexture);
        m_BigTexture = nullptr;
    }
    if(m_MedTexture)
    {
        g_Cache->Destroy(m_MedTexture);
        m_MedTexture = nullptr;
    }
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    if(m_SmallTexture)
    {
        g_Cache->Destroy(m_SmallTexture);
        m_SmallTexture = nullptr;
    }
#endif

    if(m_ZonePath) { HFree(m_ZonePath, g_MatrixHeap); m_ZonePath = nullptr; }

    if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject())
    {
        g_MatrixMap->GetPlayerSide()->SetArcadedObject(nullptr);
    }
}


void CMatrixRobotAI::GetLost(const D3DXVECTOR3& v)
{
    if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()) return;

    D3DXVECTOR3 f1, f2;

    D3DXVec3Normalize(&f1, &m_ChassisForward);
    D3DXVec3Normalize(&f2, &v);

    float cos1 = f1.x * f2.x + f1.y * f2.y;
    float angle = (float)acos(cos1);

    if(angle > GRAD2RAD(70))
    {
        RotateRobotChassis((D3DXVECTOR3(m_PosX, m_PosY, 0) + v));
        return;
    }

    D3DXVECTOR3 vLeft(0, 0, 0), vRight(0, 0, 0);
    float lost_len = 0, lost_param = 0, v_param = 0;

    vLeft.x = v.y;
    vLeft.y = -v.x;

    vRight = -vLeft;

    v_param = (float)g_MatrixMap->RndFloat(0, 1);
    lost_param = (float)g_MatrixMap->RndFloat(0, 1);

    D3DXVECTOR3 vLost = LERPVECTOR(v_param, vLeft, vRight);
    lost_len = LERPFLOAT(lost_param, GET_LOST_MIN, GET_LOST_MAX);

    vLost *= lost_len;
    vLost += v;

    vLost += D3DXVECTOR3(m_PosX, m_PosY, 0);
    int mx = Float2Int(vLost.x / GLOBAL_SCALE_MOVE);
    int my = Float2Int(vLost.y / GLOBAL_SCALE_MOVE);

    g_MatrixMap->PlaceFindNear(m_Module[0].m_Kind - 1, 4, mx, my, this);
    if(!FindOrderLikeThat(ROT_MOVE_TO, ROP_GETING_LOST))
    {
        MoveTo(mx, my);
        for(int cnt = 0; cnt < m_OrdersInPool; ++cnt)
        {
            if(m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO)
            {
                m_OrdersList[cnt].SetPhase(ROP_GETING_LOST);
                break;
            }
        }
    }
}

bool CMatrixRobotAI::SelectByGroup()
{
    //UnSelect();
    RESETFLAG(m_ObjectFlags, ROBOT_FLAG_SARCADE);

    if((!FLAG(m_ObjectFlags, ROBOT_FLAG_SGROUP) && !m_Selection) && CreateSelection())
    {
        SETFLAG(m_ObjectFlags, ROBOT_FLAG_SGROUP);
        return true;
    }
    else if(!m_Selection)
    {
        RESETFLAG(m_ObjectFlags, ROBOT_FLAG_SGROUP);
        return false;
    }
    return true;
}

bool CMatrixRobotAI::SelectArcade()
{
    //UnSelect();
    RESETFLAG(m_ObjectFlags, ROBOT_FLAG_SGROUP);

    if((!FLAG(m_ObjectFlags, ROBOT_FLAG_SARCADE) && !m_Selection) && CreateSelection())
    {
        SETFLAG(m_ObjectFlags, ROBOT_FLAG_SARCADE);
        return true;
    }
    else if(!m_Selection)
    {
        RESETFLAG(m_ObjectFlags, ROBOT_FLAG_SARCADE);
        return false;
    }
    return true;
}

void CMatrixRobotAI::UnSelect()
{
    KillSelection();

    RESETFLAG(m_ObjectFlags, ROBOT_FLAG_SGROUP);
    RESETFLAG(m_ObjectFlags, ROBOT_FLAG_SARCADE);
}

bool CMatrixRobotAI::CreateSelection()
{
    m_Selection = (CMatrixEffectSelection*)CMatrixEffect::CreateSelection(D3DXVECTOR3(m_PosX, m_PosY, m_Core->m_Matrix._43 + 3/*ROBOT_SELECTION_HEIGHT*/), ROBOT_SELECTION_SIZE);
    if(!g_MatrixMap->AddEffect(m_Selection))
    {
        m_Selection = nullptr;
        return false;
    }
    return true;
}

void CMatrixRobotAI::KillSelection()
{
    if(m_Selection)
    {
        m_Selection->Kill();
        m_Selection = nullptr;
    }
}

void CMatrixRobotAI::MoveSelection()
{
    if(m_Selection) m_Selection->SetPos(D3DXVECTOR3(m_PosX, m_PosY, m_Core->m_Matrix._43 + 3/*ROBOT_SELECTION_HEIGHT*/));
}

bool CMatrixRobotAI::IsSelected()
{
    if(FLAG(m_ObjectFlags, ROBOT_FLAG_SGROUP) || FLAG(m_ObjectFlags, ROBOT_FLAG_SARCADE)) return true;
    return false;
}

//Рассчитываем оценочную суммарную крутость робота (используется в расчётах ИИ)
void CMatrixRobotAI::CalcStrength()
{
    m_Strength = 0;
    for(int i = 0; i < m_WeaponsCount; ++i) m_Strength += g_Config.m_RobotWeaponsConsts[m_Weapons[i].m_Module->m_Kind].strength;
    m_Strength = m_Strength * (0.4f + 0.6f * (m_HitPoint / m_HitPointMax));
}

void CMatrixRobotAI::CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type)
{
    m_ProgressBar.CreateClone(clone_type, x, y, width);
}

void CMatrixRobotAI::DeleteProgressBarClone(EPBCoord clone_type)
{
    m_ProgressBar.KillClone(clone_type);
}

void CMatrixRobotAI::CreateTextures()
{
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    SRenderTexture rt[3];
    rt[0].ts = TEXSIZE_256;
    rt[1].ts = TEXSIZE_64;
    rt[2].ts = TEXSIZE_32;

    if(RenderToTexture(rt, 3))
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
#else
    SRenderTexture rt[2];
    rt[0].ts = TEXSIZE_256;
    rt[1].ts = TEXSIZE_64;

    if(RenderToTexture(rt, 2))
    {
        m_BigTexture = rt[0].tex;
        m_MedTexture = rt[1].tex;
    }
    else
    {
        m_BigTexture = nullptr;
        m_MedTexture = nullptr;
    }
#endif
}

bool CMatrixRobotAI::IsHomingMissilesEquipped()
{
    for(int i = 0; i < m_WeaponsCount; ++i)
    {
        if(m_Weapons[i].IsEffectPresent())
        {
            int weap_num = m_Weapons[i].GetWeaponNum();
            if(
                g_Config.m_WeaponsConsts[weap_num].primary_effect == EFFECT_ROCKET_LAUNCHER &&
                g_Config.m_WeaponsConsts[weap_num].projectile_homing_speed + m_MissileHomingSpeed > 0.0f
              ) return true;
        }
    }
    return false;
}

void CMatrixRobotAI::PlayHullSound()
{
    CSound::Play(g_Config.m_RobotHullsConsts[m_Module[1].m_Kind].hull_sound_num, GetGeoCenter(), SL_HULL);
}

void CMatrixRobotAI::SetWeaponToArcadedCoeff()
{
    for(int i = 0; i < m_WeaponsCount; ++i) m_Weapons[i].SetArcadeCoefficient();
}

void CMatrixRobotAI::SetWeaponToDefaultCoeff()
{
    for(int i = 0; i < m_WeaponsCount; ++i) m_Weapons[i].SetDefaultCoefficient();
}

bool CMatrixRobotAI::CheckFireDist(const D3DXVECTOR3& point)
{
    //CHelper::DestroyByGroup(0);
    for(int i = 0; i < m_WeaponsCount; ++i)
    {
        if(m_Weapons[i].IsEffectPresent())
        {
            D3DXVECTOR3 wpos(0, 0, 0);
            if(m_Weapons[i].GetWeaponPos(wpos))
            {
                D3DXVECTOR3 hitpos(0, 0, 0);
                D3DXVECTOR3 out(0, 0, 0);
                D3DXVec3Normalize(&out, &(point - wpos));
                out *= (m_Weapons[i].GetWeaponDist());
                //CHelper::Create(1, 0)->Line(wpos, wpos + out);
                CMatrixMapStatic* hito = g_MatrixMap->Trace(&hitpos, wpos, wpos + out, TRACE_ALL, this);
                if(hito == g_MatrixMap->m_TraceStopObj)
                {
                    //if(POW2(m_Weapons[i].GetWeaponDist()) > D3DXVec3LengthSq(&(hitpos - wpos)))
                    return true;
                }
            }
        }
    }
    return false;
}