#pragma once

#include <Core/Misc/Defines/Common.hpp>
#include <RenderAPI/VertexArray/VAO.hpp>
#include <Renderer/Common/Common.hpp>
#include <Renderer/MeshLoader/ModelData/ModelData.hpp>
#include <Renderer/Backend/Commands/Commands.hpp>
#include <Renderer/Materials/AbstractMaterial/AbstractMaterial.hpp>
#include <Renderer/MeshLoader/VertexData/VertexData.hpp>
#include <Renderer/Mesh/StaticMesh/StaticMesh.hpp>
#include <Renderer/MeshLoader/ModelData/ModelMaterialData.hpp>

TRE_NS_START

class Model
{
public:
	enum VertexAttributes
	{
		POSITION, 
		NORMAL, 
		TEXTURE_COORDINATES
	};

	Model(const ModelData& data);

	Model(Vector<VertexData>& ver_data, const Vector<ModelMaterialData>& mat_vec, Vector<uint32>* indices = NULL);

	StaticMesh LoadMesh(ShaderID shader_id = 0);
private:
	void LoadFromSettings(const ModelData& data);
	void LoadFromVertexData(Vector<VertexData>& ver_data);
	void RunCommand();
	void CreateIndexBuffer(Commands::CreateIndexBuffer& index_cmd);

	Commands::CreateVAO m_CreateVaoCmd;
	Vector<ModelMaterialData> m_Materials;
	uint32 m_VertexCount;
	VaoID m_VaoID;
	bool m_HaveIndexBuffer;
};

TRE_NS_END