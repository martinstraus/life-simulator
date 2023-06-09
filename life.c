#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <GL/freeglut.h>

#define false 0
#define true 1

typedef unsigned int bool;

#define PALLETE_SIZE 5

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 620
#define FOOTER_HEIGHT 20

#define SQUARE_SIZE 3
#define WORLD_WIDTH SCREEN_WIDTH/SQUARE_SIZE
#define WORLD_HEIGHT (SCREEN_HEIGHT-FOOTER_HEIGHT)/SQUARE_SIZE
#define WORLD_SPEED_MIN 1
#define WORLD_SPEED_MAX 10000
#define WORLD_SPEED_FACTOR 10
#define WORLD_SPEED WORLD_SPEED_MIN

#define CREATURE_INITIAL_ENERY 1000
#define CREATURES_RATIO 0.10 // Percentage of world cells with creatures

#define KEY_SPACEBAR ' '
#define KEY_ESCAPE 27
#define KEY_PLUS 43
#define KEY_MINUS 45

// General purpose functions

int randomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

uint64_t random_uint64() {
    uint64_t random_value = ((uint64_t)rand() << 32) | rand();
    return random_value;
}

// Should return 0 when x = 0, and approach 1 for greater values.
double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x)) - 0.5;
}

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

int randomColor() {
    return randomInt(0, PALLETE_SIZE-1);
}

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
    float y, x;
} PointF;

typedef struct Quad {
    PointF bottomLeft, topLeft, topRight, bottomRight;
} Quad;

typedef struct Viewport {
    PointI lowerLeft;
    SizeI size;
} Viewport;

bool isInsideViewport(int x, int y, Viewport *v) {
    return x >= v->lowerLeft.column && x < v->lowerLeft.column + v->size.width
        && y >= v->lowerLeft.row && y < v->lowerLeft.row + v->size.height;
}

Quad makeSquareFromBottomLeft(PointF *corner, float size) {
    Quad q = {
        { corner->y, corner->x,  },
        { corner->y + size, corner->x },
        { corner->y + size, corner->x + size },
        { corner->y, corner->x + size }
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

Quad quadForLocation(int row, int column) {
    PointF bl = {row * SQUARE_SIZE, column * SQUARE_SIZE};
    return makeSquareFromBottomLeft(&bl, SQUARE_SIZE);
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
    long energy;        // Some actions have an energy cost.
} Creature;

typedef struct Population {
    int size;
    Creature *creatures;
} Population;

typedef struct WorldTime {
    long speed;    // Time between ticks, in milliseconds.
    long current;  // Current tick; no relation to actual time.
} WorldTime;

typedef struct World {
    SizeI size;
    Medium **floor; // First dimension = rows; second dimension = columns.
    Population population;
    WorldTime time;
} World;

typedef struct Game {
    bool paused;
    bool exit;
} Game;

World WORLD;
Game GAME;
int WINDOW_ID;
Creature*** BUFFER; // Buffer for creatures locations, used while updating world.
Viewport WORLD_VIEWPORT;
Viewport FOOTER_VIEWPORT;

void setViewport(Viewport* v) {
    glViewport(v->lowerLeft.column, v->lowerLeft.row, v->size.width, v->size.height);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, WINDOW_SIZE.width, 0, WINDOW_SIZE.height, -1, 1); // Set orthographic projection with viewport
    setViewport(&WORLD_VIEWPORT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);

    for (int r = 0; r < WORLD_HEIGHT; r++) {
        for (int c = 0; c < WORLD_WIDTH; c++) {
            drawSquare(&(WORLD.floor[r][c].shape));
        }
    }

    for (int i = 0; i < WORLD.population.size; i++) {
        drawSquare(&(WORLD.population.creatures[i].shape));
    }

    glEnd();

    setViewport(&FOOTER_VIEWPORT);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(5, 0);
    char s[100];
    sprintf(s, "Tick: %ld %s", WORLD.time.current, GAME.paused ? "(Paused)": "");
    glutBitmapString(GLUT_BITMAP_9_BY_15, s);
    glFlush();
    glutSwapBuffers();
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

int creatureShouldDie(Creature *c) {
    long age = WORLD.time.current - c->birth;
    double s = sigmoid(age);
    return s > 0.95 ? 1 : 0;
}

void initWorld() {
    // Seed the random number generator
    srand(time(NULL));

    //int creaturesSize = 10;
    int creaturesSize = WORLD_WIDTH * WORLD_HEIGHT * CREATURES_RATIO;

    WORLD = (World){
        (SizeI){WORLD_HEIGHT, WORLD_WIDTH}, 
        (Medium **)malloc( WORLD_HEIGHT * sizeof(Medium *)),
        (Population){
            creaturesSize,
            (Creature *)malloc( creaturesSize * sizeof(Creature))
        },
        (WorldTime){WORLD_SPEED,0}
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
                    quadForLocation(r, c),
                    mediumType->color
                }
            };
        }
    }
    
    // Initialization of creatures
    for (int i = 0; i < WORLD.population.size; i++) {
        PointI location = randomLocation();
        ADN adn = randomADN();
        int colorIndex =(int) (adn.value & ADN_COLOR);
        WORLD.population.creatures[i] = (Creature){
            location,
            adn,
            0l,
            0,
            (Square) {
                quadForLocation(location.row, location.column),
                &PALLETE[colorIndex+1]
            },
            CREATURE_INITIAL_ENERY,
        };
    }

    // Initialize creatures buffer
    BUFFER = malloc(sizeof(Creature **) * WORLD.size.height);
    for (int r = 0; r < WORLD.size.height; r++) {
        BUFFER[r] = malloc(sizeof(Creature *) * WORLD.size.width);
        for (int c = 0; c < WORLD.size.width; c++) {
            BUFFER[r][c] = NULL;
        }
    }
}

void cleanCreaturesBuffer() {
    for (int r = 0; r < WORLD.size.height; r++) {
        for (int c = 0; c < WORLD.size.width; c++) {
            BUFFER[r][c] = NULL;
        }
    }
}

PointI positionAfterRandomMovement(PointI current) {
    PointI rp = {randomInt(0,2)-1, randomInt(0,2)-1};               // Random movement.
    PointI np = {rp.row + current.row, rp.column + current.column}; // New location
    if (np.column < 0) {
        np.column = 0;
    }
    else if (np.column >= WORLD.size.width) {
        np.column = WORLD.size.width-1;
    }
    if (np.row < 0) {
        np.row = 0;
    }
    else if (np.row >= WORLD.size.height) {
        np.row = WORLD.size.height-1;
    }
    return np;
}

bool isOutsideWorldBoundaries(PointI location) {
    return location.row < 0 || location.row > WORLD.size.height-1 
        || location.column < 0 || location.column > WORLD.size.width-1;
}

void updateWorld() {
    cleanCreaturesBuffer();

    // Update the creatures to new locations
    for (int i = 0; i < WORLD.population.size; i++) {
        Creature *c = &(WORLD.population.creatures[i]);
        if (randomInt(0,1) == 1 && c->energy > 0) {
            PointI np = positionAfterRandomMovement(c->location);
            while (isOutsideWorldBoundaries(np) || BUFFER[np.row][np.column] != NULL) {
                np = positionAfterRandomMovement(c->location);
            }
            BUFFER[np.row][np.column] = c;
            c->location.row = np.row;
            c->location.column = np.column;
            c->shape.quad = quadForLocation(np.row, np.column);
            c->energy--;
        }
    }

}

void tick() {
    if (!GAME.paused) {
        WORLD.time.current++;
        updateWorld();
        glutPostRedisplay();
    }
    glutTimerFunc(WORLD.time.speed, tick, 0);
}

void keyboardCallack(unsigned char key, int x, int y) {
    switch (key) {
        case KEY_SPACEBAR: 
            GAME.paused = GAME.paused ? false : true;
            glutPostRedisplay();
            break;
        case KEY_ESCAPE: 
            GAME.exit = true;
            break;
        case KEY_PLUS:
            WORLD.time.speed = WORLD.time.speed > WORLD_SPEED_MIN ? WORLD.time.speed / WORLD_SPEED_FACTOR : WORLD_SPEED_MIN;
            //printf("New speed: %ld\n", WORLD.time.speed);
            break;
        case KEY_MINUS:
            WORLD.time.speed = WORLD.time.speed < WORLD_SPEED_MAX ? WORLD.time.speed * WORLD_SPEED_FACTOR : WORLD_SPEED_MAX;
            //printf("New speed: %ld\n", WORLD.time.speed);
            break;
    }
    //printf("Key pressed: %d\n", key);
}

void idleCallback() {
    if (GAME.exit) {
        glutDestroyWindow(WINDOW_ID);
        exit(0);
    }
}

void mouseMoveCallback(int x, int y) {
    int yy = WINDOW_SIZE.height-y;
    if (isInsideViewport(x,yy,&WORLD_VIEWPORT)) {
        PointI p = {(yy-FOOTER_HEIGHT) / SQUARE_SIZE, x / SQUARE_SIZE};
        //printf("Mouse moved to (row: %d, col: %d)\n", p.row, p.column);
    }
}

void initGraphics(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_SIZE.width, WINDOW_SIZE.height);
    WINDOW_ID = glutCreateWindow("Life simulator");
    glutTimerFunc(WORLD.time.speed, tick, 0);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardCallack);
    glutIdleFunc(idleCallback);
    glutPassiveMotionFunc(mouseMoveCallback);
    WORLD_VIEWPORT = (Viewport){(PointI){FOOTER_HEIGHT, 0}, (SizeI){SCREEN_HEIGHT, SCREEN_WIDTH}};
    FOOTER_VIEWPORT = (Viewport){(PointI){0, 0}, (SizeI){FOOTER_HEIGHT, SCREEN_WIDTH}};
}

void run() {
    glutMainLoop();
}

int main(int argc, char** argv) {
    GAME = (Game) {
        false,
        false
    };
    initWorld();
    initGraphics(argc, argv);
    run();
    return 0;
}
