#ifndef GEOMETRY_H
#define GEOMETRY_H

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

struct Quad {
    struct PointF a, b, c, d;
};

typedef struct Quad Quad;

struct Quad makeSquare(struct PointF center, float size);

#endif