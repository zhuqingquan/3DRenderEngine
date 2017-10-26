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

HRESULT DXGI_getNearestAdapter(HWND hWnd, IDXGIAdapter** adapter)
{
	if(NULL==hWnd)
		return S_FALSE;
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if(NULL==hMonitor)
		return S_FALSE;
	IDXGIAdapter* dstAdapter = NULL;

	std::vector<IDXGIAdapter*> adapters;
	HRESULT hr = DXGI_getAdapters(adapters);
	if(S_OK!=hr)
		return hr;
	for(int i=0; i<adapters.size(); i++)
	{
		std::vector<IDXGIOutput*> outputs;
		hr = DXGI_getOutputs(adapters[i], outputs);
		if(S_OK!=hr)
		{
			DXGI_releaseAdaptersObjs(adapters);
			return hr;
		}
		for(int outIndex=0; outIndex<outputs.size(); )
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

	for(int i=0; i<adapters.size(); i++)
	{
		if(adapters[i]!=dstAdapter)
			adapters[i]->Release();
	}
	adapters.clear();
	*adapter = dstAdapter;
	return dstAdapter ? S_OK : S_FALSE;
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
	for(int i=0; i<adptVec.size(); i++)
	{
		if(adptVec[i]!=NULL)
			adptVec[i]->Release();
	}
	adptVec.clear();
	return S_OK;
}

HRESULT DXGI_releaseAdapterObj(IDXGIAdapter** adpt)
{
	if(NULL==adpt || NULL==*adpt)
		return S_FALSE;
	(*adpt)->Release();
	*adpt = NULL;
	return S_OK;
}

HRESULT DXGI_releaseOutputObjs(std::vector<IDXGIOutput*>& outputVec)
{
	for(int i=0; i<outputVec.size(); i++)
	{
		if(outputVec[i]!=NULL)
			outputVec[i]->Release();
	}
	outputVec.clear();
	return S_OK;
}