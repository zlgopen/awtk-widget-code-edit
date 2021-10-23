// Scintilla source code edit control
// Converter.h - Encapsulation of iconv
// Copyright 2004 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef CONVERTER_H
#define CONVERTER_H

namespace Scintilla {

const uint32_t sizeFailure = static_cast<uint32_t>(-1);

/**
 * Encapsulate g_iconv safely.
 */
class Converter {
  void OpenHandle(const char* fullDestination, const char* charSetSource) noexcept {
  }
  bool Succeeded() const noexcept {
    return true;
  }

 public:
  Converter() noexcept {
  }
  Converter(const char* charSetDestination, const char* charSetSource, bool transliterations) {
  }
  // Deleted so Converter objects can not be copied.
  Converter(const Converter&) = delete;
  Converter(Converter&&) = delete;
  Converter& operator=(const Converter&) = delete;
  Converter& operator=(Converter&&) = delete;
  ~Converter() {
  }
  operator bool() const noexcept {
    return true;
  }
  void Open(const char* charSetDestination, const char* charSetSource, bool transliterations) {
  }
  void Close() noexcept {
  }
  uint32_t Convert(char** src, uint32_t* srcleft, char** dst, uint32_t* dstleft) const noexcept {
    return 0;
  }
};

}  // namespace Scintilla

#endif
