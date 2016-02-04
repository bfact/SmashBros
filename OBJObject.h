#ifndef CSE167_OBJObject_h
#define CSE167_OBJObject_h

#include <iostream>
#include <vector>
#include <string>
#include "Vector3.h"
#include "Drawable.h"

struct Face
{
    int vertexIndices[3];
    int normalIndices[3];
    //Add more members as necessary
};

class OBJObject : public Drawable
{
    
protected:
    
    bool color;
    
    float minX = 0;
    float maxX = 0;
    float minY = 0;
    float maxY = 0;
    float minZ = 0;
    float maxZ = 0;
    float centerX = 0;
    float centerY = 0;
    float centerZ = 0;
    
    //Storage vectors
    std::vector<Vector3*>* vertices;
    std::vector<Vector3*>* normals;
    std::vector<Face*>* faces;
    
    //std::vector<Vector3*>* colors;

    //Helper functions
    std::vector<std::string>& split(const std::string&, char, std::vector<std::string>&);
    std::vector<std::string> split(const std::string&, char);
    
    //Parse
    void parse(std::string&);
    //Calculate center values
    void getCenter();
    void getHalfSize();
    
public:
    
    OBJObject(std::string);
    virtual ~OBJObject(void);
    
    virtual void draw(DrawData&);
    virtual void update(UpdateData&);
    float halfSizeMAX = 0;
    
};

#endif
