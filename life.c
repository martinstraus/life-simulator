#include <stdio.h>
#include <time.h>
#include <GL/freeglut.h>
#include <primitives.h>

#define CREATURE_SIZE (SizeI) { .width = 5, .height = 5 }
typedef uint32_t DNA;
typedef uint32_t Energy;

#define ENERGY_BASE 1000
#define ENERGY_MAX 10000
#define CELL_FOOD_MAX 100
#define CELL_FOOD_PROBABILITY 0.2f
#define ENERGY_COST_NONE 1
#define ENERGY_COST_EAT 2
#define ENERGY_COST_MOVE 10
#define INITIAL_CREATURES_COUNT 1000
#define FOOD_TO_EAT ENERGY_COST_EAT * 3 // It pays 3x the effort to eat

// #define TRACE_ENABLED false

typedef enum {
    NONE,
    MOVE,
    EAT
} Action;

typedef struct {
    PointI location;
    DNA dna;
    Energy energy;
    bool alive;
} Creature;

typedef struct {
    Creature* creature; // Current ocuppying creature. Might be NULL.
    Energy food; // Food in the cell.
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
    int initalCreaturesCount;
} Game;

World* world;
Game* game;

void initCells(Game* game, World* world) {
    world->cells = (Cell**)malloc(world->size.width * sizeof(Cell*));
    buffer = (Cell**)malloc(world->size.width * sizeof(Cell*));
    for (unsigned int i = 0; i < world->size.width; ++i) {
        world->cells[i] = (Cell*)malloc(world->size.height * sizeof(Cell));
        for (unsigned int j = 0; j < world->size.height; ++j) {
            world->cells[i][j].creature = NULL;

            // Randomly assign food to cell
            if ((float)rand() / RAND_MAX < CELL_FOOD_PROBABILITY) {
                world->cells[i][j].food = (Energy)(rand() % CELL_FOOD_MAX);
            } else {
                world->cells[i][j].food = 0;
            }
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

void initCreatures(Game* game, World* world) {
    world->creatures = (Creature*)malloc(game->initalCreaturesCount * sizeof(Creature));
    for (unsigned int i = 0; i < game->initalCreaturesCount; ++i) {
        PointI location = randomUnoccupiedCell(world);

        world->creatures[i].location.x = location.x;
        world->creatures[i].location.y = location.y;
        world->creatures[i].dna = (uint32_t)rand() | ((uint32_t)rand() << 16);
        world->creatures[i].energy = (ENERGY_BASE + rand()) % ENERGY_MAX;
        world->creatures[i].alive = true;

        world->cells[location.x][location.y].creature = &world->creatures[i];
    }
    world->creaturesc = game->initalCreaturesCount;
    world->alivec = world->creaturesc;
}

void initWorld(Game* game, World* world) {
    initCells(game, world);
    initCreatures(game, world);
}

void setColorForCreature(Creature* creature) {
    float r = ((creature->dna & 0xFF0000) >> 16) / 255.0f;
    float g = ((creature->dna & 0x00FF00) >> 8) / 255.0f;
    float b = (creature->dna & 0x0000FF) / 255.0f;
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

float probabilityMove(Creature* creature) {
    // divides by 63.0f because the bitmask 0x3F extracts the lower 6 bits of the adn field, which can represent values in the range [0, 63]. Dividing by 63.0f normalizes this value to the range [0.0, 1.0].
    return ((creature->dna & 0x3F) / 63.0f);
}

Energy hungerThreshold(Creature* creature) {
    // We extract hunger threshold from the adn field. The bitmask 0xFF extracts the lower 8 bits of the adn field, which can represent values in the range [0, 255]. This value is then used to determine the hunger threshold for the creature.
    return (creature->dna >> 6) & 0xFF;
}

Action decideAction1(World* world, Creature* creature) {
    // divides by 63.0f because the bitmask 0x3F extracts the lower 6 bits of the adn field, which can represent values in the range [0, 63]. Dividing by 63.0f normalizes this value to the range [0.0, 1.0].
    float move = probabilityMove(creature);
    float probabilityEat = (((creature->dna >> 6) & 0x3F) / 63.0f);
    float probabilityNone = 1.0f - (probabilityNone + move);

    float r = (float)rand() / RAND_MAX;

    if (r < move) {
        return MOVE;
    } else if (r < move + probabilityEat) {
        return EAT;
    } else {
        return NONE;
    }
}

Action decideAction2(World* world, Creature* creature) {
    bool isHungry = creature->energy < hungerThreshold(creature);
    bool theresFoodInLocation = world->cells[creature->location.x][creature->location.y].food > 0;
    if (isHungry && theresFoodInLocation) {
        return EAT;
    } else {
        float move = probabilityMove(creature);
        float r = (float)rand() / RAND_MAX;
        return (r < move) ? MOVE : NONE;
    }
}

void decreaseEnergy(Creature* creature, Energy amount) {
    if (creature->energy >= amount) {
        creature->energy -= amount;
    } else {
        creature->energy = 0;
    }
}

void none(World* world, Creature* creature) {
    decreaseEnergy(creature, ENERGY_COST_NONE); // Decrease energy for doing nothing
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
            decreaseEnergy(creature, ENERGY_COST_MOVE); // Decrease energy for moving
        }
    }
}

void eat(World* world, Creature* creature) {
    Cell* cell = &world->cells[creature->location.x][creature->location.y];
    if (cell->food > 0) {
        int foodToEat = cell->food < FOOD_TO_EAT ? cell->food : FOOD_TO_EAT; // Cap the amount of food to eat
        foodToEat = creature->energy + foodToEat > ENERGY_MAX ? ENERGY_MAX - creature->energy : foodToEat; // Prevent eating more that possible
        creature->energy += foodToEat; // Increase energy
        cell->food -= foodToEat; // Decrease food in the cell
    }
    decreaseEnergy(creature, ENERGY_COST_EAT); // Decrease energy for eating
}

void updateCreature(Creature* creature) {
    if (creature->alive) {
        Action action = decideAction1(world, creature);
        switch (decideAction2(world, creature)) {
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
        
        #ifdef TRACE_ENABLED
            printf("Creature %d action=%d energy=%d\n", creature->dna, action , creature->energy);
        #endif
        
        if (creature->energy <= 0) {
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
    unsigned int seed =  argc > 1 ? (unsigned int)atoi(argv[1]) : (unsigned int)time(NULL);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    SizeI screenSize = (SizeI) { .width = 1200, .height = 600 };

    game = &(Game) { 
        .tick = 0,
        .initalCreaturesCount = argc > 2 ? atoi(argv[2]) : INITIAL_CREATURES_COUNT,
    };
    world = &(World) { 
        .size = (SizeI) {
            .width = screenSize.width / CREATURE_SIZE.width,
            .height = screenSize.height / CREATURE_SIZE.height
         }, 
        .creaturesc = INITIAL_CREATURES_COUNT
    };
    initWorld(game, world);

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