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
class Painter;

struct UP {
      QPointF p;            // layout position relative to pos()
      QPointF off;          // user offset in spatium units
      mutable QRectF r;   // "grips"
      QPointF pos() const { return p + off * _spatium; }
      };

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

class SlurSegment : public Element {
      struct UP ups[4];
      QPainterPath* path;

      SlurTie* slur;
      RubberBand* rb;
      qreal bow;

      int mode;         // 0-4  0 - normal

      virtual QRectF bbox() const;
      void updatePath();
      void updateGrips(QMatrix&);

   public:
      SlurSegment(SlurTie*);
      SlurSegment(const SlurSegment&);
      ~SlurSegment();
      virtual SlurSegment* clone() const { return new SlurSegment(*this); }
      virtual ElementType type() const { return SLUR_SEGMENT; }

      void layout(const QPointF& p1, const QPointF& p2, qreal bow);
      virtual void resetMode();

      virtual QPointF dragOff() const;

      virtual void draw1(Painter&);
      virtual bool startEdit(QMatrix&);
      virtual void endEdit();
      virtual bool startEditDrag(const QPointF&);
      virtual bool editDrag(QMatrix&, QPointF*, const QPointF&);
      virtual bool endEditDrag();
      virtual bool edit(QKeyEvent* ev);

      virtual void move(qreal xd, qreal yd) { move(QPointF(xd, yd)); }
      virtual void move(const QPointF& s);

      SlurTie* slurTie() const     { return slur; }
      void setSlurTie(SlurTie* st) { slur = st; }

      void write(Xml& xml, int no) const;
      void read(QDomNode);
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

      QPointF slurPos(int tick, Staff* staff, int voice, System*& s);

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
      virtual void nextSeg(const QPointF, int, struct UP&)  {}
      virtual void prevSeg(const QPointF, int, struct UP&)  {}
      virtual void setSelected(bool f);
      virtual bool contains(const QPointF&) const { return false; }  // not selectable

      ElementList* elements()         { return &segments;      }
      virtual void add(Element* s);
      virtual void remove(Element* s);

      void writeProperties(Xml& xml) const;
      bool readProperties(QDomNode);
      };

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

class Slur : public SlurTie {
      int _tick1, _tick2, _voice1, _voice2;
      Staff* _staff1;
      Staff* _staff2;

   public:
      Slur(Score*);
      ~Slur();
      virtual Slur* clone() const      { return new Slur(*this); }
      virtual ElementType type() const { return SLUR; }
      virtual void write(Xml& xml) const;
      virtual void read(Score*, QDomNode);
      virtual void layout();
      virtual void layout2(const QPointF, int, struct UP&);
      virtual void nextSeg(const QPointF, int, struct UP&);
      virtual void prevSeg(const QPointF, int, struct UP&);
      virtual QRectF bbox() const;

      void setStart(int t, Staff* staff, int voice);
      void setEnd(int t, Staff* staff, int voice);
      bool startsAt(int t, Staff* staff, int voice);
      bool endsAt(int t, Staff* staff, int voice);
      };

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

class Tie : public SlurTie {
      Note* _startNote; // parent
      Note* _endNote;

   public:
      Tie(Score*);
      virtual Tie* clone() const   { return new Tie(*this); }
      virtual ElementType type() const { return TIE; }
      void setStartNote(Note* note);
      void setEndNote(Note* note)   { _endNote = note; }
      Note* startNote() const       { return _startNote; }
      Note* endNote() const         { return _endNote; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      virtual void layout();
      };

#endif
