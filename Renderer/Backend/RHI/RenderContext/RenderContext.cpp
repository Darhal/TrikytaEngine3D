#include "RenderContext.hpp"
#include <Renderer/Backend/Window/Window.hpp>
#include <Renderer/Backend/RHI/WindowSurface/WindowSurface.hpp>
#include <Renderer/Backend/RHI/RenderInstance/RenderInstance.hpp>
#include <Renderer/Backend/RHI/Buffers/Buffer.hpp>
#include <Renderer/Backend/RHI/RenderDevice/RenderDevice.hpp>
#include <Renderer/Backend/RHI/RenderBackend.hpp>
#include <unordered_set>

TRE_NS_START

Renderer::RenderContext::RenderContext(RenderBackend& backend) : internal{ 0 }, renderDevice(&backend.GetRenderDevice()), swapchain(backend)
{
    internal.numFramesInFlight = NUM_FRAMES;
    internal.currentFrame = 0;
    internal.previousFrame = (internal.currentFrame + (NUM_FRAMES - 1)) % NUM_FRAMES;
}

void Renderer::RenderContext::CreateRenderContext(TRE::Window* wnd, const Internal::RenderInstance& instance)
{
    internal.window = wnd;
    swapchain.swapchainData.swapChainExtent = VkExtent2D{ internal.window->getSize().x, internal.window->getSize().y };

    CreateWindowSurface(instance, internal);
}

void Renderer::RenderContext::InitRenderContext(const Internal::RenderInstance& renderInstance, const Internal::RenderDevice& renderDevice)
{
    swapchain.CreateSwapchain();
}

void Renderer::RenderContext::DestroyRenderContext(const Internal::RenderInstance& renderInstance,
                                                   const Internal::RenderDevice& renderDevice,
                                                   Internal::RenderContext& renderContext)
{
    swapchain.DestroySwapchain();
    Internal::DestroryWindowSurface(renderInstance.instance, renderContext.surface);
}

void Renderer::RenderContext::BeginFrame(const RenderDevice& renderDevice)
{
    Swapchain::SwapchainData& swapchainData = swapchain.swapchainData;
    VkDevice device = renderDevice.GetDevice();
    uint32 currentFrame = internal.currentFrame;

    vkWaitForFences(device, 1, &swapchainData.fences[currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(device, swapchain.GetApiObject(), UINT64_MAX, 
        swapchainData.imageAcquiredSemaphores[currentFrame], VK_NULL_HANDLE, &internal.currentImage);

    /*if (swapchainData.imageFences[internal.currentImage] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &swapchainData.imageFences[internal.currentImage], VK_TRUE, UINT64_MAX);
    }*/

    swapchainData.imageFences[internal.currentImage] = swapchainData.imageFences[currentFrame];
    vkResetFences(device, 1, &swapchainData.fences[currentFrame]);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapchain.QueueSwapchainUpdate();
        // printf("[BEGIN FRAME] Swapchain will be resized!\n");
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        ASSERTF(true, "Failed to acquire swap chain image!\n");
    }
}

void Renderer::RenderContext::EndFrame(const RenderDevice& renderDevice)
{
    const Swapchain::SwapchainData& swapchainData = swapchain.swapchainData;

    const VkQueue* queues = renderDevice.GetQueues();
    const uint32 currentFrame = internal.currentFrame;
    const uint32_t currentBuffer = internal.currentImage;

    /*VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore waitSemaphores[]      = { swapchainData.imageAcquiredSemaphores[currentFrame] };
    VkSemaphore signalSemaphores[]    = { swapchainData.drawCompleteSemaphores[currentFrame] };

    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = cmdBuffCount;
    submitInfo.pCommandBuffers      = commandBuffers;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    if (vkQueueSubmit(queues[Internal::QFT_GRAPHICS], 1, &submitInfo, swapchainData.fences[currentFrame]) != VK_SUCCESS) {
        ASSERTF(true, "failed to submit draw command buffer!");
    }*/
    
    /*if (renderDevice.IsPresentQueueSeprate()) {
        // If we are using separate queues, change image ownership to the
        // present queue before presenting, waiting for the draw complete
        // semaphore and signalling the ownership released semaphore when
        // finished
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo present_submit_info{};
        present_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        present_submit_info.waitSemaphoreCount = 1;
        present_submit_info.pWaitSemaphores = &swapchainData.drawCompleteSemaphores[currentFrame];
        present_submit_info.pWaitDstStageMask = waitStages;

        present_submit_info.commandBufferCount = 1;
        present_submit_info.pCommandBuffers = &frameResources.presentCommandBuffer; // TODO: change this command buffer (to graphics to present cmd)

        present_submit_info.signalSemaphoreCount = 1;
        present_submit_info.pSignalSemaphores = &swapchainData.imageOwnershipSemaphores[currentFrame];

        if (vkQueueSubmit(queues[Internal::QFT_PRESENT], 1, &present_submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
            ASSERTF(true, "failed to submit command buffer to present queue!");
        }
    }*/

    // Presenting:
    VkResult result = VK_SUCCESS;

    if (!swapchain.framebufferResized){
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = renderDevice.IsPresentQueueSeprate() ? &swapchainData.imageOwnershipSemaphores[currentFrame] :
                                                                             &swapchainData.drawCompleteSemaphores[currentFrame];
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = &swapchain.swapchain;
        presentInfo.pImageIndices   = &currentBuffer;

        result = vkQueuePresentKHR(queues[Internal::QFT_PRESENT], &presentInfo);
    }

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || swapchain.framebufferResized) {
        swapchain.RecreateSwapchain();
        // printf("[END FRAME] Swapchain resized! %d\n", result);
    } else if (result != VK_SUCCESS) {
        ASSERTF(true, "Failed to present swap chain image!");
    }

    internal.previousFrame = internal.currentFrame;
    internal.currentFrame = (currentFrame + 1) % internal.numFramesInFlight;

    // printf("End frame: %d | New frame: %d\n", currentFrame, internal.currentFrame);
    // vkDeviceWaitIdle(renderDevice.GetDevice());
}

TRE_NS_END
