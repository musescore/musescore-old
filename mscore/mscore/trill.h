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

#ifndef __TRILL_H__
#define __TRILL_H__

#include "line.h"

class Trill;

//---------------------------------------------------------
//   TrillSegment
//---------------------------------------------------------

class TrillSegment : public LineSegment {
   protected:
   public:
      TrillSegment(Score* s) : LineSegment(s) {}
      Trill* trill() const                { return (Trill*)parent(); }
      virtual ElementType type() const    { return TRILL_SEGMENT; }
      virtual TrillSegment* clone() const { return new TrillSegment(*this); }
      virtual void draw(QPainter&) const;
      virtual QRectF bbox() const;
      };

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

class Trill : public SLine {
   public:
      Trill(Score* s);
      virtual Trill* clone() const { return new Trill(*this); }
      virtual ElementType type() const { return TRILL; }

      virtual void layout();
      virtual LineSegment* createLineSegment();
      };

#endif

