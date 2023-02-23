// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

class CIFaceStatic;
class CIFaceElement;

struct SFrame
{
    float pos_x = 0.0f;
    float pos_y = 0.0f;
    float pos_z = 0.0f;

    float width = 0.0f;
    float height = 0.0f;

    float tex_width = 0.0f;
    float tex_height = 0.0f;
    float tex_pos_x = 0.0f;
    float tex_pos_y = 0.0f;
    
    //CWStr name;
    CTextureManaged* tex = nullptr;
    float ipos_x = 0.0f;
    float ipos_y = 0.0f;
};

class CAnimation : public CMain
{
    int m_Period = 0;
    int m_Frames = 0;
    int m_CurrentFrame = 0;
    int m_FramesLoaded = 0;
    int m_TimePass = 0;

    CIFaceStatic* m_FramesBuffer = nullptr;

public:
    int GetFramesTotal()     { return m_Frames; }
    int GetFramesLoaded()    { return m_FramesLoaded; }
    int GetCurrentFrameNum() { return m_CurrentFrame; }
    CIFaceElement* GetCurrentFrame();                            

    void LogicTact(int ms);
    bool LoadNextFrame(SFrame* frame);

    void RecalcPos(const float& ix, const float& y);

    CAnimation(int frames, int period);
    ~CAnimation();
};