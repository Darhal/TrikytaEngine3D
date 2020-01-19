#pragma once

#include "Core/Misc/Defines/Common.hpp"
#include <Renderer/Backend/CommandBucket/CommandBucket.hpp>

TRE_NS_START

class IPrimitiveMesh
{
public:
	virtual void Submit(CommandBucket&) = 0;

	virtual void Submit(CommandBucket&, ShaderID) = 0;
private:
};

TRE_NS_END