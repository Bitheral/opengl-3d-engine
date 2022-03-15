#include "Includes.h"
#include <utility>
#include <cmath>

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void drawSkybox(GLuint vbo, GLuint texture, GLuint shader, glm::mat4 view, glm::mat4 projection);

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
	glm::vec3 intensity;
	glm::vec3 attenuation;
	glm::vec3 diffuse;
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec4 ambient = glm::vec4(0.1, 0.1, 0.1, 1.0);
	GLfloat cutOff;
	GLfloat outerCutOff;

public:

	int enabled;

	Light(LightType typeIn, glm::vec3 positionIn, glm::vec3 intensityIn) {
		this->enabled = 0;
		this->lightType = typeIn;
		this->setPosition(positionIn);
		this->setIntensity(intensityIn);
		this->setCutOff(1.0, 17.5);
	}

	void processUniforms(GLuint shader, string lightIndex) {

		glUniform1i(glGetUniformLocation(shader, string(lightIndex + ".enabled").c_str()), this->enabled);
		glUniform1i(glGetUniformLocation(shader, string(lightIndex + ".type").c_str()), static_cast<GLuint>(this->lightType));
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".position").c_str()), 1, (GLfloat*)&this->position);
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".direction").c_str()), 1, (GLfloat*)&this->direction);
		glUniform4fv(glGetUniformLocation(shader, string(lightIndex + ".ambient").c_str()), 1, (GLfloat*)&this->ambient);
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".intensity").c_str()), 1, (GLfloat*)&this->intensity);
		glUniform3fv(glGetUniformLocation(shader, string(lightIndex + ".diffuse").c_str()) , 1, (GLfloat*)&this->diffuse);
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
	void setDirection(glm::vec3 directionIn) {
		this->direction = directionIn;
	}
	void setIntensity(glm::vec3 intensityIn) {
		this->intensity = intensityIn;
	}
	void setAttenuation(glm::vec3 attenuationIn) {
		this->attenuation = attenuationIn;
	}
	void setDiffusion(glm::vec3 diffuseIn) {
		this->diffuse = diffuseIn;
	}
	void setCutOff(GLfloat cutOffIn, GLfloat outerCutOffIn) {
		this->cutOff = cutOffIn;
		this->outerCutOff = outerCutOffIn;
	}

	LightType getType() {
		return lightType;
	}
	glm::vec3 getPosition() {
		return position;
	}
	glm::vec3 getIntensity() {
		return intensity;
	}
	glm::vec3 getAttenuation() {
		return attenuation;
	}
	glm::vec3 getDiffusion() {
		return diffuse;
	}
};

struct TexturedMaterial {
	GLuint albedo = 0;
	GLuint specular = 0;
	GLuint normal = 0;

	TexturedMaterial(string albedoPath) {
		albedo = TextureLoader::loadTexture(albedoPath);
	}

	TexturedMaterial(string albedoPath, string specularPath) {
		albedo = TextureLoader::loadTexture(albedoPath);
		specular = TextureLoader::loadTexture(specularPath);
	}

	TexturedMaterial(string albedoPath, string specularPath, string normalPath) {
		albedo = TextureLoader::loadTexture(albedoPath);
		specular = TextureLoader::loadTexture(specularPath);
		normal = TextureLoader::loadTexture(normalPath);
	}
};

// Camera                      screenWidth, screenHeight, nearPlane, farPlane
Camera_settings camera_settings { 1200, 1000, 0.1, 100.0 };
Camera camera(camera_settings, glm::vec3(0.0, 5.0, 12.0));

//Timer
Timer timer;

double lastX = camera_settings.screenWidth / 2.0f;
double lastY = camera_settings.screenHeight / 2.0f;

vector<Light> lights;

int main()
{
	#pragma region Initialize OpenGL
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(camera_settings.screenWidth, camera_settings.screenHeight, "Computer Graphics: Tutorial 20", NULL, NULL);
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
	GLuint metalTex;
	GLuint marbleTex;
	GLuint skyboxTexture;

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
	marbleTex = TextureLoader::loadTexture("Resources\\Models\\marble_texture.jpg");
	skyboxTexture = TextureLoader::loadCubeMapTexture("Resources\\Textures\\skybox\\", "", ".png", GL_RGBA, GL_LINEAR, GL_LINEAR, 8.0F, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

	TexturedMaterial metalMaterial = TexturedMaterial("Resources\\Models\\metal_texture.png", "Resources\\Models\\metal_texture_specular.png");


	// Models
	Model sphere = Model("Resources\\Models\\Sphere.obj");
	Model plane = Model("Resources\\Models\\Plane.obj");
	plane.attachTexture(marbleTex);

	sphere.attachTexture(metalMaterial.albedo);
	sphere.attachTexture(metalMaterial.specular, "texture_specular");
	//plane.attachTexture(metalMaterial.specular, "texture_specular");

	// Lights
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };	// White main light 

	lights.push_back(
		Light(LightType::BULB, glm::vec3(5.0, 5.0, 5.0), glm::vec3(4, 0, 0))
	);
	lights.push_back(
		Light(LightType::SPOT, glm::vec3(-5.0, 5.0, 5.0), glm::vec3(0.0, 1, 0.0))
	);
	lights.push_back(
		Light(LightType::BULB, glm::vec3(5.0, 5.0, -5.0), glm::vec3(0.0, 0.0, 4))
	);
	lights.push_back(
		Light(LightType::SPOT, glm::vec3(-5.0, 5.0, -5.0), glm::vec3(0.01, 0.01, 0.01))
	);

	lights.at(0).setAttenuation(glm::vec3(1.0, 0.09, 0.032f));
	lights.at(2).setAttenuation(glm::vec3(1.0, 0.09, 0.032f));
	lights.at(3).setAttenuation(glm::vec3(1.0, 0.09, 0.032f));
	lights.at(1).setAttenuation(glm::vec3(1.0, 0.09, 0.032f));

	// Materials
	GLfloat mat_amb_diff[] = { 1.0, 1.0, 1.0, 1.0 };	// Texture map will provide ambient and diffuse.
	GLfloat mat_specularCol[] = { 1.0, 1.0, 1.0, 1.0 }; // White highlight
	GLfloat mat_specularExp = 32.0;					// Shiny surface

	// Get material unifom locations in shader
	GLuint uMatAmbient = glGetUniformLocation(basicShader, "matAmbient");
	GLuint uMatDiffuse = glGetUniformLocation(basicShader, "matDiffuse");
	GLuint uMatSpecularCol = glGetUniformLocation(basicShader, "matSpecularColour");
	GLuint uMatSpecularExp = glGetUniformLocation(basicShader, "matSpecularExponent");

	// Skybox

	float skyboxVertices[] = {
		// positions          
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

	int frames = 0;
	int programTime = 1;

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);
		timer.tick();
		programTime += timer.getDeltaTimeSeconds();

		string fps = "Avg FPS: " + to_string(int(timer.averageFPS()));
		string windowTitle = "30003287 - Apollo / Artemis / Superheavy Starship (" + fps + ")";
		glfwSetWindowTitle(window, windowTitle.c_str());

		// render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 sphereModel = glm::mat4(1.0);
		glm::mat4 planeModel = glm::mat4(1.0);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix();

		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0), glm::vec3(0.3, 0.3, 0.3));
		glm::vec3 eyePos = camera.getCameraPosition();

		//lights.at(1).setPosition(eyePos);
		//lights.at(1).setDirection(camera.Target);

		drawSkybox(skyboxVAO, skyboxTexture, skyboxShader, view, projection);

		glUseProgram(0);
		glUseProgram(basicShader);

		glUniformMatrix4fv(glGetUniformLocation(basicShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3fv(glGetUniformLocation(basicShader, "eyePos"), 1, (GLfloat*)&eyePos);

		lights.at(1).setPosition(eyePos);
		lights.at(3).enabled = 1;

		glUniform1i(glGetUniformLocation(basicShader, "lightCount"), lights.size());
		for (int i = 0; i < lights.size(); i++) {

			Light light = lights.at(i);

			light.setDiffusion(glm::vec3(1.0, 1.0, 1.0));

			string lightLoc = "Light[" + to_string(i) + "]";
			light.processUniforms(basicShader, lightLoc);
		}

		//glUniform3fv(uLightAttenuation, 1, (GLfloat*)&attenuation);

		//Pass material data
		glUniform4fv(uMatAmbient, 1, (GLfloat*)&mat_amb_diff);
		glUniform4fv(uMatDiffuse, 1, (GLfloat*)&mat_amb_diff);
		glUniform4fv(uMatSpecularCol, 1, (GLfloat*)&mat_specularCol);
		glUniform1f(uMatSpecularExp, mat_specularExp);

		
		glUniform1i(glGetUniformLocation(basicShader, "renderSpecularMap"), 0);
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(planeModel));
		plane.draw(basicShader); //Draw the plane

		glUniform1i(glGetUniformLocation(basicShader, "renderSpecularMap"), 1);

		sphereModel = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 3.0, -3.0)) * scaleMat;
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
		sphere.draw(basicShader); //Draw first sphere

		sphereModel = glm::translate(glm::mat4(1.0), glm::vec3(3.0, 3.0, -3.0)) * scaleMat;
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
		sphere.draw(basicShader); //Draw second sphere

		sphereModel = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 3.0, 3.0)) * scaleMat;
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
		sphere.draw(basicShader); //Draw third sphere

		sphereModel = glm::translate(glm::mat4(1.0), glm::vec3(3.0, 3.0, 3.0)) * scaleMat;
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
		sphere.draw(basicShader); //Draw fourth sphere

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

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, timer.getDeltaTimeSeconds() * 4);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, timer.getDeltaTimeSeconds() * 4);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, timer.getDeltaTimeSeconds() * 4);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, timer.getDeltaTimeSeconds() * 4);

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
	// draw skybox as last
	//glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	//glDepthFunc(GL_FALSE);
	glDepthMask(GL_FALSE);
	glUseProgram(shader);
	view = glm::mat4(glm::mat3(camera.getViewMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	/*skyboxShader.setMat4("view", view);
	skyboxShader.setMat4("projection", projection);*/
	// skybox cube
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	//glDepthFunc(GL_LESS); // set depth function back to default
	//glDepthFunc(GL_TRUE);
	glDepthMask(GL_TRUE);
}