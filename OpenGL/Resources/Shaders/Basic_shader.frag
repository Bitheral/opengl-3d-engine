#version 330 core

in vec2 TexCoord;
in vec3 Normal; 
in vec3 Vertex; 

//Texture sampler
uniform sampler2D texture_diffuse1;

//Camera location
uniform vec3 eyePos;

//Light information
//uniform vec4		lightPosition;
uniform vec4		lightAmbient;
uniform vec4		lightDiffuse;
uniform vec3		lightAttenuation; // x=constant, y=linear, z=quadratic (x<0 means light is not active)

//Material iformation
uniform vec4		matAmbient;
uniform vec4		matDiffuse;
uniform vec4        matSpecularColour;
uniform float       matSpecularExponent;

// Skybox information
uniform samplerCube skybox;

uniform vec4 lightOne;
uniform vec4 lightOneColour;

uniform vec4 lightTwo;
uniform vec4 lightTwoColour;

uniform vec4 lightThree;
uniform vec4 lightThreeColour;

uniform vec4 lightFour;
uniform vec4 lightFourColour;

out vec4 FragColour;

vec4 calculateLight(vec4 lightPosition, vec4 lightColour) {
	vec3 red = vec3(255,0,0);

	//Attenuation/drop-off	
	float d = length(lightPosition.xyz - Vertex);
	float att = 1.0 / (lightAttenuation.x + lightAttenuation.y * d + lightAttenuation.z * (d * d));

	//Ambient light value
	vec4 texColour = texture(texture_diffuse1, TexCoord);
	vec4 ambient = lightAmbient * matAmbient * texColour * att;

	//Diffuse light value
	vec3 N = normalize(Normal);	
	vec3 L = normalize(lightPosition.xyz - Vertex);
	float lambertTerm = clamp(dot(N, L), 0.0, 1.0);
	vec4 diffuse = lightDiffuse * matDiffuse * lambertTerm * texColour * att;

	//Specular light value
	vec3 E = normalize(eyePos - Vertex);
	vec3 R = reflect(-L, N); // reflected light vector about normal N
	float specularIntensity = pow(max(dot(E, R), 0.0), matSpecularExponent);
	vec4 specular = matSpecularColour * specularIntensity * texColour * att * lightColour;

	return ambient + diffuse + specular;
}

void main()
{

	//Final colour is the combinatin of all components
	vec4 light1 = calculateLight(lightOne, lightOneColour);
	vec4 light2 = calculateLight(lightTwo, lightTwoColour);
	vec4 light3 = calculateLight(lightThree, lightThreeColour);
	vec4 light4 = calculateLight(lightFour, lightFourColour);
	FragColour = light1 + light2 + light3 + light4;
}