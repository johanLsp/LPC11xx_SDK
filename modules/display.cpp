// Copyright 2019 Johan Lasperas
#include "display.hpp"

#include "core/i2c.h"

#include "gfxfont.h"
#include "segment.hpp"

// Declare arrays as const to flash them to memory
namespace Display {
const uint8_t address_ = 0x78;
const uint8_t kWidth = 128;
const uint8_t kPages = 4;
uint8_t buffer_[kPages][kWidth];
uint8_t command_[8];
uint8_t xcursor_;
uint8_t ycursor_;
}  // namespace Display

void Display::Init() {
  I2C::Init();
  xcursor_ = 0;
  ycursor_ = 0;

  Set(Command::DISPLAY_OFF);
  Set(Command::CLOCK_DIV, 0x80);
  Set(Command::MULTIPLEX_RATIO, 0x1F);
  Set(Command::OFFSET, 0x0);
  Set(Command::START_LINE, 0);
  Set(Command::CHARGE_PUMP, 0x14);  // Enable charge pump

  SetMemoryAddressingMode(Mode::HORIZONTAL);
  Set(Command::SEGMENT_REMAP);
  Set(Command::SCAN_REMAP);
  Set(Command::COM_PINS, 0x02);
  Set(Command::CONTRAST, 0x8F);
  Set(Command::PRECHARGE, 0xF1);
  Set(Command::COM_DETECT, 0x40);
  Set(Command::DISPLAY_RESUME);
  Set(Command::DISPLAY_MODE_NORMAL);

  SetStartLine(0);
  SetMemoryAddressingMode(Mode::HORIZONTAL);

  Clear();
  Enable(true);
  Update();
}

void Display::SetStartLine(uint8_t line) {
  uint8_t command = 0x40 | line;
  I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
}

void Display::SetMemoryAddressingMode(Mode mode) {
  uint8_t command = 0x20;
  I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
  switch (mode) {
    case Mode::HORIZONTAL:
      command = 0x00;
      break;
    case Mode::VERTICAL:
      command = 0x01;
      break;
    case Mode::PAGE:
      command = 0x02;
      break;
  }
  I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
}

void Display::SetColumnAddress(uint8_t start, uint8_t end) {
  uint8_t command = 0x21;
  I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
  I2C::Write(address_, Control::COMMAND_STREAM, &start, 1);
  I2C::Write(address_, Control::COMMAND_STREAM, &end, 1);
}

void Display::SetPageAddress(uint8_t start, uint8_t end) {
  uint8_t command = 0x22;
  I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
  I2C::Write(address_, Control::COMMAND_STREAM, &start, 1);
  I2C::Write(address_, Control::COMMAND_STREAM, &end, 1);
}

void Display::DrawRectangle(Point p1, Point p2) {
  SetColumnAddress(p1.x, p2.x);
  SetPageAddress(p1.y, p2.y);
  int length = (p2.x - p1.x + 1) * (p2.y - p1.y + 1);
  for (int i = 0; i < length; i++) {
      command_[i+1] = 0xFF;
  }
  I2C::Write(address_, Control::DATA_STREAM, command_, length);
}


void Display::Clear() {
  for (int i = 0; i < kWidth; i++) {
    for (int j = 0; j < kPages; j++) {
      buffer_[j][i] = 0x00;
    }
  }
}

void Display::DrawGFXText(const char* c, uint8_t length, uint8_t size) {
  for (int i = 0; i < length; i++) {
    DrawGFXChar(c[i], size);
  }
}

void Display::DrawGFXChar(unsigned char c, uint8_t size) {
  // Character is assumed previously filtered by write() to eliminate
  // newlines, returns, non-printable characters, etc.  Calling
  // drawChar() directly with 'bad' characters of font may cause mayhem!

  // Map digits to chars
  if (c < 10) {
    c += '0';
  } else if (c < 16) {
    c += 'A' - 10;
  }

  const GFXfont& gfxFont = FreeMono12pt7b;
  c -= gfxFont.first;
  const GFXglyph& glyph = gfxFont.glyph[c];
  const uint8_t* bitmap;
  uint8_t bo = glyph.bitmapOffset;

  if (c < '5' - gfxFont.first) {
    bitmap = gfxFont.bitmap1;
  } else if (c < 'E' - gfxFont.first) {
    bitmap = gfxFont.bitmap2;
    bo -= gfxFont.glyph['5' - gfxFont.first].bitmapOffset;
  } else if (c < 'Q' - gfxFont.first) {
    bitmap = gfxFont.bitmap3;
    bo -= gfxFont.glyph['E' - gfxFont.first].bitmapOffset;
  } else if (c < '_' - gfxFont.first) {
    bitmap = gfxFont.bitmap4;
    bo -= gfxFont.glyph['Q' - gfxFont.first].bitmapOffset;
  } else if (c < 'o' - gfxFont.first) {
    bitmap = gfxFont.bitmap5;
    bo -= gfxFont.glyph['_' - gfxFont.first].bitmapOffset;
  } else {
    bitmap = gfxFont.bitmap6;
    bo -= gfxFont.glyph['o' - gfxFont.first].bitmapOffset;
  }

  uint8_t w  = glyph.width;
  uint8_t h  = glyph.height;
  int8_t xo = glyph.xOffset;
  int8_t yo = glyph.yOffset;
  uint8_t bits = 0;
  uint8_t bit = 0;
  int16_t xo16 = 0;
  uint8_t yo16 = 0;

  if (size > 1) {
      xo16 = xo;
      yo16 = yo;
  }

  for (uint8_t yy = 0; yy < h; yy++) {
    for (uint8_t xx = 0; xx < w; xx++) {
      if (!(bit++ & 7)) {
        bits = bitmap[bo++];
      }
      if (bits & 0x80) {
        if (size == 1) {
          DrawPixel({xcursor_+xo+xx, ycursor_+yo+yy});
        } else {
          FillRect({xcursor_+(xo16+xx)*size, ycursor_+(yo16+yy)*size},
                   size, size);
        }
      }
      bits <<= 1;
    }
  }
  xcursor_ += glyph.xAdvance * size;
}

void Display::DrawPixel(Point p) {
  buffer_[p.y/8][p.x] |= 0x1 << (p.y % 8);
}

void Display::FillRect(Point p1, uint16_t dx, uint16_t dy) {
  for (int x = p1.x; x <= p1.x + dx; x++) {
    for (int y = p1.y; y <= p1.y + dy; y++) {
      DrawPixel({x, y});
    }
  }
}

void Display::Update() {
  SetColumnAddress(0, 127);
  SetPageAddress(0, 3);
  I2C::Write(address_, Control::DATA_STREAM, buffer_[0], kWidth * kPages);
}

void Display::DisplayOn(bool keep_ram) {
  uint8_t command = keep_ram ? 0xA5 : 0xA4;
  I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
}

void Display::Enable(bool enable) {
  uint8_t command = enable ? 0xAF : 0xAE;
  I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
}

void Display::Set(const uint8_t& command) {
  I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
}

void Display::Set(const uint8_t& command, const uint8_t& arg) {
  // Following command embed the argument in the command byte
  if (command == Command::START_LINE) {
    uint8_t data = command | arg;
    I2C::Write(address_, Control::COMMAND_STREAM, &data, 1);
  } else {
    I2C::Write(address_, Control::COMMAND_STREAM, &command, 1);
    I2C::Write(address_, Control::COMMAND_STREAM, &arg, 1);
  }
}

void Display::SetCursor(uint8_t x, uint8_t y) {
  xcursor_ = x;
  ycursor_ = y;
}

void Display::DrawValue(const char* name, uint8_t name_len, uint32_t value,
                        uint32_t max_value) {
  Clear();
  SetCursor(0, 24);
  DrawGFXText(name, name_len, 1);
  if (max_value == 0) {
    char msg[6] = {
      ':', ' ',
      value / 1000,
      value / 100 % 10,
      value / 10 % 10,
      value % 10
    };
    DrawGFXText(msg, 6, 1);
  } else {
    value = value * 100 / max_value;
    char msg[5] = {
      ':', ' ',
      value / 10 % 10,
      value % 10,
      '%'
    };
    DrawGFXText(msg, 5, 1);
  }
  Update();
}
