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

#include <ctime>

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

    clock_t tmp(clock());
    ConnectedComponentForest connected_components_forest = build_connected_components_forest(pixels, width, height);
    std::cout << "build forest: " << static_cast<double>(clock() - tmp) / CLOCKS_PER_SEC << '\n';
    tmp = clock();

    /*std::cerr << "saving\n";
    connected_components_forest.save("forest/");*/

    vector<ConnectedComponent *> ok_components = connected_components_forest.get_best_subset();
    std::cerr << "ok components: " << ok_components.size() << '\n';
    std::cout << "best subset: " << static_cast<double>(clock() - tmp) / CLOCKS_PER_SEC << '\n';
    tmp = clock();

    bitonal_image image = bitonal_image(height, vector<bool> (width, false));

    place_components(ok_components, image);
    std::cout << "place components: " << static_cast<double>(clock() - tmp) / CLOCKS_PER_SEC << '\n';
    tmp = clock();

    save_pbm(fopen("good_components.pbm", "wb"), image);

    return 0;
}
