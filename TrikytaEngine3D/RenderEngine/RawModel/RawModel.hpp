#pragma once

#include <Core/Misc/Defines/Common.hpp>
#include <RenderAPI/VertexArray/VAO.hpp>
#include <RenderAPI/VertexBuffer/VBO.hpp>
#include <RenderAPI/General/GLContext.hpp>
#include "RawModelSettings.hpp"
#include <Core/Misc/Defines/DataStructure.hpp>
#include <RenderEngine/Materials/Material.hpp>

TRE_NS_START

struct MatrialForRawModel
{
	Material material;
	int32 vcount;
	MatrialForRawModel(const Material& material, int32 vcount) : material(material), vcount(vcount)
	{}
};

struct VertexData
{
	VertexData(const vec3& p, const vec3& n, const vec2& t) : pos(p), normal(n), texture(t)
	{}
	vec3 pos, normal;
	vec2 texture;
};

template<bool IsIndexed>
class RawModel
{
public:
	RawModel() = delete; // no default constructor sorry :(

	~RawModel();

	template<ssize_t V, ssize_t T, ssize_t N>
	RawModel(float(&vert)[V], float(&tex)[T], float(&normal)[N], const Vector<MatrialForRawModel>& mat_vec = {});

	template<ssize_t V, ssize_t I, ssize_t T, ssize_t N>
	RawModel(float(&vert)[V], uint32(&indices)[I], float(&tex)[T], float(&normal)[N], const Vector<MatrialForRawModel>& mat_vec = {});

	RawModel(const RawModelSettings& settings);

	template<ssize_t I>
	RawModel(const RawModelSettings&, uint32(&indices)[I]);

	RawModel(const Vector<vec3>& vertices, const Vector<uint32>& indices, const Vector<vec2>* textures = NULL, const Vector<vec3>* normals = NULL, const Vector<MatrialForRawModel>& mat_vec = NULL);
	RawModel(const Vector<vec3>& vertices, const Vector<vec2>* textures = NULL, const Vector<vec3>* normals = NULL, const Vector<MatrialForRawModel>& mat_vec = {});

	RawModel(const Vector<VertexData>& ver_data, const Vector<uint32>& indices, const Vector<MatrialForRawModel>& mat_vec);
	RawModel(const Vector<VertexData>& ver_data, const Vector<MatrialForRawModel>& mat_vec);

	void Render(const ShaderProgram& shader) const;

	FORCEINLINE void Use(const ShaderProgram& shader) const;
	FORCEINLINE const VAO& GetVAO() const { return m_ModelVAO; }

	explicit FORCEINLINE RawModel(const RawModel& other);
	FORCEINLINE RawModel& operator=(const RawModel& other);

	FORCEINLINE const ssize_t GetVertexCount() const;
private:
	// Utility Functions:
	template<ssize_t V, ssize_t T, ssize_t N>
	void LoadFromArray(float(&vert)[V], float(&tex)[T], float(&normal)[N]);
	void LoadFromSettings(const RawModelSettings& settings);
	void LoadFromVector(const Vector<vec3>& vertices, const Vector<vec2>* textures, const Vector<vec3>* normals);
	void LoadFromVertexData(const Vector<VertexData>& ver_data);

	VAO m_ModelVAO;
	VBO vertexVBO;
	VBO textureVBO;
	VBO normalVBO;
	VBO indexVBO;
	ssize_t m_VertexCount;
	const bool m_IsIndexed = IsIndexed;
	Vector<MatrialForRawModel> m_Materials;

	AUTO_CLEAN_WITH_CONTROL(RawModel);
};

template<>
template<ssize_t V, ssize_t T, ssize_t N>
RawModel<false>::RawModel(float(&vert)[V], float(&tex)[T], float(&normal)[N], const Vector<MatrialForRawModel>& mat_vec)
{
	LoadFromArray(vert, tex, normal);
	m_VertexCount = V / 3LLU; // Get the vertex Count!
	m_ModelVAO.Unuse();
	m_Materials = mat_vec;
	if (m_Materials.empty()) {
		Material default_material("Unknown");
		m_Materials.emplace_back(default_material, m_VertexCount); // default material
	}
}

template<>
template<ssize_t V, ssize_t I, ssize_t T, ssize_t N>
RawModel<true>::RawModel(float(&vert)[V], uint32(&indices)[I], float(&tex)[T], float(&normal)[N], const Vector<MatrialForRawModel>& mat_vec)
{
	LoadFromArray(vert, tex, normal);
	m_VertexCount = I; // Get the vertex Count!
	//Set up indices
	indexVBO.Generate(BufferTarget::ELEMENT_ARRAY_BUFFER);
	indexVBO.FillData(indices);
	m_ModelVAO.Unuse();
	indexVBO.Unuse();
	m_Materials = mat_vec;
	if (m_Materials.empty()) {
		Material default_material("Unknown");
		m_Materials.emplace_back(default_material, m_VertexCount); // default material
	}
}


template<>
template<ssize_t I>
RawModel<true>::RawModel(const RawModelSettings& settings, uint32(&indices)[I])
{
	ASSERTF(!(settings.vertexSize == 0 || settings.vertices == NULL), "Attempt to create a RawModel with empty vertecies!");
	LoadFromSettings(settings);
	m_VertexCount = I; // Get the vertex Count!
	//Set up indices
	indexVBO.Generate(BufferTarget::ELEMENT_ARRAY_BUFFER);
	indexVBO.FillData(indices);
	m_ModelVAO.Unuse();
	indexVBO.Unuse();
}

template<bool B>
FORCEINLINE void RawModel<B>::Use(const ShaderProgram& shader) const
{
	shader.Use();
	m_ModelVAO.Use();
}

template<bool B>
void RawModel<B>::LoadFromSettings(const RawModelSettings& settings)
{
	ASSERTF(!(settings.vertexSize == 0 || settings.vertices == NULL), "Attempt to create a RawModel with empty vertecies!");

	m_ModelVAO.Use();

	//Fill vertex:
	vertexVBO.Generate(BufferTarget::ARRAY_BUFFER);
	vertexVBO.FillData(settings.vertices, settings.vertexSize * sizeof(float));
	m_ModelVAO.BindAttribute<DataType::FLOAT>(0, vertexVBO, 3, 0, 0);

	if (settings.normalSize != 0 && settings.normals != NULL) { // Fill normals if availble
		//Fill normals:
		normalVBO.Generate(BufferTarget::ARRAY_BUFFER);
		normalVBO.FillData(settings.normals, settings.normalSize * sizeof(float));
		m_ModelVAO.BindAttribute<DataType::FLOAT>(1, normalVBO, 3, 0, 0);
		normalVBO.Unuse();
	}

	if (settings.textureSize != 0 && settings.textures != NULL) { //Fill Texture if availble
		textureVBO.Generate(BufferTarget::ARRAY_BUFFER);
		textureVBO.FillData(settings.textures, settings.textureSize * sizeof(float));
		m_ModelVAO.BindAttribute<DataType::FLOAT>(2, textureVBO, 2, 0, 0);
	}
}

template<>
FORCEINLINE void RawModel<true>::OnSetClean(bool b)
{
	m_ModelVAO.SetAutoClean(b);
	vertexVBO.SetAutoClean(b);
	textureVBO.SetAutoClean(b);
	normalVBO.SetAutoClean(b);
	indexVBO.SetAutoClean(b);
}

template<>
FORCEINLINE void RawModel<false>::OnSetClean(bool b)
{
	m_ModelVAO.SetAutoClean(b);
	vertexVBO.SetAutoClean(b);
	textureVBO.SetAutoClean(b);
	normalVBO.SetAutoClean(b);
}

template<bool B>
FORCEINLINE RawModel<B>::RawModel(const RawModel<B>& other) :
	m_ModelVAO(other.m_ModelVAO), vertexVBO(other.vertexVBO), textureVBO(other.textureVBO),
	normalVBO(other.normalVBO), indexVBO(other.indexVBO), m_VertexCount(other.m_VertexCount), m_AutoClean(true)
{
	const_cast<RawModel&>(other).SetAutoClean(false);
}

template<bool B>
FORCEINLINE RawModel<B>& RawModel<B>::operator=(const RawModel<B>& other) {
	const_cast<RawModel&>(other).SetAutoClean(false);
	this->SetAutoClean(true);
	m_ModelVAO = other.m_ModelVAO;
	vertexVBO = other.vertexVBO;
	textureVBO = other.textureVBO;
	normalVBO = other.normalVBO;
	indexVBO = other.indexVBO;
	m_VertexCount = other.m_VertexCount;
	return *this;
}

template<bool B>
FORCEINLINE const ssize_t RawModel<B>::GetVertexCount() const
{
	return m_VertexCount;
}

template<bool B>
RawModel<B>::~RawModel()
{
	if (m_AutoClean) {
		m_ModelVAO.Clean();
		vertexVBO.Clean();
		textureVBO.Clean();
		normalVBO.Clean();
		indexVBO.Clean();
	}
}

template<bool B>
void RawModel<B>::Clean()
{
	m_ModelVAO.Clean();
	vertexVBO.Clean();
	textureVBO.Clean();
	normalVBO.Clean();
	indexVBO.Clean();
}

TRE_NS_END