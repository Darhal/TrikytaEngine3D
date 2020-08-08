#pragma once

#include <Renderer/Common.hpp>
#include <Renderer/Core/Common/Globals.hpp>


TRE_NS_START

namespace Renderer
{
	RENDERER_API int32 Init(RenderEngine& engine, TRE::Window* wnd);

	RENDERER_API void Destrory(RenderEngine& engine);
};

TRE_NS_END