#pragma once
#include "d3dx12.h"
#include "dxgi1_6.h"
#include <wrl.h>
#include <vector>
#include "d3dUtil.h"

class D3DLogger
{
public:
	void LogAdapters(Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory, DXGI_FORMAT backBufferFormat);

private:
	void LogAdapterOutputs(IDXGIAdapter* adapter, DXGI_FORMAT backBufferFormat);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
};