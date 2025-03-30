#include <GL/freeglut.h>
#include <primitives.h>

#define CREATURE_SIZE (SizeI) { .width = 5, .height = 5 }
typedef uint32_t ADN;

typedef struct {
    PointI location;
    ADN adn;
} Creature;

typedef struct {
    SizeI size;
    Creature* creatures;
    unsigned int creaturesc;
} World;

World* world;

void initWorld(World* world, unsigned int creatures) {
    world->creatures = (Creature*)malloc(creatures * sizeof(Creature));
    world->creaturesc = creatures;
    for (unsigned int i = 0; i < creatures; ++i) {
        world->creatures[i].location.x = rand() % world->size.width;
        world->creatures[i].location.y = rand() % world->size.height;
        world->creatures[i].adn = rand();
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.0f, 0.0f, 1.0f); // Set color to blue
    for (int i = 0; i < world-> creaturesc; ++i) {
        Creature* creature = &world->creatures[i];
        glBegin(GL_QUADS);
        glVertex2f(creature->location.x, creature->location.y);
        glVertex2f(creature->location.x + CREATURE_SIZE.width, creature->location.y);
        glVertex2f(creature->location.x + CREATURE_SIZE.width, creature->location.y + CREATURE_SIZE.height);
        glVertex2f(creature->location.x, creature->location.y + CREATURE_SIZE.height);
        glEnd();
    }
    // Draw the world bounds
    glutSwapBuffers();
}

void update(int value) {
    // Update game state here

    glutPostRedisplay(); // Request display update
    glutTimerFunc(16, update, 0); // Schedule next update (~60 FPS)
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    world = &(World) { .size = (SizeI) { 1200, 600 } };
    initWorld(world,  1000);

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