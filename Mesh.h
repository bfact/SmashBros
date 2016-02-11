#ifndef CSE167_Mesh_h
#define CSE167_Mesh_h

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
    std::vector<Face*>* faceAdj = new std::vector<Face*>; // Vector of adjacent faces to this vertex
};

/*
struct EdgeData {
    Vertex from;
    Vertex to;
    //float weight;
    
    EdgeData(Vertex from, Vertex to) {
        this->from = from;
        this->to = to;
    }
    
    bool operator ==(const EdgeData& rhs) {
        if (( (to.coordinate == rhs.to.coordinate) && (from.coordinate == rhs.from.coordinate) ) ||
            ( (to.coordinate == rhs.from.coordinate) && (from.coordinate == rhs.to.coordinate) )) {
                return true;
            }
            else {
                return false;
            }
    }
};

struct Edge {
    // Edges are formed from 2 Vertices
    EdgeData data;
    std::vector<Face*>* faceAdj = new std::vector<Face*>; // Vector of adjacent faces to this edge
    
    Edge(Vertex from, Vertex to) : data(from, to) {}
    
    bool operator ==(const Edge& rhs) {
        return data == rhs.data;
    }
}; */


/*
 * Each face contains 3 vertices, 3 edges, a face normal, and face adjacency
 */
struct Face {
    // Each face has 3 vertices, a face normal, and
    int vertexIndices[3];
    // int edgeIndices[3];
    Vector3* faceNormal;
    
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
    std::vector<Vertex*>* vertices;
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
    
    void computeFaceNormals();
    void computeVertexNormals();
    void buildConnectivity();
    
    virtual void draw(DrawData&);
    virtual void update(UpdateData&);
    
    float halfSizeMAX = 0;
    
    // Given face, find its vertices
    // Given vertex, find faces touching it
    // Given face, find neighboring faces
    // Given vertex, find neighboring vertices
    // Given edge, find vertices and faces it touches
    
};

#endif
