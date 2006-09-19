//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chordrest.cpp,v 1.7 2006/03/28 14:58:58 wschweer Exp $
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

#include "chordrest.h"
#include "chord.h"
#include "xml.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "tuplet.h"

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
      { prallprallSym,     QString("prall prall"),     A_TOP_STAFF },
      { prallmordentSym,   QString("prall mordent"),   A_TOP_STAFF },
      { upprallSym,        QString("upprall"),         A_TOP_STAFF },
	{ downprallSym,      QString("downprall"),       A_TOP_STAFF },
	{ upmordentSym,      QString("upmordent"),       A_TOP_STAFF },
	{ downmordentSym,    QString("downmordent"),     A_TOP_STAFF },
	};

//---------------------------------------------------------
//   HelpLine
//---------------------------------------------------------

HelpLine::HelpLine(Score* s)
   : Line(s, false)
      {
      setLineWidth(style->helpLineWidth);
      setLen(Spatium(2));
      }

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
      QRectF r = bbox().translated(aref());
      setUserOff(s / _spatium);
      r |= bbox().translated(aref());
      parent()->bboxUpdate();
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

void NoteAttribute::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "idx")
                  setSubtype(e.text().toInt());
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteAttribute::write(Xml& xml) const
      {
      xml.stag("Attribute");
      xml.tag("idx", subtype());
      Element::writeProperties(xml);
      xml.etag("Attribute");
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
//   findSelectableElement
//    p is Measure relative
//---------------------------------------------------------

Element* ChordRest::findSelectableElement(QPointF p) const
      {
      for (ciAttribute ia = attributes.begin(); ia != attributes.end(); ++ia) {
            if ((*ia)->contains(p))
                  return *ia;
            }
      return 0;
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
//   writeProperties
//---------------------------------------------------------

void ChordRest::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      if (_beamMode != BEAM_AUTO)
            xml.tag("BeamMode", int(_beamMode));
      if (_tuplet) {
            int idx = measure()->tuplets()->indexOf(_tuplet);
            if (idx == -1)
                  printf("ChordRest::writeProperties(): tuplet not found\n");
            else
                  xml.tag("Tuplet", idx);
            }
      for (ciAttribute ia = attributes.begin(); ia != attributes.end(); ++ia)
            (*ia)->write(xml);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(QDomNode node)
      {
      if (Element::readProperties(node))
            return true;
      QDomElement e = node.toElement();
      if (e.isNull())
            return true;
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "BeamMode")
            _beamMode = BeamMode(i);
      else if (tag == "Attribute") {
            NoteAttribute* atr = new NoteAttribute(score());
            atr->read(node);
            add(atr);
            }
      else if (tag == "Tuplet") {
            // while reading Measure, parent of Chord or Rest is set
            // to measure; after inserting Chord or Rest into Measure
            // parent is Segment
            Measure* m    = (Measure*)parent();
            TupletList* t = m->tuplets();
            _tuplet       = t->at(i);
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

void ChordRest::layoutAttributes()
      {
      Measure* m = measure();
      System* s  = m->system();
      int idx    = staff()->rstaff();
      qreal x    = centerX();
      qreal sy   = _spatium;    // TODO: style parameter: fermata distance to top line
                              // = _spatium
      qreal chordTopY = upPos()   - point(style->propertyDistanceStem) - sy;
      qreal chordBotY = downPos() + point(style->propertyDistanceHead) + sy;
      qreal staffTopY = s->bboxStaff(idx).y() - pos().y() - sy;
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
                              y += _spatium * .5;
                        }
                  a->setPos(x, y);
                  dyTop += (point(style->propertyDistance) + a->bbox().height());
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
                              y -= _spatium * .5;
                        }
                  a->setPos(x, y);
                  dyBot += (point(style->propertyDistance) + a->bbox().height());
                  }
            }

      //
      //    pass 1
      //    now place all attributes with anchor at staff top or bottom
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
                  dyTop += (point(style->propertyDistance) + a->bbox().height());
                  }
            else if (aa == A_BOTTOM_STAFF) {
                  y = staffBotY + dyBot;
                  a->setPos(x, y);
                  dyBot += (point(style->propertyDistance) + a->bbox().height());
                  }
            }
      }

