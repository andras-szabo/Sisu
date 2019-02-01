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

	DirectX::XMFLOAT3 Position() const { return DirectX::XMFLOAT3(_position.x, _position.y, _position.z); }
	const DirectX::XMFLOAT4X4& ViewMatrix() const { return _viewMatrix; }
	const DirectX::XMFLOAT4X4& ProjectionMatrix() const { return _projectionMatrix; }

	void OnResize(float width, float height);

private:
	DirectX::XMMATRIX ToXMMatrix(const Sisu::Matrix4& m) const;
	void UpdateViewport(float newWidth, float newHeight);

public:
	float speedUnitPerSecond = 2.0f;
	float rotationSensitivity = 50.0f;
	float FOV_H = 100.0f;
	float nearPlaneDistance = 0.3f;
	float farPlaneDistance = 1000.0f;

	Sisu::Vector4 normalizedViewport;	//left, top, width, height
	D3D12_VIEWPORT viewport;

private:
	DirectX::XMFLOAT4X4 _viewMatrix;
	DirectX::XMFLOAT4X4 _projectionMatrix;

	Sisu::Vector3 _position;
	Sisu::Matrix4 _transform;
	Sisu::Matrix4 _rotationMatrix;
	Sisu::Quat _rotation;

	bool _isDirty;
};
