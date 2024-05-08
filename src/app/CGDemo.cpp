#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <filesystem>
#include <GL/glew.h>					
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "MeshData.hpp"
#include "MeshGLData.hpp"
#include "GLSetup.hpp"
#include "Shader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Utility.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
namespace fs = std::filesystem;

struct PointLight {
	glm::vec4 pos;
	glm::vec4 color;
};

PointLight light;

float roundAngle = 0;
float LIGHT_SCALE = 5.0f;

float rotAngle = 0;
glm::vec3 eye = glm::vec3(0,3,5);
glm::vec3 lookAt = glm::vec3(0,0,0);
glm::vec2 mousePos;

float metallic = 0.0f;
float roughness = 0.1f;

bool doImageCheck = true;
const int MAX_TEXTURE_CNT = 20;

void cleanupTextureIDs(vector<unsigned int> &allTexIDs) {
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

	for(int i = 0; i < allTexIDs.size(); i++) {	
	    glDeleteTextures(1, &(allTexIDs.at(i)));
	}
	allTexIDs.clear();
}

void createTextureIDs(vector<unsigned int> &allTexIDs, int desiredCnt) {

	GLenum format = GL_RGBA;
	int twidth = 256;
	int theight = 256;

	for(int i = 0; i < desiredCnt; i++) {	
		unsigned int textureID = 0;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, twidth, theight, 0, format, 
						GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		allTexIDs.push_back(textureID);
	}
}

int updateTextures(vector<unsigned int> &allTexIDs) {

	cout << "Checking textures again..." << endl;

	string path = "./face_images";
	int texIndex = 0;

	if(fs::exists(path)) {		
		for(const auto & entry : fs::directory_iterator(path)) {
			cout << "\t" << entry.path() << endl;

			int twidth, theight, tnumc;
			stbi_set_flip_vertically_on_load(1);
			unsigned char* tex_image = stbi_load(entry.path().string().c_str(), &twidth, &theight, &tnumc, 0);

			if(tex_image) {
				
				GLenum format;
				if(tnumc == 3) {
					format = GL_RGB;
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				}
				else if(tnumc == 4) {
					format = GL_RGBA;
				}
				
				// Get texture ID and bind it
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, allTexIDs.at(texIndex));

				// Copy in image data
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, twidth, theight, format, GL_UNSIGNED_BYTE, tex_image);
				
				// Clean up data
				stbi_image_free(tex_image);

				// Increment index
				texIndex++;
			}
		}
	}

	// Unbind everything for now
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

	// How many did we find?
	return texIndex;
}

void updateModelMatrices(vector<glm::mat4> &allModelMats, int desiredCnt) {
	if(desiredCnt != allModelMats.size()) {
		allModelMats.clear();

		float radius = 3.0f;
		float angleInc = (float)(M_PI*2.0/desiredCnt);

		for(int i = 0; i < desiredCnt; i++) {
			float angle = angleInc*i;
			float x = radius*sin(angle);
			float y = 0.0f;
			float z = -radius*cos(angle);

			allModelMats.push_back(glm::translate(glm::vec3(x,y,z))*glm::rotate(-angle, glm::vec3(0,1,0)));			
		}
	}
}

glm::mat4 makeRotateZ(glm::vec3 offset) {
	glm::mat4 T = glm::translate(-offset);
	glm::mat4 R = glm::rotate(glm::radians(rotAngle), glm::vec3(0,0,1));
	glm::mat4 B = glm::translate(offset);
	return B*R*T;
}


static void key_callback(GLFWwindow *window,
                        int key,
                        int scancode,
                        int action,
                        int mods) {
    if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        if(key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }
        else if(key == GLFW_KEY_J) {
            rotAngle += 1.0;
        }
        else if(key == GLFW_KEY_K) {
            rotAngle -= 1.0;
        }
		else if(key == GLFW_KEY_W) {
			glm::vec3 D = lookAt - eye;
			D *= 0.1f;
			lookAt += D;
			eye += D;
		}
		else if(key == GLFW_KEY_S) {
			glm::vec3 D = lookAt - eye;
			D *= 0.1f;
			lookAt -= D;
			eye -= D;
		}
		else if(key == GLFW_KEY_D) {
			glm::vec3 W = -(lookAt - eye);
			glm::vec3 localX = glm::cross(glm::vec3(0,1,0), W);
			localX *= 0.1f;
			lookAt += localX;
			eye += localX;
		}
		else if(key == GLFW_KEY_A) {
			glm::vec3 W = -(lookAt - eye);
			glm::vec3 localX = glm::cross(glm::vec3(0,1,0), W);
			localX *= 0.1f;
			lookAt -= localX;
			eye -= localX;
		}
		else if(key == GLFW_KEY_1) {
			light.color = glm::vec4(LIGHT_SCALE,LIGHT_SCALE,LIGHT_SCALE,1);
		}
		else if(key == GLFW_KEY_2) {
			light.color = glm::vec4(LIGHT_SCALE,0,0,1);
		}
		else if(key == GLFW_KEY_3) {
			light.color = glm::vec4(0,LIGHT_SCALE,0,1);
		}
		else if(key == GLFW_KEY_4) {
			light.color = glm::vec4(0,0,LIGHT_SCALE,1);
		}

		else if(key == GLFW_KEY_V) {
			metallic -= 0.1f;
			metallic = max(0.0f, metallic);
		}
		else if(key == GLFW_KEY_B) {
			metallic += 0.1f;
			metallic = min(1.0f, metallic);
		}

		else if(key == GLFW_KEY_N) {
			roughness -= 0.1f;
			roughness = max(0.1f, roughness);
		}
		else if(key == GLFW_KEY_M) {
			roughness += 0.1f;
			roughness = min(0.7f, roughness);
		}
		else if(key == GLFW_KEY_T) {
			roundAngle += 0.1f;
		}
		else if(key == GLFW_KEY_Y) {
			roundAngle -= 0.1f;
		}
		else if(key == GLFW_KEY_SPACE) {
			doImageCheck = true;
		}
	}
}
	
// Create very simple mesh: a quad (4 vertices, 6 indices, 2 triangles)
void createSimpleQuad(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create four corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;
	Vertex extra;

	// Set positions of vertices
	// Note: glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);

	extra.position = glm::vec3(-0.9, 0.0, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);

	extra.color = glm::vec4(0.0, 1.0, 1.0, 1.0);

	// Add to mesh's list of vertices
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	m.vertices.push_back(extra);
	
	// Add indices for two triangles
	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(4);
}

void extractMeshData(aiMesh *mesh, Mesh &m) {
	m.vertices.clear();
	m.indices.clear();

	for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vert;
		vert.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vert.color = glm::vec4(1,1,0,1);
		vert.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		float u = vert.position.x + 0.5f;
		float v = vert.position.y;
		vert.texcoord = glm::vec2(u,v);
		
		m.vertices.push_back(vert);
	}

	for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace f = mesh->mFaces[i];
		for(unsigned int j = 0; j < f.mNumIndices; j++) {
			m.indices.push_back(f.mIndices[j]);
		}
	}
}

void renderScene(
		MeshGL mgl, 
		vector<glm::mat4> &allModelMats, 
		GLint modelMatLoc, 
		GLint normMatLoc,
		glm::mat4 viewMat,
		vector<unsigned int> &allTexIDs) {
	
	// For each model matrix...
	for(int i = 0; i < allModelMats.size(); i++) {
		glm::mat4 modelMat = allModelMats[i];		
		glm::mat4 R = makeRotateZ(glm::vec3(modelMat[3]));
		glm::mat4 globalRotate = glm::rotate(roundAngle, glm::vec3(0,1,0));
		glm::mat4 tmpModel = globalRotate*R*modelMat;
		glUniformMatrix4fv(modelMatLoc, 1, false,
								glm::value_ptr(tmpModel));

		glm::mat3 normMat = glm::transpose(glm::inverse(glm::mat3(viewMat*tmpModel)));
		glUniformMatrix3fv(normMatLoc, 1, false, glm::value_ptr(normMat));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, allTexIDs.at(i));
	
		drawMesh(mgl);
	}
}

glm::mat4 makeLocalRotate(glm::vec3 offset, glm::vec3 axis, float angle) {
	glm::mat4 T = glm::translate(-offset);
	glm::mat4 R = glm::rotate(glm::radians(angle), axis);
	glm::mat4 B = glm::translate(offset);
	return B*R*T;
}

static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
	glm::vec2 diffMouse = mousePos - glm::vec2(xpos, ypos);
	int fw, fh;
	glfwGetFramebufferSize(window, &fw, &fh);
	if(fw > 0 && fh > 0) {
		diffMouse.x /= (float)fw;
		diffMouse.y /= (float)fh;

		glm::mat4 RX = makeLocalRotate(eye, glm::vec3(0,1,0), 30.0f*diffMouse.x);
		glm::vec3 camDir = lookAt - eye;
		glm::vec3 localX = glm::cross(glm::vec3(0,1,0), -camDir);
		glm::mat4 RY = makeLocalRotate(eye, localX, 30.0f*diffMouse.y);

		glm::vec4 lookAtV = glm::vec4(lookAt, 1.0);
		lookAtV = RY * RX * lookAtV;
		lookAt = glm::vec3(lookAtV);

		mousePos = glm::vec2(xpos, ypos);
	}
}

// Main 
int main(int argc, char **argv) {

	string modelpath = "./sampleModels/teapot.obj";

	if(argc > 1) {
		modelpath = argv[1];
	}	

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(modelpath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		cerr << "Error: " << importer.GetErrorString() << endl;
		exit(1);
	}

	if(scene->mNumMeshes < 1) {
		cerr << "Error: not enough meshes!" << endl;
		exit(1);
	}

	// Are we in debugging mode?
	bool DEBUG_MODE = true;

	// GLFW setup
	GLFWwindow* window = setupGLFW("CGDemo", 4, 3, 800, 800, DEBUG_MODE);

	// GLEW setup
	setupGLEW(window);

	double mx, my;
	glfwGetCursorPos(window, &mx, &my);
	mousePos = glm::vec2(mx, my);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Check OpenGL version
	checkOpenGLVersion();

	// Set up debugging (if requested)
	if(DEBUG_MODE) checkAndSetupOpenGLDebugging();

	// Set the background color to a shade of blue
	glClearColor(0.0f, 0.7f, 0.0f, 1.0f);	

	// Create and load shader
	GLuint programID = 0;
	try {		
		// Load vertex shader code and fragment shader code
		string vertexCode = readFileToString("./shaders/CGDemo/Basic.vs");
		string fragCode = readFileToString("./shaders/CGDemo/Basic.fs");

		// Print out shader code, just to check
		if(DEBUG_MODE) printShaderCode(vertexCode, fragCode);

		// Create shader program from code
		programID = initShaderProgramFromSource(vertexCode, fragCode);
	}
	catch (exception e) {		
		// Close program
		cleanupGLFW(window);
		exit(EXIT_FAILURE);
	}

	GLint modelMatLoc = glGetUniformLocation(programID, "modelMat");
	GLint viewMatLoc = glGetUniformLocation(programID, "viewMat");
	GLint projMatLoc = glGetUniformLocation(programID, "projMat");	
	GLint normMatLoc = glGetUniformLocation(programID, "normMat");
	GLint lightPosLoc = glGetUniformLocation(programID, "light.pos");
	GLint lightColorLoc = glGetUniformLocation(programID, "light.color");
	GLint roughnessLoc = glGetUniformLocation(programID, "roughness");
	GLint metallicLoc = glGetUniformLocation(programID, "metallic");
	GLint textureLoc = glGetUniformLocation(programID, "diffuseTex");

	// Set up light details
	light.pos = glm::vec4(5, 5, 5, 1.0);
	light.color = glm::vec4(LIGHT_SCALE,LIGHT_SCALE,LIGHT_SCALE,1);
		
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Load a single model	
	Mesh m;
	MeshGL mgl;
	extractMeshData(scene->mMeshes[0], m);
	createMeshGL(m, mgl);
	
	// Prepare list of model matrices
	vector<glm::mat4> allModelMats;
	allModelMats.push_back(glm::mat4(1.0f));

	// Create a list of textures
	vector<unsigned int> allTexIDs;
	createTextureIDs(allTexIDs, MAX_TEXTURE_CNT);

	int frameIndex = 0;

	// Main render loop
	while (!glfwWindowShouldClose(window)) {
		// Check image directory
		if(doImageCheck) {
			int textureCnt = updateTextures(allTexIDs);
			updateModelMatrices(allModelMats, textureCnt);
			doImageCheck = false;
		}

		// Set viewport size
		int fwidth, fheight;
		glfwGetFramebufferSize(window, &fwidth, &fheight);
		glViewport(0, 0, fwidth, fheight);

		// Clear the framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use shader program
		glUseProgram(programID);

		glm::mat4 viewMat = glm::lookAt(eye, lookAt, glm::vec3(0,1,0));
		glUniformMatrix4fv(viewMatLoc, 1, false, glm::value_ptr(viewMat));

		glm::vec4 lightPos = viewMat*light.pos;
		glUniform4fv(lightPosLoc, 1, glm::value_ptr(lightPos));
		glUniform4fv(lightColorLoc, 1, glm::value_ptr(light.color));

		glUniform1f(roughnessLoc, roughness);
		glUniform1f(metallicLoc, metallic);

		int fw, fh;
		glfwGetFramebufferSize(window, &fw, &fh);
		float aspectRatio = 1.0;
		if(fw > 0 && fh > 0) {
			aspectRatio = ((float)fw)/((float)fh);
		}

		glm::mat4 projMat = glm::perspective(glm::radians(90.0f), aspectRatio, 0.01f, 50.0f);
		glUniformMatrix4fv(projMatLoc, 1, false, glm::value_ptr(projMat));

		// Set which active texture we want
		glUniform1i(textureLoc, 0);

		// Draw scene		
		renderScene(mgl, allModelMats, modelMatLoc, normMatLoc, viewMat, allTexIDs);

		// Swap buffers and poll for window events		
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Sleep for 15 ms
		this_thread::sleep_for(chrono::milliseconds(15));

		// Increment index
		frameIndex++;
	}

	// Cleanup texture IDs
	cleanupTextureIDs(allTexIDs);

	// Clean up mesh
	cleanupMesh(mgl);

	// Clean up shader programs
	glUseProgram(0);
	glDeleteProgram(programID);
		
	// Destroy window and stop GLFW
	cleanupGLFW(window);

	return 0;
}
