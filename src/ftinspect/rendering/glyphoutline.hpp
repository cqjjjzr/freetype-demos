// glyphoutline.hpp

// Copyright (C) 2016-2022 by Werner Lemberg.


#pragma once

#include <QGraphicsItem>
#include <QPen>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>


class GlyphOutline
: public QGraphicsItem
{
public:
  GlyphOutline(const QPen& pen,
               FT_Outline* outline);
  QRectF boundingRect() const override;
  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;

private:
  QPen outlinePen_;
  FT_Outline* outline_;
  QRectF boundingRect_;
};


// end of glyphoutline.hpp
