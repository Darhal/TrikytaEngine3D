#include "pch.hpp"

#if not defined(BUILD_EXEC)

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#else

#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <Renderer/Window/Window.hpp>
#include <Renderer/Backend/RenderBackend.hpp>
#include <Renderer/Backend/SwapChain/SwapChain.hpp>
#include <Renderer/Backend/Buffers/Buffer.hpp>
#include <Renderer/Backend/Pipeline/GraphicsPipeline.hpp>
#include <Renderer/Backend/Common/Utils.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Engine/Core/Misc/Maths/Maths.hpp>
#include <Engine/Core/Misc/Utils/Logging.hpp>

struct MVP
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

std::vector<TRE::Renderer::Internal::Vertex> vertices = {
    {TRE::vec3{-0.5f, -0.5f, 0.f},  TRE::vec3{1.0f, 0.0f, 0.0f}},
    {TRE::vec3{0.5f, -0.5f, 0.f},   TRE::vec3{0.0f, 1.0f, 0.0f}},
    {TRE::vec3{0.5f, 0.5f, 0.f},    TRE::vec3{0.0f, 0.0f, 1.0f}},
    {TRE::vec3{-0.5f, 0.5f, 0.f},   TRE::vec3{1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

void updateMVP(const TRE::Renderer::RenderBackend& backend, VkDescriptorSet descriptorSet, TRE::Renderer::RingBuffer& buffer)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    
    const TRE::Renderer::Swapchain::SwapchainData& swapchainData = backend.GetRenderContext().GetSwapchain().GetSwapchainData();

    MVP mvp{};

    mvp.model   = glm::rotate(glm::mat4(1.0f), time * TRE::Math::ToRad(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp.view    = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp.proj    = glm::perspective<float>(TRE::Math::ToRad(45.0f), swapchainData.swapChainExtent.width / (float)swapchainData.swapChainExtent.height, 0.1, 10.f);
    mvp.proj[1][1] *= -1;
    
    //mvp.model.transpose();
    //mvp.view.transpose();
    //mvp.proj.transpose();

    buffer.WriteToBuffer(sizeof(mvp), &mvp);
}

void RenderFrame(uint32 i,
    TRE::Renderer::RenderBackend& backend,
    TRE::Renderer::GraphicsPipeline& graphicsPipeline,
    TRE::Renderer::Buffer& vertexIndexBuffer,
    VkDescriptorSet descriptorSet,
    const TRE::Renderer::RingBuffer& uniformBuffer)
{
    const TRE::Renderer::Swapchain::SwapchainData& swapChainData = backend.GetRenderContext().GetSwapchain().GetSwapchainData();
    TRE::Renderer::CommandBufferHandle currentCmdBuff = backend.RequestCommandBuffer(TRE::Renderer::QueueTypes::GRAPHICS_ONLY);

    currentCmdBuff->Begin();

    currentCmdBuff->BeginRenderPass({ 164.0f / 256.0f, 30.0f / 256.0f, 34.0f / 256.0f, 0.0f });
    currentCmdBuff->BindPipeline(graphicsPipeline);
    currentCmdBuff->SetViewport({ 0.f, 0.f, (float)swapChainData.swapChainExtent.width, (float)swapChainData.swapChainExtent.height, 0.f, 1.f });
    currentCmdBuff->SetScissor({ {0, 0}, swapChainData.swapChainExtent });

    currentCmdBuff->BindVertexBuffer(vertexIndexBuffer);
    currentCmdBuff->BindIndexBuffer(vertexIndexBuffer, sizeof(vertices[0]) * vertices.size());

    const uint32 dynamicOffset[] = { uniformBuffer.GetCurrentOffset() };
    vkCmdBindDescriptorSets(currentCmdBuff->GetAPIObject(), 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        graphicsPipeline.GetPipelineLayout().GetAPIObject(),
        0, 1, &descriptorSet, 1, dynamicOffset);

    currentCmdBuff->DrawIndexed(6);
    
    currentCmdBuff->EndRenderPass();
    currentCmdBuff->End();

    backend.Submit(currentCmdBuff->GetAPIObject());
}

void printFPS() {
    static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();
    static int fps;

    fps++;

    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - oldTime) >= std::chrono::seconds{ 1 }) {
        oldTime = std::chrono::high_resolution_clock::now();
        std::cout << "FPS: " << fps << std::endl;
        fps = 0;
    }
}

int main()
{
    /*uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    std::vector<const char*> extensionsNames(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    int i = 0;
    std::cout << "available extensions:\n";
    for (const auto& extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
        extensionsNames[i++] = extension.extensionName;
    }*/

    const unsigned int SCR_WIDTH = 640;//1920 / 2;
    const unsigned int SCR_HEIGHT = 480;//1080 / 2;

    using namespace TRE::Renderer;
    using namespace TRE;

    Event ev;
    Window window(SCR_WIDTH, SCR_HEIGHT, "Trikyta ENGINE 3 (Vulkan 1.2)", WindowStyle::Resize);
    RenderBackend backend{ &window };

    Internal::RenderContext& ctx = backend.GetCtxInternal();
    Internal::RenderDevice& renderDevice = backend.GetDevInternal();

    size_t vertexSize = sizeof(vertices[0]) * vertices.size();
    size_t indexSize = sizeof(indices[0]) * indices.size();

    uint32 queueFamilies = QueueFamilyFlag::NONE;

    if (renderDevice.isTransferQueueSeprate){
        queueFamilies = QueueFamilyFlag::TRANSFER | QueueFamilyFlag::GRAPHICS;
    }

    char* data = new char[vertexSize + indexSize];
    memcpy(data, vertices.data(), vertexSize);
    memcpy(data + vertexSize, indices.data(), indexSize);

    const int MAX_VERTEX_BUFFERS = 1;
    Buffer vertexIndexBuffer[MAX_VERTEX_BUFFERS];

    for (int i = 0; i < MAX_VERTEX_BUFFERS; i++) {
        vertexIndexBuffer[i] =
            backend.CreateBuffer((vertexSize + indexSize) * (i + 1), NULL,
                BufferUsage::TRANSFER_DST | BufferUsage::VERTEX_BUFFER | BufferUsage::INDEX_BUFFER,
                MemoryUsage::GPU_ONLY, queueFamilies
            );

        backend.GetStagingManager().Stage(vertexIndexBuffer[i].GetAPIObject(), (void*)data, (vertexSize + indexSize) * (i + 1));
    }

    GraphicsPipeline graphicsPipeline;
    GraphicsState state;

    graphicsPipeline.GetShaderProgram().Create(renderDevice,
        { 
            {"shaders/vert.spv", ShaderProgram::VERTEX_SHADER}, 
            {"shaders/frag.spv", ShaderProgram::FRAGMENT_SHADER} 
        }
    );

    graphicsPipeline.GetShaderProgram().GetVertexInput().AddBinding(
        0, sizeof(Internal::Vertex), 
        VertexInput::LOCATION_0 | VertexInput::LOCATION_1, 
        { offsetof(Internal::Vertex, pos), offsetof(Internal::Vertex, color) }
    );

    graphicsPipeline.SetRenderPass(backend.GetRenderContext().GetSwapchain().GetRenderPass());
    graphicsPipeline.Create(backend.GetRenderContext(), state);

    TRE::Renderer::RingBuffer uniformBuffer = backend.CreateRingBuffer(sizeof(MVP), NULL, BufferUsage::UNIFORM_BUFFER, MemoryUsage::CPU_ONLY);

    VkDescriptorPoolSize poolSize{};
    poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount  = 1;
    poolInfo.pPoolSizes     = &poolSize;
    poolInfo.maxSets        = 1;

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(renderDevice.device, &poolInfo, NULL, &descriptorPool) != VK_SUCCESS) {
        ASSERTF(true, "failed to create descriptor pool!");
    }

    VkDescriptorSetLayout layouts[] = { graphicsPipeline.GetShaderProgram().GetDescriptorSetLayout(0).GetAPIObject() };
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool        = descriptorPool;
    allocInfo.descriptorSetCount    = 1;
    allocInfo.pSetLayouts           = layouts;

    VkDescriptorSet descriptorSet;
    if (vkAllocateDescriptorSets(renderDevice.device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (uint32 i = 0; i < 1/*ctx.imagesCount*/; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer.GetAPIObject();
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(MVP);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet           = descriptorSet;
        descriptorWrite.dstBinding       = 0;
        descriptorWrite.dstArrayElement  = 0;
        descriptorWrite.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrite.descriptorCount  = 1;
        descriptorWrite.pBufferInfo      = &bufferInfo;
        descriptorWrite.pImageInfo       = NULL; // Optional
        descriptorWrite.pTexelBufferView = NULL; // Optional

        vkUpdateDescriptorSets(renderDevice.device, 1, &descriptorWrite, 0, NULL);
    }

    for (uint32 i = 0; i < 4; i++) {
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

        //float r = sin(TRE::Math::ToRad((double)i));
        //float g = cos(TRE::Math::ToRad((double)i));
        //vertices[0].pos = TRE::vec3{ r, vertices[0].pos.y, 0 };
        //vertices[1].pos = TRE::vec3{ vertices[1].pos.x, g, 0 };

        vertices[i].color = TRE::vec3{ r, g, b };
    }


    INIT_BENCHMARK;

    time_t lasttime = time(NULL);

    while (window.isOpen()) {
        window.getEvent(ev);
       
        if (ev.Type == TRE::Event::TE_RESIZE) {
            backend.GetRenderContext().GetSwapchain().UpdateSwapchain();
            continue;
        }

        backend.BeginFrame();

        /*if (time(NULL) != lasttime) {
            lasttime = time(NULL);
            srand(lasttime);

            for (uint32 i = 0; i < 4; i++) {
                float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                // float a = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

                vertices[i].color = TRE::vec3{ r, g, b };
            }

            memcpy(data, vertices.data(), vertexSize);
            backend.GetRenderContext().GetStagingManager().Stage(vertexIndexBuffer.GetAPIObject(), (void*)data, vertexSize);
        }*/


        /*TRE::Renderer::Internal::EditBuffer(renderDevice, staginVertexBuffer, vertexSize, vertices.data());
        engine.GetRenderContext().TransferBuffers(1, &transferInfo[0]);*/

        updateMVP(backend, descriptorSet, uniformBuffer);
        RenderFrame(ctx.currentFrame, backend, graphicsPipeline, vertexIndexBuffer[0], descriptorSet, uniformBuffer);

        backend.EndFrame();
        printFPS();
    }
    
    delete[] data;
    getchar();
}

#endif







/*if (!renderDevice.isTransferQueueSeprate && ctxData.transferRequests) {
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

    vkCmdPipelineBarrier(currentCmdBuff,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT,
        1, &memoryBarrier, 0, NULL, 0, NULL
    );
}*/
// vkCmdDraw(currentCmdBuff, 3, 1, 0, 0);

//VkImageSubresourceRange imgRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
//vkCmdClearColorImage(currentCmdBuff, swapChainData.swapChainImages[currentBuffer], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imgRange);