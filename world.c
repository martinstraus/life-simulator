#include <stdio.h>
#include <stdlib.h>
#include "world.h"

World* newWorld() {
    return malloc(sizeof(World));
}

void renderWorld(World* world) {
    printf("World\n");
}