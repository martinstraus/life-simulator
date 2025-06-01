#include <stdio.h>
#include <string.h>
#include <time.h>
#include <GL/freeglut.h>
#include <primitives.h>
#include <sys/time.h>

typedef uint32_t Genome;
typedef uint32_t Energy;

#define ENERGY_BASE 1000
#define ENERGY_MAX 10000
#define CELL_FOOD_MAX 100
#define CELL_FOOD_PROBABILITY 0.2f
#define ENERGY_COST_NONE 1
#define ENERGY_COST_EAT 2
#define ENERGY_COST_MOVE 10
#define ENERGY_COST_REPRODUCE 5
#define REPRODUCTION_ENERGY_THRESHOLD 1000
#define INITIAL_CREATURES_COUNT 1000
#define FOOD_TO_EAT ENERGY_COST_EAT * 3 // It pays 3x the effort to eat
#define SPEED_DELTA 10
#define MUTATION_PROBABILITY 0.01f
#define GENE_POOL_SIZE 3
#define USE_GENE_POOL true

#define UPDATE_INTERVAL 100
#define MAX_FPS 60
#define FRAME_TIME_MS (1000.0 / MAX_FPS)

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
    int generation; // How many reproductions it take to reach this creature
} Creature;

typedef struct {
    Creature* creature; // Current ocuppying creature. Might be NULL.
    Energy food; // Food in the cell.
} Cell;

typedef struct {
    SizeI size;
    Cell* cells; // 2D array of cells, in 1 dimension for performance; convention: cells[y*width+x]
    Creature* creatures;
    unsigned int creaturesc;
    uintmax_t maxPopulation;
    unsigned int alivec; // Number of alive creatures
    unsigned int reproductionc; // Number of reproductions
    unsigned int mutations;
} World;

typedef struct {
    Genome* genomes;
    unsigned int size;
} GenePool;

typedef struct {
    bool useSeed;
    int seed;
    unsigned int initialCreaturesCount;
    bool useGenePool;
    unsigned int genePoolSize;
    int updateInterval;
    bool useWorldSize;
    SizeI worldSize;
    float mutationProbability;
} Parameters;

typedef struct {
    Tick tick;
    bool running;
    bool ended;
    bool displayInformation;
    int updateInterval;
    bool timerScheduled;
    Creature* selection;
    bool useGenePool;
    GenePool genePool;
    float mutationProbability;
    double lastFrameTime;       // Used for limiting frame-rate.
} Game;

typedef struct {
    struct {
        SizeF size;
    } screen;
    struct {
        PointF position; // Always reffers to the center of the screen.
        bool dragging;
    } camera;
    struct {
        PointI bottomLeft;
        SizeI size;
    } visibleWorld; 
    float zoom; // Zoom factor
    PointI mousePosition;
} GameView;

World* world;
Game* game;
GameView* view;
PointI lastMousePosition;

void freeMemory();

void checkMemoryAllocation(void* ptr, const char* message) {
    if (!ptr) {
        fprintf(stderr, "%s", message);
        freeMemory();
        exit(EXIT_FAILURE);
    }
}

Tick creatureAge(Creature* creature) {
    return creature->alive ? game->tick - creature->birthTick : creature->deathTick - creature->birthTick;
}

void initCells(Game* game, World* world) {
    world->cells = malloc(world->size.width * world->size.height * sizeof(Cell));
    checkMemoryAllocation(world->cells, "Failed to allocate memory for cells.\n");
    for (unsigned int x = 0; x < world->size.width; ++x) {
        for (unsigned int y = 0; y < world->size.height; ++y) {
            Cell* cell = &world->cells[y * world->size.width + x];
            cell->creature = NULL;

            // Randomly assign food to cell
            if ((float)rand() / RAND_MAX < CELL_FOOD_PROBABILITY) {
                cell->food = (Energy)(rand() % CELL_FOOD_MAX);
            } else {
                cell->food = 0;
            }
        }
    }
}

bool randomUnoccupiedCell(World* world, PointI* out) {
    for (int attempts = 0; attempts < 100; ++attempts) {
        int x = rand() % world->size.width;
        int y = rand() % world->size.height;
        if (!world->cells[y * world->size.width + x].creature) {
            out->x = x; out->y = y;
            return true;
        }
    }
    return false;
}

Genome randomGenome() {
    return (uint32_t)rand() | ((uint32_t)rand() << 16);
}

Genome selectGenome(Game* game) {
    if (game->useGenePool) {
        int r = rand() % game->genePool.size;
        return game->genePool.genomes[r];
    } else {
        return randomGenome();
    }
}

void initCreatures(Game* game, World* world) {
    world->creatures = (Creature*)malloc(world->maxPopulation * sizeof(Creature));
    checkMemoryAllocation(world->creatures, "Failed to allocate memory for creatures.\n");
    for (unsigned int i = 0; i < world->creaturesc; ++i) {
        PointI location;
        if (randomUnoccupiedCell(world, &location)) {
            world->creatures[i].location.x = location.x;
            world->creatures[i].location.y = location.y;
            world->creatures[i].genome = selectGenome(game);
            world->creatures[i].energy = (ENERGY_BASE + rand()) % ENERGY_MAX;
            world->creatures[i].birthTick = game->tick;
            world->creatures[i].alive = true;
            world->cells[location.y * world->size.width + location.x].creature = &world->creatures[i]; // Assign the creature to the cell
        }
    }
    world->alivec = world->creaturesc;
}

void initGenePool(Game *game) {
    if (game->useGenePool) {
        game->genePool.genomes = (Genome*)malloc(game->genePool.size * sizeof(Genome));
        checkMemoryAllocation(game->genePool.genomes, "Failed to allocate memory for gene pool.\n");
        for (int i = 0; i < game->genePool.size; ++i) {
            game->genePool.genomes[i] = randomGenome();
        }
    }
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

void renderSelectionHighlight(Creature* creature) {
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow highlight
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(creature->location.x, creature->location.y);
    glVertex2f(creature->location.x + 1, creature->location.y);
    glVertex2f(creature->location.x + 1, creature->location.y + 1);
    glVertex2f(creature->location.x, creature->location.y + 1);
    glEnd();
    glLineWidth(1.0f); // Reset to default
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

void displayText(const char* text, void* font, float x, float y) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, view->screen.size.width, 0, view->screen.size.height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f); // Set text color to white
    glRasterPos2f(x, y); // Set position for the text
    for (const char* c = text; *c != '\0'; ++c) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c); // Render each character
    }

    // Restore previous projection and modelview
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void displayBanner(const char* text) {
    // Calculate the center position for the text
    float centerX = view->screen.size.width / 2.0f - 2.5f; // Adjust for text width
    float centerY = view->screen.size.height / 2.0f;

    displayText(text, GLUT_BITMAP_HELVETICA_18, centerX, centerY);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (game->ended) {
        displayBanner("GAME OVER");
        glutSwapBuffers();
        return;
    }

    float left = view->camera.position.x - (view->screen.size.width / view->zoom) / 2.0f;
    float right = view->camera.position.x + (view->screen.size.width / view->zoom) / 2.0f;
    float bottom = view->camera.position.y - (view->screen.size.height / view->zoom) / 2.0f;
    float top = view->camera.position.y + (view->screen.size.height / view->zoom) / 2.0f;

    for (int i = 0; i < world->creaturesc; ++i) {
        Creature* creature = &world->creatures[i];
        if (creature->alive &&
            creature->location.x >= left && creature->location.x < right &&
            creature->location.y >= bottom && creature->location.y < top) {
            renderCreature(creature);
        }
    }

    if (game->selection && game->selection->alive) {
        renderSelectionHighlight(game->selection);
    }

    if (game->displayInformation) {
        char information[256]; // Increased from 100 to 256
        if (game->selection != NULL) {
            snprintf(information, sizeof(information),
                "Tick: %d Interval: %dms Population: %d Mutations: %d Mouse: (x=%d, y=%d) Creature: (age %d energy: %d generation: %d)",
                game->tick, game->updateInterval, world->alivec, world->mutations,
                view->mousePosition.x, view->mousePosition.y,
                creatureAge(game->selection), game->selection->energy, game->selection->generation);
        } else {
            snprintf(information, sizeof(information),
                "Tick: %d Interval: %dms Population: %d Mutations: %d Mouse: (x=%d, y=%d)",
                game->tick, game->updateInterval, world->alivec, world->mutations,
                view->mousePosition.x, view->mousePosition.y);
        }
        displayText(information, GLUT_BITMAP_HELVETICA_12, 1.0f, 1.0f);
    }

    if (!game->running) displayBanner("PAUSED");

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
        bool theresFoodInLocation = world->cells[creature->location.y * world->size.width + creature->location.x].food > 0;
        if (theresFoodInLocation) return EAT;
    }
    Tick age = creatureAge(creature);
    if (age > reproductionAge(creature) && creature->energy >= REPRODUCTION_ENERGY_THRESHOLD) {
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
            Cell* oldCell = &world->cells[creature->location.y * world->size.width + creature->location.x];
            Cell* newCell = &world->cells[newY*world->size.width + newX];

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

Energy safeAddEnergy(Energy current, Energy add) {
    if (current > ENERGY_MAX - add)
        return ENERGY_MAX;
    else
        return current + add;
}

void eat(World* world, Creature* creature) {
    Cell* cell = &world->cells[creature->location.y * world->size.width + creature->location.x];
    if (cell->food > 0) {
        int foodToEat = cell->food < FOOD_TO_EAT ? cell->food : FOOD_TO_EAT; // Cap the amount of food to eat
        foodToEat = creature->energy + foodToEat > ENERGY_MAX ? ENERGY_MAX - creature->energy : foodToEat; // Prevent eating more that possible
        creature->energy = safeAddEnergy(creature->energy, foodToEat);
        cell->food -= foodToEat; // Decrease food in the cell
    }
    decreaseEnergy(creature, ENERGY_COST_EAT); // Decrease energy for eating
}

CellState cellState(World* world, int x, int y) {
    if (x < 0 || x >= world->size.width || y < 0 || y >= world->size.height) {
        return NOT_AVAILABLE; // Out of bounds
    }
    Cell* cell = &world->cells[y * world->size.width + x];
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

void cloneInto(World* world, Creature* parent, Creature* clone, PointI location) {
    clone->location = location;
    if (rand() / (float)RAND_MAX < game->mutationProbability) {
        clone->genome = parent->genome ^ (1 << (rand() % 32)); // Clone the DNA with one bit flipped
        world->mutations++;
    } else {
        clone->genome = parent->genome; // Clone the DNA without mutation
    }
    clone->generation = parent->generation + 1;
    clone->energy = parent->energy / 2; // Half the energy for the clone
    clone->alive = true;
    clone->birthTick = game->tick;
}

void reproduce(World* world, Creature* creature) {
    if (world->creaturesc+2 >= world->maxPopulation) {
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
    PointI selected[9];
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


        cloneInto(world, creature, &(world->creatures[world->creaturesc++]), l1);
        world->alivec++;
        cloneInto(world, creature, &(world->creatures[world->creaturesc++]), l2);
        world->alivec++;

        // This should be improved. The creature does not actually die.
        creature->alive = false;
        world->alivec--;
        world->reproductionc += 2;
        if (game->selection == creature) {
            game->selection = NULL; // Deselect the creature if it dies
        }
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
            if (game->selection == creature) {
                game->selection = NULL; // Deselect the creature if it dies
            }
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

    if (game->running) {

        game->tick++;

        #ifdef TRACE_ENABLED
            printf("tick %d\n", game->tick);
        #endif

        for (int i = 0; i < world->creaturesc; ++i) {
            updateCreature(&world->creatures[i]);
        }

        if (world->alivec == 0) {
            game->ended = true;
        }
    }

    glutPostRedisplay(); // Request display update
    scheduleUpdate(); // Schedule next update
}

void updateCoordinates(GameView* view) {
    glViewport(0, 0, view->screen.size.width, view->screen.size.height);

    float worldWidth = view->screen.size.width / view->zoom;
    float worldHeight = view->screen.size.height / view->zoom;
    float halfWidth = worldWidth / 2.0f;
    float halfHeight = worldHeight / 2.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(
        view->camera.position.x - halfWidth,
        view->camera.position.x + halfWidth,
        view->camera.position.y - halfHeight,
        view->camera.position.y + halfHeight,
        -1,
        1
    );
    glMatrixMode(GL_MODELVIEW);
}

void reshape(int width, int height) {
    view->screen.size.width = width;
    view->screen.size.height = height;
    updateCoordinates(view);
}

void moveCamera(int dx, int dy) {
    view->camera.position.x += dx;
    view->camera.position.y += dy;

    // Calculate half of the visible area in world units
    float halfWidth = (view->screen.size.width / view->zoom) / 2.0f;
    float halfHeight = (view->screen.size.height / view->zoom) / 2.0f;

    // Clamp camera position so the visible area stays within the world
    if (view->camera.position.x - halfWidth < 0)
        view->camera.position.x = halfWidth;
    if (view->camera.position.x + halfWidth > world->size.width)
        view->camera.position.x = world->size.width - halfWidth;
    if (view->camera.position.y - halfHeight < 0)
        view->camera.position.y = halfHeight;
    if (view->camera.position.y + halfHeight > world->size.height)
        view->camera.position.y = world->size.height - halfHeight;

    updateCoordinates(view);
    scheduleUpdate();
}

void zoomCamera(float zoomFactor) {
    float minZoomX = view->screen.size.width / world->size.width;
    float minZoomY = view->screen.size.height / world->size.height;
    float minZoom = (minZoomX < minZoomY) ? minZoomX : minZoomY;

    view->zoom *= zoomFactor;
    if (view->zoom < minZoom) {
        view->zoom = minZoom;
    }
    updateCoordinates(view);
    scheduleUpdate();
}

void zoomIn() {
    zoomCamera(0.9f);
}

void zoomOut() {
    zoomCamera(1.1f);
}

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // Escape key
            glutLeaveMainLoop(); // Exit the main loop
            break;
        case 'p': // Pause the game
        case 'P':
            game->running = !game->running; // Toggle the running state
            break;
        case 'i':
        case 'I':
            game->displayInformation = !game->displayInformation;
            break;
        case 'f': // 'f' for 'fast'
            game->updateInterval = game->updateInterval > SPEED_DELTA ? game->updateInterval - SPEED_DELTA : SPEED_DELTA; // Increase speed
            scheduleUpdate();
            break;
        case 's': // 's' for 'slow'
            game->updateInterval += SPEED_DELTA;
            scheduleUpdate();
            break;
        case '+':
            zoomIn();
            break;
        case '-':
            zoomOut();
            break;
    }
}

void handleSpecialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            moveCamera(0, 1.0f * view->zoom);
            break;
        case GLUT_KEY_DOWN:
            moveCamera(0, -1.0f * view->zoom);
            break;
        case GLUT_KEY_LEFT:
            moveCamera(-1.0f * view->zoom, 0.0f);
            break;
        case GLUT_KEY_RIGHT:
            moveCamera(1.0f * view->zoom, 0.0f);
            break;
    }
}

PointI screenToWorld(int x, int y) {
    // Invert y to match OpenGL coordinates
    int invertedY = view->screen.size.height - y;

    // Calculate visible world size in world units
    float visibleWorldWidth = view->screen.size.width / view->zoom;
    float visibleWorldHeight = view->screen.size.height / view->zoom;

    // Calculate the world coordinates of the bottom-left corner of the screen
    float worldOriginX = view->camera.position.x - visibleWorldWidth / 2.0f;
    float worldOriginY = view->camera.position.y - visibleWorldHeight / 2.0f;

    // Map screen coordinates to world coordinates, accounting for zoom and camera
    PointI point;
    point.x = (int)(worldOriginX + x / view->zoom);
    point.y = (int)(worldOriginY + invertedY / view->zoom);
    return point;
}

void selectCreature(int x, int y) {
    PointI point = screenToWorld(x, y);
    if (point.x < 0 || point.x >= world->size.width || point.y < 0 || point.y >= world->size.height) {
        return; // Out of bounds
    }
    Cell* cell = &world->cells[point.y * world->size.width + point.x];
    game->selection = cell->creature != NULL ? cell->creature : NULL;
}

void handleMouseClick(int button, int state, int x, int y) {
    switch (state) {
        case GLUT_DOWN:
            if (button == GLUT_LEFT_BUTTON) {
                view->camera.dragging = true;
                lastMousePosition.x = x;
                lastMousePosition.y = y;
                selectCreature(x, y);
            }
            break;
        case GLUT_UP:
            view->camera.dragging = false;
            break;
    }    
}

void handleMouseWheel(int wheel, int direction, int x, int y) {
    if (direction > 0) {
        zoomIn();
    } else {
        zoomOut();
    }
}

void handleMouseMotion(int x, int y) {
    view->mousePosition = screenToWorld(x, y); // Update the mouse position in world coordinates
    if (view->camera.dragging) {
        float dx = lastMousePosition.x - x;
        float dy = y - lastMousePosition.y;
        moveCamera(dx / view->zoom, dy / view->zoom);
        lastMousePosition.x = x;
        lastMousePosition.y = y;
    }
}

void handlePassiveMouseMotion(int x, int y) {
    view->mousePosition = screenToWorld(x, y); // Update the mouse position in world coordinates
}

void usage() {
    printf("Usage: life [seed] [initial creatures count] [use gene pool] [gene pool size]\n");
    printf("\t'-h' or '--help': get command line help.\n");
    printf("\t'-s' or '--seed': seed to use for randomization; the next parameter must be a positive integer number.\n");
    printf("\t'-c' or '--creatures': initial creatures count; the next parameter must be a positive integer number. Default: %d\n", INITIAL_CREATURES_COUNT);
    printf("\t'-g' or '--genepool': use a gene pool instead of random DNA generation for each creature.\n");
    printf("\t'-p' or '--poolsize': the number of DNAs in the gene pool; the next parameter must be a positive integer number. Default: %d.\n", GENE_POOL_SIZE);
    printf("\t'-u' or '--update': the update interval for animation; the next parameter must be a positive integer number. Default: %d.\n", UPDATE_INTERVAL);
    printf("\t'-w' or '--width': width of the world.");
    printf("\t'-h' or '--height': height of the world.");
    printf("\t'-m' or '--mutation': probability of mutation when reproducing.");
}

bool isParam(char* value, const char* shortParam, const char* longParam) {
    return strcmp(value, shortParam) == 0 || strcmp(value, longParam) == 0;
}

Parameters parseParameters(int argc, char** argv) {
    Parameters p = {
        .useSeed = false,
        .useGenePool = false,
        .initialCreaturesCount = INITIAL_CREATURES_COUNT,
        .updateInterval = UPDATE_INTERVAL,
        .useWorldSize = false,
        .worldSize = { .width = 0, .height = 0 },
        .mutationProbability = MUTATION_PROBABILITY
    };
    for (int i = 1; i < argc; ++i) {
        if (isParam(argv[i], "-s", "--seed") && i + 1 < argc) {
            p.useSeed = true;
            p.seed = (unsigned int)atoi(argv[++i]);
        }
        if (isParam(argv[i], "-c", "--creatures") && i + 1 < argc) {
            p.initialCreaturesCount = (unsigned int)atoi(argv[++i]);
        }
        if (isParam(argv[i], "-g", "--genepool")) {
            p.useGenePool = true;
        }
        if (isParam(argv[i], "-p", "--poolsize") && i + 1 < argc) {
            p.genePoolSize = (unsigned int)atoi(argv[++i]);
        }
        if (isParam(argv[i], "-u", "--update") && i + 1 < argc) {
            p.updateInterval = (unsigned int)atoi(argv[++i]);
        }
        if (isParam(argv[i], "-w", "--width") && i + 1 < argc) {
            p.useWorldSize = true;
            p.worldSize.width = (unsigned int)atoi(argv[++i]);
        }
        if (isParam(argv[i], "-h", "--height") && i + 1 < argc) {
            p.useWorldSize = true;
            p.worldSize.height = (unsigned int)atoi(argv[++i]);
        }
        if (isParam(argv[i], "-m", "--mutation") && i + 1 < argc) {
            p.mutationProbability = atof(argv[++i]);
        }
    }
    if ((p.worldSize.width != 0 && p.worldSize.height == 0) 
    || (p.worldSize.width == 0 && p.worldSize.height != 0)) {
        fprintf(stderr, "Error: world size must be specified in both dimensions.\n");
        usage();
        exit(1);
    }
    return p;
}

void freeMemory() {
    if (world->cells) {
        free(world->cells);
    }
    if (world->creatures) {
        free(world->creatures);
    }
    if (game->useGenePool && game->genePool.genomes) {
        free(game->genePool.genomes);
    }
}

static inline bool safe_size_t_mul(size_t a, size_t b, uintmax_t* result) {
    if (b != 0 && a > SIZE_MAX / b) {
        return false; // overflow would occur
    }
    *result = (uintmax_t)a * (uintmax_t)b;
    return true;
}

double getTimeMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

void idleFunc() {
    double now = getTimeMs();
    // This is how we limit the frame rate to FRAME_RATE FPS.
    if (now - game->lastFrameTime >= FRAME_TIME_MS) {
        game->lastFrameTime = now;
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        usage();
        return 0;
    }
    Parameters params = parseParameters(argc, argv);

    srand(params.useSeed ? params.seed : (unsigned int)time(NULL)); // Seed the random number generator

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    SizeI screenSize = (SizeI) { .width = 1200, .height = 600 };
    SizeI worldSize = params.useWorldSize ? params.worldSize : screenSize;
        
    game = &(Game) { 
        .tick = 0,
        .running = false,
        .ended = false,
        .displayInformation = true,
        .updateInterval = params.updateInterval,
        .timerScheduled = false,
        .selection = NULL,
        .useGenePool = params.useGenePool,
        .genePool = (GenePool) { .genomes = NULL, .size = params.genePoolSize },
        .mutationProbability = params.mutationProbability,
        .lastFrameTime = getTimeMs()
    };

    uintmax_t maxPopulation;
    if (!safe_size_t_mul(worldSize.width, worldSize.height, &maxPopulation)) {
        fprintf(stderr, "Error: world size is too large.\n");
        freeMemory();
        exit(EXIT_FAILURE);
    }
    world = &(World) { 
        .size = worldSize,
        .maxPopulation = maxPopulation,
        .creaturesc = params.initialCreaturesCount <= maxPopulation ? params.initialCreaturesCount : maxPopulation
    };

    // The camera is initialized to the center of the world.
    view = &(GameView) {
        .screen = {
            .size = (SizeF) {
                .width = screenSize.width,
                .height = screenSize.height
            }
        },
        .camera = {
            .position = (PointF) {
                .x = world->size.width / 2.0f,
                .y = world->size.height / 2.0f
            },
            .dragging = false
        },
        .zoom = 1.0f
    };
    
    // Set zoom so the visible area matches the world size
    float minZoomX = view->screen.size.width / world->size.width;
    float minZoomY = view->screen.size.height / world->size.height;
    float minZoom = (minZoomX < minZoomY) ? minZoomX : minZoomY;
    view->zoom = minZoom;

    initGenePool(game);
    initWorld(game, world);
    
    glutInitWindowSize(screenSize.width, screenSize.height);
    glutCreateWindow("Life Simulator");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idleFunc); // Register the idle callback

    glutKeyboardFunc(handleKeypress); // Register the keyboard callback
    glutSpecialFunc(handleSpecialKeys);
    glutMouseFunc(handleMouseClick); // Register the mouse callback
    glutMouseWheelFunc(handleMouseWheel); // Register the mouse wheel callback
    glutMotionFunc(handleMouseMotion); // Register the mouse motion callback
    glutPassiveMotionFunc(handlePassiveMouseMotion);
    scheduleUpdate();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color

    glutMainLoop();

    freeMemory();
    
    return 0;
}