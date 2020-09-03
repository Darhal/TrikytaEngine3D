#pragma once

#include <Renderer/Common.hpp>
#include <Renderer/Backend/Common/Globals.hpp>
#include <Engine/Core/DataStructure/Vector/Vector.hpp>

TRE_NS_START

namespace Renderer
{
    class RenderDevice;
    class RenderContext;

    class RENDERER_API Swapchain
    {
    public:
        struct SwapchainData
        {
            VkSemaphore						imageAcquiredSemaphores[MAX_FRAMES];
            VkSemaphore						drawCompleteSemaphores[MAX_FRAMES];
            VkFence							fences[MAX_FRAMES];

            // To use when using seprate presentation queue:
            VkSemaphore						imageOwnershipSemaphores[MAX_FRAMES];

            // To use when using seprate transfer queue:
            VkSemaphore						transferSemaphores[MAX_FRAMES];
            VkFence							transferSyncFence;

            // Swap chain images:
            VkImage							swapChainImages[MAX_IMAGES_COUNT];
            VkImageView						swapChainImageViews[MAX_IMAGES_COUNT];
            VkFramebuffer					swapChainFramebuffers[MAX_IMAGES_COUNT];

            // Other misc swapchain data:
            VkFormat						swapChainImageFormat;
            VkExtent2D						swapChainExtent;
        };

        struct SwapchainSupportDetails
        {
            VkSurfaceCapabilitiesKHR            capabilities;
            TRE::Vector<VkSurfaceFormatKHR>     formats;
            TRE::Vector<VkPresentModeKHR>       presentModes;
        };
    public:
        Swapchain(const RenderDevice& renderDevice, RenderContext& renderContext);

        static SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        void CreateSwapchain();
        void CreateSwapchainResources();
        void DestroySwapchain();
        void CleanupSwapchain();

        void UpdateSwapchain();

        void RecreateSwapchain();

        void CreateSyncObjects();

        void CreateSwapchainRenderPass();

        /*void CreateCommandPool(const RenderDevice& renderDevice, RenderContext& ctx);
        void CreateCommandBuffers(const RenderDevice& renderDevice, RenderContext& ctx);
        void BuildImageOwnershipCmd(const RenderDevice& renderDevice, RenderContext& ctx, uint32 imageIndex);*/

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const TRE::Vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const TRE::Vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& extent);

        FORCEINLINE VkFramebuffer GetCurrentFramebuffer() const;

        FORCEINLINE const SwapchainData& GetSwapchainData() const { return swapchainData; }

        FORCEINLINE const SwapchainSupportDetails& GetSwapchainSupportDetaims() const { return supportDetails; }

        FORCEINLINE VkSwapchainKHR GetAPIObject() const { return swapchain; }

        FORCEINLINE VkRenderPass GetRenderPass() const { return renderPass; }

        FORCEINLINE const VkExtent2D& GetExtent() const { return swapchainData.swapChainExtent; }

        FORCEINLINE VkFormat GetFormat() const { return swapchainData.swapChainImageFormat; }
    private:
        SwapchainData           swapchainData;
        SwapchainSupportDetails supportDetails;

        const RenderDevice&     renderDevice;
        RenderContext&          renderContext;
        VkSwapchainKHR          swapchain;
        VkRenderPass            renderPass;
        uint32                  imagesCount;

        bool					framebufferResized;

        friend class RenderContext;
    };

    /*namespace Internal 
    {
        struct SwapchainSupportDetails
        {
            VkSurfaceCapabilitiesKHR            capabilities;
            TRE::Vector<VkSurfaceFormatKHR>     formats;
            TRE::Vector<VkPresentModeKHR>       presentModes;
        };

        SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        void CreateSwapchain(const RenderDevice& renderDevice, RenderContext& ctx);
        void CreateSwapchainResources(const RenderDevice& renderDevice, RenderContext& ctx);
        void DestroySwapchain(const RenderDevice& renderDevice, RenderContext& ctx);
        void CleanupSwapchain(RenderContext& ctx, const RenderDevice& renderDevice);

        void UpdateSwapchain(RenderContext& ctx);

        void RecreateSwapchainInternal(const RenderDevice& renderDevice, RenderContext& ctx);

        void CreateSyncObjects(const RenderDevice& renderDevice, RenderContext& ctx);
        void CreateCommandPool(const RenderDevice& renderDevice, RenderContext& ctx);
        void CreateCommandBuffers(const RenderDevice& renderDevice, RenderContext& ctx);

        void CreateSwapchainRenderPass(const RenderDevice& renderDevice, RenderContext& ctx);

        void BuildImageOwnershipCmd(const RenderDevice& renderDevice, RenderContext& ctx, uint32 imageIndex);

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const TRE::Vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const TRE::Vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& extent);
    }*/
}

TRE_NS_END