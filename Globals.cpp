#include "Globals.h"

#define TEAPOT "/Users/BrittanyFactura/Github/smashbros/OFF Files/teapot.off"
#define ARMADILLO "/Users/BrittanyFactura/Github/smashbros/OFF Files/armadillo.off"

Camera Globals::camera;

Light Globals::light;

DrawData Globals::drawData;
UpdateData Globals::updateData;


OBJObject* Globals::teapot = new OBJObject(TEAPOT);
OBJObject* Globals::armadillo = new OBJObject(ARMADILLO);

Drawable *Globals::objdraw = Globals::armadillo;

// Lights

