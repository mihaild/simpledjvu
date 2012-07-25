#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include "types.h"
#include "constants.h"
#include "pgm.h"
#include "hystograms.h"
#include "disjoint_set_forest.h"
#include "connected_components.h"

using std::unordered_map;
using std::vector;
using std::min;
using std::max;
using std::pair;

int main(int argc, char *argv[]) {
    byte *pixels;
    int32 width, height, row_size, rows_count;
    FILE *data = fopen(argv[1], "rb");

    load_pgm(data, &width, &height, &row_size, &rows_count, &pixels, 0);

    fclose(data);

    ConnectedComponentForest connected_components_forest = build_connected_components_forest(pixels, width, height);

    std::cerr << "saving\n";
    connected_components_forest.save("forest/");

    return 0;
}
