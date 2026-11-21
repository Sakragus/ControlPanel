#include <Obstacle.h>

Obstacle::Obstacle() : gap(Lane::NONE) {}

Obstacle::Obstacle(Lane gap) : gap(gap) {}

int Obstacle::getBinaryValue() {
    int value = 0;
    switch (gap) {
    case Lane::TOP:
        value = 4;
        break;
    case Lane::MIDDLE:
        value = 6;
        break;
    case Lane::BOTTOM:
        value = 2;
        break;
    case Lane::NONE:
        value = 0;
        break;

    default:
        break;
    }

    return value;
}

Lane Obstacle::getGap() const { return gap; }

void Obstacle::setGap(Lane gap) {}