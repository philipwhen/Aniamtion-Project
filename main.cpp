// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"  //create and compile shader program
#include "Camera.h" //Used to create camera, accept view/projection matrix changes based on input
#include "Model.h" //Load model, vertices and its texture.
#include "Background.h"//Load background pics

//The main work for this project is in this header file
//Please define input (position, rotation etc...) in this header file
#include "MatrixTransformation.h"  

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//SOIL library is used to import images.
#include <SOIL.h>

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
GLFWwindow* GetEverythingInitialized();

// Camera
Camera camera(glm::vec3(0.0f, 50.0f, 100.0f));
bool keys[1024] = { false };
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat stepsPerSec = 0.5f;
bool gameStart = false;
bool initialized = false;
bool tripDone = false;


// The MAIN function, from here we start our application and run our Game loop

int main()
{
	// Init GLFW
	GLFWwindow* window = GetEverythingInitialized();

	// Setup and compile all the shaders
	Shader objShader("VertexShader.txt", "FragmentShader.txt");
	Shader backgroundShader("bgVertexShader.txt","bgFragmentShader.txt");


	// Load models
	Model robotBase("robot/base.dae",1);
	Model leftThigh("robot/leftThigh.dae",2, glm::vec3(0.0f, 0.0f, -3.5f));
	Model rightThigh("robot/rightThigh.dae",3, glm::vec3(0.0f, 0.0f, 3.5f));
	Model leftLeg("robot/leftLeg.dae", 4,  glm::vec3(0.0f, -10.0f, 0.0f));
	Model rightLeg("robot/rightLeg.dae", 5,glm::vec3(0.0f, -10.0f, 0.0f));
	Model leftFoot("robot/leftFoot.dae", 6,  glm::vec3(0.0f, -6.0f, 0.0f));
	Model rightFoot("robot/rightFoot.dae", 7, glm::vec3(0.0f, -6.0f, 0.0f));
	Model upperBody("robot/upperBody.dae", 8, glm::vec3(0.0f, 0.0f, 0.0f),0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	Model leftArm("robot/leftArm.dae",9, glm::vec3(0.0f, 8.7f, -5.0f));
	Model rightArm("robot/rightArm.dae", 10, glm::vec3(0.0f, 8.7f, 5.0f));
	Model leftHand("robot/leftHand.dae", 11, glm::vec3(0.0f, -5.0f, -2.1f));
	Model rightHand("robot/rightHand.dae", 12, glm::vec3(0.0f, -5.0f, 2.1f));
	Model teapot("teapot/teapot.dae",0);


	//Key frame angles for running
	GLfloat thighFramesr[] = {15.0f,  45.0f, 65.0f, 25.0f, 0.0f, -20.0f, -50.0f, -10.0f};
	GLfloat legFramesr[] =  {-120.0f,-90.0f, -45.0f, -10.0f,  0.0f,  -20.0f, -35.0f, -80.0f};
	GLfloat footFramesr[] = {-20.0f, 10.0f, -0.0f, -10.0f, 0.0f,  40.0f, 0.0f, -40.0f};
	GLfloat upperBodyFramesr[] = { 0.0f, 30.0f, 0.0f,  -30.0f};
	GLfloat armFramesr[] = {0.0f, -30.0f, -45.0f, -30.0f, 0.0f, 30.0f, 45.0f, 30.0f};
	GLfloat handFramesr[] = {45.0f, 30.0f, 45.0f, 90.0f};
	GLfloat baseFramesr[] = {0.0f,1.0f,0.0f,-1.0f,0.0f,1.0f,0.0f,-1.0f};
	//Key frame angles for walking
	GLfloat thighFramesw[] = { 7.5f,  22.5f, 30.0f, 12.5f, 0.0f, -10.0f, -25.0f, -5.0f };
	GLfloat legFramesw[] = { -60.0f,-45.0f, -22.5f, -5.0f,  0.0f,  -10.0f, -20.0f, -40.0f };
	GLfloat footFramesw[] = { -10.0f, 20.0f, -0.0f, -5.0f, 0.0f,  20.0f, 0.0f, -20.0f };
	GLfloat upperBodyFramesw[] = { 0.0f, 15.0f, 0.0f,  -15.0f };
	GLfloat armFramesw[] = { 0.0f, -15.0f, -22.0f, -15.0f, 0.0f, 15.0f, 22.0f, 15.0f };
	GLfloat handFramesw[] = { 22.0f, 15.0f, 22.0f, 45.0f };
	GLfloat baseFramesw[] = { 0.0f,1.0f,0.0f,-1.0f,0.0f,1.0f,0.0f,-1.0f };
	//Key frame angles for jumping
	GLfloat thighFramesj[] = { 0.0f,  30.0f, 0.0f, 0.0f, 0.0f, 30.0f, 0.0f, 0.0f };
	GLfloat legFramesj[] = { 0.0f, -45.0f, 0.0f, 0.0f,  0.0f,  -45.0f, 0.0f, 0.0f };
	GLfloat footFramesj[] = { 0.0f, 20.0f, 0.0f, 0.0f, 0.0f,  20.0f, 0.0f, 0.0f };
	GLfloat upperBodyFramesj[] = { 0.0f, 0.0f, 0.0f,  0.0f };
	GLfloat armFramesj[] = { 0.0f, 0.0f, 45.0f, 10.0f, 0.0f, 0.0f, 45.0f,10.0f };
	GLfloat handFramesj[] = { 0.0f, 45.0f, 90.0f,45.0f, 0.0f, 45.0f, 90.0f,45.0f };
	GLfloat baseFramesj[] = { 0.0f,-5.0f,0.0f,5.0f,0.0f,-5.0f,0.0f,5.0f };

	//Init these objects, install key frame angles, init rotation matrix
	Model::cycleDuration = 2.0f / stepsPerSec; //two steps a cycle
	robotBase.initKeyFrameInfo(false, baseFramesr, baseFramesw, baseFramesj, sizeof(baseFramesr) / sizeof(GLfloat),false);
	leftThigh.initKeyFrameInfo(false, thighFramesr, thighFramesw, thighFramesj, sizeof(thighFramesr) / sizeof(GLfloat));
	rightThigh.initKeyFrameInfo(true, thighFramesr, thighFramesw, thighFramesj, sizeof(thighFramesr) / sizeof(GLfloat));
	leftLeg.initKeyFrameInfo(false, legFramesr, legFramesw, legFramesj, sizeof(legFramesr) / sizeof(GLfloat));
	rightLeg.initKeyFrameInfo(true, legFramesr, legFramesw, legFramesj, sizeof(legFramesr) / sizeof(GLfloat) );
	leftFoot.initKeyFrameInfo(false, footFramesr, footFramesw, footFramesj, sizeof(footFramesr) / sizeof(GLfloat));
	rightFoot.initKeyFrameInfo(true, footFramesr, footFramesw, footFramesj, sizeof(footFramesr) / sizeof(GLfloat));
	upperBody.initKeyFrameInfo(false, upperBodyFramesr, upperBodyFramesw, upperBodyFramesj, sizeof(upperBodyFramesr) / sizeof(GLfloat));
	leftArm.initKeyFrameInfo(false, armFramesr, armFramesw, armFramesj, sizeof(armFramesr) / sizeof(GLfloat));
	rightArm.initKeyFrameInfo(true, armFramesr, armFramesw, armFramesj, sizeof(armFramesr) / sizeof(GLfloat));
	leftHand.initKeyFrameInfo(false, handFramesr, handFramesw, handFramesj, sizeof(handFramesr) / sizeof(GLfloat));
	rightHand.initKeyFrameInfo(true, handFramesr, handFramesw, handFramesj, sizeof(handFramesr) / sizeof(GLfloat));

	//Create VAO, VBO and texture, buffer data to VBO and construct VAO

	Background background("background.jpg",0);
	TrajectoryLine trajectoryLine(keyFramePose,numberOfPoints,stepSize,1);


	//Define variables for game loop, initialize them before game start.
	glm::mat4 cameraMatrix,model,spineMatrix, submodel;
	int currentPoint=1;
	GLfloat tValue = 0, lasttUpdate=0, tStep, tIntervalTime, walkCycleTime, walkCycleStartTime;
	//setup frame interval
	if (SPEED_CONTROL_ENABLED) {
		tStep = 1 / FRAME_PER_KEY;
		tIntervalTime = 1 / FRAME_PER_SEC / speedFactor[currentPoint - 1];
	}
	else
	{
		tStep = 1 / FRAME_PER_KEY;
		tIntervalTime = 1 / FRAME_PER_SEC;
	}
	GLfloat thisInterpQuat[7];
	GLfloat thisInterpEuler[6];
	glm::vec3 thisPosition, thisEuler;
	glm::vec4 thisQuaternion;

	//Choose which matrix to use (Catmul_Rom or B-Spline
	if(CATMUL_ROM_INTERPOLATION){
		spineMatrix = buildCatmullRomMatrix();
	}
	else { spineMatrix = buildBSplineMatrix(); }
	//If interpolation is based on Euler angles.
	if (EULER_INTERPOLATION) {
		updatePoseMatrixEuler(currentPoint, currentPoseMatrixEuler);

		//First interpolation is when t==0 at beginning point (second point since first is used for interpolation.
		setInterEulerFromEuler(currentPoint, tValue, spineMatrix, currentPoseMatrixEuler, thisInterpEuler);
		for (int i = 0; i < 3; i++) { thisPosition[i] = thisInterpEuler[i]; }
		for (int i = 0; i < 3; i++) { thisEuler[i] = thisInterpEuler[i + 3]; }
		model = glm::transpose(getFinalMatrixEuler(thisPosition, thisEuler));

	}
	//Pose matrix is a 4X7 matrix, first 3 arguments are position, next 4 arguments are quaternions.
	//When animation goes on, the two points before and after the curve is stored in this matrix.
	else {
		updatePoseMatrix(currentPoint, currentPoseMatrixQuat);

		//First interpolation is when t==0 at beginning point (second point since first is used for interpolation.
		setInterQuatFromQuat(currentPoint, tValue, spineMatrix, currentPoseMatrixQuat, thisInterpQuat);
		for (int i = 0; i < 3; i++) { thisPosition[i] = thisInterpQuat[i]; }
		for (int i = 0; i < 4; i++) { thisQuaternion[i] = thisInterpQuat[i + 3]; }
		model = glm::transpose(getFinalMatrixQuat(thisPosition, thisQuaternion));
	}
	//press B and then fighter jet will start fly
	std::cout << "Press B key to Begin!" << std::endl;
	robotState = running;

	// Game loop
	do	{
		// Set frame time for camera control
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events for camera control
		glfwPollEvents();
		Do_Movement();

		// Clear the colorbuffer and build background
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Calculate camera projection and view matrix based on key and mouse input, combine them together get cameraMatrix
		cameraMatrix = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f)*camera.GetViewMatrix();

		//Build background as visible coordinates
		backgroundShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(backgroundShader.Program, "cameraMatrix"), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
		background.Draw(backgroundShader);
		trajectoryLine.Draw(backgroundShader);

		//Now we start draw objects.
		objShader.Use();   // <-- Don't forget this one!
						// Transformation matrices
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "cameraMatrix"), 1, GL_FALSE, glm::value_ptr(cameraMatrix));

		//Check if game is still going on.
		
		if(!tripDone&&gameStart){
			if (!initialized) {
				initialized = true;
				Model::cycleStartTime=glfwGetTime();
				robotBase.startMoving();
				leftThigh.startMoving();
				rightThigh.startMoving();
				leftLeg.startMoving();
				rightLeg.startMoving();
				leftFoot.startMoving();
				rightFoot.startMoving();
				upperBody.startMoving();
				leftArm.startMoving();
				rightArm.startMoving();
				leftHand.startMoving();
				rightHand.startMoving();
			}
			//Check if t value needs to be updated.
			if ((glfwGetTime()-lasttUpdate) >= tIntervalTime) {
				lasttUpdate = glfwGetTime();
				tValue += tStep;

				//change Point
				if (tValue >= 1) {
					tValue = 0;
					currentPoint += 1;
					if (currentPoint >= 9 && currentPoint < 17) {
						robotState = walking;
					}
					else if (currentPoint >= 17) { robotState = jumping; }
					std::cout << "Trip set to point: " << currentPoint << std::endl;
					if (SPEED_CONTROL_ENABLED) {
						//change speed settings
						tIntervalTime = 1 / FRAME_PER_SEC / speedFactor[currentPoint - 1];
					}
					//Check if we arrived at last point and movement should stop
					if (currentPoint >= numberOfPoints - 2) {
						currentPoint =numberOfPoints-3;
						tValue = 1;
						tripDone = true;
					}
					//when moved to next curve, the pose matrix will need to be updated to store the new 4 points.
					if (EULER_INTERPOLATION) {
						updatePoseMatrixEuler(currentPoint, currentPoseMatrixEuler);
					}
					else { updatePoseMatrix(currentPoint, currentPoseMatrixQuat); }
				}
				if (EULER_INTERPOLATION) {
					setInterEulerFromEuler(currentPoint, tValue, spineMatrix, currentPoseMatrixEuler, thisInterpEuler);
					for (int i = 0; i < 3; i++) { thisPosition[i] = thisInterpEuler[i]; }
					for (int i = 0; i < 3; i++) { thisEuler[i] = thisInterpEuler[i + 3]; }
					model = glm::transpose(getFinalMatrixEuler(thisPosition, thisEuler));
				}
				else {
					setInterQuatFromQuat(currentPoint, tValue, spineMatrix, currentPoseMatrixQuat, thisInterpQuat);
					for (int i = 0; i < 3; i++) { thisPosition[i] = thisInterpQuat[i]; }
					for (int i = 0; i < 4; i++) { thisQuaternion[i] = thisInterpQuat[i + 3]; }
					model = glm::transpose(getFinalMatrixQuat(thisPosition, thisQuaternion));
				}
			//walkTValue
				robotBase.frameUpdate();
				leftThigh.frameUpdate();
				rightThigh.frameUpdate();
				leftLeg.frameUpdate();
				rightLeg.frameUpdate();
				leftFoot.frameUpdate();
				rightFoot.frameUpdate();
				upperBody.frameUpdate();
				leftArm.frameUpdate();
				rightArm.frameUpdate();
				leftHand.frameUpdate();
				rightHand.frameUpdate();
			}

		}
		//send model matrix to shader
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "base"), 1, GL_FALSE, glm::value_ptr(robotBase.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "leftThigh"), 1, GL_FALSE, glm::value_ptr(leftThigh.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "rightThigh"), 1, GL_FALSE, glm::value_ptr(rightThigh.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "leftLeg"), 1, GL_FALSE, glm::value_ptr(leftLeg.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "rightLeg"), 1, GL_FALSE, glm::value_ptr(rightLeg.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "leftFoot"), 1, GL_FALSE, glm::value_ptr(leftFoot.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "rightFoot"), 1, GL_FALSE, glm::value_ptr(rightFoot.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "upperBody"), 1, GL_FALSE, glm::value_ptr(upperBody.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "leftArm"), 1, GL_FALSE, glm::value_ptr(leftArm.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "rightArm"), 1, GL_FALSE, glm::value_ptr(rightArm.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "leftHand"), 1, GL_FALSE, glm::value_ptr(leftHand.currentRotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "rightHand"), 1, GL_FALSE, glm::value_ptr(rightHand.currentRotationMatrix));
		//draw main object
		robotBase.Draw(objShader);
		leftThigh.Draw(objShader);
		rightThigh.Draw(objShader);
		leftLeg.Draw(objShader);
		rightLeg.Draw(objShader);
		leftFoot.Draw(objShader);
		rightFoot.Draw(objShader);
		upperBody.Draw(objShader);
		leftArm.Draw(objShader);
		rightArm.Draw(objShader);
		leftHand.Draw(objShader);
		rightHand.Draw(objShader);


		submodel = glm::translate(model,glm::vec3(glm::sin(glfwGetTime()*1)*30, glm::sin(glfwGetTime() * 1) * 30, glm::cos(glfwGetTime() * 1)*30));
		glUniformMatrix4fv(glGetUniformLocation(objShader.Program, "submodel"), 1, GL_FALSE, glm::value_ptr(submodel));
		teapot.Draw(objShader);

		// Swap the buffers
		glfwSwapBuffers(window);

		//Until user press escape or close the window, the program should loop.
	} while (!glfwWindowShouldClose(window));

	//remove the resources used to free up memory
	

	glfwTerminate();
	return 0;
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	//checking before game start
	if (gameStart == false) {
		//If B pressed, game started.
		if (key == GLFW_KEY_B) {
			gameStart = true;
		}
	}
	//update key pressing
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
	
}
//update mouse input info to move camera (changing view and projection matrix)
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
//receive scroll information to accept zoom in and zoom out.
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

GLFWwindow* GetEverythingInitialized() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);
	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// Options, disable mouse cursor.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();
	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);
	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_PROGRAM_POINT_SIZE);
	return window;
}


#pragma endregion