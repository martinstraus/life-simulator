#include "geometry.h"

Quad makeSquareFromBottomLeft(PointF corner, SizeF size) {
    Quad q = {
        { corner.x, corner.y },
        { corner.x, corner.y + size.height },
        { corner.x + size.width, corner.y + size.height },
        { corner.x + size.width, corner.y }
    };
    return q;
}

Quad makeSquareFromCenter(PointF center, float size) {
    float half = size / (float) 2;
    Quad q = {
        { center.x - half, center.y + half },
        { center.x + half, center.y + half },
        { center.x + half, center.y - half },
        { center.x - half, center.y - half }
    };
    return q;
}