//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "config.h"
#include "style.h"
#include "mscore.h"

qreal PDPI;
qreal DPI;
qreal DPMM;

Style*  MScore::_defaultStyle;
Style*  MScore::_baseStyle;
QString MScore::_globalShare;
int     MScore::_vRaster;
int     MScore::_hRaster;
QColor  MScore::selectColor[VOICES];
QColor  MScore::defaultColor;
QColor  MScore::layoutBreakColor;
QColor  MScore::bgColor;
QColor  MScore::dropColor;
bool    MScore::warnPitchRange;

QPrinter::PageSize MScore::paperSize;
qreal   MScore::paperWidth;
qreal   MScore::paperHeight;
bool    MScore::landscape;
bool    MScore::twosided;
bool    MScore::replaceFractions;
bool    MScore::playRepeats;
qreal   MScore::nudgeStep;
int     MScore::defaultPlayDuration;
QString MScore::partStyle;
QString MScore::soundFont;
qreal   MScore::spatium;

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MScore::init()
      {
      _defaultStyle         = new Style();
      setDefaultStyle(_defaultStyle);     // initialize default style
      _baseStyle            = new Style(*_defaultStyle);

#ifdef __MINGW32__
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
      _globalShare = dir.absolutePath() + "/";
#else
#ifdef Q_WS_MAC
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
      _globalShare = dir.absolutePath() + "/";
#else
      _globalShare = QString( INSTPREFIX "/share/" INSTALL_NAME);
#endif
#endif
      selectColor[0].setRgb(0, 0, 255);     //blue
      selectColor[1].setRgb(0, 150, 0);     //green
      selectColor[2].setRgb(230, 180, 50);  //yellow
      selectColor[3].setRgb(200, 0, 200);   //purple
      defaultColor        = Qt::black;
      dropColor           = Qt::red;
      nudgeStep           = .1;       // in spatium units (default 0.1)
      defaultPlayDuration = 300;      // ms
      warnPitchRange      = true;
      paperSize           = QPrinter::A4;     // default paper size
      paperWidth          = 1.0;
      paperHeight         = 1.0;
      landscape           = false;
      twosided            = true;
      spatium             = SPATIUM20;
      replaceFractions    = true;
      partStyle           = "";
      layoutBreakColor    = Qt::green;
      soundFont           = _globalShare + "sound/TimGM6mb.sf2";
      bgColor.setRgb(0x76, 0x76, 0x6e);
      }

//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

Style* MScore::defaultStyle()
      {
      return _defaultStyle;
      }

//---------------------------------------------------------
//   baseStyle
//---------------------------------------------------------

Style* MScore::baseStyle()
      {
      return _baseStyle;
      }

