#include <stdio.h>
#include <time.h>
#include <GL/freeglut.h>
#include <primitives.h>

#define CREATURE_SIZE (SizeI) { .width = 5, .height = 5 }
typedef uint32_t Genome;
typedef uint32_t Energy;

#define MAX_CREATURES 5000
#define ENERGY_BASE 1000
#define ENERGY_MAX 10000
#define CELL_FOOD_MAX 100
#define CELL_FOOD_PROBABILITY 0.2f
#define ENERGY_COST_NONE 1
#define ENERGY_COST_EAT 2
#define ENERGY_COST_MOVE 10
#define ENERGY_COST_REPRODUCE 5
#define REPRODUCTION_ENERGY_TRHESHOLD 1000
#define INITIAL_CREATURES_COUNT 1000
#define FOOD_TO_EAT ENERGY_COST_EAT * 3 // It pays 3x the effort to eat
#define SPEED_DELTA 10
//#define DEBUG_ENABLED
//#define TRACE_ENABLED

typedef uint32_t Tick;

typedef enum {
    NOT_AVAILABLE,
    FREE,
    OCCUPIED
} CellState;

typedef enum {
    NONE,
    MOVE,
    EAT,
    REPRODUCE
} Action;

typedef struct {
    PointI location;
    Genome genome;
    Energy energy;
    bool alive;
    Tick birthTick; // Tick when the creature was born
    Tick deathTick; // Tick when the creature died
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
    unsigned int reproductionc; // Number of reproductions
} World;

Cell** buffer; // Buffer for cells

typedef struct {
    Tick tick;
    bool running;
    int initalCreaturesCount;
    int maxCreaturesCount;
    bool displayInformation;
    int updateInterval;
    bool timerScheduled;
    Creature* selection;
} Game;

World* world;
Game* game;

Tick creatureAge(Creature* creature) {
    return creature->alive ? game->tick - creature->birthTick : creature->deathTick - creature->birthTick;
}

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
    world->creatures = (Creature*)malloc(game->maxCreaturesCount * sizeof(Creature));
    for (unsigned int i = 0; i < game->initalCreaturesCount; ++i) {
        PointI location = randomUnoccupiedCell(world);
        world->creatures[i].location.x = location.x;
        world->creatures[i].location.y = location.y;
        world->creatures[i].genome = (uint32_t)rand() | ((uint32_t)rand() << 16);
        world->creatures[i].energy = (ENERGY_BASE + rand()) % ENERGY_MAX;
        world->creatures[i].birthTick = game->tick;
        world->creatures[i].alive = true;
        world->cells[location.x][location.y].creature = &world->creatures[i]; // Assign the creature to the cell
    }
    world->creaturesc = game->initalCreaturesCount;
    world->alivec = world->creaturesc;
}

void initWorld(Game* game, World* world) {
    initCells(game, world);
    initCreatures(game, world);
}

void setColorForCreature(Creature* creature) {
    float r = ((creature->genome & 0xFF0000) >> 16) / 255.0f;
    float g = ((creature->genome & 0x00FF00) >> 8) / 255.0f;
    float b = (creature->genome & 0x0000FF) / 255.0f;
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

void displayText(const char* text, float x, float y) {
    glColor3f(1.0f, 1.0f, 1.0f); // Set text color to white
    glRasterPos2f(x, y); // Set position for the text
    for (const char* c = text; *c != '\0'; ++c) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c); // Render each character
    }
}

void displayPausedText() {
    glColor3f(1.0f, 1.0f, 1.0f); // Set text color to white

    // Calculate the center position for the text
    float centerX = world->size.width / 2.0f - 2.5f; // Adjust for text width
    float centerY = world->size.height / 2.0f;

    glRasterPos2f(centerX, centerY); // Set position near the center

    const char* pausedText = "PAUSED";
    for (const char* c = pausedText; *c != '\0'; ++c) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); // Render each character
    }
}

void displayPopulation() {
    char populationText[50];
    snprintf(populationText, sizeof(populationText), "Population: %d", world->alivec);
    displayText(populationText, 40.0f, 1.0f);
}

void displayTick() {
    char tickText[50];
    snprintf(tickText, sizeof(tickText), "Tick: %d", game->tick);
    displayText(tickText, 10.0f, 1.0f);
}

void displayUpdateInterval() {
    char fpsText[50];
    snprintf(fpsText, sizeof(fpsText), "Update interval: %dms", game->updateInterval);
    displayText(fpsText, 60.0f, 1.0f);
}

void displayReproductions() {
    char text[50];
    snprintf(text, sizeof(text), "Reproductions: %d", world->reproductionc);
    displayText(text, 90.0f, 1.0f);
}

void displaySelection() {
    if (game->selection != NULL) {
        char text[50];
        snprintf(text, sizeof(text), "Creature: age=%d, energy=%d", creatureAge(game->selection), game->selection->energy);
        displayText(text, 120.0f, 1.0f);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < world->creaturesc; ++i) {
        Creature* creature = &world->creatures[i];
        if (creature->alive) {
            renderCreature(creature);
        }
    }

    if (game->displayInformation) {
        displayTick();
        displayPopulation();
        displayUpdateInterval();
        displayReproductions();
    }
    if (game->selection != NULL) {
        displaySelection();
    }
    if (!game->running) displayPausedText();

    glutSwapBuffers();
}

float probabilityMove(Creature* creature) {
    // divides by 63.0f because the bitmask 0x3F extracts the lower 6 bits of the adn field, which can represent values in the range [0, 63]. Dividing by 63.0f normalizes this value to the range [0.0, 1.0].
    return ((creature->genome & 0x3F) / 63.0f);
}

float probabilityReproduce(Creature* creature) {
    // divides by 127.0f because the bitmask 0xFE bits 1 to 7.
    return ((creature->genome & 0xFE) >> 1) / 127.0f;
}

Energy hungerThreshold(Creature* creature) {
    // We extract hunger threshold from the adn field. The bitmask 0xFF extracts the lower 8 bits of the adn field, which can represent values in the range [0, 255]. This value is then used to determine the hunger threshold for the creature.
    return (creature->genome >> 6) & 0xFF;
}

Tick reproductionAge(Creature* creature) {
    // We extract reproduction age from the adn field. The bitmask 0xFF extracts the lower 8 bits of the adn field, which can represent values in the range [0, 255]. This value is then used to determine the reproduction age for the creature.
    return ((creature->genome >> 14) & 0xFF) % 101;
}

Action decideAction(World* world, Creature* creature) {
    bool isHungry = creature->energy < hungerThreshold(creature);
    if (isHungry) {
        bool theresFoodInLocation = world->cells[creature->location.x][creature->location.y].food > 0;
        if (theresFoodInLocation) return EAT;
    }
    Tick age = creatureAge(creature);
    if (age > reproductionAge(creature) && creature->energy >= REPRODUCTION_ENERGY_TRHESHOLD) {
        float reproduce = probabilityReproduce(creature);
        float r = (float)rand() / RAND_MAX;
        if (r < reproduce) {
            return REPRODUCE;
        }
    }
    float move = probabilityMove(creature);
    float r = (float)rand() / RAND_MAX;
    return (r < move) ? MOVE : NONE;
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

CellState cellState(World* world, int x, int y) {
    if (x < 0 || x >= world->size.width || y < 0 || y >= world->size.height) {
        return NOT_AVAILABLE; // Out of bounds
    }
    Cell* cell = &world->cells[x][y];
    if (cell->creature != NULL) {
        return OCCUPIED; // Cell is occupied by a creature
    }
    return FREE; // Cell is free
}

typedef struct {
    PointI location;
    CellState state;
} SurroundingLocation;

SurroundingLocation surroundingLocation(World* world, int x, int y) {
    SurroundingLocation location;
    location.location.x = x;
    location.location.y = y;
    location.state = cellState(world, x, y);
    return location;
}

void cloneInto(Creature* parent, Creature* clone, PointI location) {
    clone->location = location;
    clone->genome = parent->genome; // Clone the DNA
    clone->energy = parent->energy / 2; // Half the energy for the clone
    clone->alive = true;
    clone->birthTick = game->tick;
}

void reproduce(World* world, Creature* creature) {
    if (world->creaturesc+2 >= game->maxCreaturesCount) {
        // If we don't decrease energy, creatures that reached maturity in an overpopulated world will be immortal.
        decreaseEnergy(creature, ENERGY_COST_REPRODUCE);
        return;
    } 

    PointI l = creature->location;

    SurroundingLocation surroundingLocations[9] = {
        surroundingLocation(world, l.x - 1, l.y + 1),
        surroundingLocation(world, l.x    , l.y + 1),
        surroundingLocation(world, l.x + 1, l.y + 1),
        surroundingLocation(world, l.x - 1, l.y    ),
        surroundingLocation(world, l.x    , l.y    ),
        surroundingLocation(world, l.x + 1, l.y    ),
        surroundingLocation(world, l.x - 1, l.y - 1),
        surroundingLocation(world, l.x    , l.y - 1),
        surroundingLocation(world, l.x + 1, l.y - 1)
    };

    int freeCount = 0;
    PointI selected[0];
    for (int i = 0; i < 9; ++i) {
        if (surroundingLocations[i].state == FREE) {
            selected[freeCount++] = surroundingLocations[i].location;
        }
    }
    if (freeCount >= 2) {
        int r1 = rand() % freeCount;
        PointI l1 = selected[r1];
        int r2 = rand() % freeCount;
        while (r2 == r1) {
            r2 = rand() % freeCount;
        }
        PointI l2 = selected[r2];


        cloneInto(creature, &(world->creatures[world->creaturesc++]), l1);
        world->alivec++;
        cloneInto(creature, &(world->creatures[world->creaturesc++]), l2);
        world->alivec++;

        // This should be improved. The creature does not actually die.
        creature->alive = false;
        world->alivec--;
        world->reproductionc += 2;
    }
}

void updateCreature(Creature* creature) {
    if (creature->alive) {
        Action action = decideAction(world, creature);
        switch (action) {
            case NONE:
                none(world, creature);
                break;
            case MOVE:
                move(world, creature);
                break;
            case EAT:
                eat(world, creature);
                break;
            case REPRODUCE:
                reproduce(world, creature);
                break;
            default:
                break;
        }
        
        #ifdef DEBUG_ENABLED
            printf("Creature %d action=%d energy=%d\n", creature->genome, action , creature->energy);
        #endif
        
        if (creature->energy <= 0) {
            creature->alive = false; // Mark as dead if energy is depleted
            creature->deathTick = game->tick; // Store the tick when the creature died
            world->alivec--;
            #ifdef TRACE_ENABLED
                printf("Creature %d died at tick %d\n", creature->genome, game->tick);
            #endif
        }
    }
}

void update(int value);

void scheduleUpdate() {
    // This synchronization is needed to avoid multiple calls to update() in the same tick.
    if (game->timerScheduled) {
        return;
    }
    game->timerScheduled = true;
    glutTimerFunc(game->updateInterval, update, 0);
}

// Update game state here
void update(int value) {
    game->timerScheduled = false; // Reset the timer scheduled flag

    if (!game->running) {
        glutPostRedisplay(); // Request display update
        return;
    }

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
    scheduleUpdate(); // Schedule next update
}

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // Escape key
            glutLeaveMainLoop(); // Exit the main loop
            break;
        case 'p': // Pause the game
        case 'P':
            game->running = !game->running; // Toggle the running state
            if (game->running) {
                scheduleUpdate();
            }
            break;
        case 'i':
        case 'I':
            game->displayInformation = !game->displayInformation;
            break;
        case '+':
            game->updateInterval = game->updateInterval > SPEED_DELTA ? game->updateInterval - SPEED_DELTA : SPEED_DELTA; // Increase speed
            scheduleUpdate();
            break;
        case '-':
            game->updateInterval += SPEED_DELTA;
            scheduleUpdate();
            break;
    }
}

PointI screenToWorld(int x, int y) {
    // Invert the y-coordinate to match OpenGL's coordinate system
    int invertedY = world->size.height * CREATURE_SIZE.height - y;

    PointI point;
    point.x = x / CREATURE_SIZE.width;
    point.y = invertedY / CREATURE_SIZE.height;
    return point;
}

void selectCreature(int x, int y) {
    PointI point = screenToWorld(x, y);
    if (point.x < 0 || point.x >= world->size.width || point.y < 0 || point.y >= world->size.height) {
        return; // Out of bounds
    }
    Cell* cell = &world->cells[point.x][point.y];
    game->selection = cell->creature != NULL ? cell->creature : NULL;
}

void handleMouseClick(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) { // Check if the mouse button was pressed
        if (button == GLUT_LEFT_BUTTON) {
            printf("Mouse click at (%d, %d)\n", x, y);
            selectCreature(x, y);
        }
    }
}

int main(int argc, char** argv) {
    unsigned int seed = argc > 1 ? (unsigned int)atoi(argv[1]) : (unsigned int)time(NULL);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    SizeI screenSize = (SizeI) { .width = 1200, .height = 600 };

    int initialCreaturesCount = argc > 2 ? atoi(argv[2]) : INITIAL_CREATURES_COUNT;
    game = &(Game) { 
        .tick = 0,
        .maxCreaturesCount = MAX_CREATURES,
        .initalCreaturesCount = initialCreaturesCount > MAX_CREATURES ? MAX_CREATURES : initialCreaturesCount,
        .running = false,
        .displayInformation = true,
        .updateInterval = 100,
        .timerScheduled = false,
        .selection = NULL
    };
    world = &(World) { 
        .size = (SizeI) {
            .width = screenSize.width / CREATURE_SIZE.width,
            .height = screenSize.height / CREATURE_SIZE.height
         }, 
        .creaturesc = INITIAL_CREATURES_COUNT
    };
    initWorld(game, world);
    game->running = true;
    
    glutInitWindowSize(screenSize.width, screenSize.height);
    glutCreateWindow("Life Simulator");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, world->size.width, 0, world->size.height, -1, 1); // Set orthographic projection
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress); // Register the keyboard callback
    glutMouseFunc(handleMouseClick); // Register the mouse callback
    scheduleUpdate();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color

    glutMainLoop();
    return 0;
}