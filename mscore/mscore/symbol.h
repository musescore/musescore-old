//=============================================================================
//  MusE Score
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

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "bsymbol.h"

//---------------------------------------------------------
//   Symbol
//    score symbol
//---------------------------------------------------------

class Symbol : public BSymbol {

   protected:
      int _sym;

   public:
      Symbol(Score* s)         : BSymbol(s) { _sym = 0;  }
      Symbol(Score* s, int sy) : BSymbol(s) { _sym = sy; }

      Symbol &operator=(const Symbol&);

      virtual Symbol* clone() const     { return new Symbol(*this); }
      virtual ElementType type() const  { return SYMBOL; }
      virtual QLineF dragAnchor() const;

      void setSym(int s) { _sym  = s;    }
      int sym() const    { return _sym;  }

      virtual void draw(QPainter&, ScoreView*) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual QRectF bbox() const;
      virtual void layout();
      void setAbove(bool);

      virtual qreal baseLine() const { return 0.0; }
      void setTick(int val) { _tick = val; }
      };

#endif

