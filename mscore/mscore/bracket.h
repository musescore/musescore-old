//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: bracket.h,v 1.6 2006/03/02 17:08:33 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

class Viewer;
class System;

// System Brackets
enum { BRACKET_NORMAL, BRACKET_AKKOLADE, NO_BRACKET = -1};

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

class Bracket : public Element {
      int _span;
      qreal h2;

      int _level;

      QPainterPath path;
      qreal yoff;

   public:
      Bracket(Score*);
      virtual Bracket* clone() const { return new Bracket(*this); }
      virtual ElementType type() const { return BRACKET; }

      int span() const      { return _span; }
      void setSpan(int val) { _span = val; }
      int level() const     { return _level; }
      void setLevel(int v)  { _level = v; }
      System* system() const { return (System*)parent(); }

      virtual QRectF bbox() const;
      virtual void setHeight(qreal);
      virtual double width() const;

      virtual void draw(QPainter&) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void layout(ScoreLayout*);

      virtual bool startEdit(const QPointF&);
      virtual void endEdit();
      virtual void editDrag(int, const QPointF&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int grip) const;

      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);
      };

#endif

