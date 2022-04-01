#version 330 core

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
uniform sampler2D texture_normal1;
uniform samplerCube skybox;

//Camera location
uniform vec3 eyePos;

uniform float       matSpecularExponent;

uniform float		textureScale;

uniform int lightCount;
uniform LightSource Light[64];

out vec4 FragColour;

vec4 calculateLight(LightSource light) {
	
	// If the light provided is either:
	//	disabled,
	//	no intensity
	//	colour is black
	// then it will return a blank vec4.
	//
	// This is to prevent unneccessary calculations
	// when the value will return 0.
	if(light.enabled == 0 || light.intensity <= 0 || light.colour == 0) {
		return vec4(0,0,0, 1.0);
	}

	// Scale the TexCoords recieved from the Vertex shader, and scale them
	// by textureScale
	vec2 texCoords = (TexCoord - 0.5) * textureScale + (0.5 * textureScale);

	// Textures	
	vec4 albedoMap = texture(texture_diffuse1, texCoords);
	vec4 specularMap = texture(texture_specular1, texCoords);
	vec3 normalMap = texture(texture_normal1, texCoords).rgb;

	// Normalize normal map
	vec3 normalTexture = normalize(normalMap * 2.0 - 1.0) * 100;

	// Calculate normals
	vec3 normalizedNormal;

	// Checks if there is a normal map provided,
	// if so (normalMap != vec3(0), it will normalize the values from the texture to use for the light
	// if not (normalMap == vec3(0), it will normalize the Normals passed from the vertex shader.
	if(normalMap == vec3(0)) normalizedNormal = normalize(Normal);
	else normalizedNormal = normalize(normalTexture);

	// Get the view direction of the camera;
	vec3 viewDirection = normalize(eyePos - Vertex);

	// Initialize final variables
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	// Calculate attenuation	
	float attD = length(light.position - Vertex);
	float att = 1.0 / (light.attenuation.x + light.attenuation.y * attD + light.attenuation.z * (attD * attD));

	// Directional Light calculation provided by GitHub repository JoeyDeVries/LearnOpenGL
	// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/2.lighting/5.1.light_casters_directional/5.1.light_casters.fs

	// Spotlight calculation provided by GitHub repository JoeyDeVries/LearnOpenGL
	// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/2.lighting/5.4.light_casters_spot_soft/5.4.light_casters.fs

	if(light.type == 0) {
		// Render PointLight

		//Ambient light value
		vec3 pointAmbient = (light.colour * light.intensity) * albedoMap.rgb;

		// Diffuse light value
		vec3 L = normalize(light.position - Vertex);
		float lambertTerm = clamp(dot(normalizedNormal, L), 0.0, 1.0);
		vec3 pointDiffuse = (light.colour * light.intensity) * albedoMap.rgb * lambertTerm;

		//Specular light value
		vec3 R = reflect(-L, normalizedNormal); // reflected light vector about normal N
		float specularIntensity = pow(max(dot(viewDirection, R), 0.0), matSpecularExponent);
		vec3 pointSpecular = (light.colour * light.intensity) * specularMap.rgb * specularIntensity;

		// Include attenuation into the calculated result
		pointAmbient *= att;
		pointDiffuse *= att;
		pointSpecular *= att;

		// Set final variables to calculated result
		ambient = vec4(pointAmbient, 1.0);
		diffuse = vec4(pointDiffuse, 1.0);
		specular = vec4(pointSpecular, 1.0);

		
	} else if(light.type == 1) {
		// Render DirectionalLight

		vec3 directionalAmbient = (light.colour * light.intensity) * albedoMap.rgb;
		//ambient = light.ambient * matAmbient * albedoMap * vec4(light.colour * light.intensity, 1.0);

		vec3 lightDir = normalize(-light.direction);
		float diff = max(dot(normalizedNormal, lightDir), 0.0);
		vec3 directionalDiffuse = (light.colour * light.intensity) * albedoMap.rgb * diff;
		//diffuse = diff * albedoMap * vec4(light.colour * light.intensity, 1.0);

		vec3 reflectDir = reflect(-lightDir, normalizedNormal);
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 1);
		vec3 directionalSpecular = (light.colour * light.intensity) * specularMap.rgb * spec;
		//specular = matSpecularColour * albedoMap  * spec * vec4(light.colour * light.intensity, 1.0);

		ambient = vec4(directionalAmbient, 1.0);
		diffuse = vec4(directionalDiffuse, 1.0);
		specular = vec4(directionalSpecular, 1.0);

	} else if(light.type == 2) {
		// Render Spotlight

		// Calculate ambient lighting
		vec3 spotAmbient = (light.colour * light.intensity) * albedoMap.rgb;
	
		// Calculate diffuse lighting
		vec3 lightDir = normalize(light.position.xyz - Vertex);
		float diff = max(dot(normalizedNormal, lightDir), 0.0);
		vec3 spotDiffuse = (light.colour * light.intensity) * albedoMap.rgb * diff;
	
		// Calculate specular lighting
		vec3 reflectDir = reflect(-lightDir, normalizedNormal);  
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), light.intensity);
		vec3 spotSpecular = (light.colour * light.intensity) * spec * specularMap.rgb;
	
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

		// Set final output to calculated result
		ambient = vec4(spotAmbient, 1.0);
		diffuse = vec4(spotDiffuse, 1.0);
		specular = vec4(spotSpecular, 1.0);
	}

	// Return final output
	return ambient + diffuse + specular;
}

void main()
{
	vec4 finalColour;

	// Add up all the lights in Light array and return finalColour to FragColour
	for(int i = 0; i < lightCount; i++) {
		finalColour += calculateLight(Light[i]);
	}

	FragColour = finalColour;
}