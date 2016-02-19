#include "Mesh.h"

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include "Window.h"
#include "math.h"
#include "Globals.h"
#include <sstream>
#include <fstream>

using namespace std;

#define deleteVector(__type__, __vect__) do {\
                                   std::vector<__type__>::iterator iter; \
                                   std::vector<__type__>::iterator end; \
                                   iter = __vect__->begin();\
                                   end = __vect__->end();\
                                   while(iter != end) delete (*(iter++));\
                                   delete __vect__;\
                               } while(false)

// Forward Declaration of Helper Methods
std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

Mesh::Mesh(std::string filename) : Drawable()
{
    this->vertices = new std::vector<Vertex*>();
    this->normals = new std::vector<Vector3*>();
    this->faces = new std::vector<Face*>();
    this->pairs = new Heap();
    //this->colors = new std::vector<Vector3*>();
    
    parse(filename);
    
    //Setup size
    float tan = (30/180.0) * M_PI;
    
    Matrix4 setup;
    setup.makeScale((16.37 * tan)/halfSizeMAX);
    toWorld = setup.multiply(toWorld);
    
}

Mesh::~Mesh()
{
    //Delete any dynamically allocated memory/objects here
    
    deleteVector(Vertex*, vertices);
    deleteVector(Vector3*, normals);
    deleteVector(Face*, faces);
    //deleteVector(Edge*, edges);
    //deleteVector(Vector3*, colors);
}


void Mesh::draw(DrawData& data)
{
    Face* face;
    
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glMultMatrixf(toWorld.ptr());
    
    glBegin(GL_TRIANGLES);
    
    
    //Loop through the faces
    //For each face:
    //  Look up the vertices, normals (if they exist), and texcoords (if they exist)
    //  Draw them as triplets:
    
    //      glNorm(normals->at(face.normalIndices[0]))
    //      glVert(vertices->at(face.vertexIndices[0]))
    //      Etc.
    //
    srand(2);
    
    for (int i = 0; i < faces->size(); i++) {
        face = faces->at(i);
        if (Globals::colors == true) {
            glColor3f((float)(rand() % 100)/100, (float)(rand() % 100)/100, (float)(rand() % 100)/100);
        }
        else {
            glColor3f(1, 1, 1);
        }
        
        for (int j = 0; j < 3; j++) {
            Vertex* v  = vertices->at(face->vertexIndices[j]);
            glNormal3f(v->vertexNormal->get(0), v->vertexNormal->get(1), v->vertexNormal->get(2));
            glVertex3f(v->coordinate->get(0), v->coordinate->get(1), v->coordinate->get(2));

        }
    }

    
    glEnd();
    
    glPopMatrix();
}

void Mesh::update(UpdateData& data)
{
    //
}

void Mesh::parse(std::string& filename)
{
    std::ifstream infile(filename);
    std::string line;
    std::vector<std::string> tokens;
    std::string token;
    std::vector<std::string> vertex1, vertex2, vertex3;
    
    int verticesAmt = 0;
    int facesAmt = 0;
    
    std::cout << "Starting Parse: " << filename << std::endl;
    
    // Read first line, should be "OFF"
    std::getline(infile, line);
    
    if (line.compare("OFF") != 0) {
        std::cerr << "Error: Not an OFF file." << std::endl;
        return;
    }
    
    // Read second line, should be Vertices + Faces amount
    std::getline(infile, line);
    
    tokens.clear();
    tokens = split(line, ' ', tokens);
    
    verticesAmt = std::stoi(tokens.at(0));
    facesAmt = std::stoi(tokens.at(1));
    
    // Loop and Draw all Faces
    for (int i = 0; i < verticesAmt; i++) {
        std::getline(infile, line);
        tokens.clear();
        tokens = split(line, ' ', tokens);
        
        float x = std::stof(tokens.at(0));
        float y = std::stof(tokens.at(1));
        float z = std::stof(tokens.at(2));
        
        if (x < minX) {
            minX = x;
        }
        if (x > maxX) {
            maxX = x;
        }
        if (y < minY) {
            minY = y;
        }
        if (y > maxY) {
            maxY = y;
        }
        if (z < minZ) {
            minZ = z;
        }
        if (z > maxZ) {
            maxZ = z;
        }
        
        Vertex* vert = new Vertex;
        vert->coordinate = new Vector3(x, y, z);
        vert->valid = true;
        vert->parent1 = -1;
        vert->parent2 = -1;
        vert->index = (int) vertices->size();
        vertices->push_back(vert);
    }
    
    for (int i = 0; i < facesAmt; i++) {
        std::getline(infile, line);
        tokens.clear();
        tokens = split(line, ' ', tokens);
        
        Face* face = new Face;

        face->vertexIndices[0] = std::stoi(tokens.at(1));
        face->vertexIndices[1] = std::stoi(tokens.at(2));
        face->vertexIndices[2] = std::stoi(tokens.at(3));
        
        face->valid = 1;
        
        faces->push_back(face);

    }
    
    std::cout << "Done parsing: " << filename << std::endl;
    
    getCenter();
    getHalfSize();
    
    for (int i = 0; i < vertices->size(); i++) {
        Vector3* v  = vertices->at(i)->coordinate;
        float x = v->get(0) - centerX;
        float y = v->get(1) - centerY;
        float z = v->get(2) - centerZ;
        *v = Vector3(x,y,z);
    }
    
    
    buildConnectivity();
    computeFaceNormals();
    computeVertexNormals();
    computeKMatrices();
    computeQMatrices();
    computeVertexErrors();
    
    findAllPairs();
    computePairCosts();
    
    meshStatus("/Users/seanwenzel/Github/SmashBros/initialization.txt");
}

/*
 * Once all of the required data is read in from the OFF file, it is necessary
 * to construct connectivity in our data structure
 */
void Mesh::buildConnectivity() {
    //std::cout << "Building Connectivity" << std::endl;
    
    /*
     * Vertex->Face Adjacency
     *      Loop through all the faces, add face to its vertices' vertex adjacencies
     */
    for (int i = 0; i < faces->size(); i++) {
        Face* currFace = faces->at(i);
        
        for (int j = 0; j < 3; j++) {
            Vertex* currVert = vertices->at(currFace->vertexIndices[j]);
            currVert->vertToFaceAdj->push_back(currFace);
        }
    }
    
    /*
     * Face->Face Adjacency:
     *      Two faces are adjacent if they share a common edge, i.e. they
     *      have 2 vertices in common.
     *
     * Pseudocode Outline:
     * 
     *      For each face:
     *          For each of its vertices:
     *              Loop through vertex's vertToFaceAdj:
     *                  If any of these faces share another vertex with original Face, it is
     *                  adjacent. Add faces to each other's faceToFaceAdj
     */
    for (int i = 0; i < faces->size(); i++) {
        Face* currFace = faces->at(i);
        for (int j = 0; j < 3; j++) {
            Vertex* currVert = vertices->at(currFace->vertexIndices[j]);
            
            // The other 2 vertexIndices that aren't j
            int j1 = (j+1)%3;
            int j2 = (j+2)%3;
            
            for (int k = 0; k < currVert->vertToFaceAdj->size(); k++) {
                Face* adjFace = currVert->vertToFaceAdj->at(k);
                if (currFace == adjFace) continue;
                
                // Check for another shared vertex besides current j
                for (int n = 0; n < 3; n++) {
                    // We know we are going to share at least one common vertex specified by index j
                    if (currFace->vertexIndices[j] == adjFace->vertexIndices[n]) {
                        continue;
                    }
                    else {
                        if (currFace->vertexIndices[j1] == adjFace->vertexIndices[n] ||
                            currFace->vertexIndices[j2] == adjFace->vertexIndices[n]) {
                            // Faces add each other to adjacencies
                            if (!checkDuplicateFaceAdj(currFace, adjFace)) {
                                currFace->faceToFaceAdj->push_back(adjFace);
                                adjFace->faceToFaceAdj->push_back(currFace);
                            }
                        }
                    }
                }
            }
        }
    }
    
    /*
     * Vert->Vert Adjacency
     */
    for (int i = 0; i < vertices->size(); i++) {
        Vertex* currVert = vertices->at(i);
        findAdjVertices(currVert);
    }
    std::cout << "Done Building Connectivity" << std::endl;
}

void Mesh::findAdjVertices(Vertex* currVert) {
    for (int j = 0; j < currVert->vertToFaceAdj->size(); j++) {
        Face* currFace = currVert->vertToFaceAdj->at(j);
        for (int k = 0; k < 3; k++) {
            Vertex* adjVert = vertices->at(currFace->vertexIndices[k]);
            if (currVert == adjVert) continue;
            if (!checkDuplicateVertToVertAdj(currVert, adjVert)) {
                currVert->vertToVertAdj->push_back(adjVert);
            }
        }
    }

}

// Call on midpoint
void Mesh::findAdjFaces(Vertex* currVert)
{
    for (int k = 0; k < currVert->vertToFaceAdj->size(); k++) {
        Face* currFace = currVert->vertToFaceAdj->at(k);
        
        for (int j = 0; j < currVert->vertToFaceAdj->size(); j++) {
            Face* adjFace = currVert->vertToFaceAdj->at(j);
            
            if (currFace == adjFace)
                continue;
            
            if (checkAdjacent(currFace, adjFace) == true) {
                if (!checkDuplicateFaceAdj(currFace, adjFace)) {
                    currFace->faceToFaceAdj->push_back(adjFace);
                    adjFace->faceToFaceAdj->push_back(currFace);
                }
            }
        }
    }
}

bool Mesh::checkAdjacent(Face* currFace, Face* adjFace)
{
    int count = 0;
    for (int i = 0; i < 3; i++) {
        int currInd = currFace->vertexIndices[i];
        for (int j = 0; j < 3; j++) {
            int adjInd = adjFace->vertexIndices[j];
            
            if (currInd == adjInd) {
                count++;
                break;
            }
        }
    }
    
    if (count == 2)
        return true;
    else
        return false;
}


void Mesh::removeFace(Face* deadFace)
{
    // Update faces that pointed to deadFace
    for (int i = (int)deadFace->faceToFaceAdj->size()-1; i >= 0; i--) {
        Face* adjFace = deadFace->faceToFaceAdj->at(i);
        
        for (int j = (int)adjFace->faceToFaceAdj->size()-1; j >= 0; j--) {
            Face* removeFace = adjFace->faceToFaceAdj->at(j);
            
            if (removeFace == deadFace) {
                adjFace->faceToFaceAdj->erase(adjFace->faceToFaceAdj->begin() + j);
                break;
            }
        }
    }
    
    // Update vertices that pointed to deadFace
    for (int i = 0; i < 3; i++) {
        Vertex* currVert = vertices->at(deadFace->vertexIndices[i]);
        for (int j = (int)currVert->vertToFaceAdj->size() - 1; j >= 0; j--) {
            Face* currFace = currVert->vertToFaceAdj->at(j);
            if (currFace == deadFace) {
                currVert->vertToFaceAdj->erase(currVert->vertToFaceAdj->begin() + j);
                break;
            }
        }
    }
}

/*
 * Check if these faces are in each other's face adjacencies
 */
bool Mesh::checkDuplicateFaceAdj(Face* f0, Face* f1) {
    for (int i = 0; i < f0->faceToFaceAdj->size(); i++) {
        if (f0->faceToFaceAdj->at(i) == f1) {
            return true;
        }
    }
    return false;
}

bool Mesh::checkDuplicateVertAdj(Vertex* v0, Face* f0)
{
    for (int i = 0; i < v0->vertToFaceAdj->size(); i++) {
        Face* currFace = v0->vertToFaceAdj->at(i);
        if (currFace == f0) {
            return true;
        }
    }
    return false;
}


bool Mesh::checkDuplicateVertToVertAdj(Vertex* v0, Vertex* v1) {
    for (int i = 0; i < v0->vertToVertAdj->size(); i++) {
        Vertex* currVert = v0->vertToVertAdj->at(i);
        if (currVert == v1) {
            return true;
        }
    }
    return false;
} 


void Mesh::computeFaceNormals() {
    std::cout << "Computing Face Normals" << std::endl;
    for (int i = 0; i < faces->size(); i++) {
        Face* currFace = faces->at(i);
        computeFaceNormal(currFace);
    }
    std::cout << "Done Computing Face Normals" << std::endl;
}

void Mesh::computeFaceNormal(Face* currFace) {
    Vertex* v0 = vertices->at(currFace->vertexIndices[0]);
    Vertex* v1 = vertices->at(currFace->vertexIndices[1]);
    Vertex* v2 = vertices->at(currFace->vertexIndices[2]);
    
    Vector3 a = *v1->coordinate - *v0->coordinate;
    Vector3 b = *v2->coordinate - *v0->coordinate;
    
    currFace->faceNormal = new Vector3((a.cross(b)).normalize());
}

void Mesh::computeVertexNormals() {
    std::cout << "Computing Vertex Normals" << std::endl;
    for (int i = 0; i < vertices->size(); i++) {
        Vertex* currVert = vertices->at(i);
        computeVertexNormal(currVert);
    }
    std::cout << "Done Computing Vertex Normals" << std::endl;
}

void Mesh::computeVertexNormal(Vertex* currVert) {
    Vector3 sum = Vector3(0.0, 0.0, 0.0);
    
    for (int j = 0; j < currVert->vertToFaceAdj->size(); j++) {
        sum = sum + *currVert->vertToFaceAdj->at(j)->faceNormal;
    }
    sum = sum.scale(1.0/currVert->vertToFaceAdj->size());
    sum.normalize();
    currVert->vertexNormal = new Vector3(sum);
}

void Mesh::computeKMatrices() {
    for (int i = 0; i < faces->size(); i++) {
        Face* currFace = faces->at(i);
        computeKMatrix(currFace);
    }
    std::cout << "Done Computing K Matrices" << std::endl;
}

void Mesh::computeKMatrix(Face* currFace) {
    Vector3 A = *vertices->at(currFace->vertexIndices[0])->coordinate;
    Vector3 normal = *currFace->faceNormal;
    
    
    Vector4 p(currFace->faceNormal->get(0),
              currFace->faceNormal->get(1),
              currFace->faceNormal->get(2),
              -(A.dot(normal)));
    
    float a = p.get(0);
    float b = p.get(1);
    float c = p.get(2);
    float d = p.get(3);
    
    
    // Compute Matrix K
    currFace->K.set(a*a, a*b, a*c, a*d,
                    a*b, b*b, b*c, b*d,
                    a*c, b*c, c*c, c*d,
                    a*d, b*d, c*d, d*d);
}

void Mesh::computeQMatrices() {
    for (int i = 0; i < vertices->size(); i++) {
        Vertex* currVert = vertices->at(i);
        computeQMatrix(currVert);
    }
    std::cout << "Done Computing Q Matrices" << std::endl;
}

void Mesh::computeQMatrix(Vertex* currVert) {
    for (int i = 0; i < currVert->vertToFaceAdj->size(); i++) {
        Face* currFace = currVert->vertToFaceAdj->at(i);
        currVert->Q = currVert->Q + currFace->K;
    }
}


void Mesh::computeVertexErrors() {
    for (int i = 0; i < vertices->size(); i++) {
        Vertex* currVert = vertices->at(i);
        computeVertexError(currVert);
        //std::cout << "Vertex Error: " << currVert->error << std::endl;
    }
    std::cout << "Done Computing Vertex Errors" << std::endl;
}

void Mesh::computeVertexError(Vertex* currVert) {
    Vector4 v = currVert->coordinate->toVector4(1.0);
    // error is vt*Q*v
    Vector4 temp = currVert->Q * v;
    currVert->error = v.dot(temp);
}

void Mesh::findAllPairs() {
    // All edges are valid pairs
    
    std::cout << "Beginning Finding All Pairs" << std::endl;
    for (int i = 0; i < vertices->size(); i++) {
        //std::cout << "Finding Pairs" << std::endl;
        Vertex* currVert = vertices->at(i);
        for (int j = 0; j < currVert->vertToVertAdj->size(); j++) {
            //std::cout << "Finding Pair" << std::endl;
            VertexPair* currPair = new VertexPair(currVert, currVert->vertToVertAdj->at(j));
            currVert->localPairs->push_back(currPair);
        }
    }
    
    std::cout << "Done finding Vertex Pairs" << std::endl;
    
    // TODO: Threshold Pairs
}

void Mesh::computePairCosts() {
    for (int i = 0; i < vertices->size(); i++) {
        Vertex* currVert = vertices->at(i);
        for (int j = 0; j < currVert->localPairs->size(); j++) {
            VertexPair* currPair = currVert->localPairs->at(j);
            computePairCost(currPair);
            pairs->insert(currPair); // insert into pairs heap
        }
        
    }
    std::cout << "Done Computing Pair Costs" << std::endl;
}

void Mesh::computePairCost(VertexPair* currPair) {
    Matrix4 QSum = currPair->a->Q + currPair->b->Q;
    
    /* TODO: Fix Optimal Vertex Computation
    
    // Find optimal vertex location for pair
    Matrix4 QSum = currPair->a->Q + currPair->b->Q;
    Matrix4 QSumCopy = QSum;
    Vector4 v(0, 0, 0, 1);
    QSum.setElement(3, 0, 0);
    QSum.setElement(3, 1, 0);
    QSum.setElement(3, 2, 0);
    QSum.setElement(3, 3, 1);
    
    Vector3* optimal = new Vector3((QSum.inverse() * v).toVector3());
    Vertex* currVert = new Vertex(optimal, QSumCopy); */
    Vector3* mid = new Vector3(((*currPair->a->coordinate) + (*currPair->b->coordinate)).scale(0.5));
    Vertex* currVert = new Vertex(mid, QSum);
    
    computeVertexError(currVert);
    currPair->error = currVert->error;
    currPair->collapsedVertex = currVert;
    
    /*
    std::cerr << "Optimal Vertex Between " << std::endl;
    currPair->a->coordinate->print("Vertex A");
    std::cerr << " and " <<  std::endl;
    currPair->b->coordinate->print("Vertex B");
    std::cerr << " is " << std::endl;
    optimal->print("Optimal"); */
    
    
    
    
    
    // Find error for optimal vertex
}

void Mesh::quadricSimplification() {
    // Get min Pair from heap
    VertexPair* currPair;
    while (1) {
        currPair = pairs->GetMin();
        if (currPair->a->valid == 1 && currPair->b->valid == 1) {
            break;
        }
        else {
            pairs->DeleteMin();
        }
    }
    std::cerr << "Collapsing:" << std::endl;
    currPair->a->coordinate->print("A");
    currPair->b->coordinate->print("B");
    std::cerr << "Error: " << currPair->error << std::endl;
    edgeCollapseAtMidpoint(currPair);
    pairs->DeleteMin();
    meshStatus("/Users/seanwenzel/Github/SmashBros/collapse.txt");
}

// Midpoint by default, can specify other destination vertex
void Mesh::edgeCollapseAtMidpoint(VertexPair* currPair) {
    Vertex* v0 = currPair->a;
    Vertex* v1 = currPair->b;
    Vector3 v0_pos = *v0->coordinate;
    Vector3 v1_pos = *v1->coordinate;
    
    v0->valid = false;
    v1->valid = false;
    
    Vector3* midpoint = new Vector3((v0_pos + v1_pos).scale(0.5));
    int vertIndex = (int)vertices->size();
    
    Vertex* midVert = new Vertex(midpoint);
    midVert->index = vertIndex;
    midVert->parent1 = v0->index;
    midVert->parent2 = v1->index;
    vertices->push_back(midVert);
    
    
    // Adjacent Faces for v0
    // Update vertex array of face to midpoint vertex
    for (int i = (int)v0->vertToFaceAdj->size()-1; i >= 0; i--) {
        Face* currFace = v0->vertToFaceAdj->at(i);
        for (int j = 0; j < 3; j++) {
            if (vertices->at(currFace->vertexIndices[j]) == v0) {
                currFace->vertexIndices[j] = vertIndex;
                if (currFace->isDegenerate()) {
                    removeFace(currFace);
                    v0->vertToFaceAdj->erase(v0->vertToFaceAdj->begin() + i);
                }
            }
        }
    }
    
    // Adjacent Faces for v1
    for (int i = (int)v1->vertToFaceAdj->size()-1; i >= 0; i--) {
        Face* currFace = v1->vertToFaceAdj->at(i);
        for (int j = 0; j < 3; j++) {
            if (vertices->at(currFace->vertexIndices[j]) == v1) {
                currFace->vertexIndices[j] = vertIndex;
                if (currFace->isDegenerate()) {
                    removeFace(currFace);
                    v1->vertToFaceAdj->erase(v1->vertToFaceAdj->begin() + i);
                }
            }
        }
    }
    
    /*
    // Remove Degenerate Faces in v0's Face Adjacency
    for (int i = (int)v0->vertToFaceAdj->size()-1; i >= 0; i--) {
        Face* currFace = v0->vertToFaceAdj->at(i);
        if (currFace->isDegenerate()) {
            removeFace(currFace); // removes face from it's neighbor's adjacency
            v0->vertToFaceAdj->erase(v0->vertToFaceAdj->begin() + i);
        }
    }
    
    // Remove Degenerate Faces in v1's Face Adjacency
    for (int i = (int)v1->vertToFaceAdj->size()-1; i >= 0; i--) {
        Face* currFace = v1->vertToFaceAdj->at(i);
        if (currFace->isDegenerate()) {
            removeFace(currFace);
            v1->vertToFaceAdj->erase(v1->vertToFaceAdj->begin() + i);
        }
    } */
    
    // Create midpoint vertex adjacent face list
    // Union of v0 and v1
    for (int i = 0; i < v0->vertToFaceAdj->size(); i++) {
        Face* currFace = v0->vertToFaceAdj->at(i);
        if (checkDuplicateVertAdj(midVert, currFace) == false) {
            midVert->vertToFaceAdj->push_back(currFace);
        }
    }
    
    for (int i = 0; i < v1->vertToFaceAdj->size(); i++) {
        Face* currFace = v1->vertToFaceAdj->at(i);
        if (checkDuplicateVertAdj(midVert, currFace) == false) {
            midVert->vertToFaceAdj->push_back(currFace);
        }
    }
    
    // Recalculate Face Normals
    for (int i = 0; i < midVert->vertToFaceAdj->size(); i++) {
        Face* currFace = midVert->vertToFaceAdj->at(i);
        computeFaceNormal(currFace);
    }
    
    /*
    // Update v0 and v1 vertToVertAdj
    for (int i = (int)v0->vertToVertAdj->size() - 1; i >= 0; i--) {
        Vertex* currVert = v0->vertToVertAdj->at(i);
        for (int j = (int)currVert->vertToVertAdj->size()-1; j >= 0; j--) {
            if (currVert->vertToVertAdj->at(j) == v0) {
                if (!checkDuplicateVertToVertAdj(currVert, midVert)) {
                    currVert->vertToVertAdj->at(j) = midVert;
                }
                else {
                    // Delete currVert
                    currVert->vertToVertAdj->erase(currVert->vertToVertAdj->begin() + j);
                }
                break;
            }
        }
    }
    
    for (int i = (int)v1->vertToVertAdj->size() - 1; i >= 0; i--) {
        Vertex* currVert = v1->vertToVertAdj->at(i);
        for (int j = (int)currVert->vertToVertAdj->size()-1; j >= 0; j--) {
            if (currVert->vertToVertAdj->at(j) == v1) {
                if (!checkDuplicateVertToVertAdj(currVert, midVert)) {
                    currVert->vertToVertAdj->at(j) = midVert;
                    break;
                }
                else {
                    // Delete currVert
                    currVert->vertToVertAdj->erase(currVert->vertToVertAdj->begin() + j);
                }
            }
        }
    } */
    
    // Construct VertToVertAdj2
    for (int i = 0; i < v0->vertToVertAdj->size(); i++) {
        Vertex* currVert = v0->vertToVertAdj->at(i);
        if (!checkDuplicateVertToVertAdj(midVert, currVert) && (currVert != midVert) &&
            currVert->valid == true) {
            midVert->vertToVertAdj->push_back(currVert);
        }
    }
    
    for (int i = 0; i < v1->vertToVertAdj->size(); i++) {
        Vertex* currVert = v1->vertToVertAdj->at(i);
        if (!checkDuplicateVertToVertAdj(midVert, currVert) && (currVert != midVert) &&
            currVert->valid == true) {
            midVert->vertToVertAdj->push_back(currVert);
        }
    }
    
    // Construct VertToVertAdj for midVert
    //findAdjVertices(midVert);
    // Calculate vertex normal for midVert
    computeVertexNormal(midVert);
    
    // Recalculate Vertex Normals for neighboring vertices
    for (int i = 0; i < midVert->vertToVertAdj->size(); i++) {
        Vertex* currVert = midVert->vertToVertAdj->at(i);
        computeVertexNormal(currVert);
    }
        
    // Update adjacent faces (call on mid vertex)
    findAdjFaces(midVert);
    
    midVert->valid = true;
    midVert->Q = v0->Q + v1->Q;
    
    /* Updating the Pairs Heap */
    for (int i = 0; i < v0->localPairs->size(); i++) {
        VertexPair* currPair = v0->localPairs->at(i);
        updatePair(currPair, v0, midVert);
    }
    
    for (int i = 0; i < v1->localPairs->size(); i++) {
        VertexPair* currPair = v1->localPairs->at(i);
        updatePair(currPair, v1, midVert);
    }
    
    // Compute new errors for relevant pairs
    for (int i = 0; i < midVert->localPairs->size(); i++) {
        VertexPair* currPair = midVert->localPairs->at(i);
        computePairCost(currPair);
    }
}

void Mesh::updatePair(VertexPair* currPair, Vertex* oldVert, Vertex* newVert) {
    //currPair->replaceVertex(oldVert, newVert);
    
    // Get corresponding pair to currPair
    Vertex* otherVert;
    if (currPair->a == oldVert) {
        otherVert = currPair->b;
    }
    else if (currPair->b == oldVert) {
        otherVert = currPair->a;
    }
    else {
        std::cerr << "Error finding corresponding Vertex" << std::endl;
        return;
    }
    for (int i = (int)otherVert->localPairs->size()-1; i >= 0; i--) {
        VertexPair* otherPair = otherVert->localPairs->at(i);
        if (otherPair->equals(*currPair)) {
            // Have both pairs now
            currPair->replaceVertex(oldVert, newVert);
            otherPair->replaceVertex(oldVert, newVert);
            
            // Check for and delete duplicates
            bool flag = true;
            for (int j = (int)otherVert->localPairs->size()-1; j >= 0; j--) {
                VertexPair* pair = otherVert->localPairs->at(j);
                if (pair->equals(*otherPair)) {
                    // Don't delete if only one
                    if (flag) {
                        flag = false;
                        //continue;
                    }
                    else {
                        otherVert->localPairs->erase(otherVert->localPairs->begin() + j);
                    }
                }
            }
            
            if (currPair->a->valid == true && currPair->b->valid == true &&
                !newVert->checkPairDuplicate(currPair)) {
                newVert->localPairs->push_back(currPair);
            }
            
            return;
            
        }
    }
}

bool Face::isDegenerate() {
    if (vertexIndices[0] == vertexIndices[1] ||
        vertexIndices[1] == vertexIndices[2] ||
        vertexIndices[0] == vertexIndices[2]) {
        return true;
    }
    return false;
}

void Vertex::printCoordinate(ofstream& oFile) {
    oFile << "Coordinate: (" << coordinate->get(0) << ", " << coordinate->get(1) << ", " << coordinate->get(2) << ")";
}
void Vertex::printNormal(ofstream& oFile) {
    oFile << "Vertex Normal: (" << vertexNormal->get(0) << ", " << vertexNormal->get(1) << ", " << vertexNormal->get(2) << ")" << endl;
}
void Vertex::printError(ofstream& oFile) {
    oFile << "Vertex Error: " << error << endl;
}
void Vertex::printAdjVertices(ofstream& oFile) {
    oFile << "Adjacent Vertices" << endl;
    oFile << "# of Adjacent Vertices: " << vertToVertAdj->size() << endl;
    for (int i = 0; i < vertToVertAdj->size(); i++) {
        oFile << "Vertex #" << i << " | ";
        vertToVertAdj->at(i)->printIndex(oFile);
        oFile << " | ";
        vertToVertAdj->at(i)->printCoordinate(oFile);
        oFile << endl;
    }
}
void Vertex::printAdjFaces(ofstream& oFile, vector<Vertex*>* vertices) {
    oFile << "Adjacent Faces" << endl;
    oFile << "# of Adjacent Faces: " << vertToFaceAdj->size() << endl;
    for (int i = 0; i < vertToFaceAdj->size(); i++) {
        oFile << "Face #" << i << endl;
        vertToFaceAdj->at(i)->printVertexCoordinates(oFile, vertices);
    }
}
void Vertex::printPairs(ofstream& oFile) {
    oFile << "Vertex Pairs" << endl;
    oFile << "# of Vertex Pairs: " << localPairs->size() << endl;
    for (int i = 0; i < localPairs->size(); i++) {
        VertexPair* currPair = localPairs->at(i);
        oFile << "Pair #" << i << " | ";
        currPair->printVertexPair(oFile);
    }
}

void Vertex::printIndex(ofstream& oFile) {
    oFile << "Index: " << index;
}

void VertexPair::printVertexPair(ofstream& oFile) {
    oFile << "Vertex A: ";
    a->printIndex(oFile);
    oFile << " | ";
    a->printCoordinate(oFile);
    oFile << " | Vertex B: ";
    b->printIndex(oFile);
    oFile << " | ";
    b->printCoordinate(oFile);
    oFile << endl;
    oFile << "\tPair Error: " << error << endl;


}

void Face::printVertexCoordinates(ofstream& oFile, std::vector<Vertex*>* vertices) {
    for (int i = 0; i < 3; i++) {
        oFile << "Vertex #" << i << " | Index: " << vertexIndices[i] << " | ";
        vertices->at(vertexIndices[i])->printCoordinate(oFile);
        oFile << endl;
    }
}

int Mesh::countValidVertices() {
    int ctr = 0;
    for (int i = 0; i < vertices->size(); i++) {
        Vertex* currVert = vertices->at(i);
        if (currVert->valid == true) {
            ctr++;
        }
    }
    return ctr;
}

void Mesh::meshStatus(string path) {
    ofstream oFile;
    oFile.open(path);
    
    oFile << "Vertices" << endl;
    oFile << endl;
    oFile << "Total # of Vertices: " << vertices->size() << endl;
    oFile << "Valid # of Vertices: " << countValidVertices() << endl;
    for (int i = 0; i < vertices->size(); i++) {
        Vertex* currVert = vertices->at(i);
        oFile << "vertices[" << i << "]:" << endl;
        oFile << "------------" << endl;
        oFile << "Valid: " << currVert->valid << endl;
        oFile << "Parents: " << currVert->parent1 << ", " << currVert->parent2 << endl;
        oFile << endl;
        currVert->printCoordinate(oFile);
        oFile << endl;
        oFile << endl;
        currVert->printNormal(oFile);
        oFile << endl;
        currVert->printError(oFile);
        oFile << endl;
        currVert->printAdjVertices(oFile);
        oFile << endl;
        currVert->printAdjFaces(oFile, vertices);
        oFile << endl;
        currVert->printPairs(oFile);
        oFile << endl;
    }
    
    oFile << "Heap" << endl;
    oFile << endl;
    oFile << "Total # of Pairs: " << pairs->size() << endl;
    for (int i = 0; i < pairs->size(); i++) {
        VertexPair* currPair = pairs->at(i);
        oFile << "Pair #" << i << endl;
        currPair->printVertexPair(oFile);
    }
    oFile << endl;
    
    
    VertexPair* currMinPair = pairs->GetMin();
    oFile << "Current Min Pair" << endl;
    currMinPair->printVertexPair(oFile);
    oFile << endl;
    oFile.close();
}

bool Vertex::checkPairDuplicate(VertexPair* pair) {
    for (int i = 0; i < localPairs->size(); i++) {
        VertexPair* currPair = localPairs->at(i);
        if (currPair->equals(*pair)) {
            return true;
        }
    }
    return false;
}

void Mesh::getCenter()
{
    centerX = (maxX + minX)/2;
    centerY = (maxY + minY)/2;
    centerZ = (maxZ + minZ)/2;
}

void Mesh::getHalfSize()
{
    float halfSizeX = (maxX - minX)/2;
    float halfSizeY = (maxY - minY)/2;
    float halfSizeZ = (maxZ - minZ)/2;
    halfSizeMAX = fmax(fmax(halfSizeX, halfSizeY), halfSizeZ);
}

//Split functions from the interwebs
//http://stackoverflow.com/questions/236129/split-a-string-in-c
std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
