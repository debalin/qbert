#include "Rasterize.h"

/*********************************************************************************************************************
* Q*bert!
*********************************************************************************************************************/

int windowHeight, windowWidth, fittingWidth;
std::vector<Rasterize::Element *> nodes;
double frameTime, framesPerSec;
int stateX, stateY, score = 0, lives = 3, cubeCount = 0, level = 1; 
std::vector<std::vector<Rasterize::Element *> *> grid;
std::vector<Rasterize::Element *> qbertLives;
Rasterize::Element *qbert;
GLuint pVariable, vVariable, tVariable, textPresentVar, textureIDVar, kaVar, kdVar, ksVar, NVar, fontVar, cVar;
ALuint sourceHop, bufferHop, sourceFall, bufferFall, sourceStart, bufferStart;
GLFWwindow* window;
GLfloat scaleCubeAnim, rotateCubeAnim;
bool alive = true, pause = false, levelOver = false;
void moveQbert(int movementDir);
void initSound();
void resetLevel1();

int Rasterize::glInitialize() {

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW!\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, "Q*bert", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; 
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW.\n");
		return -1;
	}

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 

	info.programId = NULL;

	return 0;

}

Rasterize::~Rasterize() {

	alDeleteSources(1, &sourceHop);
    alDeleteBuffers(1, &bufferHop);

    alutExit();

}

Rasterize::Rasterize() {

	nearZ = 0.1f;
	smoothingOpt = true;
	lightSwitch = true;
	baseDir = "files/";
	initSound();
	clock_t time = clock();
	if (parseInterfaceWindow("files/window.txt") != 0) {
		cout << "Error in reading interface window height and width. Default values of 512x512 taken." << endl;
		windowHeight = 512;
		windowWidth = 512;
	}
	if (glInitialize() != 0) {
		cout << "Problem in initializing OpenGL." << endl;
		system("PAUSE");
		exit(1);
	}
	if (parseBoard() != 0) {
		cout << "Error occurred while creating board." << endl;
		system("PAUSE");
		exit(1);
	}
	if (parseQbert() != 0) {
		cout << "Error occurred while creating Qbert." << endl;
		system("PAUSE");
		exit(1);
	}
	if (parseEyeLocation() != 0) {
		cout << "Error in reading eye location. Default values of [0, 0, 2] taken." << endl;
		eyeLocation.x = 0;
		eyeLocation.y = 0;
		eyeLocation.z = 2;
	}
	if (parseLightSources("files/lights.txt") != 0) {
		cout << "Error in reading light sources. Default values of [0, 5, 0] and [1, 1, 1] taken." << endl;
		Light *light = new Light();
		light->location.x = 0.0;
		light->location.y = 5.0;
		light->location.z = 0.0;
		light->la = glm::vec3(1.0f);
		light->ls = glm::vec3(1.0f);
		light->ld = glm::vec3(1.0f);
		lights.push_back(light);
	}
	time = clock() - time;
	double seconds = time / (double)1000;
	cout << "Parsing took " << seconds << " seconds." << endl << endl;

}

bool Rasterize::compareNoCase(string first, string second) {

	if (first.size() != second.size()) {
		return (false);
	}
	else {
		for (size_t i = 0; i < first.size(); i++) {
			if (tolower(first[i]) != tolower(second[i])) {
				return (false);
			}
		}
	}
	return (true);

}

void changeWindow(GLFWwindow *window, int width, int height) {

	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, windowWidth, windowHeight);

}

void keyPressed(GLFWwindow *window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS && key == GLFW_KEY_UP) {
		alSourcePlay(sourceHop);
		moveQbert(UP);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_DOWN) {
		alSourcePlay(sourceHop);
		moveQbert(DOWN);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_LEFT) {
		alSourcePlay(sourceHop);
		moveQbert(LEFT);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_RIGHT) {
		alSourcePlay(sourceHop);
		moveQbert(RIGHT);
	}

}

int Rasterize::startGame() {

	glm::vec3 lookAtVectorTemp(0.0f), eyeLocationTemp(0.0f);
	glm::mat4 translateViewMatToOrg = glm::translate(glm::mat4(1.0f), -eyeLocation);
	glm::mat4 translateViewMatBack = glm::translate(glm::mat4(1.0f), eyeLocation);
	glm::mat4 viewMat, projectionMat;
	GLfloat rotateX = 0.0f, rotateY = 0.0f, rotateZ = 0.0f;
	glm::vec3 translate(0.0f);
	glm::mat4 translateMat(1.0f), rotateMatX(1.0f), rotateMatY(1.0f), rotateMat(1.0f);
	
	initShaders();

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetFramebufferSizeCallback(window, changeWindow);
	glfwSetKeyCallback(window, keyPressed);

	double lastTime = glfwGetTime(), currentTime;
	int frames = 0;
	alSourcePlay(sourceStart);

	do {
		 projectionMat = glm::ortho(- windowWidth * 0.80f / (float)fittingWidth, windowWidth * 0.80f / (float)fittingWidth, - windowHeight / (float)fittingWidth, windowHeight / (float)fittingWidth, 100.0f, -100.0f);
		viewMat = glm::mat4(1.0f);

		currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
			frameTime = 1000.0 / double(frames);
			framesPerSec = frames;
			cout << "FPS - " << framesPerSec << endl;
			frames = 0;
			lastTime += 1.0;
		}

		draw(viewMat, projectionMat);
		
		glfwSwapBuffers(window);
		frames++;

		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	glfwTerminate();

	return 0;

}

void Rasterize::initShaders() {
	
	int numOfLights;
	GLuint lightLocVariable;
	GLuint lightLaVariable;
	GLuint lightLdVariable;
	GLuint lightLsVariable;
	GLuint numLightsVariable;
	GLuint lightSwitchVariable;
	std::vector<glm::vec3> vertices;

	info.programId = LoadShaders("shaders/vs.glsl", "shaders/fs.glsl");
	glUseProgram(info.programId);

	numOfLights = (lights.size() > 5) ? 5 : lights.size();

	for (int i = 0; i < numOfLights; i++) {
		string lightLocName = "lightLocation[" + to_string(i) + "]";
		string lightAttName1 = "lightLa[" + to_string(i) + "]";
		string lightAttName2 = "lightLd[" + to_string(i) + "]";
		string lightAttName3 = "lightLs[" + to_string(i) + "]";
		lightLocVariable = glGetUniformLocation(info.programId, lightLocName.c_str());
		lightLaVariable = glGetUniformLocation(info.programId, lightAttName1.c_str());
		lightLdVariable = glGetUniformLocation(info.programId, lightAttName2.c_str());
		lightLsVariable = glGetUniformLocation(info.programId, lightAttName3.c_str());
		glUniform3fv(lightLocVariable, 1, &lights[i]->location.x);
		glUniform3fv(lightLaVariable, 1, &lights[i]->la.x);
		glUniform3fv(lightLdVariable, 1, &lights[i]->ld.x);
		glUniform3fv(lightLsVariable, 1, &lights[i]->ls.x);
	}

	pVariable = glGetUniformLocation(info.programId, "projection");
	vVariable = glGetUniformLocation(info.programId, "view");
	tVariable = glGetUniformLocation(info.programId, "transform");

	textPresentVar = glGetUniformLocation(info.programId, "texturePresent");
	textureIDVar = glGetUniformLocation(info.programId, "textureID");
	kaVar = glGetUniformLocation(info.programId, "kaVar");
	kdVar = glGetUniformLocation(info.programId, "kdVar");
	ksVar = glGetUniformLocation(info.programId, "ksVar");
	NVar = glGetUniformLocation(info.programId, "NVar");
	fontVar = glGetUniformLocation(info.programId, "fontPresent");
	cVar = glGetUniformLocation(info.programId, "colorTextVar");

	numLightsVariable = glGetUniformLocation(info.programId, "numOfLights");
	lightSwitchVariable = glGetUniformLocation(info.programId, "lightSwitch");
	glUniform1i(numLightsVariable, numOfLights);
	if (lightSwitch)
		glUniform1i(lightSwitchVariable, 1);
	else 
		glUniform1i(lightSwitchVariable, 0);

	for (Element *node : nodes) {
		glGenVertexArrays(1, &node->vertexArray);
		glBindVertexArray(node->vertexArray);

		if (compareNoCase(node->objPath, "NOOBJ")) {
			continue;
		}

		string key;
		ModelInfo *modelInfo = node->modelInfo;
		unsigned int count = 0;
		std::vector<Primitive> primitivesToDraw;
	
		for (Triangle *triangle : modelInfo->triangles) {
			MTL_TRIANGLES::const_iterator it1 = node->trianglesInMTL.find(triangle->groupName);
			if (it1 != node->trianglesInMTL.end()) {
				node->trianglesInMTL[triangle->groupName].push_back(triangle);
			}
			else {
				std::vector<Triangle *> temp;
				temp.push_back(triangle);
				node->trianglesInMTL[triangle->groupName] = temp;
			}
			for (int i = 0; i < 3; i++) {
				key = (modelInfo->useVt) ? to_string(triangle->vertexIndices[i]) + "/" + to_string(triangle->normalIndices[i]) + "/" + to_string(triangle->textureIndices[i]) : to_string(triangle->vertexIndices[i]) + "/" + to_string(triangle->normalIndices[i]);
				BUF_INDEX::const_iterator it = node->bufferIndex.find(key);
				if (it != node->bufferIndex.end()) {
					triangle->bufferIndices[i] = node->bufferIndex[key];
				}
				else {
					triangle->bufferIndices[i] = count;
					node->bufferIndex[key] = count;
					if (modelInfo->useVt) {
						Primitive primitive;
						primitive.vertex = node->modelInfo->verticesVec[triangle->vertexIndices[i]]; 
						primitive.normal = node->modelInfo->normalsVec[triangle->normalIndices[i]]; 
						primitive.textureUV = node->modelInfo->textureVec[triangle->textureIndices[i]];
						primitivesToDraw.push_back(primitive);
					}
					else {
						Primitive primitive;
						primitive.vertex = node->modelInfo->verticesVec[triangle->vertexIndices[i]]; 
						primitive.normal = node->modelInfo->normalsVec[triangle->normalIndices[i]]; 
						primitive.textureUV = glm::vec2(-1, -1);
						primitivesToDraw.push_back(primitive);
						vertices.push_back(node->modelInfo->verticesVec[triangle->vertexIndices[i]]);
					}
					count++;
				}
			}
		}

		glGenBuffers(1, &node->primitiveBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, node->primitiveBuffer);
		glBufferData(GL_ARRAY_BUFFER, primitivesToDraw.size() * sizeof(Primitive), &primitivesToDraw[0].vertex.x, GL_STATIC_DRAW);

		std::vector<GLuint> indices;
		count = 0;

		for (MTL_TRIANGLES::const_iterator it = node->trianglesInMTL.begin(); it != node->trianglesInMTL.end(); it++) {
			std::vector<Triangle *> triangles = it->second;
			IndexInfo *indexInfo = new IndexInfo(count, (triangles.size() * 3));
			node->indexInfo[it->first] = indexInfo;
			for (Triangle * triangle : triangles) {
				indices.push_back(triangle->bufferIndices[0]);
				indices.push_back(triangle->bufferIndices[1]);
				indices.push_back(triangle->bufferIndices[2]);
			}
			count += triangles.size() * 3;
		}

		glGenBuffers(1, &node->indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
	}

	initFonts();

}

void Rasterize::initFonts() {

	int width, height;
	glGenTextures(1, &fontInfo.fontTextureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fontInfo.fontTextureID);
	unsigned char *texture = SOIL_load_image("files/font.bmp", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenVertexArrays(1, &fontInfo.fontArray);
	glBindVertexArray(fontInfo.fontArray);

	glGenBuffers(1, &fontInfo.fontVertexBufferID);
	glGenBuffers(1, &fontInfo.fontUVBufferID);

}

void Rasterize::draw(glm::mat4 viewMat, glm::mat4 projectionMat) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!alive) {
		pause = true;
		alive = true;
		glfwSetKeyCallback(window, NULL);
		scaleCubeAnim = 1.0f, rotateCubeAnim = 0.0f;
	}
	else if (pause) {
		ALint stateFall, stateStart;
		alGetSourcei(sourceFall, AL_SOURCE_STATE, &stateFall);
		alGetSourcei(sourceStart, AL_SOURCE_STATE, &stateStart);
		if (stateFall == AL_PLAYING || stateStart == AL_PLAYING) {
			if (stateStart == AL_PLAYING) {
				scaleCubeAnim -= 0.0018f;
				rotateCubeAnim += 1.0f;
				for (int i = 0; i < BOARD_HEIGHT; i++) {
					for (int j = 0; j <= i; j++) {
						Rasterize::Element *cube = grid.at(i)->at(j);
						cube->transform = glm::translate(glm::mat4(1.0f), cube->initTranslate) * glm::scale(glm::mat4(1.0f), glm::vec3(scaleCubeAnim, scaleCubeAnim, scaleCubeAnim)) * glm::rotate(glm::mat4(1.0f), rotateCubeAnim, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), -cube->initTranslate);
						float rand1 = rand() / (float)RAND_MAX, rand2 = rand() / (float)RAND_MAX, rand3 = rand() / (float)RAND_MAX;
						cube->modelInfo->materials["top"]->ka = glm::vec3(rand1, rand3, rand2);
						cube->modelInfo->materials["top"]->kd = glm::vec3(rand2, rand1, rand3);
						cube->modelInfo->materials["top"]->ks = glm::vec3(rand3, rand2, rand1);
					}
				}
			}
		}
		else {
			pause = false;
			if (lives > 0 && !levelOver) {
				stateX = 7;
				stateY = 0;
				moveQbert(DOWN);
			}
			else
				resetLevel1();
			glfwSetKeyCallback(window, keyPressed);
		}
	}

	for (Element *node : nodes) {
		if (!node->display)
			continue;
		glUniformMatrix4fv(pVariable, 1, GL_FALSE, &projectionMat[0][0]);
		glUniformMatrix4fv(vVariable, 1, GL_FALSE, &viewMat[0][0]);
		glUniformMatrix4fv(tVariable, 1, GL_FALSE, &node->transform[0][0]);

		ModelInfo *modelInfo = node->modelInfo;
		std::vector<GLuint> indices;
		glBindVertexArray(node->vertexArray);

		for (MTL_TRIANGLES::const_iterator it = node->trianglesInMTL.begin(); it != node->trianglesInMTL.end(); it++) {
			string groupName = it->first;
			IndexInfo *indexInfo = node->indexInfo[groupName];
			if (modelInfo->useVt && modelInfo->materials[groupName]->textureID != -1) {
				glUniform1i(textPresentVar, 1);
				glUniform1i(textureIDVar, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, modelInfo->materials[groupName]->textureID);
			}
			else {
				glUniform1i(textPresentVar, 0);
			}
			glUniform3fv(kaVar, 1, &modelInfo->materials[groupName]->ka.x);
			glUniform3fv(kdVar, 1, &modelInfo->materials[groupName]->kd.x);
			glUniform3fv(ksVar, 1, &modelInfo->materials[groupName]->ks.x);
			glUniform1f(NVar, modelInfo->materials[groupName]->N);
			glUniform1i(fontVar, 0);

			glBindBuffer(GL_ARRAY_BUFFER, node->primitiveBuffer);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Primitive), 0);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Primitive), (GLvoid *)sizeof(glm::vec3));

			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Primitive), (GLvoid *)(sizeof(glm::vec3) + sizeof(glm::vec3)));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->indexBuffer);
			glDrawElements(GL_TRIANGLES, indexInfo->size, GL_UNSIGNED_INT, (GLvoid *)(indexInfo->startIndex * sizeof(GLuint)));
	
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
		}
	}

	string text = "SCORE-" + to_string(score);
	writeOnScreen(text, 32, 2 * windowHeight - 32, 40, glm::vec3(0.0f, (1.0f - score / 700.0f), 1.0f));

	text = "LIVES-" + to_string(lives);
	writeOnScreen(text, 32, 2 * windowHeight - 80, 40, glm::vec3((1.0f - lives / 3.0f), (lives / 3.0f), 0.0f));

	text = "LEVEL-" + to_string(level);
	writeOnScreen(text, 2 * windowWidth - 340, 2 * windowHeight - 80, 40, glm::vec3(0.55f, 0.1f, 0.1f));

	text = "PLAYER-" + to_string(1);
	writeOnScreen(text, 2 * windowWidth - 340, 2 * windowHeight - 32, 40, glm::vec3(0.58f, 0.0f, 0.83f));

}

void Rasterize::writeOnScreen(string text, int startX, int startY, int sizeOfLetter, glm::vec3 color) {

	std::vector<glm::vec2> textVertices, textUVs;
	glm::vec2 topLeft, topRight, bottomLeft, bottomRight;
	char c;
	float uvX, uvY;

	GLuint fontIDVar = glGetUniformLocation(info.programId, "fontID");
	GLuint fontVar = glGetUniformLocation(info.programId, "fontPresent");
	glUniform3fv(cVar, 1, &color.x);

	for (unsigned int i = 0; i < text.length(); i++) {
		topLeft = (glm::vec2(startX + i * sizeOfLetter, startY - sizeOfLetter) - glm::vec2(windowWidth, windowHeight)) / glm::vec2(windowWidth, windowHeight);
		topRight = (glm::vec2(startX + i * sizeOfLetter + sizeOfLetter, startY - sizeOfLetter) - glm::vec2(windowWidth, windowHeight)) / glm::vec2(windowWidth, windowHeight);
		bottomLeft = (glm::vec2(startX + i * sizeOfLetter, startY) - glm::vec2(windowWidth, windowHeight)) / glm::vec2(windowWidth, windowHeight);
		bottomRight = (glm::vec2(startX + i * sizeOfLetter + sizeOfLetter, startY) - glm::vec2(windowWidth, windowHeight)) / glm::vec2(windowWidth, windowHeight);

		textVertices.push_back(topLeft);
		textVertices.push_back(bottomLeft);
		textVertices.push_back(topRight);

		textVertices.push_back(bottomRight);
		textVertices.push_back(topRight);
		textVertices.push_back(bottomLeft);

		c = text[i];
		uvX = (c % 16) / 16.0f;
		uvY = (c / 16 - 1) / 16.0f;

		uvY = 1.0f - uvY;

		topLeft = glm::vec2(uvX, uvY);
		topRight = glm::vec2(uvX + (1.0f / 16.0f), uvY);
		bottomLeft = glm::vec2(uvX, uvY + (1.0f / 16.0f));
		bottomRight = glm::vec2(uvX + (1.0f / 16.0f), uvY + (1.0f / 16.0f));

		textUVs.push_back(topLeft);
		textUVs.push_back(bottomLeft);
		textUVs.push_back(topRight);

		textUVs.push_back(bottomRight);
		textUVs.push_back(topRight);
		textUVs.push_back(bottomLeft);
	}

	glBindBuffer(GL_ARRAY_BUFFER, fontInfo.fontVertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(glm::vec2), &textVertices[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, fontInfo.fontUVBufferID);
	glBufferData(GL_ARRAY_BUFFER, textUVs.size() * sizeof(glm::vec2), &textUVs[0], GL_DYNAMIC_DRAW);

	glUniform1i(fontIDVar, 1);
	glUniform1i(fontVar, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fontInfo.fontTextureID);

	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, fontInfo.fontVertexBufferID);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, fontInfo.fontUVBufferID);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glDrawArrays(GL_TRIANGLES, 0, textVertices.size());

	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(2);

}

GLuint Rasterize::LoadShaders(const char* vertexFilePath, const char* fragmentFilePath){

	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertexFilePath, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory? Don't forget to read the FAQ!\n", vertexFilePath);
		getchar(); 
		return 0;
	}

	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragmentFilePath, std::ios::in);
	if (FragmentShaderStream.is_open()){
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory? Don't forget to read the FAQ!\n", fragmentFilePath);
		getchar();
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	printf("Compiling shader : %s\n", vertexFilePath);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	printf("Compiling shader : %s\n", fragmentFilePath);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;

}

int Rasterize::parseLightSources(string lightPath) {

	ifstream inputFile;
	string line, tempString;
	bool emptyLine;
	vector<string> *tokens;
	
	inputFile.open(lightPath);
	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			Light *light = new Light();
			light->location.x = std::stof((*tokens)[0]);
			light->location.y = std::stof((*tokens)[1]);
			light->location.z = std::stof((*tokens)[2]);
			light->la = glm::vec3(std::stof((*tokens)[3]), std::stof((*tokens)[4]), std::stof((*tokens)[5]));
			light->ld = glm::vec3(std::stof((*tokens)[6]), std::stof((*tokens)[7]), std::stof((*tokens)[8]));
			light->ls = glm::vec3(std::stof((*tokens)[9]), std::stof((*tokens)[10]), std::stof((*tokens)[11]));
			lights.push_back(light);
		}
	}
	return 0;

}

int Rasterize::parseEyeLocation() {

	eyeLocation.x = 0;
	eyeLocation.y = 0;
	eyeLocation.z = 2;
	lookAtVector.x = 0;
	lookAtVector.y = 0;
	lookAtVector.z = 0;
	lookUpVector.x = 0;
	lookUpVector.y = 1;
	lookUpVector.z = 0;
	return 0;

}

int Rasterize::parseInterfaceWindow(string winPath) {

	ifstream inputFile;
	string line, tempString;
	bool emptyLine;
	vector<string> *tokens;
	
	inputFile.open(winPath);
	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine && tokens->size() == 2) {
			windowWidth = std::stoi((*tokens)[0]);
			windowHeight = std::stoi((*tokens)[1]);
		}
		else if (!emptyLine && tokens->size() == 1) {
			fittingWidth = std::stoi((*tokens)[0]);
			return 0;
		}
	}
	return 1;

}

int Rasterize::parseOBJMTL(string objPath, ModelInfo *modelInfo) {

	string line, mtlPath, tempString, mtlName1, mtlName2;
	ifstream inputFile;
	float firstCoords, secondCoords, thirdCoords;
	int firstVertex, secondVertex, thirdVertex;
	Triangle *triangle;
	MTL *mtl = new MTL();
	bool firstTime = true, emptyLine, ka = false, ks = false, kd = false, N = false;
	vector<string> *tokens;
	vector<string> *groupName;
	MTL_HASHMAP materialsTemp;

	if (compareNoCase(objPath, "NOOBJ")) {
		return 0;
	}

	inputFile.open(objPath);
	if (!inputFile.is_open()) {
		cout << "The OBJ path " << objPath << " is incorrect." << endl;
		return 1;
	}

	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			if (compareNoCase((*tokens)[0], "mtllib")) {
				mtlPath = (*tokens)[1];
				break;
			}
		}
	}

	inputFile.close();
	if (mtlPath.empty()) {
		cout << "MTL path not provided in OBJ file. Please provide it after the mtllib token." << endl;
		return 1;
	}
	else {
		mtlPath = baseDir + mtlPath ;
		inputFile.open(mtlPath);
	}
	if (!inputFile.is_open()) {
		cout << "The MTL path " << mtlPath << " is incorrect." << endl;
		return 1;
	}

	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			if (compareNoCase((*tokens)[0], "newmtl")) {
				if (!firstTime) {
					if (!ka) {
						mtl->ka.x = 0.5;
						mtl->ka.y = 0.5;
						mtl->ka.z = 0.5;
					}
					if (!ks) {
						mtl->ks.x = 0.5;
						mtl->ks.y = 0.5;
						mtl->ks.z = 0.5;
					}
					if (!kd) {
						mtl->kd.x = 0.5;
						mtl->kd.y = 0.5;
						mtl->kd.z = 0.5;
					}
					if (!N) {
						mtl->N = 300.0;
					}
					ka = ks = kd = N = false;
					materialsTemp.emplace(mtlName2, mtl);
				}
				mtl = new MTL();
				mtlName2 = (*tokens)[1];
				mtl->textureID = -1;
				firstTime = false;
			}
			else if (compareNoCase((*tokens)[0], "ka") && !firstTime) {
				mtl->ka.x = std::stof((*tokens)[1]);
				mtl->ka.y = std::stof((*tokens)[2]);
				mtl->ka.z = std::stof((*tokens)[3]);
				ka = true;
			}
			else if (compareNoCase((*tokens)[0], "ks") && !firstTime) {
				mtl->ks.x = std::stof((*tokens)[1]);
				mtl->ks.y = std::stof((*tokens)[2]);
				mtl->ks.z = std::stof((*tokens)[3]);
				ks = true;
			}
			else if (compareNoCase((*tokens)[0], "kd") && !firstTime) {
				mtl->kd.x = std::stof((*tokens)[1]);
				mtl->kd.y = std::stof((*tokens)[2]);
				mtl->kd.z = std::stof((*tokens)[3]);
				kd = true;
			}
			else if ((((*tokens)[0].compare("N") == 0) || compareNoCase((*tokens)[0], "Ns")) && !firstTime) {
				mtl->N = std::stof((*tokens)[1]);
				N = true;
			}
			else if (compareNoCase((*tokens)[0], "map_kd") && !firstTime) {
				mtl->texturePath = (*tokens)[1];
				string texPath = baseDir + mtl->texturePath;
				GLuint textureID;
				int width, height;
				glGenTextures(1, &textureID);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureID);
				unsigned char *texture = SOIL_load_image(texPath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				if (textureID == 0) {
					cout << "Error in reading texture." << endl;
					modelInfo->useVt = false;
				}
				else {
					mtl->textureID = textureID;
					modelInfo->useVt = true;
				}
			}
		}
		free(tokens);
	}
	if (!ka) {
		mtl->ka.x = 0.5;
		mtl->ka.y = 0.5;
		mtl->ka.z = 0.5;
	}
	if (!ks) {
		mtl->ks.x = 0.5;
		mtl->ks.y = 0.5;
		mtl->ks.z = 0.5;
	}
	if (!kd) {
		mtl->kd.x = 0.5;
		mtl->kd.y = 0.5;
		mtl->kd.z = 0.5;
	}
	if (!N) {
		mtl->N = 300.0;
	}
	materialsTemp.emplace(mtlName2, mtl);

	inputFile.close();
	inputFile.open(objPath);

	groupName = new vector<string>();
	bool usemtlCheck = false;

	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		glm::vec3 tempVertex;
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			if (compareNoCase((*tokens)[0], "v")) {
				firstCoords = std::stof((*tokens)[1]);
				secondCoords = std::stof((*tokens)[2]);
				thirdCoords = std::stof((*tokens)[3]);
				tempVertex.x = firstCoords;
				tempVertex.y = secondCoords; 
				tempVertex.z = thirdCoords;
				modelInfo->verticesVec.push_back(tempVertex);
			}
			else if (compareNoCase((*tokens)[0], "vn")) {
				modelInfo->vnPresent = true;
				firstCoords = std::stof((*tokens)[1]);
				secondCoords = std::stof((*tokens)[2]);
				thirdCoords = std::stof((*tokens)[3]);
				tempVertex.x = firstCoords;
				tempVertex.y = secondCoords; 
				tempVertex.z = thirdCoords;
				modelInfo->normalsVec.push_back(tempVertex);
			}
			else if (compareNoCase((*tokens)[0], "vt")) {
				modelInfo->vtPresent = true;
				firstCoords = std::stof((*tokens)[1]);
				secondCoords = std::stof((*tokens)[2]);
				tempVertex.x = firstCoords;
				tempVertex.y = secondCoords; 
				modelInfo->textureVec.push_back(glm::vec2(tempVertex.x, tempVertex.y));
			}
			else if (compareNoCase((*tokens)[0], "g") || compareNoCase((*tokens)[0], "group")) {
				if (usemtlCheck || compareNoCase((*tokens)[0], "group")) {
					free(groupName);
					groupName = new vector<string>;
					usemtlCheck = false;
				}
				int groupSize = tokens->size();
				for (int i = 1; i < groupSize; i++) {
					groupName->push_back((*tokens)[i]);
				}
			}
			else if (compareNoCase((*tokens)[0], "f")) {
				int size = tokens->size();
				size_t pos, secondPos, fTokenPos, fTokenSecondPos;
				if (modelInfo->vnPresent || modelInfo->vtPresent) {
					string subToken, firstToken = (*tokens)[1];;
					fTokenPos = 0, fTokenSecondPos = 0;
					fTokenPos = firstToken.find('/', fTokenPos);
					fTokenSecondPos = firstToken.find('/', (fTokenPos + 1));
					for (int i = 2; i <= size - 2; i++) {
						triangle = new Triangle();
						triangle->groupName = groupName->at(groupName->size() - 1);
						triangle->vertexIndices[0] = std::stoi(firstToken.substr(0, fTokenPos)) - 1;
						if (modelInfo->vtPresent && (fTokenPos + 1 != fTokenSecondPos))
							triangle->textureIndices[0] = std::stoi(firstToken.substr((fTokenPos + 1), fTokenSecondPos)) - 1;
						if (modelInfo->vnPresent && (fTokenSecondPos + 1 != firstToken.length()))
							triangle->normalIndices[0] = std::stoi(firstToken.substr((fTokenSecondPos + 1), firstToken.length())) - 1;
						pos = 0, secondPos = 0;
						subToken = (*tokens)[i];
						pos = subToken.find('/', pos);
						secondPos = subToken.find('/', (pos + 1));
						triangle->vertexIndices[1] = std::stoi(subToken.substr(0, pos)) - 1;
						if (modelInfo->vtPresent && (pos + 1 != secondPos))
							triangle->textureIndices[1] = std::stoi(subToken.substr((pos + 1), secondPos)) - 1;
						if (modelInfo->vnPresent && (secondPos + 1 != subToken.length()))
							triangle->normalIndices[1] = std::stoi(subToken.substr((secondPos + 1), subToken.length())) - 1;
						pos = 0, secondPos = 0;
						subToken = (*tokens)[i + 1];
						pos = subToken.find('/', pos);
						secondPos = subToken.find('/', (pos + 1));
						triangle->vertexIndices[2] = std::stoi(subToken.substr(0, pos)) - 1;
						if (modelInfo->vtPresent && (pos + 1 != secondPos))
							triangle->textureIndices[2] = std::stoi(subToken.substr((pos + 1), secondPos)) - 1;
						if (modelInfo->vnPresent && (secondPos + 1 != subToken.length()))
							triangle->normalIndices[2] = std::stoi(subToken.substr((secondPos + 1), subToken.length())) - 1;
						modelInfo->triangles.push_back(triangle);
					}
				}
				else {
					for (int i = 2; i <= size - 2; i++) {
						triangle = new Triangle();
						triangle->groupName = groupName->at(groupName->size() - 1);
						firstVertex = std::stoi((*tokens)[1]) - 1;
						secondVertex = std::stoi((*tokens)[i]) - 1;
						thirdVertex = std::stoi((*tokens)[i + 1]) - 1;
						triangle->vertexIndices[0] = firstVertex;
						triangle->vertexIndices[1] = secondVertex;
						triangle->vertexIndices[2] = thirdVertex;
						modelInfo->triangles.push_back(triangle);
					}
				}
			}
			else if (compareNoCase((*tokens)[0], "usemtl")) {
				mtlName1 = (*tokens)[1];
				int totalGroupSize = groupName->size();
				MTL_HASHMAP::const_iterator it = materialsTemp.find(mtlName1);
				if (it != materialsTemp.end()) {
					for (int i = 0; i < totalGroupSize; i++) {
						modelInfo->materials.emplace(groupName->at(i), (*it).second);
					}
				}
				else {
					MTL *mtlTemp = new MTL();
					mtlTemp->ka.x = 0.5;
					mtlTemp->ka.y = 0.5;
					mtlTemp->ka.z = 0.5;
					mtlTemp->ks.x = 0.5;
					mtlTemp->ks.y = 0.5;
					mtlTemp->ks.z = 0.5;
					mtlTemp->kd.x = 0.5;
					mtlTemp->kd.y = 0.5;
					mtlTemp->kd.z = 0.5;
					mtlTemp->N = 300;
					for (int i = 0; i < totalGroupSize; i++) {
						modelInfo->materials.emplace(groupName->at(i), mtlTemp);
					}
				}
				usemtlCheck = true;
			}
		}
		free(tokens);
	}

	if (!usemtlCheck && groupName->size() > 0) {
		int totalGroupSize = groupName->size();
		MTL *mtlTemp = new MTL();
		mtlTemp->ka.x = 1.0;
		mtlTemp->ka.y = 1.0;
		mtlTemp->ka.z = 1.0;
		mtlTemp->ks.x = 1.0;
		mtlTemp->ks.y = 1.0;
		mtlTemp->ks.z = 1.0;
		mtlTemp->kd.x = 1.0;
		mtlTemp->kd.y = 1.0;
		mtlTemp->kd.z = 1.0;
		mtlTemp->N = 300;
		for (int i = 0; i < totalGroupSize; i++) {
			modelInfo->materials.emplace(groupName->at(i), mtlTemp);
		}
	}

	inputFile.close();

	return 0;

}

void Rasterize::normalizeVertices(Element *node, glm::mat4 initTransform) {

	glm::vec4 transformedVertex, transformedNormal;
	glm::mat4 translationMat, scaleMat;
	glm::vec3 translation, scale(1.0f);
	int count = 0;

	for (glm::vec3 vertex : node->modelInfo->verticesVec) {
		if (node->modelInfo->verticesVec[count].x > node->modelInfo->maxX) {
			node->modelInfo->maxX = node->modelInfo->verticesVec[count].x;
		}
		if (node->modelInfo->verticesVec[count].x < node->modelInfo->minX) {
			node->modelInfo->minX = node->modelInfo->verticesVec[count].x;
		}
		if (node->modelInfo->verticesVec[count].y > node->modelInfo->maxY) {
			node->modelInfo->maxY = node->modelInfo->verticesVec[count].y;
		}
		if (node->modelInfo->verticesVec[count].y < node->modelInfo->minY) {
			node->modelInfo->minY = node->modelInfo->verticesVec[count].y;
		}
		if (node->modelInfo->verticesVec[count].z > node->modelInfo->maxZ) {
			node->modelInfo->maxZ = node->modelInfo->verticesVec[count].z;
		}
		if (node->modelInfo->verticesVec[count].z < node->modelInfo->minZ) {
			node->modelInfo->minZ = node->modelInfo->verticesVec[count].z;
		}
		count++;
	}

	GLfloat xExpanse = (node->modelInfo->maxX - node->modelInfo->minX), yExpanse = (node->modelInfo->maxY - node->modelInfo->minY), zExpanse = (node->modelInfo->maxZ - node->modelInfo->minZ);
	translation.x = -((node->modelInfo->maxX + node->modelInfo->minX) / 2.0f);
	translation.y = -((node->modelInfo->maxY + node->modelInfo->minY) / 2.0f);
	translation.z = -((node->modelInfo->maxZ + node->modelInfo->minZ) / 2.0f);
	translationMat = glm::translate(glm::mat4(1.0f), translation);

	if (xExpanse > yExpanse && xExpanse > zExpanse && xExpanse > 2) 
		scale = glm::vec3((1 / (xExpanse / 2.0f)), (1 / (xExpanse / 2.0f)), (1 / (xExpanse / 2.0f)));
	else if (yExpanse > xExpanse && yExpanse > zExpanse && yExpanse > 2) 
		scale = glm::vec3((1 / (yExpanse / 2.0f)), (1 / (yExpanse / 2.0f)), (1 / (yExpanse / 2.0f)));
	else if (zExpanse > xExpanse && zExpanse > yExpanse && zExpanse > 2) 
		scale = glm::vec3((1 / (zExpanse / 2.0f)), (1 / (zExpanse / 2.0f)), (1 / (zExpanse / 2.0f)));
	scale *= (fittingWidth / GLfloat(windowWidth));
	scaleMat = glm::scale(glm::mat4(1.0f), scale);

	count = 0;
	for (glm::vec3 vertex : node->modelInfo->verticesVec) {
		transformedVertex = initTransform * scaleMat * translationMat * glm::vec4(vertex, 1.0f);
		node->modelInfo->verticesVec[count] = glm::vec3(transformedVertex);
		count++;
	}

	if (createNormals(node->modelInfo) == 0) {
		count = 0;
		for (glm::vec3 normal : node->modelInfo->normalsVec) {
			transformedNormal = node->initTransform * scaleMat * translationMat * glm::vec4(normal, 0.0f);
			if (transformedNormal.x != 0 && transformedNormal.y != 0 && transformedNormal.z != 0)
				node->modelInfo->normalsVec[count] = glm::normalize(glm::vec3(transformedNormal));
			count++;
		}

	}

}

int Rasterize::createNormals(ModelInfo *modelInfo) {

	int firstVertex, secondVertex, thirdVertex;

	if (!modelInfo->vnPresent && smoothingOpt) {
		for (Triangle *triangle : modelInfo->triangles) {
			firstVertex = triangle->vertexIndices[0];
			secondVertex = triangle->vertexIndices[1];
			thirdVertex = triangle->vertexIndices[2];
			triangle->faceNormal = glm::normalize(glm::cross((modelInfo->verticesVec[secondVertex] - modelInfo->verticesVec[firstVertex]), (modelInfo->verticesVec[thirdVertex] - modelInfo->verticesVec[firstVertex])));
		}
		int vertexCount = 0, normalCount = 0;
		for (glm::vec3 vertex : modelInfo->verticesVec) {
			glm::vec3 vertexNormal(0.0, 0.0, 0.0);
			for (Triangle *triangle : modelInfo->triangles) {
				if (triangle->vertexIndices[0] == vertexCount) {
					vertexNormal += triangle->faceNormal;
					triangle->normalIndices[0] = normalCount;
				}
				else if (triangle->vertexIndices[1] == vertexCount) {
					vertexNormal += triangle->faceNormal;
					triangle->normalIndices[1] = normalCount;
				}
				else if (triangle->vertexIndices[2] == vertexCount) {
					vertexNormal += triangle->faceNormal;
					triangle->normalIndices[2] = normalCount;
				}
			}
			if (vertexNormal.x != 0 && vertexNormal.y != 0 && vertexNormal.z != 0)
				vertexNormal = glm::normalize(vertexNormal);
			modelInfo->normalsVec.push_back(vertexNormal);
			vertexCount++;
			normalCount++;
		}
		modelInfo->vnPresent = true;
		return 1;
	}
	else
		return 0;

}

int Rasterize::parseBoard() {

	GLfloat rotateY, rotateX;
	int count = 0;
	glm::vec3 scale(1.0f, 1.0f, 1.0f);
	glm::mat4 rotateMat;
	string objPath;

	scale *= 0.10f;
	rotateY = -45.0f;
	rotateX = -36.0f;
	rotateMat = glm::rotate(glm::mat4(1.0f), rotateX, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rotateY, glm::vec3(0.0f, 1.0f, 0.0f));

	objPath = baseDir + "cube.obj";

	for (int i = 0; i < BOARD_HEIGHT; i++) {
		std::vector<Element *> *row = new std::vector<Element *>();
		grid.push_back(row);
		glm::vec3 translate(-0.64f, -0.64f, 0.5f);
		translate.x += i * 0.106f;
		translate.y += i * 0.19f;
		translate.z -= i * 0.07f;
		for (int j = 0; j <= i; j++) {
			Element *node = new Element();
			node->initTransform = glm::translate(glm::mat4(1.0f), translate);
			node->initTranslate = translate;
			node->transform = glm::mat4(1.0f);
			node->modelInfo = new ModelInfo();
			node->stepped = false;
			node->display = true;
			if (parseOBJMTL(objPath, node->modelInfo) != 0)
				return 1;
			normalizeVertices(node, node->initTransform * glm::scale(glm::mat4(1.0f), scale) * rotateMat);
			nodes.push_back(node);
			row->push_back(node);
			translate.x += 0.106f;
			translate.y -= 0.19f;
			translate.z += 0.07f;
		}
	}

	return 0;

}

int Rasterize::parseQbert() {

	string objPath = baseDir + "qzard.obj";
	glm::vec3 scale(0.12f, 0.12f, 0.12f);
	GLfloat rotateX = 0.0f, rotateY = -180.0f;
	glm::mat4 rotateMat;

	qbert = new Element();
	qbert->modelInfo = new ModelInfo();
	qbert->display = true;

	if (parseOBJMTL(objPath, qbert->modelInfo) != 0)
		return 1;

	rotateMat = glm::rotate(glm::mat4(1.0f), rotateX, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
	qbert->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(qbert, qbert->initTransform);
	nodes.push_back(qbert);

	glm::vec3 translate(-0.95f, 0.75f, -0.3f);
	for (int i = 0; i < 3; i++) {
		Element *qbertLife = new Element();
		glm::vec3 scale(0.8f, 0.8f, 0.8f);
		qbertLife->modelInfo = qbert->modelInfo;
		qbertLife->initTransform = qbert->initTransform;
		qbertLife->transform = glm::translate(glm::mat4(1.0f), translate) * glm::scale(glm::mat4(1.0f), scale);
		nodes.push_back(qbertLife);
		qbertLives.push_back(qbertLife);
		translate.x += 0.1f;
	}

	stateX = 7;
	stateY = 0;
	moveQbert(DOWN);

	return 0;

}

void moveQbert(int movementDir) {

	GLfloat rotateY;

	switch (movementDir) {
	case UP:
		rotateY = -135.0f;
		if (stateX + 1 > 6) {
			lives--;
			alive = false;
			alSourcePlay(sourceFall);
		}
		else
			stateX++;
		break;
	case DOWN:
		rotateY = 45.0f;
		if (stateX - 1 < 0 || stateY > stateX - 1) {
			lives--;
			alive = false;
			alSourcePlay(sourceFall);
		}
		else 
			stateX--;
		break;
	case LEFT:
		rotateY = 135.0f;
		if (stateY - 1 < 0) {
			lives--;
			alive = false;
			alSourcePlay(sourceFall);
		}
		else 
			stateY--;
		break;
	case RIGHT:
		rotateY = -45.0f;
		if (stateY + 1 > stateX) {
			lives--;
			alive = false;
			alSourcePlay(sourceFall);
		}
		else
			stateY++;
		break;
	}

	if (lives > 0) {		
		Rasterize::Element *cube = grid.at(stateX)->at(stateY);
		glm::mat4 cubeTransform = cube->initTransform;
		glm::vec3 translate(0.0f, 0.165f, -0.3f);
		glm::mat4 rotateMat;

		rotateMat = glm::rotate(glm::mat4(1.0f), rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
		qbert->transform = glm::translate(glm::mat4(1.0f), translate) * cubeTransform * rotateMat;

		if (!cube->stepped) {
			cube->modelInfo->materials["top"]->ka = glm::vec3(0.0f, 0.0f, 1.0f);
			cube->modelInfo->materials["top"]->kd = glm::vec3(0.0f, 0.0f, 1.0f);
			cube->modelInfo->materials["top"]->ks = glm::vec3(0.0f, 0.0f, 1.0f);
			cube->stepped = true;
			cubeCount++;
			score += 25;
		}
		else
			score -= 5;
	}

	if (cubeCount >= 28) {
		alSourcePlay(sourceStart);
		levelOver = true;
		alive = false;
	}

	for (int i = 0; i < lives; i++)
		qbertLives.at(i)->display = true;

	for (int i = 2; i > lives - 1; i--)
		qbertLives.at(i)->display = false;

}

void resetLevel1() {

	if (!levelOver)
		alSourcePlay(sourceStart);
	else
		levelOver = false;

	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j <= i; j++) {
			Rasterize::Element *cube = grid.at(i)->at(j);
			cube->modelInfo->materials["top"]->ka = glm::vec3(0.0f, 0.8f, 1.0f);
			cube->modelInfo->materials["top"]->kd = glm::vec3(0.0f, 0.8f, 1.0f);
			cube->modelInfo->materials["top"]->ks = glm::vec3(0.0f, 0.8f, 1.0f);
			cube->transform = glm::mat4(1.0f);
			cube->stepped = false;
		}
	}
	stateX = 7;
	stateY = 0;
	lives = 3;
	score = 0;
	cubeCount = 0;
	moveQbert(DOWN);

}

void initSound() {

	alutInit(0, NULL);
	bufferHop = alutCreateBufferFromFile("files/qbertHop.wav");
	bufferFall = alutCreateBufferFromFile("files/qbertFall.wav");
	bufferStart = alutCreateBufferFromFile("files/qbertStart.wav");

	alGenSources(1, &sourceHop);
	alSourcef(sourceHop, AL_PITCH, 1);
	alSourcef(sourceHop, AL_GAIN, 1);
	alSource3f(sourceHop, AL_POSITION, 0, 0, 0);
	alSource3f(sourceHop, AL_VELOCITY, 0, 0, 0);
	alSourcei(sourceHop, AL_LOOPING, AL_FALSE);
	alSourcei(sourceHop, AL_BUFFER, bufferHop);
	alGenSources(1, &sourceStart);
	alSourcef(sourceStart, AL_PITCH, 1);
	alSourcef(sourceStart, AL_GAIN, 1);
	alSource3f(sourceStart, AL_POSITION, 0, 0, 0);
	alSource3f(sourceStart, AL_VELOCITY, 0, 0, 0);
	alSourcei(sourceStart, AL_LOOPING, AL_FALSE);
	alSourcei(sourceStart, AL_BUFFER, bufferStart);
	alGenSources(1, &sourceFall);
	alSourcef(sourceFall, AL_PITCH, 1);
	alSourcef(sourceFall, AL_GAIN, 1);
	alSource3f(sourceFall, AL_POSITION, 0, 0, 0);
	alSource3f(sourceFall, AL_VELOCITY, 0, 0, 0);
	alSourcei(sourceFall, AL_LOOPING, AL_FALSE);
	alSourcei(sourceFall, AL_BUFFER, bufferFall);

}

