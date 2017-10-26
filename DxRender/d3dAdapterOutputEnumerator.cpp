#include "d3dAdapterOutputEnumerator.h"

HRESULT DXGI_getAdapters(std::vector<IDXGIAdapter*>& adaptersVec)
{
	IDXGIFactory1* factory = NULL;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
	if(FAILED(hr))
		return hr;
	UINT adptIndex = 0;
	IDXGIAdapter1* pAdapter = NULL;
	while( SUCCEEDED(factory->EnumAdapters1(adptIndex, &pAdapter)) )
	{
		adaptersVec.push_back(pAdapter);
		adptIndex++;
	}
	factory->Release();
	return S_OK;
}

HRESULT DXGI_getOutputs(IDXGIAdapter* adapter, std::vector<IDXGIOutput*>& outputVec)
{
	if(adapter==NULL)
		return S_FALSE;
	UINT outputIndex = 0;
	IDXGIOutput* pOutput = NULL;
	while( SUCCEEDED(adapter->EnumOutputs(outputIndex, &pOutput)) )
	{
		outputVec.push_back(pOutput);
		++outputIndex;
	}
	return S_OK;
}

HRESULT DXGI_releaseAdaptersObjs(std::vector<IDXGIAdapter*>& adptVec)
{
	for(size_t i=0; i<adptVec.size(); i++)
	{
		if(adptVec[i]!=NULL)
			adptVec[i]->Release();
	}
	adptVec.clear();
	return S_OK;
}

HRESULT DXGI_releaseOutputObjs(std::vector<IDXGIOutput*>& outputVec)
{
	for(size_t i=0; i<outputVec.size(); i++)
	{
		if(outputVec[i]!=NULL)
			outputVec[i]->Release();
	}
	outputVec.clear();
	return S_OK;
}

HRESULT DXGI_getNearestAdapter(HWND hWnd, IDXGIAdapter** ppOutAdapter)
{
	if(NULL==hWnd || ppOutAdapter==NULL)
		return S_FALSE;
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if(NULL==hMonitor)
		return S_FALSE;
	IDXGIAdapter* dstAdapter = NULL;

	std::vector<IDXGIAdapter*> adapters;
	HRESULT hr = DXGI_getAdapters(adapters);
	if(S_OK!=hr)
		return hr;
	for(size_t i=0; i<adapters.size(); i++)
	{
		std::vector<IDXGIOutput*> outputs;
		hr = DXGI_getOutputs(adapters[i], outputs);
		if(S_OK!=hr)
		{
			continue;
		}
		for(size_t outIndex=0; outIndex<outputs.size(); )
		{
			DXGI_OUTPUT_DESC outputDesc;
			outputs[outIndex]->GetDesc(&outputDesc);
			if(outputDesc.Monitor==hMonitor)
			{
				dstAdapter = adapters[i];
				break;
			}
			outIndex++;
		}
		DXGI_releaseOutputObjs(outputs);
		if(dstAdapter)	break;
	}

	for(size_t i=0; i<adapters.size(); i++)
	{
		if(adapters[i]!=dstAdapter)
			adapters[i]->Release();
	}
	adapters.clear();
	*ppOutAdapter = dstAdapter;
	return dstAdapter ? S_OK : S_FALSE;
	/*if(ppOutAdapter==NULL || hWnd==INVALID_HANDLE_VALUE)
	{
		return S_FALSE;
	}
	RECT winRect = {0};
	if(FALSE==GetWindowRect(hWnd, &winRect))
	{
		return S_FALSE;
	}
	std::vector<IDXGIAdapter*> adaptersVec;
	if(FAILED(DXGI_getAdapters(adaptersVec)) || adaptersVec.size()<=0)
		return S_FALSE;
	std::vector<IDXGIOutput*> outputVec;
	size_t iAdpt = 0;
	size_t selectedAdpt = 0;
	size_t overlappedArea = 0;
	for (; iAdpt<adaptersVec.size(); iAdpt++)
	{
		std::vector<IDXGIOutput*> outputVecTmp;
		if(FAILED(DXGI_getOutputs(adaptersVec[iAdpt], outputVecTmp)) || outputVecTmp.size()<=0)
			continue;
		if(winRect.right<=winRect.left || winRect.bottom<=winRect.top)
		{
			//可能窗口是隐藏的，此时返回第一个有连接显示器的adapter
			IDXGIAdapter* adpt = adaptersVec[0];
			adaptersVec.erase(adaptersVec.begin());
			DXGI_releaseAdaptersObjs(adaptersVec);
			*ppOutAdapter = adpt;
			return S_OK;
		}
		size_t iOutput = 0;
		for (; iOutput<outputVec.size(); iOutput++)
		{
			DXGI_OUTPUT_DESC outputDesc;
			outputVecTmp[iOutput]->GetDesc(&outputDesc);
			if(winRect.left>=outputDesc.DesktopCoordinates.left && winRect.left<outputDesc.DesktopCoordinates.right
				&& winRect.top>=outputDesc.DesktopCoordinates.top && winRect.top<outputDesc.DesktopCoordinates.bottom)
			{
				selectedAdpt = iAdpt;
			}
		}
		DXGI_releaseOutputObjs(outputVecTmp);
	}
	IDXGIAdapter* adpt = adaptersVec[0];
	adaptersVec.erase(adaptersVec.begin()+iAdpt);
	DXGI_releaseAdaptersObjs(adaptersVec);
	*ppOutAdapter = adpt;
	return S_OK;*/
}

HRESULT DXGI_getAdapter(HMONITOR hmonitor, IDXGIAdapter** ppOutAdapter)
{
	std::vector<IDXGIAdapter*> adapters;
	HRESULT hr = DXGI_getAdapters(adapters);
	if(S_OK!=hr)
		return hr;
	IDXGIAdapter* dstAdapter = NULL;
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
			if(outputDesc.Monitor==hmonitor)
			{
				dstAdapter = adapters[i];
				break;
			}
		}
		DXGI_releaseOutputObjs(outputs);
		if(dstAdapter)	break;
	}
	for(size_t i=0; i<adapters.size(); i++)
	{
		if(adapters[i]!=dstAdapter)
			adapters[i]->Release();
	}
	adapters.clear();
	*ppOutAdapter = dstAdapter;
	return dstAdapter ? S_OK : S_FALSE;
}
