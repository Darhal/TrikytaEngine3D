#pragma once
#include <Core/Misc/Defines/Common.hpp>

TRE_NS_START

template<typename T>
class Singleton
{
protected:
	Singleton() = default;

	Singleton(const Singleton&) = delete;

	Singleton &operator=(const Singleton&) = delete;


public:
	static T& Instance()
	{
		static T Instance;
		return Instance;
	}
};

TRE_NS_END

