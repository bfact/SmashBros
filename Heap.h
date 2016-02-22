//
//  Heap.h
//  SmashBros
//
//  Created by Sean Wenzel on 2/15/16.
//  Copyright © 2016 Sean Wenzel. All rights reserved.
//

#ifndef Heap_h
#define Heap_h

#include <stdio.h>
#include <vector>
#include <unordered_set>
#include <queue>
#include "Mesh.h"

using namespace std;
struct VertexPair;
struct Vertex;



class Heap
{
private:
    //std::priority_queue<VertexPair*, std::vector<VertexPair*>, VertexPair::Compare >* queue;
    
    /*
    vector<VertexPair*>* heapVector;
    void BubbleDown(int index);
    void BubbleUp(int index);
    void Heapify(); */
    
public:
    /*
    inline VertexPair* operator[](const int index) {
        return heapVector->at(index);
    }
    
    Heap(vector<VertexPair*>* vector);
    Heap();
    
    void insert(VertexPair* newPair);
    VertexPair* GetMin();
    void DeleteMin();
    int size();
    VertexPair* at(int);
    void updateHeap(VertexPair*, Vertex*); */
    Heap();
    VertexPair* GetMin();
    void insert(VertexPair* newPair);
    void DeleteMin();
    int size();
};

#endif /* Heap_h */
