// engine.hpp

// Copyright (C) 2016-2022 by Werner Lemberg.


#pragma once

#include "fontfilemanager.hpp"
#include "charmap.hpp"
#include "paletteinfo.hpp"
#include "fontinfo.hpp"
#include "mmgx.hpp"

#include <vector>
#include <utility>
#include <QString>
#include <QMap>
#include <QRect>
#include <QImage>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>
#include <freetype/ftcache.h>
#include <freetype/ftlcdfil.h>
#include <freetype/ftcolor.h>
#include <freetype/t1tables.h>


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

// Some helper functions.

QString* glyphFormatToName(FT_Glyph_Format format);

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
  FT_Glyph loadGlyph(int glyphIndex);
  int loadGlyphIntoSlotWithoutCache(int glyphIndex, bool noScale = false);

  // Sometimes the engine is already updated, and we want to be faster
  FT_Glyph loadGlyphWithoutUpdate(int glyphIndex,
                                  FTC_Node* outNode = NULL,
                                  bool forceRender = false);

  // Return `true` if you need to free `out`
  // `out` will be set to NULL in cases of error
  bool convertGlyphToBitmapGlyph(FT_Glyph src, FT_Glyph* out);
  FT_Bitmap convertBitmapTo8Bpp(FT_Bitmap* bitmap);
  QImage* convertBitmapToQImage(FT_Bitmap* src);
  QImage* convertGlyphToQImage(FT_Glyph src, 
                               QRect* outRect,
                               bool inverseRectY);
  QPoint computeGlyphOffset(FT_Glyph glyph, bool inverseY);

  /*
   * Directly render the glyph at the specified index
   * to a `QImage`. If you want to perform color-layer
   * rendering, call this before trying to load the
   * glyph and do normal rendering, If the returning
   * value is non-NULL, then there's no need to
   * load the glyph the normal way, just draw the `QImage`.
   * Will return NULL if not enabled or color layers not available.
   */
  QImage* tryDirectRenderColorLayers(int glyphIndex,
                                     QRect* outRect,
                                     bool inverseRectY = false);

  // reload current triplet, but with updated settings, useful for updating
  // `ftSize_` only
  void reloadFont();
  void loadPalette();

  void openFonts(QStringList fontFileNames);
  void removeFont(int fontIndex, bool closeFile = true);
  
  void update();

  //////// Getters

  FT_Library ftLibrary() const { return library_; }
  FTC_Manager cacheManager() { return cacheManager_; }

  int dpi() { return dpi_; }
  double pointSize() { return pointSize_; }

  int numberOfOpenedFonts();
  int currentFontIndex() { return curFontIndex_; }
  FT_Size currentFtSize() { return ftSize_; }
  int currentFontType() const { return fontType_; }
  const QString& currentFamilyName() { return curFamilyName_; }
  const QString& currentStyleName() { return curStyleName_; }
  int currentFontNumberOfGlyphs() { return curNumGlyphs_; }

  QString glyphName(int glyphIndex);
  long numberOfFaces(int fontIndex);
  int numberOfNamedInstances(int fontIndex,
                             long faceIndex);
  QString namedInstanceName(int fontIndex, long faceIndex, int index);

  int currentFontFirstUnicodeCharMap();
  // Note: the current font face must be properly set
  unsigned glyphIndexFromCharCode(int code, int charMapIndex);
  FT_Size_Metrics const& currentFontMetrics();
  FT_GlyphSlot currentFaceSlot();
  FT_Pos currentFontTrackingKerning(int degree);
  FT_Vector currentFontKerning(int glyphIndex, int prevIndex);
  std::pair<int, int> currentSizeAscDescPx();

  bool currentFontPSInfo(PS_FontInfoRec& outInfo);
  bool currentFontPSPrivateInfo(PS_PrivateRec& outInfo);
  std::vector<CharMapInfo>& currentFontCharMaps() { return curCharMaps_; }
  std::vector<PaletteInfo>& currentFontPalettes() { return curPaletteInfos_; }
  std::vector<SFNTName>& currentFontSFNTNames() { return curSFNTNames_; }
  MMGXState currentFontMMGXState() { return curMMGXState_; }
  std::vector<MMGXAxisInfo>& currentFontMMGXAxes() { return curMMGXAxes_; }
  std::vector<SFNTTableInfo>& currentFontSFNTTableInfo();
  FontFileManager& fontFileManager() { return fontFileManager_; }
  EngineDefaultValues& engineDefaults() { return engineDefaults_; }
  bool antiAliasingEnabled() { return antiAliasingEnabled_; }
  bool doHinting() { return doHinting_; }
  bool embeddedBitmapEnabled() { return embeddedBitmap_; }
  bool lcdUsingSubPixelPositioning() { return lcdSubPixelPositioning_; }

  QRgb foreground() { return foregroundColor_; }
  QRgb background() { return backgroundColor_; }

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
  void setGamma(double gamma);
  void setAntiAliasingTarget(int target) { antiAliasingTarget_ = target; }
  void setRenderMode(int mode) { renderMode_ = mode; }
  void setAntiAliasingEnabled(bool enabled) { antiAliasingEnabled_ = enabled; }
  void setEmbeddedBitmap(bool force) { embeddedBitmap_ = force; }
  void setUseColorLayer(bool colorLayer) { useColorLayer_ = colorLayer; }
  void setPaletteIndex(int index) { paletteIndex_ = index; }
  void setLCDUsesBGR(bool isBGR) { lcdUsesBGR_ = isBGR; }
  void setLCDSubPixelPositioning(bool sp) { lcdSubPixelPositioning_ = sp; }

  // Note: These 3 functions now takes actual mode/version from FreeType,
  // instead of values from enum in MainGUI!
  void setLcdFilter(FT_LcdFilter filter);
  void setCFFHintingMode(int mode);
  void setTTInterpreterVersion(int version);
  void setStemDarkening(bool darkening);

  void setForeground(QRgb foreground);
  void setBackground(QRgb background);

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

  int curFontIndex_ = -1;
  QString curFamilyName_;
  QString curStyleName_;
  int curNumGlyphs_ = -1;
  std::vector<SFNTName> curSFNTNames_;
  std::vector<CharMapInfo> curCharMaps_;
  std::vector<PaletteInfo> curPaletteInfos_;
  std::vector<MMGXAxisInfo> curMMGXAxes_;

  bool curSFNTTablesValid_ = false;
  std::vector<SFNTTableInfo> curSFNTTables_;
  MMGXState curMMGXState_ = MMGXState::NoMMGX;

  FT_Library library_;
  FTC_Manager cacheManager_;
  FTC_ImageCache imageCache_;
  FTC_SBitCache sbitsCache_;
  FTC_CMapCache cmapCache_;

  FTC_ScalerRec scaler_ = {};
  FTC_ImageTypeRec imageType_;
  FT_Size ftSize_;
  FT_Palette_Data paletteData_ = {};
  FT_Color* palette_ = NULL;

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
  bool embeddedBitmap_;
  bool useColorLayer_;
  int paletteIndex_ = -1;
  int antiAliasingTarget_;
  bool lcdUsesBGR_;
  bool lcdSubPixelPositioning_;
  int renderMode_;

  double gamma_;
  unsigned long loadFlags_;

  QRgb backgroundColor_;
  QRgb foregroundColor_;
  QRgb foregroundColorBlended_;
  QVector<QRgb> foregroundTable_;

  void queryEngine();
  void loadPaletteInfos();
  void calculateForegroundTable();

  // Safe to put the impl to the cpp.
  template <class Func>
  void withFace(FaceID id, Func func);

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
