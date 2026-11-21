#pragma once

#include <stdint.h>

class SevenSegmentDisplay {
  public:
    enum SevenSegmentDisplayChar {
        NONE,
        ZERO,
        ONE,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        A,
        b,
        C,
        d,
        E,
        F,
        P,
        U,
        S,
        HYPHEN,
        UNDERSCORE,
        TOP_RIGHT_VERTICAL_LINE,
        BOTTOM_RIGHT_VERTICAL_LINE,
        TOP_HORIZONTAL_LINE
    };

    SevenSegmentDisplay(SevenSegmentDisplayChar dispChar) : dispChar(dispChar) {}
    SevenSegmentDisplay() : dispChar(SevenSegmentDisplay::NONE) {}

    uint8_t toByte() const {
        switch (dispChar) {
        case ZERO:
            return 63;
        case ONE:
            return 6;
        case TWO:
            return 91;
        case THREE:
            return 79;
        case FOUR:
            return 102;
        case FIVE:
            return 109;
        case SIX:
            return 125;
        case SEVEN:
            return 7;
        case EIGHT:
            return 127;
        case NINE:
            return 111;
        case A:
            return 119;
        case b:
            return 124;
        case C:
            return 57;
        case d:
            return 94;
        case E:
            return 121;
        case F:
            return 113;
        case P:
            return 115;
        case U:
            return 62;
        case S:
            return 109;
        case HYPHEN:
            return 64;
        case UNDERSCORE:
            return 8;
        case TOP_RIGHT_VERTICAL_LINE:
            return 2;
        case BOTTOM_RIGHT_VERTICAL_LINE:
            return 4;
        case TOP_HORIZONTAL_LINE:
            return 1;
        case NONE:
        default:
            return 0;
        }
    }

    uint8_t toByte(char chararcter) {
        switch (chararcter) {
        case '0':
            return SevenSegmentDisplay(SevenSegmentDisplay::ZERO).toByte();
        case '1':
            return SevenSegmentDisplay(SevenSegmentDisplay::ONE).toByte();
        case '2':
            return SevenSegmentDisplay(SevenSegmentDisplay::TWO).toByte();
        case '3':
            return SevenSegmentDisplay(SevenSegmentDisplay::THREE).toByte();
        case '4':
            return SevenSegmentDisplay(SevenSegmentDisplay::FOUR).toByte();
        case '5':
            return SevenSegmentDisplay(SevenSegmentDisplay::FIVE).toByte();
        case '6':
            return SevenSegmentDisplay(SevenSegmentDisplay::SIX).toByte();
        case '7':
            return SevenSegmentDisplay(SevenSegmentDisplay::SEVEN).toByte();
        case '8':
            return SevenSegmentDisplay(SevenSegmentDisplay::EIGHT).toByte();
        case '9':
            return SevenSegmentDisplay(SevenSegmentDisplay::NINE).toByte();
        case 'A':
            return SevenSegmentDisplay(SevenSegmentDisplay::A).toByte();
        case 'b':
            return SevenSegmentDisplay(SevenSegmentDisplay::b).toByte();
        case 'C':
            return SevenSegmentDisplay(SevenSegmentDisplay::C).toByte();
        case 'd':
            return SevenSegmentDisplay(SevenSegmentDisplay::d).toByte();
        case 'E':
            return SevenSegmentDisplay(SevenSegmentDisplay::E).toByte();
        case 'F':
            return SevenSegmentDisplay(SevenSegmentDisplay::F).toByte();
        case 'P':
            return SevenSegmentDisplay(SevenSegmentDisplay::P).toByte();
        case 'U':
            return SevenSegmentDisplay(SevenSegmentDisplay::U).toByte();
        case 'S':
            return SevenSegmentDisplay(SevenSegmentDisplay::S).toByte();
        case '-':
            return SevenSegmentDisplay(SevenSegmentDisplay::HYPHEN).toByte();
        case '_':
            return SevenSegmentDisplay(SevenSegmentDisplay::UNDERSCORE).toByte();
        case '\'':
            return SevenSegmentDisplay(SevenSegmentDisplay::TOP_RIGHT_VERTICAL_LINE).toByte();
        case ',':
            return SevenSegmentDisplay(SevenSegmentDisplay::BOTTOM_RIGHT_VERTICAL_LINE).toByte();
        case '`':
            return SevenSegmentDisplay(SevenSegmentDisplay::TOP_HORIZONTAL_LINE).toByte();
        default:
            return SevenSegmentDisplay(SevenSegmentDisplay::NONE).toByte();
        }
    }

  private:
    SevenSegmentDisplayChar dispChar;
};
