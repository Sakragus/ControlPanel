
#pragma once

#include <Mode.h>
#include <stdint.h>

class State {
  private:
    Mode mode;

    int ledsSize;
    int buttonsSize;
    int displaysAmount;
    int displayDigitAmount;
    int encoderPosition;
    int blinkRate;

    bool showTimeFlag;
    bool showEncPosFlag;
    bool encMoveFlag;
    bool servoRotateFlag;
    bool showServoRotateAngle;
    bool showDisplaySemicolon;
    bool blinkFlag;

    bool *leds;
    bool *buttons;

    uint8_t **displays;

    uint8_t servoAngle;

    uint8_t encoderSteps;

  public:
    State(Mode mode, int ledsSize, int buttonsSize, int displaysAmount, int displayDigitAmount,
          uint8_t servoAngle, int encoderPosition);

    State();

    ~State();

    Mode getMode() const;

    void setMode(Mode mode);

    Mode changeMode();

    bool getLed(uint8_t index) const;

    void setLed(uint8_t index, bool isOn);

    void getLeds(bool *leds) const;

    void setLeds(bool *leds);

    int getLedsSize() const;

    bool getButton(uint8_t index) const;

    void setButton(uint8_t index, bool buttonVal);

    void getButtons(bool *leds) const;

    void setButtons(bool *leds);

    int getButtonsSize() const;

    uint8_t getDisplay(uint8_t display, uint8_t value) const;

    void setDisplay(uint8_t display, uint8_t value, uint8_t displayVal);

    void getDisplays(uint8_t **displays) const;

    void setDisplays(uint8_t **displays);

    int getDisplaysAmount() const;

    int getDisplaysDigitAmount() const;

    void setServoAngle(uint8_t servoAngle);

    uint8_t getServoAngle() const;

    void setEncoderPosition(int encoderPosition);

    int getEncoderPosition() const;

    void setShowTimeFlag(bool showTimeFlag);

    void setShowEncPosFlag(bool showEncPosFlag);

    void setEncMoveFlag(bool encMoveFlag);

    void setServoRotateFlag(bool servoRotateFlag);

    void setShowServoRotateAngle(bool showServoRotateAngle);

    void setBlinkFlag(bool blinkFlag);

    void setShowDisplaySemicolon(bool showDisplaySemicolon);

    bool getShowDisplaySemicolon() const;

    bool getShowTimeFlag() const;

    bool getShowEncPosFlag() const;

    bool getEncMoveFlag() const;

    bool getServoRotateFlag() const;

    bool getShowServoRotateAngle() const;

    bool getBlinkFlag() const;

    void resetFlags();

    int getBlinkRate() const;

    void setBlinkRate(int blinkRate);

    int getEncoderSteps();

    // max 20 min 0
    void setEncoderSteps(int step);
};
