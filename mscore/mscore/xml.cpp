//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "xml.h"
#include "globals.h"
#include "config.h"

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

Xml::Xml()
   : AL::Xml()
      {
      curTick       = 0;
      curTrack      = -1;
      trackDiff     = 0;
      clipboardmode = false;
      excerptmode   = false;
      tupletId      = 1;
      beamId        = 1;
      }

Xml::Xml(QIODevice* device)
   : AL::Xml(device)
      {
      curTick       = 0;
      curTrack      = -1;
      trackDiff     = 0;
      clipboardmode = false;
      excerptmode   = false;
      tupletId      = 1;
      beamId        = 1;
      }

//---------------------------------------------------------
//   pTag
//---------------------------------------------------------

void Xml::pTag(const char* name, Placement place)
      {
      const char* tags[] = {
            "auto", "above", "below", "left"
            };
      tag(name, tags[int(place)]);
      }

//---------------------------------------------------------
//   valueTypeTag
//---------------------------------------------------------

void Xml::valueTypeTag(const char* name, ValueType t)
      {
      const char* s;
      switch(t) {
            case AUTO_VAL:   s = "auto"; break;
            case USER_VAL:   s = "user"; break;
            case OFFSET_VAL: s = "offset"; break;
            }
      tag(name, s);
      }

//---------------------------------------------------------
//   readPlacement
//---------------------------------------------------------

Placement readPlacement(QDomElement e)
      {
      QString s(e.text());
      if (s == "auto" || s == "0")
            return PLACE_AUTO;
      if (s == "above" || s == "1")
            return PLACE_ABOVE;
      if (s == "below" || s == "2")
            return PLACE_BELOW;
      if (s == "left" || s == "3")
            return PLACE_LEFT;
      printf("unknown placement value <%s>\n", qPrintable(s));
      return PLACE_AUTO;
      }

//---------------------------------------------------------
//   readValueType
//---------------------------------------------------------

ValueType readValueType(QDomElement e)
      {
      QString s(e.text());
      if (s == "offset")
            return OFFSET_VAL;
      if (s == "user")
            return USER_VAL;
      return AUTO_VAL;
      }

//---------------------------------------------------------
//   fTag
//---------------------------------------------------------

void Xml::fTag(const char* name, const Fraction& f)
      {
      tagE(QString("%1 z=\"%2\" n=\"%3\"").arg(name).arg(f.numerator()).arg(f.denominator()));
      }

//---------------------------------------------------------
//   readFraction
//---------------------------------------------------------

Fraction readFraction(QDomElement e)
      {
      double z = e.attribute("z", "0.0").toDouble();
      double n = e.attribute("n", "0.0").toDouble();
      return Fraction(z, n);
      }

