//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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
      virtual void read(XmlReader*);

      Instrument instrument() const           { return _instrument; }
      void setInstrument(const Instrument& i) { _instrument = i;    }
      Segment* segment()                      { return (Segment*)parent(); }
      };

#endif
