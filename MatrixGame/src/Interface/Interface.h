// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#define MAX_STATES 5

#include "../MatrixInstantDraw.hpp"

enum IFaceElementType
{
	IFACE_UNDEF = -1,
	IFACE_STATIC = 0,
	IFACE_ANIMATION = 1,
	IFACE_PUSH_BUTTON = 2,
	IFACE_ANIMATED_BUTTON = 3,
	IFACE_CHECK_BUTTON = 4,
    IFACE_DYNAMIC_STATIC = 5,
    IFACE_IMAGE = 6,
    IFACE_CHECK_BUTTON_SPECIAL = 7,
    IFACE_CHECK_PUSH_BUTTON = 8
};

//Возможные состояния элементов интерфейса (как правило, кнопок)
enum IFaceElementState
{
	IFACE_NORMAL = 0,
	IFACE_FOCUSED = 1,
	IFACE_PRESSED = 2,
	IFACE_DISABLED = 3,
	IFACE_PRESSED_UNFOCUSED = 4
};

enum EIFaceLabel
{
    IFACE_STATIC_LABEL = 0,
    IFACE_DYNAMIC_LABEL = 1,
    IFACE_STATE_STATIC_LABEL = 2
}; 

enum EActions
{
    ON_PRESS = 0,
    ON_UN_PRESS = 1,
    ON_FOCUS = 2,
    ON_UN_FOCUS = 3,
	ON_PRESS_RIGHT = 4,
	ON_UN_PRESS_RIGHT = 5,

	USER_ACTIONS_TOTAL
};

/*
struct SAction
{
	void* m_class;
	void* m_function;

	SAction() : m_class(nullptr), m_function(nullptr) {}
};
*/

struct SStateImages
{
	CTextureManaged* pImage = nullptr;
	SVert_V4_UV      m_Geom[4]; //По умолчанию инициализированы нулями
	float            xTexPos = 0.0f;
	float            yTexPos = 0.0f;
	float            TexWidth = 0.0f;
	float            TexHeight = 0.0f;

    int              m_x = 0;
    int              m_y = 0;
    int              m_boundX = 0;
    int              m_boundY = 0;
    int              m_xAlign = 0;
    int              m_yAlign = 0;
    int              m_Perenos = 0;
    int              m_SmeX = 0;
    int              m_SmeY = 0;
	CRect            m_ClipRect = { 0, 0, 0, 0 };
    CWStr            m_Caption = (CWStr)L"";
    CWStr            m_Font = (CWStr)L"";
    dword            m_Color = 0;

	bool             Set = false;

	//SStateImages() : m_Caption(Base::g_MatrixHeap), m_Font(Base::g_MatrixHeap) {}

	void SetStateText(bool copy);
	void SetStateLabelParams(
		int x,
		int y,
		int bound_x,
		int bound_y,
		int xAlign,
		int yAlign,
		int perenos,
		int smeX,
		int smeY,
		CRect clipRect,
		CWStr t,
		CWStr font,
		dword color
	);
};

/*
void* g_asm_iface_tmp;

#define FSET(dest, pObj, src) \
	dest.m_class = (void*)pObj; \
	g_asm_iface_tmp = dest.m_function; \
	__asm { mov eax, src } \
	__asm { mov g_asm_iface_tmp, eax } \
	dest.m_function = g_asm_iface_tmp;
*/

//Производит начальную настройку кнопки при подгрузке элементов интерфейса
#define FSET(act, pBut, cl, fn, pObj, src)														  \
      pBut->m_Actions[act].m_classfunction = static_cast<void (__stdcall CMain::*)(void*)>(&src); \
      pBut->m_Actions[act].m_object = pObj;

//Старый вариант
/*
#define FSET(act, pBut, cl, fn, pObj, src) \
	cl = (void*)pObj;					   \
	__asm { mov eax, offset src }		   \
	__asm { mov [fn], eax }				   \
	pBut->m_Actions[act].m_class = cl;	   \
	pBut->m_Actions[act].m_function = fn;
*/

#define FCALL(a, from)				 \
	__asm push	from				 \
	__asm mov	eax, (a)->m_class    \
	__asm push	eax					 \
	__asm mov	eax, (a)->m_function \
	__asm call	eax

#define FCALLFROMCLASS(a)			\
	__asm push	this				\
	__asm mov	eax, dword ptr a    \
	__asm add	eax, dword ptr this \
	__asm push	[eax]				\
	__asm add	eax, 4				\
	__asm mov	eax, [eax]			\
	__asm call	eax

#define FCALLFROMCLASS2(a)		 \
	__asm push	this			 \
	__asm mov	eax, dword ptr a \
	__asm push	[eax]			 \
	__asm add	eax, 4			 \
	__asm mov	eax, [eax]		 \
	__asm call	eax


/////////////////////////////////////////////////
extern IDirect3DDevice9* g_D3DD;
extern CHeap* g_MatrixHeap;
extern CCache* g_Cache;
////////////////////////////////////////////////