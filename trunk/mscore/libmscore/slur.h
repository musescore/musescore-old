//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#ifndef __SLUR_H__
#define __SLUR_H__

#include <QtCore/QQueue>

#include "globals.h"
#include "spanner.h"
#include "painter.h"

class Note;
class System;
class SlurTie;
class Score;
class Painter;

struct UP {
      QPointF p;            // layout position relative to pos()
      QPointF off;          // user offset in spatium units
      };

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

class SlurSegment : public SpannerSegment {
      struct UP ups[4];
      PainterPath path;
      qreal bow;
      System* _system;

      void updatePath();

   public:
      SlurSegment(Score*);
      SlurSegment(const SlurSegment&);
      virtual SlurSegment* clone() const { return new SlurSegment(*this); }
      virtual ElementType type() const   { return SLUR_SEGMENT; }

      void layout(const QPointF& p1, const QPointF& p2, qreal bow);
//      virtual QPainterPath shape() const;
      virtual void draw(Painter*) const;

      virtual void move(qreal xd, qreal yd) { move(QPointF(xd, yd)); }
      virtual void move(const QPointF& s);


      SlurTie* slurTie() const              { return (SlurTie*)parent(); }

      void read(XmlReader*);
      void setSystem(System* s)             { _system = s;       }
      void setSlurOffset(int i, const QPointF& val) { ups[i].off = val;  }
      QPointF slurOffset(int i) const               { return ups[i].off; }
      };

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

class SlurTie : public Spanner {
      int _lineType;          // 0 = solid, 1 = dotted

   protected:
      qreal _len;
      bool up;
      QQueue<SlurSegment*> delSegments;   // "deleted" segments
      Direction _slurDirection;
      qreal firstNoteRestSegmentX(System* system);

   public:
      SlurTie(Score*);
      SlurTie(const SlurTie&);
      ~SlurTie();

      virtual ElementType type() const = 0;
      bool isUp() const                  { return up; }
      void setUp(bool val)               { up = val;  }
      Direction slurDirection() const    { return _slurDirection; }
      void setSlurDirection(Direction d) { _slurDirection = d; }

      virtual void layout2(const QPointF, int, struct UP&)  {}
      virtual bool contains(const QPointF&) const     { return false; }  // not selectable

      bool readProperties(XmlReader*);
      QPointF slurPos(Element*, System*& s);

      void setLen(qreal v)               { _len = v; }
      int lineType() const                { return _lineType; }
      void setLineType(int val)           { _lineType = val;  }
      SlurSegment* frontSegment() const   { return (SlurSegment*)spannerSegments().front(); }
      SlurSegment* backSegment() const    { return (SlurSegment*)spannerSegments().back();  }
      SlurSegment* takeFirstSegment()     { return (SlurSegment*)spannerSegments().takeFirst(); }
      SlurSegment* takeLastSegment()      { return (SlurSegment*)spannerSegments().takeLast(); }
      SlurSegment* segmentAt(int n) const { return (SlurSegment*)spannerSegments().at(n); }
      };

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

class Slur : public SlurTie {
      int _track2;      // obsolete used temporarily for reading old version

   public:
      Slur(Score*);
      ~Slur();
      virtual Slur* clone() const      { return new Slur(*this); }
      virtual ElementType type() const { return SLUR; }
      virtual void read(XmlReader*);
      virtual void layout();
      virtual QRectF bbox() const;
      virtual void setTrack(int val);

      int track2() const      { return _track2; }
      int staffIdx2() const   { return _track2 / VOICES; }
      void setTrack2(int val) { _track2 = val; }

      // obsolete:
      void setStart(int /*tick*/, int /*track*/) {}
      void setEnd(int /*tick*/,   int /*track*/) {}
      };

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

class Tie : public SlurTie {
   public:
      Tie(Score*);
      virtual Tie* clone() const          { return new Tie(*this);        }
      virtual ElementType type() const    { return TIE;                   }
      void setStartNote(Note* note);
      void setEndNote(Note* note)         { setEndElement((Element*)note); }
      Note* startNote() const             { return (Note*)startElement(); }
      Note* endNote() const               { return (Note*)endElement();   }
      virtual void read(XmlReader*);
      virtual void layout();
      };

#endif

