//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: stafftext.cpp 3199 2010-06-19 19:41:16Z wschweer $
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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

#include "instrchange.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "undo.h"
#include "mscore.h"

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

InstrumentChange::InstrumentChange(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_INSTRUMENT_CHANGE);
      setTextStyle(TEXT_STYLE_INSTRUMENT_CHANGE);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentChange::write(Xml& xml) const
      {
      xml.stag("InstrumentChange");
      _instrument.write(xml);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentChange::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "Instrument")
                  _instrument.read(e);
            else if (!Text::readProperties(e))
                  domError(e);
            }
      }

