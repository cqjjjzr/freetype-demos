// fontsizeselector.cpp

// Copyright (C) 2022 by Charlie Jiang.

#include "fontsizeselector.hpp"

#include "../engine/engine.hpp"

FontSizeSelector::FontSizeSelector(QWidget* parent)
: QWidget(parent)
{
  createLayout();
  createConnections();
  setDefaults();
}


double
FontSizeSelector::selectedSize()
{
  return sizeDoubleSpinBox_->value();
}


FontSizeSelector::Units
FontSizeSelector::selectedUnit()
{
  return static_cast<Units>(unitsComboBox_->currentIndex());
}


void
FontSizeSelector::applyToEngine(Engine* engine)
{
  // Spinbox value cannot become negative
  engine->setDPI(dpiSpinBox_->value());

  if (unitsComboBox_->currentIndex() == Units_px)
    engine->setSizeByPixel(sizeDoubleSpinBox_->value());
  else
    engine->setSizeByPoint(sizeDoubleSpinBox_->value());
}


void
FontSizeSelector::handleWheelResizeBySteps(int steps)
{
  double sizeAfter = sizeDoubleSpinBox_->value()
                       + steps * sizeDoubleSpinBox_->singleStep();
  sizeAfter = std::max(sizeDoubleSpinBox_->minimum(),
                       std::min(sizeAfter, sizeDoubleSpinBox_->maximum()));
  sizeDoubleSpinBox_->setValue(sizeAfter);
}


void
FontSizeSelector::handleWheelResizeFromGrid(QWheelEvent* event)
{
  int numSteps = event->angleDelta().y() / 120;
  handleWheelResizeBySteps(numSteps);
}


bool
FontSizeSelector::handleKeyEvent(QKeyEvent const* keyEvent)
{
  if (!keyEvent)
    return false;
  auto modifiers = keyEvent->modifiers();
  auto key = keyEvent->key();
  if ((modifiers == Qt::ShiftModifier
       || modifiers == (Qt::ShiftModifier | Qt::KeypadModifier))
      && (key == Qt::Key_Plus 
          || key == Qt::Key_Minus
          || key == Qt::Key_Underscore
          || key == Qt::Key_Equal
          || key == Qt::Key_ParenRight))
  {
    if (key == Qt::Key_Plus || key == Qt::Key_Equal)
      handleWheelResizeBySteps(1);
    else if (key == Qt::Key_Minus
             || key == Qt::Key_Underscore)
      handleWheelResizeBySteps(-1);
    else if (key == Qt::Key_ParenRight)
      setDefaults(true);
    return true;
  }
  return false;
}


void
FontSizeSelector::installEventFilterForWidget(QWidget* widget)
{
  widget->installEventFilter(this);
}


bool
FontSizeSelector::eventFilter(QObject* watched,
                              QEvent* event)
{
  if (event->type() == QEvent::KeyPress)
  {
    auto keyEvent = dynamic_cast<QKeyEvent*>(event);
    if (handleKeyEvent(keyEvent))
      return true;
  }
  return QWidget::eventFilter(watched, event);
}


void
FontSizeSelector::checkUnits()
{
  int index = unitsComboBox_->currentIndex();

  if (index == Units_px)
  {
    dpiLabel_->setEnabled(false);
    dpiSpinBox_->setEnabled(false);
    sizeDoubleSpinBox_->setSingleStep(1);

    QSignalBlocker blocker(sizeDoubleSpinBox_);
    sizeDoubleSpinBox_->setValue(qRound(sizeDoubleSpinBox_->value()));
  }
  else
  {
    dpiLabel_->setEnabled(true);
    dpiSpinBox_->setEnabled(true);
    sizeDoubleSpinBox_->setSingleStep(0.5);
  }

  emit valueChanged();
}


void
FontSizeSelector::createLayout()
{
  sizeLabel_ = new QLabel(tr("Size "), this);
  sizeLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  sizeDoubleSpinBox_ = new QDoubleSpinBox;
  sizeDoubleSpinBox_->setAlignment(Qt::AlignRight);
  sizeDoubleSpinBox_->setDecimals(1);
  sizeDoubleSpinBox_->setRange(1, 500);
  sizeLabel_->setBuddy(sizeDoubleSpinBox_);

  unitsComboBox_ = new QComboBox(this);
  unitsComboBox_->insertItem(Units_px, "px");
  unitsComboBox_->insertItem(Units_pt, "pt");

  dpiLabel_ = new QLabel(tr("DPI "), this);
  dpiLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  dpiSpinBox_ = new QSpinBox(this);
  dpiSpinBox_->setAlignment(Qt::AlignRight);
  dpiSpinBox_->setRange(10, 600);
  dpiLabel_->setBuddy(dpiSpinBox_);

  layout_ = new QHBoxLayout;

  layout_->addStretch(1);
  layout_->addWidget(sizeLabel_);
  layout_->addWidget(sizeDoubleSpinBox_);
  layout_->addWidget(unitsComboBox_);
  layout_->addStretch(1);
  layout_->addWidget(dpiLabel_);
  layout_->addWidget(dpiSpinBox_);
  layout_->addStretch(1);

  setLayout(layout_);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}


void
FontSizeSelector::createConnections()
{
  connect(sizeDoubleSpinBox_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &FontSizeSelector::valueChanged);
  connect(unitsComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &FontSizeSelector::checkUnits);
  connect(dpiSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &FontSizeSelector::valueChanged);
}


void
FontSizeSelector::setDefaults(bool sizeOnly)
{
  sizeDoubleSpinBox_->setValue(20);
  if (sizeOnly)
    return;
  dpiSpinBox_->setValue(96);
  checkUnits();
}


// end of fontsizeselector.cpp
