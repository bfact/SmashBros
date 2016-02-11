#ifndef CSE190_Globals_h
#define CSE190_Globals_h

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
    static Mesh* testpatch;
    static Mesh* plane;
    
    static Drawable *objdraw;
    
    // Boolean Flats
    static bool flatShading;
    static bool colors;
};

#endif
