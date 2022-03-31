#version 460 core

in vec2 TexCoord;
in vec3 Normal; 
in vec3 Vertex;

struct LightSource {
	int enabled;

	int type;
    vec3 position;
	float intensity;
	vec3 direction;
	vec3 colour;

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
	
	if(light.enabled == 0) {
		return vec4(0,0,0,1.0);
	}

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
		// Render PointLight

		//Ambient light value
		vec3 pointAmbient = (light.colour * light.intensity) * texColour.rgb;

		//ambient = light.ambient * matAmbient * texColour * att * vec4(light.colour * light.intensity, 1.0);

		//Diffuse light value

		vec3 L = normalize(light.position - Vertex);
		float lambertTerm = clamp(dot(normalizedNormal, L), 0.0, 1.0);
		vec3 pointDiffuse = (light.colour * light.intensity) * texColour.rgb * lambertTerm;
		//diffuse = matDiffuse * texColour * lambertTerm * att * vec4(light.colour * light.intensity, 1.0);

		//Specular light value

		vec3 R = reflect(-L, normalizedNormal); // reflected light vector about normal N
		float specularIntensity = pow(max(dot(viewDirection, R), 0.0), matSpecularExponent);
		vec3 pointSpecular = (light.colour * light.intensity) * texColour.rgb * specularIntensity;

		pointAmbient *= att;
		pointDiffuse *= att;
		pointSpecular *= att;

		ambient = vec4(pointAmbient, 1.0);
		diffuse = vec4(pointDiffuse, 1.0);
		specular = vec4(pointSpecular, 1.0);

		
	} else if(light.type == 1) {
		// Render DirectionalLight

		vec3 directionalAmbient = (light.colour * light.intensity) * texColour.rgb;
		//ambient = light.ambient * matAmbient * texColour * vec4(light.colour * light.intensity, 1.0);

		vec3 lightDir = normalize(-light.direction);
		float diff = max(dot(normalizedNormal, lightDir), 0.0);
		vec3 directionalDiffuse = (light.colour * light.intensity) * texColour.rgb * diff;
		//diffuse = diff * texColour * vec4(light.colour * light.intensity, 1.0);

		vec3 reflectDir = reflect(-lightDir, normalizedNormal);
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 1);
		vec3 directionalSpecular = (light.colour * light.intensity) * texColour.rgb * spec;
		//specular = matSpecularColour * texColour  * spec * vec4(light.colour * light.intensity, 1.0);

		ambient = vec4(directionalAmbient, 1.0);
		diffuse = vec4(directionalDiffuse, 1.0);
		specular = vec4(directionalSpecular, 1.0);

	} else if(light.type == 2) {
		// Render Spotlight

		// Calculate ambient lighting
		vec3 spotAmbient = (light.colour * light.intensity) * texColour.rgb;
    
		// Calculate diffuse lighting
		vec3 lightDir = normalize(light.position.xyz - Vertex);
		float diff = max(dot(normalizedNormal, lightDir), 0.0);
		vec3 spotDiffuse = (light.colour * light.intensity) * texColour.rgb * diff;
    
		// Calculate specular lighting
		vec3 reflectDir = reflect(-lightDir, normalizedNormal);  
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), light.intensity);
		vec3 spotSpecular = (light.colour * light.intensity) * spec * matSpecularColour.rgb;
    
		// Calulate spotlight diffusion
		float theta = dot(lightDir, normalize(-light.direction)); 
		float epsilon = (light.cutOff - light.outerCutOff);
		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
		spotDiffuse  *= intensity;
		spotSpecular *= intensity;
    
		// Multiply by the attenuation of the light
		spotAmbient  *= att; 
		spotDiffuse  *= att;
		spotSpecular *= att;   

		ambient = vec4(spotAmbient, 1.0);
		diffuse = vec4(spotDiffuse, 1.0);
		specular = vec4(spotSpecular, 1.0);
	}

	return ambient + diffuse + specular;
}

void main()
{
	vec4 finalColour;
	for(int i = 0; i < lightCount; i++) {
		finalColour += calculateLight(Light[i]);
	}

	// Stuff for skybox
	vec3 I = normalize(Vertex - eyePos);
    vec3 skyboxR = reflect(I, normalize(Normal));

    FragColour = finalColour;
}