// graphicsdefault.hpp

// Copyright (C) 2022 by Charlie Jiang.

#pragma once

#include <QVector>
#include <QRgb>
#include <QPen>

// This is default graphics objects fed into render functions.
struct GraphicsDefault
{
  QPen axisPen;
  QPen blueZonePen;
  QPen gridPen;
  QPen offPen;
  QPen onPen;
  QPen outlinePen;
  QPen segmentPen;

  QPen advanceAuxPen;
  QPen ascDescAuxPen;

  GraphicsDefault();

  static GraphicsDefault* deafultInstance();

private:
  static GraphicsDefault* instance_;
};


// end of graphicsdefault.hpp
