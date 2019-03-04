#pragma once
#include <string>
#include <wrl/client.h>
#include <d3d12.h>

struct Texture
{
	std::string name;
	std::wstring fileName;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap = nullptr;
};
