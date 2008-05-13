//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chordrest.cpp,v 1.7 2006/03/28 14:58:58 wschweer Exp $
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
#include "layout.h"
#include "chordlist.h"
#include "score.h"
#include "sym.h"
#include "slur.h"
#include "beam.h"

//---------------------------------------------------------
//   NoteAttribute::atrList
//---------------------------------------------------------

AttributeInfo NoteAttribute::atrList[] = {
      { ufermataSym,       QString("ufermata"),        A_TOP_STAFF    },
      { dfermataSym,       QString("dfermata"),        A_BOTTOM_STAFF },
      { thumbSym,          QString("thumb"),           A_CHORD        },
      { sforzatoaccentSym, QString("sforzato"),        A_CHORD        },
      { esprSym,           QString("espressivo"),      A_CHORD        },
      { staccatoSym,       QString("staccato"),        A_CHORD        },
      { ustaccatissimoSym, QString("ustaccatissimo"),  A_CHORD },
      { dstaccatissimoSym, QString("dstaccatissimo"),  A_CHORD },
      { tenutoSym,         QString("tenuto"),          A_CHORD },
      { uportatoSym,       QString("uportato"),        A_CHORD },
      { dportatoSym,       QString("dportato"),        A_CHORD },
      { umarcatoSym,       QString("umarcato"),        A_CHORD },
      { dmarcatoSym,       QString("dmarcato"),        A_CHORD },
      { ouvertSym,         QString("ouvert"),          A_CHORD },
      { plusstopSym,       QString("plusstop"),        A_CHORD },
      { upbowSym,          QString("upbow"),           A_CHORD },
      { downbowSym,        QString("downbow"),         A_CHORD },
      { reverseturnSym,    QString("reverseturn"),     A_CHORD },
      { turnSym,           QString("turn"),            A_CHORD },
      { trillSym,          QString("trill"),           A_TOP_STAFF },
      { prallSym,          QString("prall"),           A_TOP_STAFF },
      { mordentSym,        QString("mordent"),         A_TOP_STAFF },
      { prallprallSym,     QString("prallprall"),      A_TOP_STAFF },
      { prallmordentSym,   QString("prallmordent"),    A_TOP_STAFF },
      { upprallSym,        QString("upprall"),         A_TOP_STAFF },
	{ downprallSym,      QString("downprall"),       A_TOP_STAFF },
	{ upmordentSym,      QString("upmordent"),       A_TOP_STAFF },
	{ downmordentSym,    QString("downmordent"),     A_TOP_STAFF },
	};

//---------------------------------------------------------
//   NoteAttribute
//---------------------------------------------------------

NoteAttribute::NoteAttribute(Score* s)
   : Symbol(s)
      {
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void NoteAttribute::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      setSym(atrList[subtype()].sym);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NoteAttribute::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "idx")                         // obsolete
                  setSubtype(e.text().toInt());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteAttribute::write(Xml& xml) const
      {
      xml.stag("Attribute");
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   hasAttribute
//---------------------------------------------------------

NoteAttribute* ChordRest::hasAttribute(const NoteAttribute* a)
      {
      int idx = a->subtype();
      for (iAttribute l = attributes.begin(); l != attributes.end(); ++l) {
            if (idx == (*l)->subtype())
                  return *l;
            }
      return 0;
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString NoteAttribute::subtypeName() const
      {
      return atrList[subtype()].name;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void NoteAttribute::setSubtype(const QString& s)
      {
      if (s[0].isDigit()) {         // for backward compatibility
            setSubtype(s.toInt());
            return;
            }
      for (int i = 0; i < NOTE_ATTRIBUTES; ++i) {
            if (atrList[i].name == s) {
                  setSubtype(i);
                  return;
                  }
            }
      printf("NoteAttribute <%s> unknown\n", qPrintable(s));
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::ChordRest(Score* s)
   : Element(s)
      {
      _beam     = 0;
      _tuplet   = 0;
      _small    = false;
      _beamMode = BEAM_AUTO;
      _dots     = 0;
      }

//---------------------------------------------------------
//   isUp
//---------------------------------------------------------

bool ChordRest::isUp() const
      {
      return _beam ? _beam->up() : _up;
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF ChordRest::canvasPos() const
      {
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   properties
//---------------------------------------------------------

QList<Prop> ChordRest::properties(Xml& xml) const
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
      if (_tuplet) {
            int idx = measure()->tuplets()->indexOf(_tuplet);
            if (idx == -1)
                  printf("ChordRest::writeProperties(): tuplet not found\n");
            else
                  pl.append(Prop("Tuplet", idx));
            }
      if (_small)
            pl.append(Prop("small", _small));
      Duration d;
      d.setVal(tickLen());
      if (_duration != d)
            pl.append(Prop("durationType", _duration.name()));
      return pl;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void ChordRest::writeProperties(Xml& xml) const
      {
      QList<Prop> pl = properties(xml);
      xml.prop(pl);
      for (ciAttribute ia = attributes.begin(); ia != attributes.end(); ++ia)
            (*ia)->write(xml);
      if (!xml.noSlurs) {
            foreach(Slur* s, _slurFor)
                  xml.tagE(QString("Slur type=\"start\" number=\"%1\"").arg(s->id()+1));
            foreach(Slur* s, _slurBack)
                  xml.tagE(QString("Slur type=\"stop\" number=\"%1\"").arg(s->id()+1));
            }
      xml.curTick = tick() + tickLen();
      }

//---------------------------------------------------------
//   isSimple
//---------------------------------------------------------

bool ChordRest::isSimple(Xml& xml) const
      {
      QList<Prop> pl = properties(xml);
      return (pl.size() <= 1) && attributes.empty() && _slurFor.empty() && _slurBack.empty();
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(QDomElement e)
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
      else if (tag == "Attribute") {
            NoteAttribute* atr = new NoteAttribute(score());
            atr->read(e);
            add(atr);
            }
      else if (tag == "Tuplet") {
            // while reading Measure, parent of Chord or Rest is set
            // to measure; after inserting Chord or Rest into Measure
            // parent is Segment
            Measure* m = (Measure*)parent();
            _tuplet = 0;
            foreach(Tuplet* t, *m->tuplets()) {
                  if (t->id() == i) {
                        _tuplet = t;
                        break;
                        }
                  }
            if (_tuplet == 0)
                  printf("Tuplet id %d not found\n", i);
            else
                  setTickLen(tickLen());  // set right symbol + dots
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
      else if (tag == "durationType") {
            Duration d;
            d.setVal(val);
            setDuration(d);
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
      _small = val;
      double m   = _small ? .7 : 1.0;
      if (staff()->small())
            m *= .7;
      setMag(m);
      }

//---------------------------------------------------------
//   layoutAttributes
//---------------------------------------------------------

void ChordRest::layoutAttributes(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();

      Measure* m = measure();
      System* s  = m->system();
      int idx    = staff()->rstaff();
      qreal x    = centerX();
      qreal sy   = _spatium;      // TODO: style parameter: distance to top/bottom line
      qreal sy2  = _spatium * .5; // TODO: style parameter: distance to top/bottom note

      qreal chordTopY = upPos()   - point(score()->style()->propertyDistanceStem) - sy2;
      qreal chordBotY = downPos() + point(score()->style()->propertyDistanceHead) + sy2;
      qreal staffTopY = s->bboxStaff(idx).y() - pos().y()      - sy;
      qreal staffBotY = staffTopY + s->bboxStaff(idx).height() + sy;

      qreal dyTop = 0;
      qreal dyBot = 0;

      //
      //    pass 1
      //    place all attributes with anchor at chord/rest
      //
      for (iAttribute ia = attributes.begin(); ia != attributes.end(); ++ia) {
            NoteAttribute* a = *ia;
            qreal y = 0;
            AttrAnchor aa = NoteAttribute::atrList[a->subtype()].anchor;
            if (aa == A_CHORD)
                  aa = isUp() ? A_BOTTOM_CHORD : A_TOP_CHORD;
            if (aa == A_TOP_CHORD) {
                  y = chordTopY - dyTop;
                  //
                  // check for collision with staff line
                  //
                  if (y >= staffTopY-.1) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4)
                              y -= _spatium * .5;
                        }
                  a->setPos(x, y);
                  dyTop += (point(score()->style()->propertyDistance) + a->bbox().height());
                  }
            else if (aa == A_BOTTOM_CHORD) {
                  y = chordBotY + dyBot;
                  //
                  // check for collision with staff line
                  //
                  if (y <= staffBotY+.1) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4)
                              y += _spatium * .5;
                        }
                  a->setPos(x, y);
                  dyBot += (point(score()->style()->propertyDistance) + a->bbox().height());
                  }
            }

      //
      //    pass 1
      //    now place all attributes with staff top or bottom anchor
      //
      if (chordTopY - dyTop > staffTopY)
            dyTop = 0;
      else
            dyTop = staffTopY - (chordTopY - dyTop);

      if (chordBotY + dyBot < staffBotY)
            dyBot = 0;
      else
            dyBot = (chordBotY + dyBot) - staffBotY;

      for (iAttribute ia = attributes.begin(); ia != attributes.end(); ++ia) {
            NoteAttribute* a = *ia;
            qreal y = 0;
            AttrAnchor aa = NoteAttribute::atrList[a->subtype()].anchor;
            if (aa == A_TOP_STAFF) {
                  y = staffTopY - dyTop;
                  a->setPos(x, y);
                  dyTop += (point(score()->style()->propertyDistance) + a->bbox().height());
                  }
            else if (aa == A_BOTTOM_STAFF) {
                  y = staffBotY + dyBot;
                  a->setPos(x, y);
                  dyBot += (point(score()->style()->propertyDistance) + a->bbox().height());
                  }
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
            printf("ChordRest::removeSlurFor(): not found\n");
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
            printf("ChordRest::removeSlurBack(): not found\n");
            return;
            }
      _slurBack.removeAt(idx);
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void ChordRest::setLen(int ticks)
      {
      setTickLen(ticks);
      Duration dt;
      int dts;
      headType(ticks, &dt, &dts);
      setDuration(dt);
      setDots(dts);
      }

