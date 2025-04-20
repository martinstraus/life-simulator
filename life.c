#include <stdio.h>
#include <time.h>
#include <GL/freeglut.h>
#include <primitives.h>

#define CREATURE_SIZE (SizeI) { .width = 5, .height = 5 }
typedef uint32_t ADN;
typedef uint32_t Energy;

#define ENERGY_MAX 1000

typedef struct {
    PointI location;
    ADN adn;
    Energy energy;
    bool alive;
} Creature;

typedef struct {
    SizeI size;
    Creature* creatures;
    unsigned int creaturesc;
    unsigned int alivec; // Number of alive creatures
} World;

typedef uint32_t Tick;

typedef struct {
    Tick tick;
    bool running;
} Game;

World* world;
Game* game;

void initCreatures(World* world) {
    world->creatures = (Creature*)malloc(world->creaturesc * sizeof(Creature));
    for (unsigned int i = 0; i < world->creaturesc; ++i) {
        world->creatures[i].location.x = rand() % world->size.width;
        world->creatures[i].location.y = rand() % world->size.height;
        world->creatures[i].adn = (uint32_t)rand() | ((uint32_t)rand() << 16);
        world->creatures[i].energy = (100 + rand()) % ENERGY_MAX;
        world->creatures[i].alive = true;
    }
    world->alivec = world->creaturesc;
}

void initWorld(World* world) {
    initCreatures(world);
}

void setColorForCreature(Creature* creature) {
    float r = ((creature->adn & 0xFF0000) >> 16) / 255.0f;
    float g = ((creature->adn & 0x00FF00) >> 8) / 255.0f;
    float b = (creature->adn & 0x0000FF) / 255.0f;
    glColor3f(r, g, b);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < world-> creaturesc; ++i) {
        Creature* creature = &world->creatures[i];
        if (creature->alive) {
        setColorForCreature(creature);
        glBegin(GL_QUADS);
        glVertex2f(creature->location.x, creature->location.y);
        glVertex2f(creature->location.x + CREATURE_SIZE.width, creature->location.y);
        glVertex2f(creature->location.x + CREATURE_SIZE.width, creature->location.y + CREATURE_SIZE.height);
        glVertex2f(creature->location.x, creature->location.y + CREATURE_SIZE.height);
        glEnd();
        }
    }
    // Draw the world bounds
    glutSwapBuffers();
}

// Update game state here
void update(int value) {
    game->tick++;
    //printf("tick %d\n", game->tick);
    for (int i = 0; i < world->creaturesc; ++i) {
        Creature* creature = &world->creatures[i];
        if (creature->alive) {
            creature->energy--;
            if (creature->energy == 0) {
                creature->alive = false; // Mark as dead if energy is depleted
                world->alivec--;
            }
        }
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

    game = &(Game) { .tick = 0 };
    world = &(World) { 
        .size = (SizeI) { 1200, 600 }, 
        .creaturesc = 1000 
    };
    initWorld(world);

    glutInitWindowSize(world->size.width, world->size.height);
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