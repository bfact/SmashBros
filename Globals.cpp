#include "Globals.h"


#define TEAPOT "/Users/BrittanyFactura/Github/SmashBros/OFF Files/teapot.off"
#define ARMADILLO "/Users/BrittanyFactura/Github/SmashBros/OFF Files/armadillo.off"
#define TESTPATCH "/Users/BrittanyFactura/Github/SmashBros/OFF Files/testpatch.off"
#define PLANE "/Users/BrittanyFactura/Github/SmashBros/OFF Files/plane.off"

/*
#define TEAPOT "/Users/seanwenzel/Github/SmashBros/OFF Files/teapot.off"
#define ARMADILLO "/Users/seanwenzel/Github/SmashBros/OFF Files/armadillo.off"
#define TESTPATCH "/Users/seanwenzel/Github/SmashBros/OFF Files/testpatch.off"
#define PLANE "/Users/seanwenzel/Github/SmashBros/OFF Files/plane.off"
*/


Camera Globals::camera;

Light Globals::light;

DrawData Globals::drawData;
UpdateData Globals::updateData;


Mesh* Globals::teapot = new Mesh(TEAPOT);
Mesh* Globals::armadillo = new Mesh(ARMADILLO);
Mesh* Globals::testpatch = new Mesh(TESTPATCH);
Mesh* Globals::plane = new Mesh(PLANE);

Mesh* Globals::objdraw = Globals::testpatch;

bool Globals::flatShading = true;
bool Globals::colors = false;

// Lights

