#include "MonitorDisplayInfo.h"
#include "DisplayConfigDefine.h"
#include <stdio.h>

void sortDisplayInfoById(SOA::Mirror::Common::MonitorDisplayInfoList& infoList)
{
	for(int i=infoList.count-1; i>0; i--)
	{
		for(int j=0; j<i; j++)
		{
			SOA::Mirror::Common::MonitorDisplayInfo* info = (SOA::Mirror::Common::MonitorDisplayInfo*)infoList.pBuf + j;
			SOA::Mirror::Common::MonitorDisplayInfo* nextInfo = info + 1;
			if(info->id > nextInfo->id)
			{
				SOA::Mirror::Common::MonitorDisplayInfo tempInfo = *nextInfo;
				*nextInfo = *info;
				*info = tempInfo;
			}
		}
	}
// 	for(int x=0; x<infoList.count; x++)
// 	{
// 		SOA::Mirror::Common::MonitorDisplayInfo* info = (SOA::Mirror::Common::MonitorDisplayInfo*)infoList.pBuf + x;
// 		printf("sorted info ID : %I64x\n", info->id);
// 	}
}

int SOA::Mirror::Common::getAllMonitorDispalyInfo( SOA::Mirror::Common::MonitorDisplayInfoList& infoList )
{
	if(infoList.count<=0 || infoList.pBuf==NULL)
		return -1;
	HMODULE user32dll = LoadLibrary(TEXT("user32.dll"));
	if(NULL==user32dll)
	{
		printf("ScreenDisplay : failed in Load user32.dll\n");
		return -2;
	}
	GETDISPLAYBUFFERSIZESFUNC pGetDisplayConfigBufferSizesFunc = (GETDISPLAYBUFFERSIZESFUNC)(GetProcAddress(user32dll,"GetDisplayConfigBufferSizes"));
	QUERYDISPLAYCONFIGFUNC pQueryDisplayConfigFunc=(QUERYDISPLAYCONFIGFUNC)GetProcAddress(user32dll,"QueryDisplayConfig");
	if(NULL==pGetDisplayConfigBufferSizesFunc || NULL==pQueryDisplayConfigFunc)
	{
		printf("ScreenDisplay : failed in GetProcAddress QueryDisplayConfigFunc\n");
		FreeLibrary(user32dll);
		return -3;
	}

	GETDISPLAYBUFFERSIZESFUNC getFunc = pGetDisplayConfigBufferSizesFunc;
	QUERYDISPLAYCONFIGFUNC queryFunc = pQueryDisplayConfigFunc;

	UINT32 NumPathArrayElements = 0; 
	UINT32 NumModeInfoArrayElements = 0; 
	LONG returnValue;
	returnValue = getFunc(
		QDC_ALL_PATHS,
		&NumPathArrayElements,
		&NumModeInfoArrayElements); // Returns Success 
	if (returnValue != ERROR_SUCCESS)
	{
		printf(TEXT("GetDisplayConfigBufferSizes error")); 
		FreeLibrary(user32dll);
		return -4;
	}
	DISPLAYCONFIG_PATH_INFO *pPathInfoArray =
		new DISPLAYCONFIG_PATH_INFO[NumPathArrayElements]; 

	DISPLAYCONFIG_MODE_INFO *pModeInfoArray =
		new DISPLAYCONFIG_MODE_INFO[NumModeInfoArrayElements]; 

	returnValue = queryFunc(
		QDC_ALL_PATHS,
		&NumPathArrayElements, pPathInfoArray,
		&NumModeInfoArrayElements, pModeInfoArray,
		NULL); 
	if(returnValue != ERROR_SUCCESS)
	{
		printf(TEXT(" QueryDisplayConfig erro"));
		delete[] pPathInfoArray;
		delete[] pModeInfoArray;
		FreeLibrary(user32dll);
		return -5;
	}
// 	printf("=========================Monitor Infomation=================================\n");
// 	printf("%6s\t%6s\t%6s\t%6s\t%6s\t%6s\t%6s\n", "left", "top", "right", "bottom", "width", "height", "HMONITOR");
	int count = 0;
	for(int i=0; i<NumPathArrayElements; i++)
	{
		DISPLAYCONFIG_PATH_INFO* curPathInfo = pPathInfoArray+i;
		if(curPathInfo->flags==1)
		{
			DISPLAYCONFIG_MODE_INFO* curModeInfo = pModeInfoArray + curPathInfo->sourceInfo.modeInfoIdx;
			if(curModeInfo->infoType!=DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE)
				continue;
			DISPLAYCONFIG_MODE_INFO* curTargetInfo = pModeInfoArray + curPathInfo->targetInfo.modeInfoIdx;
			RECT rc;
			rc.left = curModeInfo->sourceMode.position.x;
			rc.top = curModeInfo->sourceMode.position.y;
			rc.right = rc.left + curModeInfo->sourceMode.width;
			rc.bottom = rc.top + curModeInfo->sourceMode.height;

			int refreshRate = 0;
			float tempRate = (float)curPathInfo->targetInfo.refreshRate.Numerator / curPathInfo->targetInfo.refreshRate.Denominator;
			if(tempRate-(int)tempRate<0.5)
			{
				refreshRate = (int)tempRate;
			}
			else
			{
				refreshRate = (int)tempRate + 1;
			}

			HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
			if(hMonitor==NULL)
			{
				printf("failed get Monitor");
				continue;
			}
			if(count>=infoList.count)
				return 0;	//给的数组大小可能不够，可能没有将所有显示器信息获取到
			SOA::Mirror::Common::MonitorDisplayInfo* pInfo = (SOA::Mirror::Common::MonitorDisplayInfo*)infoList.pBuf + count;
			pInfo->rect = rc;
			pInfo->refreshRate = refreshRate;
			pInfo->handle = hMonitor;
			pInfo->id = curTargetInfo->id;
			pInfo->id = pInfo->id << 32;
			pInfo->id |= curTargetInfo->adapterId.LowPart;
//			printf("Index : %-8d sourceID : %-8x targetID : %-8x adapterID : %-8x\n", count, curModeInfo->id, curTargetInfo->id, curTargetInfo->adapterId.LowPart);
//			printf("infoID : %I64x\n", pInfo->id);
			count++;

// 			printf("%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\t\n", 
// 				rc.left, rc.top, rc.right, rc.bottom, curModeInfo->sourceMode.width, curModeInfo->sourceMode.height,
// 				(int)hMonitor);
		}
	}
	infoList.count = count;

	sortDisplayInfoById(infoList);
//	printf("=============================================================================\n");
	delete[] pPathInfoArray;
	delete[] pModeInfoArray;

	if(NULL != user32dll)
	{
		FreeLibrary(user32dll);
		user32dll = NULL;
	}
	return 0;
}

int SOA::Mirror::Common::extendDestopToMonitorDispaly()
{
	int iType = SDC_TOPOLOGY_EXTEND;
	SETDISPLAYCONFIGFUNC      SetDisplayConfig=NULL;
	HMODULE hMod=LoadLibrary(TEXT("user32.dll"));
	if(hMod)
	{
		OutputDebugString(TEXT("Load OK"));
		SetDisplayConfig=(SETDISPLAYCONFIGFUNC)GetProcAddress(hMod,"SetDisplayConfig");
		if(SetDisplayConfig)
		{
			OutputDebugString(TEXT("GetProcAddress SetDisplayConfig OK!"));
			LONG returnValue;
			returnValue=SetDisplayConfig(0,NULL,0,NULL,iType|SDC_APPLY);//设置DC_TOPOLOGY_XXX前面4个参数must be 0;
			switch(returnValue)
			{
			case ERROR_SUCCESS:
				OutputDebugString(TEXT("Set Display Config OK"));
				return 0;
			case ERROR_INVALID_PARAMETER:
				OutputDebugString(TEXT("ERROR_INVALID_PARAMETER"));
				break;
			case ERROR_NOT_SUPPORTED:
				OutputDebugString(TEXT("ERROR_NOT_SUPPORTED"));
				break;
			case ERROR_ACCESS_DENIED:
				OutputDebugString(TEXT("ERROR_ACCESS_DENIED"));
				break;
			case ERROR_GEN_FAILURE:
				OutputDebugString(TEXT("ERROR_GEN_FAILURE"));
				break;
			case ERROR_INSUFFICIENT_BUFFER:
				OutputDebugString(TEXT("ERROR_INSUFFICIENT_BUFFER"));
				break;
			default:
				OutputDebugString(TEXT("unkonw"));
			}
		}
		FreeLibrary(hMod);
	}
	return -1;
}

int SOA::Mirror::Common::compareMonitorDisplayInfoList( const SOA::Mirror::Common::MonitorDisplayInfoList& left, 
													   const SOA::Mirror::Common::MonitorDisplayInfoList& right )
{
	if(left.count==right.count)
		return 0;
}
