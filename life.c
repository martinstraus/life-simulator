#include <stdio.h>
#include <time.h>
#include <GL/freeglut.h>
#include <primitives.h>

#define CREATURE_SIZE (SizeI) { .width = 5, .height = 5 }
typedef uint32_t ADN;
typedef uint32_t Energy;

#define ENERGY_BASE 1000
#define ENERGY_MAX 10000
#define ENERGY_COST_NONE 1
#define ENERGY_COST_EAT 3
#define ENERGY_COST_MOVE 10
#define INITIAL_CREATURES_COUNT 1000

// #define TRACE_ENABLED false

typedef enum {
    NONE,
    MOVE,
    EAT
} Action;

typedef struct {
    PointI location;
    ADN adn;
    Energy energy;
    bool alive;
} Creature;

typedef struct {
    Creature* creature; // Current ocuppying creature. Might be NULL.
} Cell;

typedef struct {
    SizeI size;
    Cell** cells; // 2D array of cells; convention: cells[x][y]
    Creature* creatures;
    unsigned int creaturesc;
    unsigned int alivec; // Number of alive creatures
} World;

Cell** buffer; // Buffer for cells

typedef uint32_t Tick;

typedef struct {
    Tick tick;
    bool running;
} Game;

World* world;
Game* game;

void initCells(World* world) {
    world->cells = (Cell**)malloc(world->size.width * sizeof(Cell*));
    buffer = (Cell**)malloc(world->size.width * sizeof(Cell*));
    for (unsigned int i = 0; i < world->size.width; ++i) {
        world->cells[i] = (Cell*)malloc(world->size.height * sizeof(Cell));
        for (unsigned int j = 0; j < world->size.height; ++j) {
            world->cells[i][j].creature = NULL;
        }
    }
}

PointI randomUnoccupiedCell(World* world) {
    PointI point;
    do {
        point.x = rand() % world->size.width;
        point.y = rand() % world->size.height;
    } while (world->cells[point.x][point.y].creature != NULL);
    return point;
}

void initCreatures(World* world) {
    world->creatures = (Creature*)malloc(world->creaturesc * sizeof(Creature));
    for (unsigned int i = 0; i < world->creaturesc; ++i) {
        PointI location = randomUnoccupiedCell(world);

        world->creatures[i].location.x = location.x;
        world->creatures[i].location.y = location.y;
        world->creatures[i].adn = (uint32_t)rand() | ((uint32_t)rand() << 16);
        world->creatures[i].energy = (ENERGY_BASE + rand()) % ENERGY_MAX;
        world->creatures[i].alive = true;

        world->cells[location.x][location.y].creature = &world->creatures[i];
    }
    world->alivec = world->creaturesc;
}

void initWorld(World* world) {
    initCells(world);
    initCreatures(world);
}

void setColorForCreature(Creature* creature) {
    float r = ((creature->adn & 0xFF0000) >> 16) / 255.0f;
    float g = ((creature->adn & 0x00FF00) >> 8) / 255.0f;
    float b = (creature->adn & 0x0000FF) / 255.0f;
    glColor3f(r, g, b);
}

void renderCreature(Creature* creature) {
    setColorForCreature(creature);
    glBegin(GL_QUADS);
    glVertex2f(creature->location.x, creature->location.y);
    glVertex2f(creature->location.x + 1, creature->location.y);
    glVertex2f(creature->location.x + 1, creature->location.y + 1);
    glVertex2f(creature->location.x, creature->location.y + 1);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < world-> creaturesc; ++i) {
        Creature* creature = &world->creatures[i];
        if (creature->alive) {
            renderCreature(creature);
        }
    }
    // Draw the world bounds
    glutSwapBuffers();
}

Action decideAction(World* world, Creature* creature) {
    // divides by 63.0f because the bitmask 0x3F extracts the lower 6 bits of the adn field, which can represent values in the range [0, 63]. Dividing by 63.0f normalizes this value to the range [0.0, 1.0].
    float probabilityMove = ((creature->adn & 0x3F) / 63.0f);
    float probabilityEat = (((creature->adn >> 6) & 0x3F) / 63.0f);
    float probabilityNone = 1.0f - (probabilityNone + probabilityMove);

    float r = (float)rand() / RAND_MAX;

    if (r < probabilityMove) {
        return MOVE;
    } else if (r < probabilityMove + probabilityEat) {
        return EAT;
    } else {
        return NONE;
    }
}

void none(World* world, Creature* creature) {
    creature->energy -= ENERGY_COST_NONE; // Decrease energy for doing nothing
}

void move(World* world, Creature* creature) {
    int dx = (rand() % 3) - 1; // Randomly choose -1, 0, or 1
    int dy = (rand() % 3) - 1; // Randomly choose -1, 0, or 1
    if (dx != 0 || dy != 0) {
        int newX = creature->location.x + dx;
        int newY = creature->location.y + dy;

        // Check bounds
        if (newX >= 0 && newX < world->size.width && newY >= 0 && newY < world->size.height) {
            Cell* oldCell = &world->cells[creature->location.x][creature->location.y];
            Cell* newCell = &world->cells[newX][newY];

            if (newCell->creature == NULL) { // Move only if the cell is unoccupied
                newCell->creature = creature; // Move to the new cell
                oldCell->creature = NULL; // Leave the old cell
                creature->location.x = newX;
                creature->location.y = newY;
            }
            creature->energy -= 2; // Decrease energy for moving
        }
    }
}

void eat(World* world, Creature* creature) {
    creature->energy -= ENERGY_COST_EAT; // Decrease energy for eating
}

void updateCreature(Creature* creature) {
    if (creature->alive) {
        switch (decideAction(world, creature)) {
            case NONE:
                none(world, creature);
                break;
            case MOVE:
                move(world, creature);
                break;
            case EAT:
                eat(world, creature);
                break;
            default:
                break;
        }
        
        
        if (creature->energy == 0) {
            creature->alive = false; // Mark as dead if energy is depleted
            world->alivec--;
        }
    }
}

// Update game state here
void update(int value) {
    game->tick++;

    #ifdef TRACE_ENABLED
        printf("tick %d\n", game->tick);
    #endif

    for (int i = 0; i < world->creaturesc; ++i) {
        updateCreature(&world->creatures[i]);
    }

    if (world->alivec == 0) {
        game->running = false; // Stop the game if all creatures are dead
        glutLeaveMainLoop(); // Exit the main loop if game is not running
    }

    glutPostRedisplay(); // Request display update
    glutTimerFunc(16, update, 0); // Schedule next update (~60 FPS)    
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL)); // Seed the random number generator

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    SizeI screenSize = (SizeI) { .width = 1200, .height = 600 };

    game = &(Game) { .tick = 0 };
    world = &(World) { 
        .size = (SizeI) {
            .width = screenSize.width / CREATURE_SIZE.width,
            .height = screenSize.height / CREATURE_SIZE.height
         }, 
        .creaturesc = INITIAL_CREATURES_COUNT
    };
    initWorld(world);

    glutInitWindowSize(screenSize.width, screenSize.height);
    glutCreateWindow("Life Simulator");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, world->size.width, 0, world->size.height, -1, 1); // Set orthographic projection
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color

    glutMainLoop();
    return 0;
}