#pragma once

#include <vector>

const int MIN_LEVEL = 10;
const int MAX_LEVEL = 250;
const int LEVEL_STEP = 15;
const int LEVELS = (MAX_LEVEL - MIN_LEVEL) / LEVEL_STEP + 1;

const int MIN_WIDTH = 4;
const int MIN_HEIGHT = 4;

using std::vector;

struct ConnectedComponent {
    int left;
    int right;
    int top;
    int bottom;
    int color;
    bitonal_image form;
    ConnectedComponent *parent;
    vector<ConnectedComponent *> childs;
    void save(FILE *file);
    ConnectedComponent();
};

vector<vector<ConnectedComponent *> > build_connected_components_forest(byte *pixels, int width, int height);

void save_component(ConnectedComponent component, std::string path);
