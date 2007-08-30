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
      { segnoSym,          QString("segno"),           A_TOP_STAFF },
      { codaSym,           QString("coda"),            A_TOP_STAFF },
      { varcodaSym,        QString("varcoda"),         A_TOP_STAFF },
	};

//---------------------------------------------------------
//   NoteAttribute
//---------------------------------------------------------

NoteAttribute::NoteAttribute(Score* s)
   : Symbol(s)
      {
      }

//---------------------------------------------------------
//   drag
//    return update Rect relative to canvas
//---------------------------------------------------------

QRectF NoteAttribute::drag(const QPointF& s)
      {
      QRectF r = bbox().translated(canvasPos());
      setUserOff(s / _spatium);
      r |= bbox().translated(canvasPos());
      return r;
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
      _beamMode = BEAM_AUTO;
      }

//---------------------------------------------------------
//   setBeamMode
//---------------------------------------------------------

void ChordRest::setBeamMode(BeamMode m)
      {
      if (!_tuplet)
            _beamMode = m;
      }

//---------------------------------------------------------
//   beams
//---------------------------------------------------------

int ChordRest::beams() const
      {
      int t = tickLen();
      if (t < division/8)
            return 4;
      else if (t < division/4)
            return 3;
      else if (t < division/2)
            return 2;
      else if (t < division)
            return 1;
      else
            printf("ChordRest::beams(): unsupported ticlLen %d\n", t);
      return 0;
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
      xml.curTick = tick() + tickLen();
      }

//---------------------------------------------------------
//   isSimple
//---------------------------------------------------------

bool ChordRest::isSimple(Xml& xml) const
      {
      QList<Prop> pl = properties(xml);
      return pl.size() <= 1 && attributes.empty();
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
            Measure* m    = (Measure*)parent();
            _tuplet       = m->tuplets()->at(i);
            if (_tuplet == 0)
                  printf("ChordRest::readProperties(): tuplet not found\n");
            _tuplet->add(this);
            }
      else
            return false;
      return true;
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

      bool up     = isUp();
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
                  aa = up ? A_BOTTOM_CHORD : A_TOP_CHORD;
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


