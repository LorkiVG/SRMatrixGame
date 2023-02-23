// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once


#define MISSILE_FIRE_PERIOD      20
#define BOMB_FIRE_PERIOD         15
#define GUN_FIRE_PERIOD          10

#define HOMING_RADIUS            1000

#define MISSILE_IMPACT_RADIUS    18
#define HM_SEEK_TIME_PERIOD      50

#define BOMB_DAMAGE_RADIUS       20
#define CANNON_ROUND_IMPACT_RADIUS 20

// moving object

struct SObjectCore;
struct SMOProps;
typedef void (*MO_MOVE_HANDLER)(D3DXMATRIX& m, SMOProps& pops, float tact); // function to build matrix for moving object
struct SMOProps
{
    D3DXVECTOR3        startpos = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3        curpos = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3        velocity = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3        target = { 0.0f, 0.0f, 0.0f };
    MO_MOVE_HANDLER    handler = nullptr;

    CVectorObjectAnim* object = nullptr;

    // runtime members!
    float              time = 0.0f;         // its automaticaly increases
    bool               endoflife = false;   // set this flag to initiate death of object
    float              distance = 0.0f;     // temporary. use it for your own purpose. initialy it is and distance between start and end pos
    FIRE_END_HANDLER   endhandler = nullptr;
    dword              uservalue = 0;
    //SObjectCore*     attacker = nullptr;  // attacker
    int                side = 0;            // side of attacker (if it dead)
    union
    {
        dword          hitmask = 0;
        EBuoyType      buoytype;
    };

    CMatrixMapStatic*  skip = nullptr;
    SEffectHandler*    shleif = nullptr;
    
    union
    {
        struct
        {
            float        next_fire_time = 0.0f;
            float        next_seek_time = 0.0f;
            float        splash_radius = 0.0f;

            D3DXVECTOR3  full_velocity = { 0.0f, 0.0f, 0.0f };
            int          full_velocity_reach = 0;
            bool         is_full_velocity = false;
            float        acceleration_coef = 0.0f; //На какое число множится скорость ракеты с каждым обсчётом логики (включается только после достижения маршевой скорости)
            float        target_capture_angle_cos = 0.0f; //Записывается сюда как косинус угла, используется для самонаведения ракеты
            float        homing_speed = 0.0f; //От 0.0 до 1.0, является маркером и заодно показателем эффективности самонаведения (включается только после достижения маршевой скорости)

            float        max_lifetime = 0.0f; //Автоматически самоуничтожается спустя указанное время
            int          fire_effect_starts = 0;

            CMatrixMapStatic* missile_owner = nullptr; //Выпустивший данную ракету юнит (робот/вертолёт/турель)

            SFloatRGBColor close_color_rgb = { 0.0f, 0.0f, 0.0f };
            SFloatRGBColor far_color_rgb = { 0.0f, 0.0f, 0.0f };

            SObjectCore* target = nullptr;
            int          in_frustum_count = 0;

        } hm = {};// homing_missile;

        struct
        {
            float        maxdist = 0.0f;
            float        dist = 0.0f;
            int          in_frustum_count = 0;
            float        splash_radius = 0.0f;

            //Для отрисовки инверсионного следа
            ESpriteTextureSort contrail_sprite_num = (ESpriteTextureSort)0; //В момент создания объекта номер всё равно будет переприсвоен, так что плевать
            int          contrail_width = 0;
            int          contrail_duration = 0;
            dword        contrail_color = 0;

        } gun;

        struct
        {
            float        next_fire_time = 0.0f;
            CTrajectory* trajectory = nullptr;
            float        pos = 0.0f;

        } bomb;

        struct
        {
            float angle = 0.0f;

        } buoy;

    };

    SMOProps() {};
};



class CMatrixEffectMovingObject : public CMatrixEffect
{
    SMOProps   m_Props;
    D3DXMATRIX m_Mat;

    CMatrixEffectMovingObject(const SMOProps& props, dword hitmask, CMatrixMapStatic* skip, FIRE_END_HANDLER handler, dword user);
	virtual ~CMatrixEffectMovingObject();

public:
    friend class CMatrixEffect;
    friend class CMatrixEffectBuoy;
    friend class CMatrixEffectMoveto;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Tact(float step);
    virtual void Release(void);
    virtual int  Priority(void) {return MAX_EFFECT_PRIORITY;};
};



class CMatrixEffectBuoy : public CMatrixEffectMovingObject
{
    CMatrixEffectBuoy(const SMOProps& props) : CMatrixEffectMovingObject(props, 0, 0, 0, 0), m_Kill(false), m_KillTime(1000),
#ifdef _DEBUG
    m_Light(DEBUG_CALL_INFO)
#else
    m_Light()
#endif

    {
        m_Props.buoytype = props.buoytype; //m_Z = props.curpos.z;
        m_BuoyColor = 0x00FF0000;
        if(m_Props.buoytype == BUOY_BLUE) m_BuoyColor = 0x000000FF;
        else if(m_Props.buoytype == BUOY_GREEN) m_BuoyColor = 0x0000FF00;
        CMatrixEffectBuoy::Tact(0);
    }

    bool    m_Kill;
    float   m_KillTime;
    dword   m_BuoyColor;

    SEffectHandler m_Light;
    //float   m_Z;
public:
    friend class CMatrixEffect;

    virtual void Draw(void);
    virtual void Tact(float);

    void Kill(void);
};

void MO_Homing_Missile_Tact(D3DXMATRIX& m, SMOProps& pops, float tact);
void MO_Grenade_Tact(D3DXMATRIX& m, SMOProps& pops, float tact);
void MO_Cannon_Round_Tact(D3DXMATRIX& m, SMOProps& pops, float tact);