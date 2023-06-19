#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <GL/freeglut.h>

#define PALLETE_SIZE 5

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

const Color BLACK =   {0, 0, 0};
const Color RED =     {255, 153, 153};
const Color GREEN =   {153, 255, 153};
const Color BLUE =    {153, 204, 255};
const Color BROWN =   {176, 152, 126};

Color PALLETE[PALLETE_SIZE] = {BLACK, RED, GREEN, BLUE, BROWN};

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

// The medium where the cells might be living. It might be the floor, air, water...
typedef struct MediumType {
    Color *color;
} MediumType;

const MediumType NOTHING = {&BLACK};
const MediumType DIRT = {&BROWN};
const MediumType GRASS = {&GREEN};
const MediumType WATER = {&BLUE};
const MediumType LAVA = {&RED};

const MediumType MEDIA[] = {NOTHING, DIRT, GRASS, WATER, LAVA};

typedef struct Medium {
    MediumType *type;
    Square shape;
} Medium;

// ADN defines a set of characteristics that influence the cell's behaviuor.
typedef struct ADN {
    uint64_t value;
} ADN;

const uint64_t ADN_COLOR = 0x3;

typedef struct Creature {
    PointI location;
    ADN adn;
    long generation;    // Generation is increased every time a creature is born out of reproduction
    long birth;         // When this creature was born, in ticks.
    Square shape;
} Creature;

typedef struct WorldTime {
    long speed;    // Time between ticks, in milliseconds.
    long current;  // Current tick; no relation to actual time.
} WorldTime;

typedef struct World {
    SizeI size;
    Medium **floor; // First dimension = rows; second dimension = columns.
    int creaturesSize;
    Creature *creatures;
    WorldTime time;
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
        for (int c = 0; c < WORLD_WIDTH; c++) {
        drawSquare(&(WORLD.floor[r][c].shape));
    }
    }

    for (int i = 0; i < WORLD.creaturesSize; i++) {
        drawSquare(&(WORLD.creatures[i].shape));
    }
    
    glEnd();
    glFlush();
}

int randomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

uint64_t random_uint64() {
    uint64_t random_value = ((uint64_t)rand() << 32) | rand();
    return random_value;
}

int randomColor() {
    return randomInt(0, PALLETE_SIZE-1);
}

MediumType * randomMediumType() {
    int rand = randomInt(0, 100);
    if (rand < 80) return &GRASS;
    if (rand < 85) return &DIRT;
    if (rand < 99) return &WATER;
    return &LAVA;
}

PointI randomLocation() {
    return (PointI) {
        randomInt(0, WORLD_HEIGHT),
        randomInt(0, WORLD_WIDTH)
    };
}

ADN randomADN() {
    return (ADN){random_uint64()};
};

void initWorld() {
    // Seed the random number generator
    srand(time(NULL));

    int creaturesSize = WORLD_WIDTH * WORLD_HEIGHT * 0.05;

    WORLD = (World){
        (SizeI){WORLD_WIDTH, WORLD_HEIGHT}, 
        (Medium **)malloc( WORLD_HEIGHT * sizeof(Medium *)),
        creaturesSize,
        (Creature *)malloc( creaturesSize * sizeof(Creature)),
        (WorldTime){500,0}
    };

    // Initialization of medium matrix.
    for (int r = 0; r < WORLD_HEIGHT; r++) {
        WORLD.floor[r] = (Medium *) malloc(WORLD_WIDTH * sizeof(Medium));

        // Initialization of the row of squares
        for(int c = 0; c < WORLD_WIDTH; c++) {
            PointF bl = {c * SQUARE_SIZE, r * SQUARE_SIZE};
            MediumType *mediumType = &NOTHING;
            WORLD.floor[r][c] = (Medium){
                mediumType,
                (Square) {
                    makeSquareFromBottomLeft(&bl, SQUARE_SIZE),
                    mediumType->color
                }
            };
        }
    }
    
    // Initialization of creatures
    for (int i = 0; i < WORLD.creaturesSize; i++) {
        PointI location = randomLocation();
        PointF bl = {
            (float) (location.row * SQUARE_SIZE), 
            (float) (location.column * SQUARE_SIZE)
        };
        ADN adn = randomADN();
        int colorIndex =(int) (adn.value & ADN_COLOR);
        WORLD.creatures[i] = (Creature){
            location,
            adn,
            0l,
            0,
            (Square) {
                makeSquareFromBottomLeft(&bl, SQUARE_SIZE),
                &PALLETE[colorIndex+1]
            }
        };
    }
}

void tick() {
    WORLD.time.current++;
    printf("Current tick: %ld\n", WORLD.time.current);
    glutTimerFunc(WORLD.time.speed, tick, 0);
}

void initGraphics(int argc, char** argv) {
    glutInit(&argc, argv);
    //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_SIZE.width, WINDOW_SIZE.height);
    glutCreateWindow("Life simulator");
    glutTimerFunc(WORLD.time.speed, tick, 0);
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
