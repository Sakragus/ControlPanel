#include <SevenSegmentDisplay.h>
#include <State.h>

State::State(Mode mode, int ledsSize, int buttonsSize, int displaysAmount, int displayDigitAmount,
             uint8_t servoAngle, int encoderPosition)
    : mode(mode), ledsSize(ledsSize), buttonsSize(buttonsSize), displaysAmount(displaysAmount),
      displayDigitAmount(displayDigitAmount), servoAngle(servoAngle),
      encoderPosition(encoderPosition), leds(nullptr), buttons(nullptr), displays(nullptr) {

    this->leds = new bool[ledsSize]{};
    this->buttons = new bool[buttonsSize]{};

    this->displays = new uint8_t *[displaysAmount];

    for (int i = 0; i < displaysAmount; i++) {
        this->displays[i] = new uint8_t[displayDigitAmount];

        for (int j = 0; j < displayDigitAmount; j++) {
            this->displays[i][j] = SevenSegmentDisplay::NONE;
        }
    }
}

State::State() : State(Mode::GAME, 8, 8, 3, 4, 0, 0) {}

State::~State() {
    delete[] leds;
    delete[] buttons;

    for (int i = 0; i < displaysAmount; i++) {
        delete[] displays[i];
    }

    delete[] displays;
}

Mode State::getMode() const { return mode; }

void State::setMode(Mode mode) { this->mode = mode; }

Mode State::changeMode() {
    switch (getMode()) {
    case Mode::GAME:
        setMode(Mode::LED);
        break;
    case Mode::LED:
        setMode(Mode::LED_INDVD);
        break;
    case Mode::LED_INDVD:
        setMode(Mode::SHOW);
        break;
    case Mode::SHOW:
        setMode(Mode::SERVO);
        break;
    case Mode::SERVO:
        setMode(Mode::GAME);
        break;
    default:
        break;
    }

    return getMode();
}

bool State::getLed(uint8_t index) const {
    if (index >= getLedsSize()) {
        return false;
    }
    return leds[index];
}

void State::setLed(uint8_t index, bool isOn) {
    if (index >= getLedsSize()) {
        return;
    }
    leds[index] = isOn;
}

void State::getLeds(bool *leds) const {
    for (uint8_t i = 0; i < getLedsSize(); i++) {
        leds[i] = this->leds[i];
    }
}

void State::setLeds(bool *leds) {
    for (uint8_t i = 0; i < getLedsSize(); i++) {
        this->leds[i] = leds[i];
    }
}

int State::getLedsSize() const { return ledsSize; }

bool State::getButton(uint8_t index) const {
    if (index >= getButtonsSize()) {
        return false;
    }
    return buttons[index];
}

void State::setButton(uint8_t index, bool isPressed) {
    if (index >= getButtonsSize()) {
        return;
    }
    buttons[index] = isPressed;
}

void State::getButtons(bool *buttons) const {
    for (uint8_t i = 0; i < getButtonsSize(); i++) {
        buttons[i] = this->buttons[i];
    }
}

void State::setButtons(bool *buttons) {
    for (uint8_t i = 0; i < getButtonsSize(); i++) {
        this->buttons[i] = buttons[i];
    }
}

int State::getButtonsSize() const { return buttonsSize; }

uint8_t State::getDisplay(uint8_t display, uint8_t digit) const {
    if (display >= getDisplaysAmount() || digit >= getDisplaysDigitAmount()) {
        return 0;
    }
    return displays[display][digit];
}

void State::setDisplay(uint8_t display, uint8_t digit, uint8_t value) {
    if (display >= getDisplaysAmount() || digit >= getDisplaysDigitAmount()) {
        return;
    }
    displays[display][digit] = value;
}

void State::getDisplays(uint8_t **displays) const {
    for (uint8_t i = 0; i < getDisplaysAmount(); i++) {
        for (uint8_t j = 0; j < getDisplaysDigitAmount(); j++) {
            displays[i][j] = this->displays[i][j];
        }
    }
}

void State::setDisplays(uint8_t **displays) {
    for (uint8_t i = 0; i < getDisplaysAmount(); i++) {
        for (uint8_t j = 0; j < getDisplaysDigitAmount(); j++) {
            this->displays[i][j] = displays[i][j];
        }
    }
}

int State::getDisplaysAmount() const { return displaysAmount; }

int State::getDisplaysDigitAmount() const { return displayDigitAmount; }

void State::setServoAngle(uint8_t servoAngle) { this->servoAngle = servoAngle; }

uint8_t State::getServoAngle() const { return servoAngle; }

void State::setEncoderPosition(int encoderPosition) { this->encoderPosition = encoderPosition; }

int State::getEncoderPosition() const { return encoderPosition; }

void State::setShowDisplaySemicolon(bool showDisplaySemicolon) {
    this->showDisplaySemicolon = showDisplaySemicolon;
}

bool State::getShowDisplaySemicolon() const { return showDisplaySemicolon; }

void State::setShowTimeFlag(bool showTimeFlag) { this->showTimeFlag = showTimeFlag; }

void State::setShowEncPosFlag(bool showEncPosFlag) { this->showEncPosFlag = showEncPosFlag; }

void State::setEncMoveFlag(bool encMoveFlag) { this->encMoveFlag = encMoveFlag; }

void State::setServoRotateFlag(bool servoRotateFlag) { this->servoRotateFlag = servoRotateFlag; }

void State::setShowServoRotateAngle(bool showServoRotateAngle) {
    this->showServoRotateAngle = showServoRotateAngle;
}

void State::setBlinkFlag(bool blinkFlag) { this->blinkFlag = blinkFlag; }

bool State::getShowTimeFlag() const { return showTimeFlag; }

bool State::getShowEncPosFlag() const { return showEncPosFlag; }

bool State::getEncMoveFlag() const { return encMoveFlag; }

bool State::getServoRotateFlag() const { return servoRotateFlag; }

bool State::getShowServoRotateAngle() const { return showServoRotateAngle; }

bool State::getBlinkFlag() const { return blinkFlag; }

void State::resetFlags() {
    showTimeFlag = false;
    showEncPosFlag = false;
    servoRotateFlag = false;
    showServoRotateAngle = false;
}

int State::getBlinkRate() const { return blinkRate; }

void State::setBlinkRate(int blinkRate) { this->blinkRate = blinkRate; }