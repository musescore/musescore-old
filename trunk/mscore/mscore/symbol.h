//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h,v 1.58 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "element.h"

//---------------------------------------------------------
//   Symbol
//    score symbol
//---------------------------------------------------------

class Symbol : public Element {
   protected:
      int _sym;

   public:
      Symbol(Score*);
      Symbol &operator=(const Symbol&);

      virtual Symbol* clone() const { return new Symbol(*this); }
      virtual ElementType type() const { return SYMBOL; }
      void setSym(int);
      int sym() const { return _sym;  }

      virtual void draw1(Painter&);
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      virtual bool isMovable() const { return true; }
      virtual const QRectF& bbox() const;
      };

typedef pstl::plist<Symbol*> SymbolList;
typedef SymbolList::iterator iSymbol;
typedef SymbolList::const_iterator ciSymbol;

#endif

