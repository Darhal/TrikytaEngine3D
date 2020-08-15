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
#include <Renderer/Core/Renderer.hpp>
#include <Renderer/Core/SwapChain/SwapChain.hpp>
#include <Renderer/Core/Buffer/Buffer.hpp>
#include <Renderer/Window/Window.hpp>
#include <Renderer/Core/Pipeline/GraphicsPipeline.hpp>
#include <Renderer/Core/Common/Utils.hpp>

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

void updateMVP(TRE::Renderer::RenderEngine& engine, TRE::Renderer::Buffer& buffer)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    TRE::Renderer::SwapChainData& swapData = engine.renderContext->swapChainData;

    MVP mvp{};

    mvp.model   = glm::rotate(glm::mat4(1.0f), time * TRE::Math::ToRad(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp.view    = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp.proj    = glm::perspective<float>(TRE::Math::ToRad(45.0f), swapData.swapChainExtent.width / (float)swapData.swapChainExtent.height, 0.1, 10.f);
    mvp.proj[1][1] *= -1;
    
    //mvp.model.transpose();
    //mvp.view.transpose();
    //mvp.proj.transpose();
   
    void* data;
    vkMapMemory(engine.renderDevice->device, buffer.bufferMemory, 0, sizeof(mvp), 0, &data);
    memcpy(data, &mvp, sizeof(mvp));
    vkUnmapMemory(engine.renderDevice->device, buffer.bufferMemory);
}

void InitRenderPassDesc(const TRE::Renderer::RenderEngine& engine, TRE::Renderer::RenderPassDesc& desc)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = engine.renderContext->swapChainData.swapChainImageFormat;
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

    desc.attachments.EmplaceBack(colorAttachment);
    desc.subpassDependency.EmplaceBack(dependency);
    desc.subpassesDesc.EmplaceBack(subpass);
}

void InitGraphicsPipelineDesc(const TRE::Renderer::RenderEngine& engine, TRE::Renderer::GraphicsPiplineDesc& desc)
{
    auto vertShaderCode = TRE::Renderer::ReadShaderFile("shaders/vert.spv");
    auto fragShaderCode = TRE::Renderer::ReadShaderFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = TRE::Renderer::CreateShaderModule(engine.renderDevice->device, vertShaderCode);
    VkShaderModule fragShaderModule = TRE::Renderer::CreateShaderModule(engine.renderDevice->device, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderInfo{};
    vertexShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderInfo.module = vertShaderModule;
    vertexShaderInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    desc.shaderStagesDesc.EmplaceBack(vertexShaderInfo);
    desc.shaderStagesDesc.EmplaceBack(fragShaderStageInfo);

    TRE::Renderer::VertexInputDesc vertexInputDesc;
    vertexInputDesc.binding = 0;
    vertexInputDesc.stride = sizeof(TRE::Renderer::Vertex);
    vertexInputDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputDesc.attribDesc.EmplaceBack(TRE::Renderer::VertextAttribDesc{ 0, offsetof(TRE::Renderer::Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT });
    vertexInputDesc.attribDesc.EmplaceBack(TRE::Renderer::VertextAttribDesc{ 1, offsetof(TRE::Renderer::Vertex, color), VK_FORMAT_R32G32B32_SFLOAT });
    desc.vertexInputDesc.EmplaceBack(vertexInputDesc);


    desc.inputAssemblyDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    desc.inputAssemblyDesc.primitiveRestartEnable = VK_FALSE;

    desc.rasterStateDesc.depthClampEnable = VK_FALSE;
    desc.rasterStateDesc.rasterizerDiscardEnable = VK_FALSE;
    desc.rasterStateDesc.polygonMode = VK_POLYGON_MODE_FILL;
    desc.rasterStateDesc.lineWidth = 1.0f;
    desc.rasterStateDesc.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT;
    desc.rasterStateDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    desc.rasterStateDesc.depthBiasEnable = VK_FALSE;


    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding                = 0;
    uboLayoutBinding.descriptorType         = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount        = 1;
    uboLayoutBinding.stageFlags             = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers     = NULL; // Optional
    
    VkDescriptorSetLayoutCreateInfo  layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings    = &uboLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(engine.renderDevice->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    desc.piplineLayoutDesc.descriptorSetLayout.EmplaceBack(descriptorSetLayout);

    desc.viewport.x = 0.0f;
    desc.viewport.y = 0.0f;
    desc.viewport.width = (float)engine.renderContext->swapChainData.swapChainExtent.width;
    desc.viewport.height = (float)engine.renderContext->swapChainData.swapChainExtent.height;
    desc.viewport.minDepth = 0.0f;
    desc.viewport.maxDepth = 1.0f;

    desc.scissor.offset = { 0, 0 };
    desc.scissor.extent = engine.renderContext->swapChainData.swapChainExtent;

    desc.subpass = 0;
    desc.basePipelineHandle = 0;
    desc.basePipelineIndex = -1;
}

void RenderFrame(TRE::Renderer::RenderEngine& engine, TRE::Renderer::GraphicsPipeline& graphicsPipeline, TRE::Renderer::Buffer& vertexBuffer, TRE::Renderer::Buffer& indexBuffer, const std::vector<VkDescriptorSet>& descriptorSets)
{
    TRE::Renderer::RenderContext& ctx = *engine.renderContext;
    TRE::Renderer::RenderDevice& renderDevice = *engine.renderDevice;
    TRE::Renderer::SwapChainData& swapChainData = ctx.swapChainData;
    TRE::Renderer::RenderContextData& ctxData = ctx.contextData;
    TRE::Renderer::ContextFrameResources& ctxResource = TRE::Renderer::GetCurrentFrameResource(ctx);

    VkDevice device = renderDevice.device;    
    VkCommandBuffer currentCmdBuff = ctxResource.graphicsCommandBuffer;

    vkResetCommandBuffer(currentCmdBuff, 0);

    VkClearColorValue clearColor = { 164.0f / 256.0f, 30.0f / 256.0f, 34.0f / 256.0f, 0.0f };
    VkClearValue clearValue = {};
    clearValue.color = clearColor;

    VkViewport viewport{};
    viewport.width      = (float)swapChainData.swapChainExtent.width;
    viewport.height     = (float)swapChainData.swapChainExtent.height;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainData.swapChainExtent;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;


    if (vkBeginCommandBuffer(currentCmdBuff, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    if (!renderDevice.isTransferQueueSeprate && ctxData.transferRequests) {
        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

        vkCmdPipelineBarrier(currentCmdBuff,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT,
            1, &memoryBarrier, 0, NULL, 0, NULL
        );
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass           = graphicsPipeline.renderPass;
    renderPassInfo.framebuffer          = ctxResource.swapChainFramebuffer;
    renderPassInfo.renderArea.offset    = { 0, 0 };
    renderPassInfo.renderArea.extent    = swapChainData.swapChainExtent;
    renderPassInfo.clearValueCount      = 1;
    renderPassInfo.pClearValues         = &clearValue;

    vkCmdBeginRenderPass(currentCmdBuff, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(currentCmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipeline);

    vkCmdSetViewport(currentCmdBuff, 0, 1, &viewport);
    vkCmdSetScissor(currentCmdBuff, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(currentCmdBuff, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(currentCmdBuff, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(currentCmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 0, 1, &descriptorSets[ctxData.currentBuffer], 0, NULL);

    // vkCmdDraw(currentCmdBuff, 3, 1, 0, 0);
    vkCmdDrawIndexed(currentCmdBuff, 6, 1, 0, 0, 0);

    vkCmdEndRenderPass(currentCmdBuff);

    //VkImageSubresourceRange imgRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    //vkCmdClearColorImage(currentCmdBuff, swapChainData.swapChainImages[currentBuffer], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imgRange);

    if (vkEndCommandBuffer(currentCmdBuff) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
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

    const unsigned int SCR_WIDTH = 1920 / 2;
    const unsigned int SCR_HEIGHT = 1080 / 2;

    TRE::Event ev;
    TRE::Window window(SCR_WIDTH, SCR_HEIGHT, "Trikyta ENGINE 3 (Vulkan 1.2)", WindowStyle::Resize);
    TRE::Renderer::RenderEngine engine{0};

    if (TRE::Renderer::Init(engine, &window) == 0) {
        puts("Rendering engine created with sucesss !");
    }

    TRE::Renderer::RenderContext& ctx = *engine.renderContext;
    TRE::Renderer::RenderDevice& renderDevice = *engine.renderDevice;
    TRE::Renderer::SwapChainData& swapChainData = engine.renderContext->swapChainData;

    uint32 queuFamilesIndiciesSeprate[] = {
        renderDevice.queueFamilyIndices.queueFamilies[TRE::Renderer::QFT_GRAPHICS],
        renderDevice.queueFamilyIndices.queueFamilies[TRE::Renderer::QFT_TRANSFER]
    };

    std::vector<TRE::Renderer::Vertex> vertices = {
        {TRE::vec3{-0.5f, -0.5f, 0.f},  TRE::vec3{1.0f, 0.0f, 0.0f}},
        {TRE::vec3{0.5f, -0.5f, 0.f},   TRE::vec3{0.0f, 1.0f, 0.0f}},
        {TRE::vec3{0.5f, 0.5f, 0.f},    TRE::vec3{0.0f, 0.0f, 1.0f}},
        {TRE::vec3{-0.5f, 0.5f, 0.f},   TRE::vec3{1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    size_t vertexSize = sizeof(vertices[0]) * vertices.size();

    uint32 queuesFamilyCount = 0;
    uint32* queuesFamilies = NULL;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (renderDevice.isTransferQueueSeprate){
        queuesFamilyCount = 2;
        queuesFamilies = queuFamilesIndiciesSeprate;
        sharingMode = VK_SHARING_MODE_CONCURRENT;
    }

    TRE::Renderer::Buffer staginVertexBuffer = TRE::Renderer::CreateStaginBuffer(renderDevice, vertexSize, vertices.data());
    TRE::Renderer::Buffer vertexBuffer =
        TRE::Renderer::CreateBuffer(renderDevice, sizeof(vertices[0]) * vertices.size(), NULL,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sharingMode,
            queuesFamilyCount, queuesFamilies
        );

    TRE::Renderer::Buffer staginIndexBuffer = TRE::Renderer::CreateStaginBuffer(renderDevice, sizeof(indices[0]) * indices.size(), indices.data());
    TRE::Renderer::Buffer indexBuffer =
        TRE::Renderer::CreateBuffer(renderDevice, sizeof(indices[0]) * indices.size(), NULL,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sharingMode,
            queuesFamilyCount, queuesFamilies
        );

    TRE::Renderer::TransferBufferInfo transferInfo[2];
    transferInfo[0].srcBuffer = &staginVertexBuffer;
    transferInfo[0].dstBuffer = &vertexBuffer;
    transferInfo[0].copyRegions.EmplaceBack(VkBufferCopy{ 0, 0, vertexSize });

    transferInfo[1].srcBuffer = &staginIndexBuffer;
    transferInfo[1].dstBuffer = &indexBuffer;
    transferInfo[1].copyRegions.EmplaceBack(VkBufferCopy{ 0, 0, sizeof(indices[0]) * indices.size() });

    TRE::Renderer::TransferBuffers(*engine.renderContext, 2, transferInfo);


    TRE::Renderer::GraphicsPipeline graphicsPipeline;
    TRE::Renderer::GraphicsPiplineDesc pipelineDesc{};
    graphicsPipeline.renderPass = ctx.contextData.renderPass;
    InitGraphicsPipelineDesc(engine, pipelineDesc);
    TRE::Renderer::CreateGraphicsPipeline(renderDevice, graphicsPipeline, pipelineDesc);

    std::vector<TRE::Renderer::Buffer> uniformBuffers(ctx.contextData.imagesCount);

    for (uint32 i = 0; i < ctx.contextData.imagesCount; i++) {
        uniformBuffers[i] =
            TRE::Renderer::CreateBuffer(renderDevice, sizeof(MVP), NULL,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type            =  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = (uint32_t)(ctx.contextData.imagesCount);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount  = 1;
    poolInfo.pPoolSizes     = &poolSize;
    poolInfo.maxSets        = (uint32_t)(ctx.contextData.imagesCount);

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(renderDevice.device, &poolInfo, NULL, &descriptorPool) != VK_SUCCESS) {
        ASSERTF(true, "failed to create descriptor pool!");
    }

    std::vector<VkDescriptorSetLayout> layouts(ctx.contextData.imagesCount, pipelineDesc.piplineLayoutDesc.descriptorSetLayout[0]);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool        = descriptorPool;
    allocInfo.descriptorSetCount    = static_cast<uint32_t>(ctx.contextData.imagesCount);
    allocInfo.pSetLayouts           = layouts.data();

    std::vector<VkDescriptorSet> descriptorSets(ctx.contextData.imagesCount);
    if (vkAllocateDescriptorSets(renderDevice.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (uint32 i = 0; i < ctx.contextData.imagesCount; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MVP);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType       = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet      = descriptorSets[i];
        descriptorWrite.dstBinding  = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo  = NULL; // Optional
        descriptorWrite.pTexelBufferView = NULL; // Optional

        vkUpdateDescriptorSets(renderDevice.device, 1, &descriptorWrite, 0, NULL);
    }

    while (window.isOpen()) {
        window.getEvent(ev);
       
        if (ev.Type == TRE::Event::TE_RESIZE) {
            TRE::Renderer::UpdateSwapChain(engine);
            continue;
        }

        TRE::Renderer::PrepareFrame(engine);

        //if ((rand() % 11) <= 5) {
            //for (uint32 i = 0; i < 3; i++) {
                //float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                //float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                //float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

                //float r = sin(TRE::Math::ToRad((double)i));
                //float g = cos(TRE::Math::ToRad((double)i));
                //vertices[0].pos = TRE::vec3{ r, vertices[0].pos.y, 0 };
                //vertices[1].pos = TRE::vec3{ vertices[1].pos.x, g, 0 };
            //}

            //TRE::Renderer::EditBuffer(renderDevice, staginVertexBuffer, vertexSize, vertices.data());
            //TRE::Renderer::TransferBuffers(*engine.renderContext, 1, &transferInfo);

            //srand(static_cast <unsigned> (time(0)));
        //}
        updateMVP(engine, uniformBuffers[ctx.contextData.currentBuffer]);
        RenderFrame(engine, graphicsPipeline, vertexBuffer, indexBuffer, descriptorSets);
     
        TRE::Renderer::Present(engine);

        printFPS();
    }
    
    TRE::Renderer::Destrory(engine);
    getchar();
}

#endif