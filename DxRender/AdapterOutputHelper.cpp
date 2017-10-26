#include "AdapterOutputHelper.h"
#include "d3dAdapterOutputEnumerator.h"

int zRender::getAllScreenLogicSysPos(std::vector<RECT>& outPosVec)
{
	outPosVec.clear();
	std::vector<IDXGIAdapter*> adapters;
	HRESULT hr = DXGI_getAdapters(adapters);
	if(S_OK!=hr)
		return -1;
	for(size_t i=0; i<adapters.size(); i++)
	{
		std::vector<IDXGIOutput*> outputs;
		hr = DXGI_getOutputs(adapters[i], outputs);
		if(S_OK!=hr)
		{
			continue;
		}
		for(size_t outIndex=0; outIndex<outputs.size(); outIndex++)
		{
			DXGI_OUTPUT_DESC outputDesc;
			outputs[outIndex]->GetDesc(&outputDesc);
			outPosVec.push_back(outputDesc.DesktopCoordinates);
		}
		DXGI_releaseOutputObjs(outputs);
	}
	DXGI_releaseAdaptersObjs(adapters);
	return outPosVec.size();
}

RECT zRender::getScreenLogicSysPos(HWND hwnd)
{
	RECT result = {0};
	if(NULL==hwnd)
		return result;
	HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if(NULL==hMonitor)
		return result;

	std::vector<IDXGIAdapter*> adapters;
	HRESULT hr = DXGI_getAdapters(adapters);
	if(S_OK!=hr)
		return result;
	for(size_t i=0; i<adapters.size(); i++)
	{
		std::vector<IDXGIOutput*> outputs;
		hr = DXGI_getOutputs(adapters[i], outputs);
		if(S_OK!=hr)
		{
			continue;
		}
		for(size_t outIndex=0; outIndex<outputs.size(); outIndex++ )
		{
			DXGI_OUTPUT_DESC outputDesc;
			outputs[outIndex]->GetDesc(&outputDesc);
			if(outputDesc.Monitor==hMonitor)
			{
				result = outputDesc.DesktopCoordinates;
				break;
			}
		}
		DXGI_releaseOutputObjs(outputs);
		if(((result.right - result.left) > 0) && ((result.bottom - result.top) > 0))
			break;
	}
	DXGI_releaseAdaptersObjs(adapters);
	return result;
}

