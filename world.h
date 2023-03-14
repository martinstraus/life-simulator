#ifndef WORLD_H
#define WORLD_H

#include "geometry.h"

typedef struct Genome {

} Genome;

typedef struct Organism {
    PointI location;
    Genome genome;
} Organism;

typedef struct Medium {
    PointI location;
} Medium;

typedef struct World {
    SizeI size;
    Medium** locations;
    Organism* organisms;
} World;

World* newWorld(SizeI size);
void renderWorld(World* world);

#endif