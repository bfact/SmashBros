#ifndef CSE190_Mesh_h
#define CSE190_Mesh_h

#include <iostream>
#include <vector>
#include <map>
#include <queue>
//#include <deque>
#include <string>
#include "Heap.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Drawable.h"

using namespace std;

/* Forward Declarations */
struct Face;
struct Vertex;
struct VertexPair;
class Heap;

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
    
    // A Heap of VertexPairs
    Heap* pairs;
    
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
    void computeFaceNormal(Face*);
    void computeVertexNormals();
    void computeVertexNormal(Vertex*);
    void computeKMatrices();
    void computeKMatrix(Face*);
    void computeQMatrices();
    void computeQMatrix(Vertex*);
    void findAllPairs();
    void computePairCosts();
    void computePairCost(VertexPair*);
    void buildConnectivity();
    bool checkDuplicateFaceAdj(Face*, Face*);
    bool checkDuplicateVertToVertAdj(Vertex*, Vertex*);
    
    void updatePair(VertexPair*, Vertex*, Vertex*);
    
    void quadricSimplification();
    
    void computeVertexErrors();
    void computeVertexError(Vertex*);
    
    void edgeCollapseAtMidpoint(VertexPair*);
    
    virtual void draw(DrawData&);
    virtual void update(UpdateData&);
    
    float halfSizeMAX = 0;
    
    void findAdjFaces(Vertex*);
    void findAdjVertices(Vertex*);
    bool checkAdjacent(Face*, Face*);
    bool checkDuplicateVertAdj(Vertex*, Face*);
    bool checkDuplicateVertToVert(Vertex*, Vertex*);
    void removeFace(Face*);
    
    int countValidVertices();
    
    
    void computeFaceNormals(Face*);
    
    // For debug
    void meshStatus(string);
    
    
    // Given face, find its vertices
    // Given vertex, find faces touching it
    // Given face, find neighboring faces
    // Given vertex, find neighboring vertices
    // Given edge, find vertices and faces it touches
    
};

struct Vertex {
    Vector3* coordinate; // Coordinate for Vertex
    Vector3* vertexNormal;
    std::vector<Face*>* vertToFaceAdj = new std::vector<Face*>; // Vector of adjacent faces to this vertex
    
    std::vector<Vertex*>* vertToVertAdj = new std::vector<Vertex*>; // change to indices
    //std::vector<int>* vertToVertAdj = new std::vector<int>;
    
    bool valid;
    int index;
    
    int parent1;
    int parent2;
    
    /* Quadric Error Metrics */
    // With each vertex, we associate the set of pairs of which it is a member
    
    // Each Vertex keeps track of it's pairs
    std::vector<VertexPair*>* localPairs = new std::vector<VertexPair*>(); // change to indices
    //std::vector<int>* localPairs = new std::vector<int>();
    
    Matrix4 Q;
    float error;
    
    Vertex() {};
    
    Vertex(Vector3* coord) {
        coordinate = coord;
    }
    
    Vertex(Vector3* coord, Matrix4 mat) {
        coordinate = coord;
        Q = mat;
    }
    
    bool checkPairDuplicate(VertexPair* pair);
    
    
    void printCoordinate(ofstream&);
    void printNormal(ofstream&);
    void printError(ofstream&);
    void printAdjVertices(ofstream&);
    void printAdjFaces(ofstream&, std::vector<Vertex*>*);
    void printPairs(ofstream&);
    void printIndex(ofstream&);
};

struct VertexPair {
    Vertex* a;
    Vertex* b;
    float error;
    
    Vertex* collapsedVertex;
    
    VertexPair(Vertex* v0, Vertex* v1) : a(v0), b(v1) {};
    
    // Compares if two VertexPairs have same points
    bool equals(const VertexPair& rhs) {
        if ((a == rhs.a && b == rhs.b) || (a == rhs.b && b == rhs.a)) {
            return true;
        }
        return false;
    }
    
    // Operators compare error values
    inline bool operator==(const VertexPair& rhs) {
        return error == rhs.error;
    }
    
    inline bool operator>(const VertexPair& rhs) {
        return error > rhs.error;
    }
    
    inline bool operator<(const VertexPair& rhs) {
        return error < rhs.error;
    }
    
    bool vertexOverlap(Vertex* currVert) {
        if (a == currVert || b == currVert) {
            return true;
        }
        return false;
    }
    
    void replaceVertex(Vertex* targetVert, Vertex* replacementVert) {
        if (a == targetVert) {
            a = replacementVert;
        }
        else if (b == targetVert) {
            b = replacementVert;
        }
        else {
            std::cerr << "Could not find targetVert" << std::endl;
            return;
        }
    }
    
    void printVertexPair(ofstream&);
};

/*
 * Each face contains 3 vertices, 3 edges, a face normal, and face adjacency
 */
struct Face {
    // Each face has 3 vertices, a face normal, and
    int vertexIndices[3];
    Vector3* faceNormal;
    std::vector<Face*>* faceToFaceAdj = new std::vector<Face*>; // Vector of neighboring faces
    bool valid;
    
    /* Quadric Error Metrics */
    //Vector4 p;
    Matrix4 K;
    
    void printVertexCoordinates(ofstream&, std::vector<Vertex*>*);
    bool isDegenerate();
};




#endif
