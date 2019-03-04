#include "stdafx.h"
#include "Texture.h"
#include "IRenderer.h"
#include "d3dUtil.h"

void TextureManager::LoadFromFile(const std::string& name, const std::wstring& fileName)
{
	auto texPtr = std::make_unique<Texture>(name, fileName);
	ThrowIfFailed(
		DirectX::CreateDDSTextureFromFile12(
			_renderer->GetDevice(), _renderer->GetCommandList(),
			texPtr->fileName.c_str(), 
			texPtr->resource, 
			texPtr->uploadHeap)
	);

	_map[name] = std::move(texPtr);
}

void TextureManager::UploadToHeap(const std::string& name)
{
	auto texture = _map[name].get();
	auto offset = _textureInHeapCount;

	//Get pointer to the start of the heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(_renderer->GetTextureHeap()->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(offset, _renderer->GetSrvDescriptorSize());

	//Create srv view desc
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texture->resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = texture->resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;

	_renderer->GetDevice()->CreateShaderResourceView(texture->resource.Get(), &srvDesc, hDescriptor);
	_textureInHeapCount++;
}

Texture* TextureManager::Get(const std::string& name)
{
	return _map[name].get();
}