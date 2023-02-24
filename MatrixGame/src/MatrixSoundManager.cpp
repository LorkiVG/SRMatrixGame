// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixSoundManager.hpp"
#include "MatrixGameDll.hpp"
#include "Matrixmap.hpp"

std::vector<CSound::SSoundItem> CSound::m_Sounds;
CSound::SLID         CSound::m_LayersI[SL_COUNT];
int                  CSound::m_LastGroup;
dword                CSound::m_LastID;
CSound::SPlayedSound CSound::m_AllSounds[MAX_SOUNDS];
CDWORDMap*           CSound::m_PosSounds;

__forceinline dword snd_create(wchar* n, int i, int j)
{
DTRACE();

    return g_RangersInterface->m_SoundCreate(n, i, j);
}

__forceinline void snd_destroy(dword s)
{
DTRACE();

    g_RangersInterface->m_SoundDestroy(s);
}

__forceinline void snd_pan(dword s, float v)
{
DTRACE();

    g_RangersInterface->m_SoundPan(s, v);
}
__forceinline void snd_vol(dword s, float v)
{
DTRACE();

    g_RangersInterface->m_SoundVolume(s, v);
}

__forceinline void snd_play(dword s)
{
DTRACE();

    g_RangersInterface->m_SoundPlay(s);
}

void CSound::Init(void)
{
DTRACE();

    m_LastGroup = 0;
    m_LastID = 0;
    for(int i = 0; i < SL_COUNT; ++i) m_LayersI[i].index = -1;

#ifdef _DEBUG
    for(int i = 0; i < S_COUNT; ++i) SETFLAG(m_Sounds[i].flags, SSoundItem::NOTINITED);
#endif

    m_PosSounds = HNew(g_MatrixHeap) CDWORDMap(g_MatrixHeap);
    
    m_Sounds.resize(S_COUNT); //»нициализируем массив на базовое количество звуков

    //«аполн€ем строки путей дл€ базового набора звуков
    m_Sounds[S_BCLICK].path_name = L"bclick";
    m_Sounds[S_BENTER].path_name = L"benter";
    m_Sounds[S_BLEAVE].path_name = L"bleave";
    m_Sounds[S_MAP_PLUS].path_name = L"map_plus";
    m_Sounds[S_MAP_MINUS].path_name = L"map_minus";
    m_Sounds[S_PRESET_CLICK].path_name = L"preset_click";
    m_Sounds[S_BUILD_CLICK].path_name = L"build_click";
    m_Sounds[S_CANCEL_CLICK].path_name = L"cancel_click";


    m_Sounds[S_DOORS_OPEN].path_name = L"base_doors_open";
    m_Sounds[S_DOORS_CLOSE].path_name = L"base_doors_close";
    m_Sounds[S_DOORS_CLOSE_STOP].path_name = L"base_doors_close_stop";
    m_Sounds[S_PLATFORM_UP].path_name = L"base_platform_up";
    m_Sounds[S_PLATFORM_DOWN].path_name = L"base_platform_down";
    m_Sounds[S_PLATFORM_UP_STOP].path_name = L"base_platform_up_stop";

    //‘оновые звуки дл€ базы и всех заводов
    //m_Sounds[S_BASE_AMBIENT].path_name = L"base_amb";

    //m_Sounds[S_TITAN_AMBIENT].path_name = L"titan_amb";
    //m_Sounds[S_ELECTRONIC_AMBIENT].path_name = L"electronic_amb";
    //m_Sounds[S_ENERGY_AMBIENT].path_name = L"energy_amb";
    //m_Sounds[S_PLASMA_AMBIENT].path_name = L"plasma_amb";
    //m_Sounds[S_REPAIR_AMBIENT].path_name = L"repair_amb";

    m_Sounds[S_SPLASH].path_name = L"splash";

    m_Sounds[S_EF_START].path_name = L"ef_start"; //elevator fields
    m_Sounds[S_EF_CONTINUE].path_name = L"ef_cont";
    m_Sounds[S_EF_END].path_name = L"ef_end";

    m_Sounds[S_FLYER_PROPELLER_START].path_name = L"fl_start";
    m_Sounds[S_FLYER_PROPELLER_CONTINUE].path_name = L"fl_cont";

    m_Sounds[S_ROBOT_BUILD_END].path_name = L"r_build_e";
    m_Sounds[S_ROBOT_BUILD_END_ALT].path_name = L"r_build_ea";

    m_Sounds[S_TURRET_BUILD_START].path_name = L"t_build_s";
    m_Sounds[S_TURRET_BUILD_0].path_name = L"t_build_0";
    m_Sounds[S_TURRET_BUILD_1].path_name = L"t_build_1";
    m_Sounds[S_TURRET_BUILD_2].path_name = L"t_build_2";
    m_Sounds[S_TURRET_BUILD_3].path_name = L"t_build_3";

    m_Sounds[S_FLYER_BUILD_END].path_name = L"f_build_e";
    m_Sounds[S_FLYER_BUILD_END_ALT].path_name = L"f_build_ea";

    m_Sounds[S_YES_SIR_1].path_name = L"s_yes_1";
    m_Sounds[S_YES_SIR_2].path_name = L"s_yes_2";
    m_Sounds[S_YES_SIR_3].path_name = L"s_yes_3";
    m_Sounds[S_YES_SIR_4].path_name = L"s_yes_4";
    m_Sounds[S_YES_SIR_5].path_name = L"s_yes_5";

    m_Sounds[S_SELECTION_1].path_name = L"s_selection_1";
    m_Sounds[S_SELECTION_2].path_name = L"s_selection_2";
    m_Sounds[S_SELECTION_3].path_name = L"s_selection_3";
    m_Sounds[S_SELECTION_4].path_name = L"s_selection_4";
    m_Sounds[S_SELECTION_5].path_name = L"s_selection_5";
    m_Sounds[S_SELECTION_6].path_name = L"s_selection_6";
    m_Sounds[S_SELECTION_7].path_name = L"s_selection_7";

    m_Sounds[S_BUILDING_SEL].path_name = L"s_building_sel";
    m_Sounds[S_BASE_SEL].path_name = L"s_base_sel";
    m_Sounds[S_WEAPON_SET].path_name = L"weapon_set";
    m_Sounds[S_GUIDANCE_SWITCH].path_name = L"guidance_switch";

    m_Sounds[S_REINFORCEMENTS_CALLED].path_name = L"s_maintenance";
    m_Sounds[S_REINFORCEMENTS_READY].path_name = L"s_maintenance_on";
    m_Sounds[S_RESINCOME].path_name = L"s_resincome";

    m_Sounds[S_SIDE_UNDER_ATTACK_1].path_name = L"s_side_attacked_1";
    m_Sounds[S_SIDE_UNDER_ATTACK_2].path_name = L"s_side_attacked_2";
    m_Sounds[S_SIDE_UNDER_ATTACK_3].path_name = L"s_side_attacked_3";

    m_Sounds[S_ENEMY_BASE_CAPTURED].path_name = L"s_eb_cap";
    m_Sounds[S_ENEMY_FACTORY_CAPTURED].path_name = L"s_ef_cap";
    m_Sounds[S_PLAYER_BASE_CAPTURED].path_name = L"s_pb_cap";
    m_Sounds[S_PLAYER_FACTORY_CAPTURED].path_name = L"s_pf_cap";

    m_Sounds[S_BASE_KILLED].path_name = L"s_base_dead";
    m_Sounds[S_FACTORY_KILLED].path_name = L"s_fa_dead";
    m_Sounds[S_BUILDING_KILLED].path_name = L"s_building_dead";

    m_Sounds[S_ORDER_INPROGRESS1].path_name = L"s_ord_inprogress1";
    m_Sounds[S_ORDER_INPROGRESS2].path_name = L"s_ord_inprogress2";

    m_Sounds[S_ORDER_ACCEPT].path_name = L"s_ord_accept";
    m_Sounds[S_ORDER_ATTACK].path_name = L"s_ord_attack";
    m_Sounds[S_ORDER_CAPTURE].path_name = L"s_ord_capture";
    m_Sounds[S_ORDER_CAPTURE_PUSH].path_name = L"s_ord_capture_push";
    m_Sounds[S_ORDER_REPAIR].path_name = L"s_ord_repair";

    m_Sounds[S_ORDER_AUTO_ATTACK].path_name = L"s_orda_attack";
    m_Sounds[S_ORDER_AUTO_CAPTURE].path_name = L"s_orda_capture";
    m_Sounds[S_ORDER_AUTO_DEFENCE].path_name = L"s_orda_defence";

    m_Sounds[S_TERRON_PAIN1].path_name = L"s_terron_pain1";
    m_Sounds[S_TERRON_PAIN2].path_name = L"s_terron_pain2";
    m_Sounds[S_TERRON_PAIN3].path_name = L"s_terron_pain3";
    m_Sounds[S_TERRON_PAIN4].path_name = L"s_terron_pain4";
    m_Sounds[S_TERRON_KILLED].path_name = L"s_terron_killed";

    m_Sounds[S_ORDER_CAPTURE_FUCK_OFF].path_name = L"s_ord_capoff";

    m_Sounds[S_ROBOT_UPAL].path_name = L"s_upal";

    m_Sounds[S_CANTBE].path_name = L"s_cantbe";

    m_Sounds[S_SPECIAL_SLOT].path_name = L"";

    m_Sounds[S_EXPLOSION_NORMAL].path_name = L"expl_norm";
    m_Sounds[S_EXPLOSION_MISSILE].path_name = L"expl_missile";
    m_Sounds[S_EXPLOSION_ROBOT_HIT].path_name = L"expl_rh";
    m_Sounds[S_EXPLOSION_LASER_HIT].path_name = L"expl_lh";
    m_Sounds[S_EXPLOSION_BUILDING_BOOM].path_name = L"expl_bb";
    m_Sounds[S_EXPLOSION_BUILDING_BOOM2].path_name = L"expl_bb2";
    m_Sounds[S_EXPLOSION_BUILDING_BOOM3].path_name = L"expl_bb3";
    m_Sounds[S_EXPLOSION_BUILDING_BOOM4].path_name = L"expl_bb4";
    m_Sounds[S_EXPLOSION_ROBOT_BOOM].path_name = L"expl_rb";
    m_Sounds[S_EXPLOSION_ROBOT_BOOM_SMALL].path_name = L"expl_rbs";
    m_Sounds[S_EXPLOSION_BIGBOOM].path_name = L"expl_bigboom";
    m_Sounds[S_EXPLOSION_OBJECT].path_name = L"expl_obj";

    for(int i = 1; i <= ROBOT_HULLS_COUNT; ++i)
    {
        ESound added_sound = FindSoundByName(g_Config.m_RobotHullsConsts[i].hull_sound_name);
        if(added_sound == S_NONE)
        {
            g_Config.m_RobotHullsConsts[i].hull_sound_num = (ESound)m_Sounds.size();
            m_Sounds.emplace_back(SSoundItem(g_Config.m_RobotHullsConsts[i].hull_sound_name));
        }
        else g_Config.m_RobotHullsConsts[i].hull_sound_num = added_sound;
    }

    for(int i = 1; i <= ROBOT_CHASSIS_COUNT; ++i)
    {
        ESound added_sound = FindSoundByName(g_Config.m_RobotChassisConsts[i].arcade_enter_sound_name);
        if(added_sound == S_NONE)
        {
            g_Config.m_RobotChassisConsts[i].arcade_enter_sound_num = (ESound)m_Sounds.size();
            m_Sounds.emplace_back(SSoundItem(g_Config.m_RobotChassisConsts[i].arcade_enter_sound_name));
        }
        else g_Config.m_RobotChassisConsts[i].arcade_enter_sound_num = added_sound;
    }

    //ƒобавл€ем звуки дл€ оружи€
    for(int i = 1; i < g_Config.m_WeaponsConsts.size(); ++i)
    {
        if(g_Config.m_WeaponsConsts[i].shot_sound_name != L"")
        {
            ESound added_sound = FindSoundByName(g_Config.m_WeaponsConsts[i].shot_sound_name);
            if(added_sound == S_NONE)
            {
                g_Config.m_WeaponsConsts[i].shot_sound_num = (ESound)m_Sounds.size();
                m_Sounds.emplace_back(SSoundItem(g_Config.m_WeaponsConsts[i].shot_sound_name));
            }
            else g_Config.m_WeaponsConsts[i].shot_sound_num = added_sound;
        }

        if(g_Config.m_WeaponsConsts[i].hit_sound_name != L"")
        {
            ESound added_sound = FindSoundByName(g_Config.m_WeaponsConsts[i].hit_sound_name);
            if(added_sound == S_NONE)
            {
                g_Config.m_WeaponsConsts[i].hit_sound_num = (ESound)m_Sounds.size();
                m_Sounds.emplace_back(SSoundItem(g_Config.m_WeaponsConsts[i].hit_sound_name));
            }
            else g_Config.m_WeaponsConsts[i].hit_sound_num = added_sound;
        }

        if(g_Config.m_WeaponsConsts[i].map_objects_ignition.is_present)
        {
            ESound added_sound = FindSoundByName(g_Config.m_WeaponsConsts[i].map_objects_ignition.burning_sound_name);
            if(added_sound == S_NONE)
            {
                g_Config.m_WeaponsConsts[i].map_objects_ignition.burning_sound_num = (ESound)m_Sounds.size();
                m_Sounds.emplace_back(SSoundItem(g_Config.m_WeaponsConsts[i].map_objects_ignition.burning_sound_name));
            }
            else g_Config.m_WeaponsConsts[i].map_objects_ignition.burning_sound_num = added_sound;
        }
    }

#ifdef _DEBUG
    for(int i = 0; i < S_COUNT; ++i)
    {
        if(FLAG(m_Sounds[i].flags, SSoundItem::NOTINITED))
        {
            ERROR_S((CWStr(L"Sound ") + i + L" not initialized!").Get());
            //MessageBoxW(nullptr, (CWStr(L"Sound ") + i + L" not initialized!").Get(), L"Error", MB_ICONERROR);
            _asm int 3
        }
    }
#endif
}

struct SDS
{
    dword key;
    CSoundArray* sa;
    float ms;
}; 
static bool update_positions(dword key, dword val, dword user)
{
DTRACE();

    SDS* kk = (SDS*)user;
    CSoundArray* sa = (CSoundArray*)val;
    if(!sa->Len())
    {
        kk->key = key;
        kk->sa = sa;
        return true;
    }
    
    int x = (key & 0x1F) | ((key & 0x1FC00) >> 5);                    // 00000000000000111111110000011111 bits
    if((key & 0x020000) != 0) x = -x;

    int y = ((key & 0x3E0) >> 5) | ((key & 0x1FC0000) >> (8 + 5));    // 00000011111111000000001111100000 bits
    if((key & 0x02000000) != 0) y = -y;

    dword z = (key >> 26); // only positive
    D3DXVECTOR3 pos(float(x * SOUND_POS_DIVIDER), float(y * SOUND_POS_DIVIDER), float(z * SOUND_POS_DIVIDER));
    sa->UpdateTimings(kk->ms);
    sa->SetSoundPos(pos);
    return true;
}

void CSound::Tact()
{
DTRACE();

    static int nextsoundtakt_1;
    int delta = nextsoundtakt_1 - g_MatrixMap->GetTime();
    if(delta < 0 || delta > 100)
    {
        nextsoundtakt_1 = g_MatrixMap->GetTime() + 100;

        if(delta < 0) delta = 100;
        if(delta > 1000) delta = 1000;

        SDS sds;
        sds.ms = float(delta);
        sds.sa = nullptr;
        m_PosSounds->Enum(update_positions, (dword)&sds);

        if(sds.sa != nullptr)
        {
            HDelete(CSoundArray, sds.sa, g_MatrixHeap);
            m_PosSounds->Del(sds.key);
        }
    }

    static int nextsoundtakt;
    delta = nextsoundtakt - g_MatrixMap->GetTime();
    if(delta < 0 || delta > 1000)
    {
        nextsoundtakt = 1000 + g_MatrixMap->GetTime();

        if(g_RangersInterface)
        {
            /*
            for(int i = 1; i < SL_COUNT; ++i)
            {
                if(m_Layers[i] != SOUND_ID_EMPTY)
                {
                    if(!g_RangersInterface->m_SoundIsPlay(m_Layers[i]))
                    {
                        snd_destroy(m_Layers[i]);
                        m_Layers[i] = SOUND_ID_EMPTY;
                    }
                }
            }
            */

            for(int i = 0; i < MAX_SOUNDS; ++i)
            {
                if(m_AllSounds[i].id_internal != 0)
                {
                    if(!g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal))
                    {
                        snd_destroy(m_AllSounds[i].id_internal);

                        m_AllSounds[i].id_internal = 0;
                        m_AllSounds[i].id = SOUND_ID_EMPTY;
                        m_AllSounds[i].curvol = 0.0f;
                        m_AllSounds[i].curpan = 0.0f;
                    }
                }
            }
        }

        if(FLAG(g_Config.m_DIFlags, DI_ACTIVESOUNDS))
        {
            int sc = 0;
            for(int i = 0; i < MAX_SOUNDS; ++i)
            {
                if(m_AllSounds[i].id != SOUND_ID_EMPTY) ++sc;
            }
            //CDText::T("SND: ", CStr(sc));
            g_MatrixMap->m_DI.T(L"Active sounds: ", CWStr(sc));
        }
    }
}

void CSound::LayerOff(ESoundLayer sl)
{
DTRACE();

    ASSERT(g_RangersInterface);
    if(m_LayersI[sl].index >= 0 && m_LayersI[sl].index < MAX_SOUNDS)
    {
        if(m_LayersI[sl].id == m_AllSounds[m_LayersI[sl].index].id)
        {
            StopPlayInternal(m_LayersI[sl].index);
        }
        m_AllSounds[m_LayersI[sl].index].id = SOUND_ID_EMPTY;
        m_AllSounds[m_LayersI[sl].index].id_internal = 0;
    }
}

void CSound::SureLoaded(ESound snd)
{
DTRACE();

    if(g_RangersInterface)
    {
        if(!FLAG(m_Sounds[snd].flags, SSoundItem::LOADED))
        {
            //load sound
            CBlockPar* bps = g_MatrixData->BlockGet(L"Sounds");

            CWStr temp(m_Sounds[snd].SoundPath().Get(), g_CacheHeap);
            CBlockPar* bp = bps->BlockGetNE(temp);
            if(bp == nullptr) bp = bps->BlockGetNE(L"dummy");

            m_Sounds[snd].ttl = 1E30f;
            m_Sounds[snd].fadetime = 1000;
            m_Sounds[snd].pan0 = 0;
            m_Sounds[snd].pan1 = 0;
            m_Sounds[snd].vol0 = 1;
            m_Sounds[snd].vol1 = 1;

            if(bp->ParCount(L"pan"))
            {
                m_Sounds[snd].pan0 = bp->ParGet(L"pan").GetFloatPar(0, L",");
                m_Sounds[snd].pan1 = bp->ParGet(L"pan").GetFloatPar(1, L",");
            }

            if(bp->ParCount(L"vol"))
            {
                m_Sounds[snd].vol0 = bp->ParGet(L"vol").GetFloatPar(0, L",");
                m_Sounds[snd].vol1 = bp->ParGet(L"vol").GetFloatPar(1, L",");
            }

            if(bp->ParCount(L"looped"))
            {
                bool looped = 0 != bp->ParGet(L"looped").GetInt();
                INITFLAG(m_Sounds[snd].flags, SSoundItem::LOOPED, looped);
            }
            else RESETFLAG(m_Sounds[snd].flags, SSoundItem::LOOPED);

            if(bp->ParCount(L"ttl"))
            {
                m_Sounds[snd].ttl = bp->ParGet(L"ttl").GetFloatPar(0, L",");
                m_Sounds[snd].fadetime = bp->ParGet(L"ttl").GetFloatPar(1, L",");
            }

            if(bp->ParCount(L"attn"))
            {
                m_Sounds[snd].attn = (0.002 * bp->ParGet(L"attn").GetFloat());
                if(m_Sounds[snd].attn == 0) m_Sounds[snd].radius = 1E15f;
                else m_Sounds[snd].radius = 1.0f / m_Sounds[snd].attn;
            }
            else
            {
                m_Sounds[snd].attn = 0.002f;
                m_Sounds[snd].radius = 1.0f / 0.002f;
            }

            m_Sounds[snd].SoundPath() = bp->ParGet(L"path");
            SETFLAG(m_Sounds[snd].flags, SSoundItem::LOADED);
        }
    }
}

/*
void CSound::ExtraRemove(void)
{
    float minv = 100;
    int deli = -1;
    for(int i = 0; i < MAX_SOUNDS; ++i)
    {
        if(!g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal))
        {
            snd_destroy(m_AllSounds[i].id_internal);

            m_AllSounds[i].id_internal = 0;
            m_AllSounds[i].id = SOUND_ID_EMPTY;
            m_AllSounds[i].curvol = 0.0f;
            m_AllSounds[i].curpan = 0.0f;

            return;
        }
        else
        {
            if(m_AllSounds[i].curvol < minv)
            {
                minv = m_AllSounds[i].curvol;
                deli = i;
            }
        }
    }

    StopPlayInternal(deli);
}
*/

void CSound::StopPlayInternal(int deli)
{
DTRACE();

    //snd_vol(m_AllSounds[deli].id_internal, 0);
    snd_destroy(m_AllSounds[deli].id_internal);

    m_AllSounds[deli].id_internal = 0;
    m_AllSounds[deli].id = SOUND_ID_EMPTY;
    m_AllSounds[deli].curvol = 0.0f;
    m_AllSounds[deli].curpan = 0.0f;
}

int CSound::FindSoundSlot(dword id)
{
DTRACE();

    for(int i = 0; i < MAX_SOUNDS; ++i)
    {
        if(m_AllSounds[i].id == id) return i;
    }
    return -1;
}

int CSound::FindSoundSlotPlayedOnly(dword id)
{
DTRACE();

    int i = FindSoundSlot(id);
    if(i >= 0)
    {
        if(g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal)) return i;
        snd_destroy(m_AllSounds[i].id_internal);

        m_AllSounds[i].id_internal = 0;
        m_AllSounds[i].id = SOUND_ID_EMPTY;
        m_AllSounds[i].curvol = 0.0f;
        m_AllSounds[i].curpan = 0.0f;
    }

    return -1;
}

int CSound::FindSlotForSound(void)
{
DTRACE();

    float minv = 100;
    int deli = -1;
    for(int i = 0; i < MAX_SOUNDS; ++i)
    {
        if(m_AllSounds[i].id_internal == 0) return i;

        if(m_AllSounds[i].id == SOUND_ID_EMPTY)
        {
            StopPlayInternal(i);
            return i;
        }

        if(!g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal))
        {
            snd_destroy(m_AllSounds[i].id_internal);

            m_AllSounds[i].id_internal = 0;
            m_AllSounds[i].id = SOUND_ID_EMPTY;
            m_AllSounds[i].curvol = 0.0f;
            m_AllSounds[i].curpan = 0.0f;

            return i;
        }
        else
        {
            if(m_AllSounds[i].curvol < minv)
            {
                minv = m_AllSounds[i].curvol;
                deli = i;
            }
        }
    }

    StopPlayInternal(deli);
    return deli;
}

dword CSound::Play(const wchar* name, const D3DXVECTOR3& pos, ESoundLayer sl, ESoundInterruptFlag interrupt)
{
DTRACE();

    if(!g_RangersInterface) return SOUND_ID_EMPTY;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].SoundPath().Set(name);

    return Play(S_SPECIAL_SLOT, pos, sl, interrupt);
}

dword CSound::Play(const wchar* name, ESoundLayer sl, ESoundInterruptFlag interrupt)
{
DTRACE();

    if(!g_RangersInterface) return SOUND_ID_EMPTY;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].SoundPath().Set(name);

    return Play(S_SPECIAL_SLOT, sl, interrupt);
}

dword CSound::Play(const D3DXVECTOR3& pos, float attn, float pan0, float pan1, float vol0, float vol1, wchar* name)
{
DTRACE();

    if(!g_RangersInterface) return SOUND_ID_EMPTY;

    int si = FindSlotForSound();

    float pan, vol;
    CalcPanVol(pos, attn, 
        pan0,
        pan1,
        vol0,
        vol1,
        &pan, &vol);

    if(vol < 0.00001f) return SOUND_ID_EMPTY;

    m_AllSounds[si].id_internal = snd_create(name, m_LastGroup++, 0);
    m_AllSounds[si].id = m_LastID++;

    snd_pan(m_AllSounds[si].id_internal, pan);
    snd_vol(m_AllSounds[si].id_internal, vol);

    //try 
    //{
    snd_play(m_AllSounds[si].id_internal);

    /*
    } catch (...)
    {
        ERROR_S(L"Problem with sound: " + CWStr(name));
    }
    */

    m_AllSounds[si].curpan = pan;
    m_AllSounds[si].curvol = vol;

    return m_AllSounds[si].id;
}

bool CSound::IsSoundPlay(dword id)
{
DTRACE();

    if(g_RangersInterface)
    {
        for(int i = 0; i < MAX_SOUNDS; ++i)
        {
            if(m_AllSounds[i].id == id)
            {
                if(g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal)) return true;
                //snd_vol(m_AllSounds[i].id_internal, 0);
                snd_destroy(m_AllSounds[i].id_internal);

                m_AllSounds[i].id_internal = 0;
                m_AllSounds[i].id = SOUND_ID_EMPTY;
                m_AllSounds[i].curvol = 0.0f;
                m_AllSounds[i].curpan = 0.0f;

                return false;
            }
        }
    }
    return false;
}

bool CSound::SLID::IsPlayed(void)
{
DTRACE();

    if(index >= 0 && index < MAX_SOUNDS)
    {
        if(CSound::m_AllSounds[index].id == id)
        {
            return g_RangersInterface->m_SoundIsPlay(CSound::m_AllSounds[index].id_internal) != 0;
        }
    }
    return false;
}

dword CSound::PlayInternal(ESound snd, float vol, float pan, ESoundLayer sl, ESoundInterruptFlag interrupt)
{
DTRACE();

    if(g_RangersInterface)
    {
        if(vol < 0.00001f) return SOUND_ID_EMPTY;

//#if defined _TRACE || defined _DEBUG
//    try 
//    {
//#endif
        dword newid = m_LastID++;

        if(sl != SL_ALL)
        {
            if(interrupt == SEF_SKIP && m_LayersI[sl].IsPlayed()) return SOUND_ID_EMPTY;
            LayerOff(sl);
            m_LayersI[sl].id = newid;
        }

        int si = FindSlotForSound();
        m_AllSounds[si].id_internal = snd_create(m_Sounds[snd].SoundPath().GetBuf(), m_LastGroup++, FLAG(m_Sounds[snd].flags, SSoundItem::LOOPED));
        m_AllSounds[si].id = newid;

        m_AllSounds[si].curpan = pan;
        m_AllSounds[si].curvol = vol;

        snd_pan(m_AllSounds[si].id_internal, pan);
        snd_vol(m_AllSounds[si].id_internal, vol);

        snd_play(m_AllSounds[si].id_internal);

        return newid;

//#if defined _TRACE || defined _DEBUG
//    } catch (...)
//    {
//        ERROR_S(L"Problem with sound: " + m_Sounds[snd].Path());
//    }
//#endif
    }
    else return SOUND_ID_EMPTY;
}

dword CSound::Play(ESound snd, ESoundLayer sl, ESoundInterruptFlag interrupt)
{
    if(g_RangersInterface)
    {
        SureLoaded(snd);
        return PlayInternal(snd,
            (float)RND(m_Sounds[snd].vol0, m_Sounds[snd].vol1),
            (float)RND(m_Sounds[snd].pan0, m_Sounds[snd].pan1),
            sl,interrupt);
    }
    else return SOUND_ID_EMPTY;
}

dword CSound::Play(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag interrupt)
{
    if(g_RangersInterface)
    {
        SureLoaded(snd);

        float pan, vol;
        CalcPanVol(pos, m_Sounds[snd].attn, 
            m_Sounds[snd].pan0,
            m_Sounds[snd].pan1,
            m_Sounds[snd].vol0,
            m_Sounds[snd].vol1,
            &pan, &vol);

        return PlayInternal(snd, vol, pan, sl, interrupt);\
    }
    else return SOUND_ID_EMPTY;
}

void CSound::CalcPanVol(const D3DXVECTOR3& pos, float attn, float pan0, float pan1, float vol0, float vol1, float* pan, float* vol)
{
    D3DXVECTOR3 dir(pos - g_MatrixMap->m_Camera.GetFrustumCenter());
    float dist = D3DXVec3Length(&dir);

    if(dist != 0.0f) dir *= (1.0f / dist);
    dist -= SOUND_FULL_VOLUME_DIST;

    if(dist < 0) dist = 0; // close enough to be at full volume
    dist *= attn; // different attenuation levels

    if(pan)
    {
        float dot = D3DXVec3Dot(&g_MatrixMap->m_Camera.GetRight(), &dir);
        *pan = LERPFLOAT((dot + 1) / 2.0f, pan0, pan1);
    }

    if(vol)
    {
        float k = (1.0f - dist);
        if(k < 0) k = 0;
        *vol = LERPFLOAT(k, vol0, vol1);
    }
}

dword CSound::Play(dword id, ESound snd, const D3DXVECTOR3& pos, ESoundLayer sl, ESoundInterruptFlag interrupt)
{
    if(g_RangersInterface)
    {
        SureLoaded(snd);

        float pan, vol;
        CalcPanVol(pos, m_Sounds[snd].attn, 
            m_Sounds[snd].pan0,
            m_Sounds[snd].pan1,
            m_Sounds[snd].vol0,
            m_Sounds[snd].vol1,
            &pan, &vol);

        int idx = FindSoundSlotPlayedOnly(id);
        if(idx >= 0)
        {
            // already played
            snd_pan(m_AllSounds[idx].id_internal, pan);
            snd_vol(m_AllSounds[idx].id_internal, vol);

            m_AllSounds[idx].curpan = pan;
            m_AllSounds[idx].curvol = vol;
        }
        else id = PlayInternal(snd, vol, pan, sl, interrupt);
        return id;
    }
    else return SOUND_ID_EMPTY;
}

dword CSound::ChangePos(dword id, ESound snd, const D3DXVECTOR3& pos)
{
    if(g_RangersInterface)
    {
        SureLoaded(snd);

        float pan, vol;
        CalcPanVol(pos, m_Sounds[snd].attn, 
            m_Sounds[snd].pan0,
            m_Sounds[snd].pan1,
            m_Sounds[snd].vol0,
            m_Sounds[snd].vol1,
            &pan, &vol);

        int idx = FindSoundSlotPlayedOnly(id);
        if(idx >= 0)
        {
            // already played
            snd_pan(m_AllSounds[idx].id_internal, pan);
            snd_vol(m_AllSounds[idx].id_internal, vol);

            m_AllSounds[idx].curpan = pan;
            m_AllSounds[idx].curvol = vol;
        }
        else id = SOUND_ID_EMPTY;
        return id;
    }
    else return SOUND_ID_EMPTY; 
}

void CSound::StopPlayAllSounds(void)
{
    if(g_RangersInterface)
    {
        for(int i = 0; i < MAX_SOUNDS; ++i) StopPlayInternal(i);
    }
}

void CSound::StopPlay(dword id)
{
    if(id == SOUND_ID_EMPTY) return;
    if(g_RangersInterface)
    {
        //g_MatrixMap->m_DI.T(CWStr(L"sndoff") + (int)id, L"");

        int idx = FindSoundSlotPlayedOnly(id);
        if(idx >= 0) StopPlayInternal(idx);
    }
}

__forceinline  dword CSound::Pos2Key(const D3DXVECTOR3& pos)
{
    int x = Float2Int(pos.x / SOUND_POS_DIVIDER);
    int y = Float2Int(pos.y / SOUND_POS_DIVIDER);
    int z = Float2Int(pos.z / SOUND_POS_DIVIDER);

    dword key = 0;

    if(x < 0)
    {
        x = -x;
        key |= 0x020000;
    }

    if(x > 4095) x = 4095;

    if(y < 0)
    {
        y = -y;
        key |= 0x02000000;
    }

    if(y > 4095) y = 4095;

    if(z < 0) z = 0;
    else if(z > 63) z = 63;

    key |= (x & 31) | ( (x & 4064) << 5 );
    key |= ((y & 31) << 5) | ( (y & 4064) << (8 + 5) );
    key |= ((dword)z) << 26;

    return key;
}

void CSound::AddSound(ESound snd, const D3DXVECTOR3& pos, ESoundLayer sl, ESoundInterruptFlag ifl) //automatic position
{
    if(!g_RangersInterface) return;
    dword key = Pos2Key(pos);

    CSoundArray* sa;
    if(!m_PosSounds->Get(key, (dword*)&sa))
    {
        sa = HNew(g_MatrixHeap) CSoundArray(g_MatrixHeap);
        m_PosSounds->Set(key, (dword)sa);
    }
    sa->AddSound(snd, pos, sl, ifl);
}

void CSound::AddSound(const wchar* name, const D3DXVECTOR3& pos)
{
    if(!g_RangersInterface) return;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].SoundPath().Set(name);
    SureLoaded(S_SPECIAL_SLOT);

    AddSound(pos, m_Sounds[S_SPECIAL_SLOT].attn,
                  m_Sounds[S_SPECIAL_SLOT].pan0,
                  m_Sounds[S_SPECIAL_SLOT].pan1,
                  m_Sounds[S_SPECIAL_SLOT].vol0,
                  m_Sounds[S_SPECIAL_SLOT].vol1,
                  m_Sounds[S_SPECIAL_SLOT].SoundPath().GetBuf());
}

void CSound::AddSound(
    const D3DXVECTOR3& pos,
    float attn,
    float pan0,
    float pan1,
    float vol0,
    float vol1,
    wchar* name
)
{
    if(!g_RangersInterface) return;
    dword key = Pos2Key(pos);

    CSoundArray* sa;
    if(!m_PosSounds->Get(key, (dword*)&sa))
    {
        sa = HNew(g_MatrixHeap) CSoundArray(g_MatrixHeap);
        m_PosSounds->Set(key, (dword)sa);
    }

    sa->AddSound(pos, attn, pan0, pan1, vol0, vol1, name);
}

static bool delete_arrays(dword key, dword val, dword user)
{
    CSoundArray* sa = (CSoundArray*)val;
    HDelete(CSoundArray, sa, g_MatrixHeap);
    return true;
}

void CSound::Clear(void)
{
    if(g_RangersInterface)
    {
        for(int i = 0; i < MAX_SOUNDS; ++i)
        {
            if(m_AllSounds[i].id_internal != 0)
            {
                //snd_vol(m_AllSounds[i].id_internal, 0);
                snd_destroy(m_AllSounds[i].id_internal);

                m_AllSounds[i].id_internal = 0;
                m_AllSounds[i].id = SOUND_ID_EMPTY;
                m_AllSounds[i].curvol = 0.0f;
                m_AllSounds[i].curpan = 0.0f;
            }
        }
    }
    m_PosSounds->Enum(delete_arrays, 0);

    m_Sounds.clear();

    HDelete(CDWORDMap, m_PosSounds, g_MatrixHeap);
}

void CSoundArray::UpdateTimings(float ms)
{
DTRACE();

    if(g_RangersInterface)
    {
        SSndData* sb = Buff<SSndData>();
        SSndData* se = BuffEnd<SSndData>();
        for(; sb < se;)
        {
            if(sb->snd == S_UNDEF)
            {
                ++sb;
                continue;
            }

            int idx = CSound::FindSoundSlotPlayedOnly(sb->id);
            if(idx >= 0)
            {
                if(sb->ttl < 0)
                {
                    if(sb->fade < 0)
                    {
                        if(sb->id == 1) _asm int 3

                        CSound::StopPlayInternal(idx);
                        goto del;
                    }

                    sb->fade -= ms;
                }
                else
                {
                    sb->ttl -= ms;
                    if(sb->ttl < 0) sb->ttl = -sb->fade;
                }
            }
            else
            {
del:
                *sb = *(--se);
                SetLenNoShrink(Len()-sizeof(SSndData));
                continue;
            }

            ++sb;
        }
    }
    else SetLenNoShrink(0);
}

void CSoundArray::SetSoundPos(const D3DXVECTOR3& pos)
{
    if(g_RangersInterface)
    {
        SSndData* sb = Buff<SSndData>();
        SSndData* se = BuffEnd<SSndData>();
        for(; sb < se;)
        {
            int idx = CSound::FindSoundSlotPlayedOnly(sb->id);
            if(idx >= 0)
            {
                float k = 1.0f;
                if(sb->ttl < 0 && sb->snd != S_UNDEF)
                {
                    k = -sb->fade / sb->ttl;
                }

                float pan, vol;
                CSound::CalcPanVol(pos, sb->attn, 
                    sb->pan0,
                    sb->pan1,
                    sb->vol0,
                    sb->vol1,
                    &pan, &vol);

                vol *= k;
                if(vol < 0.00001f)
                {
                    CSound::StopPlayInternal(idx);
                    goto dele;
                }

                snd_pan(CSound::m_AllSounds[idx].id_internal, pan);
                snd_vol(CSound::m_AllSounds[idx].id_internal, vol);

                CSound::m_AllSounds[idx].curpan = pan;
                CSound::m_AllSounds[idx].curvol = vol;

                //g_MatrixMap->m_DI.T(CWStr(vol).Get(), L"1212");
            }
            else
            {
dele:
                *sb = *(--se);
                SetLenNoShrink(Len()-sizeof(SSndData));
                continue;
            }

            ++sb;
        }
    }
    else SetLenNoShrink(0);
}

void CSoundArray::AddSound(ESound snd, const D3DXVECTOR3& pos, ESoundLayer sl, ESoundInterruptFlag ifl)
{
    SSndData* sb = Buff<SSndData>();
    SSndData* se = BuffEnd<SSndData>();
    for(; sb < se; ++sb)
    {
        if(sb->snd == snd)
        {
            if(ifl == SEF_INTERRUPT)
            {
                CSound::StopPlay(sb->id);
                (*sb) = *(--se);
                SetLenNoShrink(Len()-sizeof(SSndData));
                break;
            }

            if(!CSound::IsSoundPlay(sb->id))
            {
                (*sb) = *(--se);
                SetLenNoShrink(Len()-sizeof(SSndData));
                break;
            }

            // oops. the same sound
            // only set ttl and fade
            sb->ttl = CSound::m_Sounds[snd].ttl;
            sb->fade = CSound::m_Sounds[snd].fadetime;
            return;
        }

    }

    dword id = CSound::Play(snd, pos, sl, ifl);
    if(id == SOUND_ID_EMPTY) return;

//#ifdef _DEBUG
//    if(snd == S_WEAPON_HIT_ABLAZE)
//    {
//        DCNT("Ablaze!");
//    }
//#endif

    Expand(sizeof(SSndData));

    (BuffEnd<SSndData>() - 1)->snd = snd;
    (BuffEnd<SSndData>() - 1)->id = id;
    (BuffEnd<SSndData>() - 1)->pan0 = CSound::m_Sounds[snd].pan0;
    (BuffEnd<SSndData>() - 1)->pan1 = CSound::m_Sounds[snd].pan1;
    (BuffEnd<SSndData>() - 1)->vol0 = CSound::m_Sounds[snd].vol0;
    (BuffEnd<SSndData>() - 1)->vol1 = CSound::m_Sounds[snd].vol1;
    (BuffEnd<SSndData>() - 1)->attn = CSound::m_Sounds[snd].attn;
    (BuffEnd<SSndData>() - 1)->ttl = CSound::m_Sounds[snd].ttl;
    (BuffEnd<SSndData>() - 1)->fade = CSound::m_Sounds[snd].fadetime;
    
    return;
}

void CSound::SaveSoundLog(void)
{
    DTRACE();

    CBuf    b(g_CacheHeap);
    b.StrNZ("Sounds:\n");

    for(int i = 0; i < MAX_SOUNDS; ++i)
    {
        CStr ss(g_CacheHeap);

        ss = i;
        ss += " - id:";
        ss += int(m_AllSounds[i].id);

        ss += ", idi:";
        ss += int(m_AllSounds[i].id_internal);
        
        ss += ", vol:";
        ss += m_AllSounds[i].curvol;

        ss += ", pan:";
        ss += m_AllSounds[i].curpan;

        ss += ", rvol:";
        ss += g_RangersInterface->m_SoundGetVolume(m_AllSounds[i].id_internal);

        ss += ", is_play:";
        ss += g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal);
        ss += "\n";

        b.StrNZ(ss);

    }

    b.SaveInFile(L"log_sounds.txt");
}

