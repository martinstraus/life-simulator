#include "geometry.h"

Quad makeSquare(Point center, float size) {
    float half = size / (float) 2;
    Quad q = {
        { center.x - half, center.y + half },
        { center.x + half, center.y + half },
        { center.x + half, center.y - half },
        { center.x - half, center.y - half }
    };
    return q;
}