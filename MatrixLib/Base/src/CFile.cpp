// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "CFile.hpp"
#include "CException.hpp"
//#include "Cache.hpp"

namespace Base {

#ifndef MAXEXP_EXPORTS
CPackCollection* CFile::m_Packs;
int              CFile::m_PacksRef;

void CFile::AddPackFile(const wchar* name, CHeap* heap)
{
    if(m_Packs == nullptr) m_Packs = HNew(heap) CPackCollection(heap);
    m_Packs->AddPacketFile(name);
}

void CFile::OpenPackFiles(void)
{
    if(m_Packs)
    {
        if(!m_Packs->OpenPacketFiles()) ReleasePackFiles();
    }
}

void CFile::ReleasePackFiles(void)
{
    if(m_Packs)
    {
        ASSERT(m_PacksRef == 0);

        HDelete(CPackCollection, m_Packs, m_Packs->m_Heap);
        m_Packs = nullptr;
    }
}
#endif

CFile::CFile(CHeap* heap) : CMain(), m_FileName(heap)
{
#ifndef MAXEXP_EXPORTS 
    m_PackHandle = 0xFFFFFFFF;
#endif
    m_Handle = INVALID_HANDLE_VALUE;
    m_Open = 0;
}

CFile::CFile(const CWStr& filename, CHeap* heap) : CMain(), m_FileName(heap)
{
#ifndef MAXEXP_EXPORTS 
    m_PackHandle = 0xFFFFFFFF;
#endif
    m_Handle = INVALID_HANDLE_VALUE;
    m_Open = 0;
    Init(filename);
}

CFile::CFile(const wchar* filename, CHeap* heap) : CMain(), m_FileName(heap)
{
#ifndef MAXEXP_EXPORTS 
    m_PackHandle = 0xFFFFFFFF;
#endif
    m_Handle = INVALID_HANDLE_VALUE;
    m_Open = 0;
    Init(filename);
}

CFile::CFile(const wchar* filename, int len, CHeap* heap) : CMain(), m_FileName(heap)
{
#ifndef MAXEXP_EXPORTS 
    m_PackHandle = 0xFFFFFFFF;
#endif
    m_Handle = INVALID_HANDLE_VALUE;
    m_Open = 0;
    Init(filename, len);
}

CFile::~CFile()
{
	Clear();
}

void CFile::Clear(void)
{
    m_Open = 0;
    Close();
    m_FileName.Clear();
}

void CFile::Init(const wchar* filename, int len)
{
    Clear();
    m_FileName.Set(filename, len);
}

void CFile::Open(dword shareMode)
{
	if(!m_Open)
    {
        if(IS_UNICODE()) m_Handle = CreateFileW((const wchar*)m_FileName, GENERIC_READ | GENERIC_WRITE, shareMode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        else m_Handle = CreateFileA(CStr(m_FileName, m_FileName.GetHeap()).Get(), GENERIC_READ | GENERIC_WRITE, shareMode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if(m_Handle == INVALID_HANDLE_VALUE)
        {
			ERROR_S2(L"Error open file: ", m_FileName.Get());
		}
	}

	++m_Open;
}



bool CFile::OpenRead(dword shareMode, bool crash_on_fail_to_load)
{
	if(!m_Open)
    {
        if(IS_UNICODE()) m_Handle = CreateFileW(m_FileName.Get(), GENERIC_READ, shareMode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        else m_Handle = CreateFileA(CStr(m_FileName, m_FileName.GetHeap()).Get(), GENERIC_READ, shareMode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

#ifndef MAXEXP_EXPORTS
		if(m_Handle == INVALID_HANDLE_VALUE)
        {
            //So, real file not found. Maybe it is in package?
            if(m_Packs) m_PackHandle = m_Packs->Open(CStr(m_FileName, m_FileName.GetHeap()));

            if(m_PackHandle == 0xFFFFFFFF)
            {
                if(crash_on_fail_to_load)
                {
                    ERROR_S2(L"Error open file: ", m_FileName.Get());
                }
                else return false;
            }
            else ++m_PacksRef;
        }
#endif
	}

	++m_Open;
    return true;
}

bool CFile::OpenReadNE(dword shareMode)
{
	if(!m_Open)
    {
        if(IS_UNICODE()) m_Handle = CreateFileW(m_FileName.Get(), GENERIC_READ, shareMode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        else m_Handle = CreateFileA(CStr(m_FileName, m_FileName.GetHeap()).Get(), GENERIC_READ, shareMode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if(m_Handle == INVALID_HANDLE_VALUE) return false;
	}

	++m_Open;
	return true;
}

void CFile::Create(dword shareMode)
{
    if(!m_Open)
    {
        if(IS_UNICODE()) m_Handle = CreateFileW(m_FileName.Get(), GENERIC_READ | GENERIC_WRITE, shareMode, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        else m_Handle = CreateFileA(CStr(m_FileName, m_FileName.GetHeap()).Get(), GENERIC_READ | GENERIC_WRITE, shareMode, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if(m_Handle == INVALID_HANDLE_VALUE) ERROR_S2(L"Error creating file: ", m_FileName.Get());
    }

	++m_Open;
}

bool CFile::CreateNE(dword shareMode)
{
    if(!m_Open)
    {
        if(IS_UNICODE()) m_Handle = CreateFileW(m_FileName.Get(), GENERIC_READ | GENERIC_WRITE, shareMode, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        else m_Handle = CreateFileA(CStr(m_FileName, m_FileName.GetHeap()).Get(), GENERIC_READ | GENERIC_WRITE, shareMode, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if(m_Handle == INVALID_HANDLE_VALUE) return false;
    }

	++m_Open;
	return true;
}

void CFile::Close(void)
{
	--m_Open;
	if(m_Open <= 0)
    {
#ifndef MAXEXP_EXPORTS
        if(m_PackHandle != 0xFFFFFFFF)
        {
            m_Packs->Close(m_PackHandle);
            m_PackHandle = 0xFFFFFFFF;
            --m_PacksRef;
        }
#endif
		if(m_Handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_Handle);
            m_Handle = INVALID_HANDLE_VALUE;
        }

		m_Open = 0;
	}
}

dword CFile::Size(void) const
{
	dword lo = 0xFFFFFFFF;

#ifndef MAXEXP_EXPORTS
	if(m_PackHandle != 0xFFFFFFFF) lo = m_Packs->GetSize(m_PackHandle);
#endif

    if(m_Handle != INVALID_HANDLE_VALUE) lo = GetFileSize(m_Handle, nullptr);

	if(lo == 0xFFFFFFFF)
    {
		ERROR_S(L"Error getting file size");
	}

	return lo;
}

__int64 CFile::SizeFull(void) const
{
	dword lo = 0xFFFFFFFF, hi;

#ifndef MAXEXP_EXPORTS
	if(m_PackHandle != 0xFFFFFFFF)
    {
        lo = m_Packs->GetSize(m_PackHandle);
        hi = 0;
    }
#endif

    if(m_Handle != INVALID_HANDLE_VALUE) lo = GetFileSize(m_Handle, &hi);

	if(lo == 0xFFFFFFFF)
    {
		ERROR_S(L"Error getting file size");
	}

    return __int64(dword(lo)) | (__int64(hi) << 32);
}

__int64 CFile::PointerFull(void) const
{
	dword lo = 0xFFFFFFFF;
    LONG hi;

#ifndef MAXEXP_EXPORTS
	if(m_PackHandle != 0xFFFFFFFF)
    {
        lo = m_Packs->GetPos(m_PackHandle);
        hi = 0;
    }
#endif

    if(m_Handle != INVALID_HANDLE_VALUE)
    {
        lo = SetFilePointer(m_Handle, 0, &hi, FILE_CURRENT);
    }

	if(lo == 0xFFFFFFFF)
    {
		ERROR_S(L"Error getting file pointer");
	}

    return __int64(dword(lo)) | (__int64(hi) << 32);
}

void CFile::PointerFull(__int64 zn, int from) const
{
    LONG lo = LONG(zn & 0xffffffff), hi = dword(zn >> 32);

#ifndef MAXEXP_EXPORTS
	if(m_PackHandle != 0xFFFFFFFF)
    {
        lo = m_Packs->SetPos(m_PackHandle, lo, from);
    }
#endif

    if(m_Handle != INVALID_HANDLE_VALUE) lo = SetFilePointer(m_Handle, lo, &hi, from);

	if(lo == 0xFFFFFFFF)
    {
		ERROR_S(L"Error setting file pointer");
	}
}

dword CFile::Pointer(void) const
{
	dword lo = 0xFFFFFFFF;

#ifndef MAXEXP_EXPORTS
	if(m_PackHandle != 0xFFFFFFFF)
    {
        lo = m_Packs->GetPos(m_PackHandle);
    }
#endif

    if(m_Handle != INVALID_HANDLE_VALUE) lo = SetFilePointer(m_Handle, 0, nullptr, FILE_CURRENT);

	if(lo == 0xFFFFFFFF)
    {
		ERROR_S(L"Error getting file pointer");
	}
    return lo;
}

void CFile::Pointer(dword lo, int from) const
{
#ifndef MAXEXP_EXPORTS
	if(m_PackHandle != 0xFFFFFFFF) lo = m_Packs->SetPos(m_PackHandle, lo, from);
#endif

    if(m_Handle != INVALID_HANDLE_VALUE) lo = SetFilePointer(m_Handle, lo, nullptr, from);

	if(lo == 0xFFFFFFFF)
    {
		ERROR_S(L"Error setting file pointer");
	}
}

void CFile::Read(void* buf, dword kolbyte)
{
    bool ok = false;

#ifndef MAXEXP_EXPORTS
	if(m_PackHandle != 0xFFFFFFFF) ok = m_Packs->Read(m_PackHandle, buf, kolbyte);
#endif

    if(m_Handle != INVALID_HANDLE_VALUE)
    {
        dword temp;
        ok = ReadFile(m_Handle, buf, kolbyte, &temp, nullptr) != FALSE;
        ok &= (temp == (dword)kolbyte);
    }

	if(!ok)
    {
		ERROR_S(CWStr().Format(L"Error read file: <s> cnt=<i>",m_FileName.Get(),kolbyte).Get());
	}
}

void CFile::Write(void* buf, dword kolbyte)
{
    if(kolbyte <= 0) return;

    ASSERT(m_Handle != INVALID_HANDLE_VALUE);

	dword temp;
    if (WriteFile(m_Handle, buf, kolbyte, &temp, nullptr) == FALSE || temp != kolbyte)
    {
        ERROR_S(CWStr().Format(L"Error write file: <s> cnt=<i>", m_FileName.Get(), kolbyte).Get());
    }
}








static bool FileExistA(CWStr& outname, const wchar* mname, const wchar* exts, bool withpar)
{
DTRACE();

    int len = WStrLen(mname);
    const wchar* str = mname;

    int lenfile = 0; while (lenfile < len && str[lenfile] != '?') lenfile++;

    CWStr filename(str, lenfile, outname.GetHeap());

    WIN32_FIND_DATAA fd;
    HANDLE fh = FindFirstFileA(CStr(filename).Get(), &fd);
    if(fh != INVALID_HANDLE_VALUE)
    {
        FindClose(fh);
        if(withpar) outname = mname;
        else outname = filename; return true;
    }

    fh = FindFirstFileA(CStr(CWStr(str, lenfile, outname.GetHeap()) + L".*", outname.GetHeap()).Get(), &fd);
    if(fh == INVALID_HANDLE_VALUE) return false;
    if(exts != nullptr)
    {
        CWStr curname(outname.GetHeap());
        while (true)
        {
            curname.Set(CStr(fd.cFileName));
            int sme = curname.FindR(L'.') + 1;
            if(sme > 0 && sme < curname.GetLen())
            {
                curname.LowerCase(sme);
                const wchar* str = curname.Get() + sme;
                int len = curname.GetLen() - sme;

                const wchar* exts2 = exts;
                int cntok = 0;
                int lenext = 0;
                while(true)
                {
                    if(*exts2 == 0 || *exts2 == L'~')
                    {
                        if(cntok == len && lenext == len) break;
                        cntok = 0;
                        lenext = -1;
                        if(*exts2 == 0) break;
                    }
                    else if(*exts2 == str[cntok]) ++cntok;
                    ++exts2;
                    ++lenext;
                }
                if(cntok == len) break;
            }

            if(!FindNextFileA(fh, &fd))
            {
                FindClose(fh);
                return false;
            }
        }
    }
    FindClose(fh);

    int lenpath = lenfile; while (lenpath > 0 && str[lenpath - 1] != '\\' && str[lenpath - 1] != '/') --lenpath;

    if(lenpath > 0)
    {
        outname.Set(str, lenpath);
        outname.Add(CWStr(CStr(fd.cFileName, outname.GetHeap())));
    }
    else outname.Set(CStr(fd.cFileName, outname.GetHeap()));

    if(withpar && lenfile < len) outname.Add(str + lenfile, len - lenfile);

    return true;
}

static bool FileExistW(CWStr& outname, const wchar* mname, const wchar* exts, bool withpar)
{
DTRACE();

    int len = WStrLen(mname);
    const wchar* str = mname;

    int lenfile = 0; while (lenfile < len && str[lenfile] != '?') lenfile++;

    CWStr filename(str, lenfile, outname.GetHeap());

    WIN32_FIND_DATAW fd;
    HANDLE fh = FindFirstFileW(filename, &fd);
    if(fh != INVALID_HANDLE_VALUE)
    {
        FindClose(fh);
        if(withpar) outname = mname;
        else outname = filename;
        return true;
    }

    fh = FindFirstFileW((CWStr(str, lenfile, outname.GetHeap()) + L".*").Get(), &fd);
    if(fh == INVALID_HANDLE_VALUE) return false;
    if(exts != nullptr)
    {
        CWStr curname(outname.GetHeap());
        while(true)
        {
            curname.Set(fd.cFileName);
            int sme = curname.FindR(L'.') + 1;
            if(sme > 0 && sme < curname.GetLen())
            {
                curname.LowerCase(sme);
                const wchar* str = curname.Get() + sme;
                int len = curname.GetLen() - sme;

                const wchar* exts2 = exts;
                int cntok = 0;
                int lenext = 0;
                while(true)
                {
                    if(*exts2 == 0 || *exts2 == L'~')
                    {
                        if(cntok == len && lenext == len) break;
                        cntok = 0;
                        lenext = -1;
                        if(*exts2 == 0) break;
                    }
                    else if(*exts2 == str[cntok]) ++cntok;
                    ++exts2;
                    ++lenext;
                }

                if(cntok == len) break;
            }

            if(!FindNextFileW(fh, &fd))
            {
                FindClose(fh);
                return false;
            }
        }
    }
    FindClose(fh);

    int lenpath = lenfile;
    while(lenpath > 0 && str[lenpath - 1] != '\\' && str[lenpath - 1] != '/') --lenpath;

    if(lenpath > 0)
    {
        outname.Set(str, lenpath);
        outname.Add(fd.cFileName);
    }
    else outname.Set(fd.cFileName);

    if(withpar && lenfile < len) outname.Add(str + lenfile, len - lenfile);

    return true;
}

bool CFile::FileExist(CWStr& outname, const wchar* mname, const wchar* exts, bool withpar)
{
DTRACE();

    if(mname[0] == '.' && (mname[1] == '\\' || mname[1] == '/')) mname += 2;

	if(IS_UNICODE())
    {
        if(FileExistW(outname, mname, exts, withpar)) return true;
    }
	else if(FileExistA(outname, mname, exts, withpar)) return true;

#ifndef MAXEXP_EXPORTS
    // real file not found... may be it is in pack file

    if(!m_Packs) return false; 

	int len = WStrLen(mname);
	const wchar* str = mname;

	int lenfile = 0;
    while(lenfile < len && str[lenfile] != '?') ++lenfile;

	CWStr filename(str, lenfile, outname.GetHeap());

    if(m_Packs->FileExists(CStr(filename, outname.GetHeap())))
    {
        if(withpar)  outname=mname; else outname=filename;
        return true;
    }

    if(!exts) return false;

    int sm0 = 0;
    int sm1 = 0;

    int l = WStrLen(exts);

    CStr fn(outname.GetHeap());
    for(; sm1 <= l; ++sm1)
    {
        if(exts[sm1] == '~' || exts[sm1] == 0)
        {
            if(sm0 != sm1)
            {
                fn.Set(filename);
                fn += ".";
                fn += CStr(exts + sm0, outname.GetHeap());
                fn.SetLen(fn.Len() - (l - sm1));

                if(m_Packs->FileExists(fn))
                {
                    outname.Set(fn);
                    if(withpar) outname += (mname + lenfile);
                    return true;
                }
            }
            sm0 = sm1 + 1;
        }
    }
#endif

    return false;
}


void CFile::FindFiles(const CWStr& folderfrom, const wchar* files, ENUM_FILES ef, dword user)
{
    if(IS_UNICODE())
    {
        CWStr fn(folderfrom, folderfrom.GetHeap());
        if(fn.GetLen() > 0 && !(*(fn.Get() + fn.GetLen() - 1) == '\\' || (*(fn.Get() + fn.GetLen() - 1) == '/')))
        {
            fn.Add(L"\\");
        }
        CWStr fnf(fn, folderfrom.GetHeap());

        WIN32_FIND_DATAW fd;

        // seek files
        fnf += files;
        HANDLE h = FindFirstFileW(fnf.Get(), &fd);
        if(h != INVALID_HANDLE_VALUE)
        {
            do
            {
                if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    CWStr   found(fd.cFileName, folderfrom.GetHeap());
                    ef(fn + found, user);
                }
            } while(FindNextFileW(h, &fd));
            FindClose(h);
        }

        fnf = fn + L"*.*";
        h = FindFirstFileW(fnf.Get(), &fd);
        if(h != INVALID_HANDLE_VALUE)
        {
            do
            {
                if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    CWStr   found(fd.cFileName, folderfrom.GetHeap());
                    if(found != L"." && found != L"..")
                    {
                        FindFiles(fn + found, files, ef, user);
                    }
                }
            } while(FindNextFileW(h, &fd));
            FindClose(h);
        }
    }
    else
    {
        CStr fn(folderfrom, folderfrom.GetHeap());
        if(fn.GetLen() > 0 && !(*(fn.Get() + fn.GetLen() - 1) == '\\' || (*(fn.Get() + fn.GetLen() - 1) == '/')))
        {
            fn.Add("\\");
        }
        CStr fnf(fn, folderfrom.GetHeap());

        WIN32_FIND_DATAA fd;

        // seek files
        fnf += CStr(files, folderfrom.GetHeap());
        HANDLE h = FindFirstFileA(fnf.Get(), &fd);
        if(h != INVALID_HANDLE_VALUE)
        {
            do
            {
                if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {

                    CWStr   found(fn + CStr(fd.cFileName, folderfrom.GetHeap()));
                    ef(found, user);
                }
            } while(FindNextFileA(h, &fd));
            FindClose(h);
        }

        fnf = fn + "*.*";
        h = FindFirstFileA(fnf.Get(), &fd);
        if(h != INVALID_HANDLE_VALUE)
        {
            do
            {
                if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    CWStr found(CStr(fd.cFileName, folderfrom.GetHeap()));
                    if(found != L"." && found != L"..")
                    {
                        FindFiles(CWStr(fn) + found, files, ef, user);
                    }
                }
            } while(FindNextFileA(h, &fd));
            FindClose(h);
        }
    }
}

BASE_API void CorrectFilePath(CWStr& filepath)
{
    if(filepath.GetLen() > 0 && (*(filepath.Get() + filepath.GetLen() - 1) == '\\' || (*(filepath.Get() + filepath.GetLen() - 1) == '/')))
    {
        filepath.Add(L"\\");
    }
}

BASE_API CWStr GetFilePath(const CWStr& filepath)
{
    int cnt = filepath.GetCountPar(L"\\/");
    if(cnt > 1)
    {
        cnt = filepath.GetSmePar(cnt - 1, L"\\/");
        return CWStr(filepath.Get(), cnt);
    }
    else return CWStr();
}

}