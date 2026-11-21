#include <Ethernet.h>
#include <Game.h>
#include <LedUtil.h>
#include <Mode.h>
#include <RTClib.h>
#include <SPI.h>
#include <Servo.h>
#include <SevenSegmentDisplay.h>
#include <State.h>
#include <Wire.h>

const byte DIO_4DIG = A0;
const byte CLK_4DIG = A1;

const byte STB_LK = 10;
const byte CLK_LK = 9;
const byte DIO_LK = 8;
const byte WHITE_LED = 6;
const byte SERVO_PIN = 5;
const byte SW_ENC = 4;
const byte DT_ENC = 3;
const byte CLK_ENC = 2;

const int LED_SIZE = 8;
const int BUTTONS_SIZE = 8;
const int DISPLAYS_AMOUNT = 3;
const int DISPLAYS_DIGIT_AMOUNT = 4;

TM1638plus lk(STB_LK, CLK_LK, DIO_LK, false);
TM1637plus_modelX display4dig(CLK_4DIG, DIO_4DIG, 100, 4);
RTC_DS3231 rtc;
Servo servo;

byte lastSW;
byte lastCLK;
byte buttonsMask = 0;
byte prevButtonsMask = 0;
byte difficulty = 4;
byte displayBrightness = 7;

uint8_t servoAngle = 0;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

bool leds[LED_SIZE] = {false};

char currentTimeString[5];

int encPos = 0;
int prevEncPos = 0;
int score = 0;

unsigned long lastPress = 0;
unsigned long blinkRate = 500;
unsigned long prevBlinkTime = 0;
unsigned long lastTimeUpdate = 0;
unsigned long timeSec = 0;
unsigned long currentTime = 0;

volatile Mode mode = Mode::GAME;

State state =
    State(mode, LED_SIZE, BUTTONS_SIZE, DISPLAYS_AMOUNT, DISPLAYS_DIGIT_AMOUNT, servoAngle, encPos);

LedUtil ledUtil = LedUtil();
Game game = Game(score, difficulty, Lane::BOTTOM, GameState::READY, lk, display4dig);

EthernetServer server(80);

void setDisplayChar(uint8_t displayIndexI, uint8_t displayIndexJ, char character) {
    state.setDisplay(displayIndexI, displayIndexJ, SevenSegmentDisplay().toByte(character));
}

void setDisplayChar(uint8_t displayIndexI, int number) {
    for (int i = 3; i >= 0; i--) {
        char digit = number % 10 + '0';

        setDisplayChar(displayIndexI, i, digit);
        number /= 10;
    }
}

void setDisplayChar(uint8_t displayIndexI, char arr[4]) {
    for (int i = 3; i >= 0; i--) {
        setDisplayChar(displayIndexI, i, arr[i]);
    }
}

void updateServoAngle(bool addition, bool subtraction) {
    if (state.getServoRotateFlag()) {
        if (addition && servoAngle < 180) {
            state.setServoAngle(servoAngle++);
        }

        if (subtraction && servoAngle > 0) {
            state.setServoAngle(servoAngle--);
        }
    }
}

void updateCurrentTimeString(char *currentTimeString) {
    DateTime now = rtc.now();
    sprintf(currentTimeString, "%02d%02d", now.hour(), now.minute());
}

void changeDisplayBrightness() {
    lk.brightness(displayBrightness++);
    displayBrightness = displayBrightness > 7 ? 0 : displayBrightness;
}

void updateLeds() {
    for (byte i = 0; i < state.getLedsSize(); i++) {
        lk.setLED(i, state.getLed(i));
    }
}

byte getBitIndex(unsigned char mask) {
    for (byte i = 0; i < 8; i++) {
        if ((mask >> i) & 1) {
            return i;
        }
    }
    return -1;
}

void readEncPos() {
    byte currentCLK = digitalRead(CLK_ENC);
    if (lastCLK == HIGH && currentCLK == LOW) {
        prevEncPos = state.getEncoderPosition();
        state.setEncMoveFlag(true);

        if (digitalRead(DT_ENC) != currentCLK) {
            state.setEncoderPosition(state.getEncoderPosition() + 1);
            updateServoAngle(true, false);
        } else {
            state.setEncoderPosition(state.getEncoderPosition() - 1);
            updateServoAngle(false, true);
        }
    }
    lastCLK = currentCLK;
}

bool readEncPress() {
    bool isPressed = false;
    byte currentSW = digitalRead(SW_ENC);

    if (lastSW == HIGH && currentSW == LOW) {
        state.changeMode();
        isPressed = true;
    }

    lastSW = currentSW;
    return isPressed;
}

void initGameDifficultyLeds() {
    for (int i = 0; i < state.getLedsSize(); i++) {
        state.setLed(i, i < game.getDifficulty());
    }
}

void rotateServo(byte angle) {
    if (angle >= 0 && angle <= 180) {
        servo.write(angle);
    }
}

void readEncoder() {
    readEncPos();
    readEncPress();
    rotateServo(state.getServoAngle());
}

void updateState() { updateLeds(); }

void switchMode(int btnNum) {
    state.resetFlags();

    switch (state.getMode()) {
    case Mode::GAME:
        switch (btnNum) {
        case 0:
            blinkRate = 0;
            game.start();

            initGameDifficultyLeds();
            break;

        case 1:
            game.setGameState(GameState::PAUSE);
            break;

        case 2:
            break;

        case 3:
            game.reset();
            lk.reset();
            break;

        case 4:
            break;

        case 5:
            break;

        case 6:
            blinkRate = 0;
            game.setDifficulty(game.getDifficulty() - 1);
            initGameDifficultyLeds();
            break;

        case 7:
            blinkRate = 0;
            game.setDifficulty(game.getDifficulty() + 1);
            initGameDifficultyLeds();
            break;

        default:
            break;
        }
        break;

    case Mode::LED:
        switch (btnNum) {
        case 0:
            state.setBlinkFlag(!state.getBlinkFlag());
            break;

        case 1:
            blinkRate += 25;
            blinkRate = blinkRate > __LONG_MAX__ ? __LONG_MAX__ : blinkRate;
            break;

        case 2:
            blinkRate -= 25;
            blinkRate = blinkRate > __LONG_MAX__ ? 0 : blinkRate;
            break;

        case 3:
            changeDisplayBrightness();
            break;

        case 4:
            break;

        case 5:
            ledUtil.evenFlip(leds, state.getLedsSize());
            state.setLeds(leds);
            break;

        case 6:
            ledUtil.oddFlip(leds, state.getLedsSize());
            state.setLeds(leds);
            break;

        case 7:
            ledUtil.flip(leds, state.getLedsSize());
            state.setLeds(leds);
            break;

        default:
            break;
        }
        break;

    case Mode::LED_INDVD:
        switch (btnNum) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            state.setLed(btnNum, !state.getLed(btnNum));
            break;

        default:
            break;
        }
        break;

    case Mode::SHOW:
        switch (btnNum) {
        case 0:
            state.resetFlags();
            state.setShowTimeFlag(!state.getShowTimeFlag());

            lk.reset();
            break;

        case 1:
            state.resetFlags();
            state.setShowEncPosFlag(!state.getShowEncPosFlag());

            lk.reset();
            break;

        case 2:
            break;

        case 3:
            lk.reset();
            break;

        case 4:
            lk.display7Seg(4, Obstacle(Lane::BOTTOM).getBinaryValue());
            break;

        case 5:
            lk.display7Seg(5, Obstacle(Lane::MIDDLE).getBinaryValue());
            break;

        case 6:
            lk.display7Seg(6, Obstacle(Lane::TOP).getBinaryValue());
            break;

        case 7:
            lk.display7Seg(7, Obstacle(game.getRandLane()).getBinaryValue());
            break;

        default:
            break;
        }
        break;

    case Mode::SERVO:
        switch (btnNum) {
        case 0:
            state.resetFlags();
            state.setServoRotateFlag(!state.getServoRotateFlag());
            state.setShowServoRotateAngle(!state.getShowServoRotateAngle());

            lk.reset();
            break;

        case 1:
            break;

        case 2:
            break;

        case 3:
            break;

        case 4:
            break;

        case 5:
            break;

        case 6:
            break;

        case 7:
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

bool getStateJson(char *buffer, size_t bufferSize) {
    size_t pos = 0;

    unsigned int written =
        snprintf(buffer, bufferSize, "{\"mode\":%u,\"leds\":[", static_cast<int>(state.getMode()));

    if (written < 0 || written >= bufferSize) {
        return false;
    }

    pos += written;

    for (uint8_t i = 0; i < LED_SIZE; i++) {
        written = snprintf(buffer + pos, bufferSize - pos, "%s%s", i > 0 ? "," : "",
                           state.getLed(i) ? "true" : "false");

        if (written < 0 || written >= bufferSize - pos) {
            return false;
        }

        pos += written;
    }

    written = snprintf(buffer + pos, bufferSize - pos, "],\"displays\":[");

    pos += written;

    for (int i = 0; i < 3; i++) {
        written = snprintf(buffer + pos, bufferSize - pos, "%s[", i > 0 ? "," : "");
        pos += written;

        for (int j = 0; j < 4; j++) {
            written = snprintf(buffer + pos, bufferSize - pos, "%s%u", j > 0 ? "," : "",
                               state.getDisplay(i, j));
            pos += written;
        }

        written = snprintf(buffer + pos, bufferSize - pos, "]");
        pos += written;
    }

    written = snprintf(buffer + pos, bufferSize - pos, "],");

    if (written < 0 || written >= bufferSize - pos) {
        return false;
    }

    pos += written;

    written =
        snprintf(buffer + pos, bufferSize - pos, "\"encoder\":%d}", state.getEncoderPosition());

    if (written < 0 || written >= bufferSize - pos) {
        return false;
    }

    return true;
}

void handleHttpClient() {
    EthernetClient client = server.available();

    if (!client) {
        return;
    }

    IPAddress remoteIP = client.remoteIP();
    uint16_t remotePort = client.remotePort();

    if (remoteIP == IPAddress(0, 0, 0, 0) || remotePort == 0) {
        client.stop();
        return;
    }

    char request[100] = {0};
    uint8_t index = 0;
    bool firstLineDone = false;

    unsigned long startTime = millis();

    while (client.connected() && millis() - startTime < 500) {
        while (client.available()) {
            char c = client.read();

            if (c == '\n') {
                request[index] = '\0';
                firstLineDone = true;
                break;
            }

            if (c != '\r' && index < sizeof(request) - 1) {
                request[index++] = c;
            }
        }

        if (firstLineDone) {
            break;
        }
    }

    if (!firstLineDone) {
        client.stop();
        return;
    }

    if (strncmp(request, "GET /api/state", 14) == 0) {
        char response[135];
        getStateJson(response, sizeof(response));

        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Content-Type: application/json"));
        client.println(F("Access-Control-Allow-Origin: *"));
        client.println(F("Connection: close"));
        client.print(F("Content-Length: "));
        client.println(strlen(response));

        client.println();
        client.print(response);
    } else {
        client.println(F("HTTP/1.1 404 Not Found"));
        client.println(F("Access-Control-Allow-Origin: *"));
        client.println(F("Connection: close"));
        client.println();
    }

    client.stop();
}

void initServer() {
    if (Ethernet.begin(mac) == 0) {
        Serial.println(F("DHCP failed!"));
        while (true) {
            ;
        }
    }

    Serial.println(Ethernet.localIP());

    server.begin();

    Serial.println(F("HTTP server started!"));
}

void setup() {
    Serial.begin(115200);

    lk.displayBegin();
    display4dig.displayBegin();
    display4dig.setBrightness(displayBrightness, true);

    pinMode(SW_ENC, INPUT_PULLUP);
    pinMode(DT_ENC, INPUT_PULLUP);
    pinMode(CLK_ENC, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(DT_ENC), readEncoder, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_ENC), readEncoder, FALLING);

    pinMode(10, OUTPUT);

    servo.attach(SERVO_PIN);
    rotateServo(0);

    analogWrite(WHITE_LED, 255);

    Wire.begin();

    if (!rtc.begin()) {
        while (true)
            ;
    }

    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    srand(rtc.now().secondstime());

    initServer();
}

void loop() {
    currentTime = millis();
    timeSec = currentTime / 1000;
    buttonsMask = lk.readButtons();

    if (readEncPress()) {
        if (game.getGameState() == GameState::PLAYING) {
            game.setGameState(GameState::PAUSE);
        }
        switchMode(-1);
    } else if (currentTime - lastPress > 100 && buttonsMask != 0 && prevButtonsMask == 0) {
        lastPress = currentTime;
        byte btnNum = getBitIndex(buttonsMask);
        switchMode(btnNum);
    }

    prevButtonsMask = buttonsMask;

    if (state.getShowTimeFlag()) {
        lk.displayIntNum(timeSec, false, TMAlignTextRight);
        setDisplayChar(1, timeSec);
    }

    if (state.getShowEncPosFlag()) {
        lk.displayIntNum(state.getEncoderPosition(), false, TMAlignTextRight);
        setDisplayChar(1, state.getEncoderPosition());
    }

    if (state.getMode() == Mode::SERVO && state.getShowServoRotateAngle() &&
        state.getEncMoveFlag()) {

        lk.displayIntNum(servoAngle, false, TMAlignTextRight);
        setDisplayChar(1, servoAngle);
        rotateServo(servoAngle);
        state.setEncMoveFlag(false);
    }

    if (state.getMode() == Mode::LED) {
        analogWrite(WHITE_LED, map(state.getEncoderPosition() * 10, 0, 1000, 255, 0));
    }

    if (state.getMode() == Mode::GAME && game.getGameState() == GameState::PLAYING &&
        state.getEncoderPosition() != prevEncPos && state.getEncMoveFlag()) {
        prevEncPos < state.getEncoderPosition() ? game.moveUp() : game.moveDown();
        state.setEncMoveFlag(false);
    }

    game.update(currentTime);

    if (game.getGameState() == GameState::READY && currentTime - lastTimeUpdate > 1000) {
        lastTimeUpdate = currentTime;
        updateCurrentTimeString(currentTimeString);
        state.setShowDisplaySemicolon(timeSec % 2);

        setDisplayChar(2, currentTimeString);

        display4dig.DisplayString(currentTimeString, state.getShowDisplaySemicolon() ? 0x40 : 0, 4,
                                  0);
    }

    if (state.getBlinkFlag() && currentTime - prevBlinkTime > blinkRate && blinkRate > 0) {
        ledUtil.flip(leds, state.getLedsSize());
        state.setLeds(leds);
        prevBlinkTime = currentTime;
    }

    updateState();
    handleHttpClient();
}