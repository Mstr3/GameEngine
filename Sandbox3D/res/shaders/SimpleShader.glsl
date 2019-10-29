#type vertex
#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

out vec2 TexCoord;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewProjectionMatrix;

void main()
{
    gl_Position = u_ViewProjectionMatrix  * u_ModelMatrix * vec4(a_Pos, 1.0);
	TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D Texture1;
uniform sampler2D Texture2;

void main()
{
    FragColor = mix(texture(Texture1, TexCoord), texture(Texture2, TexCoord), 0.2);
}