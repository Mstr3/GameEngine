#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Binormal;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_ModelMatrix;

out VS_OUT
{
	vec3 FragPos;
    vec3 Normal;
	vec2 TexCoords;
} vs_Output;

void main()
{
	vs_Output.FragPos = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
    vs_Output.Normal = a_Normal;
	vs_Output.TexCoords = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);

	gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
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