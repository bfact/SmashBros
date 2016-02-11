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
    //this->colors = new std::vector<Vector3*>();
    
    parse(filename);
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
    srand(1);
    
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
}

/*
 * Once all of the required data is read in from the OFF file, it is necessary
 * to construct connectivity in our data structure
 */
void Mesh::buildConnectivity() {
    std::cout << "Building Connectivity" << std::endl;
    
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
    for (int i = 0; i < deadFace->faceToFaceAdj->size(); i++) {
        Face* adjFace = deadFace->faceToFaceAdj->at(i);
        
        for (int j = 0; j < adjFace->faceToFaceAdj->size(); j++) {
            Face* removeFace = adjFace->faceToFaceAdj->at(j);
            
            if (removeFace == deadFace) {
                adjFace->faceToFaceAdj->erase(adjFace->faceToFaceAdj->begin() + j);
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

void Mesh::edgeCollapse(Vertex* v0, Vertex* v1) {
    Vector3 v0_pos = *v0->coordinate;
    Vector3 v1_pos = *v1->coordinate;
    
    Vector3 midpoint = (v0_pos + v1_pos).scale(0.5);
    int vertIndex = (int)vertices->size();
    
    Vertex* midVert = new Vertex;
    midVert->coordinate = &midpoint;
    vertices->push_back(midVert);
    
    // Adjacent Faces for v0
    // Update vertex array of face to midpoint vertex
    for (int i = 0; i < v0->vertToFaceAdj->size(); i++) {
        Face* currFace = v0->vertToFaceAdj->at(i);
        for (int j = 0; j < 3; j++) {
            if (vertices->at(currFace->vertexIndices[j]) == v0) {
                currFace->vertexIndices[j] = vertIndex;
            }
        }
    }
    
    // Adjacent Faces for v1
    for (int i = 0; i < v1->vertToFaceAdj->size(); i++) {
        Face* currFace = v1->vertToFaceAdj->at(i);
        for (int j = 0; j < 3; j++) {
            /*
            if (currFace->vertexIndices[j] == vertIndex)
                continue; */
            
            if (vertices->at(currFace->vertexIndices[j]) == v1) {
                currFace->vertexIndices[j] = vertIndex;
            }
        }
    }
    
    // Remove Degenerate Faces in v0's Face Adjacency
    for (int i = 0; i < v0->vertToFaceAdj->size(); i++) {
        Face* currFace = v0->vertToFaceAdj->at(i);
        int i0 = currFace->vertexIndices[0];
        int i1 = currFace->vertexIndices[1];
        int i2 = currFace->vertexIndices[2];
        
        // If at least 2 of the above indices are equal, then the face is degenerate
        if (i0 == i1 || i1 == i2 || i0 == i2) {
            // TODO: Need FaceToFaceAdjacency
            removeFace(currFace);
        }
    }
    
    // Remove Degenerate Faces in v1's Face Adjacency
    for (int i = 0; i < v1->vertToFaceAdj->size(); i++) {
        Face* currFace = v1->vertToFaceAdj->at(i);
        int i0 = currFace->vertexIndices[0];
        int i1 = currFace->vertexIndices[1];
        int i2 = currFace->vertexIndices[2];
        
        // If at least 2 of the above indices are equal, then the face is degenerate
        if (i0 == i1 || i1 == i2 || i0 == i2) {
            // TODO: Need FaceToFaceAdjacency
            removeFace(currFace);
        }
    }
    
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
    
    // Construct VertToVertAdj for midVert
    findAdjVertices(midVert);
    // Calculate vertex normal for midVert
    computeVertexNormal(midVert);
    
    // Recalculate Vertex Normals for neighboring vertices
    for (int i = 0; i < midVert->vertToVertAdj->size(); i++) {
        Vertex* currVert = midVert->vertToVertAdj->at(i);
        computeVertexNormal(currVert);
    }
        
    // Update adjacent faces (call on mid vertex)
    findAdjFaces(midVert);
    
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
