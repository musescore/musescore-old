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

class Chord;

// Arpeggio types
enum { ARP_NORMAL, ARP_UP, ARP_DOWN, ARP_BRACKET};

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

class Arpeggio : public Element {
      Spatium _userLen1;
      Spatium _userLen2;
      double _height;
      int _span;              // spanning staves

      virtual QLineF dragAnchor() const;
      virtual QPointF gripAnchor(int) const;

   public:
      Arpeggio(Score* s);
      virtual Arpeggio* clone() const  { return new Arpeggio(*this); }
      virtual ElementType type() const { return ARPEGGIO; }
      Chord* chord() const             { return (Chord*)parent(); }
      virtual QRectF bbox() const;
      virtual void draw(QPainter&, ScoreView*) const;
      virtual bool isEditable() { return true; }
      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual bool edit(ScoreView*, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&);

      void read(QDomElement e);
      void write(Xml& xml) const;
      int span() const      { return _span; }
      void setSpan(int val) { _span = val; }

      void setHeight(double);
      };

#endif

