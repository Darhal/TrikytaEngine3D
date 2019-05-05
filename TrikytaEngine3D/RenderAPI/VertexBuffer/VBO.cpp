#include "VBO.hpp"
#include <Core/Misc/Utils/Common.hpp>
#include <Core/Context/Extensions.hpp>
#include <Core/Misc/Defines/Debug.hpp>
#include <RenderAPI/GlobalState/GLState.hpp>

TRE_NS_START

void VBO::Generate(BufferTarget::buffer_target_t target)
{
	m_target = target;
	glGenBuffers(1, &m_ID);
}

VBO::VBO(BufferTarget::buffer_target_t target) : m_target(target), m_AutoClean(true)
{
	glGenBuffers(1, &m_ID);
}

void VBO::FillData(const void* data, ssize_t size, BufferUsage::buffer_usage_t usage)
{
	//ASSERTF(m_target != BufferTarget::UNKNOWN, "Attempt to fill data of a vertex buffer object without setting the target (VBO ID = %d)", m_ID);
	this->Use(); //glBindBuffer(m_target, m_ID);
	glBufferData(m_target, size, data, usage);
}

void VBO::SubFillData(const void* data, ssize_t offset, ssize_t length)
{
	//ASSERTF(m_target != BufferTarget::UNKNOWN, "Attempt to fill sub data of a vertex buffer object without setting the target (VBO ID = %d)", m_ID);
	this->Use(); //glBindBuffer(m_target, m_ID);
	glBufferSubData(m_target, offset, length, data);
}

void VBO::GetSubData(void* data, ssize_t offset, ssize_t length)
{
	//ASSERTF(m_target != BufferTarget::UNKNOWN, "Attempt to get sub data of a vertex buffer object without setting the target (VBO ID = %d_n)", m_ID);
	this->Use(); //glBindBuffer(m_target, m_ID);
	glGetBufferSubData(m_target, offset, length, data);
}

void VBO::Bind() const
{
	ASSERTF(m_ID != 0, "Attempt to bind a vertex buffer without generating it (VBO ID = %d_n)", m_ID);
	ASSERTF(m_target != BufferTarget::UNKNOWN, "Attempt to bind a vertex buffer object without setting the target (VBO ID = %d_n)", m_ID);
	glBindBuffer(m_target, m_ID);
}

void VBO::Use() const
{
	GLState::Bind(this);
}

void VBO::Unbind() const
{
	ASSERTF(m_ID != 0, "Attempt to bind a vertex buffer without generating it (VBO ID = %d_n)", m_ID);
	ASSERTF(m_target != BufferTarget::UNKNOWN, "Attempt to unbind a vertex buffer object without setting the target (VBO ID = %d_n)", m_ID);
	glBindBuffer(m_target, 0);
}

void VBO::Unuse() const
{
	GLState::Unbind(this);
}

VBO::~VBO()
{
	if (m_ID != 0 && m_AutoClean) {
		Clean();
	}
}

void VBO::Clean()
{
	if (m_ID != 0) {
		m_AutoClean = false;
		glDeleteBuffers(1, &m_ID);
	}
}

TRE_NS_END