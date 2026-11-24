
#pragma once

#ifdef TARGET_UNO

namespace HardwareConfig {
constexpr uint8_t DIO_4DIG = A0;
constexpr uint8_t CLK_4DIG = A1;

constexpr uint8_t STB_LK = 10;
constexpr uint8_t CLK_LK = 9;
constexpr uint8_t DIO_LK = 8;

constexpr uint8_t SERVO_PIN = 5;

constexpr uint8_t SW_ENC = 4;
constexpr uint8_t DT_ENC = 3;
constexpr uint8_t CLK_ENC = 2;
} // namespace HardwareConfig

#elif defined(TARGET_ESP32)

namespace HardwareConfig {
constexpr uint8_t DIO_4DIG = 16;
constexpr uint8_t CLK_4DIG = 17;

constexpr uint8_t STB_LK = 21;
constexpr uint8_t CLK_LK = 22;
constexpr uint8_t DIO_LK = 23;

constexpr uint8_t SERVO_PIN = 25;

constexpr uint8_t SW_ENC = 27;
constexpr uint8_t DT_ENC = 32;
constexpr uint8_t CLK_ENC = 33;
} // namespace HardwareConfig

#else
#error "Unknown hardware target!"
#endif
