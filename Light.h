#ifndef CSE190_Light_h
#define CSE190_Light_h

#include "Matrix4.h"
#include "Vector4.h"
#include "Color.h"
#include "Drawable.h"
#include <vector>
#include <GLUT/glut.h>


class Light : public Drawable
{
    
protected:
    
    int bindID = -1;
    
public:
    
    Vector4 position;
    Vector4 direction;
    
    Color ambientColor;
    Color diffuseColor;
    Color specularColor;
    
//    GLfloat ambient[4];
//    GLfloat diffuse[4];
//    GLfloat specular[4];
    
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    
    float angle;
    float exp;
    
public:
    
    Light();
    virtual ~Light(void);
    
    void bind(int);
    void unbind(void);

    
    Light(int, GLfloat*, GLfloat*, GLfloat*);
    
    int source;
    GLenum lightsource;
    
    void enable();
    void disable();
    void setDiffuse(GLfloat*);
    void setSpecular(GLfloat*);
    void setAmbient(GLfloat*);
    
    void setVisible(bool v);

    
    
};

#endif
