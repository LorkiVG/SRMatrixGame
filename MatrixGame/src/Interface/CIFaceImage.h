// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Interface.h"
#include "CIFaceElement.h"

////////////////////////////////////////////////////////
//Image container class

class CIFaceImage : public CMain {
public:
	CTextureManaged*  m_Image = nullptr;
    CWStr             m_strName = (CWStr)L"";
	float             m_xTexPos = 0.0f;
	float             m_yTexPos = 0.0f;
	float             m_TexWidth = 0.0f;
	float             m_TexHeight = 0.0f;
    float             m_Width = 0.0f;
    float             m_Height = 0.0f;
    IFaceElementType  m_Type = IFACE_IMAGE;
	CIFaceImage*	  m_PrevImage = nullptr;
	CIFaceImage*	  m_NextImage = nullptr;

    CIFaceImage() = default;
	~CIFaceImage() = default;
};