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
#include "Mesh.h"

using namespace std;
struct VertexPair;

class Heap
{
private:
    vector<VertexPair*>* heapVector;
    void BubbleDown(int index);
    void BubbleUp(int index);
    void Heapify();
    
public:
    
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
};

#endif /* Heap_h */
