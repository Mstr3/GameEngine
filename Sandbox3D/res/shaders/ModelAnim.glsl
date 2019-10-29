#type vertex
#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBinormal;

layout (location = 5) in ivec4 a_BoneIndices;
layout (location = 6) in vec4 a_BoneWeights;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_ModelMatrix;

const int MAX_BONES = 100;
uniform mat4 u_BoneTransforms[100];

out VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

void main()
{
	mat4 boneTransform = u_BoneTransforms[a_BoneIndices[0]] * a_BoneWeights[0];
    boneTransform += u_BoneTransforms[a_BoneIndices[1]] * a_BoneWeights[1];
    boneTransform += u_BoneTransforms[a_BoneIndices[2]] * a_BoneWeights[2];
    boneTransform += u_BoneTransforms[a_BoneIndices[3]] * a_BoneWeights[3];

	vec4 localPosition = boneTransform * vec4(aPos, 1.0);

	vs_out.FragPos = vec3(u_ModelMatrix * boneTransform * vec4(aPos, 1.0));
	vs_out.Normal = mat3(transpose(inverse(u_ModelMatrix * boneTransform))) * aNormal;  
    vs_out.TexCoords = vec2(aTexCoords.x, 1.0 - aTexCoords.y);  // OpenGL texture reading flip 1.0 - aTexCoords.y
    gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * localPosition;
}

#type fragment
#version 430 core
out vec4 FragColor;

struct Light {
    vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform float shininess;

uniform vec3 viewPos;
uniform Light light;

void main()
{    
    // ambient
    vec3 ambient = light.ambient * texture(texture_diffuse1, fs_in.TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, fs_in.TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * texture(texture_specular1, fs_in.TexCoords).rgb;  
    
    // attenuation
    float distance    = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse  *= attenuation;
    specular *= attenuation;   
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
	//FragColor = vec4(texture(texture_diffuse1, fs_in.TexCoords).rgb, 1.0);
}