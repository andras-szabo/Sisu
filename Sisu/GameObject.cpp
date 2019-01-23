#include "stdafx.h"
#include "GameObject.h"

void GameObject::RefreshTransform(Sisu::Matrix4* parentTransform)
{
	// Create scale matrix
	Sisu::Matrix4 scaleMatrix(Sisu::Vector4(localScale.x, 0.0, 0.0, 0.0),
							  Sisu::Vector4(0.0, localScale.y, 0.0, 0.0),
							  Sisu::Vector4(0.0, 0.0, localScale.z, 0.0),
							  Sisu::Vector4(0.0, 0.0, 0.0, 1.0));

	//TODO - create rot matrix
	Sisu::Matrix4 rotMatrix = Sisu::Matrix4::FromQuat(rotQuat);

	// Create translation matrix
	Sisu::Matrix4 translateMatrix(Sisu::Vector4(1.0, 0.0, 0.0, 0.0),
							  Sisu::Vector4(0.0, 1.0, 0.0, 0.0),
							  Sisu::Vector4(0.0, 0.0, 1.0, 0.0),
							  Sisu::Vector4(localPosition.x, localPosition.y, localPosition.z, 1.0));

	transform = scaleMatrix * rotMatrix * translateMatrix;
	if (parentTransform != nullptr)
	{
		transform = transform * (*parentTransform);
	}
}