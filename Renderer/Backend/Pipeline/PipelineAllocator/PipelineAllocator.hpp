#pragma once

#include <Renderer/Common.hpp>
#include <Renderer/Backend/Common/Globals.hpp>
#include <Renderer/Backend/Pipeline/Pipeline.hpp>
#include <Renderer/Core/Hashmap/TemporaryHashmap.hpp>

TRE_NS_START

namespace Renderer
{
	class RenderBackend;
	class GraphicsPipleine;
	class ShaderProgram;
	class RenderPass;

	class RENDERER_API PipelineAllocator
	{
	public:
		PipelineAllocator(RenderBackend* backend);

		Pipeline& RequestPipline(const ShaderProgram& program, const RenderPass& rp, const GraphicsState& state);

		Pipeline& RequestPipline(const ShaderProgram& program);

		void BeginFrame();

		void Clear();
	private:
		RenderBackend* renderBackend;
		std::unordered_map<Hash, Pipeline> pipelineCache;
	};
}

TRE_NS_END