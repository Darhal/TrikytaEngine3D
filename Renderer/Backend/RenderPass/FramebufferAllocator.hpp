#pragma once

#include <Renderer/Common.hpp>
#include <Renderer/Backend/Common/Globals.hpp>
#include <Renderer/Backend/RenderPass/Framebuffer.hpp>
#include <Renderer/Core/Hashmap/TemporaryHashmap.hpp>

TRE_NS_START

namespace Renderer
{
	class RenderDevice;
	struct RenderPassInfo;
	class RenderPass;

	class RENDERER_API FramebufferAllocator
	{
	public:
		FramebufferAllocator(RenderDevice* device);

		Framebuffer& RequestFramebuffer(const RenderPass& renderPass, const RenderPassInfo& info);

		void BeginFrame();

		void Clear();
	private:
		struct FramebufferNode : Utils::ListNode<FramebufferNode>, Utils::HashmapNode<FramebufferNode>, Framebuffer
		{
			FramebufferNode(const RenderDevice& device, const RenderPass& rp, const RenderPassInfo& info)
				: Framebuffer(device, rp, info)
			{
			}
		};

		RenderDevice* renderDevice;
		Utils::TemporaryHashmap<FramebufferNode, FRAMEBUFFER_RING_SIZE, false> framebufferCache;
	};
}

TRE_NS_END