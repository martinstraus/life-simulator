#include <stdio.h>
#include <GL/glew.h>
#include "GL/freeglut.h"
#include "GL/gl.h"
#include "geometry.h"
#include "world.h"


#define QUADS_COUNT 2

SizeF WINDOW_SIZE = {500, 500};

void drawQuad(Quad *q) {
    glBegin(GL_QUADS);
    glVertex2f(q->a.x, q->a.y);
    glVertex2f(q->b.x, q->b.y);
    glVertex2f(q->c.x, q->c.y);
    glVertex2f(q->d.x, q->d.y);
    glEnd();

}

Quad quads[QUADS_COUNT];
World* world;

void display()
{
    glClearColor(0.4, 0.4, 0.4, 0.4);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    for (int i = 0; i < QUADS_COUNT; i++) {
        Quad *q = &(quads[i]);
        drawQuad(q);
    }
    renderWorld(world);
    glFlush();
}

void releaseEverything() {
    free(world);
}

int main(int argc, char **argv)
{
    SizeI worldSize = {100, 100};
    world = newWorld(worldSize);
    PointF p1 = {-0.5, -0.5};
    quads[0] = makeSquare(p1, 0.1);
    PointF p2 = {0.5, 0.5};
    quads[1] = makeSquare(p2, 0.1);

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