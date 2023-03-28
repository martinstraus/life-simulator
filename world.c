#include <stdio.h>
#include <stdlib.h>
#include "world.h"
#include "render.h"
#include "GL/gl.h"

void setColor(MediumType type) {
    switch (type) {
        case DIRT:
            glColor3f(0.58, 0.29, 0.09);
            break;
        case WATER:
            glColor3f(0.0, 0.5, 1.0);
            break;
        case GRASS:
            glColor3f(0.0, 0.7, 0.3);
            break;
        case ROCK:
            glColor3f(0.5, 0.5, 0.5);
            break;
    }
}

void initWorld(World *world, SizeI size) {
    world->size.height = size.height;
    world->size.width = size.width;
    world->medium = (Medium **) malloc(world->size.height * sizeof(Medium *));
    for (int row = 0; row < world->size.height; row++) {
        world->medium[row] = (Medium *) malloc(world->size.width * sizeof(Medium));
        for (int column = 0; column < world->size.width; column++) {
            Medium *m = &(world->medium[row][column]);
            m->location.row = row;
            m->location.column = column;
            m->type =  (MediumType) rand() % 4;
        }
    }
}

void renderWorld(World *world, WorldView *view) {
    for (int row = 0; row < world->size.height; row++) {
        for (int column = 0; column < world->size.width; column++) {
            setColor(world->medium[row][column].type);
            drawQuad(&(view->medium[row][column].shape));
        }
    }
    printf("World [%d,%d]\n", world->size.height, world->size.width);
}

void initView(World *world, WorldView *view) {
    SizeF mediumSize = { 
        view->size.height / world->size.height, 
        view->size.width / world->size.width 
    };
    view->medium = (MediumView **) malloc(world->size.height * sizeof(MediumView *));
    for (int row = 0; row < world->size.height; row++) {
        view->medium[row] = (MediumView *) malloc(world->size.width * sizeof(MediumView));
        for (int column = 0; column < world->size.width; column++) {
            PointF bottomLeftCorner = { column * mediumSize.width, row * mediumSize.height };
            MediumView *v = &(view->medium[row][column]);
            v->shape = makeSquareFromBottomLeft(bottomLeftCorner, mediumSize);
        }
    }
}


