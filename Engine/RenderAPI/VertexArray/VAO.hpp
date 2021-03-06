#pragma once

#include "VAODef.hpp"
#include <type_traits>

#include <Core/Misc/Defines/Common.hpp>
#include <Core/Context/Extensions.hpp>
#include <Core/DataStructure/Vector/Vector.hpp>
#include <RenderAPI/Common.hpp>
#include <RenderAPI/VertexBuffer/VBO.hpp>

TRE_NS_START

struct VertexSettings
{
	struct VertexBufferData {
		template<typename T>
		VertexBufferData(T* data, uint32 count, VBO* vbo) :
			buffer_settings(data, count),
			vbo(vbo)
		{}

		template<typename T>
		VertexBufferData(Vector<T>& data_container, VBO* vbo) :
			buffer_settings(data_container.StealPtr(), (uint32)data_container.Length()),
			vbo(vbo)
		{}

		VertexBufferData(const VertexBufferSettings& settings, VBO* vbo) : buffer_settings(settings), vbo(vbo)
		{}

		VertexBufferData() : buffer_settings(0), vbo(0)
		{}

		VertexBufferSettings buffer_settings;
		VBO* vbo;
	};

	struct VertexAttribute {
		VertexAttribute(uint32 vbo_index, uint32 attrib, uint32 sz, uint32 stride, usize offset, DataType::data_type_t type = DataType::FLOAT, uint32 divisor = 0) :
			offset(offset), vbo_index(vbo_index), size(sz), stride(stride), data_type(type), attrib_index(attrib), divisor(divisor)
		{}

		usize offset;
		uint32 vbo_index;
		uint32 size;
		uint32 stride;
		DataType::data_type_t data_type;
		uint32 divisor;
		uint8 attrib_index;
	};

	/*VertexSettings(VertexSettings&& other) : 
		vertices_data(std::move(other.vertices_data)), 
		attributes(std::move(other.attributes))
	{}*/

	Vector<VertexBufferData> vertices_data;
	Vector<VertexAttribute> attributes;
};

class VAO
{
public:
	FORCEINLINE VAO();

	FORCEINLINE ~VAO();

	FORCEINLINE uint32 Generate();

	DECLARE_BIND_FUNCS

	template<typename T>
	void BindAttribute(const uint32 attribute, const VBO& buffer, DataType::data_type_t type, uint32 size, uint32 stride, intptr offset);

	void BindAttribute(const uint32 attribute, const VBO& buffer, DataType::data_type_t type, uint32 count, int32 stride, const void* offset);

	FORCEINLINE const uint32 GetID() const { return m_ID; }
	FORCEINLINE operator uint32() const { return m_ID; }
	FORCEINLINE const TargetType::target_type_t GetBindingTarget() const { return TargetType::VAO; }

	FORCEINLINE void SetVertextAttribDivisor(uint32 attrib, uint32 div);

	FORCEINLINE void Bind() const;
	FORCEINLINE void Use() const;

	FORCEINLINE void Unbind() const;
	FORCEINLINE void Unuse() const;

	FORCEINLINE void Clean();

	FORCEINLINE void Invalidate() { m_ID = 0; }

	explicit FORCEINLINE VAO(VAO&& other);
	FORCEINLINE VAO& operator=(VAO&& other);

	explicit VAO(const VAO& other) = delete;
	VAO& operator=(const VAO& other) = delete;
	
private:
	uint32 m_ID;
};

FORCEINLINE VAO::VAO() : m_ID(0)
{
}

FORCEINLINE uint32 VAO::Generate()
{
	Call_GL(
		glGenVertexArrays(1, &m_ID)
	);
	return m_ID;
}

FORCEINLINE void VAO::Bind() const
{
	Call_GL(
		glBindVertexArray(m_ID)
	);
}

FORCEINLINE void VAO::Use() const
{
	GLState::Bind(this);
}

FORCEINLINE void VAO::Unbind() const
{
	Call_GL(
		glBindVertexArray(0)
	);
}

FORCEINLINE void VAO::Unuse() const
{
	GLState::Unbind(this);
}

FORCEINLINE void VAO::SetVertextAttribDivisor(uint32 attrib, uint32 div)
{
	Call_GL(
		glVertexAttribDivisor(attrib, div)
	);
}

FORCEINLINE VAO::~VAO()
{
	if (m_ID) {
		this->Clean();
	}
}

FORCEINLINE void VAO::Clean()
{
	Call_GL(
		glDeleteVertexArrays(1, &m_ID)
	);

	m_ID = 0;
}

template<typename T>
void VAO::BindAttribute(const uint32 attribute, const VBO& buffer, DataType::data_type_t type, uint32 size, uint32 stride, intptr offset)
{
	this->Use();
	Call_GL(
		glEnableVertexAttribArray(attribute)
	);
	Call_GL(
		glVertexAttribPointer(attribute, size, type, GL_FALSE, stride * sizeof(T), (const void*)(offset * sizeof(T)))
	);
}

FORCEINLINE VAO::VAO(VAO&& other) : m_ID(other.m_ID)
{
	other.m_ID = 0;
}

FORCEINLINE VAO& VAO::operator=(VAO&& other)
{
	m_ID = other.m_ID;
	other.m_ID = 0;
	return *this;
}

typedef VAO VertexArray;

TRE_NS_END