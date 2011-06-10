//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chordrest.cpp 3708 2010-11-16 09:54:31Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include <math.h>

#include "chordrest.h"
#include "chord.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "tuplet.h"
#include "score.h"
#include "sym.h"
#include "slur.h"
#include "beam.h"
#include "breath.h"
#include "barline.h"
#include "articulation.h"
#include "m-al/tempo.h"
#include "tempotext.h"
#include "note.h"
#include "arpeggio.h"
#include "dynamics.h"
#include "stafftext.h"
#include "m-al/sig.h"
#include "clef.h"
#include "lyrics.h"
#include "segment.h"
#include "m-al/xml.h"

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(Score* s)
   : Element(s)
      {
      _tuplet = 0;
      }

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(const DurationElement& e)
   : Element(e)
      {
      _tuplet   = e._tuplet;
      _duration = e._duration;
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int DurationElement::ticks() const
      {
      int ticks = duration().ticks();
      for (Tuplet* t = tuplet(); t; t = t->tuplet())
            ticks = ticks * t->ratio().denominator() / t->ratio().numerator();
      return ticks;
      }

//---------------------------------------------------------
//   hasArticulation
//---------------------------------------------------------

Articulation* ChordRest::hasArticulation(const Articulation* a)
      {
      int idx = a->subtype();
      for (iArticulation l = articulations.begin(); l != articulations.end(); ++l) {
            if (idx == (*l)->subtype())
                  return *l;
            }
      return 0;
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::ChordRest(Score* s)
   : DurationElement(s)
      {
      _beam      = 0;
      _small     = false;
      _beamMode  = BEAM_AUTO;
      _up        = true;
      _staffMove = 0;
      }

ChordRest::ChordRest(const ChordRest& cr)
   : DurationElement(cr)
      {
      _durationType = cr._durationType;
      _staffMove    = cr._staffMove;

      foreach(Articulation* a, cr.articulations) {    // make deep copy
            Articulation* na = new Articulation(*a);
            na->setParent(this);
            na->setTrack(track());
            articulations.append(na);
            }

      _beam               = 0;
      _beamMode           = cr._beamMode;
      _up                 = cr._up;
      _small              = cr._small;
      _extraLeadingSpace  = cr.extraLeadingSpace();
      _extraTrailingSpace = cr.extraTrailingSpace();
      _space              = cr._space;

      foreach(Lyrics* l, cr._lyricsList)        // make deep copy
            _lyricsList.append(new Lyrics(*l));
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::~ChordRest()
      {
      foreach(Articulation* a,  articulations)
            delete a;
      foreach(Lyrics* l, _lyricsList)
            delete l;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void ChordRest::scanElements(void* data, void (*func)(void*, Element*))
      {
      foreach(Slur* slur, _slurFor)
            slur->scanElements(data, func);
      for (ciArticulation i = articulations.begin(); i != articulations.end(); ++i) {
            Articulation* a = *i;
            func(data, a);
            }
      foreach(Lyrics* l, _lyricsList) {
            if (l)
                  l->scanElements(data, func);
            }
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF ChordRest::canvasPos() const
      {
      if (parent() == 0)
            return QPointF(x(), y());
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      if (system == 0)
            return QPointF();
      qreal yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(XmlReader* r, const QList<Tuplet*>& tuplets, const QList<Slur*>& slurs)
      {
      if (Element::readProperties(r))
            return true;

      MString8 tag = r->tag();
      QString val;
      qreal d;
      int i;
      Fraction fraction;

      if (r->readString("BeamMode", &val)) {
            int bm = BEAM_AUTO;
            if (val == "auto")
                  bm = BEAM_AUTO;
            else if (val == "begin")
                  bm = BEAM_BEGIN;
            else if (val == "mid")
                  bm = BEAM_MID;
            else if (val == "end")
                  bm = BEAM_END;
            else if (val == "no")
                  bm = BEAM_NO;
            else if (val == "begin32")
                  bm = BEAM_BEGIN32;
            else if (val == "begin64")
                  bm = BEAM_BEGIN64;
            else
                  bm = i;
            _beamMode = BeamMode(bm);
            }
      else if (tag == "Attribute" || tag == "Articulation") {     // obsolete: "Attribute"
            Articulation* atr = new Articulation(score());
            atr->read(r);
            add(atr);
            }
      else if (r->readInt("Tuplet", &i)) {
            setTuplet(0);
            foreach(Tuplet* t, tuplets) {
                  if (t->id() == i) {
                        setTuplet(t);
                        break;
                        }
                  }
            if (tuplet() == 0)
                  printf("Tuplet id %d not found\n", i);
            }
      else if (r->readReal("leadingSpace", &d))
            _extraLeadingSpace = Spatium(d);
      else if (r->readReal("trailingSpace", &d))
            _extraTrailingSpace = Spatium(d);
      else if (r->readInt("Beam", &i)) {
            Beam* b = score()->beam(i);
            if (b) {
                  setBeam(b);
                  b->add(this);
                  }
            else
                  printf("Beam id %d not found\n", i);
            }
      else if (r->readBool("small", &_small))
            ;
      else if (tag == "Slur") {
            int id = -1;
            QString type;

            while (r->readAttribute()) {
                  if (r->tag() == "number")
                        id = r->intValue();
                  else if (r->tag() == "type")
                        type = r->stringValue();
                  }
            Slur* slur = 0;
            foreach(Slur* s, slurs) {
                  if (s->id() == id) {
                        slur = s;
                        break;
                        }
                  }
            if (slur) {
                  if (type == "start") {
                        slur->setStartElement(this);
                        _slurFor.append(slur);
                        }
                  else if (type == "stop") {
                        slur->setEndElement(this);
                        _slurBack.append(slur);
                        }
                  else
                        printf("ChordRest::read(): unknown Slur type <%s>\n", qPrintable(type));
                  }
            else {
                  printf("ChordRest::read(): Slur id %d not found\n", id);
                  }
            }
      else if (r->readString("durationType", &val)) {
            setDurationType(val);
            if (durationType().type() != TimeDuration::V_MEASURE) {
                  setDuration(durationType().fraction());
                  }
            else {
                  if (score()->mscVersion() < 115) {
                        SigEvent e = score()->sigmap()->timesig(score()->curTick);
                        setDuration(e.timesig());
                        }
                  }
            }
      else if (r->readFraction("duration", &fraction))
            setDuration(fraction);
      else if (r->readInt("dots", &i))
            setDots(i);
      else if (r->readInt("move", &i))
            _staffMove = i;
      else if (tag == "Lyrics") {
            Lyrics* lyrics = new Lyrics(score());
            lyrics->setTrack(score()->curTrack);
            lyrics->read(r);
            add(lyrics);
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void ChordRest::setSmall(bool val)
      {
      _small   = val;
      qreal m = 1.0;
      if (_small)
            m = score()->styleD(ST_smallNoteMag);
      if (staff()->small())
            m *= score()->styleD(ST_smallStaffMag);
      setMag(m);
      }

//---------------------------------------------------------
//   layoutArticulations
//    called from chord()->layout()
//---------------------------------------------------------

void ChordRest::layoutArticulations()
      {
      if (parent() == 0 || articulations.isEmpty())
            return;
      qreal _spatium  = spatium();
//      Measure* m       = measure();
//      System* s        = m->system();
//      int idx          = staff()->rstaff() + staffMove();   // DEBUG

      qreal x          = centerX();

      qreal distance1 = point(score()->styleS(ST_propertyDistanceHead));
      qreal distance2 = point(score()->styleS(ST_propertyDistanceStem));

      qreal chordTopY = upPos();
      qreal chordBotY = downPos();

      qreal staffTopY = 0.0; // s->bboxStaff(idx).y() - pos().y();
      qreal staffBotY = staffTopY + 4.0 * _spatium + distance2;
      staffTopY      -= distance2;

      qreal dy = 0.0;

      //
      //    pass 1
      //    place all articulations with anchor at chord/rest
      //

      foreach (Articulation* a, articulations) {
            a->layout();
            ArticulationAnchor aa = a->anchor();
            if (aa != A_CHORD && aa != A_TOP_CHORD && aa != A_BOTTOM_CHORD)
                  continue;

            qreal sh = a->bbox().height() * mag();

            dy += distance1;
            if (sh > (_spatium * .5))   // hack
                  dy += sh * .5;
            qreal y;
            if (up()) {
                  y = chordBotY + dy;
                  //
                  // check for collision with staff line
                  //
                  if (y <= staffBotY-.1) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  += _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  }
            else {
                  y = chordTopY - dy;
                  //
                  // check for collision with staff line
                  //
                  if (y >= staffTopY+.1) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  -= _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  }
            a->setPos(x, y);
            }

      //
      //    pass 2
      //    now place all articulations with staff top or bottom anchor
      //
      qreal dyTop = staffTopY;
      qreal dyBot = staffBotY;

      if ((upPos() - _spatium) < dyTop)
            dyTop = upPos() - _spatium;
      if ((downPos() + _spatium) > dyBot)
            dyBot = downPos() + _spatium;

      for (iArticulation ia = articulations.begin(); ia != articulations.end(); ++ia) {
            Articulation* a = *ia;
            qreal y = 0;
            ArticulationAnchor aa = a->anchor();
            if (aa == A_TOP_STAFF) {
                  y = dyTop;
                  a->setPos(x, y);
                  dyTop -= point(score()->styleS(ST_propertyDistance)) + a->bbox().height();
                  }
            else if (aa == A_BOTTOM_STAFF) {
                  y = dyBot;
                  a->setPos(x, y);
                  dyBot += point(score()->styleS(ST_propertyDistance)) + a->bbox().height();
                  }
            }
      }

//---------------------------------------------------------
//   addSlurFor
//---------------------------------------------------------

void ChordRest::addSlurFor(Slur* s)
      {
      int idx = _slurFor.indexOf(s);
      if (idx >= 0) {
            printf("ChordRest::setSlurFor(): already there\n");
            return;
            }
      _slurFor.append(s);
      }

//---------------------------------------------------------
//   addSlurBack
//---------------------------------------------------------

void ChordRest::addSlurBack(Slur* s)
      {
      int idx = _slurBack.indexOf(s);
      if (idx >= 0) {
            printf("ChordRest::setSlurBack(): already there\n");
            return;
            }
      _slurBack.append(s);
      }

//---------------------------------------------------------
//   removeSlurFor
//---------------------------------------------------------

void ChordRest::removeSlurFor(Slur* s)
      {
      int idx = _slurFor.indexOf(s);
      if (idx < 0) {
            printf("ChordRest<%p>::removeSlurFor(): %p not found\n", this, s);
            foreach(Slur* s, _slurFor)
                  printf("  %p\n", s);
            return;
            }
      _slurFor.removeAt(idx);
      }

//---------------------------------------------------------
//   removeSlurBack
//---------------------------------------------------------

void ChordRest::removeSlurBack(Slur* s)
      {
      int idx = _slurBack.indexOf(s);
      if (idx < 0) {
            printf("ChordRest<%p>::removeSlurBack(): %p not found\n", this, s);
            foreach(Slur* s, _slurBack)
                  printf("  %p\n", s);
            return;
            }
      _slurBack.removeAt(idx);
      }

//---------------------------------------------------------
//   setBeam
//---------------------------------------------------------

void ChordRest::setBeam(Beam* b)
      {
      _beam = b;
      }

//---------------------------------------------------------
//   setDurationType
//---------------------------------------------------------

void ChordRest::setDurationType(TimeDuration::DurationType t)
      {
      _durationType.setType(t);
      }

void ChordRest::setDurationType(const QString& s)
      {
      _durationType.setType(s);
      }

void ChordRest::setDurationType(int ticks)
      {
      _durationType.setVal(ticks);
      }

void ChordRest::setDurationType(const TimeDuration& v)
      {
      _durationType = v;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void ChordRest::setTrack(int val)
      {
      foreach(Articulation* a, articulations)
            a->setTrack(val);
      Element::setTrack(val);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int ChordRest::tick() const
      {
      return segment() ? segment()->tick() : 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ChordRest::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());
      switch(e->type()) {
            case ARTICULATION:
                  articulations.push_back(static_cast<Articulation*>(e));
                  break;
            case LYRICS:
                  {
                  Lyrics* l = static_cast<Lyrics*>(e);
                  int size = _lyricsList.size();
                  if (l->no() >= size) {
                        for (int i = size-1; i < l->no(); ++i)
                              _lyricsList.append(0);
                        }
                  _lyricsList[l->no()] = l;
                  }
                  break;
            default:
                  printf("ChordRest::add: unknown element %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void ChordRest::remove(Element* e)
      {
      switch(e->type()) {
            case ARTICULATION:
                  if (!articulations.removeOne(static_cast<Articulation*>(e)))
                        printf("Chord::remove(): articulation not found\n");
                  break;
            case LYRICS:
                  {
                  for (int i = 0; i < _lyricsList.size(); ++i) {
                        if (_lyricsList[i] != e)
                              continue;
                        _lyricsList[i] = 0;
                        // shrink list if possible ?
                        return;
                        }
                  }
                  printf("Measure::remove: %s %p not found\n", e->name(), e);
                  break;
            default:
                  printf("ChordRest::remove: unknown element %s\n", e->name());
            }
      }

