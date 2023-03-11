#ifndef GEOMETRY_H
#define GEOMETRY_H

struct Size {
    int height, width;
};

typedef struct Size Size;

struct Point {
    float x, y;
};

typedef struct Point Point;

struct Quad {
    struct Point a, b, c, d;
};

typedef struct Quad Quad;

struct Quad makeSquare(struct Point center, float size);

#endif