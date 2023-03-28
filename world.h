#ifndef WORLD_H
#define WORLD_H

#include "geometry.h"

typedef struct Genome {

} Genome;

typedef struct Organism {
    PointI location;
    Genome genome;
} Organism;

typedef enum MediumType { DIRT = 0, WATER = 1, GRASS = 2, ROCK = 3 } MediumType;

typedef struct Medium {
    PointI location;
    MediumType type;
} Medium;

typedef struct World {
    SizeI size;
    Medium **medium;
    Organism* organisms;
} World;

void initWorld(World * world, SizeI size);

typedef struct MediumView {
    Quad shape;
} MediumView;

typedef struct WorldView {
    SizeF size;
    MediumView **medium;
} WorldView;

void initView(World *world, WorldView *view);
void renderWorld(World *world, WorldView *view);

#endif