#pragma once

#include <Renderer/Backend/RHI/Common/Globals.hpp>
#include <Renderer/Backend/RHI/Buffers/Buffer.hpp>

TRE_NS_START

namespace Renderer
{
	class ShaderProgram;
	class Pipeline;
    class RenderDevice;

	class SBT
	{
	public:
		SBT() = default;

        void Init(RenderDevice& device, const ShaderProgram& program, Pipeline& pipline);

		// BufferHandle GetSbtBuffer() const { return sbtBuffer; };

		// VkDeviceAddress GetSbtAddress() const { return address; }

		BufferHandle GetSbtBuffer() const { return sbtBuffer; };

		VkDeviceAddress GetSbtAddress() const { return address; }

		const VkStridedDeviceAddressRegionKHR& GetSbtEntry(uint32 i) const { return sbtEntries[i]; }
	//private:
		BufferHandle sbtBuffer;
		VkDeviceAddress address;
		VkStridedDeviceAddressRegionKHR sbtEntries[4];
	};
}


TRE_NS_END
