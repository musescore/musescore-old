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

#include "articulation.h"
#include "score.h"
#include "chordrest.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "stafftype.h"
#include "painter.h"
#include "undo.h"

//---------------------------------------------------------
//   Articulation::articulationList
//---------------------------------------------------------

ArticulationInfo Articulation::articulationList[ARTICULATIONS] = {
      { ufermataSym, dfermataSym,
            "fermata", QT_TRANSLATE_NOOP("articulation", "fermata"),
            100, 100, 2.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { ushortfermataSym, dshortfermataSym,
            "shortfermata", QT_TRANSLATE_NOOP("articulation", "shortfermata"),
            100, 100, 1.5, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { ulongfermataSym, dlongfermataSym,
            "longfermata", QT_TRANSLATE_NOOP("articulation", "longfermata"),
            100, 100, 3.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { uverylongfermataSym, dverylongfermataSym,
            "verylongfermata", QT_TRANSLATE_NOOP("articulation", "verylongfermata"),
            100, 100, 4.0, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { thumbSym, thumbSym,
            "thumb", QT_TRANSLATE_NOOP("articulation", "thumb"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { sforzatoaccentSym,   sforzatoaccentSym,
            "sforzato", QT_TRANSLATE_NOOP("articulation", "sforzato"),
            120, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { esprSym, esprSym             ,
            "espressivo", QT_TRANSLATE_NOOP("articulation", "espressivo"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { staccatoSym, staccatoSym,
            "staccato", QT_TRANSLATE_NOOP("articulation", "staccato"),
            100,  50, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF
            },
      { ustaccatissimoSym,   dstaccatissimoSym,
            "staccatissimo", QT_TRANSLATE_NOOP("articulation", "staccatissimo"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF
            },
      { tenutoSym, tenutoSym,
            "tenuto", QT_TRANSLATE_NOOP("articulation", "tenuto"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { dportatoSym, uportatoSym,
            "portato", QT_TRANSLATE_NOOP("articulation", "portato"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { umarcatoSym, dmarcatoSym,
            "marcato", QT_TRANSLATE_NOOP("articulation", "marcato"),
            110, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { ouvertSym, ouvertSym,
            "ouvert", QT_TRANSLATE_NOOP("articulation", "ouvert"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { plusstopSym, plusstopSym,
            "plusstop", QT_TRANSLATE_NOOP("articulation", "plusstop"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { upbowSym, upbowSym,
            "upbow", QT_TRANSLATE_NOOP("articulation", "upbow"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { downbowSym, downbowSym,
            "downbow", QT_TRANSLATE_NOOP("articulation", "downbow"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { reverseturnSym, reverseturnSym,
            "reverseturn", QT_TRANSLATE_NOOP("articulation", "reverseturn"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { turnSym, turnSym,
            "turn", QT_TRANSLATE_NOOP("articulation", "turn"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { trillSym, trillSym,
            "trill", QT_TRANSLATE_NOOP("articulation", "trill"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { prallSym, prallSym,
            "prall", QT_TRANSLATE_NOOP("articulation", "prall"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { mordentSym, mordentSym,
            "mordent", QT_TRANSLATE_NOOP("articulation", "mordent"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { prallprallSym, prallprallSym,
            "prallprall", QT_TRANSLATE_NOOP("articulation", "prallprall"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { prallmordentSym, prallmordentSym,
            "prallmordent", QT_TRANSLATE_NOOP("articulation", "prallmordent"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { upprallSym, upprallSym,
            "upprall", QT_TRANSLATE_NOOP("articulation", "upprall"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ downprallSym, downprallSym,
            "downprall", QT_TRANSLATE_NOOP("articulation", "downprall"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ upmordentSym, upmordentSym,
            "upmordent", QT_TRANSLATE_NOOP("articulation", "upmordent"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ downmordentSym, downmordentSym,
            "downmordent", QT_TRANSLATE_NOOP("articulation", "downmordent"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ pralldownSym, pralldownSym,
            "pralldown", QT_TRANSLATE_NOOP("articulation", "pralldown"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ prallupSym, prallupSym,
            "prallup", QT_TRANSLATE_NOOP("articulation", "prallup"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ lineprallSym, lineprallSym,
            "lineprall", QT_TRANSLATE_NOOP("articulation", "lineprall"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
	{ schleiferSym, schleiferSym,
            "schleifer", QT_TRANSLATE_NOOP("articulation", "schleifer"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { snappizzicatoSym, snappizzicatoSym,
            "snappizzicato", QT_TRANSLATE_NOOP("articulation", "snappizzicato"),
            100, 100, 0., ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE
            },
      { letterTSym, letterTSym,
            "tapping", QT_TRANSLATE_NOOP("articulation", "tapping"),
            100, 100, 0., ARTICULATION_SHOW_IN_TABLATURE
            },
      { letterSSym, letterSSym,
            "slapping", QT_TRANSLATE_NOOP("articulation", "slapping"),
            100, 100, 0., ARTICULATION_SHOW_IN_TABLATURE
            },
      { letterPSym, letterPSym,
            "popping", QT_TRANSLATE_NOOP("articulation", "popping"),
            100, 100, 0., ARTICULATION_SHOW_IN_TABLATURE
            },
	};

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(Score* s)
   : Element(s)
      {
      _direction = AUTO;
      _up = true;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Articulation::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      _anchor = score()->style()->articulationAnchor(idx);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Articulation::read(QDomElement e)
      {
      setSubtype(0);    // default
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "idx")                         // obsolete
                  setSubtype(val.toInt());
            else if (tag == "channel")
                  _channelName = e.attribute("name");
            else if (tag == "anchor")
                  _anchor = ArticulationAnchor(val.toInt());
            else if (tag == "direction") {
                  Direction dir = AUTO;
                  if (val == "up")
                        dir = UP;
                  else if (val == "down")
                        dir = DOWN;
                  else if (val == "auto")
                        dir = AUTO;
                  else
                        domError(e);
//                  printf("setDirection %s %d\n", qPrintable(val), int(dir));
                  setDirection(dir);
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Articulation::write(Xml& xml) const
      {
      xml.stag("Articulation");
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      switch(_direction) {
            case UP:
                  xml.tag("direction", QVariant("up"));
                  break;
            case DOWN:
                  xml.tag("direction", QVariant("down"));
                  break;
            case AUTO:
                  break;
            }
      Element::writeProperties(xml);
      int t = subtype();
      if (_anchor != score()->style()->articulationAnchor(t))
            xml.tag("anchor", int(_anchor));
      xml.etag();
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Articulation::subtypeName() const
      {
      return articulationList[subtype()].name;
      }

//---------------------------------------------------------
//   timeStretch
//---------------------------------------------------------

qreal Articulation::timeStretch() const
      {
      return articulationList[subtype()].timeStretch;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Articulation::setSubtype(const QString& s)
      {
      if (s[0].isDigit()) {         // for backward compatibility
            setSubtype(s.toInt());
            return;
            }
      int st;
      for (st = 0; st < ARTICULATIONS; ++st) {
            if (articulationList[st].name == s)
                  break;
            }
      if (st == ARTICULATIONS) {
            struct {
                  const char* name;
                  bool up;
                  ArticulationType type;
                  } al[] = {
                  { "umarcato",         true,  Articulation_Marcato },
                  { "dmarcato",         false, Articulation_Marcato },
                  { "ufermata",         true,  Articulation_Fermata },
                  { "dfermata",         false, Articulation_Fermata },
                  { "ushortfermata",    true,  Articulation_Shortfermata },
                  { "dshortfermata",    false, Articulation_Shortfermata },
                  { "ulongfermata",     true,  Articulation_Longfermata },
                  { "dlongfermata",     false, Articulation_Longfermata },
                  { "uverylongfermata", true,  Articulation_Verylongfermata },
                  { "dverylongfermata", false, Articulation_Verylongfermata },
                  { "uportato",         true,  Articulation_Portato },
                  { "dportato",         false, Articulation_Portato },
                  { "ustaccatissimo",   true,  Articulation_Staccatissimo },
                  { "dstaccatissimo",   false, Articulation_Staccatissimo }
                  };

            int i;
            int n = sizeof(al) / sizeof(*al);
            for (i = 0; i < n; ++i) {
                  if (s == al[i].name) {
                        _up = al[i].up;
                        st  = int(al[i].type);
                        break;
                        }
                  }
            if (i == n) {
                  st = 0;
                  printf("Articulation: unknown <%s>\n", qPrintable(s));
                  }
            }
      setSubtype(st);
      }

//---------------------------------------------------------
//   idx2name
//---------------------------------------------------------

QString Articulation::idx2name(int idx)
      {
      return articulationList[idx].name;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Articulation::pagePos() const
      {
      if (parent() == 0 || parent()->parent() == 0)
            return pos();
      if (parent()->isChordRest()) {
            ChordRest* cr = static_cast<ChordRest*>(parent());
            Measure* m = cr->measure();
            if (m == 0)
                  return pos();
            System* system = m->system();
            if (system == 0)
                  return pos();
            qreal yp = y() + system->staff(staffIdx() + cr->staffMove())->y() + system->y();
            return QPointF(pageX(), yp);
            }
      return Element::pagePos();
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Articulation::draw(Painter* painter) const
      {
      SymId sym = _up ? articulationList[subtype()].upSym : articulationList[subtype()].downSym;
      int flags = articulationList[subtype()].flags;
      if (staff()) {
            bool tab = staff()->useTablature();
            if (tab) {
                  if (!(flags & ARTICULATION_SHOW_IN_TABLATURE))
                        return;
                  }
            else {
                  if (!(flags & ARTICULATION_SHOW_IN_PITCHED_STAFF))
                        return;
                  }
            }
      symbols[score()->symIdx()][sym].draw(painter, magS());
      }

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* Articulation::chordRest() const
      {
      if (parent() && parent()->isChordRest())
            return static_cast<ChordRest*>(parent());
      return 0;
      }

//---------------------------------------------------------
//   articulationType
//---------------------------------------------------------

ArticulationType Articulation::articulationType() const
      {
      return ArticulationType(subtype());
      }

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

QString Articulation::subtypeUserName() const
      {
      return articulationList[subtype()].description;
      }

//---------------------------------------------------------
//   relGateTime
//---------------------------------------------------------

int Articulation::relGateTime() const
      {
      return articulationList[subtype()].relGateTime;
      }

//---------------------------------------------------------
//   relVelocity
//---------------------------------------------------------

int Articulation::relVelocity() const
      {
      return articulationList[subtype()].relVelocity;
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Articulation::layout()
      {
      SymId sym = _up ? articulationList[subtype()].upSym : articulationList[subtype()].downSym;
      setbbox(symbols[score()->symIdx()][sym].bbox(magS()));
      }

//---------------------------------------------------------
//   setDirection
//---------------------------------------------------------

void Articulation::setDirection(Direction d)
      {
      _direction = d;
      if (d != AUTO)
            _up = (d == UP);
//      printf("setDirection %p %d %d\n", this, _up, int(d));
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Articulation::toDefault()
      {
      if (_direction != AUTO) {
            score()->undo()->push(new ChangeArticulation(this, AUTO,
              score()->style()->articulationAnchor(subtype())));
            }
      Element::toDefault();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Articulation::dragAnchor() const
      {
      return QLineF(canvasPos(), parent()->canvasPos());
      }


