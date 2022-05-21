// Code provided from GitHub repo JoeyDeVries/LearnOpenGL
// Modified the code to fit the needs of the program
// https://github.com/JoeyDeVries/LearnOpenGL/blob/0f3d3160bb25532c0d2fba5665414b3fb0aa593f/src/4.advanced_opengl/6.1.cubemaps_skybox/6.1.cubemaps.vs

#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}