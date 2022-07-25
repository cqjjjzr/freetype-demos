// stringrenderer.hpp

// Copyright (C) 2022 by Charlie Jiang.

#pragma once

#include <vector>
#include <functional>

#include <QString>

#include <ft2build.h>
#include <qslider.h>
#include <freetype/freetype.h>
#include <freetype/ftcache.h>
#include <freetype/ftglyph.h>

// adopted from `ftcommon.h`

class Engine;
struct GlyphContext
{
  int charCode = 0;
  int glyphIndex = 0;
  FT_Glyph glyph = NULL;
  FTC_Node cacheNode = NULL;

  FT_Pos lsbDelta = 0;    // delta caused by hinting
  FT_Pos rsbDelta = 0;   // delta caused by hinting
  FT_Vector hadvance = { 0, 0 }; // kerned horizontal advance

  FT_Vector vvector = { 0, 0 };  // vert. origin => hori. origin
  FT_Vector vadvance = { 0, 0 }; // vertical advance
};

// Class to populate chars to render, to load and properly position glyphs.
// Use callbacks to receive characters.
class StringRenderer
{
public:
  StringRenderer(Engine* engine);
  ~StringRenderer();

  enum KerningDegree
  {
    KD_None = 0,
    KD_Light,
    KD_Medium,
    KD_Tight
  };

  enum KerningMode
  {
    KM_None = 0,
    KM_Normal,
    KM_Smart
  };

  /*
   * Need to pass the pen position because sometimes the outline vector
   * contains no points, and thus can't be translated to the desired pen
   * position.
   */
  using RenderCallback = std::function<void(FT_Glyph, FT_Vector)>;
  /*
   * The glyph pointer may be replaced. In that case, ownership is transfered
   * to the renderer, and the new glyph will be eventually freed by
   * the renderer. The callback is responsible to free the old glyph.
   * This allows you to do the following:
   * void callback(FT_Glyph* ptr) {
   *     ....
   *     auto oldPtr = *ptr;
   *     *ptr = ....;
   *     FT_Done_Glyph(olPtr);
   * }
   */
  using PreprocessCallback = std::function<void(FT_Glyph*)>;
  /*
   * Called when a new line begins.
   * The 1st parameter is the initial pen position;
   * The 2nd parameter is the current size in points.
   */
  using LineBeginCallback = std::function<void(FT_Vector, double)>;

  bool isWaterfall() { return waterfall_; }

  void
  setCallback(RenderCallback cb)
  {
    renderCallback_ = std::move(cb);
  }
  void
  setPreprocessCallback(PreprocessCallback cb)
  {
    glyphPreprocessCallback_ = std::move(cb);
  }
  void
  setLineBeginCallback(LineBeginCallback cb)
  {
    lineBeginCallback_ = std::move(cb);
  }

  void setCharMapIndex(int charMapIndex, int limitIndex);
  void setRepeated(bool repeated) { repeated_ = repeated; }
  void setVertical(bool vertical) { vertical_ = vertical; }
  void setRotation(double rotation);
  void setWaterfall(bool waterfall) { waterfall_ = waterfall; }
  void setPosition(double pos) { position_ = pos; }
  void setKerning(bool kerning);

  // Need to be called when font or charMap changes
  void setUseString(QString const& string);
  void setUseAllGlyphs();
  
  int render(int width,
             int height,
             int offset);
  int renderLine(int x, int y,
                 int width, int height,
                 int offset,
                 bool handleMultiLine = false);

  void reloadAll();      // text/font/charmap changes, will call
                         // `reloadGlyphs`
  void reloadGlyphs();   // any other parameter changes

private:
  Engine* engine_;

  // Generally, rendering has those steps:
  // 1. If in string mode, the string is load into `activeGlyphs_`
  //    (in `updateString`)
  // 2. The char codes in contexts are converted to glyph indices
  //    (in `reloadGlyphIndices`)
  // 3. If in string mode, glyphs are loaded into contexts.
  //    (in `loadStringGlyphs`)
  // 4. In `render` function, according to mode, `renderLine` is called line
  //    by line (as well as `prepareRendering`).
  // 5. In `renderLine`, if in all glyphs mode, glyphs from the begin index
  //    are loaded until the line is full (if the glyph already exists, it will
  //    be reused). If in string mode, it will directly use the prepared glyphs.
  //    Preprocessing is done within this step, such as emboldening or stroking.
  //    Eventually the `FT_Glyph` pointer is passed to the callback.
  
  GlyphContext tempGlyphContext_;
  // This vector stores all active glyphs for rendering. When rendering strings,
  // this is the container for chars, so DO NOT directly clear it to flush
  // cache, you should clean glyph objects only. However when rendering all
  // glyphs, it's generally to directly wipe the vector because it's dynamically
  // generated in `render` function (see above).
  //
  // Note: Because of kerning, this list must be ordered and allow duplicate
  //       characters.
  //
  // Actually this means 3 parts of storage: string charcode, glyph indices and
  // glyph (+ all related info). Different parameter changes will trigger
  // different levels of flushing.
  std::vector<GlyphContext> activeGlyphs_;
  bool glyphCacheValid_ = false;

  int charMapIndex_ = 0;
  int limitIndex_ = 0;
  bool usingString_ = false;
  bool waterfall_ = false;
  bool repeated_ = false;
  bool vertical_ = false;
  double position_ = 0;
  double rotation_ = 0;
  int kerningDegree_ = KD_None;
  KerningMode kerningMode_ = KM_None;
  FT_Pos trackingKerning_ = 0;
  FT_Matrix matrix_ = {};
  bool matrixEnabled_ = false;

  RenderCallback renderCallback_;
  PreprocessCallback glyphPreprocessCallback_;
  LineBeginCallback lineBeginCallback_;

  void reloadGlyphIndices();
  void prepareRendering();
  void loadSingleContext(GlyphContext* ctx, GlyphContext* prev);
  // Need to be called when font, charMap or size changes;
  void loadStringGlyphs();
  // Returns total line count
  int prepareLine(int offset, 
                  int lineWidth,
                  FT_Vector& outActualLineWidth,
                  bool handleMultiLine = false);
  void clearActive(bool glyphOnly = false);
};