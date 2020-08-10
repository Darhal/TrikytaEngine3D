#include "SwapChain.hpp"
#include <Renderer/Core/Common/Utils.hpp>
#include <Renderer/Window/Window.hpp>
#include <Renderer/Core/Pipeline/GraphicsPipeline.hpp>

TRE_NS_START

Renderer::SwapChainSupportDetails Renderer::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    if (formatCount != 0) {
        details.formats.Resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.Data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

    if (presentModeCount != 0) {
        details.presentModes.Resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.Data());
    }

    return details;
}

void Renderer::CreateSwapChain(RenderContext& ctx, const RenderInstance& renderInstance, const RenderDevice& renderDevice)
{
    ASSERT(renderDevice.gpu == VK_NULL_HANDLE);
    ASSERT(ctx.surface == VK_NULL_HANDLE);

    uint32 width = ctx.window->getSize().x;
    uint32 height = ctx.window->getSize().y;
    VkSwapchainKHR oldSwapChain = ctx.swapChain;

    ctx.swapChainData.swapChainExtent        = VkExtent2D{ width, height };
    
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(renderDevice.gpu, ctx.surface);
    VkSurfaceFormatKHR surfaceFormat         = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode             = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent                        = ChooseSwapExtent(swapChainSupport.capabilities, ctx.swapChainData.swapChainExtent);
    uint32 imageCount                        = swapChainSupport.capabilities.minImageCount;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR    createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = ctx.surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    const QueueFamilyIndices& indices = renderDevice.queueFamilyIndices;
    uint32 queueFamilyIndices[]       = { indices.queueFamilies[QFT_GRAPHICS], indices.queueFamilies[QFT_PRESENT] };

    if (indices.queueFamilies[QFT_GRAPHICS] != indices.queueFamilies[QFT_PRESENT]) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform     = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = presentMode;
    createInfo.clipped          = VK_TRUE;
    createInfo.oldSwapchain     = oldSwapChain;

    if (vkCreateSwapchainKHR(renderDevice.device, &createInfo, NULL, &ctx.swapChain) != VK_SUCCESS) {
        ASSERTF(true, "Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(renderDevice.device, ctx.swapChain, &imageCount, NULL);
    ctx.swapChainData.swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(renderDevice.device, ctx.swapChain, &imageCount, ctx.swapChainData.swapChainImages.data());

    ctx.swapChainData.swapChainImageFormat = surfaceFormat.format;
    ctx.swapChainData.swapChainExtent      = extent;

    if (oldSwapChain == VK_NULL_HANDLE) {
        CreateSyncObjects(renderDevice, ctx);
        CreateSwapChainRenderPass(renderDevice, ctx);
        CreateCommandPool(renderDevice, ctx);
        CreateCommandBuffers(renderDevice, ctx);
    }else {
        vkDestroySwapchainKHR(renderDevice.device, oldSwapChain, NULL);
    }

    CreateImageViews(renderDevice, ctx);
    CreateFrameBuffers(renderDevice, ctx);
}

void Renderer::DestroySwapChain(const RenderDevice& renderDevice, RenderContext& ctx)
{
    CleanupSwapChain(ctx, renderDevice);

    vkDestroySwapchainKHR(renderDevice.device, ctx.swapChain, NULL);
    ctx.swapChain = VK_NULL_HANDLE;

    for (size_t i = 0; i < SwapChainData::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(renderDevice.device, ctx.swapChainData.renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(renderDevice.device, ctx.swapChainData.imageAvailableSemaphores[i], NULL);
        vkDestroyFence(renderDevice.device, ctx.swapChainData.inFlightFences[i], NULL);
    }

    vkDestroyCommandPool(renderDevice.device, ctx.swapChainData.commandPool, NULL);
}

void Renderer::CleanupSwapChain(RenderContext& ctx, const RenderDevice& renderDevice)
{
    SwapChainData& swapChainData = ctx.swapChainData;
    VkDevice device              = renderDevice.device;

    for (size_t i = 0; i < swapChainData.swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(device, swapChainData.swapChainFramebuffers[i], NULL);
    }

    // vkFreeCommandBuffers(device, swapChainData.commandPool, static_cast<uint32_t>(swapChainData.commandBuffers.Size()), swapChainData.commandBuffers.Data());

    // vkDestroyPipeline(device, swapChainData.graphicsPipeline, NULL);
    // vkDestroyPipelineLayout(device, swapChainData.pipelineLayout, NULL);
    // vkDestroyRenderPass(device, swapChainData.renderPass, NULL);

    for (size_t i = 0; i < swapChainData.swapChainImageViews.Size(); i++) {
        vkDestroyImageView(device, swapChainData.swapChainImageViews[i], NULL);
    }
}

void Renderer::RecreateSwapChainInternal(RenderContext& ctx, const RenderInstance& renderInstance, const RenderDevice& renderDevice)
{
    vkDeviceWaitIdle(renderDevice.device);
    CleanupSwapChain(ctx, renderDevice);

    uint32 width = ctx.window->getSize().x;
    uint32 height = ctx.window->getSize().y;

    while (width == 0 || height == 0) {
        ctx.window->WaitEvents();

        width = ctx.window->getSize().x;
        height = ctx.window->getSize().y;
    }

    ctx.framebufferResized = false;
    CreateSwapChain(ctx, renderInstance, renderDevice);
}

void Renderer::UpdateSwapChain(RenderEngine& engine)
{
    engine.renderContext->framebufferResized = true;
    // RecreateSwapChainInternal(*engine.renderContext, *engine.renderInstance, *engine.renderDevice);
}

void Renderer::Present(RenderEngine& engine, const TRE::Vector<VkCommandBuffer>& cmdbuff)
{
    RenderContext& ctx = *engine.renderContext;
    RenderDevice& renderDevice = *engine.renderDevice;
    SwapChainData& swapChainData = engine.renderContext->swapChainData;
    VkDevice device = renderDevice.device;
    uint32 currentFrame = swapChainData.currentFrame;

    vkWaitForFences(device, 1, &swapChainData.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t& imageIndex = swapChainData.imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, ctx.swapChain, UINT64_MAX, swapChainData.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // TRE::Log::Write(TRE::Log::ASSERT, "RESIZE occuring on acquire!\n");
        RecreateSwapChainInternal(*engine.renderContext, *engine.renderInstance, *engine.renderDevice);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        ASSERTF(true, "Failed to acquire swap chain image!\n");
    }

    if (swapChainData.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &swapChainData.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    swapChainData.imagesInFlight[imageIndex] = swapChainData.inFlightFences[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { swapChainData.imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;

    submitInfo.commandBufferCount   = 1; // (uint32)cmdbuff.Size();
    submitInfo.pCommandBuffers      = &swapChainData.commandBuffers[imageIndex]; // cmdbuff.Data();

    VkSemaphore signalSemaphores[]  = { swapChainData.renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    vkResetFences(device, 1, &swapChainData.inFlightFences[currentFrame]);

    // TRE::Log::Write(TRE::Log::ASSERT, "Draw cmd buffer %d", currentFrame);
    if (vkQueueSubmit(renderDevice.queues[QFT_GRAPHICS], 1, &submitInfo, swapChainData.inFlightFences[currentFrame]) != VK_SUCCESS) {
        ASSERTF(true, "failed to submit draw command buffer!");
    }

    // Presenting:
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;

    VkSwapchainKHR swapChains[] = { ctx.swapChain };
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapChains;
    presentInfo.pImageIndices   = &imageIndex;

    result = vkQueuePresentKHR(renderDevice.queues[QFT_PRESENT], &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || ctx.framebufferResized) {
        // ctx.framebufferResized = false;
        //TRE::Log::Write(TRE::Log::ASSERT, "RESIZE occuring on present!\n");
        RecreateSwapChainInternal(*engine.renderContext, *engine.renderInstance, *engine.renderDevice);
    } else if (result != VK_SUCCESS) {
        ASSERTF(true, "Failed to present swap chain image!");
    }

    // vkQueueWaitIdle(renderDevice.queues[QFT_PRESENT]);
    swapChainData.currentFrame = (currentFrame + 1) % SwapChainData::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::CreateImageViews(const RenderDevice& renderDevice, RenderContext& ctx)
{
    ctx.swapChainData.swapChainImageViews.Resize(ctx.swapChainData.swapChainImages.size());

    for (size_t i = 0; i < ctx.swapChainData.swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = ctx.swapChainData.swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = ctx.swapChainData.swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(renderDevice.device, &createInfo, NULL, &ctx.swapChainData.swapChainImageViews[i]) != VK_SUCCESS) {
            ASSERTF(true, "Failed to create image views!");
        }
    }
}

void Renderer::CreateSyncObjects(const RenderDevice& renderDevice, RenderContext& ctx)
{
    ctx.swapChainData.currentFrame = 0;

    ctx.swapChainData.imageAvailableSemaphores.resize(SwapChainData::MAX_FRAMES_IN_FLIGHT);
    ctx.swapChainData.renderFinishedSemaphores.resize(SwapChainData::MAX_FRAMES_IN_FLIGHT);
    ctx.swapChainData.inFlightFences.resize(SwapChainData::MAX_FRAMES_IN_FLIGHT);
    ctx.swapChainData.imagesInFlight.resize(ctx.swapChainData.swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < SwapChainData::MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(renderDevice.device, &semaphoreInfo, NULL, &ctx.swapChainData.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(renderDevice.device, &semaphoreInfo, NULL, &ctx.swapChainData.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(renderDevice.device, &fenceInfo, NULL, &ctx.swapChainData.inFlightFences[i]) != VK_SUCCESS) {
            
            ASSERTF(true, "failed to create synchronization objects for a frame!");
        }
    }
}

void Renderer::CreateCommandPool(const RenderDevice& renderDevice, RenderContext& ctx)
{
    const QueueFamilyIndices& queueFamilyIndices = renderDevice.queueFamilyIndices;

    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.queueFamilies[QFT_GRAPHICS];
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(renderDevice.device, &poolInfo, NULL, &ctx.swapChainData.commandPool) != VK_SUCCESS) {
            ASSERTF(true, "failed to create command pool!");
        }
    }
}

void Renderer::CreateCommandBuffers(const RenderDevice& renderDevice, RenderContext& ctx)
{
    SwapChainData& swapChainData = ctx.swapChainData;

    swapChainData.commandBuffers.Resize(swapChainData.MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = swapChainData.commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)swapChainData.commandBuffers.Size();

    if (vkAllocateCommandBuffers(renderDevice.device, &allocInfo, swapChainData.commandBuffers.Data()) != VK_SUCCESS) {
        ASSERTF(true, "failed to allocate command buffers!");
    }
}


void Renderer::CreateSwapChainRenderPass(const RenderDevice& renderDevice, RenderContext& ctx)
{
    RenderPassDesc renderpassDesc;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = ctx.swapChainData.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    renderpassDesc.attachments.EmplaceBack(colorAttachment);
    renderpassDesc.subpassDependency.EmplaceBack(dependency);
    renderpassDesc.subpassesDesc.EmplaceBack(subpass);

    CreateRenderPass(renderDevice, &ctx.swapChainData.renderPass, renderpassDesc);
}

void Renderer::CreateFrameBuffers(const RenderDevice& renderDevice, RenderContext& ctx)
{
    ctx.swapChainData.swapChainFramebuffers.resize(ctx.swapChainData.swapChainImageViews.Size());

    for (size_t i = 0; i < ctx.swapChainData.swapChainImageViews.Size(); i++) {
        VkImageView attachments[] = {
            ctx.swapChainData.swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = ctx.swapChainData.renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments    = attachments;
        framebufferInfo.width           = ctx.swapChainData.swapChainExtent.width;
        framebufferInfo.height          = ctx.swapChainData.swapChainExtent.height;
        framebufferInfo.layers          = 1;

        if (vkCreateFramebuffer(renderDevice.device, &framebufferInfo, NULL, &ctx.swapChainData.swapChainFramebuffers[i]) != VK_SUCCESS) {
            ASSERTF(true, "failed to create framebuffer!");
        }
    }
}

VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const TRE::Vector<VkSurfaceFormatKHR>& availableFormats) 
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Renderer::ChooseSwapPresentMode(const TRE::Vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& extent)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = extent;

        actualExtent.width = TRE::Math::Max<uint32>(
            capabilities.minImageExtent.width, 
            TRE::Math::Min<uint32>(capabilities.maxImageExtent.width, actualExtent.width)
        );
        actualExtent.height = TRE::Math::Max<uint32>(
            capabilities.minImageExtent.height, 
            TRE::Math::Min<uint32>(capabilities.maxImageExtent.height, actualExtent.height)
        );

        return actualExtent;
    }
}

TRE_NS_END
