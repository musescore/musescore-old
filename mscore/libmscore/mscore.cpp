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
#include "mscore.h"

Style* MScore::_defaultStyle;
Style* MScore::_baseStyle;

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MScore::init()
      {
      _defaultStyle         = new Style();
      setDefaultStyle(_defaultStyle);     // initialize default style
      _baseStyle            = new Style(*_defaultStyle);
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

