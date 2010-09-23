//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
//
//  beam tables from GNU LilyPond music typesetter
//  (c) 2000--2007 Jan Nieuwenhuizen <janneke@gnu.org>
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

#include "beam.h"
#include "segment.h"
#include "score.h"
#include "chord.h"
#include "preferences.h"
#include "al/sig.h"
#include "style.h"
#include "note.h"
#include "tuplet.h"
#include "system.h"
#include "tremolo.h"
#include "measure.h"
#include "al/al.h"
#include "icons.h"
#include "undo.h"

//---------------------------------------------------------
//   endBeam
//---------------------------------------------------------

static BeamHint endBeamList[] = {
      // in 2 2 time
      //  end beams each 1 2 note
      BeamHint(0,  0,  2,  2, 1, 2 ),

      // in 3 2 time:
      //   end beams each 1 2 note
      //   end beams with 16th notes each 1 4 note
      //   end beams with 32th notes each 1 8 note

      //       note   timesig  position

      BeamHint(0,  0, 3, 2,  1, 2 ),
      BeamHint(0,  0, 3, 2,  2, 2 ),

      BeamHint(1, 16, 3, 2,  1, 4 ),
      BeamHint(1, 16, 3, 2,  1, 2 ),
      BeamHint(1, 16, 3, 2,  3, 4 ),
      BeamHint(1, 16, 3, 2,  1, 1 ),
      BeamHint(1, 16, 3, 2,  5, 4 ),

      BeamHint(1, 32, 3, 2,  1, 8 ),
      BeamHint(1, 32, 3, 2,  1, 4 ),
      BeamHint(1, 32, 3, 2,  3, 8 ),
      BeamHint(1, 32, 3, 2,  1, 2 ),
      BeamHint(1, 32, 3, 2,  5, 8 ),
      BeamHint(1, 32, 3, 2,  3, 4 ),
      BeamHint(1, 32, 3, 2,  7, 8 ),
      BeamHint(1, 32, 3, 2,  1, 1 ),
      BeamHint(1, 32, 3, 2,  9, 8 ),
      BeamHint(1, 32, 3, 2,  5, 4 ),
      BeamHint(1, 32, 3, 2, 11, 8 ),

      BeamHint(0,  0,  2,  4, 0, 0 ),  // switch-off at-any-beat feature
      BeamHint(0,  0,  2,  4, 1, 4 ),
      BeamHint(1, 32,  2,  4, 1, 8 ),
      BeamHint(1, 32,  2,  4, 3, 8 ),

      BeamHint(0,  0,  3, 4,  1, 4 ),
      BeamHint(0,  0,  3, 4,  1, 2 ),

      BeamHint(1, 16,  3, 4,  1, 4 ),
      BeamHint(1, 16,  3, 4,  1, 2 ),

      BeamHint(1, 32,  3, 4,  1, 8 ),
      BeamHint(1, 32,  3, 4,  1, 4 ),
      BeamHint(1, 32,  3, 4,  3, 8 ),
      BeamHint(1, 32,  3, 4,  1, 2 ),
      BeamHint(1, 32,  3, 4,  5, 8 ),

      // in common time:
      //   end beams each 1 2 note
      //   end beams with 32th notes each 1 8 note
      //   end beams with 1 8 triplets each 1 4 note

      BeamHint(0,  0,  4,  4, 1, 2 ),
      BeamHint(1, 12,  4,  4, 1, 4 ),
      BeamHint(1, 12,  4,  4, 3, 4 ),

      BeamHint(1, 16,  4,  4, 1, 4 ),
      BeamHint(1, 16,  4,  4, 3, 4 ),

      BeamHint(1, 32,  4,  4, 1, 8 ),
      BeamHint(1, 32,  4,  4, 1, 4 ),
      BeamHint(1, 32,  4,  4, 3, 8 ),
      BeamHint(1, 32,  4,  4, 5, 8 ),
      BeamHint(1, 32,  4,  4, 3, 4 ),
      BeamHint(1, 32,  4,  4, 7, 8 ),

      BeamHint(0,  0,  5,  4, 3, 4 ),

      BeamHint(0,  0,  6,  4, 3, 4 ),
      //BeamHint(1,  8,  6,  4, 1, 4 ),     // for promenade demo (ws)    //Moved custom beaming to promenade file (db)

      BeamHint(0,  0,  3,  8, 3, 8 ),

      BeamHint(0,  0,  4,  8, 0, 0 ), // switch-off at-any-beat feature
      BeamHint(0,  0,  4,  8, 1, 4 ),
      BeamHint(1, 32,  4,  8, 1, 8 ),
      BeamHint(1, 32,  4,  8, 3, 8 ),

    //BeamHint(0,  0,  6, 8,  0, 0 ), // switch-off at-any-beat feature    //Feature does not work when beam is every three notes instead of every two. (db)
      BeamHint(0,  0,  6, 8,  3, 8 ),

      BeamHint(0,  0,  9, 8,  3, 8 ),
      BeamHint(0,  0,  9, 8,  3, 4 ),

      BeamHint(0,  0, 12, 8,  3, 8 ),
      BeamHint(0,  0, 12, 8,  3, 4 ),
      BeamHint(0,  0, 12, 8,  9, 8 ),

      BeamHint(0,  0, 15, 8,  3, 8 ),
      BeamHint(0,  0, 15, 8,  3, 4 ),
      BeamHint(0,  0, 15, 8,  9, 8 ),
      BeamHint(0,  0, 15, 8,  6, 4 ),

      BeamHint(0,  0,  4, 16, 0, 0 ), // switch-off at-any-beat feature
      BeamHint(0,  0,  4, 16, 1, 8 ),

      };

//---------------------------------------------------------
//   endBeam
//    return true if beam should be ended
//---------------------------------------------------------

bool endBeam(const Fraction& ts, ChordRest* cr, int p)
      {
      if (cr->tuplet()) {
            if (cr->tuplet()->elements().front() == cr)     // end beam at tuplet
                  return true;
            return false;
            }
      int l = cr->ticks();
      for (unsigned i = 0; i < sizeof(endBeamList)/sizeof(*endBeamList); ++i) {
            const BeamHint& h = endBeamList[i];
            if (!h.timeSig.isZero() && (!h.timeSig.identical(ts)))
                  continue;
            if (h.noteLen.numerator()) {
                  int len = h.noteLen.ticks();
                  if (len != l) {
                        continue;
                        }
                  }
            if (!h.pos.isZero()) {
                  int pos = h.pos.ticks();
                  if (pos != p) {
                        continue;
                        }
                  }
            else {            // if (h.pos.numerator() == 0) {   // stop on every beat
                  int len = (4 * AL::division) / h.timeSig.denominator();
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
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
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
//   canvasPos
//---------------------------------------------------------

QPointF Beam::canvasPos() const
      {
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = static_cast<System*>(parent());
      if (system == 0)
            return pos();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Beam::add(ChordRest* a)
      {
      a->setBeam(this);
      _elements.append(a);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Beam::remove(ChordRest* a)
      {
      _elements.removeOne(a);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Beam::draw(QPainter& p, ScoreView*) const
      {
      QColor color(p.pen().color());
      p.setPen(QPen(Qt::NoPen));
      p.setBrush(color);
      qreal lw2 = point(score()->styleS(ST_beamWidth)) * .5 * mag();
      foreach (const QLineF* bs, beamSegments) {
            QPolygonF a(4);
            a[0] = QPointF(bs->x1(), bs->y1()-lw2);
            a[1] = QPointF(bs->x2(), bs->y2()-lw2);
            a[2] = QPointF(bs->x2(), bs->y2()+lw2);
            a[3] = QPointF(bs->x1(), bs->y1()+lw2);
            p.drawPolygon(a);
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Beam::move(double x, double y)
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
      printf("Beam::writeMusicXml(cr=%p)\n", cr);
      // dump beam contents
      foreach(ChordRest* crst, _elements) {
            if (crst->type() == CHORD) {
                  Chord* c = static_cast<Chord*>(crst);
                  printf(" chord %p tick=%d durtype=%d beams=%d\n", c, c->tick(), c->duration().type(), c->beams());
                  }
            else if (crst->type() == REST) {
                  Rest* r = static_cast<Rest*>(crst);
                  printf(" rest %p tick=%d durtype=%d beams=%d\n", r, r->tick(), r->duration().type(), r->beams());
                  }
            else {
                  printf(" type=%d %p tick=%d\n", crst->type(), crst, crst->tick());
                  }
            }
      // end dump beam contents
*/
      int idx = _elements.indexOf(cr);
      if (idx == -1) {
            printf("Beam::writeMusicXml(): cannot find ChordRest\n");
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
//      printf(" blp=%d blc=%d bln=%d\n", blp, blc, bln);
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

      maxDuration.setType(Duration::V_INVALID);
      c1 = 0;
      c2 = 0;
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
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Beam::layout()
      {
      if (_elements.isEmpty() || !c1 || !c2) {
            printf("Beam::layout: no notes\n");
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

      _bbox = QRectF();
      qreal lw2 = point(score()->styleS(ST_beamWidth)) * .5 * mag();
      foreach(const QLineF* bs, beamSegments) {
            QPolygonF a(4);
            a[0] = QPointF(bs->x1(), bs->y1()-lw2);
            a[1] = QPointF(bs->x2(), bs->y2()-lw2);
            a[2] = QPointF(bs->x2(), bs->y2()+lw2);
            a[3] = QPointF(bs->x1(), bs->y1()+lw2);
            _bbox |= a.boundingRect();
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
            QPolygonF a(4);
            a[0] = QPointF(bs->x1(), bs->y1()-lw2);
            a[1] = QPointF(bs->x2(), bs->y2()-lw2);
            a[2] = QPointF(bs->x2(), bs->y2()+lw2);
            a[3] = QPointF(bs->x1(), bs->y1()+lw2);
            pp.addRect(a.boundingRect());
            pp.closeSubpath();
            }
      return pp;
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Beam::contains(const QPointF& p) const
      {
      return shape().contains(p - canvasPos());
      }

//---------------------------------------------------------
//   layout2
//---------------------------------------------------------

void Beam::layout2(QList<ChordRest*>crl, SpannerSegmentType st, int frag)
      {
      Chord* c1 = 0;
      Chord* c2 = 0;
      foreach(ChordRest* cr, crl) {
            if (cr->type() == CHORD) {
                  if (c1 == 0)
                        c1 = static_cast<Chord*>(cr);
                  c2 = static_cast<Chord*>(cr);
                  }
            }
      if (c1 == 0)
            return;
      BeamFragment* f = fragments[frag];
      int idx         = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      QPointF cp      = canvasPos();
      double _spatium = spatium();
      double p1x      = c1->upNote()->canvasPos().x();
      double p2x      = c2->upNote()->canvasPos().x();
      cut             = 0;

      f->p1[idx] += cp;
      f->p2[idx] += cp;

      if (_userModified[idx]) {
            double beamY = f->p1[idx].y();
            slope        = (f->p2[idx].y() - f->p1[idx].y()) / (p2x - p1x);
            //
            // set stem direction for every chord
            //
            foreach(ChordRest* cr, crl) {
                  if (cr->type() != CHORD)
                        continue;
                  Chord* c  = static_cast<Chord*>(cr);
                  QPointF p = c->upNote()->canvasPos();
                  double y1 = beamY + (p.x() - p1x) * slope;
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
            double beamY   = 0.0;  // y position of main beam start
            double y1   = -100000;
            double y2   = 100000;
            foreach(ChordRest* cr, crl) {
                  if (cr->type() != CHORD)
                        continue;
                  Chord* c = static_cast<Chord*>(cr);
                  double y = c->upNote()->canvasPos().y();
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
                  double y  = c->upNote()->canvasPos().y();
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
            bool concave = false;
            for (int i = 0; i < crl.size() - 2; ++i) {
                  int l1 = crl[i]->line(_up);
                  int l  = crl[i+1]->line(_up);
                  int l2 = crl[i+2]->line(_up);

                  concave = ((l1 < l2) && ((l < l1) || (l > l2)))
                    || ((l1 > l2) && ((l > l1) || (l < l2)));
                  if (concave)
                        break;
                  }
            int l1 = crl.front()->line(_up);
            int l2 = crl.back()->line(_up);

            if (!concave) {
                  const ChordRest* a1 = crl.front();
                  const ChordRest* a2 = crl.back();
                  double dx = a2->canvasPos().x() - a1->canvasPos().x();
                  double maxSlope = score()->style(ST_beamMaxSlope).toDouble();
                  if (dx) {
                        slope = (l2 - l1) * _spatium * .5 / dx;
                        if (fabs(slope) < score()->style(ST_beamMinSlope).toDouble()) {
                              cut = slope > 0.0 ? 0 : -1;
                              slope = 0;
                              }
                        else if (slope > maxSlope) {
                              slope = maxSlope;
                              cut = 1;
                              }
                        else if (-slope > maxSlope) {
                              slope = -maxSlope;
                              cut = -1;
                              }
                        }
                  }
            cut *= (_up ? 1 : -1);
            }

      //---------------------------------------------------
      //    create beam segments
      //---------------------------------------------------

      double xoffLeft  = point(score()->styleS(ST_stemWidth)) * .5;
      double xoffRight = xoffLeft;
      double x1        = c1->stemPos(c1->up(), false).x() - xoffLeft;
      double x2        = c2->stemPos(c2->up(), false).x() + xoffRight;

      f->p1[idx].rx() = x1;
      f->p2[idx].rx() = x2;

      const Style s(score()->style());
      double bd(s[ST_beamDistance].toDouble());
      Spatium bw(s[ST_beamWidth].toSpatium());
      double beamMinLen = point(s[ST_beamMinLen].toSpatium());
      double graceMag   = score()->styleD(ST_graceNoteMag);
      double beamDist   = point(bd * bw + bw) * (_up ? 1.0 : -1.0);
      if (isGrace)
            beamDist *= graceMag;

      if (!_userModified[idx]) {
            //
            // compute final y position position of 1/8 beam
            //

            if (cross) {
                  double yDownMax   = -100000;
                  double yUpMin = 100000;
                  foreach(ChordRest* cr, crl) {
                        if (cr->type() != CHORD)
                              continue;
                        double y;
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
                  double ys = (x2 - x1) * slope;

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
                  double min =  1000.0;
                  double max = -1000.0;
                  foreach(ChordRest* cr, crl) {
                        if (cr->type() != CHORD)
                              continue;
                        Chord* chord  = static_cast<Chord*>(cr);
                        QPointF npos(chord->stemPos(_up, true));
                        // grow beams with factor 0.5
                        double bd      = (chord->beams() - 1) * beamDist * .5 * (_up ? 1.0 : -1.0);
                        double y1      = npos.y();
                        double y2      = f->p1[idx].y() + (npos.x() - x1) * slope;
                        double stemLen = _up ? (y1 - y2) : (y2 - y1);
                        stemLen -= bd;
                        if (stemLen < min)
                              min = stemLen;
                        if (stemLen > max)
                              max = stemLen;
                        }
                  // adjust beam position
                  double n = 3.0;
                  if (fabs(max-min) > (_spatium * 2.0))
                        n = 2.0;    // reduce minimum stem len (heuristic)
                  if (isGrace)
                        n *= graceMag;
                  double diff = n * _spatium - min;
                  if (_up)
                        diff = -diff;
                  f->p1[idx].ry() += diff;
                  f->p2[idx].ry() += diff;
                  }
            }
      f->p1[idx] -= cp;
      f->p2[idx] -= cp;

      if (isGrace) {
            setMag(graceMag);
            bw         *= graceMag;
            beamMinLen *= graceMag;
            }
      else
            setMag(1.0);

      //---------------------------------------------
      //   create beam segments
      //---------------------------------------------

      qreal stemWidth2 = point(score()->styleS(ST_stemWidth)) * .5;
      double p1dy = f->p1[idx].y();
      int beamNo = 0;
      for (Duration d(Duration::V_EIGHT); d >= Duration(Duration::V_256TH); d = d.shift(1)) {
            Chord* cr1 = 0;
            Chord* cr2 = 0;

            double dist = beamDist * beamNo;
            double y1   = p1dy + dist;

            foreach (ChordRest* cr, crl) {
                  if (cr->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(cr);
                  bool b32 = (beamNo >= 1) && (chord->beamMode() == BEAM_BEGIN32);
                  bool b64 = (beamNo >= 2) && (chord->beamMode() == BEAM_BEGIN64);
                  if ((chord->durationType().type() < d.type()) || b32 || b64) {
                        if (cr2) {
                              // create short segment
                              double x2 = cr1->stemPos(cr1->up(), false).x();
                              double x3 = cr2->stemPos(cr2->up(), false).x();
                              beamSegments.push_back(new QLineF(x2 - cp.x(), (x2 - x1) * slope + y1,
                                 x3 - cp.x(), (x3 - x1) * slope + y1));
                              }
                        else if (cr1) {
                              // create broken segment
                              double len = beamMinLen;
                              if (cr1 != crl[0]) {
                                    Duration d = cr1->durationType();
                                    d = d.shift(-1);
                                    int rtick = cr1->tick() - cr1->measure()->tick();
                                    if (rtick % d.ticks())
                                          len = -len;
                                    }
                              double x2 = cr1->stemPos(cr1->up(), false).x();
                              double x3 = x2 + len;
                              beamSegments.push_back(new QLineF(x2 - cp.x(), (x2 - x1) * slope + y1,
                                 x3 - cp.x(), (x3 - x1) * slope + y1));
                              }
                        if (chord->durationType().type() >= d.type()) {
                              cr1 = chord;
                              cr2 = 0;
                              }
                        else {
                              cr1 = cr2 = 0;
                              }
                        }
                  else
                        (cr1 ? cr2 : cr1) = chord;
                  }
            if (cr2) {
                  // create segment
                  double x2 = cr1->stemPos(cr1->up(), false).x();
                  double x3 = cr2->stemPos(cr2->up(), false).x();

                  if (st == SEGMENT_BEGIN)
                        x3 += _spatium * 2;
                  else if (st == SEGMENT_END)
                        x2 -= _spatium * 2;
                  else {
                        x2 -= stemWidth2;
                        x3 += stemWidth2;
                        }

                  beamSegments.push_back(new QLineF(x2 - cp.x(), (x2 - x1) * slope + p1dy + dist * _grow1,
                     x3 - cp.x(), (x3 - x1) * slope + p1dy + dist  * _grow2));
                  }
            else if (cr1) {
                  // create broken segment
                  double x3 = cr1->stemPos(cr1->up(), false).x();
                  double x2 = x3 - beamMinLen;
                  beamSegments.push_back(new QLineF(x2 - cp.x(), (x2 - x1) * slope + p1dy + dist,
                     x3 - cp.x(), (x3 - x1) * slope + p1dy + dist));
                  }
            ++beamNo;
            }

      //---------------------------------------------------
      //    create stem's
      //---------------------------------------------------

      foreach (ChordRest* cr, crl) {
            if (cr->type() != CHORD)
                  continue;
            Chord* chord = static_cast<Chord*>(cr);
            bool _up = chord->up();

            Stem* stem = chord->stem();
            if (!stem) {
                  stem = new Stem(score());
                  chord->setStem(stem);
                  }
            chord->setHook(0);

            QPointF npos(chord->stemPos(_up, false));

            double x2 = npos.x();
            double y1 = npos.y();
            double y  = _up ? qMin(qreal(p1dy), f->p1[idx].y()) : qMax(p1dy, f->p1[idx].y());
            double y2 = y + (x2 - x1) * slope + cp.y();

            double stemLen = _up ? (y1 - y2) : (y2 - y1);
            stem->setLen(stemLen);

            if (_up)
                  npos.ry() -= stemLen;
            stem->setPos(npos - chord->canvasPos());
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
            double _spatium = spatium();
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
      _id = e.attribute("id").toInt();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "y1") {
                  if (fragments.isEmpty())
                        fragments.append(new BeamFragment);
                  BeamFragment* f = fragments.back();
                  int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  f->p1[idx] = QPointF(0.0, val.toDouble());
                  }
            else if (tag == "y2") {
                  if (fragments.isEmpty())
                        fragments.append(new BeamFragment);
                  BeamFragment* f = fragments.back();
                  int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  f->p2[idx] = QPointF(0.0, val.toDouble());
                  }
            else if (tag == "Fragment") {
                  BeamFragment* f = new BeamFragment;
                  int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  double _spatium = spatium();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        double v = ee.text().toDouble() * _spatium;
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

void Beam::editDrag(int grip, const QPointF& delta)
      {
      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      QPointF d(0.0, delta.y());
      BeamFragment* f = fragments[editFragment];
      if (grip == 0)
            f->p1[idx] += d;
      f->p2[idx] += d;
      _userModified[idx] = true;
      setGenerated(false);
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
      grip[0].translate(canvasPos() + f->p1[idx]);
      grip[1].translate(canvasPos() + f->p2[idx]);
      }

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void Beam::setBeamDirection(Direction d)
      {
      _direction = d;
      if (d != AUTO)
            _up = d == UP;
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

void Beam::startEdit(ScoreView*, const QPointF& p)
      {
      QPointF pt(p - canvasPos());
      double ydiff = 100000000.0;
      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      int i = 0;
      editFragment = 0;
      foreach (BeamFragment* f, fragments) {
            double d = fabs(f->p1[idx].y() - pt.y());
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

bool Beam::acceptDrop(ScoreView*, const QPointF&, int type, int subtype) const
      {
      return (type == ICON && subtype == ICON_FBEAM1)
         || (type == ICON && subtype == ICON_FBEAM2);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Beam::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
      double g1;
      double g2;
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


