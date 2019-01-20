#include "stdafx.h"
#include "GameObject.h"

// Then:
// create new local transform matrix by:
// - create scale matrix
// - create rotation matrix
// - create translate matrix
// == local = scale * rot * translate
// THEN:
// world = parent * local
void GameObject::RefreshTransform(Sisu::Matrix4* parentTransform)
{
	// Create scale matrix
	Sisu::Matrix4 scaleMatrix(Sisu::Vector4(localScale.x, 0.0, 0.0, 0.0),
							  Sisu::Vector4(0.0, localScale.y, 0.0, 0.0),
							  Sisu::Vector4(0.0, 0.0, localScale.z, 0.0),
							  Sisu::Vector4(0.0, 0.0, 0.0, 1.0));

	//TODO - create rot matrix, but we really should have
	//		 a quaternion to store all rotations
	Sisu::Matrix4 rotMatrix = Sisu::Matrix4::Identity();

	// Create translation matrix
	Sisu::Matrix4 translateMatrix(Sisu::Vector4(1.0, 0.0, 0.0, 0.0),
							  Sisu::Vector4(0.0, 1.0, 0.0, 0.0),
							  Sisu::Vector4(0.0, 0.0, 1.0, 0.0),
							  Sisu::Vector4(localPosition.x, localPosition.y, localPosition.z, 1.0));

	// Now then
	transform = scaleMatrix * rotMatrix * translateMatrix;

	// And then the parent:
	if (parentTransform != nullptr)
	{
		transform = (*parentTransform) * transform;
	}
}