// Code provided from GitHub repo JoeyDeVries/LearnOpenGL
// https://github.com/JoeyDeVries/LearnOpenGL/blob/0f3d3160bb25532c0d2fba5665414b3fb0aa593f/src/4.advanced_opengl/6.1.cubemaps_skybox/6.1.cubemaps.fs

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}