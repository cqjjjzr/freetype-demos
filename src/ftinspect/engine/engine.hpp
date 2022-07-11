// engine.hpp

// Copyright (C) 2016-2022 by Werner Lemberg.


#pragma once

#include "fontfilemanager.hpp"

#include <vector>
#include <QString>
#include <QMap>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>
#include <freetype/ftcache.h>
#include <freetype/ftlcdfil.h>


// This structure maps the (font, face, instance) index triplet to abstract
// IDs (generated by a running number stored in MainGUI's `faceCounter'
// member).
//
// Qt's `QMap' class needs an implementation of the `<' operator.

struct FaceID
{
  int fontIndex;
  long faceIndex;
  int namedInstanceIndex;

  FaceID();
  FaceID(int fontIndex,
         long faceIndex,
         int namedInstanceIndex);
  bool operator<(const FaceID& other) const;
};

class Engine;

#define FT_ENCODING_OTHER 0xFFFE
struct CharMapInfo
{
  int index;
  FT_CharMap ptr;
  FT_Encoding encoding;
  QString* encodingName;

  // Actually this shouldn't go here, but for convenience...
  int maxIndex;

  CharMapInfo(int index, FT_CharMap cmap);

  QString stringifyIndex(int code, int index);
  QString stringifyIndexShort(int code);

private:
  int computeMaxIndex();
  static int maxIndexForFaceAndCharMap(FT_CharMap charMap, unsigned max);
};

// FreeType specific data.

class Engine
{
public:
  //////// Nested definitions (forward decl)
  enum FontType : int;

  struct EngineDefaultValues
  {
    int cffHintingEngineDefault;
    int cffHintingEngineOther;

    int ttInterpreterVersionDefault;
    int ttInterpreterVersionOther;
    int ttInterpreterVersionOther1;
  };

  //////// Ctors & Dtors

  Engine();
  ~Engine();

  // Disable copying
  Engine(const Engine& other) = delete;
  Engine& operator=(const Engine& other) = delete;

  //////// Actions

  int loadFont(int fontIndex,
               long faceIndex,
               int namedInstanceIndex); // return number of glyphs
  FT_Outline* loadOutline(int glyphIndex);

  // Sometimes the engine is already updated, and we want to be faster
  FT_Glyph loadGlyphWithoutUpdate(int glyphIndex);

  // reload current triplet, but with updated settings, useful for updating
  // `ftSize_` only
  void reloadFont(); 

  void openFonts(QStringList fontFileNames);
  void removeFont(int fontIndex, bool closeFile = true);
  
  void update();

  //////// Getters

  FT_Library ftLibrary() const { return library_; }
  int currentFontType() const { return fontType_; }
  const QString& currentFamilyName() { return curFamilyName_; }
  const QString& currentStyleName() { return curStyleName_; }
  int currentFontNumberOfGlyphs() { return curNumGlyphs_; }
  int numberOfOpenedFonts();
  QString glyphName(int glyphIndex);
  long numberOfFaces(int fontIndex);
  int numberOfNamedInstances(int fontIndex,
                             long faceIndex);
  // Note: the current font face must be properly set
  unsigned glyphIndexFromCharCode(int code, int charMapIndex);
  FT_Size_Metrics const& currentFontMetrics();

  std::vector<CharMapInfo>& currentFontCharMaps() { return curCharMaps_; }
  FontFileManager& fontFileManager() { return fontFileManager_; }
  EngineDefaultValues& engineDefaults() { return engineDefaults_; }
  bool antiAliasingEnabled() { return antiAliasingEnabled_; }


  //////// Setters (direct or indirect)

  void setDPI(int d) { dpi_ = d; }
  void setSizeByPixel(double pixelSize);
  void setSizeByPoint(double pointSize);
  void setHinting(bool hinting) { doHinting_ = hinting; }
  void setAutoHinting(bool autoHinting) { doAutoHinting_ = autoHinting; }
  void setHorizontalHinting(bool horHinting)
  {
    doHorizontalHinting_ = horHinting;
  }
  void setVerticalHinting(bool verticalHinting)
  {
    doVerticalHinting_ = verticalHinting;
  }
  void setBlueZoneHinting(bool blueZoneHinting)
  {
    doBlueZoneHinting_ = blueZoneHinting;
  }
  void setShowSegments(bool showSegments) { showSegments_ = showSegments; }
  void setGamma(double gamma) { gamma_ = gamma; }
  void setAntiAliasingTarget(int target) { antiAliasingTarget_ = target; }
  void setAntiAliasingEnabled(bool enabled) { antiAliasingEnabled_ = enabled; }

  // Note: These 3 functions now takes actual mode/version from FreeType,
  // instead of values from enum in MainGUI!
  void setLcdFilter(FT_LcdFilter filter);
  void setCFFHintingMode(int mode);
  void setTTInterpreterVersion(int version);

  //////// Misc

  friend FT_Error faceRequester(FTC_FaceID,
                                FT_Library,
                                FT_Pointer,
                                FT_Face*);

private:
  using FTC_IDType = uintptr_t;
  FTC_IDType faceCounter_; // a running number used to initialize `faceIDMap'
  QMap<FaceID, FTC_IDType> faceIDMap_;

  FontFileManager fontFileManager_;

  QString curFamilyName_;
  QString curStyleName_;
  int curNumGlyphs_ = -1;
  std::vector<CharMapInfo> curCharMaps_;

  FT_Library library_;
  FTC_Manager cacheManager_;
  FTC_ImageCache imageCache_;
  FTC_SBitCache sbitsCache_;
  FTC_CMapCache cmapCache_;

  FTC_ScalerRec scaler_;
  FT_Size ftSize_;
  FTC_ImageTypeRec imageType_;

  EngineDefaultValues engineDefaults_;

  int fontType_;

  bool antiAliasingEnabled_ = true;
  bool usingPixelSize_ = false;
  double pointSize_;
  double pixelSize_;
  unsigned int dpi_;

  bool doHinting_;
  bool doAutoHinting_;
  bool doHorizontalHinting_;
  bool doVerticalHinting_;
  bool doBlueZoneHinting_;
  bool showSegments_;
  int antiAliasingTarget_;

  double gamma_;

  unsigned long loadFlags_;

  void queryEngine();

public:

  /// Actual definition

  // XXX cover all available modules
  enum FontType : int
  {
    FontType_CFF,
    FontType_TrueType,
    FontType_Other
  };
};


// end of engine.hpp
