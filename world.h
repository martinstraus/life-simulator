#ifndef WORLD_H
#define WORLD_H

#include "geometry.h"

typedef struct Genome {

} Genome;

typedef struct Organism {
    PointI location;
    Genome genome;
} Organism;

typedef enum MediumType { Dirt = 0, Water = 1, Grass = 2, Rock = 3 } MediumType;

typedef struct Medium {
    PointI location;
    MediumType type;
} Medium;

typedef struct LocationsMatrix {
    SizeI size;
    Medium** locations;
} LocationsMatrix;

typedef struct World {
    SizeI size;
    Medium **medium;
    Organism* organisms;
} World;

void initWorld(World * world, SizeI size);
void renderWorld(World* world);

#endif