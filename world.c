#include <stdio.h>
#include <stdlib.h>
#include "world.h"

void initWorld(World *world, SizeI size) {
    world->size.height = size.height;
    world->size.width = size.width;
    world->medium = (Medium **) malloc(world->size.height * (sizeof(Medium *)));
    for (int row = 0; row < world->size.height; row++) {
        world->medium[row] = (Medium *) malloc(world->size.width * sizeof(Medium));
        for (int column = 0; column < world->size.width; column++) {
            Medium *m = &(world->medium[row][column]);
            m->location.row = row;
            m->location.column = column;
        }
    }
    
}



void renderWorld(World* world) {
    printf("World [%d,%d]\n", world->size.height, world->size.width);
}