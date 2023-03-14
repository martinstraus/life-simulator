#include <stdio.h>
#include <stdlib.h>
#include "world.h"

World* newWorld(SizeI size) {
    World* world = malloc(sizeof(World));
    world->size = size;
    world->locations = malloc(sizeof(Medium) * size.height * size.width);
    for (int row = 0; row < size.height; row++) {
        for (int column = 0; column < size.width; column++) {
        }
    }
    return world;
}

void renderWorld(World* world) {
    printf("World\n");
}