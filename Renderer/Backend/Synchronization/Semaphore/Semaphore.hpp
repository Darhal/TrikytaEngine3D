#pragma once

#include <Renderer/Common.hpp>
#include <Renderer/Backend/Common/Globals.hpp>

TRE_NS_START

namespace Renderer
{
	class RenderBackend;

	struct SemaphoreDeleter
	{
		void operator()(class Semaphore* semaphore);
	};

	class Semaphore : public Utils::RefCounterEnabled<Semaphore, SemaphoreDeleter, HandleCounter>
	{
	public:
		~Semaphore();

		FORCEINLINE VkSemaphore GetAPIObject() const { return semaphore; }
	private:
		Semaphore(RenderBackend& backend, VkSemaphore semaphore) :
			backend(backend), semaphore(semaphore)
		{};
	private:
		RenderBackend& backend;
		VkSemaphore semaphore;

		friend struct SemaphoreDeleter;
		friend class Utils::ObjectPool<Semaphore>;
		friend class Utils::StackAllocator<Semaphore, MAX_WAIT_SEMAPHORE_PER_QUEUE>;
	};

	using SemaphoreHandle = Handle<Semaphore>;
}

TRE_NS_END