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

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "bsymbol.h"

class Segment;
class Painter;

//---------------------------------------------------------
//   Symbol
//    score symbol
//---------------------------------------------------------

class Symbol : public BSymbol {

   protected:
      int _sym;
      bool _small;

   public:
      Symbol(Score* s);
      Symbol(Score* s, int sy);
      Symbol(const Symbol&);

      Symbol &operator=(const Symbol&);

      virtual Symbol* clone() const     { return new Symbol(*this); }
      virtual ElementType type() const  { return SYMBOL; }

      void setSym(int s) { _sym  = s;    }
      int sym() const    { return _sym;  }

      virtual void draw(Painter*) const;
      virtual void read(XmlReader*);
      virtual void layout();
      void setAbove(bool);

      virtual qreal baseLine() const { return 0.0; }
      Segment* segment() const { return (Segment*)parent(); }
      bool small() const       { return _small; }
      void setSmall(bool val)  { _small = val; }
      };

#endif

