#include <Ethernet.h>
#include <EthernetUdp.h>
#include <HardwareConfig.h>
#include <RTClib.h>
#include <Servo.h>
#include <SevenSegmentDisplay.h>
#include <State.h>
#include <TM1637Plus.h>
#include <TM1638Plus.h>
#include <Type.h>
#include <Wire.h>

const uint8_t DIO_4DIG = HardwareConfig::DIO_4DIG;
const uint8_t CLK_4DIG = HardwareConfig::CLK_4DIG;
const uint8_t STB_LK = HardwareConfig::STB_LK;
const uint8_t CLK_LK = HardwareConfig::CLK_LK;
const uint8_t DIO_LK = HardwareConfig::DIO_LK;
const uint8_t SERVO_PIN = HardwareConfig::SERVO_PIN;
const uint8_t SW_ENC = HardwareConfig::SW_ENC;
const uint8_t DT_ENC = HardwareConfig::DT_ENC;
const uint8_t CLK_ENC = HardwareConfig::CLK_ENC;

const uint16_t UDP_PORT = 8888;
const uint16_t SERVER_PORT = 8889;

const IPAddress SERVER_IP(192, 168, 70, 16);

const int UPD_TIME = 10000;
const int LED_SIZE = 0;
const int BUTTONS_SIZE = 0;
const int DISPLAYS_AMOUNT = 3;
const int DISPLAYS_DIGIT_AMOUNT = 4;
const int RECONNECT_TIMEOUT = 5000;

TM1638plus lk(STB_LK, CLK_LK, DIO_LK, false);
TM1637plus_modelX display4dig(CLK_4DIG, DIO_4DIG, 100, 4);
RTC_DS3231 rtc;
Servo servo;

uint8_t displayBrightness = 7;
uint8_t encHalfStep = 0;
uint8_t servoAngle = 0;

byte lastSW;
byte lastCLK;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

char currentTimeString[5];

bool isConnected = false;

int encPos = 0;
int prevEncPos = 0;
int score = 0;

unsigned long lastPress = 0;
unsigned long blinkRate = 500;
unsigned long prevBlinkTime = 0;
unsigned long lastTimeUpdate = 0;
unsigned long timeSec = 0;
unsigned long currentTime = 0;
unsigned long lastReconnectTime = 0;
unsigned long lastUpdateTime = 0;

volatile Mode mode = Mode::SERVO;

State state =
    State(mode, LED_SIZE, BUTTONS_SIZE, DISPLAYS_AMOUNT, DISPLAYS_DIGIT_AMOUNT, servoAngle, encPos);

EthernetServer server(80);
EthernetUDP udp;

void setDisplayChar(uint8_t displayIndex, uint8_t digitIndex, char character) {
    state.setDisplay(displayIndex, digitIndex, SevenSegmentDisplay().toByte(character));
}

void setDisplayChar(uint8_t displayIndexI, int number) {
    for (int i = 3; i >= 0; i--) {
        char digit = number % 10 + '0';

        setDisplayChar(displayIndexI, i, digit);
        number /= 10;
    }
}

void setDisplayChar(uint8_t displayIndex, char arr[4]) {
    for (int i = 3; i >= 0; i--) {
        setDisplayChar(displayIndex, i, arr[i]);
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

void readEncPos() {
    byte currentCLK = digitalRead(CLK_ENC);
    if (lastCLK == HIGH && currentCLK == LOW) {
        prevEncPos = state.getEncoderPosition();
        state.setEncMoveFlag(true);

        if (digitalRead(DT_ENC) != currentCLK) {
            state.setEncoderPosition(state.getEncoderPosition() + 1);
            state.setEncoderSteps(state.getEncoderSteps() + 1);
            updateServoAngle(true, false);
        } else {
            state.setEncoderPosition(state.getEncoderPosition() - 1);
            state.setEncoderSteps(state.getEncoderSteps() - 1);
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
        Serial.println(static_cast<int>(state.getMode()));
        isPressed = true;
    }

    lastSW = currentSW;
    return isPressed;
}

void rotateServo(byte angle) {
    if (angle >= 0 && angle <= 180) {
        servo.write(angle);
    }
}

void readEncoder() {
    readEncPos();
    readEncPress();
}

bool checkWrittenBounds(unsigned int written, size_t bufferSize, size_t pos) {
    return written < 0 || written >= bufferSize - pos;
}

uint8_t getEncMappedServoAngle(uint8_t encStep) { return map(encStep, 0, 20, 0, 180); }

bool findJsonValue(const char *json, const char *key, const char *&pos) {
    char keyPattern[20];

    snprintf(keyPattern, sizeof(keyPattern), "\"%s\"", key);

    pos = strstr(json, keyPattern);

    if (pos == nullptr) {
        return false;
    }

    pos = strchr(pos, ':');

    if (pos == nullptr) {
        return false;
    }

    pos++;

    while (*pos == ' ' || *pos == '\t') {
        pos++;
    }

    return true;
}

bool getJsonVal(const char *json, const char *key, bool &result) {
    const char *pos;

    if (!findJsonValue(json, key, pos)) {
        return false;
    }

    if (strncmp(pos, "true", 4) == 0) {
        result = true;
        return true;
    }

    if (strncmp(pos, "false", 5) == 0) {
        result = false;
        return true;
    }

    return false;
}

bool getJsonVal(const char *json, const char *key, int &result) {
    const char *pos;
    char *end;

    if (!findJsonValue(json, key, pos)) {
        return false;
    }

    long value = strtol(pos, &end, 10);

    if (end == pos || value < -32768 || value > 32767) {
        return false;
    }

    result = (int)value;

    return true;
}

bool getStateJson(char *buffer, size_t bufferSize) {
    size_t pos = 0;

    unsigned int written = snprintf(buffer + pos, bufferSize - pos, "{\"displays\":[");

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

    if (checkWrittenBounds(written, bufferSize, pos)) {
        return false;
    }

    pos += written;

    written =
        snprintf(buffer + pos, bufferSize - pos, "\"encoder\":%d}", state.getEncoderPosition());

    if (checkWrittenBounds(written, bufferSize, pos)) {
        return false;
    }

    return true;
}

bool setStateJson(char *buffer, size_t bufferSize) {
    if (buffer == nullptr || bufferSize == 0) {
        return false;
    }

    const char *type = strstr(buffer, "\"type\"");

    if (type == nullptr) {
        return false;
    }

    type = strchr(type, ':');

    if (type == nullptr) {
        return false;
    }

    type++;

    while (*type == ' ' || *type == '\t') {
        type++;
    }

    if (*type != '"') {
        return false;
    }

    type++;

    Type switchType = Type::UNKNOWN;
    if (strncmp(type, "encoder", 7) == 0 && type[7] == '"') {
        switchType = Type::ENCODER;
    } else if (strncmp(type, "led", 3) == 0 && type[3] == '"') {
        switchType = Type::LED;
    } else if (strncmp(type, "button", 6) == 0 && type[6] == '"') {
        switchType = Type::BUTTON;
    }

    Serial.println(static_cast<int>(switchType));

    switch (switchType) {
    case Type::ENCODER: {
        int value;
        int delta;

        if (!getJsonVal(buffer, "value", value)) {
            return false;
        }
        if (!getJsonVal(buffer, "delta", delta)) {
            return false;
        }
        state.setEncoderPosition(value);
        state.setEncoderSteps(state.getEncoderSteps() + delta);

        int newAngle = (int)servoAngle + delta;

        newAngle = constrain(newAngle, 0, 180);

        servoAngle = (uint8_t)newAngle;
        state.setEncMoveFlag(true);
        state.setServoAngle(servoAngle);
    } break;

    case Type::LED: {
        int ledIndex;
        bool ledValue;

        if (!getJsonVal(buffer, "index", ledIndex)) {
            return false;
        }

        if (!getJsonVal(buffer, "value", ledValue)) {
            return false;
        }

    } break;

    case Type::BUTTON:
        break;

    case Type::UNKNOWN:
        break;

    default:
        return false;
    }

    return true;
}

void initServer() {
    if (Ethernet.begin(mac) == 0) {
        Serial.println(F("DHCP failed! Manual encoder rotate only."));
    } else {
        Serial.println(F("DHCP initialized!"));
        isConnected = true;
    }

    Serial.println(Ethernet.localIP());

    server.begin();

    Serial.println(F("HTTP server started!"));

    if (udp.begin(UDP_PORT)) {
        Serial.println(F("UDP server started"));
        Serial.print(F("Listening on port: "));
        Serial.println(UDP_PORT);
    } else {
        Serial.println(F("UDP initialization failed"));
    }
}

void handleUdp() {
    int packetSize = udp.parsePacket();

    if (packetSize <= 0) {
        return;
    }

    char buffer[200];
    int len = udp.read(buffer, sizeof(buffer) - 1);

    if (len <= 0) {
        return;
    }

    buffer[len] = '\0';

    Serial.print(F("Message: "));
    Serial.println(buffer);

    setStateJson(buffer, sizeof(buffer));
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

    Wire.begin();

    rtc.begin();

    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    srand(rtc.now().secondstime());

    state.setServoRotateFlag(true);
    initServer();
}

void sendStateUDP() {
    char buffer[200];

    if (!getStateJson(buffer, sizeof(buffer))) {
        return;
    }

    udp.beginPacket(SERVER_IP, SERVER_PORT);
    udp.write((const uint8_t *)buffer, strlen(buffer));

    udp.endPacket();
    Serial.println(buffer);
}

void loop() {
    currentTime = millis();
    timeSec = currentTime / 1000;
    readEncPress();

    if (!isConnected && currentTime - lastTimeUpdate > RECONNECT_TIMEOUT) {
        lastReconnectTime = currentTime;
        initServer();
    }

    if (state.getEncMoveFlag()) {
        uint8_t angle = 0;
        if (state.getMode() == Mode::SERVO) {
            lk.displayIntNum(servoAngle, false, TMAlignTextRight);
            setDisplayChar(1, servoAngle);
            angle = servoAngle;
        }

        if (state.getMode() == Mode::SERVO_ENC) {
            angle = getEncMappedServoAngle(state.getEncoderSteps());
            lk.displayIntNum(angle, false, TMAlignTextRight);
            setDisplayChar(1, angle);
        }

        rotateServo(angle);
        sendStateUDP();
        state.setEncMoveFlag(false);
    }

    if (currentTime - lastTimeUpdate > 1000) {
        lastTimeUpdate = currentTime;
        updateCurrentTimeString(currentTimeString);
        state.setShowDisplaySemicolon(timeSec % 2);

        setDisplayChar(2, currentTimeString);

        display4dig.DisplayString(currentTimeString, state.getShowDisplaySemicolon() ? 0x40 : 0, 4,
                                  0);
    }

    if (currentTime - lastUpdateTime > UPD_TIME) {
        sendStateUDP();
        lastUpdateTime = currentTime;
    }

    handleUdp();
}