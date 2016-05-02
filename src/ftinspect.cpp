// ftinspect.cpp

// Copyright (C) 2016 by Werner Lemberg.

#include "ftinspect.h"


#define VERSION "X.Y.Z"


MainGUI::MainGUI()
{
  createLayout();
  createConnections();
  createActions();
  createMenus();
  createStatusBar();

  setDefaults();
  readSettings();

  setUnifiedTitleAndToolBarOnMac(true);
}


MainGUI::~MainGUI()
{
}


// overloading

void
MainGUI::closeEvent(QCloseEvent* event)
{
  writeSettings();
  event->accept();
}


void
MainGUI::about()
{
  QMessageBox::about(
    this,
    tr("About ftinspect"),
    tr("<p>This is <b>ftinspect</b> version %1<br>"
       " Copyright %2 2016<br>"
       " by Werner Lemberg <tt>&lt;wl@gnu.org&gt;</tt></p>"
       ""
       "<p><b>ftinspect</b> shows how a font gets rendered"
       " by FreeType, allowing control over virtually"
       " all rendering parameters.</p>"
       ""
       "<p>License:"
       " <a href='http://git.savannah.gnu.org/cgit/freetype/freetype2.git/tree/docs/FTL.TXT'>FreeType"
       " License (FTL)</a> or"
       " <a href='http://git.savannah.gnu.org/cgit/freetype/freetype2.git/tree/docs/GPLv2.TXT'>GNU"
       " GPLv2</a></p>")
       .arg(VERSION)
       .arg(QChar(0xA9)));
}


void
MainGUI::checkHintingMode()
{
  int index = hintingModeComboBox->currentIndex();
  const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>
                                      (antiAliasingComboBox->model());
  QStandardItem* AAslightItem = model->item(AntiAliasing_Slight);
  int AAcurrIndex = antiAliasingComboBox->currentIndex();

  if (index == HintingMode_AutoHinting)
  {
    horizontalHintingCheckBox->setEnabled(true);
    verticalHintingCheckBox->setEnabled(true);
    blueZoneHintingCheckBox->setEnabled(true);
    segmentDrawingCheckBox->setEnabled(true);
    warpingCheckBox->setEnabled(true);

    AAslightItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    AAslightItem->setData(QVariant(),
                          Qt::TextColorRole);
  }
  else
  {
    horizontalHintingCheckBox->setEnabled(false);
    verticalHintingCheckBox->setEnabled(false);
    blueZoneHintingCheckBox->setEnabled(false);
    segmentDrawingCheckBox->setEnabled(false);
    warpingCheckBox->setEnabled(false);

    AAslightItem->setFlags(AAslightItem->flags()
                           & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
    // clear item data in order to use default color;
    // this visually greys out the item
    AAslightItem->setData(antiAliasingComboBox->palette().color(
                            QPalette::Disabled, QPalette::Text),
                          Qt::TextColorRole);

    if (AAcurrIndex == AntiAliasing_Slight)
      antiAliasingComboBox->setCurrentIndex(AntiAliasing_Normal);
  }
}


void
MainGUI::checkAntiAliasing()
{
  int index = antiAliasingComboBox->currentIndex();

  if (index == AntiAliasing_None
      || index == AntiAliasing_Normal
      || index == AntiAliasing_Slight)
  {
    lcdFilterLabel->setEnabled(false);
    lcdFilterComboBox->setEnabled(false);
  }
  else
  {
    lcdFilterLabel->setEnabled(true);
    lcdFilterComboBox->setEnabled(true);
  }
}


void
MainGUI::checkShowPoints()
{
  if (showPointsCheckBox->isChecked())
    showPointIndicesCheckBox->setEnabled(true);
  else
    showPointIndicesCheckBox->setEnabled(false);
}


void
MainGUI::checkUnits()
{
  int index = unitsComboBox->currentIndex();

  if (index == Units_px)
  {
    dpiLabel->setEnabled(false);
    dpiSpinBox->setEnabled(false);
  }
  else
  {
    dpiLabel->setEnabled(true);
    dpiSpinBox->setEnabled(true);
  }
}


// XXX distances are specified in pixels,
//     making the layout dependent on the output device resolution
void
MainGUI::createLayout()
{
  // left side
  hintingModeLabel = new QLabel(tr("Hinting Mode"));
  hintingModeLabel->setAlignment(Qt::AlignRight);
  hintingModeComboBox = new QComboBox;
  hintingModeComboBox->insertItem(HintingMode_TrueType_v35,
                                  tr("TrueType v35"));
  hintingModeComboBox->insertItem(HintingMode_TrueType_v38,
                                  tr("TrueType v38"));
  hintingModeComboBox->insertItem(HintingMode_TrueType_v40,
                                  tr("TrueType v40"));
  hintingModeComboBox->insertItem(HintingMode_CFF_FreeType,
                                  tr("CFF (FreeType)"));
  hintingModeComboBox->insertItem(HintingMode_CFF_Adobe,
                                  tr("CFF (Adobe)"));
  hintingModeComboBox->insertItem(HintingMode_AutoHinting,
                                  tr("Auto-Hinting"));
  hintingModeLabel->setBuddy(hintingModeComboBox);

  horizontalHintingCheckBox = new QCheckBox(tr("Horizontal Hinting"));
  verticalHintingCheckBox = new QCheckBox(tr("Vertical Hinting"));
  blueZoneHintingCheckBox = new QCheckBox(tr("Blue-Zone Hinting"));
  segmentDrawingCheckBox = new QCheckBox(tr("Segment Drawing"));
  warpingCheckBox = new QCheckBox(tr("Warping"));

  antiAliasingLabel = new QLabel(tr("Anti-Aliasing"));
  antiAliasingLabel->setAlignment(Qt::AlignRight);
  antiAliasingComboBox = new QComboBox;
  antiAliasingComboBox->insertItem(AntiAliasing_None,
                                   tr("None"));
  antiAliasingComboBox->insertItem(AntiAliasing_Normal,
                                   tr("Normal"));
  antiAliasingComboBox->insertItem(AntiAliasing_Slight,
                                   tr("Slight"));
  antiAliasingComboBox->insertItem(AntiAliasing_LCD,
                                   tr("LCD (RGB)"));
  antiAliasingComboBox->insertItem(AntiAliasing_LCD_BGR,
                                   tr("LCD (BGR)"));
  antiAliasingComboBox->insertItem(AntiAliasing_LCD_Vertical,
                                   tr("LCD (vert. RGB)"));
  antiAliasingComboBox->insertItem(AntiAliasing_LCD_Vertical_BGR,
                                   tr("LCD (vert. BGR)"));
  antiAliasingLabel->setBuddy(antiAliasingComboBox);

  lcdFilterLabel = new QLabel(tr("LCD Filter"));
  lcdFilterLabel->setAlignment(Qt::AlignRight);
  lcdFilterComboBox = new QComboBox;
  lcdFilterComboBox->insertItem(LCDFilter_Default, tr("Default"));
  lcdFilterComboBox->insertItem(LCDFilter_Light, tr("Light"));
  lcdFilterComboBox->insertItem(LCDFilter_None, tr("None"));
  lcdFilterComboBox->insertItem(LCDFilter_Legacy, tr("Legacy"));
  lcdFilterLabel->setBuddy(lcdFilterComboBox);

  int width;
  // make all labels have the same width
  width = hintingModeLabel->minimumSizeHint().width();
  width = qMax(antiAliasingLabel->minimumSizeHint().width(), width);
  width = qMax(lcdFilterLabel->minimumSizeHint().width(), width);
  hintingModeLabel->setMinimumWidth(width);
  antiAliasingLabel->setMinimumWidth(width);
  lcdFilterLabel->setMinimumWidth(width);

  // ensure that all items in combo boxes fit completely;
  // also make all combo boxes have the same width
  width = hintingModeComboBox->minimumSizeHint().width();
  width = qMax(antiAliasingComboBox->minimumSizeHint().width(), width);
  width = qMax(lcdFilterComboBox->minimumSizeHint().width(), width);
  hintingModeComboBox->setMinimumWidth(width);
  antiAliasingComboBox->setMinimumWidth(width);
  lcdFilterComboBox->setMinimumWidth(width);

  gammaLabel = new QLabel(tr("Gamma"));
  gammaLabel->setAlignment(Qt::AlignRight);
  gammaSlider = new QSlider(Qt::Horizontal);
  gammaSlider->setRange(0, 30); // in 1/10th
  gammaSlider->setTickPosition(QSlider::TicksBelow);
  gammaLabel->setBuddy(gammaSlider);

  showBitmapCheckBox = new QCheckBox(tr("Show Bitmap"));
  showPointsCheckBox = new QCheckBox(tr("Show Points"));
  showPointIndicesCheckBox = new QCheckBox(tr("Show Point Indices"));
  showOutlinesCheckBox = new QCheckBox(tr("Show Outlines"));

  watchButton = new QPushButton(tr("Watch"));

  hintingModeLayout = new QHBoxLayout;
  hintingModeLayout->addWidget(hintingModeLabel);
  hintingModeLayout->addWidget(hintingModeComboBox);

  antiAliasingLayout = new QHBoxLayout;
  antiAliasingLayout->addWidget(antiAliasingLabel);
  antiAliasingLayout->addWidget(antiAliasingComboBox);

  lcdFilterLayout = new QHBoxLayout;
  lcdFilterLayout->addWidget(lcdFilterLabel);
  lcdFilterLayout->addWidget(lcdFilterComboBox);

  gammaLayout = new QHBoxLayout;
  gammaLayout->addWidget(gammaLabel);
  gammaLayout->addWidget(gammaSlider);

  generalTabLayout = new QVBoxLayout;
  generalTabLayout->addLayout(hintingModeLayout);
  generalTabLayout->addWidget(horizontalHintingCheckBox);
  generalTabLayout->addWidget(verticalHintingCheckBox);
  generalTabLayout->addWidget(blueZoneHintingCheckBox);
  generalTabLayout->addWidget(segmentDrawingCheckBox);
  generalTabLayout->addWidget(warpingCheckBox);
  generalTabLayout->addSpacing(20); // XXX px
  generalTabLayout->addStretch(1);
  generalTabLayout->addLayout(antiAliasingLayout);
  generalTabLayout->addLayout(lcdFilterLayout);
  generalTabLayout->addSpacing(20); // XXX px
  generalTabLayout->addStretch(1);
  generalTabLayout->addLayout(gammaLayout);
  generalTabLayout->addSpacing(20); // XXX px
  generalTabLayout->addStretch(1);
  generalTabLayout->addWidget(showBitmapCheckBox);
  generalTabLayout->addWidget(showPointsCheckBox);
  generalTabLayout->addWidget(showPointIndicesCheckBox);
  generalTabLayout->addWidget(showOutlinesCheckBox);

  generalTabWidget = new QWidget;
  generalTabWidget->setLayout(generalTabLayout);

  mmgxTabWidget = new QWidget;

  watchLayout = new QHBoxLayout;
  watchLayout->addStretch(1);
  watchLayout->addWidget(watchButton);
  watchLayout->addStretch(1);

  tabWidget = new QTabWidget;
  tabWidget->addTab(generalTabWidget, tr("General"));
  tabWidget->addTab(mmgxTabWidget, tr("MM/GX"));

  leftLayout = new QVBoxLayout;
  leftLayout->addWidget(tabWidget);
  leftLayout->addSpacing(10); // XXX px
  leftLayout->addLayout(watchLayout);

  // we don't want to expand the left side horizontally;
  // to change the policy we have to use a widget wrapper
  leftWidget = new QWidget;
  leftWidget->setLayout(leftLayout);

  QSizePolicy leftWidgetPolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  leftWidgetPolicy.setHorizontalStretch(0);
  leftWidgetPolicy.setVerticalPolicy(leftWidget->sizePolicy().verticalPolicy());
  leftWidgetPolicy.setHeightForWidth(leftWidget->sizePolicy().hasHeightForWidth());

  leftWidget->setSizePolicy(leftWidgetPolicy);

  // right side
  glyphView = new QGraphicsView;

  sizeLabel = new QLabel(tr("Size "));
  sizeLabel->setAlignment(Qt::AlignRight);
  sizeDoubleSpinBox = new QDoubleSpinBox;
  sizeDoubleSpinBox->setAlignment(Qt::AlignRight);
  sizeDoubleSpinBox->setDecimals(1);
  sizeDoubleSpinBox->setRange(1, 500);
  sizeDoubleSpinBox->setSingleStep(0.5);
  sizeDoubleSpinBox->setValue(20); // XXX default
  sizeLabel->setBuddy(sizeDoubleSpinBox);

  unitsComboBox = new QComboBox;
  unitsComboBox->insertItem(Units_px, "px");
  unitsComboBox->insertItem(Units_pt, "pt");

  dpiLabel = new QLabel(tr("DPI "));
  dpiLabel->setAlignment(Qt::AlignRight);
  dpiSpinBox = new QSpinBox;
  dpiSpinBox->setAlignment(Qt::AlignRight);
  dpiSpinBox->setRange(10, 600);
  dpiSpinBox->setValue(96); // XXX default
  dpiLabel->setBuddy(dpiSpinBox);

  toStartButton = new QPushButton("|<");
  toStartButton->setFixedWidth(40);
  toM1000Button = new QPushButton("-1000");
  toM1000Button->setFixedWidth(65);
  toM100Button = new QPushButton("-100");
  toM100Button->setFixedWidth(55);
  toM10Button = new QPushButton("-10");
  toM10Button->setFixedWidth(50);
  toM1Button = new QPushButton("-1");
  toM1Button->setFixedWidth(50);
  toP1Button = new QPushButton("+1");
  toP1Button->setFixedWidth(50);
  toP10Button = new QPushButton("+10");
  toP10Button->setFixedWidth(50);
  toP100Button = new QPushButton("+100");
  toP100Button->setFixedWidth(55);
  toP1000Button = new QPushButton("+1000");
  toP1000Button->setFixedWidth(65);
  toEndButton = new QPushButton(">|");
  toEndButton->setFixedWidth(40);

  zoomLabel = new QLabel(tr("Zoom "));
  zoomLabel->setAlignment(Qt::AlignRight);
  zoomSpinBox = new QSpinBox;
  zoomSpinBox->setAlignment(Qt::AlignRight);
  zoomSpinBox->setRange(1, 10000);
  zoomSpinBox->setSuffix("%");
  zoomSpinBox->setSingleStep(10);
  zoomSpinBox->setValue(100); // XXX default
  zoomLabel->setBuddy(zoomSpinBox);

  previousFontButton = new QPushButton(tr("Previous Font"));
  nextFontButton = new QPushButton(tr("Next Font"));

  navigationLayout = new QHBoxLayout;
  navigationLayout->setSpacing(0);
  navigationLayout->addStretch(1);
  navigationLayout->addWidget(toStartButton);
  navigationLayout->addWidget(toM1000Button);
  navigationLayout->addWidget(toM100Button);
  navigationLayout->addWidget(toM10Button);
  navigationLayout->addWidget(toM1Button);
  navigationLayout->addWidget(toP1Button);
  navigationLayout->addWidget(toP10Button);
  navigationLayout->addWidget(toP100Button);
  navigationLayout->addWidget(toP1000Button);
  navigationLayout->addWidget(toEndButton);
  navigationLayout->addStretch(1);

  fontSizeLayout = new QHBoxLayout;
  fontSizeLayout->addWidget(sizeLabel);
  fontSizeLayout->addWidget(sizeDoubleSpinBox);
  fontSizeLayout->addWidget(unitsComboBox);
  fontSizeLayout->addSpacing(10); // XXX px
  fontSizeLayout->addWidget(dpiLabel);
  fontSizeLayout->addWidget(dpiSpinBox);
  fontSizeLayout->addSpacing(10); // XXX px
  fontSizeLayout->addStretch(1);
  fontSizeLayout->addWidget(previousFontButton);
  fontSizeLayout->addStretch(1);
  fontSizeLayout->addWidget(nextFontButton);
  fontSizeLayout->addStretch(1);
  fontSizeLayout->addSpacing(10); // XXX px
  fontSizeLayout->addWidget(zoomLabel);
  fontSizeLayout->addWidget(zoomSpinBox);

  rightLayout = new QVBoxLayout;
  rightLayout->addWidget(glyphView);
  rightLayout->addLayout(navigationLayout);
  rightLayout->addSpacing(10); // XXX px
  rightLayout->addLayout(fontSizeLayout);

  // for symmetry with the left side use a widget also
  rightWidget = new QWidget;
  rightWidget->setLayout(rightLayout);

  // the whole thing
  ftinspectLayout = new QHBoxLayout;
  ftinspectLayout->addWidget(leftWidget);
  ftinspectLayout->addWidget(rightWidget);

  ftinspectWidget = new QWidget;
  ftinspectWidget->setLayout(ftinspectLayout);
  setCentralWidget(ftinspectWidget);
  setWindowTitle("ftinspect");
}


void
MainGUI::createConnections()
{
  connect(hintingModeComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(checkHintingMode()));
  connect(antiAliasingComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(checkAntiAliasing()));

  connect(showPointsCheckBox, SIGNAL(clicked()), this,
          SLOT(checkShowPoints()));

  connect(unitsComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(checkUnits()));
}


void
MainGUI::createActions()
{
  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcuts(QKeySequence::Quit);
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  aboutAct = new QAction(tr("&About"), this);
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}


void
MainGUI::createMenus()
{
  menuFile = menuBar()->addMenu(tr("&File"));
  menuFile->addAction(exitAct);

  menuHelp = menuBar()->addMenu(tr("&Help"));
  menuHelp->addAction(aboutAct);
  menuHelp->addAction(aboutQtAct);
}


void
MainGUI::createStatusBar()
{
  statusBar()->showMessage("");
}


void
MainGUI::clearStatusBar()
{
  statusBar()->clearMessage();
  statusBar()->setStyleSheet("");
}


void
MainGUI::setDefaults()
{
  // XXX only dummy values right now

  hintingModeComboBox->setCurrentIndex(HintingMode_TrueType_v35);
  antiAliasingComboBox->setCurrentIndex(AntiAliasing_LCD);
  lcdFilterComboBox->setCurrentIndex(LCDFilter_Light);

  horizontalHintingCheckBox->setChecked(true);
  verticalHintingCheckBox->setChecked(true);
  blueZoneHintingCheckBox->setChecked(true);

  showBitmapCheckBox->setChecked(true);
  showOutlinesCheckBox->setChecked(true);

  checkHintingMode();
  checkAntiAliasing();
  checkShowPoints();
  checkUnits();
}


void
MainGUI::readSettings()
{
  QSettings settings;
//  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
//  QSize size = settings.value("size", QSize(400, 400)).toSize();
//  resize(size);
//  move(pos);
}


void
MainGUI::writeSettings()
{
  QSettings settings;
//  settings.setValue("pos", pos());
//  settings.setValue("size", size());
}


int
main(int argc,
     char** argv)
{
  QApplication app(argc, argv);
  app.setApplicationName("ftinspect");
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("FreeType");
  app.setOrganizationDomain("freetype.org");

  MainGUI gui;
  gui.show();

  return app.exec();
}


// end of ftinspect.cpp
