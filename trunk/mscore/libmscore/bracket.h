//=============================================================================
//  MuseScore
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

#ifndef __BRACKET_H__
#define __BRACKET_H__

#include "element.h"

class MuseScoreView;
class System;
class Painter;

// System Brackets
enum { BRACKET_NORMAL, BRACKET_AKKOLADE, NO_BRACKET = -1};

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

class Bracket : public Element {
      qreal h2;

      int _column, _span;

      QPainterPath path;
      qreal yoff;

   public:
      Bracket(Score*);
      virtual Bracket* clone() const { return new Bracket(*this); }
      virtual ElementType type() const { return BRACKET; }

      int span() const       { return _span; }
      void setSpan(int val)  { _span = val; }
      int level() const      { return _column; }
      void setLevel(int v)   { _column = v; }
      System* system() const { return (System*)parent(); }

      virtual QRectF bbox() const;
      virtual void setHeight(qreal);
      virtual double width() const;

      virtual void draw(Painter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void layout();

      virtual bool isEditable() const { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, int, int, Qt::KeyboardModifiers, const QString&);
      virtual void endEdit();
      virtual void editDrag(const EditData&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int grip) const;

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      };

#endif

