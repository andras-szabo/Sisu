#pragma once
#include <string>
#include <unordered_map>
#include "D3DRenderer.h"
#include "DDSTextureLoader.h"

struct Texture
{
	Texture(const std::string& tName, const std::wstring& tFileName) : name(tName), fileName(tFileName) {}

	std::string name;
	std::wstring fileName;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap = nullptr;
};

class TextureManager
{
public:
	TextureManager(IRenderer* renderer) : _renderer(renderer) {}
	void LoadFromFile(const std::string& name, const std::wstring& fileName);
	Texture* Get(const std::string& name);
	
private:
	IRenderer* _renderer;
	std::unordered_map<std::string, std::unique_ptr<Texture>> _map;
};
