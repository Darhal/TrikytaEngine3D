#include "GraphicsState.hpp"

TRE_NS_START

CONSTEXPR static Renderer::InputAssemblyState defaultInputAssemblyState = {
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        false
};

CONSTEXPR static Renderer::RasterizationState defaultRasterizationState = {
    false,
    false,
    VK_POLYGON_MODE_FILL,
    VK_CULL_MODE_NONE,
    VK_FRONT_FACE_COUNTER_CLOCKWISE,
    false,
    0.0f,
    1.0f,
    1.0f,
    1.0f
};

CONSTEXPR static Renderer::MultisampleState defaultMultisampleState = {
    VK_SAMPLE_COUNT_1_BIT,
    false,
    1.0f,
    nullptr,
    false,
    false
};

CONSTEXPR static Renderer::DepthStencilState defaultDepthStencilState = {
    false,
    true,
    VK_COMPARE_OP_LESS_OR_EQUAL,
    false,
    false,
    {
        VK_STENCIL_OP_REPLACE,
        VK_STENCIL_OP_REPLACE,
        VK_STENCIL_OP_REPLACE,
        VK_COMPARE_OP_NEVER,
    },
    {
        VK_STENCIL_OP_REPLACE,
        VK_STENCIL_OP_REPLACE,
        VK_STENCIL_OP_REPLACE,
        VK_COMPARE_OP_NEVER,
    },
    0.f,
    1.f
};

CONSTEXPR static Renderer::ColorBlendState defaultColorBlendState = {
    false,
    VK_LOGIC_OP_SET,
    0,
    NULL,
    {1.0f, 1.0f, 1.0f, 1.0f}
};


CONSTEXPR static Renderer::ColorBlendAttachmentState defaultColorBlendAttachmentState = {
    false,
    VK_BLEND_FACTOR_ONE,
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_OP_ADD,
    VK_BLEND_FACTOR_ONE,
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_OP_ADD,
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
};

Renderer::GraphicsState::GraphicsState()
{
    Reset();
}

void Renderer::GraphicsState::Reset()
{
    memcpy(&inputAssemblyState, &defaultInputAssemblyState, sizeof(InputAssemblyState));
    memcpy(&rasterizationState, &defaultRasterizationState,sizeof(RasterizationState));;
    memcpy(&multisampleState, &defaultMultisampleState, sizeof(MultisampleState));
    memcpy(&depthStencilState, &defaultDepthStencilState, sizeof(DepthStencilState));
	memcpy(&colorBlendState, &defaultColorBlendState, sizeof(ColorBlendState));

    for (uint32 i = 0; i < 8; i++)
        memcpy(&colorBlendAttachmetns[i], &defaultColorBlendAttachmentState, sizeof(ColorBlendAttachmentState));

    colorBlendState.attachmentCount = 0;
    colorBlendState.pAttachments = &colorBlendAttachmetns[0];
}

TRE_NS_END