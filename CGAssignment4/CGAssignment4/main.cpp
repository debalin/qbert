#include "Rasterize.h"
using namespace std;
using namespace glm;

int main(int argc, char** argv) {

	Rasterize rasterize;
	
	if (rasterize.startGame() == 0)
		printf("GAME END!");
	else 
		printf("Ran into some problem!");

	return 0;
}
