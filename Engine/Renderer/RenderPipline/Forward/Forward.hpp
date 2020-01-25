#pragma once

#include <Core/Misc/Defines/Common.hpp>
#include <Renderer/Backend/CommandBuffer/CommandBuffer.hpp>
#include <Renderer/RenderPipline/Renderer/IRenderer.hpp>
#include <Renderer/Mesh/IPrimitiveMesh/IPrimitiveMesh.hpp>
#include <Renderer/Systems/MeshSystems/MeshSystem.hpp>
#include <Renderer/Systems/LightSystems/LightSystem.hpp>

TRE_NS_START

class ForwardRenderer : public IRenderer
{
public:
	enum {
		MAIN_PASS = 0
	};
public:
	ForwardRenderer();

	void Initialize(uint32 scr_width, uint32 scr_height);

	void SetupCommandBuffer(uint32 scr_width, uint32 scr_height);

	void SetupLightsBuffer();

	// void Draw(IPrimitiveMesh& mesh);

	void Render();

	CommandBuffer& GetCommandQueue() { return m_CommandQueue; }

	LightSystem& GetLightSystem() { return m_LightSystem; }
private:
	CommandBuffer m_CommandQueue;
	MeshSystem m_MeshSystem;
	LightSystem m_LightSystem;
	VboID m_LightBuffer;

	CONSTEXPR static float NEAR_PLANE = 0.1f;
	CONSTEXPR static float FAR_PLANE = 300.f;
	CONSTEXPR static uint32 MAX_LIGHTS = 32;
};

TRE_NS_END