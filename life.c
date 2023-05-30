#include <GL/freeglut.h>
#define SQUARE_COUNT 3

// Colors

typedef struct Color {
    float r,g,b;
} Color;

void setColor(Color *c) {
    glColor3f(c->r, c->g, c->b);
}

Color RED =     {1.0f, 0.0f, 0.0f};
Color GREEN =   {0.0f, 1.0f, 0.0f};
Color BLUE =    {0.0f, 0.0f, 1.0f};

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

Quad makeSquareFromBottomLeft(PointF *corner, float size) {
    Quad q = {
        { corner->x, corner->y },
        { corner->x, corner->y + size },
        { corner->x + size, corner->y + size },
        { corner->x + size, corner->y }
    };
    return q;
}

Quad makeSquareFromCenter(PointF *center, float size) {
    float half = size / (float) 2;
    Quad q = {
        { center->x - half, center->y + half },
        { center->x + half, center->y + half },
        { center->x + half, center->y - half },
        { center->x - half, center->y - half }
    };
    return q;
}

void drawQuad(Quad *q) {
    glVertex2f(q->bottomLeft.x, q->bottomLeft.y);
    glVertex2f(q->topLeft.x, q->topLeft.y);
    glVertex2f(q->topRight.x, q->topRight.y);
    glVertex2f(q->bottomRight.x, q->bottomRight.y);
}

typedef struct Square {
    Quad quad;
    Color *color;
} Square;

void drawSquare(Square *s) {
    glBegin(GL_QUADS);
    setColor(s->color);
    drawQuad(&(s->quad));
}

// Window

SizeF WINDOW_SIZE = {500.0f, 500.0f};

// World

float SQUARE_SIZE = 10.0f;

Square WORLD[SQUARE_COUNT];

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, WINDOW_SIZE.width, 0, WINDOW_SIZE.height, -1, 1); // Set orthographic projection with viewport size of 500
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    

    glBegin(GL_QUADS);
    for (int i = 0; i < SQUARE_COUNT; i++) {
        drawSquare(&(WORLD[i]));
    }
    
    glEnd();
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    PointF bl = {100.0f, 100.0f};
    Square sq = {makeSquareFromBottomLeft(&bl, SQUARE_SIZE), &RED};
    WORLD[0] = sq;
    bl = (PointF){200.0f, 200.0f};
    sq = (Square){makeSquareFromBottomLeft(&bl, SQUARE_SIZE), &GREEN};
    WORLD[1] = sq;
    bl = (PointF){300.0f, 300.0f};
    sq = (Square){makeSquareFromBottomLeft(&bl, SQUARE_SIZE), &BLUE};
    WORLD[2] = sq;

    glutInitWindowSize(WINDOW_SIZE.width, WINDOW_SIZE.height);
    glutCreateWindow("Life simulator");
    glutDisplayFunc(display);
    glutMainLoop();
    
    return 0;
}
