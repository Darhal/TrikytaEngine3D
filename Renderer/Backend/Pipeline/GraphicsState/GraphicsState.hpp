#pragma once

#include <Renderer/Common.hpp>

TRE_NS_START

namespace Renderer
{
	struct InputAssemblyState
	{
		VkPrimitiveTopology topology;
		VkBool32 primitiveRestartEnable;
	};

	struct RasterizationState
	{
		VkBool32                                   depthClampEnable;
		VkBool32                                   rasterizerDiscardEnable;
		VkPolygonMode                              polygonMode;
		VkCullModeFlags                            cullMode;
		VkFrontFace                                frontFace;
		VkBool32                                   depthBiasEnable;
		float                                      depthBiasConstantFactor;
		float                                      depthBiasClamp;
		float                                      depthBiasSlopeFactor;
		float                                      lineWidth;
	};

	struct MultisampleState
	{
		VkSampleCountFlagBits rasterizationSamples;
		VkBool32 sampleShadingEnable;
		float minSampleShading;
		const VkSampleMask* pSampleMask;
		VkBool32 alphaToCoverageEnable;
		VkBool32 alphaToOneEnable;
	};

	struct StencilOpState
	{
		VkStencilOp failOp;
		VkStencilOp passOp;
		VkStencilOp depthFailOp;
		VkCompareOp compareOp;
	};

	struct DepthStencilState
	{
		VkBool32 depthTestEnable;
		VkBool32 depthWriteEnable;
		VkCompareOp depthCompareOp;
		VkBool32 depthBoundsTestEnable;
		VkBool32 stencilTestEnable;
		StencilOpState front;
		StencilOpState back;
		float minDepthBounds;
		float maxDepthBounds;
	};

	typedef VkPipelineColorBlendAttachmentState ColorBlendAttachmentState;

	struct ColorBlendState
	{
		VkBool32 logicOpEnable;
		VkLogicOp logicOp;
		uint32_t attachmentCount;
		const ColorBlendAttachmentState* pAttachments;
		float blendConstants[4];
	};

	class GraphicsState
	{
	public:
		GraphicsState();

		void Reset();
	private:
		InputAssemblyState			inputAssemblyState;
		RasterizationState			rasterizationState;
		MultisampleState			multisampleState;
		DepthStencilState			depthStencilState;
		ColorBlendState				colorBlendState;
		ColorBlendAttachmentState	colorBlendAttachmetns[8];

		friend class GraphicsPipeline;
	};
}

TRE_NS_END
