//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "chordrest.h"
#include "chord.h"
#include "xml.h"
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
#include "al/tempo.h"
#include "tempotext.h"
#include "note.h"
#include "arpeggio.h"

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(Score* s)
   : Element(s)
      {
      _tuplet = 0;
      _ticks  = -1;
      }

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(const DurationElement& e)
   : Element(e)
      {
      _tuplet   = e._tuplet;
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int DurationElement::ticks() const
      {
      int ticks = fraction().ticks();
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
      _duration           = cr._duration;
      _beam               = 0;
      _up                 = cr._up;
      _staffMove          = cr._staffMove;
      _small              = cr._small;
      _beamMode           = cr._beamMode;
      _extraLeadingSpace  = cr._extraLeadingSpace;
      _extraTrailingSpace = cr._extraTrailingSpace;

      foreach(Articulation* a, cr.articulations) {            // make deep copy
            Articulation* na = new Articulation(*a);
            na->setParent(this);
            na->setTrack(track());
            articulations.append(na);
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void ChordRest::scanElements(void* data, void (*func)(void*, Element*))
      {
      foreach(Slur* slur, _slurFor)
            slur->scanElements(data, func);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF ChordRest::canvasPos() const
      {
      if (parent() == 0)
            return QPointF(x(), y());
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      if (system == 0)
            return QPointF();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   properties
//---------------------------------------------------------

QList<Prop> ChordRest::properties(Xml& xml, bool /*clipboardmode*/) const
      {
      QList<Prop> pl = Element::properties(xml);
      //
      // BeamMode default:
      //    REST  - BEAM_NO
      //    CHORD - BEAM_AUTO
      //
      if ((type() == REST && _beamMode != BEAM_NO)
         || (type() == CHORD && _beamMode != BEAM_AUTO)) {
            QString s;
            switch(_beamMode) {
                  case BEAM_AUTO:    s = "auto"; break;
                  case BEAM_BEGIN:   s = "begin"; break;
                  case BEAM_MID:     s = "mid"; break;
                  case BEAM_END:     s = "end"; break;
                  case BEAM_NO:      s = "no"; break;
                  case BEAM_BEGIN32: s = "begin32"; break;
                  case BEAM_INVALID: s = "?"; break;
                  }
            pl.append(Prop("BeamMode", s));
            }
      if (tuplet())
            pl.append(Prop("Tuplet", tuplet()->id()));
      if (_small)
            pl.append(Prop("small", _small));
      if (_extraLeadingSpace.val() != 0.0)
            pl.append(Prop("leadingSpace", _extraLeadingSpace.val()));
      if (_extraTrailingSpace.val() != 0.0)
            pl.append(Prop("trailingSpace", _extraTrailingSpace.val()));
      if (duration().dots())
            pl.append(Prop("dots", duration().dots()));
      if (_staffMove)
            pl.append(Prop("move", _staffMove));
      pl.append(Prop("durationType", duration().name()));
      return pl;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void ChordRest::writeProperties(Xml& xml) const
      {
      QList<Prop> pl = properties(xml);
      xml.prop(pl);
      for (ciArticulation ia = articulations.begin(); ia != articulations.end(); ++ia)
            (*ia)->write(xml);
      foreach(Slur* s, _slurFor)
            xml.tagE(QString("Slur type=\"start\" number=\"%1\"").arg(s->id()+1));
      foreach(Slur* s, _slurBack)
            xml.tagE(QString("Slur type=\"stop\" number=\"%1\"").arg(s->id()+1));
      if (!xml.clipboardmode && _beam)
            xml.tag("Beam", _beam->id());
      xml.curTick = tick() + ticks();
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(QDomElement e, const QList<Tuplet*>& tuplets)
      {
      if (Element::readProperties(e))
            return true;
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "BeamMode") {
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
            else
                  bm = i;
            _beamMode = BeamMode(bm);
            }
      else if (tag == "Attribute" || tag == "Articulation") {     // obsolete: "Attribute"
            Articulation* atr = new Articulation(score());
            atr->read(e);
            add(atr);
            }
      else if (tag == "Tuplet") {
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
      else if (tag == "leadingSpace")
            _extraLeadingSpace = Spatium(val.toDouble());
      else if (tag == "trailingSpace")
            _extraTrailingSpace = Spatium(val.toDouble());
      else if (tag == "Beam") {
            Beam* b = score()->beam(i);
            if (b) {
                  setBeam(b);
                  b->add(this);
                  }
            else
                  printf("Beam id %d not found\n", i);
            }
      else if (tag == "small")
            _small = i;
      else if (tag == "Slur") {
            int id = e.attribute("number").toInt() - 1;
            QString type = e.attribute("type");
            Slur* slur = 0;
            foreach(Element* e, *score()->gel()) {
                  if (e->type() == SLUR && ((Slur*)e)->id() == id) {
                        slur = (Slur*)e;
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
                        printf("Note::read(): unknown Slur type <%s>\n", qPrintable(type));
                  }
            else {
                  printf("Note::read(): Slur not found\n");
                  }
            }
      else if (tag == "durationType")
            setDurationType(val);
      else if (tag == "ticklen")      // obsolete (version < 1.12)
            _ticks = i;
      else if (tag == "dots")
            setDots(i);
      else if (tag == "move")
            _staffMove = i;
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
      double m = 1.0;
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
      if (parent() == 0)
            return;
      double _spatium  = spatium();
      Measure* m       = measure();
      System* s        = m->system();
      int idx          = staff()->rstaff() + staffMove();   // DEBUG

#if 0 // moved to chord()->layout()
      if (type() == CHORD && static_cast<Chord*>(this)->arpeggio()) {
            Chord* c          = static_cast<Chord*>(this);
            double distance   = score()->styleS(ST_ArpeggioNoteDistance).val() * _spatium;
            double headHeight = c->upNote()->headHeight();
            c->arpeggio()->layout();
            double x  = -(c->arpeggio()->width() + distance);
            double y  = c->upNote()->pos().y() - headHeight * .5;
            double h  = c->downNote()->pos().y() - y;
            y        += s->staff(staffIdx() + staffMove())->y() - s->staff(staffIdx())->y();
            c->arpeggio()->setHeight(h);
            c->arpeggio()->setPos(x, y);
            }
#endif
      qreal x          = centerX();

      double distance1 = point(score()->styleS(ST_propertyDistanceHead));
      double distance2 = point(score()->styleS(ST_propertyDistanceStem));

      qreal chordTopY = upPos();
      qreal chordBotY = downPos();

      qreal staffTopY = s->bboxStaff(idx).y() - pos().y();
      qreal staffBotY = staffTopY + 4.0 * _spatium + distance2;
      staffTopY      -= distance2;

      qreal dy = 0.0;

      //
      //    pass 1
      //    place all articulations with anchor at chord/rest
      //

      foreach (Articulation* a, articulations) {
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
      qreal dyTop = 0.0;
      qreal dyBot = 0.0;

      for (iArticulation ia = articulations.begin(); ia != articulations.end(); ++ia) {
            Articulation* a = *ia;
            qreal y = 0;
            ArticulationAnchor aa = a->anchor();
            if (aa == A_TOP_STAFF) {
                  y = staffTopY - dyTop;
                  a->setPos(x, y);
                  dyTop += point(score()->styleS(ST_propertyDistance)) + a->bbox().height();
                  }
            else if (aa == A_BOTTOM_STAFF) {
                  y = staffBotY + dyBot;
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
//   drop
//---------------------------------------------------------

Element* ChordRest::drop(ScoreView* view, const QPointF& p1, const QPointF& p2, Element* e)
      {
      Measure* m  = measure();
      switch (e->type()) {
            case BREATH:
                  {
                  Breath* b = static_cast<Breath*>(e);
                  b->setTrack(staffIdx() * VOICES);

                  // TODO: insert automatically in all staves?

                  Segment* seg = m->findSegment(SegBreath, tick());
                  if (seg == 0) {
                        seg = m->createSegment(SegBreath, tick());
                        score()->undoAddElement(seg);
                        }
                  b->setParent(seg);
                  score()->undoAddElement(b);
                  }
                  break;

            case BAR_LINE:
                  {
                  BarLine* bl = static_cast<BarLine*>(e);
                  bl->setTrack(staffIdx() * VOICES);

                  if ((bl->tick() == m->tick())
                     || (bl->tick() == m->tick() + m->tickLen())) {
                        return m->drop(view, p1, p2, e);
                        }

                  Segment* seg = m->findSegment(SegBarLine, tick());
                  if (seg == 0) {
                        seg = m->createSegment(SegBarLine, tick());
                        score()->undoAddElement(seg);
                        }
                  bl->setParent(seg);
                  score()->undoAddElement(bl);
                  }
                  break;

            case CLEF:
                  staff()->changeClef(tick(), e->subtype());
                  break;

            case TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(e);
                  score()->tempomap()->addTempo(tick(), tt->tempo());
                  tt->setTick(tick());
                  tt->setTrack(0);
                  tt->setParent(m);
                  score()->undoAddElement(tt);
                  }
                  break;

            case DYNAMIC:
                  {
                  e->setTick(tick());
                  e->setTrack(staffIdx() * VOICES);
                  e->setParent(m);
                  score()->undoAddElement(e);
                  }
                  break;

            case NOTE:
                  {
                  Note* note = static_cast<Note*>(e);
                  score()->setNoteRest(this, track(), note->pitch(),
                     Fraction(1, 4), note->headGroup(), AUTO);
                  delete e;
                  }
                  break;

            case STAFF_TEXT:
                  e->setTick(tick());
                  e->setTrack(staffIdx() * VOICES);
                  e->setParent(m);
                  score()->undoAddElement(e);
                  break;
            default:
                  printf("cannot drop %s\n", e->name());
                  delete e;
                  return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   setBeam
//---------------------------------------------------------

void ChordRest::setBeam(Beam* b)
      {
      _beam = b;
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void ChordRest::toDefault()
      {
      score()->undoChangeChordRestSpace(this, Spatium(0.0), Spatium(0.0));
      score()->undoChangeUserOffset(this, QPointF());
      }

//---------------------------------------------------------
//   fraction
//---------------------------------------------------------

Fraction ChordRest::fraction() const
      {
      if (_duration.type() == Duration::V_MEASURE || _duration.type() == Duration::V_INVALID) {
            if (parent() == 0)
                  return AL::division * 4;
            return measure()->fraction();
            }
      return _duration.fraction();
      }

//---------------------------------------------------------
//   setFraction
//---------------------------------------------------------

void ChordRest::setFraction(const Fraction& f)
      {
      _duration = Duration(f);
      }

//---------------------------------------------------------
//   convertTicks
//---------------------------------------------------------

void DurationElement::convertTicks()
      {
      if (_ticks < 0)
            return;
      if (_tuplet == 0)
            setFraction(Fraction::fromTicks(_ticks));
      }

//---------------------------------------------------------
//   setDurationType
//---------------------------------------------------------

void ChordRest::setDurationType(Duration::DurationType t)
      {
      _duration.setType(t);
      _ticks = -1;
      }

void ChordRest::setDurationType(const QString& s)
      {
      _duration.setType(s);
      _ticks = -1;
      }

//---------------------------------------------------------
//   setDurationVal
//---------------------------------------------------------

void ChordRest::setDurationVal(int ticks)
      {
      _duration.setVal(ticks);
      _ticks = -1;
      }

//---------------------------------------------------------
//   setDuration
//---------------------------------------------------------

void ChordRest::setDuration(const Duration& v)
      {
      _duration = v;
      _ticks = -1;
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

