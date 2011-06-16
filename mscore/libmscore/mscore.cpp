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

Style* MScore::_defaultStyle;
Style* MScore::_baseStyle;
QString MScore::_globalShare;

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

