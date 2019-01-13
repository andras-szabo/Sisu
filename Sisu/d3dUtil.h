#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <string>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <unordered_map>
#include <vector>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

struct SubmeshGeometry
{
	UINT indexCount = 0;
	UINT startIndexLocation = 0;
	INT baseVertexLocation = 0;
};

struct MeshGeometry
{
	int _vertexBufferCount;

	MeshGeometry(int vertexBufferCount = 1) : _vertexBufferCount(vertexBufferCount)
	{
		vertexBuffersCPU.reserve(vertexBufferCount);
		vertexBuffersGPU.reserve(vertexBufferCount);
		vertexByteStride.reserve(vertexBufferCount);
		vertexBufferByteSize.reserve(vertexBufferCount);

		for (int i = 0; i < vertexBufferCount; ++i)
		{
			vertexBuffersCPU.push_back(nullptr);
			vertexBuffersGPU.push_back(nullptr);
			vertexByteStride.push_back(0);
			vertexBufferByteSize.push_back(0);
		}
	}

	~MeshGeometry()
	{
		if (splitVertexBufferViews != nullptr)
		{
			delete[] splitVertexBufferViews;
		}

		DisposeUploaders();
	}

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(std::size_t index = 0) const
	{
		D3D12_VERTEX_BUFFER_VIEW vbView;

		vbView.BufferLocation = vertexBuffersGPU[index]->GetGPUVirtualAddress();
		vbView.StrideInBytes = vertexByteStride[index];
		vbView.SizeInBytes = vertexBufferByteSize[index];

		return vbView;
	}

	// In case we're using split vertex buffers
	D3D12_VERTEX_BUFFER_VIEW* splitVertexBufferViews = nullptr;
	D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferViews()
	{
		if (splitVertexBufferViews == nullptr)
		{
			splitVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[_vertexBufferCount];
			for (int i = 0; i < _vertexBufferCount; ++i)
			{
				splitVertexBufferViews[i] = GetVertexBufferView(i);
			}
		}

		return splitVertexBufferViews;
	}

	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibView;

		ibView.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
		ibView.Format = indexFormat;
		ibView.SizeInBytes = indexBufferByteSize;

		return ibView;
	}

	void DisposeUploaders()
	{
		vertexBufferUploader = nullptr;
		indexBufferUploader = nullptr;
	}

	std::string name;

	// System memory copies (i.e., vertex data that the CPU can access for
	// things like collision detection. It's the client's responsibility
	// to cast the blobs appropriately.

	//Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	std::vector<ComPtr<ID3DBlob>> vertexBuffersCPU;
	std::vector<ComPtr<ID3D12Resource>> vertexBuffersGPU;

	Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader = nullptr;

	std::vector<UINT> vertexByteStride;
	std::vector<UINT> vertexBufferByteSize;

	DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
	UINT indexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex / index buffer.
	// Use this container to define the Submesh geometries so we can draw the
	// submeshes individually.

	std::unordered_map<std::string, SubmeshGeometry> drawArgs;
};

class d3dUtil
{
public:
	static UINT CalcConstantBufferByteSize(UINT elementByteSize)
	{
		// We need to pad size to be a multiple of 256.
		// So: add 255 and then mask off he lower two
		// bytes. ~: bitwise NOT. 255 = 0x00ff, so ~255: 0xff00;
		// to bitwise AND with this, you'll get the upper two
		// bytes, but not the lower 2 bytes.
		return (elementByteSize + 255) & ~255;
	}

	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
	);

	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};
