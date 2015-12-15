#include "Rasterize.h"

/*********************************************************************************************************************
* Q*bert!
*********************************************************************************************************************/

clock_t levelStartTime, coilyDeadTime;
int windowHeight, windowWidth, fittingWidth, score = 0, cubeCount = 0, level = 1, redBallDead = 0, iterationCount1 = 1, iterationCount2 = 1, iterationCount3 = 1, iterationCount4 = 1, creatureWaitTime = 2500; 
double frameTime, framesPerSec;
bool alive = true, pause = false, levelOver = false, gameOver = false, levelEnd = false,  purpleTransformed = false, bufferWait = true, movedFlag = false, killedFlag = false, onRightDisk = false, onLeftDisk = false, onRightDiskTemp = false, onLeftDiskTemp = false, coilyAnimFlag = false, fallenFlag = false, alInitMove = false;
GLfloat scaleCubeAnim, rotateCubeAnim, scaleQbertAnim, rotateDiskAnim, scaleCoilyAnim, rotateCoilyAnim, rotateQbert;
GLuint pVariable, vVariable, tVariable, textPresentVar, textureIDVar, kaVar, kdVar, ksVar, NVar, fontVar, cVar;
ALuint sourceHop, bufferHop, sourceFall, bufferFall, bufferCoilyFall, sourceStart, bufferStart, bufferCoilyHop, sourceCoilyHop, sourceCoilyFall, bufferMenuSong, sourceMenuSong, bufferKilled, sourceKilled, sourceBallHop, bufferBallHop;
GLFWwindow* window;
glm::vec3 translateDiskAnimL(0.0f, 0.0f, 0.0f), translateDiskAnimR(0.0f, 0.0f, 0.0f), translateCoilyAnim(0.0f, 0.14f, -0.3f);
string baseDir;
std::vector<std::vector<Rasterize::Element *> *> grid;
std::vector<Rasterize::Element *> nodes, qbertLives, redBalls, yellowBalls, menuNodes, creatures, toAnimate;
Rasterize::Element *purpleBall, *coily, *diskLeft, *diskRight, *qbert, *al;

void moveQbert(int movementDir);
void moveCreature(Rasterize::Element *creature, int movementDir);
void resetCreatures();
void hideCreatures();
void initSound();
void resetLevel1();
void resetLevel2();
void controlDisks();
void initDiskPositions();
void initRedBall(Rasterize::Element *redBall);
void initAnimate(int direction, Rasterize::Element *node);
bool compareNoCase(string first, string second);

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
	alDeleteSources(1, &sourceCoilyFall);
	alDeleteSources(1, &sourceMenuSong);
	alDeleteSources(1, &sourceFall);
	alDeleteSources(1, &sourceBallHop);
	alDeleteSources(1, &sourceStart);
	alDeleteBuffers(1, &bufferHop);
	alDeleteBuffers(1, &bufferFall);
	alDeleteBuffers(1, &bufferStart);
	alDeleteSources(1, &bufferCoilyFall);
	alDeleteSources(1, &bufferMenuSong);
	alDeleteSources(1, &bufferBallHop);

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
	if (parseCreatures() != 0) {
		cout << "Error occurred while creating creatures." << endl;
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

bool compareNoCase(string first, string second) {

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
	glm::mat4 translateMat(1.0f), rotateMatX(1.0f), rotateMatY(1.0f), rotateMat(1.0f);
	
	initShaders();

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetFramebufferSizeCallback(window, changeWindow);

	menuNodes.push_back(qbert);
	menuNodes.push_back(coily);
	menuNodes.push_back(redBalls.at(0));
	menuNodes.push_back(purpleBall);
	menuNodes.push_back(al);
	alSourcePlay(sourceMenuSong);
	showMenu();
	alSourceStop(sourceMenuSong);

	double lastTime = glfwGetTime(), currentTime;
	int frames = 0;

	qbert->stateX = 6;
	qbert->stateY = 0;
	qbert->initFlag = true;
	qbert->rotateAmount = 45.0f;

	alSourcePlay(sourceStart);
	resetCreatures();
	initDiskPositions();
	glfwSetKeyCallback(window, keyPressed);

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

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 && !gameOver);

	showGameOver();

	glfwTerminate();

	return 0;

}

void Rasterize::showGameOver() {

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		string text = "GAME OVER!";
		writeOnScreen(text, windowWidth - 250, 2 * windowHeight - 500, 50, glm::vec3((rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)));

		if (gameOver) {
			text = "YOU WON!";
			writeOnScreen(text, windowWidth - 200, 2 * windowHeight - 580, 50, glm::vec3((rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)));						
		}

		
		glfwSwapBuffers(window);

		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

}

void Rasterize::showMenu() {

	glm::mat4 projectionMat, viewMat;

	qbert->transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 4.0f, 4.0f)) * glm::rotate(glm::mat4(1.0f), 15.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	coily->transform = glm::translate(glm::mat4(1.0f), glm::vec3(-0.45f, -0.22f, 0.2f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f)) * glm::rotate(glm::mat4(1.0f), -25.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	redBalls.at(0)->transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, -0.33f, 0.2f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f)) * glm::rotate(glm::mat4(1.0f), 195.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	purpleBall->transform = glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f, -0.33f, 0.2f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f)) * glm::rotate(glm::mat4(1.0f), 195.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	al->transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.45f, -0.2f, 0.3f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.63f, 2.63f, 2.63f)) * glm::rotate(glm::mat4(1.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	do {
		projectionMat = glm::perspective(45.0f, ((GLfloat)windowWidth / (GLfloat)windowHeight), (GLfloat)nearZ, 100.0f);
		viewMat = glm::lookAt(-eyeLocation, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (Element *node : menuNodes) {
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

		string text = "Q*BERT!";
		writeOnScreen(text, windowWidth - 180, 220, 50, glm::vec3((rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)));

		text = "Press SPACE to continue.";
		writeOnScreen(text, windowWidth - 400, 140, 32, glm::vec3(1.0f, 0.0f, 1.0f));
		
		glfwSwapBuffers(window);

		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	
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

	ALint stateFall, stateStart, stateKilled;
	alGetSourcei(sourceFall, AL_SOURCE_STATE, &stateFall);
	alGetSourcei(sourceStart, AL_SOURCE_STATE, &stateStart);
	alGetSourcei(sourceKilled, AL_SOURCE_STATE, &stateKilled);
	if (stateStart == AL_PLAYING || stateFall == AL_PLAYING || stateKilled == AL_PLAYING)
		glfwSetKeyCallback(window, NULL);
	else
		glfwSetKeyCallback(window, keyPressed);

	if (!alive) {
		pause = true;
		alive = true;
		qbert->direction = 0;
		glfwSetKeyCallback(window, NULL);
		scaleCubeAnim = 1.0f, rotateCubeAnim = 0.0f, scaleQbertAnim = 1.0f;
	}
	else if (pause) {
		if (stateFall == AL_PLAYING || stateStart == AL_PLAYING || stateKilled == AL_PLAYING) {
			if (stateStart == AL_PLAYING) {
				levelEnd = true;
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
				scaleQbertAnim += 0.0015f;
				Element *qbertCube = grid.at(qbert->stateX)->at(qbert->stateY);
				qbert->modelInfo->materials["Qzard_Material"]->ka += 0.0008f;
				qbert->modelInfo->materials["Qzard_Material"]->ks += 0.0008f;
				qbert->modelInfo->materials["Qzard_Material"]->kd += 0.0008f;
			}
		}
		else {
			pause = false;
			qbert->animate = false;
			if (qbert->lives > 0 && !levelOver) {
				if (!killedFlag) {
					qbert->stateX = 6;
					qbert->stateY = 0;
					qbert->initFlag = true;
					qbert->rotateAmount = 45.0f;
					levelStartTime = clock();
					iterationCount1 = 1;
					iterationCount2 = 1;
					iterationCount3 = 1;
				}
				else {
					resetCreatures();
					killedFlag = false;
					fallenFlag = false;
				}
				if (level == 2) {
					for (int i = 0; i < BOARD_HEIGHT; i++) {
						for (int j = 0; j <= i; j++) {
							Rasterize::Element *cube = grid.at(i)->at(j);
							cube->killIntensity = 0;
							if (cube->stepped) {
								cube->modelInfo->materials["top"]->ka = glm::vec3(0.0f, 0.2f, 0.0f);
								cube->modelInfo->materials["top"]->kd = glm::vec3(0.0f, 0.2f, 0.0f);
								cube->modelInfo->materials["top"]->ks = glm::vec3(0.0f, 0.2f, 0.0f);
							}
							else {
								cube->modelInfo->materials["top"]->ka = glm::vec3(0.0f, 0.8f, 0.0f);
								cube->modelInfo->materials["top"]->kd = glm::vec3(0.0f, 0.8f, 0.0f);
								cube->modelInfo->materials["top"]->ks = glm::vec3(0.0f, 0.8f, 0.0f);
							}
						}
					}
				}
			}
			else {
				if (level == 1) {
					if (!killedFlag && !fallenFlag)
						resetLevel2();
					else
						resetLevel1();
				}
				else if (level == 2) {
					if (!killedFlag && !fallenFlag) {
						gameOver = true;
						return;
					}
					else
						resetLevel2();
				}
				killedFlag = false;
				fallenFlag = false;
				qbert->modelInfo->materials["Qzard_Material"]->ka = glm::vec3(0.588f, 0.588f, 0.588f);
				qbert->modelInfo->materials["Qzard_Material"]->ks = glm::vec3(0.588f, 0.588f, 0.588f);
				qbert->modelInfo->materials["Qzard_Material"]->kd = glm::vec3(0.588f, 0.588f, 0.588f);
				levelEnd = false;
			}
			glfwSetKeyCallback(window, keyPressed);
		}
	}
	else {
		if (!qbert->animate)
			checkCollisions();
	}

	ALint stateCoilyFall;
	alGetSourcei(sourceCoilyFall, AL_SOURCE_STATE, &stateCoilyFall);
	if (stateCoilyFall == AL_PLAYING) {
		Rasterize::Element *cube = grid.at(coily->stateX)->at(coily->stateY);
		translateCoilyAnim.x = (rotateCoilyAnim == 135.0f) ? translateCoilyAnim.x - 0.0002f : translateCoilyAnim.x + 0.0002f;
		translateCoilyAnim.y += 0.00005f;
		scaleCoilyAnim -= 0.001f;
		coilyAnimFlag = true;
		coily->transform = glm::translate(glm::mat4(1.0f), translateCoilyAnim) * cube->initTransform * glm::scale(glm::mat4(1.0f), glm::vec3(scaleCoilyAnim, scaleCoilyAnim, scaleCoilyAnim)) * glm::rotate(glm::mat4(1.0f), rotateCoilyAnim, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else if (coilyAnimFlag) {
		coily->display = false;
		coilyAnimFlag = false;
	}

	if (!levelEnd) {
		if (!killedFlag)
			controlCreatures();
		controlDisks();
	}
	else 
		hideCreatures();

	animate();

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

	text = "LIVES-" + to_string(qbert->lives);
	writeOnScreen(text, 32, 2 * windowHeight - 80, 40, glm::vec3((1.0f - qbert->lives / 3.0f), (qbert->lives / 3.0f), 0.0f));

	text = "LEVEL-" + to_string(level);
	writeOnScreen(text, 2 * windowWidth - 340, 2 * windowHeight - 80, 40, glm::vec3(0.55f, 0.1f, 0.1f));

	text = "PLAYER-" + to_string(1);
	writeOnScreen(text, 2 * windowWidth - 340, 2 * windowHeight - 32, 40, glm::vec3(0.58f, 0.0f, 0.83f));

	if (levelEnd) {
		text = "LEVEL COMPLETE!";
		writeOnScreen(text, windowWidth - 320, 80, 45, glm::vec3((rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)));
	}

}

void Rasterize::checkCollisions() {

	Element *cube = grid.at(qbert->stateX)->at(qbert->stateY);

	if (!onRightDisk && !onLeftDisk) {
		for (Element *creature : creatures) {
			if (cube->killIntensity > 10 || (creature->display && !creature->enemyDead && !creature->animate && creature->stateX == qbert->stateX && creature->stateY == qbert->stateY && !(creature != coily && creature->stateX == 6 && creature->stateY == 0 && (creature->initDirection == DOWN || creature->initDirection == RIGHT)))) {
				killedFlag = true;
				qbert->lives--;
				qbert->direction = 0;
				alive = false;
				alSourcePlay(sourceKilled);

				for (int i = 0; i < qbert->lives; i++)
					qbertLives.at(i)->display = true;

				for (int i = 2; i > qbert->lives - 1; i--)
					qbertLives.at(i)->display = false;

				break;
			}
		}
	}

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
		translate.z += i * 0.07f;
		for (int j = 0; j <= i; j++) {
			Element *node = new Element();
			node->initTransform = glm::translate(glm::mat4(1.0f), translate);
			node->initTranslate = translate;
			node->transform = glm::mat4(1.0f);
			node->modelInfo = new ModelInfo();
			node->killIntensity = 0;
			node->stepped = false;
			node->display = true;
			node->animate = false;
			node->objPath = objPath;
			if (parseOBJMTL(objPath, node->modelInfo) != 0)
				return 1;
			normalizeVertices(node, node->initTransform * glm::scale(glm::mat4(1.0f), scale) * rotateMat);
			nodes.push_back(node);
			row->push_back(node);
			translate.x += 0.106f;
			translate.y -= 0.19f;
			translate.z -= 0.07f;
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
	qbert->lives = 3;
	qbert->animate = false;
	qbert->startFlag = true;
	qbert->objPath = objPath;

	if (parseOBJMTL(objPath, qbert->modelInfo) != 0)
		return 1;

	rotateMat = glm::rotate(glm::mat4(1.0f), rotateX, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
	qbert->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(qbert, qbert->initTransform);
	nodes.push_back(qbert);
	toAnimate.push_back(qbert);

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

	return 0;

}

int Rasterize::parseCreatures() {

	string objPath = baseDir + "coily.obj";
	glm::vec3 scale(0.12f, 0.12f, 0.12f);
	GLfloat rotateX = 0.0f, rotateY = -180.0f;
	glm::mat4 rotateMat;

	coily = new Element();
	coily->modelInfo = new ModelInfo();
	coily->display = false;
	coily->enemyDead = false;
	coily->animate = false;
	coily->objPath = objPath;
	toAnimate.push_back(coily);

	if (parseOBJMTL(objPath, coily->modelInfo) != 0)
		return 1;

	rotateMat = glm::rotate(glm::mat4(1.0f), rotateX, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
	coily->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(coily, coily->initTransform);
	nodes.push_back(coily);
	scaleCoilyAnim = 1.0f;
	creatures.push_back(coily);

	objPath = baseDir + "red_ball.obj";
	scale = glm::vec3(0.09f, 0.09f, 0.09f);

	Element *redBallTemp = new Element();
	redBallTemp->modelInfo = new ModelInfo();
	redBallTemp->display = false;
	redBallTemp->enemyDead = false;
	redBallTemp->animate = false;
	redBallTemp->objPath = objPath;
	redBalls.push_back(redBallTemp);
	creatures.push_back(redBallTemp);
	toAnimate.push_back(redBallTemp);

	if (parseOBJMTL(objPath, redBallTemp->modelInfo) != 0)
		return 1;

	redBallTemp->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(redBallTemp, redBallTemp->initTransform);
	nodes.push_back(redBallTemp);

	redBallTemp = new Element();
	redBallTemp->modelInfo = new ModelInfo();
	redBallTemp->display = false;
	redBallTemp->enemyDead = false;
	redBallTemp->animate = false;
	redBallTemp->objPath = objPath;
	redBalls.push_back(redBallTemp);
	creatures.push_back(redBallTemp);
	toAnimate.push_back(redBallTemp);

	if (parseOBJMTL(objPath, redBallTemp->modelInfo) != 0)
		return 1;

	redBallTemp->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(redBallTemp, redBallTemp->initTransform);
	nodes.push_back(redBallTemp);

	objPath = baseDir + "purple_ball.obj";
	scale = glm::vec3(0.09f, 0.09f, 0.09f);

	purpleBall = new Element();
	purpleBall->modelInfo = new ModelInfo();
	purpleBall->display = false;
	purpleBall->enemyDead = false;
	purpleBall->animate = false;
	purpleBall->objPath = objPath;

	if (parseOBJMTL(objPath, purpleBall->modelInfo) != 0)
		return 1;

	purpleBall->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(purpleBall, purpleBall->initTransform);
	nodes.push_back(purpleBall);
	creatures.push_back(purpleBall);
	toAnimate.push_back(purpleBall);

	objPath = baseDir + "yellow_ball.obj";
	scale = glm::vec3(0.09f, 0.09f, 0.09f);

	Element *yellowBallTemp = new Element();
	yellowBallTemp->modelInfo = new ModelInfo();
	yellowBallTemp->display = false;
	yellowBallTemp->enemyDead = false;
	yellowBallTemp->animate = false;
	yellowBallTemp->objPath = objPath;
	yellowBalls.push_back(yellowBallTemp);
	creatures.push_back(yellowBallTemp);
	toAnimate.push_back(yellowBallTemp);

	if (parseOBJMTL(objPath, yellowBallTemp->modelInfo) != 0)
		return 1;

	yellowBallTemp->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(yellowBallTemp, yellowBallTemp->initTransform);
	nodes.push_back(yellowBallTemp);

	yellowBallTemp = new Element();
	yellowBallTemp->modelInfo = new ModelInfo();
	yellowBallTemp->display = false;
	yellowBallTemp->enemyDead = false;
	yellowBallTemp->animate = false;
	yellowBallTemp->objPath = objPath;
	yellowBalls.push_back(yellowBallTemp);
	creatures.push_back(yellowBallTemp);
	toAnimate.push_back(yellowBallTemp);

	if (parseOBJMTL(objPath, yellowBallTemp->modelInfo) != 0)
		return 1;

	yellowBallTemp->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(yellowBallTemp, yellowBallTemp->initTransform);
	nodes.push_back(yellowBallTemp);

	objPath = baseDir + "disk.obj";
	scale = glm::vec3(0.25f, 0.03f, 0.25f);

	diskLeft = new Element();
	diskLeft->modelInfo = new ModelInfo();
	diskLeft->display = false;
	diskLeft->enemyDead = false;
	diskLeft->objPath = objPath;

	if (parseOBJMTL(objPath, diskLeft->modelInfo) != 0)
		return 1;

	diskLeft->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(diskLeft, diskLeft->initTransform);
	nodes.push_back(diskLeft);

	objPath = baseDir + "disk.obj";
	scale = glm::vec3(0.25f, 0.03f, 0.25f);

	diskRight = new Element();
	diskRight->modelInfo = new ModelInfo();
	diskRight->display = false;
	diskRight->enemyDead = false;
	diskRight->objPath = objPath;

	if (parseOBJMTL(objPath, diskRight->modelInfo) != 0)
		return 1;

	diskRight->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(diskRight, diskRight->initTransform);
	nodes.push_back(diskRight);

	objPath = baseDir + "al.obj";
	scale = glm::vec3(0.12f, 0.12f, 0.12f);

	al = new Element();
	al->modelInfo = new ModelInfo();
	al->display = false;
	al->enemyDead = false;
	al->animate = false;
	al->objPath = objPath;

	if (parseOBJMTL(objPath, al->modelInfo) != 0)
		return 1;

	rotateMat = glm::rotate(glm::mat4(1.0f), -180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	al->initTransform = glm::scale(glm::mat4(1.0f), scale) * rotateMat;
	normalizeVertices(al, al->initTransform);
	nodes.push_back(al);
	creatures.push_back(al);
	toAnimate.push_back(al);

	initDiskPositions();

	return 0;

}

void moveQbert(int movementDir) {

	switch (movementDir) {
	case UP:
		qbert->rotateAmount = -135.0f;
		if (qbert->stateX + 1 > 6) {
			if (qbert->stateY == 4 && diskRight->display) {
				onRightDiskTemp = true;
				break;
			}
			resetCreatures();
			fallenFlag = true;
			qbert->lives--;
			score -= 30;
			qbert->direction = 0;
			alive = false;
			alSourcePlay(sourceFall);
		}
		break;
	case DOWN:
		qbert->rotateAmount = 45.0f;
		if (qbert->stateX - 1 < 0 || qbert->stateY > qbert->stateX - 1) {
			resetCreatures();
			fallenFlag = true;
			qbert->lives--;
			score -= 30;
			qbert->direction = 0;
			alive = false;
			alSourcePlay(sourceFall);
		}
		break;
	case LEFT:
		qbert->rotateAmount = 135.0f;
		if (qbert->stateY - 1 < 0) {
			if (qbert->stateX == 2 && diskLeft->display) {
				onLeftDiskTemp = true;
				break;
			}
			resetCreatures();
			fallenFlag = true;
			qbert->lives--;
			score -= 30;
			qbert->direction = 0;
			alive = false;
			alSourcePlay(sourceFall);
		}
		break;
	case RIGHT:
		qbert->rotateAmount = -45.0f;
		if (qbert->stateY + 1 > qbert->stateX) {
			resetCreatures();
			fallenFlag = true;
			qbert->lives--;
			score -= 30;
			qbert->direction = 0;
			alive = false;
			alSourcePlay(sourceFall);
		}
		break;
	}
	initAnimate(movementDir, qbert);

	if (qbert->lives > 0) {		
		if (onRightDiskTemp || onLeftDiskTemp) {
			glfwSetKeyCallback(window, NULL);
		}
	}

}

void initAnimate(int direction, Rasterize::Element *node) {

	node->direction = direction;
	if (node == coily && !purpleTransformed) {
		node->animate = false;
		purpleTransformed = true;
	}
	else
		node->animate = true;
	node->animTranslate = glm::vec3(0.0f);
	node->animScale = glm::vec3(1.0f, 0.85f, 1.0f);
	node->simulatedTime = 0;

	switch (direction) {
	case UP:
		node->velocityX = 0.00125f;
		node->velocityY = 0.00725f;
		node->velocityZ = 0.002f;
		break;
	case DOWN:
		node->velocityX = -0.00125f;
		node->velocityY = 0.00292f;
		node->velocityZ = -0.002f;
		break;
	case LEFT:
		node->velocityX = -0.00125f;
		node->velocityY = 0.00725f;
		node->velocityZ = 0.002f;
		break;
	case RIGHT:
		node->velocityX = 0.00125f;
		node->velocityY = 0.00292f;
		node->velocityZ = -0.002f;
		break;
	}

}

void Rasterize::animate() {

	Rasterize::Element *cube;
	glm::mat4 cubeTransform, rotateMat;
	glm::vec3 translate;

	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j <= i; j++) {
			Rasterize::Element *cube = grid.at(i)->at(j);
			if (cube->killIntensity > 0) {
				cube->modelInfo->materials["top"]->ka.x -= 1.0f / 1500.0f;
				cube->modelInfo->materials["top"]->kd.x -= 1.0f / 1500.0f;
				cube->modelInfo->materials["top"]->ks.x -= 1.0f / 1500.0f;
				if (cube->stepped) {
					cube->modelInfo->materials["top"]->ka.y += 0.2f / 1500.0f;
					cube->modelInfo->materials["top"]->kd.y += 0.2f / 1500.0f;
					cube->modelInfo->materials["top"]->ks.y += 0.2f / 1500.0f;
				}
				else {
					cube->modelInfo->materials["top"]->ka.y += 0.8f / 1500.0f;
					cube->modelInfo->materials["top"]->kd.y += 0.8f / 1500.0f;
					cube->modelInfo->materials["top"]->ks.y += 0.8f / 1500.0f;
				}
				cube->killIntensity--;
			}
		}
	}

	for (Element *node : toAnimate) {
		rotateMat = glm::rotate(glm::mat4(1.0f), node->rotateAmount, glm::vec3(0.0f, 1.0f, 0.0f));
		if (node == qbert || node == coily || node == al) 
			translate = glm::vec3(0.0f, 0.165f, -0.3f);
		else
			translate = glm::vec3(-0.001f, 0.125f, -0.3f);
		if (node->display && (!(node == qbert && (onLeftDisk || onRightDisk))) && !killedFlag) {
			if (node->animate && !qbert->startFlag) {
				if (node == qbert)
					glfwSetKeyCallback(window, NULL);
				if (node->animScale.y < 1.0f) {
					node->animScale.y += 0.0005f;
				}
				node->animTranslate.x += node->velocityX;
				node->animTranslate.y += node->velocityY - (GRAVITY * node->simulatedTime);
				node->animTranslate.z += node->velocityZ;
				cube = grid.at(node->stateX)->at(node->stateY);
				cubeTransform = cube->initTransform;

				node->transform = glm::translate(glm::mat4(1.0f), node->animTranslate) * glm::translate(glm::mat4(1.0f), translate) * cubeTransform * glm::scale(glm::mat4(1.0f), node->animScale) * rotateMat;
				node->simulatedTime++;

				if (((node == qbert || (node != qbert && !node->enemyDead)) && node->simulatedTime > 80 && alive && !pause) || (node != qbert && node->enemyDead && node->simulatedTime > 280)) {
					node->animate = false;
					if (node == qbert)
						glfwSetKeyCallback(window, keyPressed);
					else if (node->enemyDead) {
						node->display = false;
					}
					if (onRightDiskTemp && node == qbert) {
						onRightDisk = true;
						onRightDiskTemp = false;
						node->direction = 0;
						glfwSetKeyCallback(window, NULL);
					}
					else if (onLeftDiskTemp && node == qbert) {
						onLeftDisk = true;
						onLeftDiskTemp = false;
						node->direction = 0;
						glfwSetKeyCallback(window, NULL);
					}
				}
			}
			else if (!qbert->startFlag) {
				switch (node->direction) {
				case UP:
					node->stateX++;
					break;
				case DOWN:
					node->stateX--;
					break;
				case LEFT:
					node->stateY--;
					break;
				case RIGHT:
					node->stateY++;
					break;
				}
				if (node->direction != 0)
					node->initFlag = false;
				node->direction = 0;

				cube = grid.at(node->stateX)->at(node->stateY);
				cubeTransform = cube->initTransform;

				node->transform = glm::translate(glm::mat4(1.0f), translate) * cubeTransform * rotateMat;

				if (!cube->stepped && node == qbert && !node->initFlag && !killedFlag) {
					cube->modelInfo->materials["top"]->ka = (level == 1) ? glm::vec3(0.0f, 0.18f, 1.0f) : glm::vec3(0.0f, 0.2f, 0.0f);
					cube->modelInfo->materials["top"]->kd = (level == 1) ? glm::vec3(0.0f, 0.18f, 1.0f) : glm::vec3(0.0f, 0.2f, 0.0f);
					cube->modelInfo->materials["top"]->ks = (level == 1) ? glm::vec3(0.0f, 0.18f, 1.0f) : glm::vec3(0.0f, 0.2f, 0.0f);
					cube->stepped = true;
					cubeCount++;
					score += 25;
				}
				else if (node == al) {
					cube->modelInfo->materials["top"]->ka = (level == 1) ? glm::vec3(0.0f, 0.18f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
					cube->modelInfo->materials["top"]->kd = (level == 1) ? glm::vec3(0.0f, 0.18f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
					cube->modelInfo->materials["top"]->ks = (level == 1) ? glm::vec3(0.0f, 0.18f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
					cube->killIntensity = 1500;
					cube->killTime = clock();
				}
			}
			else {
				qbert->startFlag = false;
			}
		}
	}

	if (cubeCount >= 28) {
		alSourcePlay(sourceStart);
		levelOver = true;
		alive = false;
		cubeCount = 0;
	}

	for (int i = 0; i < qbert->lives; i++)
		qbertLives.at(i)->display = true;

	for (int i = 2; i > qbert->lives - 1; i--)
		qbertLives.at(i)->display = false;

}

void resetLevel2() {

	level = 2;

	if (!levelOver)
		alSourcePlay(sourceStart);
	else
		levelOver = false;

	resetCreatures();
	initDiskPositions();

	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j <= i; j++) {
			Rasterize::Element *cube = grid.at(i)->at(j);
			cube->modelInfo->materials["top"]->ka = glm::vec3(0.0f, 0.8f, 0.0f);
			cube->modelInfo->materials["top"]->kd = glm::vec3(0.0f, 0.8f, 0.0f);
			cube->modelInfo->materials["top"]->ks = glm::vec3(0.0f, 0.8f, 0.0f);
			cube->modelInfo->materials["front"]->ka = glm::vec3(0.0f, 0.2f, 0.5f);
			cube->modelInfo->materials["front"]->kd = glm::vec3(0.0f, 0.2f, 0.5f);
			cube->modelInfo->materials["front"]->ks = glm::vec3(0.0f, 0.2f, 0.5f);
			cube->modelInfo->materials["left"]->ka = glm::vec3(0.0f, 0.1f, 0.3f);
			cube->modelInfo->materials["left"]->kd = glm::vec3(0.0f, 0.1f, 0.3f);
			cube->modelInfo->materials["left"]->ks = glm::vec3(0.0f, 0.1f, 0.3f);
			cube->transform = glm::mat4(1.0f);
			cube->stepped = false;
		}
	}
	qbert->lives = 3;
	score = 0;
	cubeCount = 0;
	qbert->stateX = 6;
	qbert->stateY = 0;
	qbert->initFlag = true;
	qbert->rotateAmount = 45.0f;

}

void resetLevel1() {

	level = 1;

	if (!levelOver)
		alSourcePlay(sourceStart);
	else
		levelOver = false;

	resetCreatures();
	initDiskPositions();

	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j <= i; j++) {
			Rasterize::Element *cube = grid.at(i)->at(j);
			cube->modelInfo->materials["top"]->ka = glm::vec3(0.0f, 0.85f, 1.0f);
			cube->modelInfo->materials["top"]->kd = glm::vec3(0.0f, 0.85f, 1.0f);
			cube->modelInfo->materials["top"]->ks = glm::vec3(0.0f, 0.85f, 1.0f);
			cube->transform = glm::mat4(1.0f);
			cube->stepped = false;
		}
	}
	qbert->lives = 3;
	score = 0;
	cubeCount = 0;
	qbert->stateX = 6;
	qbert->stateY = 0;
	qbert->initFlag = true;
	qbert->rotateAmount = 45.0f;

}

void initSound() {

	alutInit(0, NULL);
	bufferMenuSong = alutCreateBufferFromFile("files/menuSong.wav");
	bufferHop = alutCreateBufferFromFile("files/qbertHop.wav");
	bufferCoilyHop = alutCreateBufferFromFile("files/coilyHop.wav");
	bufferCoilyFall = alutCreateBufferFromFile("files/coilyFall.wav");
	bufferFall = alutCreateBufferFromFile("files/qbertFall.wav");
	bufferStart = alutCreateBufferFromFile("files/qbertStart.wav");
	bufferKilled = alutCreateBufferFromFile("files/killed.wav");
	bufferBallHop = alutCreateBufferFromFile("files/ballHop.wav");

	alGenSources(1, &sourceMenuSong);
	alSourcef(sourceMenuSong, AL_PITCH, 1);
	alSourcef(sourceMenuSong, AL_GAIN, 1);
	alSource3f(sourceMenuSong, AL_POSITION, 0, 0, 0);
	alSource3f(sourceMenuSong, AL_VELOCITY, 0, 0, 0);
	alSourcei(sourceMenuSong, AL_LOOPING, AL_TRUE);
	alSourcei(sourceMenuSong, AL_BUFFER, bufferMenuSong);
	alGenSources(1, &sourceBallHop);
	alSourcef(sourceBallHop, AL_PITCH, 1);
	alSourcef(sourceBallHop, AL_GAIN, 1);
	alSource3f(sourceBallHop, AL_POSITION, 0, 0, 0);
	alSource3f(sourceBallHop, AL_VELOCITY, 0, 0, 0);
	alSourcei(sourceBallHop, AL_LOOPING, AL_FALSE);
	alSourcei(sourceBallHop, AL_BUFFER, bufferBallHop);
	alGenSources(1, &sourceKilled);
	alSourcef(sourceKilled, AL_PITCH, 1);
	alSourcef(sourceKilled, AL_GAIN, 1);
	alSource3f(sourceKilled, AL_POSITION, 0, 0, 0);
	alSource3f(sourceKilled, AL_VELOCITY, 0, 0, 0);
	alSourcei(sourceKilled, AL_LOOPING, AL_FALSE);
	alSourcei(sourceKilled, AL_BUFFER, bufferKilled);
	alGenSources(1, &sourceCoilyFall);
	alSourcef(sourceCoilyFall, AL_PITCH, 1);
	alSourcef(sourceCoilyFall, AL_GAIN, 1);
	alSource3f(sourceCoilyFall, AL_POSITION, 0, 0, 0);
	alSource3f(sourceCoilyFall, AL_VELOCITY, 0, 0, 0);
	alSourcei(sourceCoilyFall, AL_LOOPING, AL_FALSE);
	alSourcei(sourceCoilyFall, AL_BUFFER, bufferCoilyFall);
	alGenSources(1, &sourceCoilyHop);
	alSourcef(sourceCoilyHop, AL_PITCH, 1);
	alSourcef(sourceCoilyHop, AL_GAIN, 1);
	alSource3f(sourceCoilyHop, AL_POSITION, 0, 0, 0);
	alSource3f(sourceCoilyHop, AL_VELOCITY, 0, 0, 0);
	alSourcei(sourceCoilyHop, AL_LOOPING, AL_FALSE);
	alSourcei(sourceCoilyHop, AL_BUFFER, bufferCoilyHop);
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

void resetCreatures() {

	iterationCount1 = 1;
	iterationCount2 = 1;
	iterationCount3 = 1;
	iterationCount4 = 1;
	levelStartTime = clock();
	for (Rasterize::Element *redBall : redBalls) {
		redBall->enemyDead = false;
		redBall->display = false;
		redBall->animate = false;
		redBall->stateX = 6;
		redBall->stateY = 0;
	}
	for (Rasterize::Element *yellowBall : yellowBalls) {
		yellowBall->enemyDead = false;
		yellowBall->display = false;
		yellowBall->animate = false;
		yellowBall->sideBallTrack = 0;
		yellowBall->stateX = 6;
		yellowBall->stateY = 0;
	}
	yellowBalls.at(1)->enemyDead = true;
	redBalls.at(1)->enemyDead = true;
	purpleBall->stateX = 6;
	purpleBall->stateY = 0;
	purpleBall->animate = false;
	purpleBall->enemyDead = true;
	coily->enemyDead = true;
	al->enemyDead = true;
	al->display = false;
	al->animate = false;
	alInitMove = 
	purpleBall->display = false;
	coily->display = false;
	coily->animate = false;
	purpleTransformed = false;
	scaleCoilyAnim = 1.0f;

}

void hideCreatures() {

	for (Rasterize::Element *redBall : redBalls) {
		redBall->enemyDead = false;
		redBall->display = false;
		redBall->stateX = 6;
		redBall->stateY = 0;
	}
	for (Rasterize::Element *yellowBall : yellowBalls) {
		yellowBall->enemyDead = false;
		yellowBall->display = false;
		yellowBall->stateX = 6;
		yellowBall->stateY = 0;
	}
	coily->display = false;
	purpleBall->display = false;
	purpleBall->stateX = 6;
	purpleBall->stateY = 0;
	al->display = false;

}

void Rasterize::controlCreatures() {

	clock_t time = clock();

	if (time - levelStartTime > creatureWaitTime) {
		bufferWait = false;
		if (time - levelStartTime > creatureWaitTime + 2500 * iterationCount2) {
			iterationCount2++;
			for (Element *redBall : redBalls) {
				if (redBall->enemyDead && !redBall->animate && level == 1) {
					redBall->enemyDead = false;
					redBall->display = false;
					redBall->stateX = 6;
					redBall->stateY = 0;
					break;
				}
			}
			for (Element *yellowBall : yellowBalls) {
				if (yellowBall->enemyDead && !yellowBall->animate && level == 2) {
					yellowBall->enemyDead = false;
					yellowBall->display = false;
					yellowBall->sideBallTrack = 0;
					yellowBall->stateX = 6;
					yellowBall->stateY = 0;
					break;
				}
			}
		}
		if (time - levelStartTime > creatureWaitTime + 3500) {
			if (purpleBall->enemyDead && !purpleTransformed && !purpleBall->animate && level == 1) {
				purpleBall->enemyDead = false;
				purpleBall->stateX = 6;
				purpleBall->stateY = 0;
			}
		}
		if (time - levelStartTime > creatureWaitTime + 2000) {
			if (al->enemyDead && !al->animate && level == 2) {
				al->enemyDead = false;
				al->stateX = (int)((rand() / (float)RAND_MAX) * 5) + 1;
				al->stateY = al->stateX;
				al->initDirection = LEFT;
			}
		}
		if (time - levelStartTime > creatureWaitTime + 600 * iterationCount1) {
			iterationCount1++;
			for (Element *redBall : redBalls) {
				if (!redBall->enemyDead && !redBall->animate && level == 1) {
					redBall->display = true;
					redBall->initDirection = ((rand() % 2) + 1) * 2;
					moveCreature(redBall, redBall->initDirection);
				}
			}
			for (Element *yellowBall : yellowBalls) {
				if (!yellowBall->enemyDead && !yellowBall->animate && level == 2) {
					yellowBall->display = true;
					if (yellowBall->sideBallTrack == 0) {
						yellowBall->initDirection = ((rand() % 2) + 1) * 2;
						yellowBall->sideBallTrack = yellowBall->initDirection;
					}
					else {
						yellowBall->initDirection = yellowBall->sideBallTrack;
					}
					moveCreature(yellowBall, yellowBall->initDirection);
				}
			}
			if (!coily->enemyDead && !coily->animate) {
				coily->display = true;
				movedFlag = false;
				if (!movedFlag && coily->stateX < qbert->stateX) {
					moveCreature(coily, UP);
				}
				else if (!movedFlag && coily->stateX > qbert->stateX) {
					moveCreature(coily, DOWN);
				}
				if (!movedFlag && coily->stateY > qbert->stateY) {
					moveCreature(coily, LEFT);
				}
				else if (!movedFlag && coily->stateY < qbert->stateY) {
					moveCreature(coily, RIGHT);
				}
				if ((coily->stateX == 6 && onRightDisk) || (coily->stateY == 0 && onLeftDisk)) {
					coily->enemyDead = true;
					coilyDeadTime = clock();
					if (onRightDisk)
						initAnimate(UP, coily);
					else
						initAnimate(LEFT, coily);
					score += 50;
					rotateCoilyAnim = (coily->stateX == 6) ? -135.0f : 135.0f;
					alSourcePlay(sourceCoilyFall);
				}
			}
			if (!purpleBall->enemyDead && !purpleTransformed && !purpleBall->animate) {
				purpleBall->display = true;
				purpleBall->initDirection = ((rand() % 2) + 1) * 2;
				moveCreature(purpleBall, purpleBall->initDirection);
			}
		}
		if (time - levelStartTime > creatureWaitTime + 950 * iterationCount4) {
			iterationCount4++;
			if (!al->enemyDead && !al->animate) {
				al->display = true;
				movedFlag = false;
				if (alInitMove) {
					if (!movedFlag && al->stateX < qbert->stateX) {
						moveCreature(al, UP);
					}
					else if (!movedFlag && al->stateX > qbert->stateX) {
						moveCreature(al, DOWN);
					}
					if (!movedFlag && al->stateY > qbert->stateY) {
						moveCreature(al, LEFT);
					}
					else if (!movedFlag && al->stateY < qbert->stateY) {
						moveCreature(al, RIGHT);
					}
				}
				else {
					alInitMove = true;
					moveCreature(al, al->initDirection);
				}
			}
		}
		if (time - levelStartTime > (coilyDeadTime - levelStartTime) + 8000) {
			iterationCount3++;
			if (coily->enemyDead && !coily->animate && purpleTransformed) {
				coily->enemyDead = false;
				coily->display = true;
				coily->stateX = (int)((rand() / (float)RAND_MAX) * 2 + 2);
				coily->stateY = (int)((rand() / (float)RAND_MAX) * (coily->stateX - 2) + 2);
				purpleTransformed = false;
				coily->direction = 0;
				moveCreature(coily, 0);
			}
		}
	}

}

void initDiskPositions() {

	diskLeft->display = true;
	diskRight->display = true;

	rotateDiskAnim = 0.0f;
	translateDiskAnimL = glm::vec3(0.0f, 0.0f, 0.0f);
	translateDiskAnimR = glm::vec3(0.0f, 0.0f, 0.0f);

}

void controlDisks() {

	rotateDiskAnim += 2.0f;
	glm::mat4 cubeTransform, rotateMat;
	glm::vec3 translate;
	Rasterize::Element *cube;
	GLfloat rotateX = -40.0f;
	
	rotateMat = glm::rotate(glm::mat4(1.0f), rotateX, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rotateDiskAnim, glm::vec3(0.0f, 1.0f, 0.0f));

	if (diskLeft->display) {
		diskLeft->stateX = 2;
		diskLeft->stateY = 0;
		cube = grid.at(diskLeft->stateX)->at(diskLeft->stateY);
		glm::mat4 cubeTransform = cube->initTransform;
		translate = glm::vec3(-0.17f, 0.17f, -0.3f);

		if (onLeftDisk) {
			if (translateDiskAnimL.x < 0.4f) {
				translateDiskAnimL.x += 0.00062f;
				translateDiskAnimL.y += 0.0012f;
				qbert->transform = glm::translate(glm::mat4(1.0f), glm::vec3(-0.005f, 0.12f, 0.0f)) * glm::translate(glm::mat4(1.0f), translateDiskAnimL) * glm::translate(glm::mat4(1.0f), translate) * cubeTransform;
			}
			else {
				qbert->stateX = 6;
				qbert->stateY = 0;
				qbert->rotateAmount = 45.0f;
				onLeftDisk = false;
				diskLeft->display = false;
				glfwSetKeyCallback(window, keyPressed);
			}
		}
		diskLeft->transform = glm::translate(glm::mat4(1.0f), translateDiskAnimL) * glm::translate(glm::mat4(1.0f), translate) * cubeTransform * rotateMat;
	}

	if (diskRight->display) {
		diskRight->stateX = 6;
		diskRight->stateY = 4;
		cube = grid.at(diskRight->stateX)->at(diskRight->stateY);
		cubeTransform = cube->initTransform;
		translate = glm::vec3(0.17f, 0.17f, -0.3f);

		if (onRightDisk) {			
			if (translateDiskAnimR.x > -0.4f) {
				translateDiskAnimR.x -= 0.00062f;
				translateDiskAnimR.y += 0.0012f;
				qbert->transform = glm::translate(glm::mat4(1.0f), glm::vec3(-0.005f, 0.12f, 0.0f)) * glm::translate(glm::mat4(1.0f), translateDiskAnimR) * glm::translate(glm::mat4(1.0f), translate) * cubeTransform;
			}
			else {
				qbert->stateX = 6;
				qbert->stateY = 0;
				qbert->rotateAmount = 45.0f;
				onRightDisk = false;
				diskRight->display = false;
				glfwSetKeyCallback(window, keyPressed);
			}
		}
		diskRight->transform = glm::translate(glm::mat4(1.0f), translateDiskAnimR) * glm::translate(glm::mat4(1.0f), translate) * cubeTransform * rotateMat;
	}

}

void moveCreature(Rasterize::Element *creature, int movementDir) {

	switch (movementDir) {
	case UP:
		creature->rotateAmount = -135.0f;
		if (creature->stateX + 1 > 6) {
			if (creature == yellowBalls.at(0) || creature == yellowBalls.at(1)) {
				creature->enemyDead = true;
			}
		}
		else {
			if (creature == coily)
				alSourcePlay(sourceCoilyHop);
			else
				alSourcePlay(sourceBallHop);
			movedFlag = true;
		}
		break;
	case DOWN:
		creature->rotateAmount = 45.0f;
		if (creature->stateX - 1 < 0 || creature->stateY > creature->stateX - 1) {
			if (creature == redBalls.at(0) || creature == redBalls.at(1)) {
				creature->enemyDead = true;
			}
			else if (creature == yellowBalls.at(0) || creature == yellowBalls.at(1)) {
				creature->sideBallTrack = UP;
				movementDir = UP;
			}
			else if (creature == purpleBall) {
				creature->display = false;
				coily->enemyDead = false;
				coily->stateX = creature->stateX;
				coily->stateY = creature->stateY;
				coily->display = true;
				creature = coily;
				initAnimate(0, creature);
				return;
			}
		}
		else {
			if (creature == coily)
				alSourcePlay(sourceCoilyHop);
			else
				alSourcePlay(sourceBallHop);
			movedFlag = true;
		}
		break;
	case LEFT:
		creature->rotateAmount = 135.0f;
		if (creature->stateY - 1 < 0) {
			if (creature == yellowBalls.at(0) || creature == yellowBalls.at(1)) {
				creature->enemyDead = true;
			}
		}
		else {
			if (creature == coily)
				alSourcePlay(sourceCoilyHop);
			else
				alSourcePlay(sourceBallHop);
			movedFlag = true;
		}
		break;
	case RIGHT:
		creature->rotateAmount = -45.0f;
		if (creature->stateY + 1 > creature->stateX) {
			if (creature == redBalls.at(0) || creature == redBalls.at(1)) {
				creature->enemyDead = true;
			}
			else if (creature == yellowBalls.at(0) || creature == yellowBalls.at(1)) {
				creature->sideBallTrack = LEFT;
				movementDir = LEFT;
			}
			else if (creature == purpleBall) {
				creature->display = false;
				coily->enemyDead = false;
				coily->stateX = creature->stateX;
				coily->stateY = creature->stateY;
				coily->display = true;
				creature = coily;
				initAnimate(0, creature);
				return;
			}
		}
		else {
			if (creature == coily)
				alSourcePlay(sourceCoilyHop);
			else
				alSourcePlay(sourceBallHop);
			movedFlag = true;
		}
		break;
	}

	initAnimate(movementDir, creature);

}