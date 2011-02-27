//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __BSYMBOL_H__
#define __BSYMBOL_H__

#include "element.h"
#include "elementlayout.h"

//---------------------------------------------------------
//   BSymbol
//    base class for Symbol and Image
//---------------------------------------------------------

class BSymbol : public Element, public ElementLayout {
      QList<Element*> _leafs;
      int _z;                     ///< stacking order when drawing or selecting;
                                  ///< elements are drawn from high number to low number;
                                  ///< default is type() * 100;

   public:
      BSymbol(Score* s) : Element(s) { setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE); }
      BSymbol(const BSymbol&);

      BSymbol &operator=(const BSymbol&);

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void scanElements(void* data, void (*func)(void*, Element*));
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual QRectF drag(const QPointF& pos);

      const QList<Element*>& leafs() const { return _leafs; }
      QList<Element*>& leafs()             { return _leafs; }
      virtual QPointF canvasPos() const;
      Segment* segment() const            { return (Segment*)parent(); }
      virtual int z() const               { return _z; }
      void setZ(int val)                  { _z = val;  }
      };

#endif

