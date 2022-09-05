// glyphindexselector.hpp

// Copyright (C) 2022 by Charlie Jiang.

#pragma once

#include <functional>
#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QSignalMapper>
#include <QHBoxLayout>
#include <QLabel>

class GlyphIndexSelector
: public QWidget
{
  Q_OBJECT
public:
  GlyphIndexSelector(QWidget* parent);
  ~GlyphIndexSelector() override = default;

  // Will never trigger repaint!
  void setMinMax(int min, int max);
  void setShowingCount(int showingCount);

  // Single mode will display single glyph index instead of a range.
  void setSingleMode(bool singleMode);

  void setCurrentIndex(int index, bool forceUpdate = false);
  int currentIndex();

  void setNumberRenderer(std::function<QString(int)> renderer);

signals:
  void currentIndexChanged(int index);

protected:
  void resizeEvent(QResizeEvent* event) override;
  
private:
  bool singleMode_ = true;
  int showingCount_;
  std::function<QString(int)> numberRenderer_;

  // min, max and current status are held by `indexSpinBox_`
  QWidget* navigationWidget_;
  QPushButton* toEndButton_;
  QPushButton* toM1000Button_;
  QPushButton* toM100Button_;
  QPushButton* toM10Button_;
  QPushButton* toM1Button_;
  QPushButton* toP1000Button_;
  QPushButton* toP100Button_;
  QPushButton* toP10Button_;
  QPushButton* toP1Button_;
  QPushButton* toStartButton_;

  QLabel* indexLabel_;
  QSpinBox* indexSpinBox_;

  QHBoxLayout* navigationLayout_;
  QHBoxLayout* layout_;

  QSignalMapper* glyphNavigationMapper_;

  void createLayout();
  void createConnections();

  void adjustIndex(int delta);
  void emitValueChanged();
  void updateLabel();

  static QString renderNumberDefault(int i);
};


// end of glyphindexselector.hpp
