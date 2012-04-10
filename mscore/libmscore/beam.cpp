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

#include "beam.h"
#include "segment.h"
#include "score.h"
#include "chord.h"
#include "sig.h"
#include "style.h"
#include "note.h"
#include "tuplet.h"
#include "system.h"
#include "tremolo.h"
#include "measure.h"
#include "undo.h"
#include "staff.h"
#include "stafftype.h"
#include "stem.h"
#include "hook.h"
#include "mscore.h"
#include "icon.h"

//---------------------------------------------------------
//   BeamFragment
//    user offsets for beam or beam fragment
//---------------------------------------------------------

struct BeamFragment {
      QPointF p1[2];    // saved position for AUTO or DOWN
      QPointF p2[2];    // saved position for UP
      };

//---------------------------------------------------------
//   propertyList
//---------------------------------------------------------

static bool defaultDistribute         = false;
static Direction defaultBeamDirection = AUTO;
static qreal defaultGrow              = 1.0;

Property<Beam> Beam::propertyList[] = {
      { P_STEM_DIRECTION,  &Beam::pBeamDirection, &defaultBeamDirection },
      { P_DISTRIBUTE,      &Beam::pDistribute,    &defaultDistribute    },
      { P_GROW_LEFT,       &Beam::pGrowLeft,      &defaultGrow          },
      { P_GROW_RIGHT,      &Beam::pGrowRight,     &defaultGrow          },
      { P_END, 0, 0 }
      };

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

//---------------------------------------------------------
//   endBeam
//---------------------------------------------------------

static BeamHint endBeamList[] = {
      // in 2 2 time
      //  end beams each 1 2 note

      BeamHint(Fraction(2,2), Fraction(1,2), Fraction(0,0), Fraction(0,0)),

      // in 3 2 time:
      //   end beams each 1 2 note
      //   end beams with 16th notes each 1 4 note
      //   end beams with 32th notes each 1 8 note

      //       noteLen   timesig  position

      BeamHint(Fraction(3,2), Fraction(1,2), Fraction(0,0), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(2,2), Fraction(0,0), Fraction(0,0)),

      BeamHint(Fraction(3,2), Fraction(1,4), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,2), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(3,4), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,1), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(5,4), Fraction(1,16), Fraction(0,0)),

      BeamHint(Fraction(3,2), Fraction(1,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,4), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(3,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,2), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(5,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(3,4), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(7,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,1), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(9,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(5,4), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(11,8),Fraction(1,32), Fraction(0,0)),

      BeamHint(Fraction(2,4), Fraction(0,0), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(2,4), Fraction(1,4), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(2,4), Fraction(1,9), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(2,4), Fraction(3,8), Fraction(1,32), Fraction(0,0)),

      BeamHint(Fraction(3,4), Fraction(1,4), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,2), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,4), Fraction(1,16),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,2), Fraction(1,16),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,4), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(3,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,2), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(5,8), Fraction(1,32),  Fraction(0,0)),

      BeamHint(Fraction(12,16), Fraction(3,8),  Fraction(0, 0), Fraction(0,0)),
      BeamHint(Fraction(12,16), Fraction(3,16), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(12,16), Fraction(6,16), Fraction(1,8),  Fraction(0,0)),
      BeamHint(Fraction(12,16), Fraction(9,16), Fraction(1,8),  Fraction(0,0)),
      BeamHint(Fraction(12,16), Fraction(9,16), Fraction(1,16), Fraction(0,0)),

      BeamHint(Fraction(4,4), Fraction(1,2), Fraction(0,0),   Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(1,4), Fraction(1,12),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(3,4), Fraction(1,12),  Fraction(0,0)),

      BeamHint(Fraction(4,4), Fraction(1,4), Fraction(1,8),  Fraction(1,16)),  // ws
      BeamHint(Fraction(4,4), Fraction(2,4), Fraction(1,8),  Fraction(1,16)),  // ws
      BeamHint(Fraction(4,4), Fraction(3,4), Fraction(1,8),  Fraction(1,16)),  // ws

      BeamHint(Fraction(4,4), Fraction(1,4), Fraction(1,16),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(3,4), Fraction(1,16),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(1,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(1,4), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(3,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(5,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(3,4), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(7,8), Fraction(1,32),  Fraction(0,0)),

      BeamHint(Fraction(5,4), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(6,4), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(3,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(4,8), Fraction(0,0), Fraction(0,0),   Fraction(0,0)),
      BeamHint(Fraction(4,8), Fraction(1,4), Fraction(0,0),   Fraction(0,0)),
      BeamHint(Fraction(4,8), Fraction(1,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,8), Fraction(3,8), Fraction(1,32),  Fraction(0,0)),

      BeamHint(Fraction(6,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(9,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(9,8), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(12,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(12,8), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(12,8), Fraction(9,8), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(15,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(15,8), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(15,8), Fraction(9,8), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(15,8), Fraction(6,8), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(4,16), Fraction(0,0), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(4,16), Fraction(1,8), Fraction(0,0),  Fraction(0,0))
      };

//---------------------------------------------------------
//   endBeam
//    return true if beam should be ended
//---------------------------------------------------------

bool endBeam(const Fraction& ts, ChordRest* cr, ChordRest* prevCr)
      {
      int p = cr->tick() - cr->measure()->tick();
      if (cr->tuplet() && !cr->tuplet()->elements().isEmpty()) {
            if (cr->tuplet()->elements().front() == cr)     // end beam at tuplet
                  return true;
            return false;
            }
      Fraction l  = cr->duration();
      Fraction pl = prevCr ? prevCr->duration() : Fraction(0,1);
      for (unsigned i = 0; i < sizeof(endBeamList)/sizeof(*endBeamList); ++i) {
            const BeamHint& h = endBeamList[i];
            if (!h.timeSig.isZero() && (!h.timeSig.identical(ts)))
                  continue;
            if (!h.noteLen.isZero() && (h.noteLen != l))
                  continue;
            if (!h.prevNoteLen.isZero() && (h.prevNoteLen != pl))
                  continue;
            if (!h.pos.isZero()) {
                  int pos = h.pos.ticks();
                  if (pos != p)
                        continue;
                  }
            else {            // if (h.pos.numerator() == 0) {   // stop on every beat
                  int len = (4 * MScore::division) / h.timeSig.denominator();
                  if (p % len) {
                        continue;
                        }
                  }
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_SELECTABLE);
      _direction       = AUTO;
      _up              = -1;
      _distribute      = false;
      _userModified[0] = false;
      _userModified[1] = false;
      _grow1           = 1.0;
      _grow2           = 1.0;
      editFragment     = 0;
      }

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(const Beam& b)
   : Element(b)
      {
      _elements     = b._elements;
      foreach(QLineF* bs, b.beamSegments)
            beamSegments.append(new QLineF(*bs));
      _direction       = b._direction;
      _up              = b._up;
      _userModified[0] = b._userModified[0];
      _userModified[1] = b._userModified[1];
      _grow1           = b._grow1;
      _grow2           = b._grow2;
      foreach(BeamFragment* f, b.fragments)
            fragments.append(new BeamFragment(*f));
      minMove          = b.minMove;
      maxMove          = b.maxMove;
      c1               = b.c1;
      c2               = b.c2;
      isGrace          = b.isGrace;
      cross            = b.cross;
      maxDuration      = b.maxDuration;
      slope            = b.slope;
      }

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::~Beam()
      {
      //
      // delete all references from chords
      //
      foreach(ChordRest* cr, _elements)
            cr->setBeam(0);
      qDeleteAll(beamSegments);
      qDeleteAll(fragments);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Beam::pagePos() const
      {
      System* system = static_cast<System*>(parent());
      if (system == 0)
            return pos();
      qreal yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Beam::add(ChordRest* a)
      {
      a->setBeam(this);
      if (!_elements.contains(a)) {
            //
            // insert element in same order as it appears
            // in the score
            //
            if (a->segment() && !_elements.isEmpty()) {
                  for (int i = 0; i < _elements.size(); ++i) {
                        Segment* s = _elements[i]->segment();
                        if ((s->tick() > a->segment()->tick())
                           || ((s->tick() == a->segment()->tick()) && (a->segment()->next(SegChordRest) == s))
                           )  {
                              _elements.insert(i, a);
                              return;
                              }
                        }
                  }
            _elements.append(a);
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Beam::remove(ChordRest* a)
      {
      if (!_elements.removeOne(a))
            qDebug("Beam::remove(): cannot find ChordRest");
      a->setBeam(0);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Beam::draw(QPainter* painter) const
      {
      if (staff()->useTablature()) {
            if (staff()->staffType()->slashStyle())
                  return;
            }
      painter->setBrush(QBrush(curColor()));
      painter->setPen(Qt::NoPen);
      qreal lw2 = point(score()->styleS(ST_beamWidth)) * .5 * mag();
      foreach (const QLineF* bs, beamSegments) {
            QPolygonF pg;
               pg << QPointF(bs->x1(), bs->y1()-lw2)
                  << QPointF(bs->x2(), bs->y2()-lw2)
                  << QPointF(bs->x2(), bs->y2()+lw2)
                  << QPointF(bs->x1(), bs->y1()+lw2);
            painter->drawPolygon(pg, Qt::OddEvenFill);
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Beam::move(qreal x, qreal y)
      {
      Element::move(x, y);
      foreach (QLineF* bs, beamSegments)
            bs->translate(x, y);
      }

//---------------------------------------------------------
//   writeMusicXml
//---------------------------------------------------------

// needed only for dump beam contents
// #include "rest.h"

void Beam::writeMusicXml(Xml& xml, ChordRest* cr) const
      {
/*
      qDebug("Beam::writeMusicXml(cr=%p)\n", cr);
      // dump beam contents
      foreach(ChordRest* crst, _elements) {
            if (crst->type() == CHORD) {
                  Chord* c = static_cast<Chord*>(crst);
                  qDebug(" chord %p tick=%d durtype=%d beams=%d\n", c, c->tick(), c->duration().type(), c->beams());
                  }
            else if (crst->type() == REST) {
                  Rest* r = static_cast<Rest*>(crst);
                  qDebug(" rest %p tick=%d durtype=%d beams=%d\n", r, r->tick(), r->duration().type(), r->beams());
                  }
            else {
                  qDebug(" type=%d %p tick=%d\n", crst->type(), crst, crst->tick());
                  }
            }
      // end dump beam contents
*/
      int idx = _elements.indexOf(cr);
      if (idx == -1) {
            qDebug("Beam::writeMusicXml(): cannot find ChordRest\n");
            return;
            }
      int blp = -1; // beam level previous chord
      int blc = -1; // beam level current chord
      int bln = -1; // beam level next chord
      // find beam level previous chord
      for (int i = idx - 1; blp == -1 && i >= 0; --i) {
            ChordRest* crst = _elements[i];
            if (crst->type() == CHORD)
                  blp = (static_cast<Chord*>(crst))->beams();
            }
      // find beam level current chord
      if (cr->type() == CHORD)
            blc = (static_cast<Chord*>(cr))->beams();
      // find beam level next chord
      for (int i = idx + 1; bln == -1 && i < _elements.size(); ++i) {
            ChordRest* crst = _elements[i];
            if (crst->type() == CHORD)
                  bln = (static_cast<Chord*>(crst))->beams();
            }
//      qDebug(" blp=%d blc=%d bln=%d\n", blp, blc, bln);
      for (int i = 1; i <= blc; ++i) {
            QString s;
            if (blp < i && bln >= i) s = "begin";
            else if (blp < i && bln < i) {
                  if (bln > 0) s = "forward hook";
                  else if (blp > 0) s = "backward hook";
                  }
            else if (blp >= i && bln < i) s = "end";
            else if (blp >= i && bln >= i) s = "continue";
            if (s != "")
                  xml.tag(QString("beam number=\"%1\"").arg(i), s);
            }
      }
#if 0
//---------------------------------------------------------
//   crLessThan
//---------------------------------------------------------

static bool crLessThan(const ChordRest* cr1, const ChordRest* cr2)
      {
      return cr1->segment()->tick() <= cr2->segment()->tick();
      }
#endif

//---------------------------------------------------------
//   twoBeamedNotes
//    calculate stem direction of two beamed notes
//    return true if two beamed notes found
//---------------------------------------------------------

bool Beam::twoBeamedNotes()
      {
      if ((_elements.size() != 2)
         || (_elements[0]->type() != CHORD)
         || _elements[1]->type() != CHORD) {
            return false;
            }
      const Chord* c1 = static_cast<const Chord*>(_elements[0]);
      const Chord* c2 = static_cast<const Chord*>(_elements[1]);
      if (c1->notes().size() != 1 || c2->notes().size() != 1)
            return false;
      int dist1 = c1->upNote()->line() - 4;
      int dist2 = c2->upNote()->line() - 4;
      if (qAbs(dist1) == qAbs(dist2)) {
            if (dist1 != 0) {
                  _up = dist1 > 0;
                  }
            else {
                  _up = false;
                  Segment* s = c1->segment();
                  s = s->prev1(SegChordRest);
                  if (s && s->element(c1->track())) {
                        Chord* c = static_cast<Chord*>(s->element(c1->track()));
                        if (c->beam())
                              _up = c->beam()->up();
                        }
                  }
            }
      else if (qAbs(dist1) > qAbs(dist2))
            _up = dist1 > 0;
      else
            _up = dist2 > 0;
      return true;
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Beam::layout1()
      {
      //delete old segments
      foreach(QLineF* i, beamSegments)
            delete i;
      beamSegments.clear();

      maxDuration.setType(TDuration::V_INVALID);
      c1 = 0;
      c2 = 0;

      if (staff()->useTablature()) {
            //TABULATURES: all beams (and related chords) are UP at slope 0
            _up = 1;
            cross = isGrace = false;
            slope = 0.0;
            foreach(ChordRest* cr, _elements) {
                  if (cr->type() == CHORD) {
                        // set members maxDuration, c1, c2
                        if (!maxDuration.isValid() || (maxDuration < cr->durationType()))
                              maxDuration = cr->durationType();
                        c2 = static_cast<Chord*>(cr);
                        if (c2->noteType() != NOTE_NORMAL)
                              isGrace = true;
                        if (c1 == 0)
                              c1 = c2;
                        }
                  }
            }
      else {
            //PITCHED STAVES
            minMove = 1000;
            maxMove = -1000;
            isGrace = false;

            int upCount = 0;
            int mUp     = 0;
            int mDown   = 0;
            foreach(ChordRest* cr, _elements) {
                  if (cr->type() == CHORD) {
                        c2 = static_cast<Chord*>(cr);
                        upCount += c2->up() ? 1 : -1;
                        if (c2->noteType() != NOTE_NORMAL)
                              isGrace = true;
                        if (c1 == 0)
                              c1 = c2;
                        int i = c2->staffMove();
                        if (i < minMove)
                              minMove = i;
                        if (i > maxMove)
                              maxMove = i;
                        int line = c2->upNote()->line();
                        if ((line - 4) > mUp)
                              mUp = line - 4;
                        line = c2->downNote()->line();
                        if (4 - line > mDown)
                              mDown = 4 - line;
                        }
                  if (!maxDuration.isValid() || (maxDuration < cr->durationType()))
                        maxDuration = cr->durationType();
                  }
            //
            // determine beam stem direction
            //
            if (_direction != AUTO)
                  _up = _direction == UP;
            else {
                  ChordRest* cr = _elements[0];
                  Measure* m = cr->measure();
                  if (m->hasVoices(cr->staffIdx())) {
                        switch(cr->voice()) {
                              case 0:  _up = (score()->style(ST_stemDir1).toDirection() == UP); break;
                              case 1:  _up = (score()->style(ST_stemDir2).toDirection() == UP); break;
                              case 2:  _up = (score()->style(ST_stemDir3).toDirection() == UP); break;
                              case 3:  _up = (score()->style(ST_stemDir4).toDirection() == UP); break;
                              }
                        }
                  else if (!twoBeamedNotes()) {
                        if (upCount == 0) {
                              // highest or lowest note determines stem direction
                              // down-stems is preferred if equal
                              _up = mUp > mDown;
                              }
                        else {
                              // the number of notes above/below the middle line
                              // determines stem direction
                              _up = upCount > 0;
                              }
                        }
                  }

            cross   = minMove < maxMove;
            int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
            slope   = 0.0;

            if (cross || _userModified[idx]) {
                  //
                  // guess stem direction for every chord
                  //
                  foreach(ChordRest* cr, _elements) {
                        if (cr->type() != CHORD)
                              continue;
                        Chord* c  = static_cast<Chord*>(cr);
                        int move = c->staffMove();
                        if (move == 0)
                              c->setUp(maxMove ? false : true);
                        else if (move > 0)
                              c->setUp(true);
                        else if (move < 0)
                              c->setUp(false);
                        }
                  _up = -1;
                  }
            else {
                  foreach(ChordRest* cr, _elements)
                        cr->setUp(_up);
                  }
            }     // end of if/else(tablature)
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Beam::layout()
      {
      if (_elements.isEmpty() || !c1 || !c2) {
            qDebug("Beam::layout: no notes\n");
            return;
            }
      setParent(_elements.front()->measure()->system());

      QList<ChordRest*> crl;
      System* system = c1->measure()->system();

      int n = 0;
      foreach(ChordRest* cr, _elements) {
            if (cr->measure()->system() != system) {
                  SpannerSegmentType st;
                  if (n == 0)
                        st = SEGMENT_BEGIN;
                  else
                        st = SEGMENT_MIDDLE;
                  ++n;
                  if (fragments.size() < n)
                        fragments.append(new BeamFragment);
                  layout2(crl, st, n-1);
                  crl.clear();
                  system = cr->measure()->system();
                  }
            crl.append(cr);
            }
      if (!crl.isEmpty()) {
            SpannerSegmentType st;
            if (n == 0)
                  st = SEGMENT_SINGLE;
            else
                  st = SEGMENT_END;
            if (fragments.size() < (n+1))
                  fragments.append(new BeamFragment);
            layout2(crl, st, n);
            }

      setbbox(QRectF());
      qreal lw2 = point(score()->styleS(ST_beamWidth)) * .5 * mag();
      foreach(const QLineF* bs, beamSegments) {
            QPolygonF a(4);
            a[0] = QPointF(bs->x1(), bs->y1()-lw2);
            a[1] = QPointF(bs->x2(), bs->y2()-lw2);
            a[2] = QPointF(bs->x2(), bs->y2()+lw2);
            a[3] = QPointF(bs->x1(), bs->y1()+lw2);
            addbbox(a.boundingRect());
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath Beam::shape() const
      {
      QPainterPath pp;
      qreal lw2 = point(score()->styleS(ST_beamWidth)) * .5 * mag();
      foreach(const QLineF* bs, beamSegments) {
            QPolygonF a(5);
            a[0] = QPointF(bs->x1(), bs->y1()-lw2);
            a[1] = QPointF(bs->x2(), bs->y2()-lw2);
            a[2] = QPointF(bs->x2(), bs->y2()+lw2);
            a[3] = QPointF(bs->x1(), bs->y1()+lw2);
            a[4] = QPointF(bs->x1(), bs->y1()-lw2);
            pp.addPolygon(a);
            }
      return pp;
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Beam::contains(const QPointF& p) const
      {
      return shape().contains(p - pagePos());
      }

//---------------------------------------------------------
//   alignBeam
//---------------------------------------------------------

#if 0
static qreal alignBeam(int line, qreal y, qreal _spatium, bool _up)
      {
      qreal _spatium2 = _spatium * .5;
      qreal _spatium4 = _spatium * .25;
      if (_up)
            _spatium4 = -_spatium4;
      int n = lrint(y / _spatium2);
      if (line % 2)
            y = _spatium2 * n - _spatium4;
      else
            y = _spatium2 * n - 2*_spatium4;
      return y;
      }
#endif

//---------------------------------------------------------
//   absLimit
//---------------------------------------------------------

inline qreal absLimit(qreal val, qreal limit)
      {
      if (val > limit)
            return limit;
      if (val < -limit)
            return -limit;
      return val;
      }

//---------------------------------------------------------
//   noSlope
//---------------------------------------------------------

bool Beam::noSlope(const QList<ChordRest*>& crl, Chord* c1, Chord* c2)
      {
      int l1          = c1->line(_up);
      int l2          = c2->line(_up);
//      qreal _spatium  = spatium();
      //
      //    concave beams have a slope of 0.0
      //
      bool sameLine = true;

      slope = 0.0;
      if (crl.size() >= 3) {
            int l4 = crl[1]->line(_up);
            for (int i = 1; i < crl.size()-1; ++i) {
                  int l3 = crl[i]->line(_up);
                  if (l3 != l4)
                        sameLine = false;
                  if (_up) {
                        if (l3 < l1 && l3 < l2)
                              return true;
                        }
                  else {
                        if (l3 > l1 && l3 > l2)
                              return true;
                        }
                  }
            if (sameLine && (l1 == l4 || l2 == l4)) {
                  if (_up) {
                        if (l1 == l4 && l1 < l2)
                              return true;
                        if (l2 == l4 && l2 < l1)
                              return true;
                        }
                  else {
                        if (l1 == l4 && l1 > l2)
                              return true;
                        else if (l2 == l4 && l2 > l1)
                              return true;
                        }
                  }
            }
      qreal dx = c2->pagePos().x() - c1->pagePos().x();
      if (dx == 0.0)
            return true;
      return l1 == l2;
      }

//---------------------------------------------------------
//   computeSlope
//---------------------------------------------------------

void Beam::computeSlope(const QList<ChordRest*>& crl, Chord* c1, Chord* c2)
      {
      int l1          = c1->line(_up);
      int l2          = c2->line(_up);
      qreal _spatium  = spatium();
      //
      //    concave beams have a slope of 0.0
      //
      bool sameLine = true;

      slope = 0.0;
      if (crl.size() >= 3) {
            int l4 = crl[1]->line(_up);
            for (int i = 1; i < crl.size()-1; ++i) {
                  int l3 = crl[i]->line(_up);
                  if (l3 != l4)
                        sameLine = false;
                  if (_up) {
                        if (l3 < l1 && l3 < l2)
                              return;
                        }
                  else {
                        if (l3 > l1 && l3 > l2)
                              return;
                        }
                  }
            if (sameLine && (l1 == l4 || l2 == l4)) {
                  if (_up) {
                        if (l1 == l4 && l1 < l2)
                              return;
                        if (l2 == l4 && l2 < l1)
                              return;
                        }
                  else {
                        if (l1 == l4 && l1 > l2)
                              return;
                        else if (l2 == l4 && l2 > l1)
                              return;
                        }
                  }
            }

      qreal dx = c2->pagePos().x() - c1->pagePos().x();
      if (dx == 0.0)
            return;

      // qreal maxSlope = score()->style(ST_beamMaxSlope).toDouble();
      uint interval = qAbs(l2 - l1);
      static qreal slantTable[8] = {
            0.0, 0.25, .5, 1, 1.25, 1.25, 1.5, 1.75
            };
      qreal slant;
      if (interval < sizeof(slantTable)/sizeof(*slantTable))
            slant = slantTable[interval];
      else
            slant = 1.75;

      //
      // if notes are spaced very close together (<= 3.0 spaces)
      // use only a small slope
      //
      if ((dx / ((crl.size()-1) * _spatium)) <= 2.0) {
            slant = 0.25;
            }
      if (l2 < l1)
            slant *= -1;
      slope = (slant * _spatium) / dx;
      }

//---------------------------------------------------------
//   BeamMetric
//---------------------------------------------------------

struct Bm
      {
      char l;     // stem len   in 1/4 spatium units
      char s;     // beam slant in 1/4 spatium units
      Bm() : l(0), s(0) {}
      Bm(char a, char b) : l(a), s(b) {}
      static int key(int a, int b, int c) { return ((a & 0xff) << 16) | ((b & 0xff) << 8) | (c & 0xff); }
      };

static QHash<int, Bm> bMetrics;

//---------------------------------------------------------
//   initBeamMetrics
//---------------------------------------------------------

#define B(a,b,c,d,e) bMetrics[Bm::key(a, b, c)] = Bm(d, e);

static void initBeamMetrics()
      {
      // up  step1 step2 stemLen1 slant
      //                 (- up)   (- up)
      // =================================== C
      B(1,  10, 10, -12,  0);
      B(0,   3,  3,  11,  0);
      B(1,   3,  3, -11,  0);

      B(1,  10,  9, -12, -1);
      B(1,  10,  8, -12, -4);
      B(1,  10,  7, -12, -5);
      B(1,  10,  6, -15, -4);
      B(1,  10,  5, -16, -5);
      B(1,  10,  4, -20, -4);
      B(1,  10,  3, -20, -5);

      B(1,  10,  11, -12, 1);
      B(1,  10,  12, -13, 2);      // F
      B(1,  10,  13, -13, 2);
      B(1,  10,  14, -13, 2);
      B(1,  10,  15, -13, 2);

      B(1,  3,  4, -11, 1);
      B(1,  3,  5, -11, 2);
      B(1,  3,  6, -11, 4);
      B(1,  3,  7, -11, 5);
      B(1,  3,  8, -11, 5);
      B(1,  3,  9, -11, 5);
      B(1,  3, 10, -11, 5);

      B(0, -4, -3,  15, 1);
      B(0, -4, -2,  15, 2);
      B(0, -4, -1,  15, 2);
      B(0, -4,  0,  15, 5);
      B(0, -4,  1,  16, 5);
      B(0, -4,  2,  20, 4);
      B(0, -4,  3,  20, 5);

      B(0,  3,  4,  13, 1);
      B(0,  3,  5,  13, 2);
      B(0,  3,  6,  13, 2);
      B(0,  3,  7,  13, 4);
      B(0,  3,  8,  13, 6);

      B(0,  3,  2,  11, -1);
      B(0,  3,  1,  11, -2);
      B(0,  3,  0,  11, -5);
      B(0,  3, -1,  11, -5);
      B(0,  3, -2,  11, -5);
      B(0,  3, -3,  11, -5);
      B(0,  3, -4,  11, -5);

      // =================================== D
      B(1,  9,  9,  -13, 0);
      B(0,  2,  2,   12, 0);
      B(1,  2,  2,  -11, 0);

      B(1,  9,  8,  -13, -1);
      B(1,  9,  7,  -13, -2);
      B(1,  9,  6,  -13, -5);
      B(1,  9,  5,  -14, -5);
      B(1,  9,  4,  -16, -6);
      B(1,  9,  3,  -17, -5);
      B(1,  9,  2,  -17, -8);

      B(1,  9, 10,  -11, 1);
      B(1,  9, 11,  -11, 2);
      B(1,  9, 12,  -11, 2);
      B(1,  9, 13,  -11, 2);
      B(1,  9, 14,  -11, 2);
      B(1,  9, 15,  -11, 2);

      B(1,  2, 3,   -12, 1);
      B(1,  2, 4,   -12, 2);
      B(1,  2, 5,   -12, 4);
      B(1,  2, 6,   -12, 5);
      B(1,  2, 7,   -11, 5);
      B(1,  2, 8,   -12, 5);
      B(1,  2, 9,   -12, 8);

      B(0, -5,-4,   16, 2);
      B(0, -5,-3,   16, 2);
      B(0, -5,-2,   16, 2);
      B(0, -5,-1,   16, 2);
      B(0, -5, 0,   16, 4);
      B(0, -5, 1,   16, 5);
      B(0, -5, 2,   16, 5);

      B(0,  2, 3,   12, 1);
      B(0,  2, 4,   12, 4);
      B(0,  2, 5,   13, 4);  // F
      B(0,  2, 6,   15, 5);
      B(0,  2, 7,   13, 6);
      B(0,  2, 8,   16, 8);
      B(0,  2, 9,   16, 8);

      B(0,  2,  1,   12, -1);
      B(0,  2,  0,   12, -4);
      B(0,  2, -1,   12, -5);
      B(0,  2, -2,   12, -5);
      B(0,  2, -3,   12, -4);
      B(0,  2, -4,   12, -4);
      B(0,  2, -5,   12, -5);

      // =================================== E
      B(1, 8, 8,  -12, 0);
      B(0, 1, 1,   13, 0);
      B(1, 1, 1,   -9, 0);

      B(1, 8, 7, -12, -1);
      B(1, 8, 6, -12, -4);
      B(1, 8, 5, -12, -5);
      B(1, 8, 4, -15, -5);
      B(1, 8, 3, -16, -5);
      B(1, 8, 2, -17, -6);
      B(1, 8, 1, -19, -6);

      B(1, 15, 11, -21, -1);
      B(1, 15, 10, -21, -1);
      B(1, 15,  9, -21, -1);
      B(1, 15,  8, -21, -1);

      B(1,  1,  8, -11,  6);
      B(1,  1,  7, -11,  6);
      B(1,  1,  6, -12,  6);

      B(1,  8,  9, -12,  1);
      B(1,  8, 10, -12,  4);
      B(1,  8, 11, -12,  5);
      B(1,  8, 12, -12,  5);
      B(1,  8, 13, -12,  4);
      B(1,  8, 14, -12,  5);
      B(1,  8, 15, -12,  1);

      B(0,  1,  0, 11,  -1);
      B(0,  1, -1, 11,  -2);
      B(0,  1, -2, 11,  -5);
      B(0,  1, -3, 11,  -5);
      B(0,  1, -4, 11,  -5);
      B(0,  1, -5, 11,  -5);
      B(0,  1, -6, 11,  -5);

      B(0, 1, 2, 13, 1);
      B(0, 1, 3, 13, 2);
      B(0, 1, 4, 13, 5);
      B(0, 1, 5, 14, 5);
      B(0, 1, 6, 15, 5);
      B(0, 1, 7, 17, 5);
      B(0, 1, 8, 17, 8);

      B(0, -6, -2,  19, 2);
      B(0, -6, -1,  19, 4);
      B(0, -6,  0,  20, 4);
      B(0, -6,  1,  20, 5);

      B(0, 8, 3, 9,  -6);
      B(0, 8, 2, 12, -8);
      B(0, 8, 1, 12, -8);

      // =================================== F
      B(1, 7, 7,-13, 0);      //F
      B(0, 0, 0, 12, 0);
      B(0, 7, 7, 10, 0);

      B(1, 7, 6, -13, -1);
      B(1, 7, 5, -13, -2);
      B(1, 7, 4, -13, -5);
      B(1, 7, 3, -14, -5);
      B(1, 7, 2, -15, -6);
      B(1, 7, 1, -17, -6);
      B(1, 7, 0, -18, -8);

      B(1, 14, 10, -19, -2);
      B(1, 14,  9, -19, -2);
      B(1, 14,  8, -20, -4);
      B(1, 14,  7, -20, -5);

      B(1,  0,  5,  -9, 6);
      B(1,  0,  6, -12, 8);
      B(1,  0,  7, -12, 8);

      B(1, 7,  8, -11, 1);
      B(1, 7,  9, -11, 2);
      B(1, 7, 10, -11, 5);
      B(1, 7, 11, -11, 5);
      B(1, 7, 12, -11, 5);
      B(1, 7, 13, -11, 5);
      B(1, 7, 14, -11, 5);

      B(0, 0, -1, 12, -1);
      B(0, 0, -2, 12, -4);
      B(0, 0, -3, 12, -5);
      B(0, 0, -4, 12, -5);
      B(0, 0, -5, 12, -4);
      B(0, 0, -6, 12, -4);
      B(0, 0, -7, 12, -4);

      B(0, 0, 1, 12, 1);
      B(0, 0, 2, 12, 4);
      B(0, 0, 3, 12, 5);
      B(0, 0, 4, 15, 5);
      B(0, 0, 5, 16, 5);
      B(0, 0, 6, 17, 5);
      B(0, 0, 7, 19, 6);

      B(0, -7, -3, 21, 2);
      B(0, -7, -2, 21, 2);
      B(0, -7, -1, 21, 2);
      B(0, -7,  0, 22, 4);

      B(0, 7, 2, 12, -6);
      B(0, 7, 1, 11, -6);
      B(0, 7, 0, 11, -6);

      // =================================== G
      B(1,  6,  6, -12, 0);
      B(0, -1, -1,  13, 0);
      B(0,  6,  6,  11, 0);

      B(1, 6,  5, -12, -1);
      B(1, 6,  4, -12, -4);
      B(1, 6,  3, -13, -4);
      B(1, 6,  2, -15, -5);
      B(1, 6,  1, -13, -7);
      B(1, 6,  0, -16, -8);
      B(1, 6, -1, -16, -8);

      B(1, 13, 10, -17, -2);
      B(1, 13,  9, -17, -2);
      B(1, 13,  8, -18, -4);
      B(1, 13,  7, -18, -5);
      B(1, 13,  6, -21, -5);

      B(1, -1, 6, -10, 8);

      B(1, 6,  7, -12, 1);
      B(1, 6,  8, -12, 4);
      B(1, 6,  9, -12, 5);
      B(1, 6, 10, -12, 5);
      B(1, 6, 11, -12, 4);
      B(1, 6, 12, -12, 5);
      B(1, 6, 13, -12, 5);

      B(0, -1, -2, 11, -1);
      B(0, -1, -3, 11, -2);
      B(0, -1, -4, 11, -2);
      B(0, -1, -5, 11, -2);
      B(0, -1, -6, 11, -2);
      B(0, -1, -7, 11, -2);

      B(0, -1,  0, 13, 1);
      B(0, -1,  1, 13, 2);
      B(0, -1,  2, 13, 5);
      B(0, -1,  3, 14, 5);
      B(0, -1,  4, 17, 6);
      B(0, -1,  5, 18, 5);
      B(0, -1,  6, 18, 8);

      B(0,  6,  5, 12, -4);
      B(0,  6,  4, 12, -4);
      B(0,  6,  3, 12, -4);
      B(0,  6,  2, 12, -6);
      B(0,  6,  1, 11, -6);
      B(0,  6,  0, 12, -7);
      B(0,  6, -1, 12, -8);

      // =================================== A
      B(1,  5,  5, -11, 0);
      B(0, -2, -2,  12, 0);
      B(0,  5,  5,  11, 0);

      B(1,  5,  4, -13, -1);
      B(1,  5,  3, -13, -2);
      B(1,  5,  2, -14, -4);
      B(1,  5,  1, -14, -4);
      B(1,  5,  0, -13, -6);

      B(1, 12, 11, -15, -1);
      B(1, 12, 10, -15, -2);
      B(1, 12,  9, -15, -2);
      B(1, 12,  8, -15, -5);
      B(1, 12,  7, -16, -5);
      B(1, 12,  6, -20, -4);
      B(1, 12,  5, -20, -5);

      B(1,  5,  6, -11,  1);
      B(1,  5,  7, -11,  2);
      B(1,  5,  8, -11,  5);
      B(1,  5,  9, -11,  5);
      B(1,  5, 10, -11,  5);
      B(1,  5, 11, -11,  5);
      B(1,  5, 12, -11,  5);

      B(0, -2, -1, 12, 1);
      B(0, -2,  0, 12, 4);
      B(0, -2,  1, 12, 5);
      B(0, -2,  2, 15, 5);
      B(0, -2,  3, 16, 5);
      B(0, -2,  4, 20, 4);
      B(0, -2,  5, 20, 5);

      B(0, -2, -3, 12, -1);
      B(0, -2, -4, 13, -2);
      B(0, -2, -5, 13, -2);
      B(0, -2, -6, 13, -2);
      B(0, -2, -7, 13, -2);

      B(0,  5,  4, 11, -1);
      B(0,  5,  3, 11, -2);
      B(0,  5,  2, 11, -4);
      B(0,  5,  1, 11, -5);
      B(0,  5,  0, 11, -5);
      B(0,  5, -1, 11, -5);
      B(0,  5, -2, 11, -5);

      // =================================== B
      B(1,  4,  4, -12, 0);
      B(1, 11, 11, -13, 0);
      B(0,  4,  4,  12, 0);
      B(0, -3, -3,  13, 0);

      B(1, 11, 10, -13, -1);
      B(1, 11,  9, -13, -2);
      B(1, 11,  8, -13, -5);
      B(1, 11,  7, -14, -5);
      B(1, 11,  6, -18, -4);
      B(1, 11,  5, -18, -5);
      B(1, 11,  4, -21, -5);

      B(1,  4,  3, -12, -1);
      B(1,  4,  2, -12, -4);
      B(1,  4,  1, -14, -4);
      B(1,  4,  0, -16, -4);

      B(1, 11, 12, -14, 1);
      B(1, 11, 13, -14, 1);
      B(1, 11, 14, -14, 1);
      B(1, 11, 15, -15, 2);
      B(1, 11, 16, -15, 2);

      B(1,  4,  5, -12, 1);
      B(1,  4,  6, -12, 4);
      B(1,  4,  7, -12, 5);
      B(1,  4,  8, -12, 5);
      B(1,  4,  9, -13, 6);
      B(1,  4, 10, -12, 4);
      B(1,  4, 11, -12, 5);

      B(0,  4,  3, 12, -1);
      B(0,  4,  2, 12, -4);
      B(0,  4,  1, 12, -5);
      B(0,  4,  0, 12, -5);
      B(0,  4, -1, 13, -6);
      B(0,  4, -2, 12, -4);
      B(0,  4, -3, 12, -5);

      B(0,  4,  5, 12, 1);
      B(0,  4,  6, 12, 4);

      B(0, -3, -4, 14, -1);
      B(0, -3, -5, 14, -1);
      B(0, -3, -6, 14, -1);
      B(0, -3, -7, 15, -2);
      B(0, -3, -8, 15, -2);
      B(0, -3, -9, 15, -2);

      B(0, -3, -2, 13, 1);
      B(0, -3, -1, 13, 2);
      B(0, -3,  0, 13, 5);
      B(0, -3,  1, 14, 5);
      B(0, -3,  2, 18, 4);
      B(0, -3,  3, 18, 5);
      B(0, -3,  4, 21, 5);
      }

//---------------------------------------------------------
//   beamMetric1
//    table driven
//---------------------------------------------------------

static Bm beamMetric1(bool up, char l1, char l2)
      {
      static int initialized = false;
      if (!initialized) {
            initBeamMetrics();
            initialized = true;
            }
      return bMetrics[Bm::key(up, l1, l2)];
      }

//---------------------------------------------------------
//   beamMetric2
//---------------------------------------------------------

static Bm beamMetric2(bool up, char l1, char l2)
      {
      int d = qAbs(l2 - l1);
      int slant;
      int len = 12;
      switch(d) {
            case 0: slant = 0; break;
            case 1: slant = 1; break;
            case 2: slant = 2; break;
            case 3: slant = 4; break;
            case 4: slant = 5; break;
            case 5: slant = 6; break;
            case 6: slant = 6; break;
            case 7:
            default:
                    slant = 7; break;
            }
      if (up) {
            len = -len;
            if ((l1 > 10) && (l2 > 10)) {
                  slant = 2;
                  if (l1 > l2) {
                        slant = -2;
                        len =  9 - l1 * 2;
                        }
                  else {
                        slant = 2;
                        len =  7 - l1 * 2;
                        }
                  }
            else {
                  if (l1 > l2)
                        slant = -slant;
                  if (l2 < l1)
                        len -= (l1 - l2) * 2 + slant;
                  }
            }
      else {
            if (l1 < -2 && l2 < -2) {
                  if (l1 > l2) {
                        slant = -2;
                        len =  5 - l1 * 2;
                        }
                  else {
                        slant = 2;
                        len =  7 - l1 * 2;
                        }
                  }
            else {
                  if (l1 > l2)
                        slant = -slant;
                  if (l2 > l1)
                        len += (l2 - l1) * 2 - slant;
                  }
            }
      return Bm(len, slant);
      }

//---------------------------------------------------------
//   adjust
//    adjust stem len for notes between start-end
//---------------------------------------------------------

static int adjust(qreal _spatium4, Bm& bm, const QList<ChordRest*>& crl)
      {
      int n     = crl.size() - 1;
      Chord* c1 = static_cast<Chord*>(crl[0]);
      Chord* c2 = static_cast<Chord*>(crl[n]);
      bool _up  = c1->up();

      QPointF p1(c1->stemPos(_up, false));   // canvas coordinates
      QPointF p2(c2->stemPos(_up, false));   // canvas coordinates
      p1.ry()     += bm.l * _spatium4;
      qreal slope = (bm.s * _spatium4) / (p2.x() - p1.x());

      int ml = 10;
      if (_up) {
            for (int i = 1; i < n; ++i) {
                  Chord* c3   = static_cast<Chord*>(crl[i]);
                  QPointF p3(c3->stemPos(_up, false));   // canvas coordinates
                  qreal yDown = p3.y();
                  qreal yUp   = p1.y() + (p3.x() - p1.x()) * slope;

                  qreal l     = (yDown - yUp) / _spatium4;
                  ml          = qMin(ml, int(l + .5));
                  }
            }
      else {
            for (int i = 1; i < n; ++i) {
                  Chord* c3   = static_cast<Chord*>(crl[i]);
                  QPointF p3(c3->stemPos(_up, false));   // canvas coordinates
                  qreal yDown = p3.y();
                  qreal yUp   = p1.y() + (p3.x() - p1.x()) * slope;

                  qreal l     = (yUp - yDown) / _spatium4;
                  ml          = qMin(ml, int(l + .5));
                  }
            }
      if (ml < 10)
            return 10 - ml;
      return 0;
      }

//---------------------------------------------------------
//   adjust2
//---------------------------------------------------------

static void adjust2(int /*ml*/, Bm& bm, Chord* c1)
      {
      static const int dd[4][4] = {
            // St   H  --   S
            {0,  0,  1,  0},     // St
            {0,  0, -1,  0},     // S
            {1,  1,  1, -1},     // --
            {0,  0, -1,  0}      // H
            };
      int ys = bm.l + c1->line() * 2;
      int e1 = qAbs((ys  + 1000) % 4);
      int e2 = qAbs((ys + 1000 + bm.s) % 4);
      bm.l  -= dd[e1][e2];
      }

//---------------------------------------------------------
//   adjust3
//---------------------------------------------------------

static void adjust3(int /*ml*/, Bm& bm, Chord* c1)
      {
      static const int dd[4][4] = {
            // St   H  --   S
            {0,  0,  1,  0},     // St
            {0,  0, -1,  0},     // S
            {1,  1,  1, -1},     // --
            {0,  0, -1,  0}      // H
            };
      int ys = bm.l + c1->line() * 2;
      int e1 = qAbs((ys  + 1000) % 4);
      int e2 = qAbs((ys + 1000 + bm.s) % 4);
      bm.l  -= dd[e1][e2];
      }

//---------------------------------------------------------
//   minSlant
//---------------------------------------------------------

static int minSlant(uint interval)
      {
      static const int minSlantTable[] = { 0, 1, 2, 4, 5 };
      if (interval > 4)
            return 5;
      return minSlantTable[interval];
      }

//---------------------------------------------------------
//   maxSlant
//---------------------------------------------------------

static int maxSlant(uint interval)
      {
      static const int maxSlantTable[] = { 0, 1, 4, 5, 5, 6, 7, 8 };
      if (interval > 7)
            return 8;
      return maxSlantTable[interval];
      }

//---------------------------------------------------------
//   computeStemLen
//---------------------------------------------------------

void Beam::computeStemLen(const QList<ChordRest*>& crl, QPointF& p1, QPointF& /*p2*/,
   Chord* c1, Chord* c2, int beamLevels)
      {
      if (_up == -1)
            return;
      qreal _spatium  = spatium();
      qreal _spatium4 = _spatium * .25;
      qreal dx = c2->pagePos().x() - c1->pagePos().x();

      int l1 = c1->line();
      int l2 = c2->line();

      Bm bm = beamMetric1(_up, l1, l2);
      if (bm.s == 0 && bm.l == 0)
            bm = beamMetric2(_up, l1, l2);
      if (beamLevels == 2 && crl.size() == 2) {
            uint interval = qAbs(l2 - l1);
            int minS      = minSlant(interval);
            int maxS      = maxSlant(interval);
            l1           *= 2;
            l2           *= 2;

            if (_up) {
                  //
                  // extend to middle line, slant is always 1
                  //
                  if ((l1 > 20) && (l2 > 20)) {
                        if (l1 > l2) {
                              bm.l = 9 - l1;
                              bm.s = -1;
                              }
                        else if (l1 == l2) {
                              bm.l = 9 - l1;
                              bm.s = 0;
                              }
                        else {
                              bm.l = 8 - l1;
                              bm.s = 1;
                              }
                        }
                  else {
                        int ll1 = l1 - 12;     // sp minimum to primary beam
                        // ll % 4:
                        //    0 straddle
                        //    1 hang
                        //    2 --
                        //    3 sit
                        if (l1 == l2) {
                              bm.l = ll1 - l1;
                              bm.s = 0;
                              if (l1 & 2)
                                    bm.l -= 1;
                              }
                        else {
                              int n = 0;
                              for (;;ll1--) {
                                    int i;
                                    for (i = minS; i <= maxS; ++i) {
                                          int e1  = ll1 & 3;
                                          int ll2 = ll1 + ((l2 > l1) ? i : -i);
                                          if ((l2 - ll2) < 12)
                                                continue;
                                          int e2  = ll2 & 3;
                                          if ((e1 == 0 && e2 == 1) || (e1 == 1 && e2 == 0))
                                                break;
                                          }
                                    if (i <= maxS) {
                                          bm.l = ll1 - l1;
                                          bm.s = l2 > l1 ? i : -i;
                                          break;
                                          }
                                    if (++n > 10) {
                                          printf("beam note found\n");
                                          break;
                                          }
                                    }
                              }
                        }
                  }
            else {
                  //
                  // extend to middle line, slant is always 1
                  //
                  if ((l1 < -4) && (l2 < -4)) {
                        if (l1 > l2) {
                              bm.l = 8 - l1;
                              bm.s = -1;
                              }
                        else if (l1 == l2) {
                              bm.l = 7 - l1;
                              bm.s = 0;
                              }
                        else {
                              bm.l = 7 - l1;
                              bm.s = 1;
                              }
                        }
                  else {
                        int ll1 = 12 + l1;     // sp minimum to primary beam
                        if (l1 == l2) {
                              bm.l = ll1 - l1;
                              bm.s = 0;
                              if (l1 & 2)
                                    bm.l += 1;
                              }
                        else {
                              int n = 0;
                              for (;;ll1++) {
                                    int i;
                                    for (i = minS; i <= maxS; ++i) {
                                          int e1  = ll1 & 3;
                                          int ll2 = ll1 + ((l2 > l1) ? i : -i);
                                          if ((ll2 - l2) < 12)
                                                continue;
                                          int e2  = ll2 & 3;
                                          if ((e1 == 0 && e2 == 3) || (e1 == 3 && e2 == 0))
                                                break;
                                          }
                                    if (i <= maxS) {
                                          bm.l = ll1 - l1;
                                          bm.s = l2 > l1 ? i : -i;
                                          break;
                                          }
                                    if (++n > 10) {
                                          printf("beam note found\n");
                                          break;
                                          }
                                    }
                              }
                        }
                  }
            }
      else if (beamLevels > 1 && crl.size() == 2) {
            static const int t[] = { 0, 0, 4, 4, 8, 12, 16 }; // spatium4 added to stem len
            int n = t[beamLevels];
            bm.l += _up ? -n : n;
            }
      else if (crl.size() > 2) {
            if (noSlope(crl, c1, c2))
                  bm.s = 0;
            int ml = adjust(_spatium4, bm, crl);
            // bool _up = c1->up();
            if (ml) {
                  if (c1->up())
                        bm.l -= ml;
                  else
                        bm.l += ml;
                  if (beamLevels == 1)
                        adjust2(ml, bm, c1);
                  }
            if (beamLevels > 1)
                  adjust3(ml, bm, c1);
            }
      slope   = (bm.s * _spatium4) / dx;
      p1.ry() += ((c1->line(_up) - c1->line(!_up)) * 2 + bm.l) * _spatium4;
      }

//---------------------------------------------------------
//   layout2
//---------------------------------------------------------

void Beam::layout2(QList<ChordRest*>crl, SpannerSegmentType st, int frag)
      {
      if (_distribute)
            score()->respace(&crl);       // fix horizontal spacing of stems

      Chord* c1 = 0;          // first chord in beam
      Chord* c2 = 0;          // last chord in beam
      foreach (ChordRest* cr, crl) {
            if (cr->type() == CHORD) {
                  if (c1 == 0)
                        c1 = static_cast<Chord*>(cr);
                  c2 = static_cast<Chord*>(cr);
                  }
            }
      if (c1 == 0)      // no chords?
            return;

      int beamLevels = 1;
      int chordRests = crl.size();
      bool hasBeamSegment[chordRests];
      for (int idx = 0; idx < chordRests; ++idx) {
            int n = crl[idx]->durationType().hooks();
            if (n > beamLevels)
                  beamLevels = n;
            hasBeamSegment[idx] = false;
            }

      BeamFragment* f  = fragments[frag];
      int dIdx         = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      QPointF& p1      = f->p1[dIdx];
      QPointF& p2      = f->p2[dIdx];

      qreal _spatium   = spatium();
      cut              = 0;
      QPointF canvPos(pagePos());
      qreal bd         = score()->styleD(ST_beamDistance);
      qreal bw         = score()->styleS(ST_beamWidth).val() * _spatium;
      qreal beamMinLen = point(score()->styleS(ST_beamMinLen));
      qreal graceMag   = score()->styleD(ST_graceNoteMag);

      // TODO: what about undefined direction (_up = -1)?
      if (_up == -1)
            _up = 1;

      qreal beamDist = bd * bw + bw;      // 0.75 spatium

      if (isGrace) {
            beamDist *= graceMag;
            setMag(graceMag);
            beamMinLen *= graceMag;
            }
      else
            setMag(1.0);

      if (staff()->useTablature()) {
            qreal y  = STAFFTYPE_TAB_DEFAULTSTEMPOSY * _spatium;
            p1 = QPointF(c1->stemPos(true, false).x(), y);
            p2 = QPointF(c2->stemPos(true, false).x(), y);
            }
      else {
            //
            // PITCHED STAVES: SETUP
            //
            if (_userModified[dIdx]) {
                  p1.ry() += canvPos.y();
                  p2.ry() += canvPos.y();
                  p1.rx() = c1->stemPos(c1->up(), false).x();
                  p2.rx() = c2->stemPos(c2->up(), false).x();

                  qreal beamY   = p1.y();
                  slope         = (p2.y() - p1.y()) / (p2.x() - p1.x());
                  //
                  // set stem direction for every chord
                  //
                  foreach(ChordRest* cr, crl) {
                        if (cr->type() != CHORD)
                              continue;
                        Chord* c  = static_cast<Chord*>(cr);
                        QPointF p = c->upNote()->pagePos();
                        qreal y1  = beamY + (p.x() - p1.x()) * slope;
                        bool nup  = y1 < p.y();
                        if (c->up() != nup) {
                              c->setUp(nup);
                              // guess was wrong, have to relayout
                              score()->layoutChords1(c->segment(), c->staffIdx());
                              }
                        }
                  _up = crl.front()->up();
                  }
            else if (cross) {
                  qreal beamY   = 0.0;  // y position of main beam start
                  qreal y1   = -200000;
                  qreal y2   = 200000;
                  foreach(ChordRest* cr, crl) {
                        if (cr->type() != CHORD)
                              continue;
                        Chord* c = static_cast<Chord*>(cr);
                        qreal y  = c->upNote()->pagePos().y();
                        y1       = qMax(y1, y);
                        y2       = qMin(y2, y);
                        }
                  if (y1 > y2)
                        beamY = y2 + (y1 - y2) * .5;
                  else
                        beamY = _up ? y2 : y1;
                  p1.ry() = beamY;
                  p2.ry() = beamY;
                  //
                  // set stem direction for every chord
                  //
                  foreach(ChordRest* cr, crl) {
                        if (cr->type() != CHORD)
                              continue;
                        Chord* c  = static_cast<Chord*>(cr);
                        qreal y  = c->upNote()->pagePos().y();
                        bool nup = beamY < y;
                        if (c->up() != nup) {
                              c->setUp(nup);
                              // guess was wrong, have to relayout
                              score()->layoutChords1(c->segment(), c->staffIdx());
                              }
                        }
                  _up = -1;

                  qreal yDownMax = -300000;
                  qreal yUpMin   = 300000;
                  foreach(ChordRest* cr, crl) {
                        if (cr->type() != CHORD)
                              continue;
                        bool _up = cr->up();
                        qreal y = cr->stemPos(!cr->up(), false).y();
                        if (_up)
                              yUpMin = qMin(y, yUpMin);
                        else
                              yDownMax = qMax(y, yDownMax);
                        }
                  qreal y = yUpMin + (yDownMax - yUpMin) * .5;
                  p1.ry() = y;
                  p2.ry() = y;
                  }
            else {
                  p1 = c1->stemPos(c1->up(), false);
                  p2 = c2->stemPos(c2->up(), false);
                  cut *= (_up ? 1 : -1);
                  computeStemLen(crl, p1, p2, c1, c2, beamLevels);
                  }
            p2.ry() = (p2.x() - p1.x()) * slope + p1.y();
            p1 -= canvPos;
            p2 -= canvPos;
            }

      //---------------------------------------------
      //   create beam segments:
      //   COMMON TO BOTH TABLATURES AND PITCHED
      //---------------------------------------------

      qreal p1dy = p1.y();

      int upLines   = 0;
      int downLines = 0;
      qreal stemWidth = point(score()->styleS(ST_stemWidth));

      qreal x1 = p1.x() + canvPos.x();
      for (int beamLevel = 0; beamLevel < beamLevels; ++beamLevel) {
            ChordRest* cr1 = 0;
            ChordRest* cr2 = 0;
            bool hasBeamSegment1[chordRests];
            memset(hasBeamSegment1, 0, sizeof(hasBeamSegment));

            qreal dist = beamDist * beamLevel;

            for (int idx = 0; idx < chordRests; ++idx) {
                  ChordRest* cr = crl[idx];
                  bool b32 = (beamLevel >= 1) && (cr->beamMode() == BEAM_BEGIN32);
                  bool b64 = (beamLevel >= 2) && (cr->beamMode() == BEAM_BEGIN64);

                  // end current beam level?
                  int crLevel = cr->durationType().hooks() - 1;
                  if ((crLevel < beamLevel) || b32 || b64) {
                        if (cr2) {
                              // create short segment
                              qreal y1;
                              if (cr2->up()) {
                                    ++upLines;
                                    y1 = p1dy + beamDist * (beamLevel - (downLines ? downLines - 1 : 0));
                                    }
                              else {
                                    ++downLines;
                                    y1 = p1dy - beamDist * (beamLevel - (upLines ? upLines - 1 : 0));
                                    }
                              // qreal x1  = p1.x() + canvPos.x();
                              qreal x2  = cr1->stemPos(cr1->up(), false).x();
                              qreal x3  = cr2->stemPos(cr2->up(), false).x();

                              qreal lx1 = x2 - canvPos.x();
                              qreal lx2 = x3 - canvPos.x();
                              qreal ly1 = (x2 - x1) * slope + y1;
                              qreal ly2 = (x3 - x1) * slope + y1;
                              beamSegments.append(new QLineF(lx1, ly1, lx2, ly2));
                              }
                        else if (cr1) {
                              qreal y1;
                              if (cr1->up()) {
                                    ++upLines;
                                    y1 = p1dy + dist;
                                    }
                              else {
                                    ++downLines;
                                    y1 = p1dy - dist;
                                    }
                              // create broken segment
                              qreal len = beamMinLen;

                              if ((idx > 1) && (idx < chordRests)
                                 && (crl[idx-2]->duration() != crl[idx]->duration())) {
                                    if (crl[idx-2]->duration() < crl[idx]->duration())
                                          len = -len;
                                    }
                              else {
                                    // find out direction of beam fragment
                                    // if on first chord: right
                                    // if on last chord:  left
                                    // else ...
                                    //    point to same direction as beam starting
                                    //       one level higher
                                    //
                                    if (!hasBeamSegment[idx-1] && (cr1 != crl[0])) {
                                          TDuration d = cr1->durationType();
                                          d = d.shift(-1);
                                          int rtick = cr1->tick() - cr1->measure()->tick();
                                          if (rtick % d.ticks())
                                                len = -len;
                                          }
                                    }
                              // qreal x1  = p1.x() + canvPos.x();
                              qreal x2 = cr1->stemPos(cr1->up(), false).x();
                              qreal x3 = x2 + len;
                              beamSegments.push_back(new QLineF(x2 - canvPos.x(), (x2 - x1) * slope + y1,
                                 x3 - canvPos.x(), (x3 - x1) * slope + y1));
                              hasBeamSegment1[idx-1] = false;
                              }
                        if (crLevel >= beamLevel) {
                              cr1 = cr;
                              hasBeamSegment1[idx] = true;
                              cr2 = 0;
                              }
                        else {
                              hasBeamSegment1[idx] = false;
                              cr1 = cr2 = 0;
                              }
                        }
                  else {
                        (cr1 ? cr2 : cr1) = cr;
                        hasBeamSegment1[idx] = (cr2 == 0);
                        }
                  }
            memcpy(hasBeamSegment, hasBeamSegment1, sizeof(hasBeamSegment));
            if (cr2) {
                  // create segment
                  if (!cr2->up()) {
                        ++downLines;
                        dist = -dist;
                        }
                  else
                        ++upLines;
                  // qreal x1 = p1.x() + canvPos.x();
                  qreal x2 = cr1->stemPos(cr1->up(), false).x();
                  qreal x3 = cr2->stemPos(cr2->up(), false).x();

                  if (st == SEGMENT_BEGIN)
                        x3 += _spatium * 2;
                  else if (st == SEGMENT_END)
                        x2 -= _spatium * 2;
                  else {
                        if (_up)
                              x2 -= stemWidth;
                        else
                              x3 += stemWidth;
                        }
                  qreal lx1 = x2 - canvPos.x();
                  qreal ly1 = (x2 - x1) * slope + p1dy + dist * _grow1;
                  qreal lx2 = x3 - canvPos.x();
                  qreal ly2 = (x3 - x1) * slope + p1dy + dist  * _grow2;
                  beamSegments.push_back(new QLineF(lx1, ly1, lx2, ly2));
                  }
            else if (cr1) {
                  // create broken segment
                  if (!cr1->up())
                        dist = -dist;
                  // qreal x1  = p1.x() + canvPos.x();
                  qreal x3 = cr1->stemPos(cr1->up(), false).x();
                  qreal x2 = x3 - beamMinLen;

                  qreal lx1 = x2 - canvPos.x();
                  qreal ly1 = (x2 - x1) * slope + p1dy + dist;
                  qreal lx2 = x3 - canvPos.x();
                  qreal ly2 = (x3 - x1) * slope + p1dy + dist;
                  beamSegments.push_back(new QLineF(lx1, ly1, lx2, ly2));
                  }
            }

      //---------------------------------------------------
      //    create stem's
      //---------------------------------------------------

      bool firstChord = true;
      foreach (ChordRest* cr, crl) {
            if (cr->type() != CHORD)
                  continue;
            Chord* chord = static_cast<Chord*>(cr);

            Stem* stem = chord->stem();
            if (!stem) {
                  stem = new Stem(score());
                  chord->setStem(stem);
                  }

            if (chord->hook())
                  score()->undoRemoveElement(chord->hook());

            if (staff()->useTablature()) {
                  //
                  // TABLATURE stems have fixed len and pos
                  //
                  QPointF p1(STAFFTYPE_TAB_DEFAULTSTEMPOSX * _spatium,
                     STAFFTYPE_TAB_DEFAULTSTEMPOSY * _spatium);
                  QPointF p2(STAFFTYPE_TAB_DEFAULTSTEMPOSX * _spatium,
                     (STAFFTYPE_TAB_DEFAULTSTEMPOSY - STAFFTYPE_TAB_DEFAULTSTEMLEN) * _spatium);
                  stem->setPos(STAFFTYPE_TAB_DEFAULTSTEMPOSX * _spatium, STAFFTYPE_TAB_DEFAULTSTEMPOSY * _spatium);
                  }
            else {
                  //
                  // PITCHED STAFF stems
                  //
                  bool chordUp = chord->up();
                  QPointF npos(chord->stemPos(chordUp, false));   // canvas coordinates

                  qreal x2    = npos.x();
                  qreal y1    = npos.y();
                  qreal yMin  = 1000000;
                  qreal yMax  = -1000000;

                  //  extend stem to farest beam segment
                  qreal x  = x2 - parent()->pagePos().x();
                  qreal dx = _spatium;
                  foreach(QLineF* l, beamSegments) {
                        if ((l->x1() - dx <= x) && (l->x2() + dx > x)) {
                              qreal dx = x - l->x1();
                              qreal dy = dx * slope;
                              qreal yy = l->y1() + dy;
                              yMin = qMin(yMin, yy);
                              yMax = qMax(yMax, yy);
                              }
                        }
                  qreal yo = chordUp ? yMin : yMax;
                  stem->setLen(yo + canvPos.y() - y1);
                  stem->setPos(npos - chord->pagePos());
                  }

            //
            // layout stem slash for acciacatura
            //
            if (firstChord && chord->noteType() == NOTE_ACCIACCATURA) {
                  StemSlash* stemSlash = chord->stemSlash();
                  if (!stemSlash) {
                        stemSlash = new StemSlash(score());
                        chord->add(stemSlash);
                        }
                  stemSlash->layout();
                  }
            else
                  chord->setStemSlash(0);
            firstChord = false;

            Tremolo* tremolo = chord->tremolo();
            if (tremolo)
                  tremolo->layout();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Beam::write(Xml& xml) const
      {
      xml.stag(QString("Beam id=\"%1\"").arg(_id));
      Element::writeProperties(xml);

      for (int i = 0;; ++i) {
            const Property<Beam>& p = propertyList[i];
            P_ID id = p.id;
            if (id == P_END)
                  break;
            xml.tag(id, ((*(Beam*)this).*(p.data))(), propertyList[i].defaultVal);
            }

      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      if (_userModified[idx]) {
            qreal _spatium = spatium();
            foreach(BeamFragment* f, fragments) {
                  xml.stag("Fragment");
                  xml.tag("y1", f->p1[idx].y() / _spatium);
                  xml.tag("y2", f->p2[idx].y() / _spatium);
                  xml.etag();
                  }
            }
#ifndef NDEBUG
      //
      // this info is used for regression testing
      // l1/l2 is the beam position of the layout engine
      //
      if (score()->testMode()) {
            qreal _spatium4 = spatium() * .25;
            foreach(BeamFragment* f, fragments) {
                  xml.tag("l1", int(lrint(f->p1[idx].y() / _spatium4)));
                  xml.tag("l2", int(lrint(f->p2[idx].y() / _spatium4)));
                  }
            }
#endif
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Beam::read(const QDomElement& de)
      {
      QPointF p1, p2;
      qreal _spatium = spatium();
      _id = de.attribute("id").toInt();
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (setProperty(tag, e))
                  ;
            else if (tag == "y1") {
                  if (fragments.isEmpty())
                        fragments.append(new BeamFragment);
                  BeamFragment* f = fragments.back();
                  int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  f->p1[idx] = QPointF(0.0, val.toDouble() * _spatium);
                  }
            else if (tag == "y2") {
                  if (fragments.isEmpty())
                        fragments.append(new BeamFragment);
                  BeamFragment* f = fragments.back();
                  int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  f->p2[idx] = QPointF(0.0, val.toDouble() * _spatium);
                  }
            else if (tag == "Fragment") {
                  BeamFragment* f = new BeamFragment;
                  int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  qreal _spatium = spatium();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        qreal v = ee.text().toDouble() * _spatium;
                        if (tag == "y1")
                              f->p1[idx] = QPointF(0.0, v);
                        else if (tag == "y2")
                              f->p2[idx] = QPointF(0.0, v);
                        else
                              domError(ee);
                        }
                  fragments.append(f);
                  }
#ifndef NDEBUG
            else if (tag == "l1" || tag == "l2")      // ignore
                  ;
#endif
            else if (tag == "subtype")          // obsolete
                  ;
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Beam::editDrag(const EditData& ed)
      {
      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      QPointF d(0.0, ed.delta.y());
      BeamFragment* f = fragments[editFragment];
      if (ed.curGrip == 0)
            f->p1[idx] += d;
      f->p2[idx] += d;
      _userModified[idx] = true;
      setGenerated(false);
// layout1();
// layout();
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Beam::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 2;
      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      BeamFragment* f = fragments[editFragment];
      grip[0].translate(pagePos() + f->p1[idx]);
      grip[1].translate(pagePos() + f->p2[idx]);
      }

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void Beam::setBeamDirection(Direction d)
      {
      _direction = d;
      if (d != AUTO) {
            _up = d == UP;
            setGenerated(false);
            }
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Beam::toDefault()
      {
      for (int i = 0;; ++i) {
            const Property<Beam>& p = propertyList[i];
            P_ID id = p.id;
            if (id == P_END)
                  break;
            if (p.defaultVal) {
                  QVariant defaultVal = getVariant(id, p.defaultVal);
                  if (getProperty(id) != defaultVal)
                        score()->undoChangeProperty(this, id, defaultVal);
                  }
            }

      _userModified[0] = false;
      _userModified[1] = false;
      setGenerated(true);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Beam::startEdit(MuseScoreView*, const QPointF& p)
      {
      QPointF pt(p - pagePos());
      qreal ydiff = 100000000.0;
      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      int i = 0;
      editFragment = 0;
      foreach (BeamFragment* f, fragments) {
            qreal d = fabs(f->p1[idx].y() - pt.y());
            if (d < ydiff) {
                  ydiff = d;
                  editFragment = i;
                  }
            ++i;
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Beam::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return (e->type() == ICON) && ((static_cast<Icon*>(e)->subtype() == ICON_FBEAM1)
         || (static_cast<Icon*>(e)->subtype() == ICON_FBEAM2));
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Beam::drop(const DropData& data)
      {
      Icon* e = static_cast<Icon*>(data.element);
      if (e->type() != ICON)
            return 0;
      qreal g1;
      qreal g2;

      if (e->subtype() == ICON_FBEAM1) {
            g1 = 1.0;
            g2 = 0.0;
            }
      else if (e->subtype() == ICON_FBEAM2) {
            g1 = 0.0;
            g2 = 1.0;
            }
      else
            return 0;
      if (g1 != growLeft())
            score()->undoChangeProperty(this, P_GROW_LEFT, g1);
      if (g2 != growRight())
            score()->undoChangeProperty(this, P_GROW_RIGHT, g2);
      return 0;
      }

PROPERTY_FUNCTIONS(Beam)

