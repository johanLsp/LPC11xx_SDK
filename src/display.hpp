// Copyright 2019 Johan Lasperas
#ifndef SRC_DISPLAY_HPP_
#define SRC_DISPLAY_HPP_

#include "core/LPC11xx.h"

namespace Display {

enum class Mode {HORIZONTAL, VERTICAL, PAGE};

namespace Control {
  const uint8_t COMMAND_SINGLE = 0x80;
  const uint8_t COMMAND_STREAM = 0x00;
  const uint8_t DATA_SINGLE = 0xC0;
  const uint8_t DATA_STREAM = 0x40;
}

namespace Command {
  const uint8_t START_LINE = 0x40;
  const uint8_t CONTRAST = 0x81;
  const uint8_t CHARGE_PUMP = 0x8D;
  const uint8_t SEGMENT_NORMAL = 0xA0;
  const uint8_t SEGMENT_REMAP = 0xA1;
  const uint8_t DISPLAY_RESUME = 0xA4;
  const uint8_t DISPLAY_CLEAR = 0xA5;
  const uint8_t DISPLAY_MODE_NORMAL = 0xA6;
  const uint8_t DISPLAY_MODE_INVERSE = 0xA7;
  const uint8_t MULTIPLEX_RATIO = 0xA8;
  const uint8_t DISPLAY_OFF = 0xAE;
  const uint8_t DISPLAY_ON = 0xAF;
  const uint8_t SCAN_NORMAL = 0xC0;
  const uint8_t SCAN_REMAP = 0xC8;
  const uint8_t OFFSET = 0xD3;
  const uint8_t CLOCK_DIV = 0xD5;
  const uint8_t PRECHARGE = 0xD9;
  const uint8_t COM_PINS = 0xDA;
  const uint8_t COM_DETECT = 0xDB;
}  // namespace Command

struct Point {
  uint16_t x;
  uint16_t y;
};

void Init();
void Set(const uint8_t& command);
void Set(const uint8_t& command, const uint8_t& arg);
void SetStartLine(uint8_t line);
void SetMemoryAddressingMode(Mode mode);
void SetColumnAddress(uint8_t start, uint8_t end);
void SetPageAddress(uint8_t start, uint8_t end);
void DrawRectangle(Point p1, Point p2);
void DisplayOn(bool keep_ram = false);
void Enable(bool enable);
void DrawChar(char c, uint8_t x, uint8_t y);
void DrawText(const char* c, uint8_t length, uint8_t x, uint8_t y);
void DrawPixel(Point p);
void FillRect(Point p1, uint16_t dx, uint16_t dy);
void Update();
void DrawGFXChar(unsigned char c, uint8_t size);
void DrawGFXText(const char* c, uint8_t length, uint8_t size);
void SetCursor(uint8_t x, uint8_t y);
void DrawValue(const char* name, uint8_t name_len, uint32_t value,
               uint32_t max_value = 0);

void Clear();
}  // namespace Display

#endif  // SRC_DISPLAY_HPP_
