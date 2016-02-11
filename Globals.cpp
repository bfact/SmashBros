#include "Globals.h"

/*
#define TEAPOT "/Users/BrittanyFactura/Github/smashbros/OFF Files/teapot.off"
#define ARMADILLO "/Users/BrittanyFactura/Github/smashbros/OFF Files/armadillo.off" */

#define TEAPOT "/Users/seanwenzel/Github/SmashBros/OFF Files/teapot.off"
#define ARMADILLO "/Users/seanwenzel/Github/SmashBros/OFF Files/armadillo.off"
#define TESTPATCH "/Users/seanwenzel/Github/SmashBros/OFF Files/testpatch.off"
#define PLANE "/Users/seanwenzel/Github/SmashBros/OFF Files/plane.off"



Camera Globals::camera;

Light Globals::light;

DrawData Globals::drawData;
UpdateData Globals::updateData;


Mesh* Globals::teapot = new Mesh(TEAPOT);
Mesh* Globals::armadillo = new Mesh(ARMADILLO);
Mesh* Globals::testpatch = new Mesh(TESTPATCH);
Mesh* Globals::plane = new Mesh(PLANE);

Drawable *Globals::objdraw = Globals::plane;

bool Globals::flatShading = true;

// Lights

