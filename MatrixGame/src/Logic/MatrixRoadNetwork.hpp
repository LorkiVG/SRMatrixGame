// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "base.hpp"

class CMatrixCrotch;
class CMatrixRoadNetwork;

class CMatrixRoad : public Base::CMain {
public:
    CMatrixRoadNetwork* m_Parent = nullptr;
    CMatrixRoad* m_Prev = nullptr;
    CMatrixRoad* m_Next = nullptr;

    CMatrixCrotch* m_Start = nullptr;
    CMatrixCrotch* m_End = nullptr;

    int m_PathCnt = 0;
    Base::CPoint* m_Path = nullptr;

    int m_ZoneCnt = 0;
    int m_ZoneCntMax = 0;
    int* m_Zone = nullptr;

    int m_Dist = 0;

    byte m_Move = 0;

    int m_Data = 0; //Данные для расчета

public:
    CMatrixRoad() = default;
    ~CMatrixRoad();

    void DeleteCrotch(const CMatrixCrotch* crotch);

    void DeletePath();
    void LoadPath(const int cnt, const Base::CPoint* path);
    void AddPath(int x, int y);
    void AddPathFromRoad(CMatrixRoad* road);
    void CorrectStartEndByPath(void);

    void DeleteZone();
    void LoadZone(const int cnt, const int* path);
    void AddZone(int zone);
    int  GetIndexZone(int zone);
    void CorrectStartEndByZone();
    void SplitZone(int index, CMatrixRoad* road);
    void AddZoneFromRoad(CMatrixRoad* road);
    void DeleteZoneByIndex(int index);

    void DeleteCircleZone();
    bool CompareZone(CMatrixRoad* road);

    void MarkZoneRoad();

    void CalcDist();

    CMatrixCrotch* GetOtherCrotch(const CMatrixCrotch* crotch) { if(m_Start != crotch) return m_Start; return m_End; }
};

class CMatrixCrotch : public Base::CMain {
public:
    CMatrixRoadNetwork* m_Parent = nullptr;
    CMatrixCrotch* m_Prev = nullptr;
    CMatrixCrotch* m_Next = nullptr;

    Base::CPoint m_Center;
    int m_Zone = 0;

    int m_RoadCnt = 0;
    CMatrixRoad* m_Road[16] = { nullptr };

    int m_Island = 0;               // Номер острова
    int m_Region = 0;               // Номер региона. -1 - не в регионе
    int m_Data = 0;                 // Данные для расчета

    bool m_Select = false;
    bool m_Critical = false;        // Важная зона (рядом база, завод)

public:
    CMatrixCrotch();
    ~CMatrixCrotch();

    void DeleteRoad(const CMatrixRoad* road);
    void AddRoad(CMatrixRoad* road);

    int CalcCntRoadEqualMoveTypeAndNoDeadlock();
};

struct SMatrixMapZone {
	Base::CPoint m_Center;	        // Центр
    //bool m_Critical = false;      // Важная зона (рядом база, завод)
    //int m_Group = 0;              // Сгрупперованные зоны между узкими проходами
    //int m_Island = 0;             // Остров
	int m_Size = 0;					// Cnt in unit
	int m_Perim = 0;                // Периметр
	Base::CRect m_Rect;				// Bound zone
    byte m_Move = 0;                // (1-нельзя пройти) 1-Shasi1(Пневматика) 2-Shasi2(Колеса) 4-Shasi3(Гусеницы) 8-Shasi4(Подушка) 16-Shasi5(Крылья)
    bool m_Road = false;            // По этой зоне проходит дорога
    bool m_Bottleneck = false;      // Узкое место для всех видов шасси
    bool m_Access = false;

	int m_NearZoneCnt = 0;                  // Кол-во ближних зон
	int m_NearZone[64] = { 0 };             // Список ближних зон
	int m_NearZoneConnectSize[64] = { 0 };  // Длина связи между ближними зонами
    byte m_NearZoneMove[64] = { 0 };

    int m_PlaceCnt = 0;
    int m_Place[4] = { 0 };

    int m_Region = 0;

    dword m_Color = 0;
    dword m_ColorGroup = 0;

    //dword m_CriticalCnt = 0;

	int m_FPLevel = 0;
	int m_FPWt = 0;
	int m_FPWtp = 0;
};

struct SMatrixRoadRouteUnit {
    CMatrixRoad* m_Road = nullptr;
    CMatrixCrotch* m_Crotch = nullptr;
};

struct SMatrixRoadRouteHeader {
    int m_Cnt = 0;
    int m_Dist = 0;
};

class CMatrixRoadRoute : public Base::CMain {
public:
    CMatrixRoadNetwork* m_Parent = nullptr;
    CMatrixRoadRoute* m_Prev = nullptr;
    CMatrixRoadRoute* m_Next = nullptr;

    CMatrixCrotch* m_Start = nullptr;
    CMatrixCrotch* m_End = nullptr;

    int m_ListCnt = 0, m_ListCntMax = 0;
    SMatrixRoadRouteHeader* m_Header = nullptr;
    SMatrixRoadRouteUnit* m_Modules = nullptr;

public:
    CMatrixRoadRoute(CMatrixRoadNetwork* parent);
    ~CMatrixRoadRoute();

    void Clear();
    void ClearFast();

    void NeedEmpty(int cnt = 1, bool strong = false);

    int AddList();
    void CopyUnit(int listfrom, int listto);
    int AddUnit(int listno, CMatrixRoad* road, CMatrixCrotch* crotch);
};

struct SMatrixPlace {
    Base::CPoint m_Pos;
    byte m_Move = 0;
    byte m_BorderLeft = 0, m_BorderTop = 0, m_BorderRight = 0, m_BorderBottom, m_BorderCnt = 0;
    byte m_EdgeLeft = 0, m_EdgeTop = 0, m_EdgeRight = 0, m_EdgeBottom = 0, m_EdgeCnt = 0;
    bool m_Blockade[3 * 3] = { false }; // Непроходимая стена относительно места

    byte m_Cannon = 0;

    int m_Region = 0;

    byte m_NearCnt = 0;
    int m_Near[16] = { 0 };
    byte m_NearMove[16] = { 0 };

    // Нужны только в редакторе карт
    bool m_Empty = false;
    int m_EmptyNext = 0;
    int m_Rank = 0;

    // Нужны только в игре
    int m_Data = 0;
    byte m_Underfire = 0;
};

struct SMatrixPlaceList {
    int m_Sme = 0;
    int m_Cnt = 0;
};

struct SMatrixRegion {
    CPoint m_Center = { 0.0f, 0.0f };
    int m_RadiusPlace = 0;

    int m_CrotchCnt = 0;
    CMatrixCrotch* m_Crotch[16] = { nullptr };
    int m_PlaceCnt = 0;
    int m_Place[16] = { 0 };

	int m_PlaceAllCnt = 0;
	int m_PlaceAll[64] = { 0 };

    int m_NearCnt = 0;
    int m_Near[16] = { 0 };
    byte m_NearMove[16] = { 0 };

	int m_FPLevel = 0;
	int m_FPWt = 0;
	int m_FPWtp = 0;

    dword m_Color = 0;
};

class CMatrixRoadNetwork : public Base::CMain {
public:
    Base::CHeap* m_Heap = nullptr;

    int m_RoadCnt = 0;
    CMatrixRoad* m_RoadFirst = nullptr;
    CMatrixRoad* m_RoadLast = nullptr;
    CMatrixRoad** m_RoadFindIndex = nullptr;

    int m_CrotchCnt = 0;
    CMatrixCrotch* m_CrotchFirst = nullptr;
    CMatrixCrotch* m_CrotchLast = nullptr;
    CMatrixCrotch** m_CrotchFindIndex = nullptr;

    SMatrixMapZone* m_Zone = nullptr;
	int m_ZoneCnt = 0;
    int m_ZoneCntMax = 0;

    CMatrixRoadRoute* m_RouteFirst = nullptr;
    CMatrixRoadRoute* m_RouteLast = nullptr;

    int m_PlaceCnt = 0;
    int m_PlaceEmpty = 0;
    SMatrixPlace* m_Place = nullptr;

    int m_RegionCnt = 0;
    SMatrixRegion* m_Region = nullptr;
    int* m_RegionFindIndex = nullptr;


    SMatrixPlaceList* m_PLList = nullptr;
    int m_PLSizeX = 0, m_PLSizeY = 0;
    int m_PLShift = 0;
    int m_PLMask = 0;

    CMatrixRoadNetwork(Base::CHeap* he);
    ~CMatrixRoadNetwork();

    void Clear();

    CMatrixRoad* AddRoad();
    void DeleteRoad(CMatrixRoad* un);
    bool IsRoadExist(CMatrixRoad* un);
    CMatrixRoad* FindRoadByZone(int zone);

    CMatrixCrotch* AddCrotch();
    void DeleteCrotch(CMatrixCrotch* un);
    bool IsCrotchExist(CMatrixCrotch* un);
    CMatrixCrotch* FindCrotchByZone(int zone);

    void UnionRoad(dword flags = 1);
    bool UnionRoadStep(dword flags = 1); // 1-merge path 2-merge zone

    int SelectCrotchByRoadCnt(int amin, bool skipcritical = false); // Select crotch.m_PathCnt from 0 to amin
    void SelectCrotchSingleAndRadius(int radius);
    void DeleteSelectedCrotchAndRoad();

    void SplitRoad();

    void ClearZone();
    void SetZoneCntMax(int cnt);
    int FindNearZoneByCenter(Base::CPoint& p, int* dist2 = nullptr);
    void CalcZoneColor();

    void MarkZoneRoad();
    CMatrixCrotch* InsertCrotch(int zone);
    bool CreateCrotchAndRoadFromZone(int zone);
    bool CreateCrotchAndRoadByPath(int* path, int cnt);
    void MergeEqualCrotch();

    void CalcIsland();
    void SelectUnneededIsland();
    void Unselect();

    int CalcPathZoneByRoad(int zstart, int zend, int* path);
    int CalcDistByRoad(int zstart, int zend, int* path = nullptr);
    void CalcDistRoad();

    void DeleteCircleZone();
    void DeleteEqualRoad();

    void CreateAdditionalRoad();

    void FinalizeBuild();
    void FinalizePlace();

    bool IsValidate(bool exception = true);
    void Test();

    void ClearRoute();
    void DeleteRoute(CMatrixRoadRoute* route);
    CMatrixRoadRoute* AddRoute();
    CMatrixRoadRoute* CalcRoute(CMatrixCrotch* cstart, CMatrixCrotch* cend);
    CMatrixRoadRoute* FindRoute(CMatrixCrotch* cstart, CMatrixCrotch* cend);
    CMatrixRoadRoute* GetRoute(CMatrixCrotch* cstart, CMatrixCrotch* cend);

    void FindPathFromCrotchToRegion(byte mm, CMatrixCrotch* cstart, int region, CMatrixRoadRoute* rr, bool test);
    void FindPathFromRegionPath(byte mm, int rcnt, int* rlist, CMatrixRoadRoute* rr, bool test = false);

    void ClearPlace();
    SMatrixPlace* GetPlace(int no) { return m_Place + no; }
    void SetPlaceCnt(int cnt);
    int AllocPlace();
    void FreePlace(int no);

    void ClearRegion();
    int AddRegion();
    void DeleteRegion(int no);
    SMatrixRegion* GetRegion(int no) { return m_Region + no; }
    void CalcNearRegion(int no);
    void CalcCenterRegion(int no);
    int CalcRadiusRegion(int no);
    void CalcRadiusPlaceRegion(int no);
    bool IsNearestRegion(int r1, int r2);
    int FindNearestRegion(const CPoint& tp);
    int FindNearestRegionByRadius(const CPoint& tp, int curregion = -1);

    void FindPathInRegionInit();
    int FindPathInRegionRun(byte mm, int rstart, int rend, int* path, int maxpath, bool err = true);

    void ClearPL();
    void InitPL(int mapsizex, int mapsizey);
    CRect CorrectRectPL(const CRect& mapcoords);
    void ActionDataPL(const CRect& mapcoords, dword mask_and, dword mask_or = 0);
    int FindInPL(const CPoint& mappos);

	void Save(CBuf& b);
	void Load(CBuf& b, int ver);
};
