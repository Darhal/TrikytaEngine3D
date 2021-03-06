#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 MVP;
uniform mat4 model;

out vec3 FragPos;
out vec3 viewPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

layout (std140) uniform VertexUBO
{
								// base alignment	| aligned offset
	mat4 Projection;			// 16 * 4		 	|	0
	mat4 View;					// 16 * 4			|	64
	mat4 ProjectionView;		// 16 * 4			|	128
	mat4 lightSpaceMatrix;		// 16 * 4			|	192
	vec3 ViewPosition;			// 16				|	192+64
}; 


void main()
{
	TexCoords = aTexCoord;
	viewPos = ViewPosition;
	FragPos = vec3(model * vec4(aPos, 1.0));
	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    Normal = mat3(transpose(inverse(model))) * aNormals;  
	
	gl_Position =  ProjectionView * model * vec4(aPos, 1.0);
}
