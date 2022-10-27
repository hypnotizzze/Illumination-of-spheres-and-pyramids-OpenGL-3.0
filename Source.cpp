#define GLEW_STATIC
#include <iostream>
#include <filesystem>
#include <GL/glew.h> // GLEW
#include <GLFW/glfw3.h> // GLFW
#include <Windows.h>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "V.h"
#include "Camera.h"
#include "Model.h"

#define PI 3.14159265


int color = 0, lab = 1, direct = 1;
string s_model, s_pyramid, s_cube, p;

int WIDTH = 1000, HEIGHT = 800;
GLfloat lastX = WIDTH/2, lastY = HEIGHT/2, radius = 2.0f, radius_2 = 3.0f, speed = 0.0f;

float scale_fil = 0.1f, scale_yellow = 0.1f;

double x = 0, z = 0;

bool keys[1024], mouses[10];
bool firstMouse = true;

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void do_movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

glm::vec3 objectColor = glm::vec3(1.0f, 1.0, 1.0f); // default object color

// Position of light
glm::vec3 lightPos(sin(radius * PI / 180), 1.0f, radius * cos(z * PI / 180)); // sph 1, 2

glm::vec3 lightColor = glm::vec3(1.0f, 0.0f, 0.0f); // sph 1, 2

glm::vec3 sph_clr_1 = glm::vec3(1.0f, 0.0f, 1.0f); // 3_1 color

glm::vec3 sph_clr_2 = glm::vec3(1.0f, 1.0f, 0.0f); // 3_2 color

glm::vec3 sph_3_1 = glm::vec3(2.0f, 1.0f, -3.0f); // sph 3_1

glm::vec3 sph_3_2 = glm::vec3(-2.0f, 1.0f, -3.0f); // sph 3_2

glm::vec3 sph_pos = glm::vec3(3.0f, 1.5f, 0.0f); // sph pos in 4th lab

glm::mat4 model = glm::mat4(1.0f);

glm::mat4 pyram_model = glm::mat4(1.0f);

glm::mat4 cube_model = glm::mat4(1.0f);

int main()
{
	std::filesystem::path cwd = std::filesystem::current_path();
	p = cwd.string();
	std::string::iterator iter = std::find(p.begin(), p.end(), '\\');
	if (iter < p.end()) {
		std::replace(++iter, p.end(), '\\', '/');
	}
	//Инициализация GLFW
	if (!glfwInit())
		return -1;
	//Настройка GLFW
	//Задается минимальная требуемая версия OpenGL. 
	//Мажорная 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//Минорная
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//Установка профайла для которого создается контекст
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Выключение возможности изменения размера окна
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(1000, 800, "LearnOpenGL", nullptr, nullptr);

	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	//	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	GLint nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

	Shader_2 ourShader("vertex_1.txt", "fragment_light.txt"); //1, 2
	Shader_2 myShader("vertex_1.txt", "fragment_1.txt"); // 1,2

	Shader_2 filShader("vertex_1.txt", "fragment_fil.txt"); // 3
	Shader_2 yellowShader("vertex_1.txt", "fragment_yellow.txt");
	Shader_2 pyramShader("vertex_1.txt", "fragment_pyram.txt");

//	Shader_2 CubeShader("vertex_1.txt", "fragment_1.txt"); // 1 variant
	Shader_2 CubeShader("vertex_cube_test.txt", "fragment_cube_test.txt"); // 2 variant
	s_model = p + "/sphere/globe-sphere.obj";
	s_pyramid = p + "/pyramid/pyramid-advance-design.obj";
	s_cube = p + "/cube2/cube.obj";
	cout << s_model << "\n";
	Model ourModel(s_model);
	Model pyramid(s_pyramid);
	//Model Cube("C:/GL/Project4/cube/chamfer-cube.obj"); // 1 variant
	Model Cube(s_cube); // 2 variant

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	 
	pyram_model = glm::scale(pyram_model, glm::vec3(0.1f, 0.1f, 0.1f));

	cube_model = glm::scale(cube_model, glm::vec3(0.1f, 0.1f, 0.1f));

	while (!glfwWindowShouldClose(window))
	{
		GLfloat value_time = glfwGetTime();
		deltaTime = value_time - lastFrame;
		lastFrame = value_time; 
		glfwPollEvents(); // Проверка вызова события
		do_movement();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		if (lab == 1) {
			ourShader.use(); // light sph 1 - 1, 2 zadanie
			ourShader.setMat4("projection", projection);
			ourShader.setMat4("view", view);
			model = glm::translate(model, lightPos);
			ourShader.setMat4("model", model);
			ourShader.setVec3("Color", lightColor);
			ourModel.Draw(ourShader);

			myShader.use(); //lighting sph 1
			glm::vec3 lightPos_2 = glm::vec3(lightPos.x, lightPos.y, lightPos.z);
			myShader.setVec3("objectColor", objectColor);
			myShader.setVec3("lightColor", lightColor);
			myShader.setVec3("lightPos", lightPos_2);
			model = glm::mat4(1.0f);
			myShader.setMat4("projection", projection);
			myShader.setMat4("view", view);
			myShader.setMat4("model", model);
			ourModel.Draw(myShader);
		}

		else if (lab == 2) {
			model = glm::mat4(1.0f); // sph 3_1
			filShader.use();
			filShader.setMat4("projection", projection);
			filShader.setMat4("view", view);
			model = glm::translate(model, sph_3_1);
			filShader.setMat4("model", model);
			filShader.setFloat("Intense", scale_fil);
			ourModel.Draw(filShader);

			model = glm::mat4(1.0f); // sph 3_2
			yellowShader.use();
			yellowShader.setMat4("projection", projection);
			yellowShader.setMat4("view", view);
			model = glm::translate(model, sph_3_2);
			yellowShader.setMat4("model", model);
			yellowShader.setFloat("Intense", scale_yellow);
			ourModel.Draw(yellowShader);

			pyramShader.use(); // pyram
			pyramShader.setFloat("f1", scale_fil);
			pyramShader.setFloat("f2", scale_yellow);
			glm::vec3 objectColor = glm::vec3(1.0f, 1.0, 1.0f);
			pyramShader.setVec3("objectColor", objectColor);

			glm::vec3 lightPos_s1 = glm::vec3(sph_3_1.x, sph_3_1.y, sph_3_1.z); // 3_1
			pyramShader.setVec3("lightColor_1", sph_clr_1);
			pyramShader.setVec3("lightPos_1", lightPos_s1);

			glm::vec3 lightPos_s2 = glm::vec3(sph_3_2.x, sph_3_2.y, sph_3_2.z); // 3_2
			pyramShader.setVec3("lightColor_2", sph_clr_2);
			pyramShader.setVec3("lightPos_2", lightPos_s2);

			pyramShader.setMat4("projection", projection);
			pyramShader.setMat4("view", view);
			pyramShader.setMat4("model", pyram_model);
			pyramid.Draw(pyramShader);
		}

		else if (lab == 3) { 
			if (x >= 361)
				x -= 361;
			if (z >= 361)
				z -= 361;
			x += speed;
			z += speed;
			sph_pos = glm::vec3(sin(x  * PI / 180) * radius_2, 1.5f, cos(z  * PI / 180) * radius_2);
			model = glm::mat4(1.0f); // sph 3_2
			yellowShader.use();
			yellowShader.setMat4("projection", projection);
			yellowShader.setMat4("view", view);
			model = glm::translate(model, sph_pos);
			yellowShader.setMat4("model", model);
			yellowShader.setFloat("Intense", 1.0f);
			ourModel.Draw(yellowShader);

			CubeShader.use(); //lighting sph 2
			glm::vec3 lightPos_sph = glm::vec3(sph_pos.x, sph_pos.y, sph_pos.z);
			CubeShader.setVec3("objectColor", objectColor);
			CubeShader.setVec3("lightColor", sph_clr_2);
			CubeShader.setVec3("lightPos", lightPos_sph);
			CubeShader.setMat4("projection", projection);
			CubeShader.setMat4("view", view);
			CubeShader.setMat4("model", cube_model);
			Cube.Draw(CubeShader);
		}

		glfwSwapBuffers(window); // Смена местами буффера
	}
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
	if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
		scale_yellow += 0.05f;
		if (scale_yellow >= 1.0f)
			scale_yellow = 0.1f;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		scale_fil += 0.05f;
		if (scale_fil >= 1.0f)
			scale_fil = 0.1f;
	}
}

void do_movement()
{
	// Camera controls
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_Z])
		lab = 1;
	if (keys[GLFW_KEY_X])
		lab = 2;
	if (keys[GLFW_KEY_C])
		lab = 3;
	if (keys[GLFW_KEY_RIGHT]) {
		if (color == 1000)
		  color = 0;
		else
			color += 1;
		lightColor = glm::vec3(1.0f, 0.0f, color * 0.0005f);
		}
	if (keys[GLFW_KEY_UP]) {
		speed += 0.01f;
	}
	if (keys[GLFW_KEY_DOWN]) {
		speed -= 0.01f;
	}
	if (mouses[GLFW_MOUSE_BUTTON_1]) {
		speed = 0.0f;
		if (x < 360)
			x++;
		else
			x = 0;
		if (z < 360)
			z++;
		else
			z = 0; 
		lightPos = glm::vec3(radius * sin(x * PI / 180), 1.0f, radius * cos(z * PI / 180));
	}
	if (mouses[GLFW_MOUSE_BUTTON_2]) {
		speed = 0.0f;
		if (x >= 0 )
			x--;
		else
			x = 360;
		if (z >= 0)
			z--;
		else
			z = 360;
		lightPos = glm::vec3(radius * sin(x * PI / 180), 1.0f, radius * cos(z * PI / 180));
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button >= 0 && button < 10)
	{
		if (action == GLFW_PRESS)
			mouses[button] = true;
		else if (action == GLFW_RELEASE)
			mouses[button] = false;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}