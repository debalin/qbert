#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <string>
#include <iomanip>
#include <unordered_map>
using namespace std;
#include "glew/glew.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "soil/src/SOIL.h"
using namespace glm;
#include <al.h>
#include <alc.h>
#include <alut/alut.h>

#define MTL_HASHMAP std::unordered_map<string, MTL *>
#define BUF_INDEX std::unordered_map<string, int>
#define MTL_TRIANGLES std::unordered_map<string, std::vector<Triangle *>>
#define MTL_INDICES std::unordered_map<string, IndexInfo *>
#define BOARD_HEIGHT 7
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

class Rasterize {

public:
	struct ShaderInfo {
		GLuint programId;
	};
	struct Triangle {
		int vertexIndices[3], normalIndices[3], textureIndices[3];
		unsigned int bufferIndices[3];
		glm::vec3 faceNormal;
		string groupName;
	};
	struct Light {
		glm::vec3 location, la, ls, ld;
	};
	struct MTL {
		glm::vec3 ka, ks, kd;
		float N;
		string texturePath;
		GLuint textureID;
	};
	struct ModelInfo {
		std::vector<glm::vec3> verticesVec, normalsVec;
		std::vector<glm::vec2> textureVec;
		std::vector<Triangle *> triangles;
		GLfloat maxX, maxY, maxZ, minX, minY, minZ;
		MTL_HASHMAP materials;
		bool vnPresent, vtPresent, useVt;
		ModelInfo() {
			vnPresent = false, vtPresent = false, useVt = false;
			maxX = -999999, maxY = -999999, maxZ = -999999, minX = 999999, minY = 999999, minZ = 999999;
		}
	};
	struct IndexInfo {
		int startIndex, size;
		IndexInfo(int startIndexInp, int sizeInp) {
			startIndex = startIndexInp;
			size = sizeInp;
		}
	};
	struct Primitive {
		glm::vec3 vertex, normal;
		glm::vec2 textureUV;
	};
	struct Element {
		string objPath;
		glm::mat4 initTransform, transform;
		glm::vec3 initTranslate;
		ModelInfo *modelInfo;
		GLuint vertexArray, primitiveBuffer, indexBuffer;
		BUF_INDEX bufferIndex;
		MTL_TRIANGLES trianglesInMTL;
		MTL_INDICES indexInfo;
		bool stepped, display, enemyDead, movedFlag;
		int lives;
		int stateX, stateY, initDirection, direction;
		GLfloat velocityX, velocityY;
	};
	struct FontInfo {
		GLuint fontTextureID, fontArray, fontUVBufferID, fontVertexBufferID;
	};

	std::vector<Light *> lights;
	glm::vec3 eyeLocation;
	float nearZ;
	bool smoothingOpt, lightSwitch;
	ShaderInfo info;
	FontInfo fontInfo;

	Rasterize();
	~Rasterize();
	int parseOBJMTL(string objPath, ModelInfo *modelInfo);
	int parseInterfaceWindow(string windowPath);
	int parseEyeLocation();
	int parseLightSources(string lightPath);
	void normalizeVertices(Element *node, glm::mat4 initTransform);
	int createNormals(ModelInfo *modelInfo);
	int startGame();
	GLuint LoadShaders(const char* vertexFilePath, const char* fragmentFilePath);
	void initShaders();
	void draw(glm::mat4 viewMat, glm::mat4 projectionMat);
	int glInitialize();
	int parseBoard();
	int parseQbert();
	int parseCreatures();
	void controlCreatures();
	void showMenu();
	void checkCollisions();
	int buildElement(Element *node, glm::mat4 parentTransform, std::ifstream &inputFile);
	void initFonts();
	void writeOnScreen(string text, int startX, int startY, int sizeOfLetter, glm::vec3 color);

};

