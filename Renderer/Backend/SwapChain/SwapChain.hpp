#pragma once

#include <Renderer/Common.hpp>
#include <Engine/Core/DataStructure/Vector/Vector.hpp>
#include <Renderer/Backend/RenderContext/RenderContext.hpp>

TRE_NS_START

namespace Renderer
{
    namespace Internal 
    {
        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR            capabilities;
            TRE::Vector<VkSurfaceFormatKHR>     formats;
            TRE::Vector<VkPresentModeKHR>       presentModes;
        };

        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        void CreateSwapChain(const RenderDevice& renderDevice, RenderContext& ctx);
        void CreateSwapChainResources(const RenderDevice& renderDevice, RenderContext& ctx);
        void DestroySwapChain(const RenderDevice& renderDevice, RenderContext& ctx);
        void CleanupSwapChain(RenderContext& ctx, const RenderDevice& renderDevice);

        void UpdateSwapChain(RenderContext& ctx);

        void RecreateSwapChainInternal(const RenderDevice& renderDevice, RenderContext& ctx);

        void CreateSyncObjects(const RenderDevice& renderDevice, RenderContext& ctx);
        void CreateCommandPool(const RenderDevice& renderDevice, RenderContext& ctx);
        void CreateCommandBuffers(const RenderDevice& renderDevice, RenderContext& ctx);

        void CreateSwapChainRenderPass(const RenderDevice& renderDevice, RenderContext& ctx);

        void BuildImageOwnershipCmd(const RenderDevice& renderDevice, RenderContext& ctx, uint32 imageIndex);

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const TRE::Vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const TRE::Vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& extent);
    }
}

TRE_NS_END