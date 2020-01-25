#version 330 core

out vec4 FragColor;

const int MAX_LIGHTS = 32;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
	float alpha;
	sampler2D diffuse_tex;
    sampler2D specular_tex;  
};

uniform Material material;

layout (std140) uniform LightUBO
{
								// base alignment	| aligned offset
	mat4 Lights[MAX_LIGHTS];	// 16 * 4 * MAX		|	0
	int NumLights; 				// 4				|	16 * 4 * MAX
}; 

in vec3 FragPos;
in vec3 ViewPos;
in vec3 Normal;

vec3 CalculateLight(mat4 light, vec3 normal, vec3 viewDir, vec3 fragPos, mat3 material_color);
vec3 CalculateDirectionalLight(mat4 light, vec3 normal, vec3 viewDir, mat3 material_color);
vec3 CalculatePointLight(mat4 light, vec3 normal, vec3 viewDir, vec3 fragPos, mat3 material_color);
mat3 GetMaterialColor();

void main()
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(ViewPos - FragPos);
	mat3 material_color = GetMaterialColor(); // 0: ambient, 1: diffuse, 2: specular
	vec3 result = material_color[0];
	
	// Calculate lights : 
	for(int i = 0; i < NumLights; i++){
		result += CalculateLight(Lights[i], norm, viewDir, FragPos, material_color);
	}
	
	FragColor = vec4(result, material.alpha);
} 

mat3 GetMaterialColor()
{
	return mat3(
		material.ambient, 
		material.diffuse, 
		material.specular
	);
}

// Calculate light depending on the light type
vec3 CalculateLight(mat4 light, vec3 normal, vec3 viewDir, vec3 fragPos, mat3 material_color)
{
	int l_type = int(light[0][3]);
	
	if (l_type == 0){ // Directional light
		return CalculateDirectionalLight(light, normal, viewDir, material_color);
	}else if(l_type == 1){ // Point light
		return CalculatePointLight(light, normal, viewDir, fragPos, material_color);
	}else if(l_type == 2){ // Spot light
		// return CalculateSpotLight(light, normal, viewDir, fragPos, material_color);
	}
}

// calculates the color when using a directional light.
vec3 CalculateDirectionalLight(mat4 light, vec3 normal, vec3 viewDir, mat3 material_color)
{
	vec3 l_direction = vec3(light[0][0], light[0][1], light[0][2]); // direction
	vec3 l_color = vec3(light[1][0], light[1][1], light[1][2]);
    vec3 lightDir =  normalize(-l_direction);
	
	// diffuse shading
    float diff = max(dot(lightDir, normal), 0.0); // diffuse shading
	
	// specular shading (blinn)
	vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	
	// Phong:
	// vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);

    // combine results
    vec3 diffuse  = l_color * diff * material_color[1];
    vec3 specular = l_color * spec * material_color[2];
	
    return (diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalculatePointLight(mat4 light, vec3 normal, vec3 viewDir, vec3 fragPos, mat3 material_color)
{
	vec3 l_position = vec3(light[0][0], light[0][1], light[0][2]);
	vec3 l_color = vec3(light[1][0], light[1][1], light[1][2]);
	float l_constant = light[2][0];
	float l_linear = light[2][1];
	float l_quadratic = light[2][2];
	
    vec3 lightDir = normalize(l_position - fragPos); 
    
    float diff = max(dot(normal, lightDir), 0.0); 
	
	// diffuse shading
    vec3 reflectDir = reflect(-lightDir, normal); 
	
	// specular shading
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    float distance = length(l_position - fragPos); // attenuation
    float attenuation = 1.0 / (l_constant + l_linear * distance + l_quadratic * (distance * distance)); 
	
    // combine results
    // 0: ambient, 1: diffuse, 2: specular
    // vec3 ambient = material_color[0];
    vec3 diffuse = l_color * diff * material_color[1];
    vec3 specular = l_color * spec * material_color[2];
	
    return (diffuse + specular) * attenuation;
}

// calculates the color when using a spot light.
vec3 CalculateSpotLight(mat4 light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
	vec3 l_position = vec3(light[0][0], light[0][1], light[0][2]);
	vec3 l_color = vec3(light[1][0], light[1][1], light[1][2]);
	vec3 l_direction = vec3(light[2][0], light[2][1], light[2][2]);
	float l_constant = light[2][3];
	float l_linear = light[3][0];
	float l_quadratic = light[3][1];
	float l_cutoff = light[3][2];
	float l_outerCutoff = light[3][3];
	
    vec3 lightDir = normalize(l_position - fragPos);
	
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
	
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
    // attenuation
    float distance = length(l_position - fragPos);
    float attenuation = 1.0 / (l_constant + l_linear * distance + l_quadratic * (distance * distance));    
	
    // spotlight intensity
    float theta = dot(lightDir, normalize(-l_direction)); 
    float epsilon = l_cutoff - l_outerCutoff;
    float intensity = clamp((theta - l_outerCutoff) / epsilon, 0.0, 1.0);
	
    // combine results
	mat3 material_color = GetMaterialColor(); // 0: ambient, 1: diffuse, 2: specular
    vec3 ambient =  material_color[0];
    vec3 diffuse = l_color * diff * material_color[1];
    vec3 specular = l_color * spec * material_color[2];
	
    return (ambient + diffuse + specular) * attenuation * intensity;
}