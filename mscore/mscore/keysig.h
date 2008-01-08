//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#ifndef __KEYSIG_H__
#define __KEYSIG_H__

#include "element.h"

class Sym;
class Segment;

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

/**
 The KeySig class represents a Key Signature on a staff
*/

class KeySig : public Element {
      void addLayout(Sym*, double x, double y);

      double yoffset() const;

   public:
      KeySig(Score*);
      virtual KeySig* clone() const { return new KeySig(*this); }
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      virtual void draw(QPainter&) const;
      virtual ElementType type() const { return KEYSIG; }
      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);
      virtual void layout(ScoreLayout*);
      void setSig(int oldSig, int newSig);
      Segment* segment() const { return (Segment*)parent(); }
      };

#endif

