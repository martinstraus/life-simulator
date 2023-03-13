#ifndef WORLD_H
#define WORLD_H

#include "geometry.h"

struct Genome {

};

typedef struct Genome Genome;

struct Organism {
    Point location;
    Genome genome;
};

typedef struct Organism Organism;

struct Medium {
    Point location;
};

typedef struct Medium Medium;

struct World {
    Size size;
    Medium* locations;
    Organism* organisms;
};

typedef struct World World;

World* newWorld();
void renderWorld(World* world);

#endif