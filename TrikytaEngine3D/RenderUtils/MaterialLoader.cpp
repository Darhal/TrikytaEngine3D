#include <fstream>
#include <iostream>
#include "MaterialLoader.hpp"
#include <Core/Misc/Defines/Debug.hpp>
#include <RenderEngine/Materials/Material.hpp>

TRE_NS_START

MaterialLoader::MaterialLoader()
{
}

void MaterialLoader::LoadFileMTL(const char* path)
{
	FILE* file = fopen(path, "r");
	ASSERTF(file != NULL, "MaterialLoader couldn't open the file %s.", path);
	char buffer[255];
	int64 line_len = 0;
	printf("Parsing the materials: %s\n", path);
	uint64 current_line = 1;
	String current_name;
	while ((line_len = ReadLine(file, buffer, 255)) != EOF) {
		if (IsEqual(buffer, "newmtl")) {
			char m_Name[25];
			sscanf(buffer, "newmtl %s", m_Name);
			//printf("Adding new material: %s\n", m_Name);
			current_name = m_Name;
			m_NameToMaterial[m_Name] = Material(m_Name);
		}else if (IsEqual(buffer, "Ka")) {
			vec3 ambient;
			int32 res = sscanf(buffer, "Ka %f %f %f", &ambient.x, &ambient.y, &ambient.z);
			ASSERTF(res == 3, "Attempt to parse a corrupted MTL file 'usemtl', failed while reading the ambient component (Line : %llu).", current_line);
			m_NameToMaterial[current_name].m_Ambient = ambient;
			//printf("Ka %f %f %f\n", ambient.x, ambient.y, ambient.z);
		}else if (IsEqual(buffer, "Kd")) {
			vec3 diffuse;
			int32 res = sscanf(buffer, "Kd %f %f %f", &diffuse.x, &diffuse.y, &diffuse.z);
			ASSERTF(res == 3, "Attempt to parse a corrupted MTL file 'usemtl', failed while reading the diffuse component (Line : %llu).", current_line);
			m_NameToMaterial[current_name].m_Diffuse = diffuse;
			//printf("Kd %f %f %f\n", diffuse.x, diffuse.y, diffuse.z);
		}else if (IsEqual(buffer, "Ks")) {
			vec3 specular;
			int32 res = sscanf(buffer, "Ks %f %f %f", &specular.x, &specular.y, &specular.z);
			ASSERTF(res == 3, "Attempt to parse a corrupted MTL file 'usemtl', failed while reading the specular component (Line : %llu).", current_line);
			m_NameToMaterial[current_name].m_Specular = specular;
			//printf("Ks %f %f %f\n", specular.x, specular.y, specular.z);
		}else if (IsEqual(buffer, "Ns")) {
			float shininess;
			int32 res = sscanf(buffer, "Ns %f", &shininess);
			ASSERTF(res == 1, "Attempt to parse a corrupted MTL file 'usemtl', failed while reading the shininess component (Line : %llu).", current_line);
			m_NameToMaterial[current_name].m_Shininess = shininess;
			//printf("Ks %f %f %f\n", specular.x, specular.y, specular.z);
		}
		current_line++;
	}
	fclose(file);
}

Material& MaterialLoader::GetMaterialFromName(const char* name)
{
	return m_NameToMaterial[name];
}

TRE_NS_END