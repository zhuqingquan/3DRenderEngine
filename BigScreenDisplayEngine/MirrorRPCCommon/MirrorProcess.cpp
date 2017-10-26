#include "MirrorProcess.h"
#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>

using namespace SOA::Mirror;

Process::Process()
: m_startUpInfo(NULL)
, m_processInfo(NULL)
{
	m_startUpInfo = malloc(sizeof(STARTUPINFO));
	m_processInfo = malloc(sizeof(PROCESS_INFORMATION));
	memset(m_startUpInfo, 0, sizeof(STARTUPINFO));
	memset(m_processInfo, 0, sizeof(PROCESS_INFORMATION));
	((STARTUPINFO*)m_startUpInfo)->cb = sizeof(STARTUPINFO);
}

Process::~Process()
{
	free(m_startUpInfo);
	free(m_processInfo);
	m_startUpInfo = NULL;
	m_processInfo = NULL;
}

bool Process::start( const char* name )
{
	if(isProcessRunning())
	{
		return true;
	}
	BOOL ret = CreateProcess(name,		//执行文件的路径名称
							 NULL,		//命令行
							 NULL,		//Process handle not inheritable
							 NULL,		//Thread handle not inheritable
							 false,		//Set handle inheritable to FALSE
							 CREATE_NEW_CONSOLE,			//No creation flags
							 NULL,		//Use parent's environment block
							 NULL,		//Use parent's starting directory
							 (LPSTARTUPINFO)m_startUpInfo,				//Pointer to STARTUPINFO structure
							 (LPPROCESS_INFORMATION)m_processInfo);		//PONINTER to PROCESS_INFOMATION structure
	if(!ret)
	{
		printf( "CreateProcess %s  failed (%d).\n", name, GetLastError());
	}
	return ret;
}

void Process::kill()
{
	if(((LPPROCESS_INFORMATION)m_processInfo)->hProcess==0)
	{
		printf("Process have not created.\n");
		return;
	}
	BOOL ret = TerminateProcess(((LPPROCESS_INFORMATION)m_processInfo)->hProcess, 0);
	if(!ret)
	{
		printf("TerminateProcess failed (%d).\n", GetLastError());
	}

	WaitForSingleObject( ((LPPROCESS_INFORMATION)m_processInfo)->hProcess, INFINITE );

	CloseHandle( ((LPPROCESS_INFORMATION)m_processInfo)->hProcess );
	CloseHandle( ((LPPROCESS_INFORMATION)m_processInfo)->hThread );

	memset(m_startUpInfo, 0, sizeof(STARTUPINFO));
	memset(m_processInfo, 0, sizeof(PROCESS_INFORMATION));
}

bool Process::close()
{
	return false;
}

Process::Process( const char* processExeName )
{
	m_startUpInfo = malloc(sizeof(STARTUPINFO));
	m_processInfo = malloc(sizeof(PROCESS_INFORMATION));
	memset(m_startUpInfo, 0, sizeof(STARTUPINFO));
	memset(m_processInfo, 0, sizeof(PROCESS_INFORMATION));
	((STARTUPINFO*)m_startUpInfo)->cb = sizeof(STARTUPINFO);

	InitProcessInfoFromSystemExecutingProcess(processExeName);
}

void Process::InitProcessInfoFromSystemExecutingProcess(const char* processExeName)
{

	if(processExeName==NULL)
		return;
	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hProcessSnapshot==INVALID_HANDLE_VALUE)
	{
		printf("Faile in Process::InitProcessInfoFromSystemExecutingProcess do CreateToolhelp32Snapshot\n");
		return;
	}
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if(!Process32First(hProcessSnapshot, &pe32))
	{
		printf("Faile in Process::InitProcessInfoFromSystemExecutingProcess do Process32First\n");
		CloseHandle(hProcessSnapshot);
		return;
	}
	do 
	{
		if(strcmp(processExeName, pe32.szExeFile)!=0)
		{
			continue;
		}
		HANDLE hProcessTemp = OpenProcess(PROCESS_ALL_ACCESS, false, pe32.th32ProcessID);
		if(hProcessTemp==NULL)
		{
			printf("Faile in Process::InitProcessInfoFromSystemExecutingProcess do OpenProcess : %d\n", pe32.th32ProcessID);
			continue;
		}
		((LPPROCESS_INFORMATION)m_processInfo)->hProcess = hProcessTemp;
		break;
	} while (Process32Next(hProcessSnapshot, &pe32));
	CloseHandle(hProcessSnapshot);
}

bool SOA::Mirror::Process::isProcessRunning()
{
	if( ((LPPROCESS_INFORMATION)m_processInfo)->hProcess != NULL )		
	{
		if( WaitForSingleObject(((LPPROCESS_INFORMATION)m_processInfo)->hProcess, 1)==WAIT_OBJECT_0 )
			return false;
		else
			return true;
	}
	return false;
}
