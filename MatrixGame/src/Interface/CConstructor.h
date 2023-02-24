// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once
#include "../MatrixObject.hpp"
#include "../MatrixRobot.hpp"
#include "../Logic/MatrixAIGroup.h"
#include <vector>

class CIFaceElement;

struct SPrice
{
    int m_Resources[MAX_RESOURCES] = { 0 }; //titan, electronics, energy, plasma

    void GiveRndPrice()
    {
        for(int cnt = 0; cnt < MAX_RESOURCES; ++cnt) m_Resources[cnt] = g_MatrixMap->Rnd(0, 50);
    }
    
    void SetPrice(ERobotModuleType type, ERobotModuleKind kind);
};

struct SModule {
    ERobotModuleType m_nType = MRT_EMPTY;
    ERobotModuleKind m_nKind = RUK_EMPTY;
    SPrice m_Price;
};

struct SHullModule {
	int		m_MaxWeaponsCnt = 0;
    SModule	m_Module;
};

struct SWeaponModule {
    byte m_ConstructorPylonNum = 0;
    //byte m_ModelPylonNum = 0; //Реальный номер пилона на модели корпуса, т.к. пилонов там может быть меньше максимального числа, а в конструкторе они перебираются полностью

    SModule m_Module;
};

struct SNewBorn {
    CMatrixRobotAI* m_Robot = nullptr;
    int             m_Team = -1;
};

struct SRobotTemplate {
    SModule         m_Head;
	SWeaponModule   m_Weapon[RUK_WEAPON_PYLONS_COUNT];
    SHullModule     m_Hull;
	SModule         m_Chassis;
    int             m_Team = -1;

    int             m_Resources[MAX_RESOURCES] = { 0 }; //titan, electronics, energy, plasma

    int             m_TemplatePriority = 0;     //Предварительно заданный в конфиге приоритет шаблона робота, нужен для ускорения сортировки шаблонов при выборе ИИ робота для постройки (также ИИ делает выбор исходя из стоимости и m_Strength шаблона)
    float           m_HitPointsOverride = 0.0f; //Параметр для принудительного выставления указанного количества HP для шаблона робота, загружаемого из конфига (если есть, игнорирует установленные модули)
    float           m_Strength = 0.0f;          //Средняя ударная сила шаблона робота, считается в момент его подгрузки из конфига, учитывает суммарный урон от орудий
    bool            m_HaveBomb = false;
    bool            m_HaveRepair = false;

    static std::vector<SRobotTemplate> m_AIRobotTypeList[TOTAL_SIDES + 2]; //Два дополнительных места резервируем для загрузки шаблонов спавнера и подкреплений

    static void AIRobotTypeVectorSort(std::vector<SRobotTemplate>& vector, int low, int high);
    static int AIRobotTypeVectorPartition(std::vector<SRobotTemplate>& vector, int low, int high);

    static void LoadAIRobotType(void* bp, int side_num = 0); //По умолчанию шаблоны роботов запоминаются в номер нейтральной стороны, которая считается общей (на случай, если в data.txt нет шаблонов для конкретной стороны)
    static void ClearAIRobotType(void);

    void CalcStrength(void);             // Расчитываем силу робота
    float DifWeapon(SRobotTemplate& other); // 0..1 на сколько отличается оружие у роботов

    bool CreateRobotTemplateFromPar(const CWStr& par_name, const CWStr& par_val);
    CMatrixRobotAI* CreateRobotByTemplate(const D3DXVECTOR3& pos, int side, bool map_prepare_in_process = false); //Последний маркер нужен только для ебаного костыля, т.к. в шаблонах стартовых роботов на картах сбита нумерация корпусов
};

void GetConstructionName(CMatrixRobotAI* robot);
int GetConstructionDamage(CMatrixRobotAI* robot);

class CConstructor : public CMain
{
	float             m_RobotPosX, m_RobotPosY;
	int               m_Side, m_ShadowType, m_ShadowSize, m_nModuleCnt,  m_ViewWidthX, m_ViewHeightY;
	float             m_ViewPosX, m_ViewPosY;
	int               m_nPos;
	CMatrixRobotAI*   m_Robot, *m_Build;
	
    SModule           m_Module[MR_MAX_MODULES];
	
    SModule           m_Head;
	SWeaponModule     m_Weapon[RUK_WEAPON_PYLONS_COUNT];
    SHullModule       m_Hull;
	SModule           m_Chassis;

	CMatrixBuilding*  m_Base;
    SNewBorn*         m_NewBorn;
	CWStr             m_ConstructionName;
	void InsertModules();
	void ResetConstruction();

public:
    class CIFaceButton* m_BaseConstBuildBut = nullptr; //Сюда заносится указатель на кнопку старта постройки роботов

    CMatrixRobotAI* GetRenderBot() { return m_Robot; }

    void GetConstructionPrice(int* res);
    int GetConstructionStructure();

    void SetSide(int side)
    {
        m_Side = side;
        if(m_Robot) m_Robot->m_Side = side;
    }
    int GetSide() { return m_Side; }

    void SetBase(CMatrixBuilding* pBase)
    {
        m_Base = pBase;
        //m_Side = side;
    }

    bool CheckMaxUnits() { return (m_nModuleCnt <= MR_MAX_MODULES); }

    void SetRenderProps(float x, float y, int width, int height)
    {
        m_ViewPosX = x;
        m_ViewPosY = y;
        m_ViewWidthX = width;
        m_ViewHeightY = height;
    }

	void __stdcall RemoteOperateModule(void* pObj);
	void OperateModule(
        ERobotModuleType type,
        ERobotModuleKind kind,
        int pylon = 0
    );
    void ReplaceRobotModule(
        ERobotModuleType type,
        ERobotModuleKind kind,
        int pylon,
        int model_pylon = 0,
        bool ld_from_history = false
    );
    void TempReplaceRobotModuleFromMenu(
        ERobotModuleType type,
        ERobotModuleKind kind,
        int pylon,
        int model_pylon = 0
    );

	void __stdcall RemoteBuild(void* pObj);
	SNewBorn* ProduceRobot(void* pObject);
    void AddRobotToBuildingQueue(
        void* pObject,
        int team = 0
    );
    void BeforeRender(void);

	void Render();

//STUB: FAKE FUNCTIONS MOTHERFUCKERS
    void BuildRandomBot()
	{
//Chassis
		int rnd = g_MatrixMap->Rnd(1, ROBOT_CHASSIS_COUNT);
		OperateModule(MRT_CHASSIS, (ERobotModuleKind)rnd);
//ARMOR
		rnd = g_MatrixMap->Rnd(1, ROBOT_HULLS_COUNT);
       
        OperateModule(MRT_HULL, (ERobotModuleKind)rnd);
//WEAPON
		rnd = g_MatrixMap->Rnd(1, g_Config.m_RobotHullsConsts[rnd].weapon_pylon_data.size());
		for(int nC = 0; nC <= rnd; nC++)
        {
			OperateModule(MRT_WEAPON, (ERobotModuleKind)g_MatrixMap->Rnd(1, ROBOT_WEAPONS_COUNT), nC);
		}
//HEAD
		OperateModule(MRT_HEAD, (ERobotModuleKind)g_MatrixMap->Rnd(1, ROBOT_HEADS_COUNT));
	}
    
    //STUB:
    void BuildRobotByTemplate(const SRobotTemplate& bot);

    void OperateCurrentConstruction(); 

	CConstructor();
	~CConstructor();
};


#define PRESETS 1

struct SRobotConfig
{
    SModule          m_Head;
    SModule          m_Weapon[RUK_WEAPON_PYLONS_COUNT];
    SModule          m_Chassis;
    SHullModule      m_Hull;

    int             m_titX;
    int             m_elecX;
    int             m_enerX;
    int             m_plasX;

    int             m_Structure;
    int             m_Damage;

    SRobotConfig*   m_NextConfig;
    SRobotConfig*   m_PrevConfig;

    SRobotConfig()
    {
        m_titX = 0;
        m_elecX = 0;
        m_enerX = 0;
        m_plasX = 0;
        m_Structure = 0;
        m_Damage = 0;
        m_NextConfig = nullptr;
        m_PrevConfig = nullptr;
    }
};


class CConstructorPanel : public CMain {
public:
    CIFaceElement*  m_FocusedElement;

    int             m_CurrentConfig;
    CWStr           m_FocusedLabel;
    CWStr           m_FocusedDescription;
    byte            m_Active;

    int             m_TitanResCountX;
    int             m_ElectronicsResCountX;
    int             m_EnergyResCountX;
    int             m_PlasmaResCountX;

    SRobotConfig    m_Configs[PRESETS];

    void ActivateAndSelect();
    void ResetGroupNClose();                                                     

    void ResetConfig()
    {
        ZeroMemory(&m_Configs, sizeof(m_Configs));
    }
    bool IsActive()
    {
        return m_Active == 1;
    }

    void ResetWeapon()
    {
        ZeroMemory(m_Configs[m_CurrentConfig].m_Weapon, sizeof(SModule) * RUK_WEAPON_PYLONS_COUNT);
        m_Configs[m_CurrentConfig].m_Damage = 0;
    }
    void __stdcall RemoteFocusElement(void* object);
    void __stdcall RemoteUnFocusElement(void* object);
    void FocusElement(CIFaceElement* element);
    void UnFocusElement(CIFaceElement* element);
    void SetLabelsAndPrice(
        ERobotModuleType type,
        ERobotModuleKind kind
    );

    bool IsEnoughResourcesForThisModule(
        int pilon,
        ERobotModuleType type,
        ERobotModuleKind kind,
        float cost_modify_by_head = 0.0f
    );

    void MakeItemReplacements(
        ERobotModuleType type,
        ERobotModuleKind kind
    );

    CConstructorPanel(): m_FocusedLabel(Base::g_MatrixHeap), m_FocusedDescription(Base::g_MatrixHeap)
    {
        m_CurrentConfig = 0;
        m_Active = 0; 
        ZeroMemory(&m_Configs, sizeof(m_Configs)); 
        m_Configs[m_CurrentConfig].m_Chassis.m_nType = MRT_CHASSIS; 
        m_Configs[m_CurrentConfig].m_Hull.m_Module.m_nType = MRT_HULL; 
        m_Configs[m_CurrentConfig].m_Head.m_nType = MRT_HEAD; 
        for(int cnt = 0; cnt < RUK_WEAPON_PYLONS_COUNT; cnt++)
        { 
            m_Configs[m_CurrentConfig].m_Weapon[cnt].m_nType = MRT_WEAPON;
        } 
        m_FocusedElement = nullptr; 
        m_TitanResCountX = 0; 
        m_ElectronicsResCountX = 0; 
        m_EnergyResCountX = 0; 
        m_PlasmaResCountX = 0;
    }
    ~CConstructorPanel() = default;
};


