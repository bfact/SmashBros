#ifndef CSE167_Globals_h
#define CSE167_Globals_h

#include "Camera.h"
#include "Light.h"
#include "DrawData.h"
#include "UpdateData.h"
#include "Mesh.h"

class Globals
{
    
public:
    
    static Camera camera;
    static Light light;
    static DrawData drawData;
    static UpdateData updateData;
    //Feel free to add more member variables as needed
    
    static Mesh* teapot;
    static Mesh* armadillo;
    
    static Drawable *objdraw;
    
    // Boolean Flats
    static bool flatShading;
};

#endif
