//COMPP 371 Project

//header files
#include "stdafx.h"
#include <map>
#include <glew.h>
#include <glfw3.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <../COMP371_Lab7/objloader.hpp>
#include <../soil/SOIL.h>


#include <stdlib.h>     /* srand, rand */
#include <time.h> 
#include  <windows.h>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

// IrrKlang
#include <../irrKlang.h>
#pragma comment(lib, "irrKlang.lib")

using namespace std;
using namespace irrklang;

/**
*
*Sounds
*/
ISoundEngine *SoundEngine = createIrrKlangDevice();
ISoundEngine *BackEngine = createIrrKlangDevice();
bool soundMenu = false;
bool soundGame = false;

/**
*
*Set higher when working with fast computer
*/
float COMPSLOW = 4,
copyslow = COMPSLOW;

//gamescore
int	score = 0,
highscore = 0;

/**
*
*Car motions rights global variables and textures
*/
bool moveLeft = false,
moveRight = false,
switchScreen = false,
carTexture1 = true,
carTexture2 = false,
changeTexture = false,
changeEnemiesTexture = false;
int textureIndexEnemies = 0;
int textureIndexMainCar = 1;

/**
*Screen ENUM
*/
enum screenMode { TITLESCREEN, GAMESCREEN, CARDEATHSCREEN, COWDEATHSCREEN };
screenMode mode = TITLESCREEN;

//Textures
GLuint cow_texture;
GLuint car_texture1;
GLuint car_texture2;
GLuint car_texture3;
GLuint car_texture4;
GLuint car_texture5;

/**
*
*Main car motions frame number global variables
*/
int lguard = 0,
rguard = 0;

/**
*
*Window Dimentions
*/
const GLuint WIDTH = 1200, HEIGHT = 1000;

/**
*
*Step size
*/
const GLfloat COW_MOVEMENT_STEP = 0.5f,
CAR_MOVEMENT_STEP = 0.1f,
MAIN_CAR_MOVEMENT_STEP = .1f;

/**
*
*Camera Rotate Size CURRENTLY USED
*/
const float ANGLE_ROTATION_STEP = 0.002f;

/**
*
*Main VAO
*/
GLuint Main_CarVAO, Main_CarVerticesVBO, Main_CarNormalsVBO, Main_CarUVsVBO;

//function declaration
int randomGenerator();
void randomStart();
void randomOStart();
void carAI();
void draw(vector<glm::vec3> &, vector<glm::vec3> &, vector<glm::vec2> &);
bool checkCollision(glm::vec3 o1, glm::vec3 o2);
void setMainCarTexture();
void setEnemiesCarTexture();

/**
*
*Camera postion
*/
glm::vec3 camera_position = glm::vec3(0.0f, -2.5f, -15.0f);

/**
*
*Starting Position
*/
glm::vec3 mainCarTrans = glm::vec3(0.0, 0.0, -1.0f),
carTranslation = glm::vec3(0.0f, 0.0f, -150.0f),
cowTranslation = glm::vec3(0.0f, 0.0f, -200.0f);

/**
*
*Camera Matrix
*/
glm::mat4 projection_matrix;

/**
*
*Draw Element Matrix
*/
glm::mat4 model_matrix;

/**
*
*rotation angle of undefined NOT USED
*/
float y_rotation_angle = 0.0f, x_rotation_angle = 0.0f; // of the Main_Car

														/**
														*
														*Resize Handler
														*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	projection_matrix = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.0f); // Update the Projection matrix after a window resize event
}


/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
GLuint VAO, VBO;

void RenderText(GLuint shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

/**
*
*Key Callbacks
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		if (mainCarTrans.x <= -4)
			moveLeft = false;
		else
		{
			moveLeft = true;
			SoundEngine->play2D("audio/beep.wav", GL_FALSE);
		}
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		if (mainCarTrans.x >= 4)
			moveRight = false;
		else
		{
			moveRight = true;
			SoundEngine->play2D("audio/beep.wav", GL_FALSE);
		}
	}

	if (changeTexture == true)
	{
		//texture cars
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
			textureIndexMainCar += 1;
			if (textureIndexMainCar > 5)
				textureIndexMainCar = 5;
		}

		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			textureIndexMainCar -= 1;
			if (textureIndexMainCar < 1)
				textureIndexMainCar = 1;
		}
	}

	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		switchScreen = true;
	}


}

/**
*
*Load Shaders
*/
GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);

	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ?\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);

	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ?\n", fragment_shader_path.c_str());
		getchar();
		exit(-1);
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s", vertex_shader_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_shader_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glBindAttribLocation(ProgramID, 0, "in_Position");

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID); //free up memory
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/**
*
*Load skybox \mapping
*/
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
		);

		SOIL_free_image_data(image); //free resources
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

/**
*
*Main game loop
*/
int main()
{
	ifstream myReadFile;
	myReadFile.open("highscore.txt");
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile >> highscore;
			cout << "Highscore: " << highscore << endl;
		}
	}
	myReadFile.close();

	BackEngine->play2D("audio/gameloop.wav", GL_TRUE, GL_FALSE, GL_FALSE);

	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CarGame", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Define the viewport dimensions
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLuint carShaderProgram = loadShaders("shaders/car_vertex.shader", "shaders/car_fragment.shader");
	GLuint skyboxShaderProgram = loadShaders("shaders/skybox_vertex.shader", "shaders/skybox_fragment.shader");
	GLuint textShaderProgram = loadShaders("shaders/cube_vertex.shader", "shaders/cube_fragment.shader");


	// Set OpenGL options
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(WIDTH), 0.0f, static_cast<GLfloat>(HEIGHT));

	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	// Load font as face
	FT_Face face;
	if (FT_New_Face(ft, "fonts/font.ttf", 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load first 128 characters of ASCII set
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



	projection_matrix = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	std::vector<glm::vec3> Main_Car_vertices;
	std::vector<glm::vec3> Main_Car_normals;
	std::vector<glm::vec2> Main_Car_UVs;

	std::vector<glm::vec3> model_vertices;
	std::vector<glm::vec3> model_normals;
	std::vector<glm::vec2> model_UVs;

	std::vector<glm::vec3> obj_vertices;
	std::vector<glm::vec3> obj_normals;
	std::vector<glm::vec2> obj_UVs;

	//Object loaders 
	loadOBJ("car.obj", Main_Car_vertices, Main_Car_normals, Main_Car_UVs);
	loadOBJ("cow.obj", model_vertices, model_normals, model_UVs);
	loadOBJ("car.obj", obj_vertices, obj_normals, obj_UVs);

	glGenVertexArrays(1, &Main_CarVAO);
	glGenBuffers(1, &Main_CarVerticesVBO);
	glGenBuffers(1, &Main_CarNormalsVBO);
	glGenBuffers(1, &Main_CarUVsVBO);

	glActiveTexture(GL_TEXTURE0); //select texture unit 0

	//setup Main_Car texture

	glGenTextures(1, &car_texture1);
	glBindTexture(GL_TEXTURE_2D, car_texture1); //bind this texture to the currently bound texture unit

											   // Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load image, create texture
	int car_texture_width, car_texture_height;
	unsigned char* cube_image = SOIL_load_image("brick.jpg", &car_texture_width, &car_texture_height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, car_texture_width, car_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, cube_image);

	SOIL_free_image_data(cube_image); //free resources

	 //setup cow texture

	glGenTextures(1, &cow_texture);
	glBindTexture(GL_TEXTURE_2D, cow_texture); //bind this texture to the currently bound texture unit

											   // Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load image, create texture
	int cow_texture_width, cow_texture_height;
	unsigned char* cow_image = SOIL_load_image("cowPatternNum.jpg", &cow_texture_width, &cow_texture_height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cow_texture_width, cow_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, cow_image);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(cow_image); //free resources

									 //Car texture 2;
	glGenTextures(1, &car_texture2);
	glBindTexture(GL_TEXTURE_2D, car_texture2); //bind this texture to the currently bound texture unit

												// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load image, create texture
	int car_texture_width2, car_texture_height2;
	unsigned char* car2_image = SOIL_load_image("carPat2.jpg", &car_texture_width2, &car_texture_height2, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, car_texture_width2, car_texture_height2, 0, GL_RGB, GL_UNSIGNED_BYTE, car2_image);

	SOIL_free_image_data(car2_image); //free resources

	glGenTextures(1, &car_texture3);
	glBindTexture(GL_TEXTURE_2D, car_texture3); //bind this texture to the currently bound texture unit

												// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load image, create texture
	int car_texture_width3, car_texture_height3;
	unsigned char* car3_image = SOIL_load_image("carPat3.jpg", &car_texture_width3, &car_texture_height3, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, car_texture_width3, car_texture_height3, 0, GL_RGB, GL_UNSIGNED_BYTE, car3_image);

	SOIL_free_image_data(car3_image); //free resources


	glGenTextures(1, &car_texture4);
	glBindTexture(GL_TEXTURE_2D, car_texture4); //bind this texture to the currently bound texture unit

												// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load image, create texture
	int car_texture_width4, car_texture_height4;
	unsigned char* car4_image = SOIL_load_image("carPat4.jpg", &car_texture_width4, &car_texture_height4, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, car_texture_width4, car_texture_height4, 0, GL_RGB, GL_UNSIGNED_BYTE, car4_image);

	SOIL_free_image_data(car4_image); //free resources


	glGenTextures(1, &car_texture5);
	glBindTexture(GL_TEXTURE_2D, car_texture5); //bind this texture to the currently bound texture unit

												// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load image, create texture
	int car_texture_width5, car_texture_height5;
	unsigned char* car5_image = SOIL_load_image("carPat1.jpg", &car_texture_width5, &car_texture_height5, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, car_texture_width5, car_texture_height5, 0, GL_RGB, GL_UNSIGNED_BYTE, car5_image);

	SOIL_free_image_data(car5_image); //free resources


	std::vector<glm::vec3> skybox_vertices;
	std::vector<glm::vec3> skybox_normals; //unused
	std::vector<glm::vec2> skybox_UVs; //unused

	loadOBJ("cube.obj", skybox_vertices, skybox_normals, skybox_UVs);

	///prepare skybox VAO
	GLuint skyboxVAO, skyboxVerticesVBO;

	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &skyboxVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, skybox_vertices.size() * sizeof(glm::vec3), &skybox_vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	int mapselect = randomGenerator();
	cout <<"Map selection: ";

	//prepare skybox cubemap
	vector<const GLchar*> faces;
	faces.push_back("right.png");
	faces.push_back("left.png");
	faces.push_back("top.png");
	faces.push_back("bottom.png");
	faces.push_back("back.png");
	
	if (mapselect == 1) {
		faces.push_back("front.png");
		cout << "Hillside" << endl;
	}
	else if (mapselect == 2)
	{
		faces.push_back("front.png");
		cout << "Hillside" << endl;

	}
	else if (mapselect == 3)
	{
		faces.push_back("background4.png");
		cout << "Dust Bowl" << endl;

	}
	else if (mapselect == 4)
	{
		faces.push_back("background4.png");
		cout << "Dust Bowl" << endl;

	}
	else if (mapselect == 5)
	{
		faces.push_back("background1.jpg");
		cout << "Rainbow Road!!" << endl;

	}
	else if (mapselect == 6)
	{
		faces.push_back("background3.png");
		cout << "Desert" << endl;

	}
	else if (mapselect == 7)
	{
		faces.push_back("background3.png");
		cout << "Desert" << endl;

	}
	else if (mapselect == 8)
	{
		faces.push_back("background2.jpg");
		cout << "Countryside" << endl;

	}
	else if (mapselect == 9)
	{
		faces.push_back("background2.jpg");
		cout << "Countryside" << endl;

	}
	else
	{
		faces.push_back("front.png");
		cout << "Hillside" << endl;

	}

	//prepare title screen cubemap
	vector<const GLchar*> Tfaces;
	Tfaces.push_back("Tright.jpg");
	Tfaces.push_back("Tleft.jpg");
	Tfaces.push_back("Ttop.jpg");
	Tfaces.push_back("Tbottom.jpg");
	Tfaces.push_back("back.jpg");
	Tfaces.push_back("Tfront.jpg");


	//prepare cowdeath cubemap
	vector<const GLchar*> cowfaces;
	cowfaces.push_back("Cfront.jpg");
	cowfaces.push_back("Cfront.jpg");
	cowfaces.push_back("Cfront.jpg");
	cowfaces.push_back("Cfront.jpg");
	cowfaces.push_back("Cfront.jpg");
	cowfaces.push_back("Cfront.jpg");

	//prepare cardeath cubemap
	vector<const GLchar*> carfaces;
	carfaces.push_back("Tright.jpg");
	carfaces.push_back("Tleft.jpg");
	carfaces.push_back("Ttop.jpg");
	carfaces.push_back("Tbottom.jpg");
	carfaces.push_back("back.jpg");
	carfaces.push_back("Carfront.jpg");


	glActiveTexture(GL_TEXTURE1);
	GLuint TcubemapTexture = loadCubemap(Tfaces);
	GLuint cubemapTexture = loadCubemap(faces);
	GLuint carcubemapTexture = loadCubemap(carfaces);
	GLuint cowcubemapTexture = loadCubemap(cowfaces);


	glBindTexture(GL_TEXTURE_CUBE_MAP, TcubemapTexture);



	//references to uniforms
	GLuint projectionLoc = glGetUniformLocation(carShaderProgram, "projection_matrix");
	GLuint viewMatrixLoc = glGetUniformLocation(carShaderProgram, "view_matrix");
	GLuint transformLoc = glGetUniformLocation(carShaderProgram, "model_matrix");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


	while (!glfwWindowShouldClose(window))
	{

		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (mode == TITLESCREEN)
		{
			changeTexture = true;
			glm::mat4 view_matrix;
			view_matrix = translate(view_matrix, camera_position);

			//draw skybox
			glUseProgram(skyboxShaderProgram);
			glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix)); //remove the translation data
			glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(skybox_view));

			glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "view_matrix"), 1, GL_FALSE, glm::value_ptr(skybox_view));
			glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

			glUniform1i(glGetUniformLocation(skyboxShaderProgram, "skyboxTexture"), 1); //use texture unit 1

			glDepthMask(GL_FALSE);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, TcubemapTexture);

			glBindVertexArray(skyboxVAO);
			glDrawArrays(GL_TRIANGLES, 0, skybox_vertices.size());
			glBindVertexArray(0);

			glDepthMask(GL_TRUE);

			//ROTATE car
			y_rotation_angle -= ANGLE_ROTATION_STEP;

			//draw Main_Car
			glUseProgram(carShaderProgram);

			glm::mat4 main_car_matrix;
			main_car_matrix = glm::rotate(main_car_matrix, y_rotation_angle, glm::vec3(0.0f, 0.5f, 0.0f));
			main_car_matrix = glm::rotate(main_car_matrix, x_rotation_angle, glm::vec3(1.0f, 0.0f, 0.0f));

			model_matrix = main_car_matrix;
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));


			glActiveTexture(GL_TEXTURE0);
			setMainCarTexture();

			glUniform1i(glGetUniformLocation(carShaderProgram, "cubeTexture"), 0); //use texture unit 0

			draw(Main_Car_vertices, Main_Car_normals, Main_Car_UVs);

			glUseProgram(textShaderProgram);
			RenderText(textShaderProgram, "high score  .  " + to_string(highscore), 200.0f, 900.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			if (switchScreen == true) {
				mode = GAMESCREEN;
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
				switchScreen = false;

			}
		}
		if (mode == GAMESCREEN) {
			changeTexture = false;
			BackEngine->stopAllSounds();
			if (soundGame == false)
			{
				SoundEngine->play2D("audio/countdown.wav", GL_TRUE);
				soundGame = true;
				soundMenu = false;
			}
			glm::mat4 view_matrix;
			view_matrix = translate(view_matrix, camera_position);

			//draw skybox

			glUseProgram(skyboxShaderProgram);
			glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix)); //remove the translation data
			glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(skybox_view));

			glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "view_matrix"), 1, GL_FALSE, glm::value_ptr(skybox_view));
			glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

			glUniform1i(glGetUniformLocation(skyboxShaderProgram, "skyboxTexture"), 1); //use texture unit 1


			glDepthMask(GL_FALSE);

			glBindVertexArray(skyboxVAO);
			glDrawArrays(GL_TRIANGLES, 0, skybox_vertices.size());
			glBindVertexArray(0);

			glDepthMask(GL_TRUE);

			//draw Main_Car
			glUseProgram(carShaderProgram);

			cowTranslation.z += COW_MOVEMENT_STEP / COMPSLOW;
			carTranslation.z += CAR_MOVEMENT_STEP / COMPSLOW;

			if (cowTranslation.z - 5 > mainCarTrans.z) {
				randomStart();
				score++;

				SoundEngine->play2D("audio/moo.wav", GL_FALSE);

				if (score % 5 == 0)
					COMPSLOW = COMPSLOW / 1.25;
				//cout << score << endl;
			}
			if (carTranslation.z - 5 > mainCarTrans.z) {
				randomOStart();
				if (score % 5 == 0)
					COMPSLOW = COMPSLOW / 1.25;
				score++;
				SoundEngine->play2D("audio/honk.wav", GL_FALSE);
				//cout << score << endl;
			}

			carAI();

			if (checkCollision(carTranslation, cowTranslation)) {
				randomStart();
			}

			glm::mat4 main_car_matrix;
			main_car_matrix = glm::translate(main_car_matrix, mainCarTrans);

			glm::mat4 cow_matrix;
			cow_matrix = glm::translate(cow_matrix, cowTranslation);

			glm::mat4 car_matrix;
			car_matrix = glm::translate(car_matrix, carTranslation);


			if (moveLeft && lguard < 50)
			{
				mainCarTrans.x -= MAIN_CAR_MOVEMENT_STEP;
				lguard++;
			}
			else
			{
				lguard = 0;

				moveLeft = false;
			}
			if (moveRight && rguard < 50)
			{
				mainCarTrans.x += MAIN_CAR_MOVEMENT_STEP;
				rguard++;

			}
			else
			{
				rguard = 0;
				moveRight = false;
			}

			model_matrix = main_car_matrix;
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

			//Main Car
			setMainCarTexture();
			glUniform1i(glGetUniformLocation(carShaderProgram, "cubeTexture"), 0); //use texture unit 1
			draw(Main_Car_vertices, Main_Car_normals, Main_Car_UVs);


			//Enemies Cars
			setEnemiesCarTexture();
			model_matrix = car_matrix;
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
			draw(obj_vertices, obj_normals, obj_UVs);

			//Cow
			glBindTexture(GL_TEXTURE_2D, cow_texture);
			model_matrix = cow_matrix;
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
			draw(model_vertices, model_normals, model_UVs);

			glUseProgram(textShaderProgram);
			RenderText(textShaderProgram, "score .   " + to_string(score), 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
			glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


			if (checkCollision(cowTranslation, mainCarTrans)) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cowcubemapTexture);
				if (score > highscore)
					highscore = score;
				switchScreen = false;
				mode = COWDEATHSCREEN;
			}

			if (checkCollision(carTranslation, mainCarTrans)) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_CUBE_MAP, carcubemapTexture);
				if (score > highscore)
					highscore = score;
				switchScreen = false;
				mode = CARDEATHSCREEN;
			}

		}
		if (mode == CARDEATHSCREEN)

		{
			if (soundGame == true)
			{
				BackEngine->play2D("audio/crash.wav", GL_FALSE);
				soundGame = false;
			}
			SoundEngine->stopAllSounds();
			glm::mat4 view_matrix;
			view_matrix = translate(view_matrix, camera_position);

			//draw skybox
			glUseProgram(skyboxShaderProgram);
			glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix)); //remove the translation data
			glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(skybox_view));

			glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "view_matrix"), 1, GL_FALSE, glm::value_ptr(skybox_view));
			glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

			glUniform1i(glGetUniformLocation(skyboxShaderProgram, "skyboxTexture"), 1); //use texture unit 1

			glDepthMask(GL_FALSE);

			glBindVertexArray(skyboxVAO);
			glDrawArrays(GL_TRIANGLES, 0, skybox_vertices.size());
			glBindVertexArray(0);

			glActiveTexture(GL_TEXTURE0);
			glUseProgram(textShaderProgram);
			RenderText(textShaderProgram, "score .   " + to_string(score), 25.0f, 900.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			RenderText(textShaderProgram, "highscore .    " + to_string(highscore), 375.0f, 900.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glActiveTexture(GL_TEXTURE1);

			if (switchScreen == true) {
				mode = TITLESCREEN;
				if (soundMenu == false)
				{
					BackEngine->play2D("audio/gameloop.wav", GL_TRUE);
					soundMenu = true;
				}
				mainCarTrans = glm::vec3(0.0, 0.0, -1.0f),
					cowTranslation = glm::vec3(0.0f, 0.0f, -200.0f);
				carTranslation = glm::vec3(0.0f, 0.0f, -150.0f);
				COMPSLOW = copyslow;
				score = 0;
				glBindTexture(GL_TEXTURE_CUBE_MAP, TcubemapTexture);
				switchScreen = false;
			}

		}
		if (mode == COWDEATHSCREEN)
		{
			if (soundGame == true)
			{
				BackEngine->play2D("audio/crash.wav", GL_FALSE);
				soundGame = false;
			}
			SoundEngine->stopAllSounds();
			glm::mat4 view_matrix;
			view_matrix = translate(view_matrix, camera_position);

			//draw skybox
			glUseProgram(skyboxShaderProgram);
			glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix)); //remove the translation data
			glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(skybox_view));

			glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "view_matrix"), 1, GL_FALSE, glm::value_ptr(skybox_view));
			glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgram, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

			glUniform1i(glGetUniformLocation(skyboxShaderProgram, "skyboxTexture"), 1); //use texture unit 1

			glDepthMask(GL_FALSE);

			glBindVertexArray(skyboxVAO);
			glDrawArrays(GL_TRIANGLES, 0, skybox_vertices.size());
			glBindVertexArray(0);

			glActiveTexture(GL_TEXTURE0);
			glUseProgram(textShaderProgram);
			RenderText(textShaderProgram, "score .   " + to_string(score), 25.0f, 900.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			RenderText(textShaderProgram, "highscore .   " + to_string(highscore), 375.0f, 900.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glActiveTexture(GL_TEXTURE1);

			if (switchScreen == true) {
				mode = TITLESCREEN;
				if (soundMenu == false)
				{
					BackEngine->play2D("audio/gameloop.wav", GL_TRUE);
					soundMenu = true;
				}
				mainCarTrans = glm::vec3(0.0, 0.0, -1.0f),
					cowTranslation = glm::vec3(0.0f, 0.0f, -200.0f);
				carTranslation = glm::vec3(0.0f, 0.0f, -150.0f);
				COMPSLOW = copyslow;
				score = 0;
				glBindTexture(GL_TEXTURE_CUBE_MAP, TcubemapTexture);
				switchScreen = false;
			}

		}

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}
	ofstream myfile;
	myfile.open("highscore.txt");
	myfile << to_string(highscore);
	myfile.close();
	glfwTerminate();

	return 0;
}

/**
*
*Load the right matrix to draw into the shaders
*/
void draw(vector<glm::vec3> &vertices, vector<glm::vec3> &normals, vector<glm::vec2> &uvs) {

	glBindVertexArray(Main_CarVAO);


	glBindBuffer(GL_ARRAY_BUFFER, Main_CarVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);


	glBindBuffer(GL_ARRAY_BUFFER, Main_CarNormalsVBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, Main_CarUVsVBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(2);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

/**
*
*initialize random seed:
*/
int randomGenerator() {
	srand(time(NULL));
	return  rand() % 9 + 1;
}

/**
*
*Generate random start position
*/
void randomStart() {

	int randPosition = randomGenerator();

	if (randPosition > 0 && randPosition < 4)
	{
		cowTranslation = glm::vec3(-5.0f, 0.0f, -100.0f);
	}
	else if (randPosition > 3 && randPosition < 7)
	{
		cowTranslation = glm::vec3(0.0f, 0.0f, -100.0f);
	}
	else if (randPosition > 6 && randPosition < 10)
	{
		cowTranslation = glm::vec3(5.0f, 0.0f, -100.0f);
	}

}

/**
*
*Generate random start position
*/
void randomOStart() {

	int randPosition = randomGenerator();

	if (randPosition > 0 && randPosition < 2)
	{
		carTranslation = glm::vec3(-5.0f, 0.0f, -100.0f);
		textureIndexEnemies = 0;
	}
	else if (randPosition > 1 && randPosition < 4)
	{
		carTranslation = glm::vec3(0.0f, 0.0f, -100.0f);
		textureIndexEnemies = 1;
	}
	else if (randPosition > 3 && randPosition < 6)
	{
		carTranslation = glm::vec3(5.0f, 0.0f, -100.0f);
		textureIndexEnemies = 2;
	}
	else if (randPosition > 5 && randPosition < 8)
	{
		carTranslation = glm::vec3(0.0f, 0.0f, -100.0f);
		textureIndexEnemies = 3;
	}
	else if (randPosition > 7 && randPosition < 10)
	{
		carTranslation = glm::vec3(5.0f, 0.0f, -100.0f);
		textureIndexEnemies = 4;
	}
}

/**
*
*check Collisions
*/
bool checkCollision(glm::vec3 obj1, glm::vec3 obj2) {
	if (abs(obj1.x - obj2.x) <= 2 && obj1.z >= obj2.z)
		return true;
	else
		return false;
}

void RenderText(GLuint shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	glActiveTexture(GL_TEXTURE0);
	glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);

	glBindVertexArray(VAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
AI random movement
**/
void carAI() {
	int randPosition = randomGenerator();
	if (randPosition > 0 && randPosition <= 1)
	{
		if (carTranslation.x > -6) {
			carTranslation.x -= 0.1;
		}
	}
	else if (randPosition > 1 && randPosition <= 3)
	{
		if (carTranslation.x < 6) {
			carTranslation.x += 0.1;
		}
	}
	else if (randPosition > 3 && randPosition < 10) {
		carTranslation.x += 0;
	}
}

//Set main car texture
void setMainCarTexture() {
	glActiveTexture(GL_TEXTURE0);
	if (textureIndexMainCar == 1) {
		glBindTexture(GL_TEXTURE_2D, car_texture2);
	}
	else if (textureIndexMainCar == 2) {
		glBindTexture(GL_TEXTURE_2D, car_texture1);
	}
	else if (textureIndexMainCar == 3) {
		glBindTexture(GL_TEXTURE_2D, car_texture3);
	}
	else if (textureIndexMainCar == 4) {
		glBindTexture(GL_TEXTURE_2D, car_texture4);
	}
	else if (textureIndexMainCar == 5) {
		glBindTexture(GL_TEXTURE_2D, car_texture5);
	}
}

//Set enemies car texture
void setEnemiesCarTexture() {
	if (textureIndexEnemies == 0) {
		glBindTexture(GL_TEXTURE_2D, car_texture2);
	}
	else if (textureIndexEnemies == 1) {
		glBindTexture(GL_TEXTURE_2D, car_texture1);
	}
	else if (textureIndexEnemies == 2) {
		glBindTexture(GL_TEXTURE_2D, car_texture3);
	}
	else if (textureIndexEnemies == 3) {
		glBindTexture(GL_TEXTURE_2D, car_texture4);
	}
	else if (textureIndexEnemies == 4) {
		glBindTexture(GL_TEXTURE_2D, car_texture5);
	}
}