#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <random>
#include <array>
#include <algorithm>

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
Camera camera(glm::vec3(0.0f, 0.0f, 11.0f));  // far enough to frame the 5-sphere row

// Lighting
glm::vec3 lightPos = glm::vec3(2.0f, 4.0f, -2.0f);
glm::vec3 lightColor = glm::vec3(0.2f, 0.2f, 0.7f);

// variables
bool isGPressed = false;
bool isHPressed = false;
bool isMPressed = false;
bool isBPressed = false;
bool gammaCorrectionEnabled = true;
bool hdrEnabled = true;

// scenes, switched with M
const char* sceneNames[] = { "Spheres", "Damaged Helmet", "Cerberus" };
const int sceneCount = sizeof(sceneNames) / sizeof(sceneNames[0]);
int currentScene = 0;

// IBL environments, switched with B
const char* hdrPaths[] = {
	"resources/imgs/hdr/newport_loft.hdr",
	"resources/imgs/hdr/brown_photostudio_02_2k.hdr",
	"resources/imgs/hdr/venice_sunset_2k.hdr",
	"resources/imgs/hdr/kloofendal_48d_partly_cloudy_puresky_2k.hdr",
	"resources/imgs/hdr/industrial_sunset_puresky_2k.hdr",
};
const int envCount = sizeof(hdrPaths) / sizeof(hdrPaths[0]);
int currentEnv = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// true only on the frame the key goes down, so holding it doesn't toggle every frame
bool keyPressedOnce(GLFWwindow* window, int key, bool& isPressed) {
	bool down = glfwGetKey(window, key) == GLFW_PRESS;
	bool pressedNow = down && !isPressed;
	isPressed = down;
	return pressedNow;
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

	if (keyPressedOnce(window, GLFW_KEY_G, isGPressed)) {
		gammaCorrectionEnabled = !gammaCorrectionEnabled;
		std::cout << "Gamma Correction Enabled: " << (gammaCorrectionEnabled ? "True" : "False") << std::endl;
	}

	if (keyPressedOnce(window, GLFW_KEY_H, isHPressed)) {
		hdrEnabled = !hdrEnabled;
		std::cout << "HDR Tone Mapping Enabled: " << (hdrEnabled ? "True" : "False") << std::endl;
	}

	if (keyPressedOnce(window, GLFW_KEY_M, isMPressed)) {
		currentScene = (currentScene + 1) % sceneCount;
		std::cout << "Scene: " << sceneNames[currentScene] << std::endl;
	}

	if (keyPressedOnce(window, GLFW_KEY_B, isBPressed)) {
		currentEnv = (currentEnv + 1) % envCount;
		std::cout << "Environment: " << hdrPaths[currentEnv] << std::endl;
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

float lerp(float a, float b, float f) {
	return a + f * (b - a);
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
	std::cout << "HDR Tone Mapping Enabled: " << (hdrEnabled ? "True" : "False") << std::endl;
	std::cout << "M: switch scene, B: switch environment" << std::endl;

	// ===== glfw initialize and configure =====
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	stbi_set_flip_vertically_on_load(true);

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
	glDepthFunc(GL_LEQUAL);  // skybox depth is pinned to 1.0, so it must pass against the cleared depth
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);  // filter across cube face edges, otherwise the blurry prefilter mips show seams

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

	// sphere VAO
	// ----------
	std::vector<float> sphereData;
	std::vector<unsigned int> sphereIndices;
	const unsigned int X_SEGMENTS = 64;
	const unsigned int Y_SEGMENTS = 64;
	const float PI = 3.14159265359f;
	for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			// position, normal (unit sphere: same as position), texcoord
			sphereData.insert(sphereData.end(), {xPos, yPos, zPos, xPos, yPos, zPos, xSegment, ySegment});
		}
	}
	bool oddRow = false;
	for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
		if (!oddRow) {
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
				sphereIndices.push_back(y * (X_SEGMENTS + 1) + x);
				sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		} else {
			for (int x = X_SEGMENTS; x >= 0; --x) {
				sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				sphereIndices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}
	unsigned int sphereIndexCount = static_cast<unsigned int>(sphereIndices.size());

	unsigned int sphereVAO, sphereVBO, sphereEBO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glGenBuffers(1, &sphereEBO);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereData.size() * sizeof(float), sphereData.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// load PBR material textures
	// ==========================
	// albedo is linearized in the shader (pow 2.2), so load everything without sRGB
	// 1x1 single-value texture, stands in for maps a material doesn't ship with
	auto createSolidTexture = [](unsigned char value) {
		unsigned int id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &value);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // default filter expects mipmaps and would sample black
		return id;
	};
	unsigned int whiteTexture = createSolidTexture(255);  // ao: no occlusion
	unsigned int blackTexture = createSolidTexture(0);    // emissive: no glow

	// maps in sampler unit order: albedo, normal, metallic, roughness, ao, emissive
	using MaterialMaps = std::array<unsigned int, 6>;
	auto loadMaterial = [&](const std::string& dir) -> MaterialMaps {
		return {
			loadTexture((dir + "/albedo.png").c_str(), false),
			loadTexture((dir + "/normal.png").c_str(), false),
			loadTexture((dir + "/metallic.png").c_str(), false),
			loadTexture((dir + "/roughness.png").c_str(), false),
			loadTexture((dir + "/ao.png").c_str(), false),
			blackTexture,
		};
	};
	std::vector<MaterialMaps> sphereMaterials = {
		{
			loadTexture("resources/imgs/rustediron/rustediron2_basecolor.png", false),
			loadTexture("resources/imgs/rustediron/rustediron2_normal.png", false),
			loadTexture("resources/imgs/rustediron/rustediron2_metallic.png", false),
			loadTexture("resources/imgs/rustediron/rustediron2_roughness.png", false),
			loadTexture("resources/imgs/rustediron/ao.png", false),
			blackTexture,
		},
		loadMaterial("resources/imgs/gold"),
		loadMaterial("resources/imgs/grass"),
		loadMaterial("resources/imgs/plastic"),
		loadMaterial("resources/imgs/wall"),
	};

	// Model flips UVs on import, so the models' textures below must be loaded unflipped
	Model gun("cerberus/cerberus.fbx");
	Model helmet("damaged_helmet/DamagedHelmet.gltf");

	stbi_set_flip_vertically_on_load(false);
	MaterialMaps gunMaterial = {
		loadTexture("resources/models/cerberus/cerberus_A.png", false),
		loadTexture("resources/models/cerberus/cerberus_N.png", false),
		loadTexture("resources/models/cerberus/cerberus_M.png", false),
		loadTexture("resources/models/cerberus/cerberus_R.png", false),
		whiteTexture,  // the gun ships without an AO map
		blackTexture,
	};
	// glTF packs metallic (B) and roughness (G) into one image; they were split into the two grayscale files read here
	MaterialMaps helmetMaterial = {
		loadTexture("resources/models/damaged_helmet/Default_albedo.jpg", false),
		loadTexture("resources/models/damaged_helmet/Default_normal.jpg", false),
		loadTexture("resources/models/damaged_helmet/Default_metallic.png", false),
		loadTexture("resources/models/damaged_helmet/Default_roughness.png", false),
		loadTexture("resources/models/damaged_helmet/Default_AO.jpg", false),
		loadTexture("resources/models/damaged_helmet/Default_emissive.jpg", false),
	};
	stbi_set_flip_vertically_on_load(true);

	auto bindMaterial = [](const MaterialMaps& maps) {
		for (int i = 0; i < 6; ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, maps[i]);
		}
	};

	// Setup UBO
	// =========
	unsigned int uboMatrices;
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);

	// build and compile shaders
	// -------------------------
	Shader pbrShader("pbr_specularIBL.vs", "pbr_specularIBL.fs");
	pbrShader.use();
	pbrShader.setInt("albedoMap", 0);
	pbrShader.setInt("normalMap", 1);
	pbrShader.setInt("metallicMap", 2);
	pbrShader.setInt("roughnessMap", 3);
	pbrShader.setInt("aoMap", 4);
	pbrShader.setInt("emissiveMap", 5);
	pbrShader.setInt("irradianceMap", 6);
	pbrShader.setInt("prefilterMap", 7);
	pbrShader.setInt("brdfLUT", 8);
	unsigned int uniformBlockIndex = glGetUniformBlockIndex(pbrShader.ID, "Matrices");
	glUniformBlockBinding(pbrShader.ID, uniformBlockIndex, 0);

	Shader backgroundShader("skybox.vs", "background.fs");
	backgroundShader.use();
	backgroundShader.setInt("environmentMap", 0);

	// IBL: equirectangular map -> environment cubemap -> irradiance + prefiltered specular cubemaps
	// =============================================================================================
	auto createCubemap = [](int size) {
		unsigned int id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		return id;
	};

	// a 90 degree camera at the origin looking at each cube face
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] = {
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// draws the inside of the cube once per face into the cubemap;
	// no depth buffer needed: seen from the center the faces never overlap
	unsigned int captureFBO;
	glGenFramebuffers(1, &captureFBO);
	auto renderToCubemap = [&](Shader& shader, unsigned int cubemap, int size, int mip) {
		shader.use();
		shader.setMat4("projection", captureProjection);
		glViewport(0, 0, size, size);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < 6; ++i) {
			shader.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, mip);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	};

	Shader equirectToCubemapShader("skybox.vs", "equirectangular_to_cubemap.fs");
	equirectToCubemapShader.use();
	equirectToCubemapShader.setInt("equirectangularMap", 0);

	Shader irradianceShader("skybox.vs", "irradiance_convolution.fs");
	irradianceShader.use();
	irradianceShader.setInt("environmentMap", 0);

	Shader prefilterShader("skybox.vs", "pbr_specularIBL_prefilter.fs");
	prefilterShader.use();
	prefilterShader.setInt("environmentMap", 0);

	struct Environment {
		unsigned int envCubemap;
		unsigned int irradianceMap;
		unsigned int prefilterMap;
	};
	std::vector<Environment> environments;
	const int prefilterMipLevels = 5;  // matches MAX_REFLECTION_LOD = 4 in pbr_specularIBL.fs

	for (const char* hdrPath : hdrPaths) {
		int hdrWidth, hdrHeight, hdrComponents;
		float* hdrData = stbi_loadf(hdrPath, &hdrWidth, &hdrHeight, &hdrComponents, 3);
		unsigned int hdrTexture;
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		if (hdrData) {
			// the sun in some skies is brighter than GL_RGB16F can hold (65504); stored as Inf it turns
			// every blurred IBL sample into NaN, which renders black
			for (int i = 0; i < hdrWidth * hdrHeight * 3; ++i)
				hdrData[i] = std::min(hdrData[i], 65504.0f);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, hdrWidth, hdrHeight, 0, GL_RGB, GL_FLOAT, hdrData);
		} else {
			std::cout << "Failed to load HDR image: " << hdrPath << std::endl;
		}
		stbi_image_free(hdrData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Environment env;
		env.envCubemap = createCubemap(512);
		env.irradianceMap = createCubemap(32);
		env.prefilterMap = createCubemap(128);
		// each mip of the prefilter map stores a blurrier reflection for a higher roughness
		glBindTexture(GL_TEXTURE_CUBE_MAP, env.prefilterMap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);  // allocates the mip chain, filled below

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		renderToCubemap(equirectToCubemapShader, env.envCubemap, 512, 0);

		// the prefilter pass reads blurrier mips for rarely sampled directions, which removes bright dots
		glBindTexture(GL_TEXTURE_CUBE_MAP, env.envCubemap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		renderToCubemap(irradianceShader, env.irradianceMap, 32, 0);

		for (int mip = 0; mip < prefilterMipLevels; ++mip) {
			prefilterShader.use();
			prefilterShader.setFloat("roughness", (float)mip / (float)(prefilterMipLevels - 1));
			renderToCubemap(prefilterShader, env.prefilterMap, 128 >> mip, mip);
		}

		glDeleteTextures(1, &hdrTexture);
		environments.push_back(env);
	}

	// IBL: BRDF integration LUT, only depends on (NdotV, roughness) so one texture serves every environment
	// ===================================================================================================
	unsigned int brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	Shader brdfShader("hdr.vs", "pbr_specularIBL_brdf.fs");  // hdr.vs is a plain fullscreen-quad pass-through
	brdfShader.use();
	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// only needed for the one-time precompute above
	glDeleteFramebuffers(1, &captureFBO);

	// lights (shader loops over 4; unused slots stay black)
	// ------
	glm::vec3 lightPositions[] = {
		glm::vec3(0.0f, 0.0f, 10.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(150.0f, 150.0f, 150.0f),
	};
	float spacing = 2.2f;

	// lights are static, so upload them once (glUniform* targets the bound program, and the IBL passes above switched it)
	pbrShader.use();
	for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) {
		pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
		pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
	}

	// the models' own materials point at files we don't use (missing .tga, packed glTF maps),
	// so skip Mesh::Draw's texture binding and draw the VAOs directly
	auto drawModel = [&](Model& m, const MaterialMaps& maps, const glm::mat4& model) {
		bindMaterial(maps);
		pbrShader.setMat4("model", model);
		for (Mesh& mesh : m.meshes) {
			glBindVertexArray(mesh.VAO);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
		}
	};

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// time
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// clear
		// -----
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// set uniforms
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		pbrShader.use();
		pbrShader.setVec3("camPos", camera.Position);
		pbrShader.setBool("gammaEnabled", gammaCorrectionEnabled);
		pbrShader.setBool("hdrEnabled", hdrEnabled);

		const Environment& env = environments[currentEnv];
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, env.irradianceMap);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_CUBE_MAP, env.prefilterMap);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

		if (currentScene == 2) {
			// the FBX stands on its barrel axis (Y) in centimeters: lay it flat with Z as up, scale to ~8 units and center it
			model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.05f));
			model = glm::translate(model, glm::vec3(3.7f, 52.2f, 14.2f));
			drawModel(gun, gunMaterial, model);
		} else if (currentScene == 1) {
			// Model ignores node transforms: redo the glTF node's 90 degree X rotation that stands the Z-up mesh upright
			model = glm::scale(glm::mat4(1.0f), glm::vec3(2.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			drawModel(helmet, helmetMaterial, model);
		} else {
			// one sphere per material in a row, centered on the origin
			glBindVertexArray(sphereVAO);
			int sphereCount = static_cast<int>(sphereMaterials.size());
			for (int i = 0; i < sphereCount; ++i) {
				bindMaterial(sphereMaterials[i]);
				model = glm::translate(glm::mat4(1.0f), glm::vec3((i - sphereCount / 2) * spacing, 0.0f, 0.0f));
				pbrShader.setMat4("model", model);
				glDrawElements(GL_TRIANGLE_STRIP, sphereIndexCount, GL_UNSIGNED_INT, 0);
			}
		}

		// render skybox last so it only fills pixels the spheres left empty
		backgroundShader.use();
		backgroundShader.setMat4("projection", projection);
		backgroundShader.setMat4("view", glm::mat4(glm::mat3(view)));  // drop translation
		backgroundShader.setBool("gammaEnabled", gammaCorrectionEnabled);
		backgroundShader.setBool("hdrEnabled", hdrEnabled);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, env.envCubemap);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// swap buffers and poll IO events (keys pressed/released, mouse move etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &sphereVBO);
	glDeleteBuffers(1, &sphereEBO);

	// ===== terminate, clearing all previously allocated glfw resources =====
	glfwTerminate();

	return 0;
}
