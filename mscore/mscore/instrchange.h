//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: stafftext.h 3229 2010-06-27 14:55:28Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __INSTRCHANGE_H__
#define __INSTRCHANGE_H__

#include "text.h"
#include "part.h"
#include "instrument.h"

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

class InstrumentChange : public Text  {
      Instrument _instrument;

   public:
      InstrumentChange(Score*);
      virtual InstrumentChange* clone() const { return new InstrumentChange(*this); }
      virtual ElementType type() const        { return INSTRUMENT_CHANGE; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      Instrument instrument() const           { return _instrument; }
      void setInstrument(const Instrument& i) { _instrument = i;    }
      Segment* segment()                      { return (Segment*)parent(); }
      };

#endif
