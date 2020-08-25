#pragma once

#include <Renderer/Common.hpp>
#include <Renderer/Backend/Common/Globals.hpp>
#include <Renderer/Backend/ShaderProgram/ShaderProgram.hpp>
#include <Engine/Core/DataStructure/Vector/Vector.hpp>

TRE_NS_START

namespace Renderer
{
	class RENDERER_API RenderDevice
	{
	public:
		RenderDevice();

		int32 CreateRenderDevice(const Internal::RenderInstance& renderInstance, const Internal::RenderContext& ctx);

		void DestroryRenderDevice();

		int32 CreateLogicalDevice(const Internal::RenderInstance& renderInstance, const Internal::RenderContext& ctx);

		void CreateShaderProgram(const std::initializer_list<ShaderProgram::ShaderStage>& shaderStages, ShaderProgram* shaderProgramOut);
	private:
		typedef bool(*FPN_RankGPU)(VkPhysicalDevice, VkSurfaceKHR);

		static bool IsDeviceSuitable(VkPhysicalDevice gpu, VkSurfaceKHR surface);

		static VkPhysicalDevice PickGPU(const Internal::RenderInstance& renderInstance, const Internal::RenderContext& ctx, FPN_RankGPU p_pick_func = IsDeviceSuitable);

		static Internal::QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice p_gpu, VkSurfaceKHR p_surface = NULL);

	private:
		Internal::RenderDevice internal;

		friend class RenderBackend;
	};
};

TRE_NS_END