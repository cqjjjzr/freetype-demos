// charmap.hpp

// Copyright (C) 2022 by Charlie Jiang.

#pragma once

#include <QString>

#include <ft2build.h>
#include <freetype/freetype.h>

class Engine;

#define FT_ENCODING_OTHER 0xFFFE
struct CharMapInfo
{
  int index;
  FT_CharMap ptr;
  FT_Encoding encoding;
  unsigned short platformID;
  unsigned short encodingID;
  QString* encodingName;

  // Actually this shouldn't go here, but for convenience...
  int maxIndex;

  CharMapInfo(int index, FT_CharMap cmap);

  QString stringifyIndex(int code, int idx);
  QString stringifyIndexShort(int code);


  friend bool
  operator==(const CharMapInfo& lhs, const CharMapInfo& rhs)
  {
    return lhs.index == rhs.index && lhs.encoding == rhs.encoding;
  }


  friend bool
  operator!=(const CharMapInfo& lhs, const CharMapInfo& rhs)
  {
    return !(lhs == rhs);
  }


private:
  int computeMaxIndex();
  static int maxIndexForFaceAndCharMap(FT_CharMap charMap, unsigned max);
};


// end of charmap.hpp