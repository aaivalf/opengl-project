//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include "Windows.h"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

int glWindowWidth = 1280;
int glWindowHeight = 720;


GLfloat lastX = glWindowWidth / 2.0;
GLfloat lastY = glWindowHeight / 2.0;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 newCoord = glm::vec3(1.0f);

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

//flashlight
glm::vec3 pointLightPos(0.0f, 1.0f, 0.0f);
glm::vec3 pointLightColor(1.0f, 1.0f, 0.75f);
float pointLightRadius = 10.0f;
bool pointLightEnabled = false;

GLuint pointLightPosLoc;
GLuint pointLightColorLoc;
GLuint pointLightRadiusLoc;
GLuint pointLightEnabledLoc;
//end flashlight


gps::Camera myCamera(glm::vec3(9.813542f, 3.011043f, -20.226547f), glm::vec3(0.0f, 0.0f, 0.0f));
GLfloat cameraSpeed = 0.02f;

bool pressedKeys[1024];
GLfloat angle;
GLfloat lightAngle;

GLfloat ambientStrength = 0.7f;
GLfloat specularStrength = 0.5f;
GLfloat shininess = 64.0f;
GLfloat showFog = 0.0f;

GLfloat bias = 0.05f;
gps::Model3D myModel;

gps::Model3D lightCube;
//gps::Model3D sunModel;


gps::Model3D scene;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;

gps::SkyBox mySkyBox;

GLuint shadowMapFBO;
GLuint depthMapTexture;

std::vector<const GLchar*> faces;

std::ifstream file("cam_animation_dir.txt");


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	lightShader.useShaderProgram();
	
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Adjust the camera speed based on the scroll input
	cameraSpeed += static_cast<float>(yoffset) * 0.01f;
	if (cameraSpeed < 0.01f) {
		cameraSpeed = 0.01f; // Set a minimum speed to avoid negative or zero speed
	}
	std::cout << "Camera Speed: " << cameraSpeed << std::endl;
}

bool firstMouse = true;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{

	float pitch = 0;
	float yaw = 0;
	if (firstMouse == false) {
		pitch = (lastY - ypos) * 0.0025;
		yaw = (lastX - xpos) * 0.0025;
	}
	else {
		firstMouse = false;
	}

	lastX = xpos;
	lastY = ypos;

	myCamera.rotate(pitch, yaw);
}

void processMovement()
{
	//CAMERA MOVEMENT
	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 1.0f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 1.0f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_UP]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_LEFT]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	//-----------------------functionalitati----------------------
	
	//wireframe - poligoane
	if (pressedKeys[GLFW_KEY_M]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}

	//cu texturi
	if (pressedKeys[GLFW_KEY_N]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//vede puncte
	if (pressedKeys[GLFW_KEY_V]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	//accelereaza rotatia soarelui
	if (pressedKeys[GLFW_KEY_J]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	//stopeaza soarele intr-o directie
	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f; 
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}


	//spotlight - lumina orientata doar intr-o singura directie
	if (pressedKeys[GLFW_KEY_1])
	{
		lightDir.z -= 0.01f;
		lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
	}

	//intensitatea luminii: + de pe numpad
	if (pressedKeys[GLFW_KEY_KP_ADD])
	{
		myCustomShader.useShaderProgram();
		ambientStrength += 0.005f;
		specularStrength +=0.005f;
		shininess +=0.25f;

		
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "shininess"), shininess);
	}

	//intensitatea luminii : - de pe numpad
	if (pressedKeys[GLFW_KEY_KP_SUBTRACT])
	{
		myCustomShader.useShaderProgram();
		ambientStrength -= 0.005f;
		specularStrength -= 0.005f;
		shininess -= 0.25f;


		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "shininess"), shininess);
	}

	//PLAY camera animation - ia directiile din file - SE BLOCHEAZA DUPA CE APAS SPACE
	if (pressedKeys[GLFW_KEY_SPACE]) {

		float readA, readB, readC, readD, readE, readF;
		if (!file.eof()) {
			file >> readA >> readB >> readC >> readD >> readE >> readF;
			myCamera.moveFreely(glm::vec3(readD, readE, readF), glm::vec3(readA, readB, readC));
		}
		else {//ca sa nu se blocheze
			myCamera.moveFreely(glm::vec3(37.0121,2.84192,-2.65135), glm::vec3(-0.996838,0.0693025,0.038971));
		}
	}

	//ceata - on/off
	if (pressedKeys[GLFW_KEY_F])
	{
		
		if (showFog == 0)
		{
			myCustomShader.useShaderProgram();
			showFog = 1.0f;
			glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "showFog"), showFog);
			
		}
		else
		{
			myCustomShader.useShaderProgram();
			showFog = 0.0f;
			glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "showFog"), showFog);
			
		}
		Sleep(100);
	}
	//inregistrare camera animation - record in cam_animation_dir.txt
	if (pressedKeys[GLFW_KEY_KP_0]) {
		std::ofstream outFile("cam_animation_dir.txt", std::ios::app); //in append mode
		if (outFile.is_open()) {
			glm::vec3 position = myCamera.getCameraPosition();
			glm::vec3 direction = myCamera.getCameraDirection();
			outFile << direction.x << " " << direction.y << " " << direction.z << " ";
			outFile << position.x << " " << position.y << " " << position.z << std::endl;
			outFile.close();
			std::cout << "Camera position and direction written to cam_animation_dir.txt" << std::endl;
		}
		else {
			std::cerr << "Failed to open cam_animation_dir.txt for writing" << std::endl;
		}

	}

	//RELOAD animation
	if (pressedKeys[GLFW_KEY_R])
	{
		file.close();
		file.open("cam_animation_dir.txt");
	}

	//point light
	bool pKeyPressed = false;
	if (pressedKeys[GLFW_KEY_P]) {
		if (!pKeyPressed) {
			pointLightEnabled = !pointLightEnabled; // Toggle point light state
			myCustomShader.useShaderProgram();
			glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightEnabled"), pointLightEnabled);
			pKeyPressed = true; // Mark the key as pressed
		}
	}
	else {
		pKeyPressed = false; // Reset the key press state
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetScrollCallback(glWindow, scrollCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
	glClearColor(0.5f, 0.5f, 0.5f, 1);

}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 17.0f; //adjusted far plane
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initModels()
{

	lightCube = gps::Model3D("objects/cube/sun.obj", "objects/cube/");
	//sunModel = gps::Model3D("objects/cube/sun.obj", "objects/cube/");
	scene = gps::Model3D("objects/nature/nature_tarziu.obj", "objects/nature/");
	faces.push_back("skybox/Sorsele33/bluecloud_ft.jpg");
	faces.push_back("skybox/Sorsele33/bluecloud_bk.jpg");
	faces.push_back("skybox/Sorsele33/bluecloud_up.jpg");
	faces.push_back("skybox/Sorsele33/bluecloud_dn.jpg");
	faces.push_back("skybox/Sorsele33/bluecloud_rt.jpg");
	faces.push_back("skybox/Sorsele33/bluecloud_lf.jpg");
	mySkyBox.Load(faces);
}



void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initUniforms()
{
	
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	
	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");


	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(1.0f, 2.0f, 1.5f); // Adjust these values to position the light closer
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 0.75f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos"), 1, glm::value_ptr(pointLightPos));

	// Initialize  flashlight uniforms
	pointLightPos = myCamera.getCameraPosition(); // Place slightly ahead of the camera
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos"), 1, glm::value_ptr(pointLightPos));

	pointLightColor = glm::vec3(2.0f, 2.0f, 2.0f); // White point light
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor"), 1, glm::value_ptr(pointLightColor));

	pointLightRadius = 5.0f; // Set a reasonable radius for the point light
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightRadius"), pointLightRadius);

	pointLightEnabled = 0; // Start with the point light disabled
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightEnabled"), pointLightEnabled);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}


void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();

	// Render the scene to the depth buffer (first pass)
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	// Send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	scene.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render the scene (second pass)
	myCustomShader.useShaderProgram();

	// Send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	// Send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));

	// Compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	// Send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	// Bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	// Send model matrix data to shader    
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	// Send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));


	//set flashlight uniforms
	pointLightPos = pointLightPos = myCamera.getCameraPosition() + myCamera.getCameraTarget() * 1.0f; // Update flashlight position dynamically
	glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));
	glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));
	glUniform1f(pointLightRadiusLoc, pointLightRadius);
	glUniform1i(pointLightEnabledLoc, pointLightEnabled);
	//end set
	scene.Draw(myCustomShader);

	// Draw a white cube around the light
	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, 1.0f * lightDir);
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightCube.Draw(lightShader);

	mySkyBox.Draw(skyboxShader, view, projection);
}

void rotateLightSource()
{

	lightAngle += 0.3f;
	if (lightAngle > 360.0f)
		lightAngle -= 360.0f;
	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	myCustomShader.useShaderProgram();
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));

	//Update the sun position
    //sunPosition = lightDirTr;
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();


	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();
		rotateLightSource();
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
