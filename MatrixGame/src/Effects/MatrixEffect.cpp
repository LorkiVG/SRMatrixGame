// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Gamesvector
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../stdafx.h"
#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include "../MatrixObject.hpp"
#include "../MatrixRenderPipeline.hpp"
#include "../MatrixSkinManager.hpp"
#include <math.h>

#define M_PI 3.14159265358979323846

float g_WeaponDamageNormalCoef = 1.0f;
float g_WeaponDamageArcadeCoef = 1.2f;
float g_UnitSpeedArcadeCoef = 1.1f;

dword        CMatrixEffect::m_before_draw_done;
CDebris*     CMatrixEffect::m_Debris;
int          CMatrixEffect::m_DebrisCnt;

std::vector<SSpotProperties> CMatrixEffect::m_SpotProperties = std::vector<SSpotProperties>(int(SPOT_TYPES_CNT));
std::vector<SSpriteTextureArrayElement> CMatrixEffect::m_SpriteTextures = std::vector<SSpriteTextureArrayElement>(int(SPR_LAST));

float        CMatrixEffect::m_Dist2;    // used by movin objects (missiles cannons)

Base::CHeap* CMatrixEffect::m_Heap;

ELIST_DECLARE_OUTCLASS(EFFECT_EXPLOSION);
ELIST_DECLARE_OUTCLASS(EFFECT_POINT_LIGHT);
ELIST_DECLARE_OUTCLASS(EFFECT_SMOKE);
ELIST_DECLARE_OUTCLASS(EFFECT_LANDSCAPE_SPOT);
ELIST_DECLARE_OUTCLASS(EFFECT_MOVETO);
ELIST_DECLARE_OUTCLASS(EFFECT_FIREANIM);


//////////////////////////////
//////////////////////////////

CEffectSpawner::CEffectSpawner(int minperiod, int maxperiod, int ttl, SpawnEffectProps* props) :
    CMain(), m_MIN_period(minperiod), m_MAX_period(maxperiod), m_NextTime(Double2Int(g_MatrixMap->GetTime() + (RND(minperiod, maxperiod)))),
    m_Under(TRACE_STOP_NONE), m_TTL((ttl > 1) ? (g_MatrixMap->GetTime() + ttl) : 0)
{
    m_Props = (SpawnEffectProps*)HAlloc(props->m_Size, CMatrixEffect::m_Heap);
    memcpy(m_Props, props, props->m_Size);
}

void SpawnEffectLighteningDestructor(SpawnEffectProps* props)
{
    SpawnEffectLightening* li = (SpawnEffectLightening*)props;
    if(li->m_Pair)
    {
        li->m_Pair->m_Pair = nullptr;
#ifdef _DEBUG
        li->m_Effect.Release(DEBUG_CALL_INFO);
#else
        li->m_Effect.Release();
#endif
    }
}

CEffectSpawner::~CEffectSpawner()
{
    if(m_Props->m_Destructor) m_Props->m_Destructor(m_Props);
    HFree(m_Props, CMatrixEffect::m_Heap);
}

struct FOwner
{
    float dist;
    CMatrixMapStatic** target;
};

static bool FindOwner(const D3DXVECTOR3& center, CMatrixMapStatic* ms, dword user)
{
    FOwner* d = (FOwner*)user;

    D3DXVECTOR3 bmin, bmax;

    ms->CalcBounds(bmin, bmax);
    D3DXVECTOR3 pos = (bmin + bmax) * 0.5f;

    if(center.x < bmin.x) return true;
    if(center.y < bmin.y) return true;
    if(center.z < bmin.z) return true;

    if(center.x > bmax.x) return true;
    if(center.y > bmax.y) return true;
    if(center.z > bmax.z) return true;

    D3DXVECTOR3 temp = center - pos;
    float di = D3DXVec3LengthSq(&temp);
    if(di < d->dist)
    {
        d->dist = di;
        *d->target = ms;
    }
    return true;
}


void CEffectSpawner::Tact(float)
{
    if(m_Under == nullptr)
    {
        FOwner data;
        data.dist = 100 * 100;
        data.target = &m_Under;

        g_MatrixMap->FindObjects(m_Props->m_Pos, 100, 1.0f, TRACE_BUILDING | TRACE_OBJECT, nullptr, FindOwner, (dword)&data);
        if(m_Under == nullptr) m_Under = TRACE_STOP_LANDSCAPE;
    }

    if(g_MatrixMap->GetTime() > m_NextTime)
    {
        m_NextTime = g_MatrixMap->GetTime() + Double2Int(RND(m_MIN_period, m_MAX_period));
        m_Props->m_Spawner(m_Props);
    }
}

bool CEffectSpawner::OutOfTime() const
{
    return (m_TTL > 1) && (g_MatrixMap->GetTime() > m_TTL);
}

void EffectSpawnerFire(SpawnEffectProps* props)
{
    SpawnEffectFire* fire = (SpawnEffectFire*)props;
    CMatrixEffect::CreateFire(nullptr, fire->m_Pos, fire->m_ttl, fire->m_puffttl, fire->m_spawntime, fire->m_dispfactor, fire->m_IsBright, fire->m_speed);
}
void EffectSpawnerSmoke(SpawnEffectProps *props)
{
    SpawnEffectSmoke* smoke = (SpawnEffectSmoke*)props;
    CMatrixEffect::CreateSmoke(nullptr, smoke->m_Pos, smoke->m_ttl, smoke->m_puffttl, smoke->m_spawntime, smoke->m_color, smoke->m_IsBright, smoke->m_speed);
}
void EffectSpawnerSound(SpawnEffectProps* props)
{
    SpawnEffectSound* sound = (SpawnEffectSound*)props;
    CSound::AddSound(sound->m_Pos, sound->m_attn * 0.002f, sound->m_pan0, sound->m_pan1, sound->m_vol0, sound->m_vol1, sound->m_name);
}
void EffectSpawnerLightening(SpawnEffectProps* props)
{
    SpawnEffectLightening* li = (SpawnEffectLightening*)props;

    ASSERT(li->m_Width > 0);    // make sure it has pair (editor must save this effect correctly)

    if(li->m_Pair && li->m_Dispers >= 0)
    {
        // ok, effect has pair and its host
        SpawnEffectLightening* l = (FRND(1) < 0.5f) ? li : li->m_Pair;

        CMatrixEffect::CreateLightening(&li->m_Effect, li->m_Pos, li->m_Pair->m_Pos, l->m_ttl, li->m_Dispers, l->m_Width, l->m_Color);
    }
}


//////////////////////////////
//////////////////////////////

void CMatrixEffect::ClearEffects()
{
DTRACE();

    if(m_Debris)
    {
        if(m_DebrisCnt)
        {
            for(int i = 0; i < m_DebrisCnt; ++i) m_Debris[i].~CDebris();
            m_DebrisCnt = 0;
        }

        HFree(m_Debris, m_Heap);
        m_Debris = nullptr;
    }

    CSprite::Deinit();
    CMatrixEffectPointLight::ClearAll();

    //Также очищаем всё лишнее сверх базового размера из массивов следов и спрайтов
    if(m_SpotProperties.size() > SPOT_TYPES_CNT) m_SpotProperties.erase(m_SpotProperties.begin() + SPOT_TYPES_CNT, m_SpotProperties.end());
    if(m_SpriteTextures.size() > SPR_LAST) m_SpriteTextures.erase(m_SpriteTextures.begin() + SPR_LAST, m_SpriteTextures.end());

    if(m_Heap)
    {
        HDelete(CHeap, m_Heap, nullptr);
        m_Heap = nullptr;
    }
}

void CMatrixEffect::CreatePoolDefaultResources(void)
{
DTRACE();

    CSprite::Init();
}

void CMatrixEffect::ReleasePoolDefaultResources(void)
{
DTRACE();

    CSprite::Deinit();

    CMatrixEffectBigBoom::MarkAllBuffersNoNeed();
    CMatrixEffectBillboard::MarkAllBuffersNoNeed();
    CMatrixEffectKonus::MarkAllBuffersNoNeed();
    CMatrixEffectLandscapeSpot::MarkAllBuffersNoNeed();
    CMatrixEffectPointLight::MarkAllBuffersNoNeed();
}

void CMatrixEffect::InitEffects(CBlockPar& bp_in)
{
DTRACE();

    //Static Init
    CMatrixEffectBigBoom::StaticInit();
    CMatrixEffectBillboard::StaticInit();
    CMatrixEffectKonus::StaticInit();
    CMatrixEffectLandscapeSpot::StaticInit();
    CMatrixEffectMoveto::StaticInit();
    CMatrixEffectPointLight::StaticInit();

    ELIST_INIT(EFFECT_EXPLOSION);
    ELIST_INIT(EFFECT_POINT_LIGHT);
    ELIST_INIT(EFFECT_SMOKE);
    ELIST_INIT(EFFECT_LANDSCAPE_SPOT);
    ELIST_INIT(EFFECT_MOVETO);
    ELIST_INIT(EFFECT_FIREANIM);

    m_Heap = HNew(nullptr) CHeap();

    // init weapon range modificators

    g_WeaponDamageNormalCoef = bp_in.BlockGet(BLOCK_PATH_MAIN_CONFIG)->ParGet(L"WeaponDamageNormalCoef").GetDouble();
    g_WeaponDamageArcadeCoef = bp_in.BlockGet(BLOCK_PATH_MAIN_CONFIG)->ParGet(L"WeaponDamageArcadeCoef").GetDouble();
    g_UnitSpeedArcadeCoef = bp_in.BlockGet(BLOCK_PATH_MAIN_CONFIG)->ParGet(L"UnitSpeedArcadeCoef").GetDouble();

    // init debris
    CBlockPar& bp = *bp_in.BlockGet(L"Models")->BlockGet(L"Debris");

    m_DebrisCnt = bp.ParCount();
    m_Debris = (CDebris*)HAlloc(sizeof(CDebris) * m_DebrisCnt, m_Heap);

    for(int i = 0; i < m_DebrisCnt; ++i)
    {
        m_Debris[i] = CDebris();

        CVectorObject* vo = (CVectorObject*)g_Cache->Get(cc_VO, bp.ParGet(i).Get());
        vo->PrepareSpecial(OLF_AUTO, CSkinManager::GetSkin, GSP_ORDINAL);
        m_Debris[i].Init(vo, nullptr);

        int n = bp.ParGetName(i).GetStrPar(0, L",").GetInt();
        m_Debris[i].SetDebType(n);
    }

    //CBlockPar bp_spot_props(false); //В сам массив пихать имена смысла нет, так что используем временный BlockPar

    /*
    bp_spot_props.ParAdd(TEXTURE_PATH_SPOT, (CWStr)SPOT_CONSTANT);
    bp_spot_props.ParAdd(TEXTURE_PATH_OBJSEL, (CWStr)SPOT_SELECTION);
    bp_spot_props.ParAdd(TEXTURE_PATH_HIT, (CWStr)SPOT_PLASMA_HIT);
    //bp_spot_props.ParAdd(TEXTURE_PATH_MOVETO, (CWStr)SPOT_MOVE_TO);
    bp_spot_props.ParAdd(TEXTURE_PATH_POINTLIGHT, (CWStr)SPOT_POINTLIGHT);
    bp_spot_props.ParAdd(TEXTURE_PATH_VORONKA, (CWStr)SPOT_VORONKA);
    bp_spot_props.ParAdd(TEXTURE_PATH_TURRET_RADIUS, (CWStr)SPOT_TURRET);
    */

    //init spots
    m_SpotProperties[SPOT_CONSTANT].color = 0xFF000000;
    m_SpotProperties[SPOT_CONSTANT].func = CMatrixEffectLandscapeSpot::SpotTactConstant;
    m_SpotProperties[SPOT_CONSTANT].texture = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPOT);
    m_SpotProperties[SPOT_CONSTANT].ttl = 15000;
    m_SpotProperties[SPOT_CONSTANT].flags = 0;

    m_SpotProperties[SPOT_SELECTION].color = 0x80808080;
    m_SpotProperties[SPOT_SELECTION].func = CMatrixEffectLandscapeSpot::SpotTactAlways;
    m_SpotProperties[SPOT_SELECTION].texture = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_OBJSEL);
    m_SpotProperties[SPOT_SELECTION].texture->MipmapOff();
    m_SpotProperties[SPOT_SELECTION].ttl = 0;
    m_SpotProperties[SPOT_SELECTION].flags = 0;

    m_SpotProperties[SPOT_PLASMA_HIT].color = 0xFFFFFFFF;
    m_SpotProperties[SPOT_PLASMA_HIT].func = CMatrixEffectLandscapeSpot::SpotTactPlasmaHit;
    m_SpotProperties[SPOT_PLASMA_HIT].texture = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_HIT);
    m_SpotProperties[SPOT_PLASMA_HIT].ttl = 3000;
    m_SpotProperties[SPOT_PLASMA_HIT].flags = LSFLAG_SCALE_BY_NORMAL;

    //m_SpotProperties[SPOT_MOVE_TO].color = 0x80808080;
    //m_SpotProperties[SPOT_MOVE_TO].func = CMatrixEffectLandscapeSpot::SpotTactMoveTo;
    //m_SpotProperties[SPOT_MOVE_TO].texture = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_MOVETO);
    //m_SpotProperties[SPOT_MOVE_TO].ttl = 1500;
    //m_SpotProperties[SPOT_MOVE_TO].flags = LSFLAG_SCALE_BY_NORMAL;

    m_SpotProperties[SPOT_POINTLIGHT].color = 0xFFFFFFFF;
    m_SpotProperties[SPOT_POINTLIGHT].func = CMatrixEffectLandscapeSpot::SpotTactPointlight;
    m_SpotProperties[SPOT_POINTLIGHT].texture = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_POINTLIGHT);
    m_SpotProperties[SPOT_POINTLIGHT].ttl = 1500;
    m_SpotProperties[SPOT_POINTLIGHT].flags = LSFLAG_INTENSE;

    m_SpotProperties[SPOT_VORONKA].color = 0x00FFFFFF;
    m_SpotProperties[SPOT_VORONKA].func = CMatrixEffectLandscapeSpot::SpotTactVoronka;
    m_SpotProperties[SPOT_VORONKA].texture = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_VORONKA);
    m_SpotProperties[SPOT_VORONKA].ttl = 30000;
    m_SpotProperties[SPOT_VORONKA].flags = 0;

    m_SpotProperties[SPOT_TURRET].color = 0x80FFFFFF;
    m_SpotProperties[SPOT_TURRET].func = CMatrixEffectLandscapeSpot::SpotTactAlways;
    m_SpotProperties[SPOT_TURRET].texture = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_TURRET_RADIUS);
    m_SpotProperties[SPOT_TURRET].texture->MipmapOff();
    m_SpotProperties[SPOT_TURRET].ttl = 0;
    m_SpotProperties[SPOT_TURRET].flags = 0;

    //Затем добавляем текстуры следов для колёсных и гусеничных типов шасси
    for(int i = 1; i <= ROBOT_CHASSIS_COUNT; ++i)
    {
        if(!g_Config.m_RobotChassisConsts[i].is_walking && !g_Config.m_RobotChassisConsts[i].is_rolling) continue;
        
        g_Config.m_RobotChassisConsts[i].ground_trace.trace_num = m_SpotProperties.size();
        SSpotProperties spot_tex;
        spot_tex.color = 0xFFFFFFFF; //Цвет в данном случае не используется
        spot_tex.func = CMatrixEffectLandscapeSpot::SpotTactConstant;
        spot_tex.texture = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, g_Config.m_RobotChassisConsts[i].ground_trace.texture_path);
        spot_tex.ttl = g_Config.m_RobotChassisConsts[i].ground_trace.trace_duration;
        spot_tex.flags = 0;

        m_SpotProperties.push_back(spot_tex);
    }

    // init sprites
    CBlockPar bp_sprite_textures(false); //В сам массив пихать имена смысла нет, так что используем временный BlockPar

    bp_sprite_textures.ParAdd(L"SmokeParticle", (CWStr)SPR_SMOKE_PART);
    bp_sprite_textures.ParAdd(L"BrightSmokeParticle", (CWStr)SPR_BRIGHT_SMOKE_PART);
    bp_sprite_textures.ParAdd(L"FireParticle", (CWStr)SPR_FIRE_PART);
    bp_sprite_textures.ParAdd(L"BrightFireParticle", (CWStr)SPR_BRIGHT_FIRE_PART);

    bp_sprite_textures.ParAdd(L"LightSpot", (CWStr)SPR_SPOT_LIGHT);
    bp_sprite_textures.ParAdd(L"SelectionSpot", (CWStr)SPR_SELECTION_PART);
    bp_sprite_textures.ParAdd(L"PathSpot", (CWStr)SPR_PATH_PART);

    bp_sprite_textures.ParAdd(L"Intense", (CWStr)BBT_INTENSE);

    bp_sprite_textures.ParAdd(L"GunFlash1", (CWStr)SPR_GUN_FLASH_1);
    bp_sprite_textures.ParAdd(L"GunFlash2", (CWStr)SPR_GUN_FLASH_2);
    bp_sprite_textures.ParAdd(L"GunFlash3", (CWStr)SPR_GUN_FLASH_3);

    bp_sprite_textures.ParAdd(L"Contrail", (CWStr)SPR_CONTRAIL);

    bp_sprite_textures.ParAdd(L"JetStreamAnim1", (CWStr)SPR_JET_STREAM_ANIM_FRAME_1);
    bp_sprite_textures.ParAdd(L"JetStreamAnim2", (CWStr)SPR_JET_STREAM_ANIM_FRAME_2);
    bp_sprite_textures.ParAdd(L"JetStreamAnim3", (CWStr)SPR_JET_STREAM_ANIM_FRAME_3);
    bp_sprite_textures.ParAdd(L"JetStreamAnim4", (CWStr)SPR_JET_STREAM_ANIM_FRAME_4);
    bp_sprite_textures.ParAdd(L"JetStreamAnim5", (CWStr)SPR_JET_STREAM_ANIM_FRAME_5);

    bp_sprite_textures.ParAdd(L"Spark", (CWStr)SPR_SPARK);
    bp_sprite_textures.ParAdd(L"Efield", (CWStr)BBT_EFIELD);

    bp_sprite_textures.ParAdd(L"FlameAnim1", (CWStr)SPR_FLAME_ANIM_FRAME_1);
    bp_sprite_textures.ParAdd(L"FlameAnim2", (CWStr)SPR_FLAME_ANIM_FRAME_2);
    bp_sprite_textures.ParAdd(L"FlameAnim3", (CWStr)SPR_FLAME_ANIM_FRAME_3);
    bp_sprite_textures.ParAdd(L"FlameAnim4", (CWStr)SPR_FLAME_ANIM_FRAME_4);
    bp_sprite_textures.ParAdd(L"FlameAnim5", (CWStr)SPR_FLAME_ANIM_FRAME_5);
    bp_sprite_textures.ParAdd(L"FlameAnim6", (CWStr)SPR_FLAME_ANIM_FRAME_6);
    bp_sprite_textures.ParAdd(L"FlameAnim7", (CWStr)SPR_FLAME_ANIM_FRAME_7);
    bp_sprite_textures.ParAdd(L"FlameAnim8", (CWStr)SPR_FLAME_ANIM_FRAME_8);

    bp_sprite_textures.ParAdd(L"FlamethrowerSpot", (CWStr)SPR_FLAMETHROWER_SPOT);
    bp_sprite_textures.ParAdd(L"FlamethrowerBeam", (CWStr)SPR_FLAMETHROWER_BEAM);

    bp_sprite_textures.ParAdd(L"LaserSpot", (CWStr)SPR_LASER_SPOT);
    bp_sprite_textures.ParAdd(L"LaserBeam", (CWStr)SPR_LASER_BEAM);

    bp_sprite_textures.ParAdd(L"PlasmaSpot", (CWStr)SPR_PLASMA_SPOT);
    bp_sprite_textures.ParAdd(L"PlasmaRect", (CWStr)SPR_PLASMA_RECT);

    bp_sprite_textures.ParAdd(L"DischargerSpot", (CWStr)SPR_DISCHARGER_SPOT);
    bp_sprite_textures.ParAdd(L"DischargerBeam", (CWStr)SPR_DISCHARGER_BEAM);

    bp_sprite_textures.ParAdd(L"RepairerSpot", (CWStr)SPR_REPAIRER_SPOT);
    bp_sprite_textures.ParAdd(L"RepairerRect", (CWStr)SPR_REPAIRER_RECT);

    bp_sprite_textures.ParAdd(L"ScorePlusSign", (CWStr)SPR_SCORE_PLUS_SIGN);
    bp_sprite_textures.ParAdd(L"ScoreNum0", (CWStr)SPR_SCORE_NUMBER_0);
    bp_sprite_textures.ParAdd(L"ScoreNum1", (CWStr)SPR_SCORE_NUMBER_1);
    bp_sprite_textures.ParAdd(L"ScoreNum2", (CWStr)SPR_SCORE_NUMBER_2);
    bp_sprite_textures.ParAdd(L"ScoreNum3", (CWStr)SPR_SCORE_NUMBER_3);
    bp_sprite_textures.ParAdd(L"ScoreNum4", (CWStr)SPR_SCORE_NUMBER_4);
    bp_sprite_textures.ParAdd(L"ScoreNum5", (CWStr)SPR_SCORE_NUMBER_5);
    bp_sprite_textures.ParAdd(L"ScoreNum6", (CWStr)SPR_SCORE_NUMBER_6);
    bp_sprite_textures.ParAdd(L"ScoreNum7", (CWStr)SPR_SCORE_NUMBER_7);
    bp_sprite_textures.ParAdd(L"ScoreNum8", (CWStr)SPR_SCORE_NUMBER_8);
    bp_sprite_textures.ParAdd(L"ScoreNum9", (CWStr)SPR_SCORE_NUMBER_9);

    bp_sprite_textures.ParAdd(L"TitanIcon", (CWStr)SPR_TITAN_ICON);
    bp_sprite_textures.ParAdd(L"ElectronicsIcon", (CWStr)SPR_ELECTRONICS_ICON);
    bp_sprite_textures.ParAdd(L"EnergyIcon", (CWStr)SPR_ENERGY_ICON);
    bp_sprite_textures.ParAdd(L"PlasmaIcon", (CWStr)SPR_PLASMA_ICON);
    bp_sprite_textures.ParAdd(L"ResourcesIcons", (CWStr)SPR_RESOURCES_ICON);

    CBlockPar* sprites_block = g_CacheData->BlockGet(L"Sprites");
    CBlockPar* merged_sprites = sprites_block->BlockGet(L"MergedSprites"); //Блок с обычными не подсвечивающимися спрайтами с одной общей текстуры

    //Для спрайтов, который нужно вставлять в игру как есть, без дополнительной подсветки от рендера, есть всего одна общая текстура,
    // спрайты с которой запоминаются по координатам
    CTextureManaged* merged_tex = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, merged_sprites->ParGet(L"Texture"));
    merged_tex->Preload();
    CSprite::SetSortTexture(merged_tex);
    float tex_X = (float)merged_tex->GetSizeX();
    float tex_Y = (float)merged_tex->GetSizeY();

    //Все прочие спрайты должны были получать подсветку рендера и изначально размещались на второй аналогичной общей текстуре,
    // однако места на ней всему не хватило, так что их в итоге их просто разбили по индивидуальным пикчам
    //CTextureManaged* ti = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, merged_sprites.ParGet(L"Texture"));
    //ti->Preload();

    for(int i = 0; i < SPR_LAST; ++i)
    {
        SSpriteTextureArrayElement* cur_element = &m_SpriteTextures[i];
        CWStr cur_par_name = bp_sprite_textures.ParGetName(i);
        CWStr par = sprites_block->ParGetNE(cur_par_name); //Либо координаты и размер области на общем листе, либо путь к отдельному файлу текстуры
        if(par == L"") //Если грузим текстуру с общего листа
        {
            par = merged_sprites->ParGet(cur_par_name);

            //По этому маркеру код в эффектах понимает, откуда ему нужно брать данную конкретную текстуру,
            // поскольку и указатель на отдельную текстуру спрайта и её координаты, если это текстура с общего листа, подгружаются из одного массива
            cur_element->SetSingleBrightTexture(false);

            int x0, y0, sx, sy;
            //Координаты текстуры на листе
            x0 = par.GetIntPar(0, L",");
            y0 = par.GetIntPar(1, L",");
            //Размер текстуры
            sx = par.GetIntPar(2, L",");
            sy = par.GetIntPar(3, L",");

            cur_element->spr_tex.tu0 = float(x0) / tex_X;
            cur_element->spr_tex.tv0 = float(y0) / tex_Y;

            cur_element->spr_tex.tu1 = float(x0 + sx) / tex_X;
            cur_element->spr_tex.tv1 = float(y0 + sy) / tex_Y;
        }
        else //Если грузим отдельную текстуру для рендера с подсветкой
        {
            cur_element->SetSingleBrightTexture(true);
            cur_element->tex = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, par.Get());
            //cur_element->tex->Preload();
        }
    }

    //Добавляем дополнительные спрайты по указателю из конфига, если нужно
    //Добавляем спрайты для эффектов шасси
    for(int i = 1; i < g_Config.m_RobotChassisConsts.size(); ++i)
    {
        for(int t = 0; t < g_Config.m_RobotChassisConsts[i].jet_stream.size(); ++t)
        {
            CWStr sprite_name;
            int sprite_num = -1;
            int sprites_count;

            sprite_name = g_Config.m_RobotChassisConsts[i].jet_stream[t].sprites_name;
            sprites_count = g_Config.m_RobotChassisConsts[i].jet_stream[t].sprites_count;
            for(int j = 1; j <= sprites_count; ++j)
            {
                sprite_num = AddSpriteTexture(sprite_name, j, sprites_count, sprites_block, bp_sprite_textures);
                g_Config.m_RobotChassisConsts[i].jet_stream[t].sprites_num.push_back(sprite_num);
            }
        }
    }

    //Добавляем спрайты для эффектов оружия
    for(int i = 1; i < g_Config.m_WeaponsConsts.size(); ++i)
    {
        //Немножечко кринжатины... Фиксированный 1 проход для map_objects_ignition
        // и дополнительные проходы по числу структур со спрайтовыми анимациями (или отдельными спрайтами)
        for(int t = 0; t < 1 + g_Config.m_WeaponsConsts[i].sprite_set.size(); ++t)
        {
            CWStr sprite_name;
            int sprite_num = -1;
            int sprites_count;
            if(!t)
            {
                sprite_name = g_Config.m_WeaponsConsts[i].map_objects_ignition.sprites_name;
                sprites_count = g_Config.m_WeaponsConsts[i].map_objects_ignition.sprites_count;
            }
            else
            {
                if(g_Config.m_WeaponsConsts[i].sprite_set[t - 1].sprites_name != L"")
                {
                    sprite_name = g_Config.m_WeaponsConsts[i].sprite_set[t - 1].sprites_name;
                    sprites_count = g_Config.m_WeaponsConsts[i].sprite_set[t - 1].sprites_count;
                }
                else //Например, для primary_effect типа EFFECT_CANNON список спрайтов (хотя там всего один) может оказаться пустым
                {
                    for(int j = 0; j < g_Config.m_WeaponsConsts[i].sprite_set[t - 1].sprites_count; ++j) g_Config.m_WeaponsConsts[i].sprite_set[t - 1].sprites_num.push_back(SPR_NONE);
                    continue;
                }
            }
            if(!sprites_count) continue;

            for(int j = 1; j <= sprites_count; ++j)
            {
                sprite_num = AddSpriteTexture(sprite_name, j, sprites_count, sprites_block, bp_sprite_textures);
                if(!t) g_Config.m_WeaponsConsts[i].map_objects_ignition.sprites_num.push_back(sprite_num);
                else g_Config.m_WeaponsConsts[i].sprite_set[t - 1].sprites_num.push_back(sprite_num);
            }
        }
    }

    m_before_draw_done = 0;
}

int CMatrixEffect::AddSpriteTexture(CWStr sprite_name, int sprite_num, int sprites_count, CBlockPar* sprites_block, CBlockPar& check_bp)
{
    CWStr name = sprite_name;
    int new_num;

    if(sprites_count > 1) name += CWStr(sprite_num);
    CWStr existed_num = check_bp.ParGetNE(name);
    if(existed_num != L"") new_num = existed_num.GetInt(); //Если данная текстура уже есть в списке загруженных, то просто возвращаем её номер
    else //Иначе добавляем новую
    {
        SSpriteTextureArrayElement sprite;
        new_num = m_SpriteTextures.size();
        check_bp.ParAdd(name, (CWStr)new_num);

        sprite.SetSingleBrightTexture(true);
        sprite.tex = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, sprites_block->ParGet(name).Get());
        m_SpriteTextures.push_back(sprite);
    }

    return new_num;
}

#ifdef _DEBUG
void SEffectHandler::Release(SDebugCallInfo& from)
#else
void SEffectHandler::Release(void)
#endif
{
    if(effect)
    {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(from, effect);
#else
        g_MatrixMap->SubEffect(effect);
#endif
        effect = nullptr;
    }
}

void CMatrixEffect::CreateExplosion(const D3DXVECTOR3& pos, const SExplosionProperties& props, bool fire)
{
    D3DXVECTOR3 temp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    if(ELIST_CNT(EFFECT_EXPLOSION) >= MAX_EFFECTS_EXPLOSIONS)
    {
        // remove oldest

#ifdef _DEBUG
        //DM("aaaaaaa", "Remove oldest explosion");
#endif

        ELSIT_REMOVE_OLDEST(EFFECT_EXPLOSION);
    }

    CMatrixEffectExplosion* e;

    float z = g_MatrixMap->GetZ(pos.x, pos.y);
    if(pos.z < (z + 10)) e = HNew(m_Heap) CMatrixEffectExplosion(D3DXVECTOR3(pos.x, pos.y, z + 10), props);
    else e = HNew(m_Heap) CMatrixEffectExplosion(pos, props);

    if(!g_MatrixMap->AddEffect(e)) return;

    if(props.sound != S_NONE) CSound::AddSound(props.sound, pos);
    //CMatrixEffect* ee = CMatrixEffect::CreateLandscapeSpot(m_Heap, D3DXVECTOR2(pos.x, pos.y), FSRND(3.1415), FRND(2) + 0.5f, SPOT_CONSTANT);
    
    if(fire)
    {
        //CMatrixEffect::CreateFire(nullptr, pos, 1000, 1500, 50, 10, true);

        CMatrixEffect::CreateFireAnim(nullptr, D3DXVECTOR3(pos.x, pos.y, z), 35, 60, 2000);
        CMatrixEffect::CreateSmoke(nullptr, pos, 1500, 1500, 100, 0xFF000000);
    }

    if(props.voronka != SPOT_TYPES_CNT)
    {
        int x = TruncFloat(pos.x * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE));
        int y = TruncFloat(pos.y * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE));
        CMatrixMapGroup* g = g_MatrixMap->GetGroupByIndexTest(x, y);
        if(g && !g->IsBaseOn()) CMatrixEffect::CreateLandscapeSpot(nullptr, D3DXVECTOR2(pos.x, pos.y), FSRND(M_PI), props.voronka_scale, props.voronka);
    }
}

void CMatrixEffect::CreateSmoke(
    SEffectHandler* eh,
    const D3DXVECTOR3& pos,
    float ttl,
    float puffttl,
    float spawntime,
    dword color,
    bool is_bright,
    float speed
)
{
    D3DXVECTOR3 temp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    if(ELIST_CNT(EFFECT_SMOKE) >= MAX_EFFECTS_SMOKEFIRES)
    {
        // remove oldest
        ELSIT_REMOVE_OLDEST(EFFECT_SMOKE);
    }

    CMatrixEffectSmoke* e = HNew(m_Heap) CMatrixEffectSmoke(pos, ttl, puffttl, spawntime, color, is_bright, speed);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

//Анимация горения с рендером партиклов в реальном времени (используется для горящих роботов и т.д.)
void CMatrixEffect::CreateFire(
    SEffectHandler* eh,
    const D3DXVECTOR3& pos,
    float ttl,
    float puffttl,
    float spawntime,
    float dispfactor,
    bool is_bright,
    float speed,
    const SFloatRGBColor& close_color,
    const SFloatRGBColor& far_color
)
{
    D3DXVECTOR3 temp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    if(ELIST_CNT(EFFECT_SMOKE) >= MAX_EFFECTS_SMOKEFIRES)
    {
        // remove oldest
        ELSIT_REMOVE_OLDEST(EFFECT_SMOKE);
    }

    CMatrixEffectFire* e = HNew(m_Heap) CMatrixEffectFire(pos, ttl, puffttl, spawntime, dispfactor, is_bright, speed, close_color, far_color);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

//Анимация горения с использованием готовых текстур (используется для горящих декоративных объектов)
void CMatrixEffect::CreateFireAnim(SEffectHandler* eh, const D3DXVECTOR3& pos, float anim_width, float anim_height, int time_to_live, const std::vector<int>& sprites_id)
{
    D3DXVECTOR3 temp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    if(ELIST_CNT(EFFECT_FIREANIM) >= MAX_EFFECTS_FIREANIM)
    {
        // remove oldest
        ELSIT_REMOVE_OLDEST(EFFECT_FIREANIM);
    }

    CMatrixEffectFireAnim* e = HNew(m_Heap) CMatrixEffectFireAnim(pos, anim_width, anim_height, time_to_live, sprites_id, sprites_id.size());
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}


void CMatrixEffect::CreateFirePlasma(const D3DXVECTOR3& start, const D3DXVECTOR3& end, float speed, dword hitmask, CMatrixMapStatic* skip, FIRE_END_HANDLER handler, dword user)
{
    CMatrixEffectFirePlasma* e = HNew(m_Heap) CMatrixEffectFirePlasma(start, end, speed, hitmask, skip, handler, user);
    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::CreateLandscapeSpot(
    SEffectHandler* eh,
    const D3DXVECTOR2& pos,
    float angle,
    float scale,
    ESpotType type
)
{
    if(type != SPOT_TURRET)
    {
        D3DXVECTOR2 temp = pos - *(D3DXVECTOR2*)&g_MatrixMap->m_Camera.GetFrustumCenter();
        if(MAX_EFFECT_DISTANCE_SQ < D3DXVec2LengthSq(&temp)) return;
    }

    CMatrixEffectLandscapeSpot* e = HNew(m_Heap) CMatrixEffectLandscapeSpot(pos, angle, scale, type);

    if(ELIST_CNT(EFFECT_LANDSCAPE_SPOT) >= MAX_EFFECTS_LANDSPOTS)
    {
        // remove oldest
//#ifdef _DEBUG
//
//        {
//            CMatrixEffect *ef = ELIST_FIRST(EFFECT_LANDSCAPE_SPOT);
//
//            CStr log;
//            DM("ef", "begin...........");
//
//            for(; ef; ef = ef->m_TypeNext)
//            {
//                CWStr hex;
//                hex.AddHex(byte((dword(ef) >> 24) & 0xFF));
//                hex.AddHex(byte((dword(ef) >> 16) & 0xFF));
//                hex.AddHex(byte((dword(ef) >> 8) & 0xFF));
//                hex.AddHex(byte((dword(ef) >> 0) & 0xFF));
//
//                log += (int)ef->GetType();
//                log += ":";
//                log += CStr(hex);
//                log += ":";
//                log += ef->Priority();
//                //log += "\n";
//
//                DM("ef", log);
//                log = "";
//            }
//
//            log += "effect:";
//            log += e->Priority();
//            DM("ef", log);
//            //log += "\n";
//
//            //SLOG("ls.txt", log.Get());
//        }
//#endif
        int p = e->Priority() + 1;
        ELSIT_REMOVE_OLDEST_PRIORITY(EFFECT_LANDSCAPE_SPOT, p);
    }

    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateMovingObject(SEffectHandler* eh, const SMOProps& props, dword hitmask, CMatrixMapStatic* skip, FIRE_END_HANDLER handler, dword user)
{
DTRACE();

    CMatrixEffectMovingObject* e = HNew(m_Heap) CMatrixEffectMovingObject(props, hitmask, skip,  handler, user);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

//Упрощённый вызов функции отрисовки вейпоинта, отрисовывает данный эффект в текущей точке положения курсора
void CMatrixEffect::CreateMoveToAnim(int type)
{
DTRACE();

    CreateMoveToAnim(g_MatrixMap->m_TraceStopPos, type);
}
//Непосредственный вызов отрисовки вейпоинта на поверхности (необходимо указывать точный вектор)
void CMatrixEffect::CreateMoveToAnim(const D3DXVECTOR3 &pos, int type)
{
DTRACE();

    D3DXVECTOR3 tp = g_MatrixMap->m_Camera.GetFrustumCenter() - pos;
    D3DXVec3Normalize(&tp, &tp);

    CMatrixEffectMoveto* e = HNew(m_Heap) CMatrixEffectMoveto(pos + tp * 10, type);

    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::DeleteAllMoveto()
{
    while(ELIST_FIRST(EFFECT_MOVETO)) ELSIT_REMOVE_OLDEST(EFFECT_MOVETO);
}

void CMatrixEffect::CreateBuoy(SEffectHandler* eh, const D3DXVECTOR3& pos, EBuoyType bt)
{
DTRACE();

    SMOProps mo;
    memset(&mo, 0, sizeof(mo));
    mo.curpos = pos;
    mo.buoytype = bt;
    //mo.velocity = D3DXVECTOR3(1, 1, 1);

    const wchar* name = OBJECT_PATH_LOGO_RED;

    if(bt == BUOY_BLUE) name = OBJECT_PATH_LOGO_BLUE;
    else if(bt == BUOY_GREEN) name = OBJECT_PATH_LOGO_GREEN;

    mo.object = LoadObject(name, m_Heap);

    CMatrixEffectBuoy* e = HNew(m_Heap) CMatrixEffectBuoy(mo);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

CMatrixEffect* CMatrixEffect::CreateSelection(const D3DXVECTOR3& pos, float r, dword color)
{
DTRACE();

    CMatrixEffectSelection* e = HNew(m_Heap) CMatrixEffectSelection(pos, r, color);
    return e;
}

CMatrixEffect* CMatrixEffect::CreatePath(
    const D3DXVECTOR3* pos,
    int cnt
)
{
DTRACE();

    CMatrixEffectPath* e = HNew(m_Heap) CMatrixEffectPath(pos, cnt);
    return e;
}

void CMatrixEffect::CreatePointLight(
    SEffectHandler* eh,
    const D3DXVECTOR3& pos,
    float radius,
    dword color,
    bool drawbill
)
{
    D3DXVECTOR3 temp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    if(ELIST_CNT(EFFECT_POINT_LIGHT) >= MAX_EFFECTS_POINT_LIGHTS)
    {
        // remove oldest
        ELSIT_REMOVE_OLDEST(EFFECT_POINT_LIGHT);
    }

    CMatrixEffectPointLight* e = HNew(m_Heap) CMatrixEffectPointLight(pos, radius, color, drawbill);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateKonus(
    SEffectHandler* eh,
    const D3DXVECTOR3& start,
    const D3DXVECTOR3& dir,
    float radius,
    float height,
    float angle,
    float ttl,
    bool is_bright,
    CTextureManaged* tex
)
{
    D3DXVECTOR3 temp = start - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    CMatrixEffectKonus* e = HNew(m_Heap) CMatrixEffectKonus(start, dir, radius, height, angle, ttl, is_bright, tex);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateKonusSplash(
    const D3DXVECTOR3& start,
    const D3DXVECTOR3& dir,
    float radius,
    float height,
    float angle,
    float ttl,
    bool is_bright,
    CTextureManaged* tex
)
{
    D3DXVECTOR3 temp = start - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    CMatrixEffectKonusSplash* e = HNew(m_Heap) CMatrixEffectKonusSplash(start, dir, radius, height, angle, ttl, is_bright, tex);

    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(e)
    {
        CSound::AddSound(S_SPLASH, start);
    }
}

CMatrixEffect* CMatrixEffect::CreateWeapon(
    const D3DXVECTOR3& start,
    const D3DXVECTOR3& dir,
    dword user,
    FIRE_END_HANDLER handler,
    int type,
    int cooldown
)
{
DTRACE();

    CMatrixEffectWeapon* e = HNew(m_Heap) CMatrixEffectWeapon(start, dir, user, handler, type, cooldown);
    return e;
}

void CMatrixEffect::CreateFlame(
    SEffectHandler* eh,
    float ttl,
    dword hitmask,
    CMatrixMapStatic* skip,
    dword user,
    FIRE_END_HANDLER handler
)
{
DTRACE();

    CMatrixEffectFlame* e = HNew(m_Heap) CMatrixEffectFlame(ttl, hitmask, skip, user, handler);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateBigBoom(
    const D3DXVECTOR3& pos,
    float radius,
    float ttl,
    dword hitmask,
    CMatrixMapStatic* skip,
    dword user,
    FIRE_END_HANDLER handler,
    dword light
)
{
DTRACE();

    CMatrixEffectBigBoom* e = HNew(m_Heap) CMatrixEffectBigBoom(pos, radius, ttl, hitmask, skip, user, handler, light);
    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::CreateLightening(
    SEffectHandler* eh,
    const D3DXVECTOR3& pos0,
    const D3DXVECTOR3& pos1,
    float ttl,
    float dispers,
    float width,
    dword color,
    int spot_sprite_num,
    int beam_sprite_num
)
{
    D3DXVECTOR3 temp = pos0 - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    CMatrixEffectLightening* e = HNew(m_Heap) CMatrixEffectLightening(pos0, pos1, ttl, dispers, width, color, spot_sprite_num, beam_sprite_num);
    e->SetPos(pos0, pos1);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateShorted(
    const D3DXVECTOR3& pos0,
    const D3DXVECTOR3& pos1,
    float ttl,
    dword color,
    int beam_sprite_num
)
{
DTRACE();

    CMatrixEffectShorted* e = HNew(m_Heap) CMatrixEffectShorted(pos0, pos1, ttl, color, beam_sprite_num);
    e->SetPos(pos0, pos1);
    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::CreateBillboard(SEffectHandler* eh, const D3DXVECTOR3& pos, float radius1, float radius2, dword color1, dword color2, float ttl, float delay, const wchar* tex, const D3DXVECTOR3& dir, ADD_TAKT addtakt)
{
DTRACE();

    CMatrixEffectBillboard* e = HNew(m_Heap) CMatrixEffectBillboard(pos, radius1, radius2, color1, color2, ttl, delay, tex, dir, addtakt);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateBillboardScore(const wchar* n, const D3DXVECTOR3& pos, dword color)
{
DTRACE();

    CSound::AddSound(S_RESINCOME, pos);

    CMatrixEffectBillboardScore* e = HNew(m_Heap) CMatrixEffectBillboardScore(n, pos, color);
    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::CreateSpritesLine(SEffectHandler* eh, const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, float width, dword color1, dword color2, float ttl, CTextureManaged* tex)
{
    D3DXVECTOR3 temp = pos0 - g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&temp)) return;

    CMatrixEffectSpritesLine* e = HNew(m_Heap) CMatrixEffectSpritesLine(pos0, pos1, width, color1, color2, ttl, tex);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

CMatrixEffect * CMatrixEffect::CreateZahvat(const D3DXVECTOR3 &pos, float radius, float angle, int cnt)
{
DTRACE();

    CMatrixEffectZahvat *e = HNew(m_Heap) CMatrixEffectZahvat(pos, radius, angle, cnt);
    return e;
}

CMatrixEffect * CMatrixEffect::CreateElevatorField(const D3DXVECTOR3 &pos0,const D3DXVECTOR3 &pos1, float radius, const D3DXVECTOR3 & fwd)
{
DTRACE();

    CMatrixEffectElevatorField *e = HNew(m_Heap) CMatrixEffectElevatorField(pos0, pos1, radius, fwd);
    return e;
}

void CMatrixEffect::CreateDust(SEffectHandler* eh, const D3DXVECTOR2& pos, const D3DXVECTOR2& adddir, float ttl)
{
    D3DXVECTOR2 temp = pos - *(D3DXVECTOR2*)&g_MatrixMap->m_Camera.GetFrustumCenter();
    if(MAX_EFFECT_DISTANCE_SQ < D3DXVec2LengthSq(&temp)) return;

    CMatrixEffectDust* e = HNew(m_Heap) CMatrixEffectDust(pos, adddir, ttl);
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateShleif(SEffectHandler* eh)
{
DTRACE();

    CMatrixEffectShleif* e = HNew(m_Heap) CMatrixEffectShleif();
    if(!g_MatrixMap->AddEffect(e)) e = nullptr;
    if(eh && e)
    {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

CMatrixEffect* CMatrixEffect::CreateRepair(const D3DXVECTOR3& pos, const D3DXVECTOR3& dir, float seek_radius, CMatrixMapStatic* skip, ESpriteTextureSort sprite_spot)
{
DTRACE();

    CMatrixEffectRepair* e = HNew(m_Heap) CMatrixEffectRepair(pos, dir, seek_radius, skip, sprite_spot);
    return e;
}

CMatrixEffectFireStream* CMatrixEffect::CreateFireStream(const D3DXVECTOR3& pos0, const D3DXVECTOR3& pos1, const std::vector<int>& sprites_num)
{
DTRACE();

    CMatrixEffectFireStream* e = HNew(m_Heap) CMatrixEffectFireStream(pos0, pos1, sprites_num);
    return e;
}

void CMatrixEffect::DrawBegin(void)
{
DTRACE();

    float fBias = -1.0f;
    for(int i = 0; i < 4; ++i) ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&(fBias)))));

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));

    //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    //DCPO_START();
}
 
void CMatrixEffect::DrawEnd(void)
{
    DTRACE();
    //ASSERT_DX(g_D3DD->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE ));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE,	TRUE));

    //restore view matrix

    //ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW,&g_MatrixMap->GetViewMatrix()));

    m_before_draw_done = 0;

/*
#ifdef _TRACE
    int pri[16];
    int pric[16];
    memset(pri, -1, sizeof(pri));
    memset(pric, 0, sizeof(pri));

    CMatrixEffectLandscapeSpot* ls = (CMatrixEffectLandscapeSpot*)ELIST_FIRST(EFFECT_LANDSCAPE_SPOT);
    for(; ls; ls = (CMatrixEffectLandscapeSpot*)ls->m_TypeNext)
    {
        int pr = ls->Priority();
        for(int i = 0; i < 16; ++i)
        {
            if(pri[i] == -1 || pri[i] == pr)
            {
                pri[i] = pr;
                ++pric[i];
                break;
            }
        }
    }

    for(int i = 0; i < 16; ++i)
    {
        if(pri[i] == -1) break;
        g_MatrixMap->m_DI.T(L"ls:" + CWStr(pri[i]), CWStr(pric[i]), 1000);
    }
#endif
*/
}