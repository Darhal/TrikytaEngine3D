#pragma once

#include <Core/Misc/Defines/Common.hpp>
#include <Core/Misc/Maths/Matrix4x4.hpp>
#include <Core/ECS/Component/BaseComponent.hpp>

TRE_NS_START

struct TransformComponent : public Component<TransformComponent>
{
	TransformComponent(const Mat4f& transform = {}) : transform_matrix(transform)
	{}
	Mat4f transform_matrix;
};

TRE_NS_END