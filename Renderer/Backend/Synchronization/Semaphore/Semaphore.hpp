#pragma once

#include <Renderer/Common.hpp>
#include <Renderer/Backend/Common/Globals.hpp>

TRE_NS_START

namespace Renderer
{
    class RenderDevice;

	struct SemaphoreDeleter
	{
		void operator()(class Semaphore* semaphore);
	};

	class Semaphore : public Utils::RefCounterEnabled<Semaphore, SemaphoreDeleter, HandleCounter>
	{
	public:
		~Semaphore();

		FORCEINLINE VkSemaphore GetApiObject() const { return semaphore; }

		void SetNoClean() { clean = false; };
	private:
        Semaphore(RenderDevice& device, VkSemaphore semaphore) :
            device(device), semaphore(semaphore), clean(true)
		{};
		
	private:
        RenderDevice& device;
		VkSemaphore semaphore;
		bool clean;

		friend struct SemaphoreDeleter;
		friend class Utils::ObjectPool<Semaphore>;
		friend class Utils::StackAllocator<Semaphore, MAX_WAIT_SEMAPHORE_PER_QUEUE>;
	};

	using SemaphoreHandle = Handle<Semaphore>;
}

TRE_NS_END
