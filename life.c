#include <stdio.h>
#include <GL/glew.h>
#include "GL/freeglut.h"
#include "GL/gl.h"
#include "geometry.h"
#include "world.h"
#include "render.h"
#include <time.h>

#define QUADS_COUNT 2

SizeF WINDOW_SIZE = {500, 500};

Quad quads[QUADS_COUNT];
World world;
WorldView view;

void display()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    for (int row = 0; row < world.size.height; row++) {
        for (int column = 0; column < world.size.width; column++) {
            Quad *shape = &(view.medium[row][column].shape);
            drawQuad(shape);
        }
    }
    renderWorld(&world, &view);
    glFlush();
}

void releaseEverything() {
    free(world.medium);
    free(view.medium);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    world = (World) {};
    view = (WorldView) {};
    initWorld(&world, (SizeI) {100, 100});
    initView(&world, &view);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(WINDOW_SIZE.width, WINDOW_SIZE.height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Life");
    glutDisplayFunc(display);

    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        releaseEverything();
        return 1;
    }

    glutMainLoop();
    releaseEverything();
    return 0;
}