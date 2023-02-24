// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../stdafx.h"
#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include "../MatrixRenderPipeline.hpp"
#include <math.h>
#include "../MatrixFlyer.hpp"

void UnloadObject(CVectorObjectAnim* o, CHeap* heap);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMatrixEffectMovingObject::CMatrixEffectMovingObject(const SMOProps& props, dword hitmask, CMatrixMapStatic* skip, FIRE_END_HANDLER handler, dword user) :
CMatrixEffect(), m_Props(props)
{
DTRACE();
    
    m_EffectType = EFFECT_MOVING_OBJECT;

    m_Props.time = 0;
    m_Props.endoflife = false;
    D3DXVECTOR3 temp = m_Props.target - m_Props.startpos;
    m_Props.distance = D3DXVec3Length(&temp);
    //m_Props.curpos = m_Props.startpos;
    m_Props.endhandler = handler;
    m_Props.uservalue = user;
    m_Props.hitmask = hitmask;
    m_Props.skip = skip;

    if(props.handler == MO_Grenade_Tact)
    {
        m_Props.bomb.pos = props.bomb.pos;
        m_Props.bomb.trajectory = props.bomb.trajectory;
    }
    else if(props.handler == MO_Cannon_Round_Tact)
    {
        m_Props.gun.maxdist = props.gun.maxdist;
    }

    if(m_Props.handler) m_Props.handler(m_Mat, m_Props, 0);
}

CMatrixEffectMovingObject::~CMatrixEffectMovingObject()
{
    if(m_Props.object) UnloadObject(m_Props.object, m_Heap);
    if(m_Props.endhandler)
    {
        D3DXVECTOR3 temp = m_Props.curpos - m_Props.startpos;
        m_Dist2 = D3DXVec3LengthSq(&temp);
        m_Props.endhandler(TRACE_STOP_NONE, m_Props.curpos, (dword)m_Props.uservalue, FEHF_LASTHIT);
    }

    //if(m_Props.attacker) m_Props.attacker->Release();

    if(m_Props.handler == MO_Homing_Missile_Tact)
    {
        if(m_Props.hm.target != nullptr) m_Props.hm.target->Release();
    }
    else if(m_Props.handler == MO_Grenade_Tact)
    {
        if(m_Props.bomb.trajectory != nullptr) HDelete(CTrajectory, m_Props.bomb.trajectory, g_MatrixHeap);
    }

    if(m_Props.shleif)
    {
        if(m_Props.shleif->effect) m_Props.shleif->Unconnect();
        HFree(m_Props.shleif, m_Heap);
    }
}


void CMatrixEffectMovingObject::BeforeDraw(void)
{
#ifdef _DEBUG
    if(m_Props.object == nullptr) _asm int 3
#endif
    m_Props.object->BeforeDraw();
}
void CMatrixEffectMovingObject::Draw(void)
{
DTRACE();

#ifdef _DEBUG
    if(m_Props.object == nullptr) _asm int 3
#endif

    CVectorObject::DrawBegin();
    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,0xFFFFFFFF);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_AmbientColorObj));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD,&m_Mat));

    m_Props.object->Draw(0);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    CVectorObject::DrawEnd();
}
void CMatrixEffectMovingObject::Tact(float step)
{
DTRACE();

    m_Props.object->Tact(Float2Int(step));
    m_Props.time += step;
    m_Props.handler(m_Mat, m_Props, step);
    if(m_Props.endoflife)
    {
        UnloadObject(m_Props.object, m_Heap);
        m_Props.object = nullptr;
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
    }
}
void CMatrixEffectMovingObject::Release(void)
{
DTRACE();

    SetDIP();
    HDelete(CMatrixEffectMovingObject, this, m_Heap);
}

struct HMData
{
    D3DXVECTOR3 dir = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 to_tgt = { 0.0f, 0.0f, 0.0f };
    //CMatrixMapStatic* target = nullptr;
    float maxcos = 0.0f;
    SMOProps* props = nullptr;
    bool      found = false;
};

//Используется для проверки валидности цели ракеты, выбранной в сфере вокруг неё из функции подбора указанного типа объекта FindObject
static bool HMEnum(const D3DXVECTOR3& fpos, CMatrixMapStatic* ms, dword user)
{
    D3DXVECTOR3 to_dir;
    const D3DXVECTOR3* p;
    bool select_new;
    HMData* hmd = (HMData*)user;

    //int side = 0;
    //if(hmd->props->attacker && hmd->props->attacker->m_Object) side = hmd->props->attacker->m_Object->GetSide();

    if(hmd->props->side == ms->GetSide()) return true; //No need friendly object
    if(hmd->props->hm.target != nullptr && ((hmd->props->hm.target->m_Type == OBJECT_TYPE_FLYER) || (hmd->props->hm.target->m_Type == OBJECT_TYPE_ROBOTAI) || (hmd->props->hm.target->m_Type == OBJECT_TYPE_CANNON)) && (ms->GetObjectType() == OBJECT_TYPE_BUILDING))
    {
        return true; //No need building if robot located
    }

    select_new = (hmd->props->hm.target != nullptr ) && (hmd->props->hm.target->m_Type == OBJECT_TYPE_BUILDING) && 
                ((ms->GetObjectType() == OBJECT_TYPE_FLYER) || (ms->GetObjectType() == OBJECT_TYPE_ROBOTAI) || (ms->GetObjectType() == OBJECT_TYPE_CANNON));

    p = &ms->GetGeoCenter();
    D3DXVECTOR3 temp = *p - hmd->props->curpos;
    float cc = D3DXVec3Dot(&hmd->dir, D3DXVec3Normalize(&to_dir, &temp));

    SObjectCore* oc = ms->GetCore(DEBUG_CALL_INFO);

    if((cc > hmd->props->hm.target_capture_angle_cos) && ((cc > hmd->maxcos) || select_new || (oc == hmd->props->hm.target)))
    {
        CMatrixMapStatic* t = g_MatrixMap->Trace(nullptr, hmd->props->curpos, *p + (*p - hmd->props->curpos) * 0.1f, TRACE_ALL, hmd->props->skip);
        if((t == TRACE_STOP_NONE) || (t == ms))
        {
            if(hmd->props->hm.target != nullptr) hmd->props->hm.target->Release();
            hmd->props->hm.target = oc;
            oc->RefInc();

            hmd->to_tgt = to_dir;
            hmd->maxcos = cc;
            hmd->found = true;
        }
    }

    oc->Release();
    return true;
}

static bool MOEnum(const D3DXVECTOR3& center, CMatrixMapStatic* ms, dword user)
{
    SMOProps* props = (SMOProps*)user;
    if(props->endhandler)
    {
        D3DXVECTOR3 temp = center - props->startpos;
        CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&temp);
        props->endhandler(ms, center, (dword)props->uservalue, 0);
    }
    return true;
}


void MO_Cannon_Round_Tact(D3DXMATRIX& m, SMOProps& props, float tact)
{
DTRACE();

    float dtime = 0.1f * float(tact);

    float vel = D3DXVec3Length(&props.velocity);
    D3DXVECTOR3 dir(props.velocity * (1.0f / vel));

    bool hit = false;

    D3DXVECTOR3 newpos = props.curpos + props.velocity * dtime;

    props.gun.dist += vel * dtime;

    if((newpos.z - g_MatrixMap->GetZ(newpos.x, newpos.y)) < CANNON_ROUND_IMPACT_RADIUS)
    {
        hit = g_MatrixMap->FindObjects(newpos, props.gun.splash_radius, 1, props.hitmask, props.skip, MOEnum, (dword)&props); //props нихуя не CMatrixMapStatic, но за это говно скажем спасибо дабу
    }

    D3DXVECTOR3 hitpos = newpos;
    CMatrixMapStatic* hito = g_MatrixMap->Trace(&hitpos, props.curpos, newpos, props.hitmask, props.skip);
    if(hito != TRACE_STOP_NONE) hit = true;

    if(g_MatrixMap->m_Camera.IsInFrustum(newpos)) props.gun.in_frustum_count = 100;
    else --props.gun.in_frustum_count;

    if(props.gun.in_frustum_count > 0 && props.gun.contrail_sprite_num != SPR_NONE)
    {
        CMatrixEffect::CreateSpritesLine(nullptr, props.curpos, hitpos, props.gun.contrail_width, props.gun.contrail_color, 0, props.gun.contrail_duration, CMatrixEffect::GetSingleBrightSpriteTex(props.gun.contrail_sprite_num));
        //DM("cc", CStr(props.curpos.x) + "," + CStr(props.curpos.y) + "," + CStr(props.curpos.z) + "," + CStr(hitpos.x) + "," + CStr(hitpos.x) + "," + CStr(hitpos.x));
    }

    props.curpos = hitpos;

    VecToMatrixY(m, props.curpos, dir);

    if(hit)
    {
        //if(hito == TRACE_STOP_NONE && target != nullptr) hito = target;
        if(hito != TRACE_STOP_WATER)
        {
            bool fire = false;
            if(hito == TRACE_STOP_LANDSCAPE)
            {
                CMatrixEffect::CreateLandscapeSpot(nullptr, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(3) + 6, SPOT_VORONKA);
                hitpos.z = g_MatrixMap->GetZ(hitpos.x, hitpos.y) + 10;
                fire = true;
            }
            CMatrixEffect::CreateExplosion(hitpos, ExplosionMissile, fire);
        }

        //impact
        props.endoflife = true;
        if(props.endhandler)
        {
            D3DXVECTOR3 temp = hitpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&temp);
            props.endhandler(hito, hitpos, (dword)props.uservalue, FEHF_LASTHIT);
            props.endhandler = nullptr;
        }
    }
    else if(props.gun.dist > props.gun.maxdist)
    {
        props.endoflife = true;
        if(props.endhandler)
        {
            D3DXVECTOR3 temp = hitpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&temp);
            props.endhandler(TRACE_STOP_NONE, hitpos, (dword)props.uservalue, FEHF_LASTHIT);
            props.endhandler = nullptr;
        }
    }
}

void MO_Homing_Missile_Tact(D3DXMATRIX& m, SMOProps& props, float tact)
{
    float dtime = 0.1f * float(tact);

    //calc target
    HMData data;

    //Если ракета сейчас идёт на начальной скорости, ловим момент и переводим её на полную
    if(!props.hm.is_full_velocity)
    {
        if(props.time >= props.hm.full_velocity_reach)
        {
            props.velocity = props.hm.full_velocity;
            props.hm.is_full_velocity = true;
        }
    }
    //Непосредственно в момент вылета ракеты не должны срабатывать об робота под прямым управлением игрока, но вот через некоторое время после уже вполне
    else if(props.time > 1250) props.skip = nullptr; //Снимаем "защиту" через 1.25 секунды после запуска

    D3DXVec3Normalize(&data.dir, &props.velocity);

    if(props.hm.homing_speed > 0.0f) //Если у ракеты включено самонаведение (при нулевом или минусовом значении оно отключено)
    {
        //props.hm.target - текущий объект-цель, в который должна попасть ракета
        //props.target - координаты цели, в которые должна попасть ракета (при наличии объекта в props.hm.target в props.target будет записан его координатный центр)
        //data.to_tgt - необходимый вектор отклонения от текущих координат ракеты до выхода на координаты цели props.target

        D3DXVECTOR3 seek_center;
        D3DXVECTOR3 temp;

        //Для наведения ракеты по курсору из ручного управления роботом
        if(g_Config.IsManualMissileControl())
        {
            //Если эту ракету выпустил робот, в данный момент находящийся под управлением игрока
            if(g_MatrixMap->GetPlayerSide()->IsArcadeMode() && g_MatrixMap->GetPlayerSide()->GetArcadedObject() == props.hm.missile_owner)
            {
                props.target = g_MatrixMap->m_TraceStopPos;
                D3DXVECTOR3 temp = props.target - props.curpos;
                D3DXVec3Normalize(&data.to_tgt, &temp);
                data.props = &props;
                data.found = false; //На всякий случай, чтобы наверняка сбросить ранее выбранную цель
                props.hm.target = nullptr;
                goto skip_obj_seek;
            }
        }

        //Этот параметр используется в проверке валидности выбора цели HMEnum
        if(props.hm.target)
        {
            D3DXVECTOR3 temp = props.target - props.curpos;
            data.maxcos = D3DXVec3Dot(&data.dir, D3DXVec3Normalize(&seek_center, &temp));
            //seekcenter = props.target;
        }
        else data.maxcos = -1;

        seek_center = (props.curpos + data.dir * HOMING_RADIUS);
        //D3DXVECTOR2 vmin(seek_center.x - HOMING_RADIUS, seek_center.y - HOMING_RADIUS), vmax(seek_center.x + HOMING_RADIUS, seek_center.y + HOMING_RADIUS);

        if(props.time > props.hm.next_seek_time) props.hm.next_seek_time = props.time + HM_SEEK_TIME_PERIOD;
        else goto skip_obj_seek;

        data.props = &props;
        data.found = false;

        g_MatrixMap->FindObjects(seek_center, HOMING_RADIUS, 1, TRACE_ROBOT | TRACE_FLYER | TRACE_CANNON | TRACE_BUILDING, props.skip, HMEnum, (dword)&data); //props нихуя не CMatrixMapStatic, но за это говно скажем спасибо дабу

        if(data.found == false)
        {
            if(props.hm.target) props.hm.target->Release();
            props.hm.target = nullptr;
        }
        else if(props.hm.target != nullptr) props.target = props.hm.target->m_GeoCenter;

        //В комменте просто ручной вариант нормализации вектора
        //data.to_tgt = props.target - props.curpos;
        //data.to_tgt /= D3DXVec3Length(&data.to_tgt) + 0.000001;
        temp = props.target - props.curpos;
        D3DXVec3Normalize(&data.to_tgt, &temp);

    skip_obj_seek:
        //Меняем направление движения ракеты, если включено самонаведение и уже выбрана цель
        if(props.hm.is_full_velocity)
        {
            float old_vel = D3DXVec3Length(&props.velocity);
            props.velocity += data.to_tgt * props.hm.homing_speed * dtime;
            props.velocity *= old_vel / D3DXVec3Length(&props.velocity);
        }
    }

    //Считаем постоянное ускорение ракеты (если равно 0, то ускорения не будет)
    if(props.hm.is_full_velocity) props.velocity += data.dir * props.hm.acceleration_coef * dtime;

    float k = float(1.0 / D3DXVec3Length(&props.velocity));
    D3DXVECTOR3 newpos = props.curpos + props.velocity * dtime;

    if(g_MatrixMap->m_Camera.IsInFrustum(newpos))
    {
        if(props.hm.in_frustum_count <= 0) props.hm.next_fire_time = props.time;
        props.hm.in_frustum_count = 10;
    }
    else --props.hm.in_frustum_count;

    if(props.hm.in_frustum_count > 0) 
    {
        float t, dt;
        if(props.time > props.hm.next_fire_time)
        {
            t = 0;
            dt = MISSILE_FIRE_PERIOD / float(props.time - props.hm.next_fire_time);
            while(props.time > props.hm.next_fire_time)
            {
                props.hm.next_fire_time += MISSILE_FIRE_PERIOD;

                D3DXVECTOR3 p = (newpos - props.curpos) * t + props.curpos;
                t += dt;

                if(props.shleif->effect)
                {
                    if(props.time < props.hm.fire_effect_starts) ((CMatrixEffectShleif*)props.shleif->effect)->AddSmoke(p, 300, 300, 400, 0xFFFFFFFF, true, 0);
                    else
                    {
                        ((CMatrixEffectShleif*)props.shleif->effect)->AddSmoke(p, 300, 600, 800, 0xFFFFFFFF, true, 0);
                        ((CMatrixEffectShleif*)props.shleif->effect)->AddFire(p, 300, 300, 800, 1, true, 0, props.hm.close_color_rgb, props.hm.far_color_rgb);
                    }
                }
            }
        }
    }
    
    D3DXVECTOR3 hit_pos = newpos;
    bool hit = false;
    CMatrixMapStatic* hit_obj = g_MatrixMap->Trace(&hit_pos, props.curpos, newpos, props.hitmask, props.skip);
    if(hit_obj != TRACE_STOP_NONE) hit = true;

    props.curpos = hit_pos; //Обновляем текущие координаты ракеты на место попадания, либо просто на новую позицию (если попадания не было), рассчитанную из вектора и скорости движения

    VecToMatrixY(m, props.curpos, props.velocity * k);

    D3DXVECTOR3 temp = props.curpos - props.target;
    float distance = D3DXVec3LengthSq(&temp);
    if(hit || distance < (MISSILE_IMPACT_RADIUS * MISSILE_IMPACT_RADIUS)) //Сравниваем площади
    {
        if(hit_obj == TRACE_STOP_NONE && props.hm.target != nullptr && props.hm.target->m_Object != nullptr)
        {
            hit_obj = props.hm.target->m_Object;
        }

        // seek objects
        hit |= g_MatrixMap->FindObjects(props.curpos, props.hm.splash_radius, 1, props.hitmask, props.skip, MOEnum, (dword)&props); //props нихуя не CMatrixMapStatic, но за это говно скажем спасибо дабу

        if(!hit)
        {
            hit = true;
            //Доп. проверка на случай, если ракета таки улетела далеко в небо (что крайне маловероятно), а не в землю
            if(props.curpos.z - g_MatrixMap->GetZ(props.curpos.x, props.curpos.y) < MISSILE_IMPACT_RADIUS) hit_obj = TRACE_STOP_LANDSCAPE;
        }

        bool fire = false;
        if(hit_obj == TRACE_STOP_LANDSCAPE)
        {
            CMatrixEffect::CreateLandscapeSpot(nullptr, D3DXVECTOR2(hit_pos.x, hit_pos.y), FSRND(M_PI), FRND(3) + 6, SPOT_VORONKA);
            hit_pos.z = g_MatrixMap->GetZ(hit_pos.x, hit_pos.y) + 10;
            fire = true;
        }
        CMatrixEffect::CreateExplosion(hit_pos, ExplosionMissile, fire);

        // impact
        props.endoflife = true;
        if(props.endhandler)
        {
            D3DXVECTOR3 temp = props.curpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&temp);
            props.endhandler(TRACE_STOP_NONE, props.curpos, (dword)props.uservalue, FEHF_LASTHIT);
            props.endhandler = nullptr;
        }

        if(props.hm.target)
        {
            props.hm.target->Release();
            props.hm.target = nullptr;
        }
    }
    //Время жизни ракеты закончилось, либо она оказалась "глубоко под водой" и до сих пор жива - самоуничтожаем её
    else if(props.time > props.hm.max_lifetime || props.curpos.z < WATER_LEVEL - 50)
    {
        props.endoflife = true;

        if(props.endhandler)
        {
            D3DXVECTOR3 temp = props.curpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&temp);
            props.endhandler(hit_obj, hit ? hit_pos : props.curpos, (dword)props.uservalue, FEHF_LASTHIT);
            props.endhandler = nullptr;
            CMatrixEffect::CreateExplosion(props.curpos, ExplosionMissile, false);
        }

        if(props.hm.target)
        {
            props.hm.target->Release();
            props.hm.target = nullptr;
        }
    }
}

void MO_Grenade_Tact(D3DXMATRIX& m, SMOProps& props, float tact)
{
    float dtime = props.velocity.x * float(tact);

    //D3DXVECTOR3 to_tgt(props.target - props.curpos);
    //props.velocity.z -= 0.035f * dtime;
    //float vel = D3DXVec3Length(&props.velocity);

    bool hit = false;

    //D3DXVECTOR3 newpos = props.curpos + props.velocity * dtime;

    props.bomb.pos += dtime;

    D3DXVECTOR3 newpos, dir;
    props.bomb.trajectory->CalcPoint(newpos, props.bomb.pos);

    D3DXVECTOR3 temp = newpos - props.curpos;
    D3DXVec3Normalize(&dir, &temp);

    float t, dt;
    if(props.time > props.bomb.next_fire_time)
    {
        t = 0;
        dt = BOMB_FIRE_PERIOD / float(props.time-props.bomb.next_fire_time);
        while(props.time > props.bomb.next_fire_time)
        {
            props.bomb.next_fire_time += BOMB_FIRE_PERIOD;

            D3DXVECTOR3 p = (newpos - props.curpos) * t + props.curpos;
            t += dt;

            if(props.time < 200)
            {
                if(props.shleif->effect) ((CMatrixEffectShleif*)props.shleif->effect)->AddSmoke(p, 300, 300, 300, 0xFFFFFFFF, true, 0);
            }
            else
            {
                //CMatrixEffect::CreateFire(p, 300, 300, 300, 1, true, 0);
                if(props.shleif->effect) ((CMatrixEffectShleif*)props.shleif->effect)->AddSmoke(p, 300, 600, 400, 0xFFFFFFFF, true, 0);
            }
        }
    }

    //CMatrixMapStatic* target = nullptr;

    if((newpos.z - g_MatrixMap->GetZ(newpos.x, newpos.y)) < (BOMB_DAMAGE_RADIUS + 10))
    {
        hit = g_MatrixMap->FindObjects(newpos, BOMB_DAMAGE_RADIUS, 1, props.hitmask, props.skip, MOEnum, (dword)&props); //props нихуя не CMatrixMapStatic, но за это говно скажем спасибо дабу
    }

    D3DXVECTOR3 hitpos = newpos;
    //DDVECT("bla0", hitpos);
    CMatrixMapStatic* hito = g_MatrixMap->Trace(&hitpos, props.curpos, newpos, props.hitmask, props.skip);
    //DDVECT("bla1", hitpos);

    if(hito != TRACE_STOP_NONE) hit = true;

    // check if bomb is on last trajectory point, then explode if true
    D3DXVECTOR3 last;
    props.bomb.trajectory->CalcPoint(last, 1.0);

    if(IsVec3Equal(last, newpos, 0.001f)) hit = true;

    props.curpos = newpos;

    VecToMatrixY(m, props.curpos, dir);

    if(hit) 
    {
        //if(hito == TRACE_STOP_NONE && target != nullptr) hito = target;

        if(hito != TRACE_STOP_WATER)
        {
            bool fire = false;
            if(hito == TRACE_STOP_LANDSCAPE)
            {
                CMatrixEffect::CreateLandscapeSpot(nullptr, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(3) + 6, SPOT_VORONKA);
                hitpos.z = g_MatrixMap->GetZ(hitpos.x, hitpos.y) + 10;
                fire = true;
            }
            CMatrixEffect::CreateExplosion(hitpos, ExplosionMissile, fire);
        }
        else
        {
            float z = g_MatrixMap->GetZ(hitpos.x, hitpos.y);
            if(z > WATER_LEVEL)
            {
                CMatrixEffect::CreateLandscapeSpot(nullptr, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(3) + 6, SPOT_VORONKA);
                hitpos.z = z + 10;
                CMatrixEffect::CreateExplosion(hitpos, ExplosionMissile, true);
            }
        }

        //impact
        props.endoflife = true;
        if(props.endhandler)
        {
            D3DXVECTOR3 temp = hitpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&temp);
            props.endhandler(hito, hitpos, (dword)props.uservalue, FEHF_LASTHIT);
            props.endhandler = nullptr;
        }
    }
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

 void CMatrixEffectBuoy::Kill(void)
 {
     m_Kill = true;
     if(m_Light.effect)
     {
         ((CMatrixEffectPointLight*)m_Light.effect)->Kill(1100);
         m_Light.Unconnect();
     }
 }

void CMatrixEffectBuoy::Tact(float step)
{
DTRACE();

    m_Props.object->Tact(Float2Int(step));
    m_Props.time += step;

    D3DXMATRIX mr, mr1;

    m_Props.buoy.angle += float(step * 0.001);
    if(m_Props.buoy.angle > M_PI_MUL(2)) m_Props.buoy.angle -= M_PI_MUL(2);
    D3DXMatrixRotationYawPitchRoll(&mr, m_Props.startpos.x, m_Props.startpos.y, m_Props.startpos.z + m_Props.buoy.angle);

    D3DXMatrixScaling(&m_Mat, m_Props.velocity.x, m_Props.velocity.y, m_Props.velocity.z);

    m_Mat *= mr;

    m_Mat._41 = m_Props.curpos.x;
    m_Mat._42 = m_Props.curpos.y;
    //m_Mat._43 = m_Z + sin(m_Props.any0f) * 20;
    m_Mat._43 = m_Props.curpos.z;

    float mul = (float)pow(0.995, double(step));

    m_Props.velocity.x = (m_Props.velocity.x - 1) * mul + 1;
    m_Props.velocity.y = (m_Props.velocity.y - 1) * mul + 1;
    m_Props.velocity.z = (m_Props.velocity.z - 1) * mul + 1;

    m_Props.startpos *= mul;

    //CDText::T("mul", CStr(mul));
 
    if(IRND(7000) < step)
    {
        ((float*)&m_Props.velocity)[IRND(3)] *= 2;

        CMatrixEffect::CreateBillboard(nullptr, m_Props.curpos, 1, 30, 0xFF000000 | m_BuoyColor, m_BuoyColor, 1000, 0, TEXTURE_PATH_WAVE, D3DXVECTOR3(1, 0, 0));
        CMatrixEffect::CreateBillboard(nullptr, m_Props.curpos, 1, 30, 0xFF000000 | m_BuoyColor, m_BuoyColor, 1000, 300, TEXTURE_PATH_WAVE, D3DXVECTOR3(1, 0, 0));
        CMatrixEffect::CreateBillboard(nullptr, m_Props.curpos, 1, 30, 0xFF000000 | m_BuoyColor, m_BuoyColor, 1000, 600, TEXTURE_PATH_WAVE, D3DXVECTOR3(1, 0, 0));
    }
    //if(IRND(7000) < tact) ((float*)&m_Props.startpos)[IRND(3)] = M_PI_MUL(2);

    if(m_Kill)
    {
        m_KillTime -= step;
        if(m_KillTime < 0) goto endoflife;
    }

    if(m_Props.endoflife)
    {
endoflife:
        UnloadObject(m_Props.object, m_Heap);
        m_Props.object = nullptr;
        
#ifdef _DEBUG
        m_Light.Release(DEBUG_CALL_INFO);
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        m_Light.Release();
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    dword lcol = LIC(0, m_BuoyColor, 0.2f);
    lcol = LIC(0, lcol, m_Props.velocity.x);

    if(m_Light.effect) ((CMatrixEffectPointLight*)m_Light.effect)->SetColor(lcol);
    else CMatrixEffect::CreatePointLight(&m_Light, *(D3DXVECTOR3*)&m_Mat._41, 30, lcol, true);
}

void BuoySetupTex(SVOSurface* vo, dword user_param, int)
{
    //ASSERT_DX(g_D3DD->SetTexture(0, vo->m_Tex->Tex()));
    return;
}

bool BuoySetupStages(dword user_param, int)
{
DTRACE();

	ASSERT_DX(g_D3DD->SetFVF(VO_FVF));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));

    //g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetColorOpAnyOrder(1, D3DTOP_ADD, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetAlphaOpSelect(1, D3DTA_CURRENT);

    SetColorOpDisable(2);

    return false;
}

void CMatrixEffectBuoy::Draw(void)
{
DTRACE();

    byte a = 255;

    if(m_Kill) a = byte((m_KillTime / 1000.0f) * 255.0f);

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,0xFFFFFF | (a << 24));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_Mat));

    m_Props.object->Draw(0);

    //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
    //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));

    //g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
}