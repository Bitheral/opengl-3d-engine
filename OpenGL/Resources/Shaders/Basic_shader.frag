#version 460 core

in vec2 TexCoord;
in vec3 Normal; 
in vec3 Vertex;

struct LightSource {
	int type;
    vec3 position;
	vec3 intensity;
	vec3 direction;

	vec3 diffuse;
	vec3 attenuation;
	float cutOff;
	float outerCutOff;
};

//Texture sampler
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform samplerCube skybox;

//Camera location
uniform vec3 eyePos;

//Light information
//uniform vec4		lightPosition;
uniform vec4		lightAmbient;

//Material iformation
uniform vec4		matAmbient;
uniform vec4		matDiffuse;
uniform vec4        matSpecularColour;
uniform float       matSpecularExponent;
uniform float       smoothness;

uniform int lightCount;
uniform LightSource Light[16];

out vec4 FragColour;

vec4 calculateLight(LightSource light) {

	// Ambience	
	vec4 texColour = texture(texture_diffuse1, TexCoord);

	// Diffuse
	vec3 normalizedNormal = normalize(Normal);	
	vec3 viewDirection = normalize(eyePos - Vertex);

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	//Attenuation/drop-off	
	float attD = length(light.position - Vertex);
	float att = 1.0 / (light.attenuation.x + light.attenuation.y * attD + light.attenuation.z * (attD * attD));

	if(light.type == 0) {
		//Ambient light value
		ambient = lightAmbient * matAmbient * texColour * att;

		//Diffuse light value

		vec3 L = normalize(light.position - Vertex);
		float lambertTerm = clamp(dot(normalizedNormal, L), 0.0, 1.0);
		diffuse = vec4(light.diffuse, 1.0) * matDiffuse * texColour * lambertTerm * att;

		//Specular light value

		vec3 R = reflect(-L, normalizedNormal); // reflected light vector about normal N
		float specularIntensity = pow(max(dot(viewDirection, R), 0.0), matSpecularExponent);
		specular = matSpecularColour * texColour * specularIntensity * att * vec4(light.intensity, 1.0);

		
	} else if(light.type == 1) {
		ambient = lightAmbient * matAmbient * texColour;

		vec3 lightDir = normalize(-light.direction);
		float diff = max(dot(normalizedNormal, lightDir), 0.0);
		diffuse = vec4(light.diffuse, 1.0) * diff * texColour;

		vec3 reflectDir = reflect(-lightDir, normalizedNormal);
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 1);
		specular = matSpecularColour * texColour * vec4(light.intensity, 1.0) * spec;

	} else if(light.type == 2) {
		ambient = ambient = lightAmbient * matAmbient * texColour;

		vec3 lightDir = normalize(light.position - Vertex);
		float diff = max(dot(normalizedNormal, lightDir), 0.0);
		diffuse = vec4(light.diffuse, 1.0) * diff * matDiffuse * texColour;

		vec3 reflectDir = reflect(-viewDirection, normalizedNormal);  
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 1.0);
		specular = matSpecularColour * texColour * vec4(light.intensity, 1.0) * spec;

		// spotlight (soft edges)
		float theta = dot(lightDir, normalize(-light.direction)); 
		float epsilon = (light.cutOff - light.outerCutOff);
		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
		diffuse  *= intensity;
		specular *= intensity;
    
		// attenuation
		ambient  *= att; 
		diffuse  *= att;
		specular *= att; 
	}

	return ambient + diffuse + specular;
}

void main()
{
	vec4 finalColour;
	for(int i = 0; i < lightCount; i++) {
		finalColour += calculateLight(Light[i]);
	}

	vec3 I = normalize(Vertex - eyePos);
    vec3 skyboxR = reflect(I, normalize(Normal));

    FragColour = finalColour;
}