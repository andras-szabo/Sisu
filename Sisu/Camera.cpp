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

void D3DCamera::OnResize(const DirectX::XMMATRIX& newProjectionMatrix)
{
	DirectX::XMStoreFloat4x4(&_projectionMatrix, newProjectionMatrix);
}