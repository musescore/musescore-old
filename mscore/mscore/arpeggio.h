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

#ifndef __ARPEGGIO_H__
#define __ARPEGGIO_H__

#include "element.h"

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

class Arpeggio : public Element {
      Spatium _userLen1;
      Spatium _userLen2;
      double _height;

      virtual bool isMovable() const          { return true; }
      virtual QLineF dragAnchor() const;

   public:
      Arpeggio(Score* s);
      virtual Arpeggio* clone() const { return new Arpeggio(*this); }
      virtual ElementType type() const { return ARPEGGIO; }
      virtual QRectF bbox() const;
      virtual void draw(QPainter&) const;
      virtual bool isEditable() { return true; }
      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;
      void read(QDomElement e);
      void write(Xml& xml) const;

      void setHeight(double);
      };

#endif

