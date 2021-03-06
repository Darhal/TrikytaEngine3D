#pragma once

#include <Legacy/Misc/Defines/Common.hpp>
#include <Legacy/Misc/Defines/Debug.hpp>
#include <Legacy/Memory/Utils/Utils.hpp>

TRE_NS_START

class BaseAllocator
{
public:
	virtual ~BaseAllocator() {};

	virtual void* Allocate(uint32 sz = 0, uint32 alignement = 0) = 0;
	virtual void Deallocate(void* ptr) = 0;
};

TRE_NS_END