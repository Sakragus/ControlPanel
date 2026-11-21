#include <LedUtil.h>

void LedUtil::setLed(bool array[], int size, int index, bool value) {
    if (index < size) {
        array[index] = value;
    }
}

void LedUtil::setAllTrue(bool array[], int size) {
    for (int i = 0; i < size; i++) {
        setLed(array, size, i, true);
    }
}

void LedUtil::setAllFalse(bool array[], int size) {
    for (int i = 0; i < size; i++) {
        setLed(array, size, i, false);
    }
}

void LedUtil::flip(bool array[], int size, int index) {
    if (index < size) {
        setLed(array, size, index, !array[index]);
    }
}

void LedUtil::flip(bool array[], int size) {
    for (int i = 0; i < size; i++) {
        flip(array, size, i);
    }
}

void LedUtil::oddFlip(bool array[], int size) {
    for (int i = 0; i < size; i++) {
        if (i % 2) {
            flip(array, size, i);
        }
    }
}

void LedUtil::evenFlip(bool array[], int size) {
    for (int i = 0; i < size; i++) {
        if (!(i % 2)) {
            flip(array, size, i);
        }
    }
}
