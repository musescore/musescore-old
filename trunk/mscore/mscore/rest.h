//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: rest.h,v 1.3 2006/03/02 17:08:41 wschweer Exp $
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

#ifndef __REST_H__
#define __REST_H__

#include "chordrest.h"

//---------------------------------------------------------
//   Rest
//---------------------------------------------------------

class Rest : public ChordRest {
      Q_DECLARE_TR_FUNCTIONS(Rest)

      int _staffMove;         // -1, 0, +1, used for crossbeaming
      int _sym;

      int dotline;            // depends on rest symbol
      double _mmWidth;        // width of multi measure rest

      virtual bool isMovable() const { return true; }
      virtual QRectF drag(const QPointF& s);
      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void setUserOffset(double x, double y);

   public:
      Rest(Score*);
      Rest(Score*, int tick, int len);
      virtual Rest* clone() const { return new Rest(*this); }
      virtual ElementType type() const { return REST; }

      virtual void draw(QPainter&) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement, const QList<Tuplet*>&, const QList<Beam*>&);
      virtual void add(Element*);
      virtual void remove(Element*);

      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);
      virtual void layout(ScoreLayout*);
      virtual QRectF bbox() const;
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);

      virtual int staffMove() const { return _staffMove; }
      void setStaffMove(int val)    { _staffMove = val; }

      virtual void setDuration(Duration t);
      void setMMWidth(double val)   { _mmWidth = val; }
      double mmWidth() const        { return _mmWidth; }
      };

#endif

