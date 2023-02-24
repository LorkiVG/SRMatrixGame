// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

class CCache;

enum CacheClass {
	cc_Unknown = 0,
	cc_Texture,
	cc_TextureManaged,
	cc_VO
};

class CCacheData : public CMain {
public:
	CCacheData* m_Prev = nullptr;
	CCacheData* m_Next = nullptr;
	CCache*     m_Cache = nullptr;
#ifdef _DEBUG
	CCacheData* d_Prev = nullptr;
	CCacheData* d_Next = nullptr;
	const char* d_file = nullptr;
	int         d_line = 0;
#endif

	CacheClass m_Type = cc_Unknown;
	CWStr m_Name = (CWStr)L"";

	int m_Ref = 0;			// Кол-во ссылок на эти данные. (Для временных данных)

    static bool m_dip;    // cache del in progress

public:
    static void StaticInit();

	CCacheData() = default;
    virtual ~CCacheData() {};

	void RefInc()		  { ++m_Ref; }
	void RefDec()		  { --m_Ref; ASSERT(m_Ref >= 0); }
	int  Ref()			  { return m_Ref; }
    bool RefDecUnload()	  { --m_Ref; if(m_Ref <= 0) { Unload(); return true; } return false; }

	void Prepare();

	void LoadFromFile(CBuf& buf, const wchar* exts = nullptr);

	virtual bool IsLoaded() = 0;
	virtual void Unload() = 0;
	virtual void Load() = 0;
};

class CCache : public CMain {
public:
	CCacheData* m_First = nullptr;
	CCacheData* m_Last = nullptr;

public:
	CCache();
	~CCache();

#ifdef _DEBUG
    static void Dump();
#endif

	void Add(CCacheData* cd);
	void Delete(CCacheData* cd);
	void Up(CCacheData* cd);

	void PreLoad();

	CCacheData* Find(CacheClass cc, const wchar* name);
	CCacheData* Get(CacheClass cc, const wchar* name);
#ifdef _DEBUG
	static CCacheData* Create(CacheClass cc, const char* file, int line);
#else
	static CCacheData* Create(CacheClass cc);
#endif
	static void Destroy(CCacheData* cd);

    void Clear();
};

#ifdef _DEBUG
#define CACHE_CREATE_VO() (CVectorObject *)CCache::Create(cc_VO, __FILE__, __LINE__)
#define CACHE_CREATE_TEXTURE() (CTexture *)CCache::Create(cc_Texture, __FILE__, __LINE__)
#define CACHE_CREATE_TEXTUREMANAGED() (CTextureManaged *)CCache::Create(cc_TextureManaged, __FILE__, __LINE__)
#else
#define CACHE_CREATE_VO() (CVectorObject*)CCache::Create(cc_VO)
#define CACHE_CREATE_TEXTURE() (CTexture*)CCache::Create(cc_Texture)
#define CACHE_CREATE_TEXTUREMANAGED() (CTextureManaged*)CCache::Create(cc_TextureManaged)
#endif


extern CCache* g_Cache;
extern CBlockPar* g_CacheData; // confusing name, but best of possible, imo
extern CHeap* g_CacheHeap;
extern const wchar* CacheExtsTex;

void CacheInit();
void CacheDeinit();

//bool CacheFileGet(CWStr& outname, const wchar* mname, const wchar* exts = nullptr, bool withpar = false);
void CacheReplaceFileExt(CWStr& outname, const wchar* mname, const wchar* ext = nullptr);
void CacheReplaceFileNameAndExt(CWStr& outname, const wchar* mname, const wchar* replname);