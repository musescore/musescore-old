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

//---------------------------------------------------------
//   startBeam
//---------------------------------------------------------

#if 0
static BeamHint startBeamList[] = {
      BeamHint(0, 0, 0, 0, 0, 0),
      };
#endif

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
//---------------------------------------------------------

static bool endBeam(int tsZ, int tsN, ChordRest* cr, int p)
      {
      if (cr->tuplet()) {
            if (cr->tuplet()->elements().front() == cr) {
                  return true;
                  }
            return false;
            }
      int l = cr->ticks();
      for (unsigned i = 0; i < sizeof(endBeamList)/sizeof(*endBeamList); ++i) {
            const BeamHint& h = endBeamList[i];
            if (h.timeSigZ && (h.timeSigZ != tsZ || h.timeSigN != tsN))
                  continue;
            if (h.noteLenZ) {
                  int len = (4 * h.noteLenZ * AL::division)/ h.noteLenN;
                  if (len != l)
                        continue;
                  }
            if (h.posZ) {
                  int pos = (4 * h.posZ * AL::division) / h.posN;
                  if (pos != p)
                        continue;
                  }
            if (h.posZ == 0) {
                  // stop on every beat
                  int len = (4 * AL::division) / h.timeSigN;
                  if (p % len)
                        continue;
                  }

/*   printf("  endBeam(pos %d len %d  ts %d/%d: table-note: %d/%d  table-pos:%d/%d\n",
      p, l,
      tsZ, tsN,
      h.noteLenZ, h.noteLenN,
      h.posZ, h.posN);
*/
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
      _direction    = AUTO;
      _up           = -1;
      _userModified[0] = false;
      _userModified[1] = false;
      }

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(const Beam& b)
   : Element(b)
      {
      _elements     = b._elements;
      foreach(BeamSegment* bs, b.beamSegments)
            beamSegments.append(new BeamSegment(*bs));
      _direction       = b._direction;
      _up              = b._up;
      _userModified[0] = b._userModified[0];
      _userModified[1] = b._userModified[1];
      _p1[0]           = b._p1[0];
      _p1[1]           = b._p1[1];
      _p2[0]           = b._p2[0];
      _p2[1]           = b._p2[1];
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
      for (iBeamSegment i = beamSegments.begin(); i != beamSegments.end(); ++i)
            delete *i;
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Beam::canvasPos() const
      {
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
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

void Beam::draw(QPainter& p) const
      {
      p.setPen(QPen(Qt::NoPen));
      p.setBrush(selected() ? preferences.selectColor[0] : preferences.defaultColor);

      for (ciBeamSegment ibs = beamSegments.begin();
         ibs != beamSegments.end(); ++ibs) {
            BeamSegment* bs = *ibs;

            QPointF ip1 = bs->p1;
            QPointF ip2 = bs->p2;
            qreal lw2   = point(score()->styleS(ST_beamWidth)) * .5 * mag();

            QPolygonF a(4);
            a[0] = QPointF(ip1.x(), ip1.y()-lw2);
            a[1] = QPointF(ip2.x(), ip2.y()-lw2);
            a[2] = QPointF(ip2.x(), ip2.y()+lw2);
            a[3] = QPointF(ip1.x(), ip1.y()+lw2);
            p.drawPolygon(a);
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Beam::move(double x, double y)
      {
      Element::move(x, y);
      for (ciBeamSegment ibs = beamSegments.begin();
         ibs != beamSegments.end(); ++ibs) {
            BeamSegment* bs = *ibs;
            bs->move(x, y);
            }
      }

//---------------------------------------------------------
//   layoutBeams1
//    auto - beamer
//    called before layout spacing of notes
//---------------------------------------------------------

void Measure::layoutBeams1(int track)
      {
      ChordRest* a1 = 0;      // start of (potential) beam
      Beam* beam    = 0;
      Beam* oldBeam = 0;

      for (Segment* segment = first(); segment; segment = segment->next()) {
            Element* e = segment->element(track);
            if (((segment->subtype() != Segment::SegChordRest) && (segment->subtype() != Segment::SegGrace))
               || e == 0 || !e->isChordRest())
                  continue;
            ChordRest* cr = static_cast<ChordRest*>(e);
            if (cr->type() == CHORD)
                  static_cast<Chord*>(cr)->computeUp();
            if (segment->subtype() == Segment::SegGrace) {
                  Segment* nseg = segment->next();
                  if (nseg && nseg->subtype() == Segment::SegGrace && nseg->element(track)) {
                        Beam* b = cr->beam();
                        if (b == 0) {
                              b = new Beam(score());
                              b->setTrack(track);
                              b->setGenerated(true);
                              add(b);
                              }
                        b->add(cr);
                        Segment* s = nseg;
                        for (;;) {
                              nseg = s;
                              ChordRest* cr = static_cast<ChordRest*>(nseg->element(track));
                              b->add(cr);
                              s = nseg->next();
                              if (!s || (s->subtype() != Segment::SegGrace) || !s->element(track))
                                    break;
                              }
                        b->layout1();
                        segment = nseg;
                        }
                  else {
                        cr->setBeam(0);
                        cr->layoutStem1();
                        }
                  continue;
                  }
            BeamMode bm = cr->beamMode();
            int len     = cr->duration().ticks();

            if ((len >= AL::division) || (bm == BEAM_NO)) {
                  if (beam) {
                        beam->layout1();
                        beam = 0;
                        }
                  if (a1) {
                        a1->setBeam(0);
                        a1->layoutStem1();
                        a1 = 0;
                        }
                  cr->setBeam(0);
                  cr->layoutStem1();
                  continue;
                  }
            bool beamEnd = false;
            if (beam) {
                  ChordRest* le = beam->elements().back();
                  if (((bm != BEAM_MID) && (le->tuplet() != cr->tuplet())) || (bm == BEAM_BEGIN)) {
                        beamEnd = true;
                        }
                  else if (bm != BEAM_MID) {
                        int z, n;
                        _score->sigmap()->timesig(cr->tick(), z, n);
                        if (endBeam(z, n, cr, cr->tick() - tick()))
                              beamEnd = true;
                        }
                  if (beamEnd) {
                        beam->layout1();
                        beam = 0;
                        a1   = 0;
                        }
                  else {
                        beam->add(cr);
                        cr = 0;

                        // is this the last beam element?
                        if (bm == BEAM_END) {
                              beam->layout1();
                              beam = 0;
                              }
                        }
                  }
            if (cr && cr->tuplet() && (cr->tuplet()->elements().back() == cr)) {
                  if (beam) {
                        beam->layout1();
                        beam = 0;

                        cr->setBeam(0);
                        cr->layoutStem1();
                        }
                  else if (a1) {
                        beam = a1->beam();
                        if (beam == 0 || (beam == oldBeam)) {
                              beam = new Beam(score());
                              beam->setTrack(track);
                              beam->setGenerated(true);
                              add(beam);
                              }
                        oldBeam = beam;
                        beam->add(a1);
                        beam->add(cr);
                        a1 = 0;
                        beam->layout1();
                        beam = 0;
                        }
                  else {
                        cr->setBeam(0);
                        cr->layoutStem1();
                        }
                  }
            else if (cr) {
                  if (a1 == 0)
                        a1 = cr;
                  else {
                        int z, n;
                        _score->sigmap()->timesig(cr->tick(), z, n);
                        if (bm != BEAM_MID
                           &&
                             (endBeam(z, n, cr, cr->tick() - tick())
                             || bm == BEAM_BEGIN
                             || (a1->segment()->subtype() != cr->segment()->subtype())
                             )
                           ) {
                              a1->setBeam(0);
                              a1->layoutStem1();      //?
                              a1 = cr;
                              }
                        else {
                              beam = a1->beam();
                              if (beam == 0 || (beam == oldBeam)) {
                                    beam = new Beam(score());
                                    beam->setGenerated(true);
                                    beam->setTrack(track);
                                    add(beam);
                                    }
                              oldBeam = beam;
                              beam->add(a1);
                              beam->add(cr);
                              a1 = 0;
                              }
                        }
                  }
            }
      if (beam)
            beam->layout1();
      else if (a1) {
            a1->setBeam(0);
            a1->layoutStem1();
            }
      }

//---------------------------------------------------------
//   layoutBeams
//    auto - beamer
//    called after layout spacing of notes
//---------------------------------------------------------

void Measure::layoutBeams()
      {
      int nstaves = _score->nstaves();
      int tracks = nstaves * VOICES;

      foreach(Beam* beam, _beams)
            beam->layout();

      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);
                  if (e && e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (cr->beam())
                              continue;
                        cr->layoutStem();
                        }
                  }
            }
      //
      //    Experimental
      //    check for special case:
      //    - triplet with middle chord in different staff
      //      (cross staff beam)
      //    Apply optical correction
      //
      double _spatium = spatium();
      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);
                  if (e && e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (!cr->beam() || !cr->tuplet())
                              continue;
                        Tuplet* tuplet = cr->tuplet();
                        const QList<DurationElement*> el = tuplet->elements();

                        if ((el.size() == 3)
                           && (el[0]->isChordRest())
                           && (el[1]->isChordRest())
                           && (el[2]->isChordRest())
                           && (
                              static_cast<ChordRest*>(el[0])->staffMove()
                              || static_cast<ChordRest*>(el[1])->staffMove()
                              || static_cast<ChordRest*>(el[2])->staffMove()
                              )
                           ) {
                              int m0 = static_cast<ChordRest*>(el[0])->staffMove();
                              int m1 = static_cast<ChordRest*>(el[1])->staffMove();
                              int m2 = static_cast<ChordRest*>(el[2])->staffMove();
                              //
                              //  todo: use note head width instead of _spatium
                              //
                              if (m1 && (m0 == 0) && (m2 == 0))
                                    el[1]->setUserXoffset(m1 > 0 ? -_spatium : _spatium);
                              else if (m0 && m2 && (m1 == 0) && (m0 == m2))
                                    el[1]->setUserXoffset(m0 > 0 ? _spatium : -_spatium);
                              }

                        // skip tuplet
                        while (tuplet->elements().back()->type() == TUPLET)
                              tuplet = static_cast<Tuplet*>(tuplet->elements().back());
                        segment = static_cast<ChordRest*>(tuplet->elements().back())->segment();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   xmlType
//---------------------------------------------------------

QString Beam::xmlType(ChordRest* cr) const
      {
      if (cr == _elements.front())
            return QString("begin");
      if (cr == _elements.back())
            return QString("end");
      int idx = _elements.indexOf(cr);
      if (idx == -1)
            printf("Beam::xmlType(): cannot find ChordRest\n");
      return QString("continue");
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Beam::layout1()
      {
      //delete old segments
      for (iBeamSegment i = beamSegments.begin(); i != beamSegments.end(); ++i)
            delete *i;
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
            if (!maxDuration.isValid() || (maxDuration < cr->duration()))
                  maxDuration = cr->duration();
            }
      _up     = (_direction == AUTO) ? (upCount >= 0) : (_direction == UP);
      cross   = minMove < maxMove;
      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      slope   = 0.0;

      if (cross && !_userModified[idx]) {
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
      else if (_userModified[idx]) {
            double p1x   = c1->upNote()->canvasPos().x();
            double p2x   = c2->upNote()->canvasPos().x();
            double beamY = _p1[idx].y() + c1->upNote()->chord()->canvasPos().y();
            slope        = (_p2[idx].y() - _p1[idx].y()) / (p2x - p1x);
            //
            // set stem direction for every chord
            //
            foreach(ChordRest* cr, _elements) {
                  if (cr->type() != CHORD)
                        continue;
                  Chord* c  = static_cast<Chord*>(cr);
                  QPointF p = c->upNote()->canvasPos();
                  double y1 = beamY + (p.x() - p1x) * slope;
                  cr->setUp(y1 < p.y());
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
      double _spatium = spatium();
      double p1x      = c1->upNote()->canvasPos().x();
      double p2x      = c2->upNote()->canvasPos().x();
      int cut         = 0;

      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      if (_userModified[idx]) {
            double beamY = _p1[idx].y() + c1->upNote()->chord()->canvasPos().y();
            slope        = (_p2[idx].y() - _p1[idx].y()) / (p2x - p1x);
            //
            // set stem direction for every chord
            //
            foreach(ChordRest* cr, _elements) {
                  if (cr->type() != CHORD)
                        continue;
                  Chord* c  = static_cast<Chord*>(cr);
                  QPointF p = c->upNote()->canvasPos();
                  double y1 = beamY + (p.x() - p1x) * slope;
                  cr->setUp(y1 < p.y());
                  }
            _up = -1;
            }
      else if (cross) {
            double beamY   = 0.0;  // y position of main beam start
            double y1   = -100000;
            double y2   = 100000;
            foreach(ChordRest* cr, _elements) {
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
            _p1[idx].ry() = beamY;
            _p2[idx].ry() = beamY;
            //
            // set stem direction for every chord
            //
            foreach(ChordRest* cr, _elements) {
                  if (cr->type() != CHORD)
                        continue;
                  Chord* c  = static_cast<Chord*>(cr);
                  double y  = c->upNote()->canvasPos().y();
                  c->setUp(beamY < y);
                  }
            _up = -1;
            }
      else {
            bool concave = false;
            for (int i = 0; i < _elements.size() - 2; ++i) {
                  int l1 = _elements[i]->line(_up);
                  int l  = _elements[i+1]->line(_up);
                  int l2 = _elements[i+2]->line(_up);

                  concave = ((l1 < l2) && ((l < l1) || (l > l2)))
                    || ((l1 > l2) && ((l > l1) || (l < l2)));
                  if (concave)
                        break;
                  }
            int l1 = _elements.front()->line(_up);
            int l2 = _elements.back()->line(_up);

            if (!concave) {
                  const ChordRest* a1 = _elements.front();
                  const ChordRest* a2 = _elements.back();
                  double dx = (a2->pos().x() + a2->segment()->pos().x())
                          - (a1->pos().x() + a1->segment()->pos().x());
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
      _p1[idx].rx()    = x1;
      _p2[idx].rx()    = x2;

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
                  foreach(ChordRest* cr, _elements) {
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
                  _p1[idx].ry() = _p2[idx].ry() = yUpMin + (yDownMax - yUpMin) * .5;
                  }
            else {
                  QPointF p1s(c1->stemPos(c1->up(), false));
                  QPointF p2s(c2->stemPos(c2->up(), false));
                  double ys = (x2 - x1) * slope;

                  if (cut >= 0) {
                        // left dot is reference
                        _p1[idx].ry() = p1s.y();
                        _p2[idx].ry() = _p1[idx].y() + ys;
                        }
                  else {
                        // right dot is reference
                        _p2[idx].ry() = p2s.y();
                        _p1[idx].ry() = _p2[idx].y() - ys;
                        }
                  double min =  1000.0;
                  double max = -1000.0;
                  foreach(ChordRest* cr, _elements) {
                        if (cr->type() != CHORD)
                              continue;
                        Chord* chord  = static_cast<Chord*>(cr);
                        QPointF npos(chord->stemPos(_up, true));
                        // grow beams with factor 0.5
                        double bd      = (chord->beams() - 1) * beamDist * .5 * (_up ? 1.0 : -1.0);
                        double y1      = npos.y();
                        double y2      = _p1[idx].y() + (npos.x() - x1) * slope;
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
                  _p1[idx].ry() += diff;
                  _p2[idx].ry() += diff;
                  }
            }

      if (isGrace) {
            setMag(graceMag);
            bw         *= graceMag;
            beamMinLen *= graceMag;
            }
      else
            setMag(1.0);

      int n = maxDuration.hooks();
      for (int i = 0; i < n; ++i) {
            BeamSegment* bs = new BeamSegment(_p1[idx], _p2[idx]);
            bs->move(0, beamDist * i);
            beamSegments.push_back(bs);
            }
      double p1dy = _p1[idx].y() + beamDist * (n-1);

      //---------------------------------------------
      //   create broken/short beam segments
      //---------------------------------------------

      for (Duration d(maxDuration.shift(1)); d >= Duration(Duration::V_64TH); d = d.shift(1)) {
            int nn = d.hooks() - n;

            Chord* nn1 = 0;
            Chord* nn2 = 0;
            bool nn1r  = false;
            double y1  = 0.0;
            foreach(ChordRest* cr, _elements) {
                  if (cr->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(cr);
                  bool cup = chord->up();
                  if (cross) {
                        if (!cup)
                              y1 = _p1[idx].y() - beamDist * nn;
                        else
                              y1 = p1dy    + beamDist * nn;
                        }
                  else
                        y1 = p1dy + beamDist * nn;

                  if (chord->duration() > d) {
                        if (nn2) {
                              // create short segment
                              BeamSegment* bs = new BeamSegment;
                              beamSegments.push_back(bs);
                              double x2 = nn1->stemPos(cup, false).x();
                              double x3 = nn2->stemPos(cup, false).x();
                              bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                              bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                              }
                        else if (nn1) {
                              // create broken segment
                              bool toRight;
                              if (nn1 == _elements[0])
                                    toRight = true;
                              else {
                                    Duration d = nn1->duration();
                                    d = d.shift(-1);
                                    int rtick = nn1->tick() - nn1->measure()->tick();
                                    if (rtick % d.ticks())
                                          toRight = false;
                                    else
                                          toRight = true;
                                    }
                              BeamSegment* bs = new BeamSegment;
                              beamSegments.push_back(bs);
                              double x2 = nn1->stemPos(cup, false).x();
                              double x3 = x2 + (toRight ? beamMinLen : -beamMinLen);
                              bs->p1    = QPointF(x2, (x2 - x1) * slope + y1);
                              bs->p2    = QPointF(x3, (x3 - x1) * slope + y1);
                              }
                        nn1r = false;
                        nn1 = nn2 = 0;
                        continue;
                        }
                  nn1r = false;
                  if (nn1)
                        nn2 = chord;
                  else {
                        nn1 = chord;
                        nn1r = cr == _elements.front();
                        }
                  }
            if (nn2) {
                  // create short segment
                  BeamSegment* bs = new BeamSegment;
                  beamSegments.push_back(bs);
                  double x2 = nn1->stemPos(nn1->up(), false).x();
                  double x3 = nn2->stemPos(nn2->up(), false).x();
                  bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                  bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                  }
           else if (nn1) {
                  // create broken segment
                  BeamSegment* bs = new BeamSegment;
                  beamSegments.push_back(bs);
                  double x3 = nn1->stemPos(nn1->up(), false).x();
                  double x2 = x3 - point(score()->styleS(ST_beamMinLen));
                  bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                  bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                  }
            }

      //---------------------------------------------------
      //    create stem's
      //    stem Pos() is relative to Chord
      //---------------------------------------------------

      foreach(ChordRest* cr, _elements) {
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
            double y  = _up ? qMin(p1dy, _p1[idx].y()) : qMax(p1dy, _p1[idx].y());
            double y2 = y + (x2 - x1) * slope;

            double stemLen = _up ? (y1 - y2) : (y2 - y1);
            stem->setLen(stemLen);

            if (_up)
                  npos.ry() -= stemLen;
            stem->setPos(npos - chord->pos() - chord->segment()->pos());
            Tremolo* tremolo = chord->tremolo();
            if (tremolo)
                  tremolo->layout();
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Beam::bbox() const
      {
      QRectF r;
      for (ciBeamSegment ibs = beamSegments.begin();
         ibs != beamSegments.end(); ++ibs) {
            BeamSegment* bs = *ibs;
            r |= QRectF(bs->p1, QSizeF(1.0, 1.0));
            r |= QRectF(bs->p2, QSizeF(1.0, 1.0));
            }
      return r;
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
            xml.tag("y1", _p1[idx].y());
            xml.tag("y2", _p2[idx].y());
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Beam::read(QDomElement e)
      {
      QPointF p1, p2;
      bool modified = false;
      _id = e.attribute("id").toInt();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "y1") {
                  modified = true;
                  p1 = QPointF(0.0, val.toDouble());
                  }
            else if (tag == "y2") {
                  modified = true;
                  p2 = QPointF(0.0, val.toDouble());
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
            else if (!Element::readProperties(e))
                  domError(e);
            }
      if (modified) {
            int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
            _userModified[idx] = true;
            _p1[idx] = p1;
            _p2[idx] = p2;
            }
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Beam::editDrag(int grip, const QPointF& delta)
      {
      int idx = (_direction == AUTO || _direction == DOWN) ? 0 : 1;
      QPointF d(0.0, delta.y());
      if (grip == 0)
            _p1[idx] += d;
      _p2[idx] += d;
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
      grip[0].translate(canvasPos() + _p1[idx]);
      grip[1].translate(canvasPos() + _p2[idx]);
      }

//---------------------------------------------------------
//   isUp
//---------------------------------------------------------

bool Beam::isUp()
      {
      return _up;
      }

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

