// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

namespace Base {


#define HASH_TABLE_SIZE 1024

typedef bool (*ENUM_DWORD)(dword key, dword val, dword user);

#define PTR2KEY(p) (((dword)p)>>3)

class CDWORDMap : public CMain
{
    typedef struct SPair
    {
        union
        {
            dword   key;
            dword   allocated;
        };
        union
        {
            dword   value;
            dword   contained;
        };
    } *PSPair;
    

    CHeap       *m_Heap;
    PSPair       m_Table[HASH_TABLE_SIZE];

    int          m_Cnt;
public:

    CDWORDMap(CHeap *heap = nullptr):m_Heap(heap) 
    {
        memset(&m_Table, 0, sizeof(m_Table)); 
        m_Cnt = 0;
    }
    ~CDWORDMap();

    int  GetCount(void) const {return m_Cnt;};
    void Set(dword key, dword v);
    bool Get(dword key, dword *v);
    bool Del(dword key);
    void Enum(ENUM_DWORD, dword user);
    void Clear(void);


};

__forceinline CDWORDMap::~CDWORDMap()
{
    Clear();
}

__forceinline void CDWORDMap::Clear(void)
{
    for (int i=0; i<HASH_TABLE_SIZE; ++i)
    {
        if (m_Table[i]) HFree(m_Table[i], m_Heap);
        m_Table[i] = nullptr;
    }
}

__forceinline void CDWORDMap::Set(dword key, dword v)
{
    dword index = key & (HASH_TABLE_SIZE-1);
    PSPair vv = m_Table[index];
    if (vv == nullptr)
    {
        //not present
        vv = m_Table[index] = (PSPair)HAlloc(16 * sizeof(SPair), m_Heap);

        vv->allocated = 15;
        vv->contained = 1;

        ++vv;

        vv->key = key;
        vv->value = v;

#ifdef _DEBUG
        ++m_Cnt;
#endif

    } else
    {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv)
        {
            if (vvseek->key == key)
            {
                vvseek->value = v;
                return;
            }
            --vvseek;
        }

        ++m_Cnt;

        if ( vv->contained < vv->allocated)
        {
            ++vv->contained;
            (vv + vv->contained)->key = key;
            (vv + vv->contained)->value = v;

        } else
        {
            int newsize = vv->allocated + 16 + 1;
            vv = m_Table[index] = (PSPair)HAllocEx(vv, newsize * sizeof(SPair), m_Heap);

            vv->allocated = newsize - 1;
            ++vv->contained;
            (vv + vv->contained)->key = key;
            (vv + vv->contained)->value = v;
        }

    }
 
}

__forceinline bool CDWORDMap::Get(dword key, dword *v)
{
    dword index = key & (HASH_TABLE_SIZE-1);
    PSPair vv = m_Table[index];
    if (vv == nullptr)
    {
        //not present

        return false;

    } else
    {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv)
        {
            if (vvseek->key == key)
            {
                if (v) *v = vvseek->value;
                return true;
            }
            --vvseek;
        }
        return false;
    }
}

__forceinline bool CDWORDMap::Del(dword key)
{
    dword index = key & (HASH_TABLE_SIZE-1);
    PSPair vv = m_Table[index];
    if (vv == nullptr)
    {
        //not present

        return false;

    } else
    {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv)
        {
            if (vvseek->key == key)
            {
                *vvseek = *(vv + vv->contained);
                --vv->contained;
                --m_Cnt;
                return true;
            }
            --vvseek;
        }
        return false;
    }
}

}