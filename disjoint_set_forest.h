#pragma once

#include <vector>

using std::vector;

class DisjointSetForest {
    private:
        vector<int> parent;
        vector<int> rank;
    public:
        DisjointSetForest(int size);
        int find(int x);
        void unite(int x, int y);
        int operator[] (int x);
};
