#pragma once
#include "D3DRenderer.h"
#include "SisuUtilities.h"

class D3DCamera
{
public:
	D3DCamera() : _position(Sisu::Vector3(0.0f, 0.0f, -15.0f)), _isDirty(true)
	{
	}

	void SetViewport(Sisu::Vector4 normalizedVP, float width, float height, float mindepth, float maxdepth)
	{
		normalizedViewport = normalizedVP;
		viewport.MinDepth = mindepth;
		viewport.MaxDepth = maxdepth;
		UpdateViewport(width, height);
	}

	void Update(const GameTimer& gt, 
				const Sisu::Vector3& inputAxes,
			    const Sisu::Vector3& inputEuler);

	void SetPosition(const Sisu::Vector3& vec) { _position = vec; }
	void SetRotation(const Sisu::Vector3& euler) { _rotation = Sisu::Quat::Euler(euler); }
	void SetClearColor(const DirectX::XMVECTORF32& color)
	{
		for (int i = 0; i < 4; ++i) { _clearColor[i] = color.f[i]; }
	}
	void SetCameraIndex(std::size_t index) { _cameraIndex = index; }

	DirectX::XMFLOAT3 Position() const { return DirectX::XMFLOAT3(_position.x, _position.y, _position.z); }
	const DirectX::XMFLOAT4X4& ViewMatrix() const { return _viewMatrix; }
	const DirectX::XMFLOAT4X4& ProjectionMatrix() const { return _projectionMatrix; }
	bool ShouldClearRenderTargetView(OUT D3D12_RECT& rect, OUT float* clearColor) const;
	std::size_t CbvIndex() const { return _cameraIndex; }

	void OnResize(float width, float height);

private:
	DirectX::XMMATRIX ToXMMatrix(const Sisu::Matrix4& m) const;
	void UpdateViewport(float newWidth, float newHeight);
	void UpdateTransform();

public:
	float speedUnitPerSecond = 2.0f;
	float rotationSensitivity = 50.0f;
	float FOV_H = 100.0f;
	float orthoSizeWidth = 10.0f;
	float nearPlaneDistance = 0.3f;
	float farPlaneDistance = 1000.0f;

	Sisu::Vector4 normalizedViewport;	//left, top, width, height
	D3D12_VIEWPORT viewport;

	bool clearDepthOnly = false;
	bool isPerspective = true;

private:
	DirectX::XMFLOAT4X4 _viewMatrix;
	DirectX::XMFLOAT4X4 _projectionMatrix;

	Sisu::Vector3 _position;
	Sisu::Matrix4 _transform;
	Sisu::Matrix4 _rotationMatrix;
	Sisu::Quat _rotation;

	float _clearColor[4] = { 0.2f, 0.3f, 1.0f, 1.0f };
	
	// One cbv index for each possible frame resource
	std::size_t _cameraIndex;
	bool _isDirty;
};
