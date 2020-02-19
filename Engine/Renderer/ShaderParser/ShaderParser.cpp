#include "ShaderParser.hpp"
#include <Core/FileSystem/File/File.hpp>

TRE_NS_START

ShaderParser::ShaderParser(const String& filename)
{
	File file(filename, File::OPEN_READ);
	m_ShaderCode = file.ReadAll();
	printf("%s\n", m_ShaderCode.Buffer());
}

TRE_NS_END