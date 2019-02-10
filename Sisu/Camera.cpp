#include "stdafx.h"
#include "Camera.h"

DirectX::XMMATRIX D3DCamera::ToXMMatrix(const Sisu::Matrix4& m) const
{
	//TODO - we could probably optimize this.
	//TODO - also this is the same as in BrickRenderer
	DirectX::XMFLOAT4X4 xm(m.r0.x, m.r0.y, m.r0.z, m.r0.w,
		m.r1.x, m.r1.y, m.r1.z, m.r1.w,
		m.r2.x, m.r2.y, m.r2.z, m.r2.w,
		m.r3.x, m.r3.y, m.r3.z, m.r3.w);

	return DirectX::XMLoadFloat4x4(&xm);
}

//TODO: only do this if dirty
void D3DCamera::Update(const GameTimer& gt, 
					   const Sisu::Vector3& inputAxes,
					   const Sisu::Vector3& inputEuler)
{
	auto relativeRotation = inputEuler * _rotationMatrix;

	auto deltaRotationEuler = relativeRotation * (rotationSensitivity / 1000.0f);
	auto rotq = Sisu::Quat::Euler(deltaRotationEuler);
	_rotation = rotq * _rotation;
	_rotationMatrix = Sisu::Matrix4::FromQuat(_rotation);

	auto velocity = inputAxes * speedUnitPerSecond * gt.DeltaTimeSeconds();
	velocity = velocity * _rotationMatrix;
	_position += velocity;

	Sisu::Matrix4 translateMatrix(Sisu::Vector4(1.0, 0.0, 0.0, 0.0),
								  Sisu::Vector4(0.0, 1.0, 0.0, 0.0),
								  Sisu::Vector4(0.0, 0.0, 1.0, 0.0),
								  Sisu::Vector4(_position.x, _position.y, _position.z, 1.0));

	_transform = _rotationMatrix * translateMatrix;

	DirectX::XMMATRIX view = DirectX::XMMatrixInverse(nullptr, ToXMMatrix(_transform));
	DirectX::XMStoreFloat4x4(&_viewMatrix, view);
}

void D3DCamera::UpdateTransform()
{
	_rotationMatrix = Sisu::Matrix4::FromQuat(_rotation);
	Sisu::Matrix4 translateMatrix(Sisu::Vector4(1.0, 0.0, 0.0, 0.0),
		Sisu::Vector4(0.0, 1.0, 0.0, 0.0),
		Sisu::Vector4(0.0, 0.0, 1.0, 0.0),
		Sisu::Vector4(_position.x, _position.y, _position.z, 1.0));
	
	_transform = _rotationMatrix * translateMatrix;

	DirectX::XMMATRIX view = DirectX::XMMatrixInverse(nullptr, ToXMMatrix(_transform));
	DirectX::XMStoreFloat4x4(&_viewMatrix, view);
}

bool D3DCamera::ShouldClearRenderTargetView(OUT D3D12_RECT& rect, OUT float* clearColor) const
{
	rect.left = viewport.TopLeftX;
	rect.top = viewport.TopLeftY;
	rect.right = rect.left + viewport.Width;
	rect.bottom = rect.top + viewport.Height;

	clearColor[0] = _clearColor[0];
	clearColor[1] = _clearColor[1];
	clearColor[2] = _clearColor[2];
	clearColor[3] = _clearColor[3];

	return !clearDepthOnly;
}

void D3DCamera::OnResize(float width, float height)
{
	auto aspectRatio = width / height;
	if (isPerspective)
	{
		auto FOV_vertical = FOV_H / aspectRatio;

		DirectX::XMMATRIX projectionMatrix =
			DirectX::XMMatrixPerspectiveFovLH(FOV_vertical * MathHelper::Pi / 180.0f, aspectRatio, nearPlaneDistance, farPlaneDistance);
		DirectX::XMStoreFloat4x4(&_projectionMatrix, projectionMatrix);
	}
	else
	{
		auto camSpaceWidth = orthoSizeWidth;
		auto camSpaceHeight = camSpaceWidth / aspectRatio;
		DirectX::XMMATRIX projectionMatrix =
			DirectX::XMMatrixOrthographicLH(camSpaceWidth, camSpaceHeight, nearPlaneDistance, farPlaneDistance);
		DirectX::XMStoreFloat4x4(&_projectionMatrix, projectionMatrix);
	}

	UpdateViewport(width, height);
	UpdateTransform();
}

void D3DCamera::UpdateViewport(float newWidth, float newHeight)
{
	viewport.TopLeftX = newWidth * normalizedViewport.x;
	viewport.TopLeftY = newHeight * normalizedViewport.y;
	viewport.Width = newWidth * normalizedViewport.z;
	viewport.Height = newHeight * normalizedViewport.w;
}