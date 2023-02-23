// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_CAMERA_INCLUDE
#define MATRIX_CAMERA_INCLUDE

#include <math.h>
#include "math3d.hpp"
#include "MatrixConfig.hpp"

enum EFrustumPlane
{
	FPLANE_LEFT,
	FPLANE_RIGHT,
	FPLANE_BOTTOM,
	FPLANE_TOP
};

#define CAM_ACTION_MOVE_LEFT    SETBIT(0)
#define CAM_ACTION_MOVE_RIGHT   SETBIT(1)
#define CAM_ACTION_MOVE_UP      SETBIT(2)
#define CAM_ACTION_MOVE_DOWN    SETBIT(3)
#define CAM_ACTION_ROT_LEFT     SETBIT(4)
#define CAM_ACTION_ROT_RIGHT    SETBIT(5)
#define CAM_ACTION_ROT_UP       SETBIT(6)
#define CAM_ACTION_ROT_DOWN     SETBIT(7)

#define CAM_LINK_POINT_CHANGED  SETBIT(8)   // camera changes its link point
//#define CAM_PREVARCADED         SETBIT(9)
//#define CAM_RESTORE             SETBIT(10)
#define CAM_XY_LERP_OFF         SETBIT(11) // xy pos restore
//#define CAM_LANDRELATIVE        SETBIT(12)

#define CAM_SELECTED_TARGET      SETBIT(13)
#define CAM_NEED2END_DIALOG_MODE SETBIT(14)

//#define CAM_LINK_DIST           0.2f
//#define CAM_UNLINK_DIST         0.02f

//#define CAM_MOVE_TIME           1
//#define CAM_ROT_SPEED           0.001f
//#define CAM_MOVE_SPEED          0.35f

//#define CAM_BOT_POINT_DIST      40
//#define CAM_BOT_WEIGHT          1
//#define CAM_BOT_MAX_SPEED       20
//#define CAM_BOT_MAX_FORCE       8
//#define CAM_BOT_HEIGHT          55

#define CAM_HFOV                GRAD2RAD(g_CamFieldOfView) //D3DX_PI/3/*6*/

//#define MAX_VIEW_DISTANCE       4000
//#define MAX_VIEW_DISTANCE_SQ    (MAX_VIEW_DISTANCE * MAX_VIEW_DISTANCE)
//extern float MAX_VIEW_DISTANCE;

void SetMaxCameraDistance(float perc);

//#define FOG_NEAR_K              0.5 //Точка удаления от камеры, в которой начинается отрисовка разреженного тумана
//#define FOG_FAR_K               0.7 //Точка удаления от камеры, в которой начинается отрисовка сплошного тумана


//#define MAX_TIME_ON_TRAJ        8000
#define MAX_WAR_PAIRS           16
#define RECALC_DEST_ANGZ_PERIOD 150
#define WAR_PAIR_TTL            1000
#define OBZOR_TIME              12500
#define PI                      3.141592653589793

//const static double pi = 3.141592653589793;

struct SObjectCore;

enum ETrajectoryStatus
{
    TRJS_JUST_STAYING,
    TRJS_MOVING_TO,
    TRJS_MOVING_TO_BIGBOOM,
    TRJS_FLY_AROUND,
};

struct SAutoFlyData
{
    struct SWarPair
    {
        SObjectCore* target;
        SObjectCore* attacker;
        float        ttl = 0.0f;
    };

    ETrajectoryStatus m_Status;

    //CTrajectory*  m_Traj;

    D3DXVECTOR3     m_Cur;

    float           m_CurDist;
    float           m_CurAngZ;
    float           m_CurAngX;

    //D3DXVECTOR3   m_NewActual;
    D3DXVECTOR3     m_New;
    float           m_NewDist;
    float           m_NewAngZ;
    float           m_NewAngX;

    float           m_RotSpeedZ;
    float           m_RotSpeedX;
    
    float           m_Speed;
    //float         m_OnTrajectorySpeedNeed;
    //float         m_OnTrajectorySpeedCurrent;

    SWarPair        m_WarPairCurrent;
    SWarPair        m_WarPairs[MAX_WAR_PAIRS];
    int             m_WarPairsCnt;

    int             m_LastStatTime;
    int             m_LastObzorTime;


    //float         m_CalcDestTime;
    float           m_CalcDestAngZTime;
    //float         m_OnTrajTime;
    float           m_ObzorTime;
    
    float           m_BeforeObzorAngX;

    //void          BuildTrajectory(ETrajectoryStatus s);
    void            Tact(float ms);

    void            FindAutoFlyTarget();

    void            KillTrajectory();
    void            Stat();
    void            Release();
    void            AddWarPair(CMatrixMapStatic *tgt, CMatrixMapStatic *attacker);

};

class CMatrixCamera : public CMain
{
    D3DVIEWPORT9    m_ViewPort;
	D3DXMATRIX      m_MatProj;
	D3DXMATRIX      m_MatView;
    D3DXMATRIX      m_MatViewInversed;

    D3DXMATRIX      m_DN_iview;
    D3DXVECTOR3     m_DN_fc = { 0.0f, 0.0f, 0.0f };

    D3DXVECTOR3     m_FrustumCenter = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3     m_FrustumDirLT = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3     m_FrustumDirLB = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3     m_FrustumDirRT = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3     m_FrustumDirRB = { 0.0f, 0.0f, 0.0f };

    SPlane          m_FrustPlaneL;
    SPlane          m_FrustPlaneR;
    SPlane          m_FrustPlaneT;
    SPlane          m_FrustPlaneB;

    float           _mp_11_inversed = 0.0f;
    float           _mp_22_inversed = 0.0f;

    float           _res_x_inversed = 0.0f;
    float           _res_y_inversed = 0.0f;

    dword           m_Flags = 0;

    float           m_AngleX = 0.0f;
    float           m_AngleZ = 0.0f;
    float           m_AngleParam[CAMERA_PARAM_CNT] = { 0.0f };
    int             m_ModeIndex = CAMERA_STRATEGY;
    
    // current cam values
    D3DXVECTOR3     m_LinkPoint = { 200.0f, 200.0f, 140.0f }; // real link point (do not modify, it is calculated automaticaly)

    float           m_Dist = 0.0f;
    float			m_DistParam[CAMERA_PARAM_CNT] = { 0.0f };

    D3DXVECTOR2 m_XY_Strategy = { 200.0f, 200.0f };

    //Содержит угол в радианах, запоминая (суммируя) каждый сделанный камерой по часовой (плюсует), либо против часовой (вычитает) стрелке оборот
    //Запоминает горизонтальный угол поворота камеры в стратегическом режиме (стартовый угол 0 находится на юге карты)
    float m_Ang_Strategy = 0.0f;

    SAutoFlyData* m_AFD = nullptr;

    //D3DXVECTOR2 m_XY_Strategy = { 0.0f, 0.0f };  // used in permode moving

    //D3DXVECTOR3 m_Target = { 0.0f, 0.0f, 0.0f };
    //D3DXVECTOR3 m_TargetDisp = { 0.0f, 0.0f, 0.0f };

    //float m_NextMoveTime = 0.0f;

    //float m_LandRelativeZ = 0.0f;

    //float m_DistArcadedStart = 0.0f;
    //float m_AngleArcadedStart = 0.0f;


    __forceinline float LerpDist(int index = -1)
    {
        if(index < 0) index = m_ModeIndex;
        SCamParam* cp = g_Config.m_CamParams+index;
        return LERPFLOAT(m_DistParam[m_ModeIndex], cp->m_CamDistMin, cp->m_CamDistMax);
    }

    __forceinline float LerpAng(int index = -1)
    {
        if(index < 0) index = m_ModeIndex;
        SCamParam* cp = g_Config.m_CamParams + index;
        return LERPFLOAT(m_AngleParam[m_ModeIndex], cp->m_CamRotAngleMin, cp->m_CamRotAngleMax);
    }

    void CalcLinkPoint(D3DXVECTOR3& lp, float& angz);

    friend struct SAutoFlyData;

public:
    //Для вычисления смещения камеры от центра точки центровки с учётом её текущего горизонтального угла
    //dist - дальность необходимого смещения
    //par_type - смещение по какой координате вернуть: 0 - X, !0 - Y
    float CamAngleToCoordOffset(double dist, int par_type)
    {
        //Для перевода радиан в градусы
        //double ang = m_Ang_Strategy / 0.0174533;
        //ang = int(ang) % 360;
        //if(ang < 0) ang += 360.0;
        //Обратите внимание, переводим угол из дабологики (где начало окружности находится на юге и продвигается по часовой стрелке) в нормальную систему
        //if(ang <= 270.0) ang = fabs(ang - 270.0);
        //else ang = 360.0 - ang + 270.0;

        //Вычисляем смещение по X
        //if(par_type) return sin(ang * PI / 180.0) * dist;
        //Вычисляем смещение по Y
        //else return cos(ang * PI / 180.0) * dist;

        if(!par_type) return -sin(m_Ang_Strategy) * dist;
        else return cos(m_Ang_Strategy) * dist;
    }

    CMatrixCamera();
    ~CMatrixCamera();

    __forceinline float GetResXInversed() const { return _res_x_inversed; }
    __forceinline float GetResYInversed() const { return _res_y_inversed; }

    __forceinline void  SetAngleParam(float p) { m_AngleParam[m_ModeIndex] = p; }

    __forceinline float GetAngleX() const { return m_AngleX; }
    __forceinline float GetAngleZ() const { return m_AngleZ; }
    __forceinline void  SetAngleZ(float angle) { m_AngleZ = angle; }
    //__forceinline void RotateZ(float da) { m_AngleZ += da; }
    //__forceinline void RotateX(float angle) { m_AngleX += angle; }

    void RotateByMouse(int dx, int dy)
    {
        if(m_ModeIndex == CAMERA_STRATEGY)
        {
            m_Ang_Strategy += g_Config.m_CamParams[m_ModeIndex].m_CamRotSpeedZ * dx * 10;
        }

        m_AngleParam[m_ModeIndex] -= g_Config.m_CamParams[m_ModeIndex].m_CamRotSpeedX * dy * 5;
        if(m_AngleParam[m_ModeIndex] > 1.0f) m_AngleParam[m_ModeIndex] = 1.0f;
        if(m_AngleParam[m_ModeIndex] < 0.0f) m_AngleParam[m_ModeIndex] = 0.0f;
    }

    void InitStrategyAngle(float ang)
    {
        m_Ang_Strategy = (float)AngleNorm(g_Config.m_CamBaseAngleZ + ang);
        m_AngleZ = m_Ang_Strategy;
    }

    void CalcSkyMatrix(D3DXMATRIX& m);

    __forceinline void SetXYStrategy(const D3DXVECTOR2& pos)
    {
        SETFLAG(m_Flags, CAM_XY_LERP_OFF);
        m_XY_Strategy = pos;
    }

    __forceinline const D3DXVECTOR2& GetXYStrategy() const { return m_XY_Strategy; }
    //void SetTarget(const D3DXVECTOR3 & pos) { RESETFLAG(m_Flags, CAM_RESTOREXY); m_Target = pos;}
    const D3DXVECTOR3& GetLinkPoint() const { return m_LinkPoint; }
    //float GetZRel() const { return m_LandRelativeZ; }
    //void SetZRel(float z) { m_LandRelativeZ = z; }

    __forceinline const D3DXVECTOR3& GetDir() const { return *(D3DXVECTOR3*)&m_MatViewInversed._31; }
    __forceinline const D3DXVECTOR3& GetRight() const { return *(D3DXVECTOR3*)&m_MatViewInversed._11; }
    __forceinline const D3DXVECTOR3& GetUp() const { return *(D3DXVECTOR3*)&m_MatViewInversed._21; }

    void RotLeft() { SETFLAG(m_Flags, CAM_ACTION_ROT_LEFT); }
    void RotRight() { SETFLAG(m_Flags, CAM_ACTION_ROT_RIGHT); }
    void RotUp() { SETFLAG(m_Flags, CAM_ACTION_ROT_UP); }
    void RotDown() { SETFLAG(m_Flags, CAM_ACTION_ROT_DOWN); }

	void ZoomOutStep()
	{
		m_DistParam[m_ModeIndex] += g_Config.m_CamParams[m_ModeIndex].m_CamMouseWheelStep * 4.5f;//1.5f;
		if(m_DistParam[m_ModeIndex] > 4.0f) m_DistParam[m_ModeIndex] = 4.0f;
		if(m_DistParam[m_ModeIndex] < 0.25f) m_DistParam[m_ModeIndex] = 0.25f;
		//if(m_DistParam[m_ModeIndex] < 0.0f) m_DistParam[m_ModeIndex] = 0.0f;
	}

	void ZoomInStep()
	{
		m_DistParam[m_ModeIndex] -= g_Config.m_CamParams[m_ModeIndex].m_CamMouseWheelStep * 4.5f;//1.5f;
		if(m_DistParam[m_ModeIndex] > 4.0f) m_DistParam[m_ModeIndex] = 4.0f;
		if(m_DistParam[m_ModeIndex] < 0.25f) m_DistParam[m_ModeIndex] = 0.25f;
		//if(m_DistParam[m_ModeIndex] < 0.0f) m_DistParam[m_ModeIndex] = 0.0f;
	}

    void RotUpStep()
    {
        m_AngleParam[m_ModeIndex] += g_Config.m_CamParams[m_ModeIndex].m_CamMouseWheelStep;
        if(m_AngleParam[m_ModeIndex] > 1.0f) m_AngleParam[m_ModeIndex] = 1.0f;
        if(m_AngleParam[m_ModeIndex] < 0.0f) m_AngleParam[m_ModeIndex] = 0.0f;
    }
    void RotDownStep()
    {
        m_AngleParam[m_ModeIndex] -= g_Config.m_CamParams[m_ModeIndex].m_CamMouseWheelStep;
        if(m_AngleParam[m_ModeIndex] > 1.0f) m_AngleParam[m_ModeIndex] = 1.0f;
        if(m_AngleParam[m_ModeIndex] < 0.0f) m_AngleParam[m_ModeIndex] = 0.0f;
    }

    void MoveLeft() { SETFLAG(m_Flags, CAM_ACTION_MOVE_LEFT); }
    void MoveRight() { SETFLAG(m_Flags, CAM_ACTION_MOVE_RIGHT); }
    void MoveUp() { SETFLAG(m_Flags, CAM_ACTION_MOVE_UP); }
    void MoveDown() { SETFLAG(m_Flags, CAM_ACTION_MOVE_DOWN); }

    void ResetAngles();

    void CalcPickVector(const CPoint& p, D3DXVECTOR3& vdir) const;
    //void MoveCamera( const D3DXVECTOR3 & d) { m_Target += d; }

    void Stat()
    {
        if(m_AFD) m_AFD->Stat();
    }

    //bool IsLinked(void) const {return FLAG(m_Flags, CAM_LINKED);}

    void AddWarPair(CMatrixMapStatic* tgt, CMatrixMapStatic* attacker)
    {
        if(m_AFD) m_AFD->AddWarPair(tgt, attacker);
    }

    const D3DXMATRIX& GetViewMatrix()         const { return m_MatView; }
    const D3DXMATRIX& GetProjMatrix()         const { return m_MatProj; }
    const D3DXMATRIX& GetViewInversedMatrix() const { return m_MatViewInversed; }
    const D3DXVECTOR3& GetFrustumCenter()     const { return m_FrustumCenter; }
    const D3DXVECTOR3& GetFrustumLT()         const { return m_FrustumDirLT; }
    const D3DXVECTOR3& GetFrustumLB()         const { return m_FrustumDirLB; }
    const D3DXVECTOR3& GetFrustumRT()         const { return m_FrustumDirRT; }
    const D3DXVECTOR3& GetFrustumRB()         const { return m_FrustumDirRB; }
    const SPlane& GetFrustPlaneL()            const { return m_FrustPlaneL; }
    const SPlane& GetFrustPlaneR()            const { return m_FrustPlaneR; }
    const SPlane& GetFrustPlaneT()            const { return m_FrustPlaneT; }
    const SPlane& GetFrustPlaneB()            const { return m_FrustPlaneB; }

    void SetDrawNowParams(const D3DXMATRIX& iview, const D3DXVECTOR3& fc) { m_DN_iview = iview;  m_DN_fc = fc; }    // hack function
    const D3DXMATRIX& GetDrawNowIView() const { return m_DN_iview; }
    const D3DXVECTOR3& GetDrawNowFC() const { return m_DN_fc; }

    __forceinline D3DXVECTOR2 Project(const D3DXVECTOR3& pos, const D3DXMATRIX& world)
    {
        D3DXVECTOR3 out;
        D3DXVec3Project(&out, &pos, &m_ViewPort, &m_MatProj, &m_MatView, &world);
        return D3DXVECTOR2(out.x, out.y);
    }

    __forceinline D3DXVECTOR3 ProjectNorm(const D3DXVECTOR3& pos)
    {
        D3DXVECTOR3 out;
        D3DXMATRIX m(GetViewMatrix() * GetProjMatrix());
        D3DXVec3TransformCoord(&out, &pos, &m);
        out.z = GetViewMatrix()._13 * pos.x + GetViewMatrix()._23 * pos.y + GetViewMatrix()._33 * pos.z + GetViewMatrix()._43;
        return out;
    }

    void RestoreCameraParams();
    
    // visibility functions

    bool IsInFrustum(const D3DXVECTOR3& p) const;                  //point in frustum
    bool IsInFrustum(const D3DXVECTOR3& p, float radius) const;    // sphere in frustum
    bool IsInFrustum(const D3DXVECTOR3& mins, const D3DXVECTOR3& maxs) const; // check that box is in frustum
    float GetFrustPlaneDist(EFrustumPlane plane, const D3DXVECTOR3& pos, const D3DXVECTOR3& dir);
    float GetFrustPlaneMinDist(const D3DXVECTOR3& pos);

    void BeforeDraw();
    void Tact(float ms);
};

__forceinline void CMatrixCamera::CalcPickVector(const CPoint& p, D3DXVECTOR3& vdir) const
{
    D3DXVECTOR3 v;

    v.x = (((float)(p.x << 1) * _res_x_inversed) - 1.0f) * _mp_11_inversed;
    v.y = (((float)(p.y << 1) * _res_y_inversed) - 1.0f) * _mp_22_inversed;
    v.z = 1.0f;

    vdir.x = v.x*m_MatViewInversed._11 - v.y*m_MatViewInversed._21 + m_MatViewInversed._31;
    vdir.y = v.x*m_MatViewInversed._12 - v.y*m_MatViewInversed._22 + m_MatViewInversed._32;
    vdir.z = v.x*m_MatViewInversed._13 - v.y*m_MatViewInversed._23 + m_MatViewInversed._33;
    D3DXVec3Normalize(&vdir,&vdir);
}

__forceinline bool CMatrixCamera::IsInFrustum(const D3DXVECTOR3& p) const
{
    /*
    D3DXVECTOR3 tp =  p - GetFrustumCenter();
    if ( (tp.x * GetFrustPlaneL().norm.x + tp.y * GetFrustPlaneL().norm.y + tp.z * GetFrustPlaneL().norm.z) < 0.0f) return false;
    if ( (tp.x * GetFrustPlaneR().norm.x + tp.y * GetFrustPlaneR().norm.y + tp.z * GetFrustPlaneR().norm.z) < 0.0f) return false;
    if ( (tp.x * GetFrustPlaneT().norm.x + tp.y * GetFrustPlaneT().norm.y + tp.z * GetFrustPlaneT().norm.z) < 0.0f) return false;
    if ( (tp.x * GetFrustPlaneB().norm.x + tp.y * GetFrustPlaneB().norm.y + tp.z * GetFrustPlaneB().norm.z) < 0.0f) return false;
    */
    if(!GetFrustPlaneL().IsOnSide(p)) return false;
    if(!GetFrustPlaneR().IsOnSide(p)) return false;
    if(!GetFrustPlaneT().IsOnSide(p)) return false;
    if(!GetFrustPlaneB().IsOnSide(p)) return false;
    return true;
}

__forceinline bool CMatrixCamera::IsInFrustum(const D3DXVECTOR3& p, float r) const
{
    if(!GetFrustPlaneL().IsOnSide(p + GetFrustPlaneL().norm * r)) return false;
    if(!GetFrustPlaneR().IsOnSide(p + GetFrustPlaneR().norm * r)) return false;
    if(!GetFrustPlaneT().IsOnSide(p + GetFrustPlaneT().norm * r)) return false;
    if(!GetFrustPlaneB().IsOnSide(p + GetFrustPlaneB().norm * r)) return false;
    return true;
}


__forceinline bool CMatrixCamera::IsInFrustum(const D3DXVECTOR3& mins, const D3DXVECTOR3& maxs) const
{
    if(GetFrustPlaneL().BoxSide(mins, maxs) == 2) return false;
    if(GetFrustPlaneR().BoxSide(mins, maxs) == 2) return false;
    if(GetFrustPlaneB().BoxSide(mins, maxs) == 2) return false;
    if(GetFrustPlaneT().BoxSide(mins, maxs) == 2) return false;
    return true;
}


#endif