// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once
#include "Interface.h"
#include "../MatrixMap.hpp"
#include "../MatrixObject.hpp"
#include "../MatrixRobot.hpp"

class CAnimation;


//////////////////////////////////////////////////
//Interface Element Class

#define IFEF_VISIBLE    SETBIT(0)
#define IFEF_CLEARRECT  SETBIT(1)

struct SElementHint
{
    CWStr HintTemplate = (CWStr)L"";
    int   timer = 0;
    int   x = 0;
    int   y = 0;
};

class CIFaceElement : public CMain {
	dword m_Flags = 0;

public:
    CRect             m_ClearRect = { 0, 0, 0, 0 };    // rect for clearing Z buffer
    SElementHint      m_Hint;
    CAnimation*       m_Animation = nullptr;
    byte              m_VisibleAlpha = 255; //IS_VISIBLE
    SStateImages      m_StateImages[MAX_STATES];
	CWStr             m_strName = (CWStr)L"";
	float             m_Param1 = 0.0f;
	float             m_Param2 = 0.0f;
    int               m_iParam = 0;
	int               m_nId = 0;
	int               m_nGroup = 0;

	IFaceElementType  m_Type = IFACE_UNDEF;
    float             m_xPos = 0.0f;
    float             m_yPos = 0.0f;
    float             m_zPos = 0.0f;
    float             m_xSize = 0.0f;
    float             m_ySize = 0.0f;
    float             m_PosElInX = 0.0f;
    float             m_PosElInY = 0.0f;

	CIFaceElement*    m_NextElement = nullptr;
    CIFaceElement*    m_PrevElement = nullptr;

    struct SAction
    {
        void (*m_nonclassfunction)(void) = nullptr;

        void (__stdcall CMain::* m_classfunction)(void*) = nullptr;
        void* m_object = nullptr;
    };
    SAction m_Actions[USER_ACTIONS_TOTAL];
	
    IFaceElementState m_CurState = IFACE_NORMAL;
	IFaceElementState m_DefState = IFACE_NORMAL;
	

    CIFaceElement();
    ~CIFaceElement();

    void Action(EActions action);                                                                
	//CSound m_StateSounds; //MAX_STATES
	//bool SetStateSound(IFaceElementState State, CSound Sound);

    IFaceElementState GetState();
	bool SetState(IFaceElementState State);
    bool SetStateImage(IFaceElementState State, CTextureManaged* pImage, float x, float y, float width, float height);
	LPDIRECT3DTEXTURE9 GetStateImage(IFaceElementState State);

	bool GetVisibility() const { return FLAG(m_Flags, IFEF_VISIBLE); }
    void SetVisibility(bool visible);                                                

    bool HasClearRect() const  { return FLAG(m_Flags, IFEF_CLEARRECT); }
    void SetClearRect()        { SETFLAG(m_Flags, IFEF_CLEARRECT); }

	bool ElementCatch(CPoint);
    bool ElementAlpha(CPoint mouse);

    virtual void CheckGroupReset(CIFaceElement*, CIFaceElement*);
    virtual void ElementGeomInit(void* pObj, bool full_size = false);
	virtual bool OnMouseMove(CPoint) { return false; }
	virtual void OnMouseLBUp()       { }
    virtual bool OnMouseLBDown()     { return false; }
    virtual void OnMouseRBUp()       { }
    virtual bool OnMouseRBDown()     { return false; }
    virtual void BeforeRender();
	virtual void Render(byte m_VisibleAlpha);
	virtual void Reset();
    void LogicTact(int ms);

    void RecalcPos(const float& x, const float& y, bool ichanged = true);

    //void GenerateClearRect();
};