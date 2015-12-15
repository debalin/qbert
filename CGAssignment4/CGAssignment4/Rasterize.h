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
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alut.h"

#define MTL_HASHMAP std::unordered_map<string, MTL *>
#define BUF_INDEX std::unordered_map<string, int>
#define MTL_TRIANGLES std::unordered_map<string, std::vector<Triangle *>>
#define MTL_INDICES std::unordered_map<string, IndexInfo *>
#define BOARD_HEIGHT 7
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define GRAVITY 0.00012f

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
		glm::vec3 initTranslate, animTranslate, animScale;
		ModelInfo *modelInfo;
		GLuint vertexArray, primitiveBuffer, indexBuffer;
		GLfloat rotateAmount;
		BUF_INDEX bufferIndex;
		MTL_TRIANGLES trianglesInMTL;
		MTL_INDICES indexInfo;
		bool stepped, display, enemyDead, movedFlag, animate, startFlag, initFlag;
		int lives, stateX, stateY, initDirection, direction, simulatedTime, sideBallTrack;
		GLfloat velocityX, velocityY, velocityZ;
		int killIntensity;
		clock_t killTime;
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

	void initShaders();
	void initFonts();
	GLuint LoadShaders(const char* vertexFilePath, const char* fragmentFilePath);
	int glInitialize();

	int parseOBJMTL(string objPath, ModelInfo *modelInfo);
	int parseInterfaceWindow(string windowPath);
	int parseEyeLocation();
	int parseLightSources(string lightPath);
	int parseBoard();
	int parseQbert();
	int parseCreatures();
	void normalizeVertices(Element *node, glm::mat4 initTransform);
	int createNormals(ModelInfo *modelInfo);

	int startGame();
	void draw(glm::mat4 viewMat, glm::mat4 projectionMat);
	void controlCreatures();
	void showMenu();
	void showGameOver();
	void checkCollisions();
	void animate();
	void writeOnScreen(string text, int startX, int startY, int sizeOfLetter, glm::vec3 color);

};

