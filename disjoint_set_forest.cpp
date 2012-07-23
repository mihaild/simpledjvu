#include "disjoint_set_forest.h"

DisjointSetForest::DisjointSetForest(int size): parent(size), rank(size, 0) {
    for (int i = 0; i < size; ++i) {
        parent[i] = i;
    }
}

inline int DisjointSetForest::find(int x) {
    if (parent[x] != x) {
        parent[x] = find(parent[x]);
    }
    return parent[x];
}

void DisjointSetForest::unite(int x, int y) {
    x = find(x);
    y = find(y);
    if (x == y) {
        return;
    }
    if (rank[x] < rank[y]) {
        parent[x] = y;
    }
    else {
        parent[y] = x;
        if (rank[x] == rank[y]) {
            ++rank[x];
        }
    }
}

inline int DisjointSetForest::operator[] (int x) {
    return find(x);
}
