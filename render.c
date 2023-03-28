#include <GL/glew.h>
#include "GL/freeglut.h"
#include "GL/gl.h"
#include "geometry.h"

void drawQuad(Quad *q) {
    glBegin(GL_QUADS);
    glVertex2f(q->bottomLeft.x, q->bottomLeft.y);
    glVertex2f(q->topLeft.x, q->topLeft.y);
    glVertex2f(q->topRight.x, q->topRight.y);
    glVertex2f(q->bottomRight.x, q->bottomRight.y);
    glEnd();
}