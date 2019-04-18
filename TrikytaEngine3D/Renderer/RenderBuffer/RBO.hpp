#pragma once

#include <Core/Misc/Defines/Common.hpp>
#include <Renderer/Common.hpp>
#include <Core/Context/GLDefines.hpp>
#include <Renderer/FrameBuffer/FBO.hpp>

TRE_NS_START

namespace RBOInternal
{
	enum rbo_internal_format_t {
		DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8
	};
}

struct RenderbufferSettings
{
	RenderbufferSettings() : fbo(NULL), w(0), h(0), internal_format(RBOInternal::DEPTH24_STENCIL8), attachement(FBOAttachement::DEPTH_STENCIL_ATTACHMENT), color_index(0)
	{}

	RenderbufferSettings(
		FBO&& fbo, uint32 w, uint32 h, 
		RBOInternal::rbo_internal_format_t internal_format = RBOInternal::DEPTH24_STENCIL8,
		FBOAttachement::framebuffer_attachement_t attach = FBOAttachement::DEPTH_STENCIL_ATTACHMENT, 
		uint32 color_index = 0)
		: fbo(&fbo), w(w), h(h), internal_format(internal_format), attachement(attach), color_index(color_index)
	{}
	FBO* fbo;
	uint32 w, h;
	RBOInternal::rbo_internal_format_t internal_format;
	FBOAttachement::framebuffer_attachement_t attachement;
	uint8 color_index;
};

class FBO;

class RBO
{
public:
	RBO();
	RBO(const RenderbufferSettings& settings);
	~RBO();

	void SetStorage(uint32 w, uint32 h, RBOInternal::rbo_internal_format_t internal_format = RBOInternal::rbo_internal_format_t::DEPTH24_STENCIL8);
	void AttachToFBO(const FBO& fbo, FBOAttachement::framebuffer_attachement_t attchement = FBOAttachement::DEPTH_STENCIL_ATTACHMENT, uint8 color_index = 0);

	FORCEINLINE const int32 GetID() const;
	FORCEINLINE operator uint32() const;
	FORCEINLINE const int32 GetTarget() const;
	FORCEINLINE const TargetType::target_type_t GetBindingTarget() const { return TargetType::target_type_t::RBO; }

	void Bind() const;
	void Use() const;

	void Unbind() const;
	void Unuse() const;
private:
	uint32 m_ID;
};

FORCEINLINE const int32 RBO::GetID() const
{
	return m_ID;
}

FORCEINLINE RBO::operator uint32() const
{
	return m_ID;
}

FORCEINLINE const int32 RBO::GetTarget() const
{
	return TargetType::target_type_t::RBO;
}

TRE_NS_END