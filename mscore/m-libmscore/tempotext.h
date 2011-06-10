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

#ifndef __TEMPOTEXT_H__
#define __TEMPOTEXT_H__

#include "text.h"

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

class TempoText : public Text  {
      qreal _tempo;     // beats per second

   public:
      TempoText(Score*);
      virtual TempoText* clone() const { return new TempoText(*this); }
      virtual ElementType type() const { return TEMPO_TEXT; }
      virtual void read(XmlReader*);
      Segment* segment() const { return (Segment*)parent(); }
      Measure* measure() const { return (Measure*)parent()->parent(); }
      qreal tempo() const     { return _tempo; }
      void setTempo(qreal v)  { _tempo = v;  }
      };

#endif
