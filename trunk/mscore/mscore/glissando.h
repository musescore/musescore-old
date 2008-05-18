//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#ifndef __GLISSANDO_H__
#define __GLISSANDO_H__

#include "element.h"

class Note;

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

class Glissando : public Element {
      Note* _anchor1;
      Note* _anchor2;
      QLine line;

   public:
      Glissando(Score* s);
      virtual Glissando* clone() const { return new Glissando(*this); }
      virtual ElementType type() const { return GLISSANDO; }
      virtual void space(double& min, double& extra) const;
      virtual QRectF bbox() const;

      virtual void draw(QPainter&) const;
      virtual void layout(ScoreLayout*);
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      virtual bool isMovable() const   { return false; }

      void setSize(const QSizeF&);        // used for palette
      };

#endif

