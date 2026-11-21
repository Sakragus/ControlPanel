#pragma once

class LedUtil {

  public:
    void flip(bool array[], int size, int index);

    void flip(bool array[], int size);

    void oddFlip(bool array[], int size);

    void evenFlip(bool array[], int size);

    void setLed(bool array[], int size, int index, bool value);

    void setAllTrue(bool array[], int size);

    void setAllFalse(bool array[], int size);
};