// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <vector>

#ifndef MATRIX_SOUND_MANAGER
#define MATRIX_SOUND_MANAGER

#define SOUND_ID_EMPTY  (0xFFFFFFFF)
#define SOUND_FULL_VOLUME_DIST  200

#define SOUND_POS_DIVIDER   (GLOBAL_SCALE + GLOBAL_SCALE)

#define MAX_SOUNDS      16  // 16 mixed sound for SL_ALL

enum ESoundLayer
{
    SL_ALL = 0,
    SL_INTERFACE,
    SL_ELEVATORFIELD,   // its only one per game
    SL_SELECTION,
    SL_HULL,
    SL_CHASSIS,

    SL_ORDER,

    SL_ABLAZE0,
    SL_SHORTED0,

    SL_COUNT
};

// works only for non SL_ALL layer
enum ESoundInterruptFlag
{
    SEF_INTERRUPT,
    SEF_SKIP
};

//Базовые звуки, которые по умолчанию задействованы в ПБ (не менять размер с int, т.к. массив базовых звуков расширяется после загрузки новых из конфига)
enum ESound : int
{
    // interface
    S_BCLICK = 0,
    S_BENTER,
    S_BLEAVE,
    S_MAP_PLUS,
    S_MAP_MINUS,
    S_PRESET_CLICK,
    S_BUILD_CLICK,
    S_CANCEL_CLICK,

    // sounds of base
    S_DOORS_OPEN,
    S_DOORS_CLOSE,
    S_DOORS_CLOSE_STOP,
    S_PLATFORM_UP,
    S_PLATFORM_DOWN,
    S_PLATFORM_UP_STOP,
    //S_BASE_AMBIENT,

    // other building ambients
    //S_TITAN_AMBIENT,
    //S_PLASMA_AMBIENT,
    //S_ELECTRONIC_AMBIENT,
    //S_ENERGY_AMBIENT,
    //S_REPAIR_AMBIENT,

    // effects explosions
    S_EXPLOSION_NORMAL,
    S_EXPLOSION_MISSILE,
    S_EXPLOSION_ROBOT_HIT,
    S_EXPLOSION_LASER_HIT,
    S_EXPLOSION_BUILDING_BOOM,      // small explosives
    S_EXPLOSION_BUILDING_BOOM2,     // small explosives 2
    S_EXPLOSION_BUILDING_BOOM3,     // non base buildings
    S_EXPLOSION_BUILDING_BOOM4,     // base buildings
    S_EXPLOSION_ROBOT_BOOM,
    S_EXPLOSION_ROBOT_BOOM_SMALL,
    S_EXPLOSION_BIGBOOM,
    S_EXPLOSION_OBJECT,

    S_SPLASH,

    S_EF_START,
    S_EF_CONTINUE,
    S_EF_END,

    // flyer
    S_FLYER_PROPELLER_START,
    S_FLYER_PROPELLER_CONTINUE,

    S_ROBOT_UPAL, //Видимо, с вертолёта

    S_ROBOT_BUILD_END, //Новый робот построен
    S_ROBOT_BUILD_END_ALT, //Робот готов

    S_TURRET_BUILD_START,
    S_TURRET_BUILD_0,
    S_TURRET_BUILD_1,
    S_TURRET_BUILD_2,
    S_TURRET_BUILD_3,

    S_FLYER_BUILD_END,
    S_FLYER_BUILD_END_ALT,
    
    S_YES_SIR_1,    
    S_YES_SIR_2,
    S_YES_SIR_3,
    S_YES_SIR_4,
    S_YES_SIR_5,

    S_SELECTION_1,
    S_SELECTION_2,
    S_SELECTION_3,
    S_SELECTION_4,
    S_SELECTION_5,
    S_SELECTION_6,
    S_SELECTION_7,

    S_BUILDING_SEL,
    S_BASE_SEL,
    S_WEAPON_SET, //При включении/выключении оружия в ручном режиме
    S_GUIDANCE_SWITCH, //При смене режима управления наведением ракет в ручном режиме

    S_SIDE_UNDER_ATTACK_1,
    S_SIDE_UNDER_ATTACK_2,
    S_SIDE_UNDER_ATTACK_3,

    S_ENEMY_BASE_CAPTURED,
    S_ENEMY_FACTORY_CAPTURED,

    S_PLAYER_BASE_CAPTURED,
    S_PLAYER_FACTORY_CAPTURED,

    S_BASE_KILLED,
    S_FACTORY_KILLED,
    S_BUILDING_KILLED,

    S_REINFORCEMENTS_CALLED,
    S_REINFORCEMENTS_READY,

    S_RESINCOME,

    S_TERRON_PAIN1,
    S_TERRON_PAIN2,
    S_TERRON_PAIN3,
    S_TERRON_PAIN4,
    S_TERRON_KILLED,

    //orders
    S_ORDER_INPROGRESS1,
    S_ORDER_INPROGRESS2,

    S_ORDER_ACCEPT,

    S_ORDER_ATTACK,
    S_ORDER_CAPTURE,
    S_ORDER_CAPTURE_PUSH,
    S_ORDER_REPAIR,

    S_ORDER_AUTO_ATTACK,
    S_ORDER_AUTO_CAPTURE,
    S_ORDER_AUTO_DEFENCE,

    S_ORDER_CAPTURE_FUCK_OFF,

    S_CANTBE,

    S_SPECIAL_SLOT,

    S_COUNT,
    S_UNDEF = -1,
    S_NONE = -2
};


class CSound : public CMain
{
public:
    struct SSoundItem
    {
        float vol0 = 1.0f, vol1 = 1.0f;
        float pan0 = 0.0f, pan1 = 0.0f;
        dword flags = 0;
        float attn = 0.0f;
        float radius = 0.0f;
        float ttl = 0.0f;          // valid only for looped pos sounds
        float fadetime = 0.0f;     // valid only for looped pos sounds
        CWStr path_name = (CWStr)L"";

        SSoundItem() = default;
        SSoundItem(const wchar* sound_name) { path_name = (CWStr)sound_name; }
        ~SSoundItem() = default;
        CWStr& SoundPath() { return path_name; }

        static const dword LOOPED = SETBIT(0);
        static const dword LOADED = SETBIT(1);
    };

private:
    struct SLID
    {
        int   index = 0;
        dword id = 0;

        bool IsPlayed();
    };

    struct SPlayedSound
    {
        dword id_internal = 0;     //Used in Rangers engine
        dword id = SOUND_ID_EMPTY; //In robots always uniq! there is no the same id's per game
        float curvol = 0.0f;
        float curpan = 0.0f;
    };

    static SPlayedSound m_AllSounds[MAX_SOUNDS];
    static SLID         m_LayersI[SL_COUNT]; // indices in m_AllSounds array
    static int          m_LastGroup;
    static dword        m_LastID;

    static CDWORDMap*   m_PosSounds;

    static std::vector<CSound::SSoundItem> m_Sounds;
    static ESound FindSoundByName(CWStr& name) { for(int i = 0; i < m_Sounds.size(); ++i) { if(m_Sounds[i].path_name == name) return (ESound)i; } return S_NONE; }

    static dword PlayInternal(ESound snd, float vol, float pan, ESoundLayer sl, ESoundInterruptFlag interrupt);
    static void  StopPlayInternal(int deli);
    static int   FindSlotForSound();
    static int   FindSoundSlot(dword id);
    static int   FindSoundSlotPlayedOnly(dword id);
    //static void ExtraRemove(void);  // extra remove sound from SL_ALL layer.

public:
    friend class CSoundArray;

    static bool IsSoundPlay(dword id);

    static void Init();
    static void Clear();
    static void LayerOff(ESoundLayer sl);
    static void SureLoaded(ESound snd);

    static dword Play(ESound snd, ESoundLayer sl = SL_ALL, ESoundInterruptFlag interrupt = SEF_INTERRUPT);
    static void Play(ESound snd, float vol, float pan, ESoundLayer sl = SL_ALL, ESoundInterruptFlag interrupt = SEF_INTERRUPT) { SureLoaded(snd); PlayInternal(snd, vol, pan, sl, interrupt); }
    static dword Play(const wchar* name, ESoundLayer sl = SL_ALL, ESoundInterruptFlag interrupt = SEF_INTERRUPT); // name - robots sound
    static dword Play(const wchar* name, const D3DXVECTOR3& pos, ESoundLayer sl = SL_ALL, ESoundInterruptFlag interrupt = SEF_INTERRUPT); // name - robots sound
    static dword Play(const D3DXVECTOR3& pos, float attn, float pan0, float pan1, float vol0, float vol1, wchar* name);
    static dword Play(ESound snd, const D3DXVECTOR3& pos, ESoundLayer sl = SL_ALL, ESoundInterruptFlag interrupt = SEF_INTERRUPT);
    static dword Play(dword id, ESound snd, const D3DXVECTOR3& pos, ESoundLayer sl = SL_ALL, ESoundInterruptFlag interrupt = SEF_INTERRUPT);  // use only for ambient
    static void StopPlay(dword id);
    static void StopPlayAllSounds();
    static dword ChangePos(dword id, ESound snd, const D3DXVECTOR3& pos);
    static float GetSoundMaxDistSQ(ESound snd) { SureLoaded(snd); return m_Sounds[snd].radius * m_Sounds[snd].radius; }

    static void CalcPanVol(const D3DXVECTOR3& pos, float attn, float pan0, float pan1, float vol0, float vol1, float* pan, float* vol);

    static void Tact();

    static void AddSound(ESound snd, const D3DXVECTOR3& pos, ESoundLayer sl = SL_ALL, ESoundInterruptFlag ifl = SEF_INTERRUPT); // automatic position

    static void AddSound(const wchar* name, const D3DXVECTOR3& pos); // automatic position
    static void AddSound(const D3DXVECTOR3& pos, float attn, float pan0, float pan1, float vol0, float vol1, wchar* name);

    static __forceinline dword Pos2Key(const D3DXVECTOR3& pos);

    static void SaveSoundLog();
};

class CSoundArray : public Base::CBuf
{
public:
    struct SSndData
    {
        ESound snd = S_UNDEF;
        dword id = -1;
        float pan0 = 0.0f, pan1 = 0.0f;
        float vol0 = 0.0f, vol1 = 0.0f;
        float attn = 0.0f;
        float ttl = 0.0f, fade = 0.0f;
    };

    CSoundArray(CHeap* heap) : CBuf(heap) {};

    void AddSound(ESound snd, const D3DXVECTOR3& pos, ESoundLayer sl = SL_ALL, ESoundInterruptFlag ifl = SEF_INTERRUPT);
    void AddSound(const D3DXVECTOR3& pos, float attn, float pan0, float pan1, float vol0, float vol1, wchar* name)
    {
        dword id = CSound::Play(pos, attn, pan0, pan1, vol0, vol1, name);
        if(id == SOUND_ID_EMPTY) return;

        Expand(sizeof(SSndData));
        (BuffEnd<SSndData>() - 1)->id = id;
        (BuffEnd<SSndData>() - 1)->pan0 = pan0;
        (BuffEnd<SSndData>() - 1)->pan1 = pan1;
        (BuffEnd<SSndData>() - 1)->vol0 = vol0;
        (BuffEnd<SSndData>() - 1)->vol1 = vol1;
        (BuffEnd<SSndData>() - 1)->attn = attn;
        (BuffEnd<SSndData>() - 1)->snd = S_UNDEF;
    }
    void SetSoundPos(const D3DXVECTOR3& pos);
    void UpdateTimings(float ms);
};

#endif