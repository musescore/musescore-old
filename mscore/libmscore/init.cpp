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

#include "style.h"

static Style* _defaultStyle;
static Style* _baseStyle;

//---------------------------------------------------------
//   initMuseScore
//---------------------------------------------------------

void initMuseScore()
      {
      _defaultStyle         = new Style();
      setDefaultStyle(_defaultStyle);     // initialize default style
      _baseStyle            = new Style(*_defaultStyle);
      }

//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

Style* defaultStyle()
      {
      return _defaultStyle;
      }

//---------------------------------------------------------
//   baseStyle
//---------------------------------------------------------

Style* baseStyle()
      {
      return _baseStyle;
      }

