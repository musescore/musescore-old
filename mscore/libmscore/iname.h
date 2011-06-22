//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __INAME_H__
#define __INAME_H__

#include "text.h"

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

class InstrumentName : public Text  {
      int _layoutPos;

   public:
      InstrumentName(Score*);
      virtual InstrumentName* clone() const { return new InstrumentName(*this); }
      virtual ElementType type() const { return INSTRUMENT_NAME; }
      int layoutPos() const      { return _layoutPos; }
      void setLayoutPos(int val) { _layoutPos = val;  }
      };

#endif

