#version 460 core

in vec2 TexCoord;
in vec3 Normal; 
in vec3 Vertex;

struct LightSource {
	int enabled;

	int type;
    vec3 position;
	vec3 intensity;
	vec3 direction;

	vec4 ambient;
	vec3 diffuse;
	vec3 specular;

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

	light.specular = vec3(1.0,1.0,1.0);

	//Attenuation/drop-off	
	float attD = length(light.position - Vertex);
	float att = 1.0 / (light.attenuation.x + light.attenuation.y * attD + light.attenuation.z * (attD * attD));

	if(light.type == 0) {
		//Ambient light value
		ambient = light.ambient * matAmbient * texColour * att;

		//Diffuse light value

		vec3 L = normalize(light.position - Vertex);
		float lambertTerm = clamp(dot(normalizedNormal, L), 0.0, 1.0);
		diffuse = vec4(light.diffuse, 1.0) * matDiffuse * texColour * lambertTerm * att;

		//Specular light value

		vec3 R = reflect(-L, normalizedNormal); // reflected light vector about normal N
		float specularIntensity = pow(max(dot(viewDirection, R), 0.0), matSpecularExponent);
		specular = matSpecularColour * texColour * specularIntensity * att * vec4(light.intensity, 1.0);

		
	} else if(light.type == 1) {
		ambient = light.ambient * matAmbient * texColour;

		vec3 lightDir = normalize(-light.direction);
		float diff = max(dot(normalizedNormal, lightDir), 0.0);
		diffuse = vec4(light.diffuse, 1.0) * diff * texColour;

		vec3 reflectDir = reflect(-lightDir, normalizedNormal);
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 1);
		specular = matSpecularColour * texColour * vec4(light.intensity, 1.0) * spec;

	} else if(light.type == 2) {
		// ambient
		vec3 spotAmbient = light.ambient.xyz * texColour.rgb;
    
		// diffuse 
		vec3 lightDir = normalize(light.position.xyz - Vertex);
		float diff = max(dot(normalizedNormal, lightDir), 0.0);
		vec3 spotDiffuse = light.diffuse * diff * texColour.rgb;
    
		// specular
		vec3 reflectDir = reflect(-lightDir, normalizedNormal);  
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 1.0);
		vec3 spotSpecular = vec3(1.0,1.0,1.0) * spec * light.intensity * matSpecularColour.rgb;
    
		// spotlight (soft edges)
		float theta = dot(lightDir, normalize(-light.direction)); 
		float epsilon = (light.cutOff - light.outerCutOff);
		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
		spotDiffuse  *= intensity;
		spotSpecular *= intensity;
    
		// attenuation
		spotAmbient  *= att; 
		spotDiffuse   *= att;
		spotSpecular *= att;   
        
		ambient = vec4(spotAmbient, 1.0);
		diffuse = vec4(spotDiffuse, 1.0);
		specular = vec4(spotSpecular, 1.0);
	}

	if(light.enabled == 1) {
		return ambient + diffuse + specular;
	}
	else { 
		return vec4(0,0,0,1.0);
	}
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