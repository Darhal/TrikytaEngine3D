#include "CommandList.hpp"
#include <Renderer/Backend/RenderContext/RenderContext.hpp>
#include <Renderer/Backend/Pipeline/GraphicsPipeline.hpp>
#include <Renderer/Backend/Buffers/Buffer.hpp>
#include <Renderer/Backend/Descriptors/DescriptorSetAlloc.hpp>

TRE_NS_START

Renderer::CommandBuffer::CommandBuffer(RenderContext* renderContext, VkCommandBuffer buffer, Type type) :
    renderContext(renderContext), commandBuffer(buffer), type(type), allocatedSets{}
{
}

void Renderer::CommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        ASSERTF(true, "Failed to begin recording command buffer!");
    }
}

void Renderer::CommandBuffer::End()
{
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
       ASSERTF(true, "failed to record command buffer!");
    }
}

void Renderer::CommandBuffer::SetViewport(const VkViewport& viewport)
{
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void Renderer::CommandBuffer::SetScissor(const VkRect2D& scissor)
{
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::CommandBuffer::BeginRenderPass(VkClearColorValue clearColor)
{
    VkClearValue clearValue = {};
    clearValue.color = clearColor;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = renderContext->GetSwapchain().GetRenderPass();
    renderPassInfo.framebuffer       = renderContext->GetSwapchain().GetCurrentFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = renderContext->GetSwapchain().GetExtent();
    renderPassInfo.clearValueCount   = 1;
    renderPassInfo.pClearValues      = &clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::CommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::CommandBuffer::BindPipeline(const GraphicsPipeline& pipeline)
{
    this->pipeline = &pipeline;
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetAPIObject());
}

void Renderer::CommandBuffer::BindVertexBuffer(const Buffer& buffer, DeviceSize offset)
{
    VkBuffer vertexBuffers[] = { buffer.GetAPIObject() };
    VkDeviceSize offsets[]   = { offset };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
}

void Renderer::CommandBuffer::BindIndexBuffer(const Buffer& buffer, DeviceSize offset)
{
    vkCmdBindIndexBuffer(commandBuffer, buffer.GetAPIObject(), offset, VK_INDEX_TYPE_UINT16);
}

void Renderer::CommandBuffer::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance)
{
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void Renderer::CommandBuffer::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
{
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void Renderer::CommandBuffer::BindDescriptorSet(const GraphicsPipeline& pipeline, const std::initializer_list<VkDescriptorSet>& descriptors, 
    const std::initializer_list<uint32>& dyncOffsets)
{
    vkCmdBindDescriptorSets(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.GetPipelineLayout().GetAPIObject(),
        0, descriptors.size(), descriptors.begin(), dyncOffsets.size(), dyncOffsets.begin());
}

void Renderer::CommandBuffer::SetUniformBuffer(uint32 set, uint32 binding, const Buffer& buffer, DeviceSize offset, DeviceSize range)
{
    ASSERT(set >= MAX_DESCRIPTOR_SET);
    ASSERT(binding >= MAX_DESCRIPTOR_BINDINGS);

    auto& b = bindings.bindings[set][binding];
    b.buffer = VkDescriptorBufferInfo{ buffer.GetAPIObject(), 0, range };
    b.dynamicOffset = offset;

    dirtySets.dirtySets |= (1u << set);
    dirtySets.dirtyBindings[set] |= (1u << binding);
}

void Renderer::CommandBuffer::UpdateDescriptorSet(uint32 set, VkDescriptorSet descSet, const DescriptorSetLayout& layout, const ResourceBinding* bindings)
{
    VkWriteDescriptorSet writes[MAX_DESCRIPTOR_BINDINGS];
    uint32 writeCount = 0;

    for (uint32 binding = 0; binding < MAX_DESCRIPTOR_BINDINGS; binding++) {
        if (dirtySets.dirtyBindings[set] && (1u << binding)) {
            if (layout.GetDescriptorSetLayoutBinding(binding).descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                writes[writeCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[writeCount].pNext = NULL;
                writes[writeCount].dstSet = descSet;
                writes[writeCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                writes[writeCount].descriptorCount = 1;
                writes[writeCount].dstBinding = binding;
                writes[writeCount].dstArrayElement = 0;
                writes[writeCount].pBufferInfo = &bindings[binding].buffer;
                writes[writeCount].pImageInfo = NULL;
                writes[writeCount].pTexelBufferView = NULL;
                break;
            }
        }
    }
}

void Renderer::CommandBuffer::FlushDescriptorSet(uint32 set)
{
    ASSERT(pipeline == NULL);

    const PipelineLayout& layout = pipeline->GetShaderProgram().GetPipelineLayout();
    VkDescriptorSet descSet = layout.GetAllocator(set)->Allocate(); 

    this->UpdateDescriptorSet(set, descSet, layout.GetDescriptorSetLayout(set), bindings.bindings[set]);


    // TODO: start working from here
    allocatedSets[set] = descSet;
}

TRE_NS_END

