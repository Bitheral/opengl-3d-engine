#include "Includes.h"

#include <stdlib.h>
#include <utility>
#include <cmath>
#include <ctime>

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void drawSkybox(GLuint vbo, GLuint texture, GLuint shader, glm::mat4 view, glm::mat4 projection);

glm::vec3 getMatrixPosition(glm::mat4 matrix);

enum class LightType {
	BULB = 0,
	DIRECTIONAL = 1,
	SPOT = 2
};

// Classes
class Light {

private:

	LightType lightType;

	glm::vec3 position;
	glm::vec3 colour;
	glm::vec3 attenuation;
	glm::vec3 diffuse;
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

	glm::vec4 ambient = glm::vec4(0.1, 0.1, 0.1, 1.0);

	GLfloat cutOff;
	GLfloat outerCutOff;
	GLfloat intensity;

public:

	int enabled;

	Light(LightType typeIn, glm::vec3 positionIn, glm::vec3 colourIn, GLfloat intensityIn, glm::vec3 directionIn = glm::vec3(0, -1, 0)) {
		this->enabled = 1;
		this->lightType = typeIn;
		this->setPosition(positionIn);
		this->setDirection(directionIn);
		this->setColour(colourIn);
		this->setIntensity(intensityIn);
		this->setCutOff(12.5, 17.5);
		this->setAttenuation(0.09, 0.032f);
	}

	void processUniforms(GLuint shader, string lightIndex) {

		// Set light uniforms in shader with light index
		glUniform1i(glGetUniformLocation(shader, string(lightIndex + ".enabled").c_str()), this->enabled);
		glUniform1i(glGetUniformLocation(shader, string(lightIndex + ".type").c_str()), static_cast<GLuint>(this->lightType));
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".position").c_str()), 1, (GLfloat*)&this->position);
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".direction").c_str()), 1, (GLfloat*)&this->direction);
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".colour").c_str()), 1, (GLfloat*)&this->colour);
		glUniform4fv(glGetUniformLocation(shader, string(lightIndex + ".ambient").c_str()), 1, (GLfloat*)&this->ambient);
		glUniform1f(glGetUniformLocation(shader, string(lightIndex + ".intensity").c_str()), this->intensity);
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".diffuse").c_str()), 1, (GLfloat*)&this->diffuse);
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".attenuation").c_str()), 1, (GLfloat*)&this->attenuation);
		glUniform1f(glGetUniformLocation(shader, string(lightIndex + ".cutOff").c_str()), this->cutOff);
		glUniform1f(glGetUniformLocation(shader, string(lightIndex + ".outerCutOff").c_str()), this->outerCutOff);
	}

	void setType(LightType typeIn) {
		this->lightType = typeIn;
	}

	void setPosition(glm::vec3 positionIn) {
		this->position = positionIn;
	}
	void setPosition(float x, float y, float z) {
		this->position = glm::vec3(x, y, z);
	}

	void setDirection(glm::vec3 directionIn) {
		this->direction = directionIn;
	}
	void setColour(glm::vec3 colourIn) {
		this->colour = colourIn;
	}
	void setIntensity(GLfloat intensityIn) {
		this->intensity = intensityIn;
	}
	void setAttenuation(GLfloat linear, GLfloat quadratic) {
		this->attenuation = glm::vec3(1.0, linear, quadratic);
	}
	void setDiffusion(glm::vec3 diffuseIn) {
		this->diffuse = diffuseIn;
	}
	void setCutOff(GLfloat cutOffIn, GLfloat outerCutOffIn) {
		this->cutOff = glm::cos(glm::radians(cutOffIn));
		this->outerCutOff = glm::cos(glm::radians(outerCutOffIn));
	}

	LightType getType() {
		return lightType;
	}
	glm::vec3 getPosition() {
		return position;
	}
	GLfloat getIntensity() {
		return intensity;
	}
	glm::vec3 getAttenuation() {
		return attenuation;
	}
	glm::vec3 getDiffusion() {
		return diffuse;
	}
};

// Camera                      screenWidth, screenHeight, nearPlane, farPlane
Camera_settings camera_settings{ 800, 600, 0.1, 1000.0 };
Camera camera(camera_settings, glm::vec3(0.0, 5.0, 12.0));

//Timer
Timer timer;

double lastX = camera_settings.screenWidth / 2.0f;
double lastY = camera_settings.screenHeight / 2.0f;

vector<Light> lights;

// Position for Mobile Launcher
glm::vec3 ML_Position = glm::vec3(-1.23f, 0.0f, -4.0f);

// Heading to control Mobile Launcher heading
GLfloat ML_heading;

class Vehicle {
private:
	float decisionTime = 0;
	float decisionTimeLimit = 0;
public:
	Model* model;
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 direction = glm::vec3(0, 0, 0);
	glm::vec3 target = glm::vec3(0, 0, 0);
	float heading = 0.0f;
	float desiredHeading = 0.0f;
	int headLightIndex = 0;

	GLfloat speed = 5.0f;

	Vehicle() {
		// Pick a random heading and random time to choose between decision making
		this->desiredHeading = rand() % 360;
		this->decisionTimeLimit = rand() % 5 + 2;

		// Load truck model, and load random Truck texture
		this->model = new Model("Resources\\Models\\Truck.obj");
		this->model->attachTexture(TextureLoader::loadTexture(string("Resources\\Textures\\Truck\\Truck_" + to_string(rand() % 3 + 1) + ".png").c_str()));

		// Set current decision time, to time left
		this->decisionTime = this->decisionTimeLimit;
	}

	void addLight(vector<Light>& lights) {
		// Add light to light vector array, and get index
		lights.push_back(Light(LightType::SPOT, this->position, glm::vec3(1, 1, 1), 0.5, this->target));
		this->headLightIndex = lights.size() - 1;
	}

	void render(GLuint shader) {

		// Render truck model at position, with desired heading as rotation
		glm::mat4 truckModel = glm::translate(glm::mat4(1.0), this->position);
		truckModel = glm::rotate(truckModel, glm::radians(-this->desiredHeading), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(truckModel));
		this->model->draw(shader);
	}

	void drive() {

		// Set target heading
		this->target.x = sin(this->desiredHeading * 3.1415965 / 180.0);
		this->target.z = -cos(this->desiredHeading * 3.1415965 / 180.0);

		// Steer towards target
		glm::vec3 desired_velocity = this->target * this->speed * (float)timer.getDeltaTimeSeconds();
		glm::vec3 steering = desired_velocity - this->direction;

		// Move towards target
		this->direction += steering;
		this->position += direction;

		// Get heading from current direction
		this->heading = atan2(this->direction.z, this->direction.x);

		// Move towards direction
		this->position.x += direction.x * 0.125;
		this->position.z += direction.z * 0.125;

		// Check if Vehicle should pick a new heading
		if (this->decisionTime <= 0) {
			// Randomly choose a new heading based on current heading
			int min = this->heading - 180;
			int max = this->heading + 180;
			this->desiredHeading = (rand() % (max - min + 1)) + min;

			// Reset decision time
			this->decisionTime = decisionTimeLimit;
		}

		// Decrease decision time
		this->decisionTime -= timer.getDeltaTimeSeconds();
	}
};

// Properties to check whether SLS has launched
bool hasLaunched = false;
glm::vec3 SLSOffset = glm::vec3(0, 0, 0);
float velocity = 0;
glm::mat4 MLModel;

int main()
{
	srand(time(NULL));

	float programTime = 0.0;

	#pragma region Initialize OpenGL
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(camera_settings.screenWidth, camera_settings.screenHeight, "30003287 - Artemis Generation", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set the callback functions
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Rendering settings
	glfwSwapInterval(1);		// glfw enable swap interval to match screen v-sync
	glEnable(GL_DEPTH_TEST);	//Enables depth testing
	glEnable(GL_CULL_FACE);		//Enables face culling
	glFrontFace(GL_CCW);		//Specifies which winding order if front facing
	#pragma endregion

	//Shaders
	GLuint basicShader;
	GLuint skyboxShader;

	// Textures
	// Grass
	GLuint grassAlbedo;
	GLuint grassSpec;
	GLuint grassNorm;

	// SLS
	GLuint SLS_Core;
	GLuint SLS_SRB;
	GLuint SLS_Engine;

	GLuint skyboxTexture;
	GLuint VABTexture;
	GLuint coreStageSpec;

	// Load shaders
	GLSL_ERROR glsl_err_basic =
		ShaderLoader::createShaderProgram(
			string("Resources\\Shaders\\Basic_shader.vert"),
			string("Resources\\Shaders\\Basic_shader.frag"),
			&basicShader
		);
	GLSL_ERROR glsl_err_skybox =
		ShaderLoader::createShaderProgram(
			string("Resources\\Shaders\\skybox_vert.glsl"),
			string("Resources\\Shaders\\skybox_frag.glsl"),
			&skyboxShader
		);

	// Load textures
	// Grass textures obtained from ambientCG (Public Domain)
	// https://ambientCG.com/a/Ground037
	grassAlbedo = TextureLoader::loadTexture("Resources\\Textures\\Grass\\GrassColour.png");
	grassSpec = TextureLoader::loadTexture("Resources\\Textures\\Grass\\GrassSpec.png");
	grassNorm = TextureLoader::loadTexture("Resources\\Textures\\Grass\\GrassNormal.png");

	// VAB and VAB texture obtained from Sketchfab
	// Modifed to fix roof UV, and removal of unnecessary features such as Humans for scale, Crawlerway and Doors, etc.
	// https://sketchfab.com/3d-models/vehicle-assembly-building-3b37036c698548a48f727f8c8f21c01b
	VABTexture = TextureLoader::loadTexture("Resources\\Textures\\VAB_Texture.png");
	coreStageSpec = TextureLoader::loadTexture("Resources\\Models\\SLS\\CoreStage_spec.png");

	// Load Cubemap texture
	// Moonlit Golf obtained from PolyHaven & was converted from HDRI into a Cubemap
	// Skybox texture: https://polyhaven.com/a/moonlit_golf
	// HDRI to Cubemap: https://matheowis.github.io/HDRI-to-CubeMap/
	skyboxTexture = TextureLoader::loadCubeMapTexture("Resources\\Textures\\skybox\\moonlit-golf\\", "1024", ".png", GL_RGBA, GL_LINEAR, GL_LINEAR, 8.0F, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

	// Load models
	Model plane = Model("Resources\\Models\\Plane.obj");
	
	// Mobile Launcher uses a texture from ambientCG
	// https://ambientCG.com/a/Metal038
	Model ML = Model("Resources\\Models\\SLS\\ML.obj");
	Model VAB = Model("Resources\\Models\\VAB.obj");
	Model SLS = Model("Resources\\Models\\SLS\\SLS.obj");

	// Attach textures onto models;
	plane.attachTexture(grassAlbedo);
	plane.attachTexture(grassSpec, "texture_specular");
	plane.attachTexture(grassNorm, "texture_normal");

	VAB.attachTexture(VABTexture);
	SLS.attachTexture(coreStageSpec, "texture_specular");

	GLuint uMatSpecularExp = glGetUniformLocation(basicShader, "matSpecularExponent");
	GLfloat mat_specularExp = 32;

	#pragma region Skybox

	// Code obtained from GitHub repository JoeyDeVries/LearnOpenGL
	// https://github.com/JoeyDeVries/LearnOpenGL/blob/166aeced4b950daf8f7617a8e68568a9e500970f/src/4.advanced_opengl/6.1.cubemaps_skybox/cubemaps_skybox.cpp#L130-L194

	// Setup skybox VAO
	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	GLuint skyboxVAO, skyboxVBO;

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);

	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glUseProgram(skyboxShader);
	glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);
	#pragma endregion

	// Setup lights
	glm::vec3 moonColour = glm::vec3(0.05, 0.11, 0.28);
	Light Moon = Light(LightType::DIRECTIONAL, glm::vec3(0.0f, 0.0f, 0.0f), moonColour, 0.5, glm::vec3(-0.6, -0.5, -0.7));

	Light USAFlag_light = Light(LightType::SPOT, glm::vec3(4.8f, 1.7f, 0.8), glm::vec3(1, 1, 1), 0.5, glm::vec3(-0.7, 0.6, 0.3));
	Light NASALogo_light = Light(LightType::SPOT, glm::vec3(4.8f, 1.7f, -0.8), glm::vec3(1, 1, 1), 0.5, glm::vec3(-0.8, 0.6, -0.3));
	Light SLS_SRB_l_light = Light(LightType::BULB, glm::vec3(0,0,0), glm::vec3(1, 1, 1), 1);
	Light SLS_SRB_r_light = Light(LightType::BULB, glm::vec3(0,0,0), glm::vec3(1, 1, 1), 1);
	
	Light ML_Light_1 = Light(LightType::BULB, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 0.25);
	Light ML_Light_2 = Light(LightType::BULB, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 0.25);
	Light ML_Light_3 = Light(LightType::BULB, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 0.25);

	Light VABTI_light = Light(LightType::BULB, glm::vec3(5.0f, 1.5f, 0), glm::vec3(0.858, 0.458, 0.231), 0.01);
	VABTI_light.setAttenuation(4, 5);
	
	// Add lights to vector
	lights.push_back(Moon);

	lights.push_back(USAFlag_light);
	lights.push_back(NASALogo_light);
	lights.push_back(VABTI_light);
	
	lights.push_back(SLS_SRB_l_light);
	lights.push_back(SLS_SRB_r_light);
	
	lights.push_back(ML_Light_1);
	lights.push_back(ML_Light_2);
	lights.push_back(ML_Light_3);

	// Setup vehicles
	vector<Vehicle> vehicles;
	int vehicleCount = rand() % 10 + 5;

	for (int i = 0; i < vehicleCount; i++) {
		Vehicle truck = Vehicle();
		truck.addLight(lights);
		vehicles.push_back(truck);
	}

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		// Process Input
		processInput(window);
		timer.tick();
		programTime += timer.getDeltaTimeSeconds();

		// Update Window title to include average FPS
		string fps = "Avg FPS: " + to_string(int(timer.averageFPS()));
		string windowTitle = "30003287 - Artemis Generation (" + fps + ")";
		glfwSetWindowTitle(window, windowTitle.c_str());

		// Start rendering
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get default values for View, Project and Model matrix
		glm::mat4 identity = glm::mat4(1.0);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix();

		// Get current camera position
		glm::vec3 eyePos = camera.getCameraPosition();

		// Render the skybox before the scene
		drawSkybox(skyboxVAO, skyboxTexture, skyboxShader, view, projection);

		// Clear shaders and set current shader to basicShader
		glUseProgram(0);
		glUseProgram(basicShader);

		// Include view and projection and camera position into basicShader
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3fv(glGetUniformLocation(basicShader, "eyePos"), 1, (GLfloat*)&eyePos);

		//Pass material data
		glUniform1f(uMatSpecularExp, mat_specularExp);

		// Create a model matrix, and set it's position to the center
		// of the scene and scale it by 10 units
		// Pass the model into the shader and set the textureScale to 20
		glm::mat4 model = identity * glm::scale(identity, glm::vec3(10, 10.0, 10.0));
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1f(glGetUniformLocation(basicShader, "textureScale"), 20);
		plane.draw(basicShader); //Draw the plane as the floor of the scene
		
		// Reset the textureScale back to 1 in the shader
		glUniform1f(glGetUniformLocation(basicShader, "textureScale"), 1);

		// Draw the VAB at the center of the scne
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(identity));
		VAB.draw(basicShader); //Draw the VAB

		// Set the MLModel to be translated to its position and rotated by its heading
		MLModel = glm::translate(identity, ML_Position) * glm::rotate(identity, glm::radians(-ML_heading), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(MLModel));
		ML.draw(basicShader);

		// Create an SLSModel matrix which checks whether hasLaunched is true
		// If so, it will translate to the SLSOffset
		// if not, it will be set to the MLModel matrix and translate by its offset
		glm::mat4 SLSModel = hasLaunched ? glm::translate(identity, SLSOffset) : MLModel * glm::translate(identity, SLSOffset);
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(SLSModel));

		// Only draw the SLS model if the Y axis on SLSOffset is less or equal to 256
		if (SLSOffset.y <= 256) {
			SLS.draw(basicShader);
		}

		// Set the light positions for SLS_SRB_l_light and SLS_SRB_r_light
		// respective of the SRBs nozzle on the SLS model and only enable
		// the lights if hasLaunched is true.
		lights.at(4).setPosition(getMatrixPosition(SLSModel).x - 0.25, getMatrixPosition(SLSModel).y + 0.5, getMatrixPosition(SLSModel).z);
		lights.at(4).enabled = hasLaunched;
		lights.at(5).setPosition(getMatrixPosition(SLSModel).x + 0.25, getMatrixPosition(SLSModel).y + 0.5, getMatrixPosition(SLSModel).z);
		lights.at(5).enabled = hasLaunched;

		// Set the light positions of ML_Light_1, ML_Light_2, ML_Light_3
		// to the center point of MLModel's position, at varying heights
		lights.at(6).setPosition(getMatrixPosition(MLModel).x, getMatrixPosition(MLModel).y + 0.45, getMatrixPosition(MLModel).z);
		lights.at(7).setPosition(getMatrixPosition(MLModel).x, getMatrixPosition(MLModel).y + 2.25, getMatrixPosition(MLModel).z);
		lights.at(8).setPosition(getMatrixPosition(MLModel).x, getMatrixPosition(MLModel).y + 4.5, getMatrixPosition(MLModel).z);

		// Go through each vehicle in vehicles
		// and animate them using randomization
		for (int i = 0; i < vehicles.size(); i++) {

			// Gets current truck at index
			Vehicle truck = vehicles.at(i);

			// Sets the light's position and direction based on the truck's position and direction
			lights.at(truck.headLightIndex).setPosition(truck.position.x, truck.position.y + 0.25, truck.position.z + 0.15);
			lights.at(truck.headLightIndex).setDirection(truck.direction);

			// Move the truck somewhere new
			truck.drive();
			// Render truck at new location/direction
			truck.render(basicShader);

			// Set the vehicles current index to truck
			vehicles.at(i) = truck;
		}


		// Set the lightCount uniform to the size of the lights vector
		// so that OpenGL doesn't need to go through all 64 (Maximum allowed for this application) elements
		// just to render only a few lights;
		glUniform1i(glGetUniformLocation(basicShader, "lightCount"), lights.size());
		for (int i = 0; i < lights.size(); i++) {

			// Get current light
			Light light = lights.at(i);

			// Format current index to GLSL
			string lightLoc = "Light[" + to_string(i) + "]";
			
			// Set the uniform variables on basic shader
			// to the current index of lights vector
			light.processUniforms(basicShader, lightLoc);
		}

		// Check if SLS has launched
		if (hasLaunched) {
			// Increase velocity over time
			// Increase Y offset from origin
			velocity += timer.getDeltaTimeSeconds() * 2;
			SLSOffset.y += velocity * timer.getDeltaTimeSeconds();
		}

		// glfw: swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	timer.updateDeltaTime();

	// Close program
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Move Camera
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, timer.getDeltaTimeSeconds() * 4);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, timer.getDeltaTimeSeconds() * 4);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, timer.getDeltaTimeSeconds() * 4);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, timer.getDeltaTimeSeconds() * 4);

	// Get current direction that the Mobile Launcher is facing
	// into a vector (Converting from an angle to Vector)
	GLfloat directionX = sin(ML_heading * 3.1415965 / 180.0);
	GLfloat directionZ = -cos(ML_heading * 3.1415965 / 180.0);

	// Move Mobile Launcher forward or backward depending whether
	// user is holding down Up Arrow key or Down Arrow Key

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		ML_Position.x += directionX * 3.0f * timer.getDeltaTimeSeconds();
		ML_Position.z += directionZ * 3.0f * timer.getDeltaTimeSeconds();
	}

	// Move back
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		ML_Position.x -= directionX * 3.0f * timer.getDeltaTimeSeconds();
		ML_Position.z -= directionZ * 3.0f * timer.getDeltaTimeSeconds();
	}

	// Rotate the Mobile Launcher depending whether 
	// user is holding down Left Arrow Key or Right Arrow Key
	
	// Rotate Left
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		ML_heading -= timer.getDeltaTimeSeconds() * 100;
	// Rotate Right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		ML_heading += timer.getDeltaTimeSeconds() * 100;

	// "Launch" SLS Rocket when user presses Spacebar
	// SLSOffset will be set the Matrix position of MLModel
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		SLSOffset = getMatrixPosition(MLModel);
		hasLaunched = true;
	}

	// SLS rocket will reset when user presses R
	// SLSOffset will return back to origin point of 0, 0, 0
	// and velocity will be set to 0
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		SLSOffset = glm::vec3(0, 0, 0);
		velocity = 0;
		hasLaunched = false;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	glViewport(0, 0, width, height);
	camera.updateScreenSize(width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		camera.processMouseMovement(xoffset, yoffset);
	}
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.processMouseScroll(yoffset);
}

void drawSkybox(GLuint vao, GLuint texture, GLuint shader, glm::mat4 view, glm::mat4 projection) {
	
	// Code obtained from GitHub repository JoeyDeVries/LearnOpenGL
	// https://github.com/JoeyDeVries/LearnOpenGL/blob/166aeced4b950daf8f7617a8e68568a9e500970f/src/4.advanced_opengl/6.1.cubemaps_skybox/cubemaps_skybox.cpp#L253-L265

	// Disable depth mask
	// This is so that there is no depth
	// percieved whilst rendering the cube
	// to draw the skybox
	glDepthMask(GL_FALSE);

	// Setup shader matrices
	glUseProgram(shader);
	view = glm::mat4(glm::mat3(camera.getViewMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Render the skybox cube
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	// Re-enable depth mask
	glDepthMask(GL_TRUE);
}


// Get position vector from Matrix
// Solution:
// https://stackoverflow.com/a/19448411
glm::vec3 getMatrixPosition(glm::mat4 matrix) {
	return matrix[3];
}