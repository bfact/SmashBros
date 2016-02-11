#include "Globals.h"

/*
#define TEAPOT "/Users/BrittanyFactura/Github/smashbros/OFF Files/teapot.off"
#define ARMADILLO "/Users/BrittanyFactura/Github/smashbros/OFF Files/armadillo.off" */

#define TEAPOT "/Users/seanwenzel/Github/SmashBros/OFF Files/teapot.off"
#define ARMADILLO "/Users/seanwenzel/Github/SmashBros/OFF Files/armadillo.off"



Camera Globals::camera;

Light Globals::light;

DrawData Globals::drawData;
UpdateData Globals::updateData;


Mesh* Globals::teapot = new Mesh(TEAPOT);
Mesh* Globals::armadillo = new Mesh(ARMADILLO);

Drawable *Globals::objdraw = Globals::armadillo;

bool Globals::flatShading = true;

// Lights

