#include "stdafx.h"
#include "Texture.h"

void TextureManager::LoadFromFile(const std::string& name, const std::wstring& fileName)
{
	auto texPtr = std::make_unique<Texture>(name, fileName);
	ThrowIfFailed(
		DirectX::CreateDDSTextureFromFile12(
			_renderer->GetDevice(), _renderer->GetCommandList(),
			texPtr->fileName.c_str(), texPtr->resource, texPtr->uploadHeap)
	);

	_map[name] = std::move(texPtr);
}

Texture* TextureManager::Get(const std::string& name)
{
	return _map[name].get();
}