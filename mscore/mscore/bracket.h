//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: bracket.h,v 1.6 2006/03/02 17:08:33 wschweer Exp $
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

#ifndef __BRACKET_H__
#define __BRACKET_H__

#include "element.h"

// System Brackets
enum { BRACKET_NORMAL, BRACKET_AKKOLADE, NO_BRACKET = -1};

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

class Bracket : public Element {
      int _span;
      int mode;
      qreal h2;

      QPainterPath* path;
      QRectF grip[2];
      qreal yoff;

      void updateGrips(QMatrix& matrix);

   public:
      Bracket(Score*, int);
      virtual ElementType type() const { return BRACKET; }

      int span() const      { return _span; }
      void setSpan(int val) { _span = val; }

      virtual void setHeight(qreal);
      virtual double width() const;

      virtual void draw1(Painter&);
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      virtual void layout();

      virtual bool startEdit(QMatrix&);
      virtual bool edit(QKeyEvent*);
      virtual void endEdit();
      virtual bool startEditDrag(const QPointF&);
      virtual bool editDrag(QMatrix&, QPointF*, const QPointF&);
      virtual bool endEditDrag();
      virtual void bboxUpdate();
      virtual QPointF dragOff() const;

      virtual bool acceptDrop(const QPointF&, int, int) const;
      virtual void drop(const QPointF&, int, int);
      };

#endif

