//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BEAM_H__
#define __BEAM_H__

#include "element.h"
#include "durationtype.h"
#include "spanner.h"

class ChordRest;
class MuseScoreView;
class Chord;
class Painter;

//---------------------------------------------------------
//   BeamHint
//    beam hint for autobeamer
//---------------------------------------------------------

struct BeamHint {
      Fraction noteLen;
      Fraction prevNoteLen; // zero = all notes
      Fraction timeSig;     // valid for this timesig; zero = valid for all
      Fraction pos;

      BeamHint(Fraction sig, Fraction p, Fraction len, Fraction prevLen)
         : noteLen(len), prevNoteLen(prevLen), timeSig(sig), pos(p) {}
      };

//
// user offsets for beam or beam fragment
//
struct BeamFragment {
      QPointF p1[2];
      QPointF p2[2];
      };

//---------------------------------------------------------
//   Beam
//    Balken
//---------------------------------------------------------

class Beam : public Element {
      QList<ChordRest*> _elements;
      QList<QLineF*> beamSegments;
      Direction _direction;
      int _up;                // -1: unknown  0: down   1: up
      qreal _grow1;          // define "feather" beams
      qreal _grow2;

      QList<BeamFragment*> fragments;     // beam splits across systems

      bool _userModified[2];    // 0: auto/down  1: up

      mutable int _id;          // used in read()/write()

      int minMove;              // set in layout1()
      int maxMove;
      Chord* c1;
      Chord* c2;
      bool isGrace;
      bool cross;
      Duration maxDuration;
      qreal slope;
      int cut;

      int editFragment;       // valid in edit mode

      void layout2(QList<ChordRest*>, SpannerSegmentType, int frag);

   public:
      Beam(Score* s);
      Beam(const Beam&);
      ~Beam();
      virtual Beam* clone() const         { return new Beam(*this); }
      virtual ElementType type() const    { return BEAM; }
      virtual QPointF pagePos() const;  ///< position in page coordinates

      virtual bool isEditable() const { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);

      virtual void toDefault();

      System* system() const { return (System*)parent(); }

      void layout1();
      void layout();

      const QList<ChordRest*>& elements() { return _elements;  }
      void clear()                        { _elements.clear(); }
      bool isEmpty() const                { return _elements.isEmpty(); }
      virtual void add(ChordRest* a);
      virtual void remove(ChordRest* a);
      void writeMusicXml(Xml& xml, ChordRest* cr) const;
      virtual void move(qreal, qreal);
      virtual void draw(Painter*) const;
      int up() const                      { return _up; }
      void setUp(int v)                   { _up = v; }
      void setId(int i) const             { _id = i; }
      int id() const                      { return _id; }
      bool isUp() const                   { return _up; }

      void setBeamDirection(Direction d);
      virtual QPainterPath shape() const;
      virtual bool contains(const QPointF& p) const;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);

      qreal grow1() const      { return _grow1; }
      qreal grow2() const      { return _grow2; }
      void setGrow1(qreal val) { _grow1 = val; }
      void setGrow2(qreal val) { _grow2 = val; }
      };

extern bool endBeam(const Fraction&, ChordRest* cr, ChordRest* prevCr);
#endif

