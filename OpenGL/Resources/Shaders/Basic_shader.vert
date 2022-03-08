#version 330 core

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 Normal; 
out vec3 Vertex; 

void main()
{
	TexCoord = texCoord;
	
	Normal = mat3(transpose(inverse(model))) * normal;  // normal vector in eye coordinates
	
	Vertex = vec3(model * vec4(vertexPos, 1.0)); // vertex in eye coordinates (map to vec3 since gl_Vertex is a vec4)

	gl_Position = projection * view * model * vec4(vertexPos, 1.0f);
}