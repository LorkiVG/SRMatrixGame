// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"

CRITICAL_SECTION SynBuf_CS;
HANDLE SynBuf_DMSP=nullptr;
HANDLE SynBuf_File=nullptr;
char * SynBuf_Buf=nullptr;
HANDLE SynBuf_Mutex=nullptr;
HANDLE SynBuf_Event=nullptr;

char SynBuf_FileName[MAX_PATH];
char SynBuf_MutexName[MAX_PATH];
char SynBuf_EventName[MAX_PATH];

bool SynBufInit()
{
    if(SynBuf_File!=nullptr) {
		if(WaitForSingleObject(SynBuf_DMSP,0)==WAIT_TIMEOUT) return true;
		SynBufClear();
	}

	if(!GlobalMsgNewSynBuf(SynBuf_FileName,SynBuf_MutexName,SynBuf_EventName)) return false;

    SynBuf_File=OpenFileMapping(FILE_MAP_ALL_ACCESS,false,SynBuf_FileName);
    if(SynBuf_File==nullptr) return false;

    SynBuf_Buf=(char *)MapViewOfFile(SynBuf_File,FILE_MAP_ALL_ACCESS,0,0,0);
    if(SynBuf_Buf==nullptr) {
    	CloseHandle(SynBuf_File); SynBuf_File=nullptr;
    	return false;
    }

	SynBuf_Mutex=OpenMutex(MUTEX_ALL_ACCESS,false,SynBuf_MutexName);
    if(SynBuf_Mutex==nullptr) {
	    UnmapViewOfFile(SynBuf_Buf); SynBuf_Buf=nullptr;
    	CloseHandle(SynBuf_File); SynBuf_File=0;
    	return false;
    }

    SynBuf_Event=OpenEvent(EVENT_ALL_ACCESS,false,SynBuf_EventName);
    if(SynBuf_Event==nullptr) {
    	CloseHandle(SynBuf_Mutex); SynBuf_Mutex=nullptr;
	    UnmapViewOfFile(SynBuf_Buf); SynBuf_Buf=nullptr;
    	CloseHandle(SynBuf_File); SynBuf_File=nullptr;
    	return false;
    }

	SynBuf_DMSP=GlobalMsgProcessHandle();
	if(SynBuf_DMSP==0) {
		SynBufClear();
		return false;
	}

	return true;
}

void SynBufClear()
{
	EnterCriticalSection(&SynBuf_CS);
	if(SynBuf_Event!=nullptr) { SetEvent(SynBuf_Event); CloseHandle(SynBuf_Event); SynBuf_Event=nullptr; }
	if(SynBuf_Mutex!=nullptr) { CloseHandle(SynBuf_Mutex); SynBuf_Mutex=nullptr; }
    if(SynBuf_Buf!=nullptr) { UnmapViewOfFile(SynBuf_Buf); SynBuf_Buf=nullptr; }
	if(SynBuf_File!=nullptr) { CloseHandle(SynBuf_File); SynBuf_File=nullptr; }
	if(SynBuf_DMSP!=nullptr) { CloseHandle(SynBuf_DMSP); SynBuf_DMSP=nullptr; }
	LeaveCriticalSection(&SynBuf_CS);
}

bool SynBuf_Write(void * buf1,int size1,
				  void * buf2=nullptr,int size2=0,
				  void * buf3=nullptr,int size3=0,
				  void * buf4=nullptr,int size4=0,
				  void * buf5=nullptr,int size5=0
				  )
{
	int i=0;

	EnterCriticalSection(&SynBuf_CS);
	while(i<=1) {
		if(!SynBufInit()) break;

		if(WaitForSingleObject(SynBuf_Mutex,30*1000)!=WAIT_OBJECT_0) {
			SynBufClear();
			i++; continue;
		}

		int maxsize=*(int *)SynBuf_Buf;
		int cnt=*(int *)(SynBuf_Buf+4);
		int cursize=*(int *)(SynBuf_Buf+8);

		if((cursize+size1+size2+size3+size4+size5)>maxsize) {
			SetEvent(SynBuf_Event);
			ReleaseMutex(SynBuf_Mutex);
			Sleep(0);
			continue;
		}

		CopyMemory(SynBuf_Buf+cursize+16,buf1,size1);
		if(buf2!=nullptr) CopyMemory(SynBuf_Buf+cursize+16+size1,buf2,size2);
		if(buf3!=nullptr) CopyMemory(SynBuf_Buf+cursize+16+size1+size2,buf3,size3);
		if(buf4!=nullptr) CopyMemory(SynBuf_Buf+cursize+16+size1+size2+size3,buf4,size4);
		if(buf5!=nullptr) CopyMemory(SynBuf_Buf+cursize+16+size1+size2+size3+size4,buf5,size5);
		*(int *)(SynBuf_Buf+8)=cursize+size1+size2+size3+size4+size5;
		*(int *)(SynBuf_Buf+4)=cnt+1;

		ReleaseMutex(SynBuf_Mutex);
		LeaveCriticalSection(&SynBuf_CS);
		return true;
	}
	LeaveCriticalSection(&SynBuf_CS);

	return false;
}

DEBUGMSG_API void DMcc(char * path,char * text)
{
	TEST_ACCESS;

	int lenpath=strlen(path);
	int lentext=strlen(text);

	SZagMsg zag;
	zag.Size=sizeof(SZagMsg)+4+lenpath+4+lentext;
    QueryPerformanceCounter((LARGE_INTEGER *)(&zag.Time));
	zag.ProcessId=GetCurrentProcessId();
	zag.ThreadId=GetCurrentThreadId();
	zag.Type=1;

	SynBuf_Write(&zag,sizeof(SZagMsg),
				 &lenpath,4,
				 path,lenpath,
				 &lentext,4,
				 text,lentext);
}

DEBUGMSG_API void DMww(wchar_t * path,wchar_t * text)
{
	TEST_ACCESS;

	int lenpath=wcslen(path);
	int lentext=wcslen(text);

	SZagMsg zag;
	zag.Size=sizeof(SZagMsg)+4+lenpath*2+4+lentext*2;
    QueryPerformanceCounter((LARGE_INTEGER *)(&zag.Time));
	zag.ProcessId=GetCurrentProcessId();
	zag.ThreadId=GetCurrentThreadId();
	zag.Type=2;

	SynBuf_Write(&zag,sizeof(SZagMsg),
				 &lenpath,4,
				 path,lenpath*2,
				 &lentext,4,
				 text,lentext*2);
}
