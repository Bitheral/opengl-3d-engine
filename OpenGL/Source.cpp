#include "Includes.h"

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Camera settings
//							  width, heigh, near plane, far plane
Camera_settings camera_settings{ 1200, 1000, 0.1, 100.0 };

//Timer
Timer timer;

// Instantiate the camera object with basic data
Camera camera(camera_settings, glm::vec3(0.0, 5.0, 12.0));

double lastX = camera_settings.screenWidth / 2.0f;
double lastY = camera_settings.screenHeight / 2.0f;


// Obtained from LearnOpenGL
// https://learnopengl.com/code_viewer.php?code=advanced/cubemaps_skybox_data
const GLfloat skyboxVertices[] = {
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

int main()
{
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

	////	Shaders - Textures - Models	////

	GLuint basicShader;
	GLuint skyboxShader;

	// Texture container
	GLuint metalTex;
	GLuint marbleTex;
	GLuint skyboxTexture;

	// build and compile our shader program
	GLSL_ERROR glsl_err_basic = ShaderLoader::createShaderProgram(
		string("Resources\\Shaders\\Basic_shader.vert"), 
		string("Resources\\Shaders\\Basic_shader.frag"),
		&basicShader);

	GLSL_ERROR glsl_err_skybox = ShaderLoader::createShaderProgram(
		string("Resources\\Shaders\\skybox_vert.glsl"),
		string("Resources\\Shaders\\skybox_frag.glsl"),
		&skyboxShader);

	GLuint skyboxArrayBuffer;
	GLuint skyboxVertexBuffer;

	glGenVertexArrays(1, &skyboxArrayBuffer);
	glBindVertexArray(skyboxArrayBuffer);

	glGenBuffers(1, &skyboxVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);


	Model sphere("Resources\\Models\\Sphere.obj");
	Model plane("Resources\\Models\\Plane.obj");

	metalTex = TextureLoader::loadTexture("Resources\\Models\\metal_texture.png");
	marbleTex = TextureLoader::loadTexture("Resources\\Models\\marble_texture.jpg");
	skyboxTexture = TextureLoader::loadTexture("Resources\\Models\\marble_texture.jpg");

	sphere.attachTexture(metalTex);
	plane.attachTexture(marbleTex);

	//Light Data///////////////////////////////////////////////
	// Lights
	GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };	// Dim light 
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };	// White main light 

	GLfloat lightOne_position[] = { 5.0, 5.0, 5.0, 1.0 };	// Point light (w=1.0)
	GLfloat lightOne_colour[] = { 8.0, 0.0, 0.0, 1.0 };

	GLfloat lightTwo_position[] = { -5.0, 5.0, 5.0, 1.0 };	// Point light (w=1.0)
	GLfloat lightTwo_colour[] = { 0.0, 8.0, 0.0, 1.0 };

	GLfloat lightThree_position[] = { 5.0, 5.0, -5.0, 1.0 };	// Point light (w=1.0)
	GLfloat lightThree_colour[] = { 0.0, 0.0, 8.0, 1.0 };

	GLfloat lightFour_position[] = { -5.0, 5.0, -5.0, 1.0 };	// Point light (w=1.0)
	GLfloat lightFour_colour[] = { 8.0, 8.0, 8.0, 1.0 };

	GLfloat	attenuation[] = { 1.0, 0.10, 0.08 };

	// Materials
	GLfloat mat_amb_diff[] = { 1.0, 1.0, 1.0, 1.0 };	// Texture map will provide ambient and diffuse.
	GLfloat mat_specularCol[] = { 1.0, 1.0, 1.0, 1.0 }; // White highlight
	GLfloat mat_specularExp = 32.0;					// Shiny surface

	//Uniform Locations - Basic Shader////////////////////////////////////////////
	// Get unifom locations in shader
	GLuint uLightAmbient = glGetUniformLocation(basicShader, "lightAmbient");
	GLuint uLightDiffuse = glGetUniformLocation(basicShader, "lightDiffuse");
	GLuint uLightAttenuation = glGetUniformLocation(basicShader, "lightAttenuation");

	GLuint uLightOnePosition = glGetUniformLocation(basicShader, "lightOne");
	GLuint uLightOneColour = glGetUniformLocation(basicShader, "lightOneColour");

	GLuint uLightTwoPosition = glGetUniformLocation(basicShader, "lightTwo");
	GLuint uLightTwoColour = glGetUniformLocation(basicShader, "lightTwoColour");

	GLuint uLightThreePosition = glGetUniformLocation(basicShader, "lightThree");
	GLuint uLightThreeColour = glGetUniformLocation(basicShader, "lightThreeColour");

	GLuint uLightFourPosition = glGetUniformLocation(basicShader, "lightFour");
	GLuint uLightFourColour = glGetUniformLocation(basicShader, "lightFourColour");


	GLuint uEyePos = glGetUniformLocation(basicShader, "eyePos");

	// Get material unifom locations in shader
	GLuint uMatAmbient = glGetUniformLocation(basicShader, "matAmbient");
	GLuint uMatDiffuse = glGetUniformLocation(basicShader, "matDiffuse");
	GLuint uMatSpecularCol = glGetUniformLocation(basicShader, "matSpecularColour");
	GLuint uMatSpecularExp = glGetUniformLocation(basicShader, "matSpecularExponent");

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);
		timer.tick();

		// render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 sphereModel = glm::mat4(1.0);
		glm::mat4 planeModel = glm::mat4(1.0);

		glDepthMask(GL_FALSE);
		glUseProgram(skyboxShader);

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix();

		glBindVertexArray(skyboxArrayBuffer);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0), glm::vec3(0.3, 0.3, 0.3));
		glm::vec3 eyePos = camera.getCameraPosition();

		glUseProgram(basicShader); //Use the Basic shader

		//Pass the uniform data to Basic shader///////////////////////////////////
		//Pass the light data
		glUniform4fv(uLightDiffuse, 1, (GLfloat*)&light_diffuse);
		glUniform4fv(uLightAmbient, 1, (GLfloat*)&light_ambient);
		
		glUniform4fv(uLightOnePosition, 1, (GLfloat*)&lightOne_position);
		glUniform4fv(uLightOneColour, 1, (GLfloat*)&lightOne_colour);

		glUniform4fv(uLightTwoPosition, 1, (GLfloat*)&lightTwo_position);
		glUniform4fv(uLightTwoColour, 1, (GLfloat*)&lightTwo_colour);

		glUniform4fv(uLightThreePosition, 1, (GLfloat*)&lightThree_position);
		glUniform4fv(uLightThreeColour, 1, (GLfloat*)&lightThree_colour);

		glUniform4fv(uLightFourPosition, 1, (GLfloat*)&lightFour_position);
		glUniform4fv(uLightFourColour, 1, (GLfloat*)&lightFour_colour);

		glUniform3fv(uLightAttenuation, 1, (GLfloat*)&attenuation);
		glUniform3fv(uEyePos, 1, (GLfloat*)&eyePos);

		//Pass material data
		glUniform4fv(uMatAmbient, 1, (GLfloat*)&mat_amb_diff);
		glUniform4fv(uMatDiffuse, 1, (GLfloat*)&mat_amb_diff);
		glUniform4fv(uMatSpecularCol, 1, (GLfloat*)&mat_specularCol);
		glUniform1f(uMatSpecularExp, mat_specularExp);

		glUniformMatrix4fv(glGetUniformLocation(basicShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(basicShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glm::vec3 cameraPos = camera.getCameraPosition();

		lightFour_colour[0] = cameraPos.r;
		lightFour_colour[1] = cameraPos.g;
		lightFour_colour[2] = cameraPos.b;

		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(planeModel));
		plane.draw(basicShader); //Draw the plane

		//sphereModel = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 3.0, -3.0)) * scaleMat;
		//glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
		//sphere.draw(basicShader); //Draw first sphere

		//sphereModel = glm::translate(glm::mat4(1.0), glm::vec3(3.0, 3.0, -3.0)) * scaleMat;
		//glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
		//sphere.draw(basicShader); //Draw second sphere

		//sphereModel = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 3.0, 3.0)) * scaleMat;
		//glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
		//sphere.draw(basicShader); //Draw third sphere

		//sphereModel = glm::translate(glm::mat4(1.0), glm::vec3(3.0, 3.0, 3.0)) * scaleMat;
		//glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
		//sphere.draw(basicShader); //Draw fourth sphere

		// glfw: swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
	timer.updateDeltaTime();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, timer.getDeltaTimeSeconds());
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

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}