// stringrenderer.cpp

// Copyright (C) 2022 by Charlie Jiang.

#include "stringrenderer.hpp"

#include "engine.hpp"

#include <cmath>


StringRenderer::StringRenderer(Engine* engine)
: engine_(engine)
{
}


StringRenderer::~StringRenderer()
{
  clearActive();
}


void
StringRenderer::setCharMapIndex(int charMapIndex,
                                int limitIndex)
{
  auto& charmaps = engine_->currentFontCharMaps();
  if (charMapIndex < 0
      || static_cast<uint>(charMapIndex) >= charmaps.size())
    charMapIndex = -1;

  charMapIndex_ = charMapIndex;
  limitIndex_ = limitIndex;
}


void
StringRenderer::setRotation(double rotation)
{
  rotation_ = rotation;

  if (rotation <= -180)
    rotation += 360;
  if (rotation > 180)
    rotation -= 360;

  if (rotation == 0)
  {
    matrixEnabled_ = false;
    return;
  }

  matrixEnabled_ = true;
  double radian = rotation * 3.14159265 / 180.0;
  auto cosinus = static_cast<FT_Fixed>(cos(radian) * 65536.0);
  auto sinus = static_cast<FT_Fixed>(sin(radian) * 65536.0);

  matrix_.xx = cosinus;
  matrix_.yx = sinus;
  matrix_.xy = -sinus;
  matrix_.yy = cosinus;
}


void
StringRenderer::setKerning(bool kerning)
{
  if (kerning)
  {
    kerningMode_ = KM_Normal;
    kerningDegree_ = KD_Medium;
  }
  else
  {
    kerningMode_ = KM_None;
    kerningDegree_ = KD_None;
  }
}


void
StringRenderer::reloadAll()
{
  clearActive(usingString_); // if "All Glyphs", then do a complete wipe
  if (usingString_)
    reloadGlyphIndices();
}

void
StringRenderer::reloadGlyphs()
{
  clearActive(true);
}


void
StringRenderer::setUseString(QString const& string)
{
  clearActive(); // clear existing
  usingString_ = true;

  long long totalCount = 0;
  for (uint ch : string.toUcs4())
  {
    activeGlyphs_.emplace_back();
    auto& it = activeGlyphs_.back();
    it.charCode = static_cast<int>(ch);
    it.glyphIndex = 0;
    ++totalCount;
    if (totalCount >= INT_MAX)
      break;
  }
  reloadGlyphIndices();
}


void
StringRenderer::setUseAllGlyphs()
{
  if (usingString_)
    clearActive();
  usingString_ = false;
}


void
StringRenderer::reloadGlyphIndices()
{
  if (!usingString_)
    return;
  int charMapIndex = charMapIndex_;
  auto& charmaps = engine_->currentFontCharMaps();
  if (charMapIndex < 0
      || static_cast<uint>(charMapIndex) >= charmaps.size()
      || charmaps[charMapIndex].encoding != FT_ENCODING_UNICODE)
    charMapIndex = engine_->currentFontFirstUnicodeCharMap();

  if (charMapIndex < 0)
    return;
  for (auto& ctx : activeGlyphs_)
  {
    auto index = engine_->glyphIndexFromCharCode(ctx.charCode, charMapIndex);
    ctx.glyphIndex = static_cast<int>(index);
  }
}


void
StringRenderer::prepareRendering()
{
  engine_->reloadFont();
  if (kerningDegree_ != KD_None)
    trackingKerning_ = engine_->currentFontTrackingKerning(kerningDegree_);
  else
    trackingKerning_ = 0;
}


void
StringRenderer::loadSingleContext(GlyphContext* ctx,
                                  GlyphContext* prev)
{
  if (ctx->cacheNode)
  {
    FTC_Node_Unref(ctx->cacheNode, engine_->cacheManager());
    ctx->cacheNode = NULL;
  }
  else if (ctx->glyph)
    FT_Done_Glyph(ctx->glyph); // when caching isn't used

  // TODO use FTC?

  // After `prepareRendering`, current size/face is properly set
  FT_GlyphSlot slot = engine_->currentFaceSlot();
  if (engine_->loadGlyphIntoSlotWithoutCache(ctx->glyphIndex) != 0)
  {
    ctx->glyph = NULL;
    return;
  }
  if (FT_Get_Glyph(slot, &ctx->glyph) != 0)
  {
    ctx->glyph = NULL;
    return;
  }
  auto& metrics = slot->metrics;
  //ctx->glyph = engine_->loadGlyphWithoutUpdate(ctx->glyphIndex, 
  //                                            &ctx->cacheNode);

  if (!ctx->glyph)
    return;

  ctx->vvector.x = metrics.vertBearingX - metrics.horiBearingX;
  ctx->vvector.y = -metrics.vertBearingY - metrics.horiBearingY;

  ctx->vadvance.x = 0;
  ctx->vadvance.y = -metrics.vertAdvance;

  ctx->lsbDelta = slot->lsb_delta;
  ctx->rsbDelta = slot->rsb_delta;

  ctx->hadvance.x = metrics.horiAdvance;
  ctx->hadvance.y = 0;

  if (engine_->lcdUsingSubPixelPositioning())
    ctx->hadvance.x += ctx->lsbDelta - ctx->rsbDelta;
  prev->hadvance.x += trackingKerning_;

  if (kerningMode_ != KM_None)
  {
    FT_Vector kern = engine_->currentFontKerning(ctx->glyphIndex, 
                                        prev->glyphIndex);

    prev->hadvance.x += kern.x;
    prev->hadvance.y += kern.y;

    if (!engine_->lcdUsingSubPixelPositioning() && kerningMode_ > KM_Normal)
    {
      if (prev->rsbDelta - ctx->lsbDelta > 32)
        prev->hadvance.x -= 64;
      else if (prev->rsbDelta - ctx->lsbDelta < -31)
        prev->hadvance.x += 64;
    }
  }

  if (!engine_->lcdUsingSubPixelPositioning() && engine_->doHinting())
  {
    prev->hadvance.x = (prev->hadvance.x + 32) & -64;
    prev->hadvance.y = (prev->hadvance.y + 32) & -64;
  }
}


void
StringRenderer::loadStringGlyphs()
{
  if (!usingString_)
    return;
  GlyphContext* prev = &tempGlyphContext_; // = empty
  tempGlyphContext_ = {};

  for (auto& ctx : activeGlyphs_)
  {
    loadSingleContext(&ctx, prev);
    prev = &ctx;
  }

  glyphCacheValid_ = true;
}


int
StringRenderer::prepareLine(int offset,
                            int lineWidth,
                            FT_Vector& outActualLineWidth)
{
  int totalCount = 0;
  outActualLineWidth = {0, 0};
  if (!usingString_)
  {
    // The thing gets a little complicated when we're using "All Glyphs" mode
    // The input sequence is actually infinite
    // so we have to combine loading glyph into rendering, and can't preload
    // all glyphs

    // TODO: Low performance when the begin index is large.
    // TODO: Optimize: use a sparse vector...!
    // The problem is that when doing a `list::resize`, the ctor is called
    // for unnecessarily many times.
    GlyphContext* prev = &tempGlyphContext_;
    tempGlyphContext_ = {};
    for (unsigned n = offset; n < static_cast<unsigned>(limitIndex_);)
    {
      if (activeGlyphs_.capacity() <= n)
        activeGlyphs_.reserve(static_cast<size_t>(n) * 2);
      if (activeGlyphs_.size() <= n)
        activeGlyphs_.resize(n + 1);

      auto& ctx = activeGlyphs_[n];
      ctx.charCode = static_cast<int>(n);
      ctx.glyphIndex = static_cast<int>(
          engine_->glyphIndexFromCharCode(static_cast<int>(n), charMapIndex_));

      if (!ctx.glyph)
        loadSingleContext(&ctx, prev);

      if (outActualLineWidth.x + ctx.hadvance.x > lineWidth)
        break;
      outActualLineWidth.x += ctx.hadvance.x;
      outActualLineWidth.y += ctx.hadvance.y;
      ++n;
      ++totalCount;
      prev = &ctx;
    }
  }
  else
  {
    if (!glyphCacheValid_)
    {
      clearActive(true);
      loadStringGlyphs();
    }

    for (unsigned n = offset; n < activeGlyphs_.size();)
    {
      auto& ctx = activeGlyphs_[n];
      if (repeated_) // if repeated, we must stop when we touch the end of line
      {
        if (outActualLineWidth.x + ctx.hadvance.x > lineWidth)
          break;
        outActualLineWidth.x += ctx.hadvance.x;
        outActualLineWidth.y += ctx.hadvance.y;
        ++n;
        n %= static_cast<int>(activeGlyphs_.size()); // safe
      }
      else if (vertical_)
      {
        outActualLineWidth.x += ctx.vadvance.x;
        outActualLineWidth.y += ctx.vadvance.y;
        ++n;
      }
      else
      {
        outActualLineWidth.x += ctx.hadvance.x;
        outActualLineWidth.y += ctx.hadvance.y;
        ++n;
      }
      ++totalCount;
    }
  }
  return totalCount;
}


int
StringRenderer::render(int width,
                       int height,
                       int offset)
{
  if (usingString_)
    offset = 0;
  if (limitIndex_ <= 0)
    return 0;

  // Separated into 3 modes:
  // Waterfall, fill the whole canvas and only single string.

  if (waterfall_)
  {
    // Waterfall

    vertical_ = false;
    auto originalSize = static_cast<int>(engine_->pointSize() * 64);
    auto ptSize = originalSize;
    auto ptHeight = 64 * 72 * height / engine_->dpi();
    auto step = (ptSize * ptSize / ptHeight + 64) & ~63;
    ptSize = ptSize - step * (ptSize / step); // modulo

    int y = 0;
    // no position param in "All Glyphs" mode
    int x = static_cast<int>(usingString_ ? (width * position_) : 0);
    int count = 0;

    while (true)
    {
      ptSize += step;
      engine_->setSizeByPoint(ptSize / 64);
      clearActive(true);
      prepareRendering(); // set size/face for engine, so metrics are valid
      auto& metrics = engine_->currentFontMetrics();

      if (ptSize == originalSize)
      {
        // TODO draw a blue line
      }

      engine_->setSizeByPoint(ptSize / 64);
      y += (metrics.height >> 6) + 1;

      if (y >= height)
        break;

      if (ptSize == originalSize)
      {
        // TODO draw a blue line
      }

      loadStringGlyphs();
      auto lcount = renderLine(x, y + (metrics.descender >> 6),
                                   width, height,
                                   offset);
      count = std::max(count, lcount);
    }
    engine_->setSizeByPoint(originalSize / 64);

    return count;
  }

  if (repeated_ || !usingString_)
  {
    // Fill the whole canvas

    prepareRendering();
    auto& metrics = engine_->currentFontMetrics();
    auto stepY = (metrics.height >> 6) + 1;
    auto y = 4 + (metrics.ascender >> 6);
    auto limitY = height + (metrics.descender >> 6);

    for (; y < limitY; y += stepY)
      offset = renderLine(0, y, width, height, offset);
    return offset;
  }

  // Single string
  prepareRendering();
  auto& metrics = engine_->currentFontMetrics();
  auto x = static_cast<int>(width * position_);
  // Anchor at top-left in vertical mode, at the center in horizontal mode
  auto y = vertical_ ? 0 : (height / 2);
  y += 4 + (metrics.ascender >> 6);
  return renderLine(x, y, width, height, offset);
}


int
StringRenderer::renderLine(int x,
                           int y,
                           int width,
                           int height,
                           int offset)
{
  if (x < 0 || y < 0 || x > width || y > height)
    return 0;

  y = height - y; // change to Cartesian coordinates

  FT_Vector pen = { 0, 0 };
  FT_Vector advance;

  // When in "All Glyphs"  mode, no vertical support.
  if (repeated_ || !usingString_)
    vertical_ = false; // TODO: Support vertical + repeated

  int lineLength = 64 * (vertical_ ? height : width);

  // first prepare the line & determine the line length
  int totalCount = prepareLine(offset, lineLength, pen);

  // round to control initial pen position and preserve hinting...
  // pen.x, y is the actual length now, and we multiple it by pos
  auto centerFixed = static_cast<int>(0x10000 * position_);
  if (!usingString_ || repeated_)
    centerFixed = 0;
  pen.x = FT_MulFix(pen.x, centerFixed) & ~63;
  pen.y = FT_MulFix(pen.y, centerFixed) & ~63;

  // ... unless rotating; XXX sbits
  if (matrixEnabled_)
    FT_Vector_Transform(&pen, &matrix_);

  // get pen position: penPos = center - pos * width
  pen.x = (x << 6) - pen.x;
  pen.y = (y << 6) - pen.y;

  // Need to transform the coord back to normal coord system
  lineBeginCallback_({ (pen.x >> 6), 
                       height - (pen.y >> 6) },
                     engine_->pointSize());

  for (int i = offset; i < totalCount + offset; i++)
  {
    auto& ctx = activeGlyphs_[i % activeGlyphs_.size()];
    FT_Glyph image = NULL; // Remember to clean up
    FT_BBox bbox;

    if (!ctx.glyph)
      continue;

    // copy the glyph because we're doing manipulation
    auto error = FT_Glyph_Copy(ctx.glyph, &image);
    if (error)
      continue;

    glyphPreprocessCallback_(&image);

    if (image->format != FT_GLYPH_FORMAT_BITMAP)
    {
      if (vertical_)
        error = FT_Glyph_Transform(image, NULL, &ctx.vvector);

      if (!error)
      {
        if (matrixEnabled_)
          error = FT_Glyph_Transform(image, &matrix_, &pen);
        else
          error = FT_Glyph_Transform(image, NULL, &pen);
      }

      if (error)
      {
        FT_Done_Glyph(image);
        continue;
      }
    }
    else
    {
      auto bitmap = reinterpret_cast<FT_BitmapGlyph>(image);

      if (vertical_)
      {
        bitmap->left += (ctx.vvector.x + pen.x) >> 6;
        bitmap->top += (ctx.vvector.y + pen.y) >> 6;
      }
      else
      {
        bitmap->left += pen.x >> 6;
        bitmap->top += pen.y >> 6;
      }
    }

    advance = vertical_ ? ctx.vadvance : ctx.hadvance;

    if (matrixEnabled_)
      FT_Vector_Transform(&advance, &matrix_);

    pen.x += advance.x;
    pen.y += advance.y;

    FT_Glyph_Get_CBox(image, FT_GLYPH_BBOX_PIXELS, &bbox);

    // check bounding box; if it is completely outside the
    // display surface, we don't need to render it
    if (bbox.xMax > 0 
        && bbox.yMax > 0
        && bbox.xMin < width
        && bbox.yMin < height)
    {
      renderCallback_(image);
    }

    FT_Done_Glyph(image);
  }

  // For repeating
  if (usingString_ && activeGlyphs_.size())
    return (offset + totalCount) % activeGlyphs_.size();
  return offset + totalCount;
}


void
StringRenderer::clearActive(bool glyphOnly)
{
  for (auto& ctx : activeGlyphs_)
  {
    if (ctx.cacheNode)
      FTC_Node_Unref(ctx.cacheNode, engine_->cacheManager());
    else if (ctx.glyph)
      FT_Done_Glyph(ctx.glyph); // when caching isn't used
    ctx.cacheNode = NULL;
    ctx.glyph = NULL;
  }
  if (!glyphOnly)
    activeGlyphs_.clear();

  glyphCacheValid_ = false;
}


// end of stringrenderer.cpp
