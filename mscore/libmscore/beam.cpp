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
      foreach(QLineF* bs, beamSegments)
            delete bs;
      foreach(BeamFragment* f, fragments)
            delete f;
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
      if (!_elements.contains(a))
            _elements.append(a);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Beam::remove(ChordRest* a)
      {
      _elements.removeOne(a);
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
      painter->setBrush(QBrush(QColor(painter->pen().color())));
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
//                        cr->setUp(_up);
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
            foreach(ChordRest* cr, _elements) {
                  if (cr->type() == CHORD) {
                        c2 = static_cast<Chord*>(cr);
                        //
                        // if only one stem direction is manually set it
                        // determines if beams are up or down
                        //
                        if (c2->stemDirection() != AUTO)
                              upCount += c2->stemDirection() == UP ? 1000 : -1000;
                        else
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
                        }
                  if (!maxDuration.isValid() || (maxDuration < cr->durationType()))
                        maxDuration = cr->durationType();
                  }

            _up     = (_direction == AUTO) ? (upCount >= 0) : (_direction == UP);
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
            }                                   // end of if/else(tablature)
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
//   layout2
//---------------------------------------------------------

void Beam::layout2(QList<ChordRest*>crl, SpannerSegmentType st, int frag)
      {
      Chord* c1 = 0;          // first chord in beam
      Chord* c2 = 0;          // last chord in beam
      foreach (ChordRest* cr, crl) {
            if (cr->type() == CHORD) {
                  if (c1 == 0)
                        c1 = static_cast<Chord*>(cr);
                  c2 = static_cast<Chord*>(cr);
                  }
            }
      if (c1 == 0)
            return;

      BeamFragment* f  = fragments[frag];
      int idx          = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      qreal _spatium  = spatium();
      cut               = 0;
      QPointF canvPos(pagePos());
      qreal bd         = score()->styleD(ST_beamDistance);
      Spatium bw        = score()->styleS(ST_beamWidth);
      qreal beamMinLen = point(score()->styleS(ST_beamMinLen));
      qreal graceMag   = score()->styleD(ST_graceNoteMag);

      // TODO: what about undefined direction (_up = -1)?
      if (_up == -1)
            _up = 1;

      // qreal beamDist = point(bd * bw + bw) * (_up ? 1.0 : -1.0);
      qreal beamDist = point(bd * bw + bw);
      if (isGrace)
            beamDist *= graceMag;
      qreal x1, x2;

      if (staff()->useTablature()) {

            //
            // TABLATURE STAVES: SETUP
            //

            qreal xoffLeft  = point(score()->styleS(ST_stemWidth)) * .5;
            qreal xoffRight = xoffLeft;
            QPointF c1StemPos= c1->stemPos(true, false);
            QPointF c2StemPos= c2->stemPos(true, false);
            x1        = c1StemPos.x() - xoffLeft;
            x2        = c2StemPos.x() + xoffRight;

            f->p1[0].rx() = x1;
            f->p2[0].rx() = x2;
            f->p1[0].ry() = f->p2[0].ry() = STAFFTYPE_TAB_DEFAULTSTEMPOSY*_spatium;

            setMag(isGrace ? graceMag : 1.0);
            }

      else {

            //
            // PITCHED STAVES: SETUP
            //

            qreal p1x = c1->upNote()->pagePos().x();
            qreal p2x = c2->upNote()->pagePos().x();
            int l1     = c1->line(_up);
            int l2     = c2->line(_up);

            f->p1[idx] += canvPos;
            f->p2[idx] += canvPos;

            if (_userModified[idx]) {
                  qreal beamY = f->p1[idx].y();
                  slope        = (f->p2[idx].y() - f->p1[idx].y()) / (p2x - p1x);
                  //
                  // set stem direction for every chord
                  //
                  foreach(ChordRest* cr, crl) {
                        if (cr->type() != CHORD)
                              continue;
                        Chord* c  = static_cast<Chord*>(cr);
                        QPointF p = c->upNote()->pagePos();
                        qreal y1 = beamY + (p.x() - p1x) * slope;
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
                        qreal y = c->upNote()->pagePos().y();
                        y1       = qMax(y1, y);
                        y2       = qMin(y2, y);
                        }
                  if (y1 > y2)
                        beamY = y2 + (y1 - y2) * .5;
                  else
                        beamY = _up ? y2 : y1;
                  f->p1[idx].ry() = beamY;
                  f->p2[idx].ry() = beamY;
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
                  }
            else {
                  //
                  //    compute concave flag
                  //    concave beams have a slope of 0.0
                  //
                  bool concave = false;
                  bool sameLine = true;

                  if (crl.size() >= 3) {
                        int l4 = crl[1]->line(_up);
                        for (int i = 1; i < crl.size()-1; ++i) {
                              int l3 = crl[i]->line(_up);
                              if (l3 != l4)
                                    sameLine = false;
                              if (_up) {
                                    if (l3 < l1 && l3 < l2) {
                                          concave = true;
                                          break;
                                          }
                                    }
                              else {
                                    if (l3 > l1 && l3 > l2) {
                                          concave = true;
                                          break;
                                          }
                                    }
                              }
                        if (sameLine && (l1 == l4 || l2 == l4)) {
                              if (_up) {
                                    if (l1 == l4 && l1 < l2)
                                          concave = true;
                                    else if (l2 == l4 && l2 < l1)
                                          concave = true;
                                    }
                              else {
                                    if (l1 == l4 && l1 > l2)
                                          concave = true;
                                    else if (l2 == l4 && l2 > l1)
                                          concave = true;
                                    }
                              }
                        }

                  if (!concave) {
                        qreal dx = c2->pagePos().x() - c1->pagePos().x();
                        if (dx) {
                              qreal maxSlope = score()->style(ST_beamMaxSlope).toDouble();
                              qreal lslope = l2 - l1;
                              if (crl.size() <= 3) {
                                    //
                                    // if notes are spaced very close together (<= 3.0 spaces)
                                    // use only a small slope
                                    //
                                    if (dx / _spatium <= 3.0) {
                                          lslope *= .25;
                                          lslope = absLimit(lslope, 0.5);
                                          }
                                    else {
                                          lslope *= .5;
                                          lslope = absLimit(lslope, 1.0);
                                          }
                                    }
                              else {
                                    lslope *= .3;
                                    lslope = absLimit(lslope, 2.0);
                                    }
                              slope = (lslope * _spatium) / dx;

                              //if (fabs(slope) < score()->style(ST_beamMinSlope).toDouble()) {
                              //      cut = slope > 0.0 ? 0 : -1;
                              //      slope = 0;
                              //      }
                              //else
                              if (slope > maxSlope) {
                                    slope = maxSlope;
                                    cut = 1;
                                    }
                              else if (-slope > maxSlope) {
                                    slope = -maxSlope;
                                    cut = -1;
                                    }
                              }
                        else
                              slope = 0.0;
                        }
                  else
                        slope = 0.0;
                  cut *= (_up ? 1 : -1);
                  }

            //---------------------------------------------------
            //    create beam segments
            //---------------------------------------------------

            qreal xoffLeft  = point(score()->styleS(ST_stemWidth)) * .5;
            qreal xoffRight = xoffLeft;
            x1               = c1->stemPos(c1->up(), false).x() - xoffLeft;
            x2               = c2->stemPos(c2->up(), false).x() + xoffRight;

            f->p1[idx].rx()  = x1;
            f->p2[idx].rx()  = x2;

            if (!_userModified[idx]) {
                  //
                  // compute final y position of 1/8 beam
                  //
                  if (cross) {
                        qreal yDownMax = -300000;
                        qreal yUpMin   = 300000;
                        foreach(ChordRest* cr, crl) {
                              if (cr->type() != CHORD)
                                    continue;
                              qreal y;
                              bool _up = cr->up();
                              y = cr->stemPos(!cr->up(), false).y();
                              if (_up)
                                    yUpMin = qMin(y, yUpMin);
                              else
                                    yDownMax = qMax(y, yDownMax);
                              }
                        f->p1[idx].ry() = f->p2[idx].ry() = yUpMin + (yDownMax - yUpMin) * .5;
                        }
                  else {
                        QPointF p1s(c1->stemPos(c1->up(), false));
                        QPointF p2s(c2->stemPos(c2->up(), false));
                        qreal ys = (x2 - x1) * slope;

                        if (cut >= 0) {
                              // left dot is reference
                              f->p1[idx].ry() = p1s.y();
                              f->p2[idx].ry() = f->p1[idx].y() + ys;
                              }
                        else {
                              // right dot is reference
                              f->p2[idx].ry() = p2s.y();
                              f->p1[idx].ry() = f->p2[idx].y() - ys;
                              }
                        qreal my  = _spatium * 2.0 + canvPos.y();

                        qreal min        =  1000000.0;
                        qreal max        = -1000000.0;
                        bool toMiddleLine = true;
                        qreal minStemLen = 3.0 * _spatium;
                        if (isGrace)
                              minStemLen *= graceMag;

// qDebug("Beam===%f\n", minStemLen);
                        int beams = 0;
                        foreach(ChordRest* cr, crl) {
                              if (cr->type() != CHORD)
                                    continue;
                              Chord* chord  = static_cast<Chord*>(cr);
                              QPointF npos(chord->stemPos(_up, true));
                              qreal y1 = npos.y();
                              qreal y2 = f->p1[idx].y() + (npos.x() - x1) * slope;
                              qreal stemLen;
                              if (_up) {
                                    if ((y1-my) < minStemLen)
                                          toMiddleLine = false;
                                    stemLen = y1 - y2;
                                    }
                              else {
                                    if ((my-y1) < minStemLen)
                                          toMiddleLine = false;
                                    stemLen = y2 - y1;
                                    }

// qDebug("   min %f len %f max %f\n", min, stemLen, max);
                              if (stemLen < min) {
                                    min = stemLen;
                                    beams = chord->beams() -1;
                                    }
                              if (stemLen > max)
                                    max = stemLen;
                              }
                        if (toMiddleLine) {
                              // extend stems to middle staff line
                              f->p1[idx].ry() = my;
                              f->p2[idx].ry() = my;
                              slope           = 0.0;
                              }
                        else {
                              // adjust beam position
                              qreal n = 3.5 * _spatium;
                              if (isGrace)
                                    n *= graceMag;

                              qreal diff;
                              qreal beamsHeight = 0.0; // beams * beamDist * .5;
                              switch (beams) {
                                    case 0:
                                    case 1: beamsHeight = 0.0; break;
                                    case 2: beamsHeight = _spatium; break;
                                    case 3: beamsHeight = _spatium * 1.5; break;
                                    case 4: beamsHeight = _spatium * 2.0; break;
                                    }
                              if (_up) {
                                    diff =  min - n - beamsHeight;
                                    }
                              else {
                                    diff =  n - min + beamsHeight;
                                    }
                              f->p1[idx].ry() += diff;
                              f->p2[idx].ry() += diff;

                              bool shortenStem = score()->styleB(ST_shortenStem);
                              if (shortenStem) {
                                    qreal progression(score()->styleS(ST_shortStemProgression).val());
                                    Spatium shortest(score()->styleS(ST_shortestStem));
                                    qreal ty   = c1->measure()->system()->staffY(c1->staffIdx());
                                    qreal diff = 0.0;
                                    if (_up) {
                                          qreal y = qMax(f->p1[idx].y(), f->p2[idx].y());
                                          diff = (ty - y) / _spatium;
                                          }
                                    else {
                                          qreal by = ty + _spatium * 4;
                                          qreal y = qMin(f->p1[idx].y(), f->p2[idx].y());
                                          diff = (y - by) / _spatium;
                                          }
                                    if (diff > 2.0) {
                                          diff *= progression;
                                          if (diff > (3.0 - shortest.val()))
                                                diff = 3.0 - shortest.val();
                                          diff            *= _up ? _spatium : -_spatium;
                                          f->p1[idx].ry() += diff;
                                          f->p2[idx].ry() += diff;
                                          }
                                    }
                              }
                        }
                  qreal yy       = system()->staffY(c1->staffIdx());
                  f->p1[idx].ry() = alignBeam(l1, f->p1[idx].y() - yy, _spatium, _up) + yy;
                  f->p2[idx].ry() = alignBeam(l2, f->p2[idx].y() - yy, _spatium, _up) + yy;
                  }
            f->p1[idx] -= canvPos;
            f->p2[idx] -= canvPos;

            if (isGrace) {
                  setMag(graceMag);
                  bw         *= graceMag;
                  beamMinLen *= graceMag;
                  }
            else
                  setMag(1.0);
            }

      //---------------------------------------------
      //   create beam segments:
      //   COMMON TO BOTH TABLATURES AND PITCHED
      //---------------------------------------------

      qreal stemWidth2 = point(score()->styleS(ST_stemWidth)) * .5;
      qreal p1dy = f->p1[idx].y();

      int beamLevels = 1;
      int chordRests = crl.size();
      bool hasBeamSegment[chordRests];
      for (int idx = 0; idx < chordRests; ++idx) {
            int n = crl[idx]->durationType().hooks();
            if (n > beamLevels)
                  beamLevels = n;
            hasBeamSegment[idx] = false;
            }

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
                              if (cr2->up())
                                    y1 = p1dy + dist;
                              else
                                    y1 = p1dy - dist;
                              qreal x2 = cr1->stemPos(cr1->up(), false).x();
                              qreal x3 = cr2->stemPos(cr2->up(), false).x();
                              beamSegments.push_back(new QLineF(x2 - canvPos.x(), (x2 - x1) * slope + y1,
                                 x3 - canvPos.x(), (x3 - x1) * slope + y1));
                              }
                        else if (cr1) {
                              qreal y1;
                              if (cr1->up())
                                    y1 = p1dy + dist;
                              else
                                    y1 = p1dy - dist;
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
                  if (!cr2->up())
                        dist = -dist;

                  qreal x2 = cr1->stemPos(cr1->up(), false).x();
                  qreal x3 = cr2->stemPos(cr2->up(), false).x();

                  if (st == SEGMENT_BEGIN)
                        x3 += _spatium * 2;
                  else if (st == SEGMENT_END)
                        x2 -= _spatium * 2;
                  else {
                        x2 -= stemWidth2;
                        x3 += stemWidth2;
                        }
                  beamSegments.push_back(new QLineF(x2 - canvPos.x(), (x2 - x1) * slope + p1dy + dist * _grow1,
                     x3 - canvPos.x(), (x3 - x1) * slope + p1dy + dist  * _grow2));
                  }
            else if (cr1) {
                  // create broken segment
                  if (!cr1->up())
                        dist = -dist;
                  qreal x3 = cr1->stemPos(cr1->up(), false).x();
                  qreal x2 = x3 - beamMinLen;
                  beamSegments.push_back(new QLineF(x2 - canvPos.x(), (x2 - x1) * slope + p1dy + dist,
                     x3 - canvPos.x(), (x3 - x1) * slope + p1dy + dist));
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
                  stem->setLen(STAFFTYPE_TAB_DEFAULTSTEMLEN*_spatium);
                  stem->setPos(STAFFTYPE_TAB_DEFAULTSTEMPOSX*_spatium, STAFFTYPE_TAB_DEFAULTSTEMPOSY*_spatium);
                  }
            else {
                  //
                  // PITCHED STAFF stems
                  //
                  bool chordUp = chord->up();
                  QPointF npos(chord->stemPos(chordUp, false));   // canvas coordinates

                  qreal x2 = npos.x();
                  qreal y1 = npos.y();
                  qreal y  = chordUp ? 1000000.0 : -1000000;
                  //  extend stem to farest beam segment
                  qreal x = x2 - parent()->pagePos().x();
                  foreach(QLineF* l, beamSegments) {
                        if ((l->x1() <= x) && (l->x2() > x)) {
                              qreal dx = x - l->x1();
                              qreal dy = dx * slope;
                              qreal yy = l->y1() + dy;

                              if (chordUp) {
                                    if (yy < y)
                                          y  = yy;
                                    }
                              else {
                                    if (yy > y)
                                          y = yy;
                                    }
                              }
                        }
                  stem->setLen(y + canvPos.y() - y1);
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
      switch(_direction) {
            case UP:
                  xml.tag("StemDirection", QVariant("up"));
                  break;
            case DOWN:
                  xml.tag("StemDirection", QVariant("down"));
                  break;
            case AUTO:
                  break;
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
      if (_grow1 != 1.0)
            xml.tag("growLeft", _grow1);
      if (_grow2 != 1.0)
            xml.tag("growRight", _grow2);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Beam::read(QDomElement e)
      {
      QPointF p1, p2;
      qreal _spatium = spatium();
      _id = e.attribute("id").toInt();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "y1") {
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
                        QString tag(ee.tagName());
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
            else if (tag == "StemDirection") {
                  if (val == "up") {
                        _direction = UP;
                        _up = 1;
                        }
                  else if (val == "down") {
                        _direction = DOWN;
                        _up = 0;
                        }
                  else
                        domError(e);
                  }
            else if (tag == "growLeft")
                  _grow1 = val.toDouble();
            else if (tag == "growRight")
                  _grow2 = val.toDouble();
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
layout1();
layout();
      score()->setUpdateAll(true);
//      score()->setLayoutAll(true);
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
      _direction = AUTO;
      _up        = -1;
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

bool Beam::acceptDrop(MuseScoreView*, const QPointF&, int type, int subtype) const
      {
      return (type == ICON && subtype == ICON_FBEAM1)
         || (type == ICON && subtype == ICON_FBEAM2);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Beam::drop(const DropData& data)
      {
      Element* e = data.element;
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
      score()->undo()->push(new ChangeBeamProperties(this, g1, g2));
      return 0;
      }


