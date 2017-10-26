/*
 *	@file		d3dAdapterOutputEnumerator.h
 *	@data		2014-08-20 00:15
 *	@author		zhu qing quan	
 *	@brief		使用DXGI获取系统中检测到的所有显卡（IDXGIAdapter)和显示器(IDXGIOutput)对象
 */

#include <DXGI.h>
#include <vector>

/**
  *	@name		DXGI_getAdapters
  * @brief		get all Adapter(video card) in this host
  *	@param[out]	std::vector<IDXGIAdapter*>& adaptersVec 保存所有的Adapter对象，此参数原有的Adapter不会释放。新获取的Adapter对象将会加入到vector的后面
  *	@return		HRESULT S_OK--成功  Other--其他
  */
HRESULT DXGI_getAdapters(std::vector<IDXGIAdapter*>& adaptersVec);

/**
  *	@name		DXGI_getOutputs
  * @brief		获取所有连接到显卡adapter中的输出设备（显示器）
  * @param[in]	IDXGIAdapter1* adapter 显卡Adapter对象
  *	@param[out]	std::vector<IDXGIOutput*>& outputVec 保存所有连接到显卡adapter中的输出设备（显示器），此参数原有的Output不会释放。新获取的Output对象将会加入到vector的后面
  *	@return		HRESULT S_OK--成功  Other--其他
  */
HRESULT DXGI_getOutputs(IDXGIAdapter* adapter, std::vector<IDXGIOutput*>& outputVec);

/**
  *	@name		DXGI_releaseAdaptersObjs
  * @brief		释放vector中的所有Adapter对象
  *				对每个对象调用release，并调用vector.clear()清空vector
  *	@param[in,out]	std::vector<IDXGIAdapter*>& adptVec 保存所有Adapter（显卡）对象的vector
  *	@return		HRESULT S_OK--成功  Other--其他
  */
HRESULT DXGI_releaseAdaptersObjs(std::vector<IDXGIAdapter*>& adptVec);

/**
  *	@name		DXGI_releaseOutputObjs
  * @brief		释放vector中的所有Output（显示器）对象
  *				对每个对象调用release，并调用vector.clear()清空vector
  *	@param[out]	std::vector<IDXGIOutput*>& outputVec 保存所有Output（显示器）对象的vector
  *	@return		HRESULT S_OK--成功  Other--其他
  */
HRESULT DXGI_releaseOutputObjs(std::vector<IDXGIOutput*>& outputVec);

HRESULT DXGI_getNearestAdapter(HWND hWnd, IDXGIAdapter** ppOutAdapter);

HRESULT DXGI_getAdapter(HMONITOR hmonitor, IDXGIAdapter** ppOutAdapter);