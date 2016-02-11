#ifndef CSE190_Mesh_h
#define CSE190_Mesh_h

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "Vector3.h"
#include "Drawable.h"

/* Forward Declarations */
struct Face;

struct Vertex {
    Vector3* coordinate; // Coordinate for Vertex
    Vector3* vertexNormal;
    std::vector<Face*>* vertToFaceAdj = new std::vector<Face*>; // Vector of adjacent faces to this vertex
};


/*
 * Each face contains 3 vertices, 3 edges, a face normal, and face adjacency
 */
struct Face {
    // Each face has 3 vertices, a face normal, and
    int vertexIndices[3];
    Vector3* faceNormal;
    std::vector<Face*>* faceToFaceAdj = new std::vector<Face*>; // Vector of neighboring faces
    
    /*
    int vertexIndices[3];
    int normalIndices[3]; */
    //Add more members as necessary
};





class Mesh : public Drawable
{
    
protected:
    
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

    std::vector<Vector3*>* normals;
    std::vector<Face*>* faces;
    
    //Parse
    void parse(std::string&);
    int findHole();
    
    //Calculate center values
    void getCenter();
    void getHalfSize();
    
public:
    
    Mesh(std::string);
    virtual ~Mesh(void);

    std::vector<Vertex*>* vertices;
    
    void computeFaceNormals();
    void computeVertexNormals();
    void buildConnectivity();
    bool checkDuplicateFaceAdj(Face*, Face*);
    
    void edgeCollapse(Vertex*, Vertex*);
    
    virtual void draw(DrawData&);
    virtual void update(UpdateData&);
    
    float halfSizeMAX = 0;
    
    void findAdjFaces(Vertex*);
    bool checkAdjacent(Face*, Face*);
    bool checkDuplicateVertAdj(Vertex*, Face*);
    void removeFace(Face*);
    
    // Given face, find its vertices
    // Given vertex, find faces touching it
    // Given face, find neighboring faces
    // Given vertex, find neighboring vertices
    // Given edge, find vertices and faces it touches
    
};

#endif
