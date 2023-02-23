// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../stdafx.h"
#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include "../MatrixObject.hpp"
#include "../MatrixRobot.hpp"
#include "../MatrixFlyer.hpp"
#include "../MatrixObjectCannon.hpp"
#include <math.h>

CMatrixEffectWeapon::CMatrixEffectWeapon(const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, dword user, FIRE_END_HANDLER handler, int type, int cooldown) :
    CMatrixEffect(), m_WeaponNum(type), m_User(user), m_Handler(handler), m_Pos(pos), m_Dir(dir), m_CoolDown(cooldown ? float(cooldown) : ((float)(int)type))
#ifdef _DEBUG
    , m_Effect(DEBUG_CALL_INFO)
#endif
{
    m_EffectType = EFFECT_WEAPON;
    m_WeaponCoefficient = g_WeaponDamageNormalCoef;

    m_WeaponDist = g_Config.m_WeaponsConsts[m_WeaponNum].shot_range;
    m_CoolDown = g_Config.m_WeaponsConsts[m_WeaponNum].shots_delay;
    m_ShotSoundType = g_Config.m_WeaponsConsts[m_WeaponNum].shot_sound_num;

    if(!g_Config.m_WeaponsConsts[m_WeaponNum].shot_sound_looped)
    {
        RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
        RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
    }
    else
    {
        SETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
        SETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
    }
}

CMatrixEffectWeapon::~CMatrixEffectWeapon()
{
    FireEnd();

    if(m_WeaponNum == WEAPON_FLAMETHROWER)
    {
        if(m_Effect.effect) m_Effect.Unconnect();
    }

#ifdef _DEBUG
    m_Effect.Release(DEBUG_CALL_INFO);
#else
    m_Effect.Release();
#endif

    if(m_Owner) m_Owner->Release();
}

void CMatrixEffectWeapon::WeaponHit(CMatrixMapStatic* hiti, const D3DXVECTOR3& pos, dword user, dword flags)
{
DTRACE();
    
    CMatrixEffectWeapon* w = (CMatrixEffectWeapon*)user;

    bool give_damage = true;
    byte prim_effect = g_Config.m_WeaponsConsts[w->m_WeaponNum].primary_effect;
    if(prim_effect == EFFECT_CANNON || prim_effect == EFFECT_ROCKET_LAUNCHER)
    {
        if(POW2(w->m_WeaponDist * w->m_WeaponCoefficient) < m_Dist2) give_damage = false;
    }

    bool odead = false;

    dword flags_add = 0;

    if(give_damage && IS_TRACE_STOP_OBJECT(hiti))
    {
        if(w->m_WeaponNum == WEAPON_BOMB)
        {
            D3DXVECTOR3 dir(pos - w->m_Pos);
            D3DXVec3Normalize(&dir, &dir);
            odead = hiti->TakingDamage(w->m_WeaponNum, pos, dir, w->GetSideStorage(), w->GetOwner());
        }
        else odead = hiti->TakingDamage(w->m_WeaponNum, pos, w->m_Dir, w->GetSideStorage(), w->GetOwner());

        if(!odead)
        {
            if(hiti->IsRobot()) flags_add = FEHF_DAMAGE_ROBOT;
        }
    }
    else if(hiti == TRACE_STOP_WATER && w->m_WeaponNum != WEAPON_FLAMETHROWER && w->m_WeaponNum != WEAPON_BOMB)
    {
        CMatrixEffect::CreateKonusSplash(pos, D3DXVECTOR3(0, 0, 1), 10, 5, FSRND(M_PI), 1000, true, (CTextureManaged*)g_Cache->Get(cc_TextureManaged,TEXTURE_PATH_SPLASH));
    }

    if(odead) hiti = TRACE_STOP_NONE;

    if(w->m_Handler) w->m_Handler(hiti, pos, w->m_User, flags | flags_add);

    if(FLAG(flags, FEHF_LASTHIT)) w->Release();
}

void CMatrixEffectWeapon::Release(void)
{
DTRACE();

    --m_Ref;

#ifdef _DEBUG
    if(m_Ref < 0) _asm int 3
#endif

    if(m_Ref <= 0)
    {
        SetDIP();
        HDelete(CMatrixEffectWeapon, this, m_Heap);
    }
}

void CMatrixEffectWeapon::Tact(float step)
{
DTRACE();
    
    if(m_Time < 0) m_Time += step;
    if(m_Time > 0 && !IsFire()) m_Time = 0;

    if(m_Sound != SOUND_ID_EMPTY) m_Sound = CSound::ChangePos(m_Sound, m_ShotSoundType, m_Pos);

    if(IsFire())
    {
        while(m_Time >= 0)
        {
            FireWeapon();
            SETFLAG(m_Flags, WEAPFLAGS_FIREWAS);

            m_Time -= m_CoolDown;
        }
    }
}

void CMatrixEffectWeapon::Modify(const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, const D3DXVECTOR3& speed)
{
    m_Pos = pos;
    m_Dir = dir;
    m_Speed = speed;

    if(m_WeaponNum == WEAPON_PLASMAGUN)
    {
        if(m_Effect.effect && m_Effect.effect->GetType() == EFFECT_KONUS)
        {
            ((CMatrixEffectKonus*)m_Effect.effect)->Modify(pos, dir);
        }
    }
    else if(m_WeaponNum == WEAPON_MACHINEGUN)
    {
        if(m_Volcano) m_Volcano->SetPos(pos, pos + dir * VOLCANO_FIRE_LENGHT, dir);
    }
    else if(m_WeaponNum == WEAPON_DISCHARGER)
    {
        if(m_Effect.effect && m_Effect.effect->GetType() == EFFECT_LIGHTENING)
        {
            D3DXVECTOR3 hitpos(m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient);
            g_MatrixMap->Trace(&hitpos, m_Pos, hitpos, TRACE_ALL, m_Skip);
            ((CMatrixEffectLightening*)m_Effect.effect)->SetPos(m_Pos, hitpos);
        }
    }
    else if(m_WeaponNum == WEAPON_LASER || m_WeaponNum == WEAPON_TURRET_LASER)
    {
        if(m_Laser)
        {
            D3DXVECTOR3 hitpos(m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient);
            g_MatrixMap->Trace(&hitpos, m_Pos, hitpos, TRACE_ALL, m_Skip);
            
            m_Laser->SetPos(m_Pos, hitpos);
        }
    }
    else if(g_Config.m_WeaponsConsts[m_WeaponNum].is_repairer)
    {
        if(m_Repair) m_Repair->UpdateData(pos, dir);
    }
}

void CMatrixEffectWeapon::FireWeapon()
{
    ++m_FireCount;

    if(m_ShotSoundType != S_NONE)
    {
        if(FLAG(m_Flags, WEAPFLAGS_SND_SKIP)) m_Sound = CSound::Play(m_Sound, m_ShotSoundType, m_Pos);
        else m_Sound = CSound::Play(m_ShotSoundType, m_Pos);
    }

    if(g_Config.m_WeaponsConsts[m_WeaponNum].primary_effect)
    {
        switch(g_Config.m_WeaponsConsts[m_WeaponNum].primary_effect)
        {
            //Оружие, спавнящее проджектайлы
            case EFFECT_CANNON:
            {
                SMOProps mo;
                mo.gun.maxdist = m_WeaponDist * m_WeaponCoefficient;
                mo.gun.splash_radius = g_Config.m_WeaponsConsts[m_WeaponNum].projectile_splash_radius;
                //Для отрисовки инверсионного следа от снаряда
                if(g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[1].sprites_count)
                {
                    mo.gun.contrail_sprite_num = (ESpriteTextureSort)g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[1].sprites_num[0];
                    mo.gun.contrail_width = g_Config.m_WeaponsConsts[m_WeaponNum].contrail_width;
                    mo.gun.contrail_duration = g_Config.m_WeaponsConsts[m_WeaponNum].contrail_duration;
                    mo.gun.contrail_color = g_Config.m_WeaponsConsts[m_WeaponNum].hex_BGRA_sprites_color;
                }
                else mo.gun.contrail_sprite_num = SPR_NONE;

                mo.startpos = m_Pos;
                mo.target = m_Speed; //Нахуя?
                mo.curpos = mo.startpos;

                mo.side = m_SideStorage;
                //mo.attacker = m_Owner;
                //if(mo.attacker) mo.attacker->RefInc();

                mo.velocity = m_Dir * g_Config.m_WeaponsConsts[m_WeaponNum].projectile_full_velocity + m_Speed;
                mo.object = LoadObject(g_Config.m_WeaponsConsts[m_WeaponNum].projectile_model_path, m_Heap);
                mo.handler = MO_Cannon_Round_Tact;

                ++m_Ref;
                CMatrixEffect::CreateMovingObject(nullptr, mo, TRACE_ALL, m_Skip, WeaponHit, (dword)this);

                if(g_Config.m_WeaponsConsts[m_WeaponNum].light_radius)
                {
                    SEffectHandler eh;
                    CMatrixEffect::CreatePointLight(&eh, m_Pos, g_Config.m_WeaponsConsts[m_WeaponNum].light_radius, g_Config.m_WeaponsConsts[m_WeaponNum].hex_BGRA_light_color, false);
                    if(eh.effect)
                    {
                        ((CMatrixEffectPointLight*)eh.effect)->Kill(g_Config.m_WeaponsConsts[m_WeaponNum].light_duration);
                        eh.Unconnect();
                    }
                }

                if(g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[0].sprites_count) //На случай, если спрайтовую "вспышку" отрубили совсем
                {
                    int f = IRND(g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[0].sprites_count);
                    CMatrixEffect::CreateSpritesLine(nullptr, m_Pos, m_Pos + m_Dir * g_Config.m_WeaponsConsts[m_WeaponNum].sprites_lenght, g_Config.m_WeaponsConsts[m_WeaponNum].sprites_width, 0xFFFFFFFF, 0, 1000, m_SpriteTextures[g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[0].sprites_num[f]].tex);
                }

                break;
            }
            case EFFECT_ROCKET_LAUNCHER:
            {
                SMOProps mo;
                mo.startpos = m_Pos;
                mo.target = m_Pos + m_Dir * 5000;
                mo.curpos = mo.startpos;

                mo.hm.full_velocity_reach = g_Config.m_WeaponsConsts[m_WeaponNum].projectile_full_velocity_reach;
                if(!mo.hm.full_velocity_reach) //Если у ракеты нет периода начальной скорости, то стартует она сразу с полной
                {
                    mo.velocity = m_Dir * g_Config.m_WeaponsConsts[m_WeaponNum].projectile_full_velocity + m_Speed;
                    mo.hm.is_full_velocity = true;
                }
                else
                {
                    mo.velocity = m_Dir * g_Config.m_WeaponsConsts[m_WeaponNum].projectile_start_velocity + m_Speed;
                    mo.hm.full_velocity = m_Dir * g_Config.m_WeaponsConsts[m_WeaponNum].projectile_full_velocity + m_Speed;
                }
                mo.hm.acceleration_coef = g_Config.m_WeaponsConsts[m_WeaponNum].projectile_acceleration_coef;
                mo.hm.target_capture_angle_cos = g_Config.m_WeaponsConsts[m_WeaponNum].projectile_target_capture_angle_cos;
                mo.hm.homing_speed = g_Config.m_WeaponsConsts[m_WeaponNum].projectile_homing_speed;
                if(m_User)
                {
                    /* хуй почему знает, но такое выставление цели крашит игру где-то в переборе эффектов
                    if(((CMatrixMapStatic*)m_User)->IsCannon())
                    {
                        CMatrixCannon* cannon = (CMatrixCannon*)m_User;
                        if(cannon->m_TargetCore) mo.hm.target = cannon->m_TargetCore;
                    }
                    else
                    */
                    if(((CMatrixMapStatic*)m_User)->IsRobot())
                    {
                        //Если у робота установлен модуль, изменяющий систему наведения ракет
                        CMatrixRobotAI* robot = (CMatrixRobotAI*)m_User;
                        if(robot->m_MissileTargetCaptureAngleCos != 1.0f)
                        {
                            mo.hm.target_capture_angle_cos = mo.hm.target_capture_angle_cos * robot->m_MissileTargetCaptureAngleCos - g_Config.m_WeaponsConsts[m_WeaponNum].projectile_target_capture_angle_sin * robot->m_MissileTargetCaptureAngleSin;
                        }
                        mo.hm.homing_speed += robot->m_MissileHomingSpeed;
                    }
                }
                mo.hm.splash_radius = g_Config.m_WeaponsConsts[m_WeaponNum].projectile_splash_radius;
                mo.hm.max_lifetime = g_Config.m_WeaponsConsts[m_WeaponNum].projectile_max_lifetime;

                if(m_User) mo.hm.missile_owner = (CMatrixMapStatic*)m_User;

                mo.hm.fire_effect_starts = g_Config.m_WeaponsConsts[m_WeaponNum].contrail_fire_effect_starts;
                mo.hm.close_color_rgb = g_Config.m_WeaponsConsts[m_WeaponNum].close_color_rgb;
                mo.hm.far_color_rgb = g_Config.m_WeaponsConsts[m_WeaponNum].far_color_rgb;

                mo.object = LoadObject(g_Config.m_WeaponsConsts[m_WeaponNum].projectile_model_path, m_Heap);
                mo.handler = MO_Homing_Missile_Tact;

                mo.side = m_SideStorage;
                //mo.attacker = m_Owner;
                //if(mo.attacker) mo.attacker->RefInc();

                mo.shleif = (SEffectHandler*)HAlloc(sizeof(SEffectHandler), m_Heap);
                mo.shleif->effect = nullptr;
                if(g_Config.m_WeaponsConsts[m_WeaponNum].contrail_duration) CMatrixEffect::CreateShleif(mo.shleif);
                ++m_Ref;
                CMatrixEffect::CreateMovingObject(nullptr, mo, TRACE_ALL, m_Skip, WeaponHit, (dword)this);

                if(g_Config.m_WeaponsConsts[m_WeaponNum].light_radius)
                {
                    SEffectHandler eh;
                    CMatrixEffect::CreatePointLight(&eh, m_Pos, g_Config.m_WeaponsConsts[m_WeaponNum].light_radius, g_Config.m_WeaponsConsts[m_WeaponNum].hex_BGRA_light_color, false);
                    if(eh.effect)
                    {
                        ((CMatrixEffectPointLight*)eh.effect)->Kill(g_Config.m_WeaponsConsts[m_WeaponNum].light_duration);
                        eh.Unconnect();
                    }
                }

                if(g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[0].sprites_count) //На случай, если спрайтовую "вспышку" отрубили совсем
                {
                    int f = IRND(g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[0].sprites_count);
                    CMatrixEffect::CreateSpritesLine(nullptr, m_Pos, m_Pos + m_Dir * g_Config.m_WeaponsConsts[m_WeaponNum].sprites_lenght, g_Config.m_WeaponsConsts[m_WeaponNum].sprites_width, 0xFFFFFFFF, 0, 1000, m_SpriteTextures[g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[0].sprites_num[f]].tex);
                }

                break;
            }
            case EFFECT_MORTAR:
            {
                SMOProps mo;
                mo.startpos = m_Pos;
                mo.curpos = mo.startpos;

                //CHelper::Create(100, 0)->Line(m_Speed, m_Speed + D3DXVECTOR3(0, 0, 100));

                float len = D3DXVec3Length(&(m_Pos - m_Speed));
                D3DXVECTOR3 dir((m_Pos - m_Speed) * (1.0f / len));
                if(len > m_WeaponDist * m_WeaponCoefficient) len = m_WeaponDist * m_WeaponCoefficient;

                mo.target = m_Pos - dir * len;
                D3DXVECTOR3 mid((m_Pos + mo.target) * 0.5f);

                //SPlane pl;
                //SPlane::BuildFromPointNormal(pl, mid, dir);
                //float ttt;
                //pl.FindIntersect(m_Pos, m_Dir, ttt);

                int pcnt = 5;
                D3DXVECTOR3 pts[5];

                pts[0] = m_Pos;

                if(m_Dir.z < 0)
                {
                    // on flyer bombomet
                    pts[1] = m_Pos + m_Dir * 70 - D3DXVECTOR3(0, 0, 25);
                    pts[2] = m_Pos + m_Dir * 110 - D3DXVECTOR3(0, 0, 100);
                    pts[3] = m_Pos + m_Dir * 130 - D3DXVECTOR3(0, 0, 300);

                    pcnt = 4;

                    mo.velocity.x = 0.0006f;
                }
                else
                {
                    pts[1] = mid + dir * len * 0.15f + D3DXVECTOR3(0, 0, len * 0.5f);
                    pts[2] = mid - dir * len * 0.15f + D3DXVECTOR3(0, 0, len * 0.5f);
                    pts[3] = mo.target;
                    // more beautiful trajectory
                    pts[4] = mo.target - dir * len * 0.35f - D3DXVECTOR3(0, 0, len * 0.5f);

                    mo.velocity.x = 0.0005f;
                }

                //CHelper::Create(1, 0)->Line(pts[0], pts[1]);
                //CHelper::Create(1, 0)->Line(pts[1], pts[2]);
                //CHelper::Create(1, 0)->Line(pts[2], pts[3]);

                mo.bomb.pos = 0;
                mo.bomb.trajectory = HNew(g_MatrixHeap) CTrajectory(g_MatrixHeap, pts, pcnt);

                //CHelper::DestroyByGroup(123);

                //D3DXVECTOR3 pp;
                //mo.common.bomb.trajectory->CalcPoint(pp,0);
                //for(float t = 0.01f; t <= 1.0f; t += 0.01f)
                //{
                //    D3DXVECTOR3 ppp;
                //    mo.common.bomb.trajectory->CalcPoint(ppp, t);
                //    CHelper::Create(100000, 123)->Line(pp, ppp);

                //    pp = ppp;
                //}

                //float speed = D3DXVec3Length(&(m_Pos-m_Speed)) / 100;
                //if(speed < 1) speed = 1;
                //if(speed > 3) speed = 3;

                //speed -= (speed - 1.2f) * 0.3f;

                //mo.velocity = m_Dir * speed * 1.5f;
                mo.object = LoadObject(L"Matrix\\Bullets\\mina.vo", m_Heap);
                mo.handler = MO_Grenade_Tact;

                mo.side = m_SideStorage;
                //mo.attacker = m_Owner;
                //if(mo.attacker) mo.attacker->RefInc();

                mo.shleif = (SEffectHandler*)HAlloc(sizeof(SEffectHandler), m_Heap);
                mo.shleif->effect = nullptr;
                CMatrixEffect::CreateShleif(mo.shleif);

                ++m_Ref;
                CMatrixEffect::CreateMovingObject(nullptr, mo, TRACE_ALL, m_Skip, WeaponHit, (dword)this);

                break;
            }

            case EFFECT_REPAIRER:
            {
                if(m_Repair) m_Repair->UpdateData(m_Pos, m_Dir);
                else
                {
                    m_Repair = (CMatrixEffectRepair*)CreateRepair(m_Pos, m_Dir, m_WeaponDist * m_WeaponCoefficient, m_Skip, (ESpriteTextureSort)g_Config.m_WeaponsConsts[m_WeaponNum].sprite_set[0].sprites_num[0]);
                    if(!g_MatrixMap->AddEffect(m_Laser)) m_Repair = nullptr;
                }

                if(m_Repair)
                {
                    CMatrixMapStatic* ms = m_Repair->GetTarget();
                    if(ms)
                    {
                        if(m_Handler) m_Handler(ms, ms->GetGeoCenter(), m_User, FEHF_LASTHIT);
                        SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
                        ms->TakingDamage(m_WeaponNum, ms->GetGeoCenter(), m_Dir, GetSideStorage(), GetOwner());
                    }
                    else if(m_Handler) m_Handler(TRACE_STOP_NONE, m_Pos, m_User, FEHF_LASTHIT);
                }

                break;
            }
            case EFFECT_BOMB:
            {
                ++m_Ref;
                CMatrixEffect::CreateBigBoom(m_Pos, m_WeaponDist * m_WeaponCoefficient, 300, TRACE_ALL, nullptr/*m_Skip*/, (dword)this, WeaponHit);
                CMatrixEffect::CreateBigBoom(m_Pos, m_WeaponDist, 350, 0, 0, 0, 0);
                CMatrixEffect::CreateBigBoom(m_Pos, m_WeaponDist, 400, 0, 0, 0, 0, 0xFFAFAF40);
                CMatrixEffect::CreateExplosion(m_Pos, ExplosionBigBoom, true);

                break;
            }
        }
    }
    else switch(m_WeaponNum)
    {
        case WEAPON_PLASMAGUN:
        {
            float ang = float(2 * M_PI / 4096.0) * (g_MatrixMap->GetTime() & 4095);
            CMatrixEffect::CreateKonus(&m_Effect, m_Pos, m_Dir, 10, 10, ang, 300, true, nullptr);
            ++m_Ref;
            CMatrixEffect::CreateFirePlasma(m_Pos, m_Pos + (m_Dir * m_WeaponDist * m_WeaponCoefficient), 0.5f, TRACE_ALL, m_Skip, WeaponHit, (dword)this);
            break;
        }
        case WEAPON_MACHINEGUN:
        {
            if(m_Volcano)
            {
                //g_MatrixMap->SubEffect(m_Konus);
                float ang = float(2 * M_PI / 4096.0) * (g_MatrixMap->GetTime() & 4095);
                m_Volcano->SetPos(m_Pos, m_Pos + m_Dir * VOLCANO_FIRE_LENGHT, m_Dir, ang);
            }
            else
            {
                float ang = float(2 * M_PI / 4096.0) * (g_MatrixMap->GetTime() & 4095);
                m_Volcano = HNew (m_Heap) CVolcano(m_Pos, m_Dir, ang);
                if(!g_MatrixMap->AddEffect(m_Volcano)) m_Volcano = nullptr;
            }

            dword flags_add = FEHF_LASTHIT;

            D3DXVECTOR3 hitpos;
            D3DXVECTOR3 splash;
            CMatrixMapStatic* s = g_MatrixMap->Trace(&hitpos, m_Pos, m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient, TRACE_ALL, m_Skip);
            if(s == TRACE_STOP_NONE) break;
            if(IS_TRACE_STOP_OBJECT(s))
            {
                bool dead = s->TakingDamage(m_WeaponNum, hitpos, m_Dir, GetSideStorage(), GetOwner());
                SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
                if(dead) s = TRACE_STOP_NONE;
                splash = -m_Dir;
            }
            else if(s == TRACE_STOP_WATER)
            {
                splash = D3DXVECTOR3(0, 0, 1);
                CMatrixEffect::CreateKonusSplash(hitpos, splash, 10, 5, FSRND(M_PI), 1000, true, (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPLASH));
            }
            else
            {
                g_MatrixMap->GetNormal(&splash, hitpos.x, hitpos.y);
                CMatrixEffect::CreateKonus(nullptr, hitpos, splash, 5, 10, FSRND(M_PI), 300, true, (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_GUN_BULLETS1));
                CMatrixEffect::CreateKonus(nullptr, hitpos, splash, 5, 5, FSRND(M_PI), 300, true, (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_GUN_BULLETS2));
            }

            if(FRND(1) < 0.1f)
            {
                //CHelper::Create(1, 1)->Line(m_Pos, hitpos, 0x80808080, 0x80808080);
                CMatrixEffect::CreateSpritesLine(nullptr, m_Pos, hitpos , 0.5f, 0x80FFFFFF, 0, 100, m_SpriteTextures[SPR_CONTRAIL].tex);
            }

            if(m_Handler) m_Handler(s, hitpos, m_User, FEHF_LASTHIT);

            break;
        }

        case WEAPON_DISCHARGER:
        {
            D3DXVECTOR3 hitpos = m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient;
            CMatrixMapStatic* s = g_MatrixMap->Trace(&hitpos, m_Pos, hitpos, TRACE_ALL, m_Skip);

            //float l = D3DXVec3Length(&(m_Pos - hitpos));
            //CDText::T("len", CStr(l));

            //CHelper::Create(10, 0)->Line(m_Pos, hitpos);
            //CHelper::Create(10, 0)->Line(hitpos, hitpos + D3DXVECTOR3(0, 0, 100));

            if(IS_TRACE_STOP_OBJECT(s))
            {
                bool dead = s->TakingDamage(m_WeaponNum, hitpos, m_Dir, GetSideStorage(), GetOwner());
                SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
                if(dead) s = TRACE_STOP_NONE;
                else
                {
                    if(!s->IsFlyer())
                    {
                        D3DXVECTOR3 pos1(hitpos.x + FSRND(20), hitpos.y + FSRND(20), 0);
                        pos1.z = g_MatrixMap->GetZ(pos1.x, pos1.y);
                        CMatrixEffect::CreateShorted(hitpos, pos1,FRND(500) + 400);
                    }
                }
            }
            else if(s == TRACE_STOP_LANDSCAPE)
            {
                CMatrixEffect::CreateLandscapeSpot(nullptr, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(1) + 0.5f, SPOT_PLASMA_HIT);

                //e = CMatrixEffect::CreateShorted(hitpos + D3DXVECTOR3(FSRND(5), FSRND(5), 0), hitpos + D3DXVECTOR3(FSRND(5), FSRND(5), 0), int(FRND(500) + 400));
                //g_MatrixMap->AddEffect(e);
            }

            if(m_Effect.effect && m_Effect.effect->GetType() == EFFECT_LIGHTENING) ((CMatrixEffectLightening *)m_Effect.effect)->SetPos(m_Pos, hitpos);
            else CMatrixEffect::CreateLightening(&m_Effect, m_Pos, hitpos, 1000000, 3, LIGHTENING_WIDTH);

            if(m_Handler) m_Handler(s, hitpos, m_User, FEHF_LASTHIT);

            break;
        }
        case WEAPON_LASER:
        case WEAPON_TURRET_LASER:
        {
            D3DXVECTOR3 hitpos = m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient;
            CMatrixMapStatic* s = g_MatrixMap->Trace(&hitpos, m_Pos, hitpos, TRACE_ALL, m_Skip);

            if(IS_TRACE_STOP_OBJECT(s))
            {
                bool dead = s->TakingDamage(m_WeaponNum, hitpos, m_Dir, GetSideStorage(), GetOwner());
                SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
                if(dead) s = TRACE_STOP_NONE;

                /*
                D3DXVECTOR3 pos1(hitpos.x + FSRND(20), hitpos.y + FSRND(20), 0);
                pos1.z = g_MatrixMap->GetZ(pos1.x, pos1.y);
                e = CMatrixEffect::CreateShorted(hitpos, pos1, int(FRND(500) + 400));
                g_MatrixMap->AddEffect(e);
                */
                
                CMatrixEffect::CreateExplosion(hitpos, ExplosionLaserHit);

                if(m_Handler) m_Handler(s, hitpos, m_User, FEHF_LASTHIT);
            }
            else if(s == TRACE_STOP_LANDSCAPE)
            {
                CMatrixEffect::CreateLandscapeSpot(nullptr, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(1) + 0.5f, SPOT_PLASMA_HIT);
                CMatrixEffect::CreateExplosion(hitpos, ExplosionLaserHit);
            }
            else if(s == TRACE_STOP_WATER)
            {
                CMatrixEffect::CreateSmoke(nullptr, hitpos, 100, 1400, 10, 0xFFFFFF);
            }

            if(m_Laser) m_Laser->SetPos(m_Pos, hitpos);
            else
            {
                m_Laser = HNew(m_Heap) CLaser(m_Pos, hitpos);
                if(!g_MatrixMap->AddEffect(m_Laser)) m_Laser = nullptr;
            }

            if(m_Handler) m_Handler(s, hitpos, m_User, FEHF_LASTHIT);

            break;
        }
        case WEAPON_FLAMETHROWER:
        {
            if(m_Effect.effect == nullptr)
            {
                float ttl = m_WeaponDist * m_WeaponCoefficient * 8.333f;
                CMatrixEffect::CreateFlame(&m_Effect, ttl, TRACE_ALL, m_Skip, (dword)this, WeaponHit);
                if(m_Effect.effect == nullptr) break;
                ++m_Ref;
            }
            ((CMatrixEffectFlame*)m_Effect.effect)->AddPuff(m_Pos, m_Dir, m_Speed);
            break;
        }

        default: CMatrixEffect::CreateExplosion(m_Pos, ExplosionRobotBoom, true);
    }
}

void CMatrixEffectWeapon::FireEnd(void)
{
    if(!IsFire()) return;
    RESETFLAG(m_Flags, WEAPFLAGS_FIRE);

    if(m_ShotSoundType != S_NONE)
    {
        if(FLAG(m_Flags, WEAPFLAGS_SND_OFF))
        {
            CSound::StopPlay(m_Sound);
            m_Sound = SOUND_ID_EMPTY;
        }
    }

    if(g_Config.m_WeaponsConsts[m_WeaponNum].is_repairer)
    {
        if(m_Repair)
        {
            //CMatrixMapStatic* ms = m_Repair->GetTarget();
            //if(ms && m_Handler) m_Handler(ms, ms->GetGeoCenter(), m_User, FEHF_LASTHIT);
            g_MatrixMap->SubEffect(m_Repair);
            m_Repair = nullptr;
        }
    }
    else switch(m_WeaponNum)
    {
        case WEAPON_DISCHARGER:
        {
            m_Effect.Release();
            break;
        }
        case WEAPON_LASER:
        case WEAPON_TURRET_LASER:
        {
            if(m_Laser)
            {
                g_MatrixMap->SubEffect(m_Laser);
                m_Laser = nullptr;
            }
            break;
        }
        case WEAPON_MACHINEGUN:
        {
            if(m_Volcano)
            {
                g_MatrixMap->SubEffect(m_Volcano);
                m_Volcano = nullptr;
            }
            break;
        }
        case WEAPON_FLAMETHROWER:
        {
            if(m_Effect.effect) ((CMatrixEffectFlame*)m_Effect.effect)->Break();
            break;
        }
    }
}

//Предполагалось использовать для отрисовки эффекта автохила, но не разобрался с конвертацией подходящей модели для головы
void CMatrixEffectWeapon::SelfRepairEffect()
{
    /*
    if(m_Repair) m_Repair->UpdateData(m_Pos, m_Dir);
    else
    {
        m_Repair = (CMatrixEffectRepair*)CreateRepair(m_Pos, m_Dir, m_WeaponDist * m_WeaponCoefficient, m_Skip, (ESpriteTextureSort)g_Config.m_WeaponsConsts[m_Type].sprite_set[0].sprites_num[0]);
        if(!g_MatrixMap->AddEffect(m_Laser)) m_Repair = nullptr;
    }

    if(m_Repair)
    {
        CMatrixMapStatic* ms = m_Repair->GetTarget();
        if(ms)
        {
            if(m_Handler) m_Handler(ms, ms->GetGeoCenter(), m_User, FEHF_LASTHIT);
            SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
            ms->TakingDamage(m_WeaponNum, ms->GetGeoCenter(), m_Dir, GetSideStorage(), GetOwner());
        }
        else if(m_Handler) m_Handler(TRACE_STOP_NONE, m_Pos, m_User, FEHF_LASTHIT);
    }
    */
}

void CMatrixEffectWeapon::SoundHit(int weapon_num, const D3DXVECTOR3& pos)
{
    if(g_Config.m_WeaponsConsts[weapon_num].hit_sound_num == S_NONE) return;
    //CSound::AddSound(g_Config.m_WeaponsConsts[8].hit_sound_num, pos, SL_ALL, SEF_SKIP); //plasma hit
    CSound::AddSound(g_Config.m_WeaponsConsts[weapon_num].hit_sound_num, pos, SL_ALL, SEF_SKIP);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

CLaser::CLaser(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1) :
CMatrixEffect(), m_Sprites(TRACE_PARAM_CALL pos0, pos1, LASER_WIDTH, 0xFFFFFFFF, m_SpriteTextures[SPR_LASER_BEAM].tex)
{
    m_EffectType = EFFECT_LASER;

    if(m_SpriteTextures[SPR_LASER_SPOT].IsSingleBrightTexture()) m_end.CSprite::CSprite(TRACE_PARAM_CALL pos0, LASER_WIDTH * 0.5f, 0, 0xFFFFFFFF, m_SpriteTextures[SPR_LASER_SPOT].tex);
    else m_end.CSprite::CSprite(TRACE_PARAM_CALL pos0, LASER_WIDTH * 0.5f, 0, 0xFFFFFFFF, &m_SpriteTextures[SPR_LASER_SPOT].spr_tex);
}

void CLaser::Draw(void)
{
DTRACE();

    BYTE a = g_MatrixMap->IsPaused()?240:(BYTE(FRND(128) + 128));

    m_Sprites.SetAlpha(a);
    m_Sprites.AddToDrawQueue();

    m_end.SetAlpha(a);
    m_end.Sort(g_MatrixMap->m_Camera.GetViewMatrix());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

CVolcano::CVolcano(const D3DXVECTOR3& start, const D3DXVECTOR3& dir, float angle) :
CMatrixEffect(), m_Konus(start, dir, VOLCANO_FIRE_KONUS_RADIUS, VOLCANO_FIRE_KONUS_LENGTH, angle, 300, true, nullptr),
m_bl1(TRACE_PARAM_CALL start, start + dir * VOLCANO_FIRE_LENGHT, VOLCANO_FIRE_WIDTH, 0xFFFFFFFF, m_SpriteTextures[SPR_GUN_FLASH_1].tex),
m_bl2(TRACE_PARAM_CALL start, start + dir * VOLCANO_FIRE_LENGHT, VOLCANO_FIRE_WIDTH, 0xFFFFFFFF, m_SpriteTextures[SPR_GUN_FLASH_2].tex),
m_bl3(TRACE_PARAM_CALL start, start + dir * VOLCANO_FIRE_LENGHT, VOLCANO_FIRE_WIDTH, 0xFFFFFFFF, m_SpriteTextures[SPR_GUN_FLASH_3].tex)
{
    m_EffectType = EFFECT_VOLCANO;
}

void CVolcano::Draw(void)
{
    m_Konus.Draw();

    switch(g_MatrixMap->IsPaused() ? 0 : (IRND(3)))
    {
        case 0: m_bl1.AddToDrawQueue(); break;
        case 1: m_bl2.AddToDrawQueue(); break;
        case 2: m_bl3.AddToDrawQueue(); break;
    }
}