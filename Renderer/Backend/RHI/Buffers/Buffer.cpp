#include "Buffer.hpp"
#include <Renderer/Backend/RHI/Swapchain/Swapchain.hpp>
#include <Renderer/Backend/RHI/RenderDevice/RenderDevice.hpp>
#include <Renderer/Backend/Core/Alignement/Alignement.hpp>

TRE_NS_START

void Renderer::BufferDeleter::operator()(Buffer* buff)
{
    buff->device.GetObjectsPool().buffers.Free(buff);
}

Renderer::Buffer::Buffer(RenderDevice& dev, VkBuffer buffer, const BufferInfo& info, const MemoryAllocation& mem) :
    device(dev), bufferInfo(info), bufferMemory(mem), apiBuffer(buffer),
    ringSize(1), unitSize((uint32)info.size), bufferIndex(0)
{
}

Renderer::Buffer::Buffer(RenderDevice& dev, VkBuffer buffer, const BufferInfo& info,
                         const MemoryAllocation& mem, uint32 unitSize, uint32 ringSize) :
    device(dev), bufferInfo(info), bufferMemory(mem), apiBuffer(buffer),
    ringSize(ringSize), unitSize(unitSize), bufferIndex(0)
{
}

Renderer::Buffer::~Buffer()
{
    if (apiBuffer != VK_NULL_HANDLE) {
        device.DestroyBuffer(apiBuffer);
        device.FreeMemory(bufferMemory);
        apiBuffer = VK_NULL_HANDLE;
    }
}

void Renderer::Buffer::WriteToBuffer(VkDeviceSize size, const void* data, VkDeviceSize offset, VkDeviceSize alignement)
{
	ASSERTF(!data, "Can't write to a buffer that have its memory unmapped (or data is NULL)");
    const uint32 memIndex = MemoryAllocation::GetTypeIndex(bufferMemory);
    const uint32 cpuFlags = device.IsMemoryInDomains(memIndex, CPU_MEMORY_TYPES);

    if (cpuFlags) {
        void* bufferData = (uint8*)bufferMemory.mappedData + bufferMemory.offset + offset;
        memcpy(bufferData, data, size);

        if (cpuFlags & (1u << (uint32)MemoryDomain::CPU_CACHED) && bufferInfo.domain == MemoryDomain::CPU_CACHED) {
            VkMappedMemoryRange range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
            range.memory = bufferMemory.memory;
            range.offset = bufferMemory.offset + offset;
            range.size   = size; //Utils::AlignUp(size, 256);
            //vkFlushMappedMemoryRanges(device.GetDevice(), 1, &range);
        }
    } else {
        device.GetStagingManager().Stage(apiBuffer, data, size, alignement, offset);
    }
}

void Renderer::Buffer::WriteToRing(VkDeviceSize size, const void* data, VkDeviceSize offset, VkDeviceSize alignement)
{
    ASSERTF(!data, "Can't write to a buffer that have its memory unmapped (or data is NULL)");
    const uint32 memIndex = MemoryAllocation::GetTypeIndex(bufferMemory);
    const uint32 cpuFlags = device.IsMemoryInDomains(memIndex, CPU_MEMORY_TYPES);

    if (cpuFlags) {
        bufferIndex = (bufferIndex + 1) % ringSize;
        void* bufferData = (uint8*)bufferMemory.mappedData + bufferMemory.offset + bufferIndex * unitSize + offset;
        memcpy(bufferData, data, size);

        if (cpuFlags & (1u << (uint32)MemoryDomain::CPU_CACHED) && bufferInfo.domain == MemoryDomain::CPU_CACHED) {
            VkMappedMemoryRange range{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
            range.memory = bufferMemory.memory;
            range.offset = bufferMemory.offset + bufferIndex * unitSize + offset;
            range.size = size;
            // vkFlushMappedMemoryRanges(device.GetDevice(), 1, &range);
        }
    }else{
        bufferIndex = (bufferIndex + 1) % ringSize;
        device.GetStagingManager().Stage(apiBuffer, data, size, alignement, bufferIndex * unitSize + offset);
    }
}

TRE_NS_END
