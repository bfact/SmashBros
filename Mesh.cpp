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
    this->edges = new std::map<int, Edge>();
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
    
    for (int i = 0; i < faces->size(); i++) {
        face = faces->at(i);
        for (int j = 0; j < 3; j++) {
            
            /*
            Vector3 vn = *normals->at(face->normalIndices[j]);
            vn = vn.normalize();
            glNormal3f(vn[0], vn[1], vn[2]); */
            
            /*
            if (color) {
                Vector3 c = *colors->at(face->vertexIndices[j]);
                glColor4f(c[0], c[1], c[2], 0.5);
            } */

            Vertex* v  = vertices->at(face->vertexIndices[j]);
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
    Drawable::getHalfSize(maxX, maxY, maxZ, minX, minY, minZ);
    
    for (int i = 0; i < vertices->size(); i++) {
        Vector3* v  = vertices->at(i)->coordinate;
        float x = v->get(0) - centerX;
        float y = v->get(1) - centerY;
        float z = v->get(2) - centerZ;
        *v = Vector3(x,y,z);
    }
    
    //computeEdges();
    //computeFaceNormals();
    buildConnectivity();
}

/*
 * Check if edge exists in edge map
 */
int Mesh::containsValue(Edge e) {
    std::map<int, Edge>::iterator it;
    bool found = false;
    while ( it != edges->end()) {
        if (it->first) continue;
        found = (it->second == e);
        if (found) {
            return it->first;
        }
        ++it;
    }
    return(-1);
}

/*
 * Find first available hole in edge map
 */
int Mesh::findHole() {
    int freeIndex = 0;
    std::map<int, Edge>::iterator it;
    std::map<int, Edge>::iterator end;
    for (it = edges->begin(), end = edges->end();
         it != end && freeIndex == it->first; ++it, ++freeIndex) {}
    return freeIndex;
}

// Create an indexed set of edges (similar to vertices) associated with faces
static int edgeIndex = 0;
void Mesh::computeEdges() {
    std::cout << "Computing Edges" << std::endl;
    for (int i = 0; i < faces->size(); i++) {
        int edgeIndex0, edgeIndex1, edgeIndex2;
        
        // Get Face
        Face* currFace = faces->at(i);
        
        Vertex v0 = *vertices->at(currFace->vertexIndices[0]);
        Vertex v1 = *vertices->at(currFace->vertexIndices[1]);
        Vertex v2 = *vertices->at(currFace->vertexIndices[2]);
        
        Edge e0(v0, v1);
        Edge e1(v1, v2);
        Edge e2(v2, v0);
        
        edgeIndex0 = containsValue(e0);
        // Wasn't found
        if (edgeIndex0 == -1) {
            edgeIndex0 = findHole();
            edges->insert(std::pair<int, Edge>(edgeIndex0, e0));
        }
        
        edgeIndex1 = containsValue(e1);
        // Wasn't found
        if (edgeIndex1 == -1) {
            edgeIndex1 = findHole();
            edges->insert(std::pair<int, Edge>(edgeIndex1, e1));
        }
        
        edgeIndex0 = containsValue(e0);
        // Wasn't found
        if (edgeIndex2 == -1) {
            edgeIndex2 = findHole();
            edges->insert(std::pair<int, Edge>(edgeIndex2, e2));
        }
        
        
        currFace->edgeIndices[0] = edgeIndex0;
        currFace->edgeIndices[1] = edgeIndex1;
        currFace->edgeIndices[2] = edgeIndex2;
    }
    std::cout << "Done Computing Edges" << std::endl;
}

void Mesh::computeFaceNormals() {
    
}

/*
 * Once all of the required data is read in from the OFF file, it is necessary
 * to construct connectivity in our data structure
 */
void Mesh::buildConnectivity() {
    // What I have to work with: Faces and Vertices
    std::cout << "Building Connectivity" << std::endl;
    /*
     * Vertex->Face Adjacency
     *      Loop through all the faces, add face to its vertices' vertex adjacencies
     */
    for (int i = 0; i < faces->size(); i++) {
        Face* currFace = faces->at(i);
        
        for (int j = 0; j < 3; j++) {
            Vertex* currVert = vertices->at(currFace->vertexIndices[j]);
            currVert->faceAdj->push_back(currFace);
        }
    }
    std::cout << "Done Building Connectivity" << std::endl;
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
