//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: slur.h,v 1.27 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __SLUR_H__
#define __SLUR_H__

#include "globals.h"
#include "element.h"

class Note;
class System;
class SlurTie;
class Score;

struct UP {
      QPointF p;            // layout position relative to pos()
      QPointF off;          // user offset in spatium units
      QPointF pos() const { return p + off * _spatium; }
      };

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

class SlurSegment : public Element {
      struct UP ups[4];
      QPainterPath path;

      SlurTie* slur;
      qreal bow;

      virtual QRectF bbox() const;
      void updatePath();
//      void setDropAnchor(Viewer* viewer);

   public:
      SlurSegment(SlurTie*);
      SlurSegment(const SlurSegment&);
      virtual SlurSegment* clone() const { return new SlurSegment(*this); }
      virtual ElementType type() const { return SLUR_SEGMENT; }

      void layout(ScoreLayout*, const QPointF& p1, const QPointF& p2, qreal bow);
      virtual QPainterPath shape() const;
      virtual void draw(QPainter&);

      virtual bool startEdit(const QPointF&);
      virtual void editDrag(int, const QPointF&, const QPointF&);
      virtual bool edit(int, QKeyEvent*);
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int grip);

      virtual void move(qreal xd, qreal yd) { move(QPointF(xd, yd)); }
      virtual void move(const QPointF& s);

      virtual bool genPropertyMenu(QMenu*) const;

      SlurTie* slurTie() const     { return slur; }
      void setSlurTie(SlurTie* st) { slur = st; }

      void write(Xml& xml, int no) const;
      void read(QDomElement);
      void dump() const;
      };

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

class SlurTie : public Element {
   protected:
      bool up;
      ElementList segments;
      Direction _slurDirection;

   public:
      SlurTie(Score*);
      SlurTie(const SlurTie&);
      ~SlurTie();

      virtual ElementType type() const = 0;
      bool isUp() const                  { return up; }
      void setUp(bool val)               { up = val;  }
      Direction slurDirection() const    { return _slurDirection; }
      void setSlurDirection(Direction d) { _slurDirection = d; }

      virtual void layout2(ScoreLayout*, const QPointF, int, struct UP&)  {}
      virtual void setSelected(bool f);
      virtual bool contains(const QPointF&) const { return false; }  // not selectable

      ElementList* elements()             { return &segments;      }
      virtual void add(Element* s);
      virtual void remove(Element* s);

      void writeProperties(Xml& xml) const;
      bool readProperties(QDomElement);
      QPointF slurPos(int tick, int track, System*& s);
      };

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

class Slur : public SlurTie {
      int _track1, _track2;
      int _tick1, _tick2;

   public:
      Slur(Score*);
      ~Slur();
      virtual Slur* clone() const      { return new Slur(*this); }
      virtual ElementType type() const { return SLUR; }
      virtual void write(Xml& xml) const;
      virtual void read(Score*, QDomElement);
      virtual void layout(ScoreLayout*);
      virtual void layout2(ScoreLayout*, const QPointF, int, struct UP&);
      virtual QRectF bbox() const;

      int tick1() const { return _tick1; }
      int tick2() const { return _tick2; }
      void setTick1(int val);
      void setTick2(int val);
      int track1() const { return _track1; }
      int track2() const { return _track2; }
      void setTrack1(int val) { _track1 = val; }
      void setTrack2(int val) { _track2 = val; }

      void setStart(int t, int track);
      void setEnd(int t,   int track);
      bool startsAt(int t, int track);
      bool endsAt(int t,   int track);
      };

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

class Tie : public SlurTie {
      Note* _startNote; // parent
      Note* _endNote;

   public:
      Tie(Score*);
      virtual Tie* clone() const    { return new Tie(*this); }
      virtual ElementType type() const { return TIE; }
      void setStartNote(Note* note);
      void setEndNote(Note* note)   { _endNote = note; }
      Note* startNote() const       { return _startNote; }
      Note* endNote() const         { return _endNote; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void layout(ScoreLayout*);
      };

#endif
