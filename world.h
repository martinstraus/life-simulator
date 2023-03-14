#ifndef WORLD_H
#define WORLD_H

#include "geometry.h"

struct Genome {

};

typedef struct Genome Genome;

struct Organism {
    PointI location;
    Genome genome;
};

typedef struct Organism Organism;

struct Medium {
    PointI location;
};

typedef struct Medium Medium;

struct World {
    SizeI size;
    Medium** locations;
    Organism* organisms;
};

typedef struct World World;

World* newWorld(SizeI size);
void renderWorld(World* world);

#endif