#include <GL/freeglut.h>

// Geometry

typedef struct SizeI {
    int height, width;
} SizeI;

typedef struct PointI {
    int row, column;
} PointI;

typedef struct SizeF {
    float height, width;
} SizeF;

typedef struct PointF {
    float x, y;
} PointF;

typedef struct Quad {
    PointF bottomLeft, topLeft, topRight, bottomRight;
} Quad;

Quad makeSquareFromBottomLeft(PointF corner, SizeF size) {
    Quad q = {
        { corner.x, corner.y },
        { corner.x, corner.y + size.height },
        { corner.x + size.width, corner.y + size.height },
        { corner.x + size.width, corner.y }
    };
    return q;
}

Quad makeSquareFromCenter(PointF center, float size) {
    float half = size / (float) 2;
    Quad q = {
        { center.x - half, center.y + half },
        { center.x + half, center.y + half },
        { center.x + half, center.y - half },
        { center.x - half, center.y - half }
    };
    return q;
}

// Colors

typedef struct Color {
    float r,g,b;
} Color;

Color RED =     {1.0f, 0.0f, 0.0f};
Color GREEN =   {0.0f, 1.0f, 0.0f};
Color BLUE =    {0.0f, 0.0f, 1.0f};

// Render

void setColor(Color *c) {
    glColor3f(c->r, c->g, c->b);
}

// Window

SizeF WINDOW_SIZE = {500.0f, 500.0f};

void drawQuad(Quad *q) {
    glVertex2f(q->bottomLeft.x, q->bottomLeft.y);
    glVertex2f(q->topLeft.x, q->topLeft.y);
    glVertex2f(q->topRight.x, q->topRight.y);
    glVertex2f(q->bottomRight.x, q->bottomRight.y);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, WINDOW_SIZE.width, 0, WINDOW_SIZE.height, -1, 1); // Set orthographic projection with viewport size of 500
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    PointF bl = {100.0f, 100.0f};
    SizeF s = {30.0f, 30.0f};
    Quad q = makeSquareFromBottomLeft(bl, s);

    glBegin(GL_QUADS);
    setColor(&BLUE);
    drawQuad(&q);
    
    glEnd();
    
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(WINDOW_SIZE.width, WINDOW_SIZE.height);
    glutCreateWindow("Life simulator");
    glutDisplayFunc(display);
    glutMainLoop();
    
    return 0;
}
