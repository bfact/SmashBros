#include "Drawable.h"
#include "Window.h"

Drawable::Drawable()
{
    this->toWorld.identity();
}

Drawable::~Drawable()
{
    //
}

void Drawable::draw(DrawData& data)
{
    std::cout << "Warning: draw() is not implemented" << std::endl;
}

void Drawable::update(UpdateData& data)
{
    std::cout << "Warning: update() is not implemented" << std::endl;
}


void Drawable::getHalfSize(float maxX, float maxY, float maxZ, float minX, float minY, float minZ)
{
    float halfSizeX = (maxX - minX)/2;
    float halfSizeY = (maxY - minY)/2;
    float halfSizeZ = (maxZ - minZ)/2;
    halfSizeMAX = fmax(fmax(halfSizeX, halfSizeY), halfSizeZ);
}