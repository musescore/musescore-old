//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: ottava.h,v 1.2 2006/03/02 17:08:40 wschweer Exp $
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

#ifndef __OTTAVA_H__
#define __OTTAVA_H__

#include "line.h"

class Ottava;

//---------------------------------------------------------
//   OttavaSegment
//---------------------------------------------------------

class OttavaSegment : public LineSegment {
   protected:

   public:
      OttavaSegment(Score* s) : LineSegment(s) {}
      virtual ElementType type() const { return OTTAVA_SEGMENT; }
      virtual OttavaSegment* clone() const { return new OttavaSegment(*this); }
      virtual void draw(QPainter&);
      virtual QRectF bbox() const;
      Ottava* ottava() const { return (Ottava*)parent(); }
      };

//---------------------------------------------------------
//   Ottava
//    brackets
//---------------------------------------------------------

class Ottava : public SLine {

   protected:
      QString text;
      int shift;                    ///< pitch shift in semitones
      mutable qreal textHeight;     ///< cached value

      friend class OttavaSegment;

   public:
      Ottava(Score* s);
      virtual Ottava* clone() const    { return new Ottava(*this); }
      virtual ElementType type() const { return OTTAVA; }
      virtual void layout(ScoreLayout*);
      virtual void setSubtype(int val);
      virtual void write(Xml&) const;
      virtual void read(QDomNode);
      virtual LineSegment* createSegment();
      };

#endif

