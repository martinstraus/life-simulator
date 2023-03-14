#include "geometry.h"

Quad makeSquare(PointF center, float size) {
    float half = size / (float) 2;
    Quad q = {
        { center.x - half, center.y + half },
        { center.x + half, center.y + half },
        { center.x + half, center.y - half },
        { center.x - half, center.y - half }
    };
    return q;
}