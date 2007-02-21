//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: beam.cpp,v 1.41 2006/09/15 09:34:57 wschweer Exp $
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

#include "beam.h"
#include "segment.h"
#include "score.h"
#include "chord.h"
#include "preferences.h"
#include "sig.h"
#include "painter.h"
#include "style.h"
#include "note.h"
#include "tuplet.h"

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::~Beam()
      {
      //
      // delete all references from chords
      //
      for (iChordRest i = elements.begin(); i != elements.end(); ++i) {
            ChordRest* cr = i->second;
            cr->setBeam(0);
            }
      for (iBeamSegment i = beamSegments.begin();
         i != beamSegments.end(); ++i) {
            delete *i;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Beam::remove(ChordRest* a)
      {
      for (iChordRest i = elements.begin(); i != elements.end(); ++i) {
            if (i->second == a) {
                  elements.erase(i);
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Beam::draw1(Painter& p)
      {
      p.setPen(QPen(Qt::NoPen));
      p.setBrush(selected() ? preferences.selectColor[0] : Qt::black);

      for (ciBeamSegment ibs = beamSegments.begin();
         ibs != beamSegments.end(); ++ibs) {
            BeamSegment* bs = *ibs;

            QPointF ip1 = bs->p1;
            QPointF ip2 = bs->p2;
            qreal lw2   = point(style->beamWidth) * .5;

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
//   layoutBeams
//    layout beams and stems
//    auto - beamer
//---------------------------------------------------------

void Measure::layoutBeams()
      {
      for (iBeam i = _beamList.begin(); i != _beamList.end(); ++i)
            delete *i;
      _beamList.clear();

      int tracks = _score->nstaves() * VOICES;
      for (int track = 0; track < tracks; ++track) {
            ChordRest* a1 = 0;      // start of (potential) beam
            Beam* beam    = 0;
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);
                  if (e == 0)
                        continue;
                  if (e->type() == CLEF) {
                        if (beam) {
                              beam->layout();
                              beam = 0;
                              }
                        if (a1) {
                              a1->layoutStem();
                              a1 = 0;
                              }
                        continue;
                        }
                  if (!e->isChordRest())
                        continue;
                  ChordRest* cr = (ChordRest*) e;
                  BeamMode bm   = cr->beamMode();

                  //---------------------------------------
                  //   check for beam end
                  //---------------------------------------

                  bool tooLong = cr->tickLen() >= division;
                  if (beam) {
                        int group = division;
                        int z, n;
                        _score->sigmap->timesig(cr->tick(), z, n);
                        if (z == 3 && n == 8)   // hack!
                              group = division * 3 / 2;
                        else if (z == 9 && n == 8)
                              group = division * 3 / 2;
                        else if (z == 4 && n == 4)
                              group = division;

                        bool styleBreak = (cr->tick() % group) == 0;
                        bool hintBreak  = bm == BEAM_BEGIN || bm == BEAM_NO;

                        bool mustBreak = (bm != BEAM_MID)
                                      && (bm != BEAM_END)
                                      && (styleBreak || tooLong || hintBreak);
                        if (mustBreak) {
                              beam->layout();
                              beam = 0;
                              a1   = 0;
                              goto newBeam;
                              }
                        else {
                              cr->setBeam(beam);
                              beam->add(cr);

                              // is this the last beam element?
                              if (bm == BEAM_END) {
                                    beam->layout();
                                    beam = 0;
                                    a1   = 0;
                                    }
                              }
                        }
                  else {
newBeam:
                        bool hint = !tooLong;	  // start new beam
                        if (bm == BEAM_NO)
                              hint = false;
                        else if (bm == BEAM_BEGIN)
                              hint = true;
                        else if (bm == BEAM_AUTO) {
                        	// dont automatically start beam at a rest
                        	if (cr->type() == REST && a1 == 0)
                              	hint = false;
                              else if (a1 == 0) {
                                    //
                                    // start a new beam?
                                    //
                        		int group = cr->tickLen();
                        		int z, n;
                        		_score->sigmap->timesig(cr->tick(), z, n);

                                    // handle special time signatures:
                        		if (z == 9 && n == 8) {
                              		group = division * 3 / 2;
                                          }
                                    if (cr->tick() % group)
                                          hint = false;
                                    }
                              }
                        if (hint) {
                              if (a1 == 0) {
                                    a1 = cr;
                                    }
                              else {
                                    if (bm == BEAM_BEGIN) {
                                          a1->layoutStem();
                                          a1 = cr;
                                          }
                                    else {
                                          beam = new Beam(score());
                                          beam->setStaff(a1->staff());
                                          addBeam(beam);
                                          a1->setBeam(beam);
                                          beam->add(a1);
                                          cr->setBeam(beam);
                                          beam->add(cr);
                                          }
                                    }
                              }
                        else {
                              cr->layoutStem();
                              if (a1)
                                    a1->layoutStem();
                              a1 = 0;
                              }
                        }
                  }
            if (beam)
                  beam->layout();
            else if (a1)
                  a1->layoutStem();
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ChordRestList::add(ChordRest* n)
      {
      std::multimap<const int, ChordRest*, std::less<int> >::insert(std::pair<const int, ChordRest*> (n->tick(), n));
      }

//---------------------------------------------------------
//   xmlType
//---------------------------------------------------------

QString Beam::xmlType(ChordRest* cr) const
      {
      for (ciChordRest i = elements.begin(); i != elements.end(); ++i) {
            if (i->second == cr) {
                  if (i == elements.begin())
                        return QString("begin");
                  ++i;
                  if (i == elements.end())
                        return QString("end");
                  return QString("continue");
                  }
            }
      printf("Beam::xmlType(): cannot find ChordRest\n");
      return QString("continue");
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Beam::layout()
      {
      //delete old segments
      for (iBeamSegment i = beamSegments.begin(); i != beamSegments.end(); ++i)
            delete *i;
      beamSegments.clear();

      //---------------------------------------------------
      //   calculate direction of beam
      //    - majority
      //          number count of up or down notes
      //    - mean
      //          mean centre distance of all notes
      //    - median
      //          mean centre distance weighted per note
      //
      //   currently we use the "majority" method
      //---------------------------------------------------

      int upCount    = 0;
      int maxTickLen = 0;
      const ChordRest* a1  = elements.front();
      const ChordRest* a2  = elements.back();
      Chord* c1      = 0;
      Chord* c2      = 0;
      int move       = 0;
      int firstMove  = elements.front()->move();

// printf("Layout Beam\n");
      for (iChordRest i = elements.begin(); i != elements.end(); ++i) {
            ChordRest* cr = i->second;
            //
            // delete stem & flag for this chord
            //
            if (cr->type() == CHORD) {
                  c2 = (Chord*)(cr);
                  if (c1 == 0)
                        c1 = c2;
                  Chord* chord = c2;
                  chord->setStem(0);
                  chord->setHook(0);
                  //
                  // if only one stem direction is manually set it
                  // determines if beams are up or down
                  //
                  if (chord->stemDirection() != AUTO)
                        upCount += chord->stemDirection() == UP ? 1000 : -1000;
                  else
                        upCount += chord->isUp() ? 1 : -1;
                  if (chord->move()) {
                        if (firstMove == 0)
                              move = chord->move() * -1;
                        else
                              move = chord->move() * -1;
                        }
                  }
            int tl = cr->tickLen();
            Tuplet* tuplet = cr->tuplet();
            if (tuplet)
                  tl = tuplet->baseLen();
            if (tl > maxTickLen)
                  maxTickLen = tl;
            }

// printf("   move %d  firstMove %d\n", move, firstMove);

      bool upFlag = upCount >= 0;
      //
      //  move = 0     no cross beaming
      //       = -1    staff 1 - 2
      //       = 1     staff 2 - 1

      for (iChordRest i = elements.begin(); i != elements.end(); ++i) {
            ChordRest* cr = (ChordRest*)(i->second);
            if (move == 1)
                  cr->setUp(cr->move() == 0);
            else if (move == -1)
                  cr->setUp(cr->move() != 0);
            else
                  cr->setUp(upFlag);
            }

      if (move == 1)
            upFlag = true;
      else if (move == -1)
            upFlag = false;

      //------------------------------------------------------------
      //   calculate slope of beam
      //    - the slope is set to zero on "concave" chord sequences
      //------------------------------------------------------------

      bool concave = false;
      int l1 = a1->line(a1->isUp());
      int l2 = a2->line(a2->isUp());

      iChordRest i = elements.begin();
      ++i;
      for (;;) {
            ChordRest* cr = i->second;
            ++i;
            if (i == elements.end())
                  break;
            int l = cr->line(cr->isUp());
            concave = ((l1 < l2) && ((l < l1) || (l > l2)))
                    || ((l1 > l2) && ((l > l1) || (l < l2)));
            if (concave)
                  break;
            }


      int cut = 0;
      qreal slope = 0.0;

      if (!concave) {
            double dx = (a2->pos().x() + a2->segment()->pos().x())
                          - (a1->pos().x() + a1->segment()->pos().x());
            if (dx) {
                  slope = (l2 - l1) * _spatium * .5 / dx;
                  if (fabs(slope) < style->beamMinSlope) {
                        cut = slope > 0.0 ? 0 : -1;
                        slope = 0;
                        }
                  else if (slope > style->beamMaxSlope) {
                        slope = style->beamMaxSlope;
                        cut = 1;
                        }
                  else if (-slope > style->beamMaxSlope) {
                        slope = -style->beamMaxSlope;
                        cut = -1;
                        }
                  }
            }

      cut *= (upFlag ? 1 : -1);

      //---------------------------------------------------
      //    create beam segments
      //---------------------------------------------------

            //---------------------------------------------
            //   create top beam segment
            //---------------------------------------------

      double xoffLeft  = point(style->stemWidth)/2;
      double xoffRight = xoffLeft;

      QPointF p1s(a1->stemPos(a1->isUp(), false) + a1->pos() + a1->segment()->pos());
      QPointF p2s(a2->stemPos(a2->isUp(), false) + a2->pos() + a2->segment()->pos());
      double x1 = p1s.x() - xoffLeft;
      double x2 = p2s.x() + xoffRight;

      QPointF p1, p2;
      double ys = (x2 - x1) * slope;
      if (cut >= 0) {
            // linker Punkt ist Referenz
            p1 = QPointF(x1, p1s.y());
            p2 = QPointF(x2, p1.y() + ys);
            }
      else {
            // rechter Punkt ist Referenz
            p2 = QPointF(x2, p2s.y());
            p1 = QPointF(x1, p2.y() - ys);
            }

      //---------------------------------------------------
      // calculate min stem len
      //    adjust beam position if necessary
      //
      double beamDist = point(style->beamDistance * style->beamWidth
                        + style->beamWidth) * (upFlag ? 1.0 : -1.0);
      double min = 1000;
      double max = -1000;
      int lmove = elements.front()->move();
      for (iChordRest i = elements.begin(); i != elements.end(); ++i) {
            if (i->second->type() != CHORD)
                  continue;
            Chord* chord  = (Chord*)(i->second);
            if (chord->move() != lmove)
                  break;
            QPointF npos(chord->stemPos(chord->isUp(), true) + chord->pos() + chord->segment()->pos());
            double bd = (chord->beams() - 1) * beamDist * (chord->isUp() ? 1.0 : -1.0);
            double y1 = npos.y();
            double y2  = p1.y() + (npos.x() - x1) * slope;
            double stemLen = chord->isUp() ? (y1 - y2) : (y2 - y1);
            stemLen -= bd;
            if (stemLen < min)
                  min = stemLen;
            if (stemLen > max)
                  max = stemLen;
            }

      // adjst beam position
      double n = 3.0;   // minimum stem len (should be a style parameter)
      if (fabs(max-min) > _spatium * 2)
            n = 2.0;    // reduce minimum stem len (heuristic)

      {
      double diff = _spatium * n - min;
      if (upFlag)
            diff = -diff;
      p1.ry() += diff;
      p2.ry() += diff;
      }

      //---------------------------------------------------

      BeamSegment* bs = new BeamSegment;
      beamSegments.push_back(bs);
      bs->p1  = p1;
      bs->p2  = p2;

//      if (maxTickLen > division/2 + division/4) {
            if (maxTickLen <= division/16) {       // 1/64     24
                  bs = new BeamSegment(p1, p2);
                  bs->move(0, beamDist * 3);
                  beamSegments.push_back(bs);
                  }
            if (maxTickLen <= division/8) {        // 1/32   48
                  bs = new BeamSegment(p1, p2);
                  bs->move(0, beamDist * 2);
                  beamSegments.push_back(bs);
                  }
            if (maxTickLen <= division/4+division/8) {   //       144
                  bs = new BeamSegment(p1, p2);
                  bs->move(0, beamDist);
                  beamSegments.push_back(bs);
                  }
//            }

            //---------------------------------------------
            //   create broken/short beam segments
            //---------------------------------------------

      int l = maxTickLen;
      if (l > division/4)
            l = division/4;
      else if (l > division/8)
            l = division/8;
      else if (l > division/16)
            l = division/16;
      else
            l /= 2;

      for (;l >= division/16; l = l/2) {
            int n = 1;
            if (l == division/8)
                  n = 2;
            else if (l == division/16)
                  n = 3;

            double y1 = p1.y() + beamDist * n;

            Note* nn1 = 0;
            Note* nn2 = 0;
            bool nn1r = false;
            for (iChordRest i = elements.begin(); i != elements.end(); ++i) {
                  if (i->second->type() != CHORD)
                        continue;
                  Chord* chord = (Chord*)(i->second);
                  int tl = chord->tickLen();
                  if (tl > l) {
                        if (nn2) {
                              // create short segment
                              bs = new BeamSegment;
                              beamSegments.push_back(bs);
                              double x2 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                              double x3 = nn2->stemPos(nn2->chord()->isUp()).x() + nn2->chord()->pos().x() + nn2->chord()->segment()->pos().x();
                              bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                              bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                              }
                        else if (nn1) {
                              // create broken segment
                              bs = new BeamSegment;
                              beamSegments.push_back(bs);
                              double x2, x3;
                              if (!nn1r) {
                                    x3 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                                    x2 = x3 - point(style->beamMinLen);
                                    }
                              else {
                                    x2 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                                    x3 = x2 + point(style->beamMinLen);
                                    }

                              bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                              bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                              }
                        nn1r = false;
                        nn1 = nn2 = 0;
                        continue;
                        }
                  Note* n = chord->isUp() ? chord->noteList()->back()
                                      : chord->noteList()->front();
                  nn1r = false;
                  if (nn1)
                        nn2 = n;
                  else {
                        nn1 = n;
                        nn1r = i == elements.begin();
                        }
                  }
            if (nn2) {
                  // create short segment
                  bs = new BeamSegment;
                  beamSegments.push_back(bs);
                  double x2 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                  double x3 = nn2->stemPos(nn2->chord()->isUp()).x() + nn2->chord()->pos().x() + nn2->chord()->segment()->pos().x();
                  bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                  bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                  }
           else if (nn1) {
                  // create broken segment
                  bs = new BeamSegment;
                  beamSegments.push_back(bs);
                  double x3 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                  double x2 = x3 - point(style->beamMinLen);
                  bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                  bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                  }
            }

      //---------------------------------------------------
      //    create stem's
      //    stem Pos() is relative to Chord
      //---------------------------------------------------

      for (iChordRest i = elements.begin(); i != elements.end(); ++i) {
            if (i->second->type() != CHORD)
                  continue;
            Chord* chord = (Chord*)(i->second);

            QPointF npos(chord->stemPos(chord->isUp(), false) + chord->pos() + chord->segment()->pos());

            Stem* stem = chord->stem();
            if (!stem) {
                  stem = new Stem(score());
                  chord->setStem(stem);
                  }

            double x2 = npos.x();
            double xd = x2 - x1;
            double y1 = npos.y();
            double y2 = p1.y() + xd * slope;

            double stemLen = chord->isUp() ? (y1 - y2) : (y2 - y1);
            stem->setLen(spatium(stemLen));

            if (chord->isUp())
                  npos += QPointF(0, -stemLen);
            stem->setPos(npos - chord->pos() - chord->segment()->pos());
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


