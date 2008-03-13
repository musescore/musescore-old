//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: hairpin.h,v 1.13 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __HAIRPIN_H__

#include "line.h"

class Score;
class Hairpin;

//---------------------------------------------------------
//   HairpinSegment
//---------------------------------------------------------

class HairpinSegment : public LineSegment {
   protected:
   public:
      HairpinSegment(Score* s) : LineSegment(s) {}
      Hairpin* hairpin() const              { return (Hairpin*)parent(); }
      virtual HairpinSegment* clone() const { return new HairpinSegment(*this); }
      virtual ElementType type() const      { return HAIRPIN_SEGMENT; }
      virtual void draw(QPainter&) const;
      virtual QRectF bbox() const;
      };

//---------------------------------------------------------
//   Hairpin
//
//    subtype: 0 = crescendo,  1 = decrescendo
//---------------------------------------------------------

class Hairpin : public SLine {
   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const { return new Hairpin(*this); }
      virtual ElementType type() const { return HAIRPIN; }
      virtual void layout(ScoreLayout*);
      virtual LineSegment* createLineSegment();
      };

#define __HAIRPIN_H__

#endif

