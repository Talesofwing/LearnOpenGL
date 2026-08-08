#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "stb_image.h"

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Global Variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
bool firstMouse = true;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

// Lighting
glm::vec3 lightPos(3.0f, 0.5f, 1.0f);
glm::vec3 lightColor(0.3f);

// variables
bool isGPressed = false;
bool gammaCorrectionEnabled = true;
bool isBPressed = false;
bool bloomEnabled = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !isGPressed) {
		gammaCorrectionEnabled = !gammaCorrectionEnabled;
		isGPressed = true;

		std::cout << "Gamma Correction Enabled: " << (gammaCorrectionEnabled ? "True" : "False") << std::endl;
	}

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
		isGPressed = false;
	}


	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !isBPressed) {
		bloomEnabled = !bloomEnabled;
		isBPressed = true;

		std::cout << "Bloom Enabled: " << (bloomEnabled ? "True" : "False") << std::endl;
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
		isBPressed = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

float getValue() {
	float time = glfwGetTime();
	return (sin(time) / 2.0f) + 0.5f;
}

unsigned int loadTexture(char const* path, bool gammaCorrection) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format = GL_RGB;
		GLenum internalFormat = GL_SRGB;
		if (nrComponents == 1) {
			internalFormat = format = GL_RED;
		} else if (nrComponents == 3) {
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			format = GL_RGB;
		} else if (nrComponents == 4) {
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	} else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		} else {
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

float cubeVertices[] = {
	// back face
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
	 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
	// front face
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
	 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	// left face
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	// right face
	 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
	 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
	 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
	 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
	 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
	 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
	// bottom face
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
	 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	// top face
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
	 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
};

float planeVertices[] = {
	// positions            // normals         // texcoords
	 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
	-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
	-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

	 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
	-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
	 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
};

float quadVertices[84];

void calculateQuadTBN() {
	glm::vec3 p1(-1.0f,  1.0f, 0.0f);
	glm::vec3 p2( 1.0f,  1.0f, 0.0f);
	glm::vec3 p3( 1.0f, -1.0f, 0.0f);
	glm::vec3 p4(-1.0f, -1.0f, 0.0f);

	glm::vec2 uv1(0.0f, 1.0f);
	glm::vec2 uv2(1.0f, 1.0f);
	glm::vec2 uv3(1.0f, 0.0f);
	glm::vec2 uv4(0.0f, 0.0f);

	glm::vec3 n(0.0f, 0.0f, 1.0f);

	glm::vec3 t1, b1;
	glm::vec3 t2, b2;

	// triangle 1
	// ==========
	glm::vec3 edge1 = p2 - p1;
	glm::vec3 edge2 = p3 - p1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	t1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	t1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	t1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	b1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	b1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	b1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

	// triangle 2
	// ==========
	edge1 = p3 - p1;
	edge2 = p4 - p1;
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv4 - uv1;

	f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	t2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	t2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	t2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	b2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	b2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	b2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

	const float vertexData[] = {
		// position         // normal        // texcoord     // tangent        // bitangent
		p1.x, p1.y, p1.z,   n.x, n.y, n.z,   uv1.x, uv1.y,   t1.x, t1.y, t1.z, b1.x, b1.y, b1.z,
		p3.x, p3.y, p3.z,   n.x, n.y, n.z,   uv3.x, uv3.y,   t1.x, t1.y, t1.z, b1.x, b1.y, b1.z,
		p2.x, p2.y, p2.z,   n.x, n.y, n.z,   uv2.x, uv2.y,   t1.x, t1.y, t1.z, b1.x, b1.y, b1.z,

		p1.x, p1.y, p1.z,   n.x, n.y, n.z,   uv1.x, uv1.y,   t2.x, t2.y, t2.z, b2.x, b2.y, b2.z,
		p4.x, p4.y, p4.z,   n.x, n.y, n.z,   uv4.x, uv4.y,   t2.x, t2.y, t2.z, b2.x, b2.y, b2.z,
		p3.x, p3.y, p3.z,   n.x, n.y, n.z,   uv3.x, uv3.y,   t2.x, t2.y, t2.z, b2.x, b2.y, b2.z
	};

	std::copy(std::begin(vertexData), std::end(vertexData), quadVertices);
}

int main() {
	std::cout << "Gamma Correction Enabled: " << (gammaCorrectionEnabled ? "True" : "False") << std::endl;
	std::cout << "Bloom Correction Enabled: " << (bloomEnabled ? "True" : "False") << std::endl;

	// ===== glfw initialize and configure =====
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	stbi_set_flip_vertically_on_load(false);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GGLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	GLint vertexLimit;
	GLint fragmentLimit;
	GLint maxAttributes;

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &vertexLimit);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &fragmentLimit);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes);

	std::cout << "Vertex uniform components: " << vertexLimit << '\n';
	std::cout << "Fragment uniform components: " << fragmentLimit << '\n';
	std::cout << "Vertex Attribute Max Count: " << maxAttributes << '\n';

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// configure mouse state
	// ---------------------
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// lighting info
	// -------------
	// positions
	std::vector<glm::vec3> lightPositions;
	lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f));
	lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
	lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
	lightPositions.push_back(glm::vec3(-0.8f, 2.4f, -1.0f));
	// colors
	std::vector<glm::vec3> lightColors;
	lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
	lightColors.push_back(glm::vec3(10.0f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 0.0f, 15.0f));
	lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f));

	// build vertex array object
	// =========================
	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// quad VAO
	calculateQuadTBN();
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);  // pos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);  // normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);  // texcoord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);  // tangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);  // bitangent
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	glBindVertexArray(0);
	// plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// load textures
	// -------------
	unsigned int cubeTexture = loadTexture("resources/imgs/wood.png", false);			// don't gamma correction
	unsigned int srgbCubeTexture = loadTexture("resources/imgs/wood.png", true);
	unsigned int woodTexture = loadTexture("resources/imgs/wood.png", false);
	unsigned int srgbWoodTexture = loadTexture("resources/imgs/wood.png", true);
	unsigned int brickTexture = loadTexture("resources/imgs/brickwall.jpg", false);
	unsigned int srgbBrickTexture = loadTexture("resources/imgs/brickwall.jpg", true);
	unsigned int brickNormalMap = loadTexture("resources/imgs/brickwall_normal.jpg", false);
	unsigned int brick2Texture = loadTexture("resources/imgs/bricks2.jpg", false);
	unsigned int srgbBrick2Texture = loadTexture("resources/imgs/bricks2.jpg", true);
	unsigned int brick2NormalMap = loadTexture("resources/imgs/bricks2_normal.jpg", false);
	unsigned int brick2DisplacementMap = loadTexture("resources/imgs/bricks2_disp.jpg", false);
	unsigned int containerTexture = loadTexture("resources/imgs/container.jpg", false);
	unsigned int srgbContainerTexture = loadTexture("resources/imgs/container.jpg", true);

	// configure about shadow mapping
	// ==============================
	const unsigned int DEPTH_MAP_WIDTH = 1024, DEPTH_MAP_HEIGHT = 1024;

	unsigned int depthFBO;
	glGenFramebuffers(1, &depthFBO);

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, DEPTH_MAP_WIDTH, DEPTH_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float near_plane = 1.0f; float far_plane = 25.0f;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), DEPTH_MAP_WIDTH / (float)DEPTH_MAP_HEIGHT, near_plane, far_plane);
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

	// configure floating point framebuffer
	unsigned int hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	unsigned int colorBuffers[2];
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}

	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, attachments);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ping-pong-framebuffer for blurring
	unsigned int pingpongFBO[2];
	unsigned int pingpongColorbuffers[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

	// Load model
	// ==========
	//Model backpack("backpack/backpack.obj", gammaCorrectionEnabled);
	Model cyborg("cyborg/cyborg.obj", gammaCorrectionEnabled);

	// Wireframe draw
	// ==============
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// build and compile shaders
	// -------------------------
	Shader colorShader("color.vs", "color.fs");
	Shader lightingShader("lighting.vs", "lighting.fs");
	Shader bloomShader("lighting_bloom.vs", "lighting_bloom.fs");
	Shader blurShader("blur.vs", "blur.fs");
	Shader bloomFinalShader("bloom.vs", "bloom.fs");
	Shader modelShader("model.vs", "model.fs");

	lightingShader.use();
	lightingShader.setInt("diffuseTexture", 0);

	bloomShader.use();
	bloomShader.setInt("diffuseTexture", 0);
	bloomShader.setFloat("threshold", 1.0f);

	colorShader.use();
	colorShader.setFloat("threshold", 1.0f);

	blurShader.use();
	blurShader.setInt("image", 0);

	modelShader.use();
	modelShader.setInt("texture_diffuse1", 0);
	modelShader.setInt("texture_specular1", 1);
	modelShader.setInt("texture_normal1", 2);
	modelShader.setVec3("lightPos", lightPos);

	bloomFinalShader.use();
	bloomFinalShader.setFloat("exposure", 1.0f);
	bloomFinalShader.setInt("scene", 0);
	bloomFinalShader.setInt("bloomBlur", 1);

	// Setup UBO
	// =========
	unsigned int uniformBlockIndex = glGetUniformBlockIndex(colorShader.ID, "Matrices");
	glUniformBlockBinding(colorShader.ID, uniformBlockIndex, 0);
	uniformBlockIndex = glGetUniformBlockIndex(lightingShader.ID, "Matrices");
	glUniformBlockBinding(lightingShader.ID, uniformBlockIndex, 0);
	uniformBlockIndex = glGetUniformBlockIndex(modelShader.ID, "Matrices");
	glUniformBlockBinding(modelShader.ID, uniformBlockIndex, 0);

	uniformBlockIndex = glGetUniformBlockIndex(bloomShader.ID, "Matrices");
	glUniformBlockBinding(bloomShader.ID, uniformBlockIndex, 0);

	unsigned int uboMatrices;
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// clear
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// set uniforms
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// draw
		// ====
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		// scene
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			bloomShader.use();
			glActiveTexture(GL_TEXTURE0);
			for (unsigned int i = 0; i < lightPositions.size(); i++) {
				bloomShader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
				bloomShader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
			}
			bloomShader.setVec3("viewPos", camera.Position);
			// render
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
			bloomShader.setMat4("model", model);
			glBindTexture(GL_TEXTURE_2D, gammaCorrectionEnabled ? srgbWoodTexture : woodTexture);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			glBindTexture(GL_TEXTURE_2D, gammaCorrectionEnabled ? srgbContainerTexture : containerTexture);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
			model = glm::scale(model, glm::vec3(0.5f));
			bloomShader.setMat4("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
			model = glm::scale(model, glm::vec3(0.5f));
			bloomShader.setMat4("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
			model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
			bloomShader.setMat4("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
			model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
			model = glm::scale(model, glm::vec3(1.25));
			bloomShader.setMat4("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
			model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
			bloomShader.setMat4("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
			model = glm::scale(model, glm::vec3(0.5f));
			bloomShader.setMat4("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			// finally show all the light sources as bright cubes
			colorShader.use();

			for (unsigned int i = 0; i < lightPositions.size(); i++) {
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(lightPositions[i]));
				model = glm::scale(model, glm::vec3(0.25f));
				colorShader.setMat4("model", model);
				colorShader.setVec3("color", lightColors[i]);
				glBindVertexArray(cubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
			}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. blur
		glDisable(GL_DEPTH_TEST);
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 10;
		blurShader.use();
		for (unsigned int i = 0; i < amount; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			blurShader.setInt("horizontal", horizontal);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);

		// 3. final render
		glDisable(GL_DEPTH_TEST);
		bloomFinalShader.use();
		bloomFinalShader.setBool("gammaCorrectionEnabled", gammaCorrectionEnabled);
		bloomFinalShader.setBool("bloomEnabled", bloomEnabled);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		// ==========================================

		// swap buffers and poll IO events (keys pressed/released, mouse move etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &planeVBO);

	// ===== terminate, clearing all previously allocated glfw resources =====
	glfwTerminate();

	return 0;
}
