#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/freeglut.h>

#define PALLETE_SIZE 3

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 500

#define SQUARE_SIZE 10
#define WORLD_WIDTH SCREEN_WIDTH/SQUARE_SIZE
#define WORLD_HEIGHT SCREEN_WIDTH/SQUARE_SIZE

// Colors

typedef struct Color {
    int r,g,b;
} Color;

void setColor(Color *c) {
    glColor3ub(c->r, c->g, c->b);
}

const Color RED =     {255, 153, 153};
const Color GREEN =   {153, 255, 153};
const Color BLUE =    {153, 204, 255};

Color PALLETE[PALLETE_SIZE] = {RED, GREEN, BLUE};

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

SizeF WINDOW_SIZE = {SCREEN_HEIGHT, SCREEN_WIDTH};

// World

typedef struct World {
    SizeI size;
    Square **squares; // First dimension = rows; second dimension = columns.
} World;

World WORLD;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, WINDOW_SIZE.width, 0, WINDOW_SIZE.height, -1, 1); // Set orthographic projection with viewport size of 500
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    for (int r = 0; r < WORLD_HEIGHT; r++) {
        for (int c = 0; c < WORLD_WIDTH; c++)
        drawSquare(&(WORLD.squares[r][c]));
    }
    
    glEnd();
    glFlush();
}

int randomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

int randomColor() {
    return randomInt(0, PALLETE_SIZE-1);
}

void initWorld() {
    // Seed the random number generator
    srand(time(NULL));

    WORLD = (World){
        (SizeI){WORLD_WIDTH, WORLD_HEIGHT}, 
        (Square **)malloc( WORLD_HEIGHT * sizeof(Square *))
    };

    // Initialization of squares matrix.
    for (int r = 0; r < WORLD_HEIGHT; r++) {
        WORLD.squares[r] = (Square *) malloc(WORLD_WIDTH * sizeof(Square));

        // Initialization of the row of squares
        for(int c = 0; c < WORLD_WIDTH; c++) {
            PointF bl = {c * SQUARE_SIZE, r * SQUARE_SIZE};
            WORLD.squares[r][c] = (Square){
                makeSquareFromBottomLeft(&bl, SQUARE_SIZE), 
                &(PALLETE[randomColor()])
            };
        }
    }
    
}

void initGraphics(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(WINDOW_SIZE.width, WINDOW_SIZE.height);
    glutCreateWindow("Life simulator");
    glutDisplayFunc(display);
}

void run() {
    glutMainLoop();
}

int main(int argc, char** argv) {
    initWorld();
    initGraphics(argc, argv);
    run();
    return 0;
}
