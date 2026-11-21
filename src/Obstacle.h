#pragma once

#include <GameType.h>

class Obstacle {
  private:
    Lane gap;

  public:
    Obstacle();

    Obstacle(Lane gap);

    int getBinaryValue();

    Lane getGap() const;

    void setGap(Lane gap);
};