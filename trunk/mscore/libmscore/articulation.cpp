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
#include "sym.h"
#include "score.h"
#include "chordrest.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "stafftype.h"
#include "painter.h"

//---------------------------------------------------------
//   Articulation::articulationList
//---------------------------------------------------------

ArticulationInfo Articulation::articulationList[ARTICULATIONS] = {
      { ufermataSym,         QT_TRANSLATE_NOOP("articulation", "ufermata"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE },
      { dfermataSym,         QT_TRANSLATE_NOOP("articulation", "dfermata"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { ushortfermataSym,    QT_TRANSLATE_NOOP("articulation", "ushortfermata"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { dshortfermataSym,    QT_TRANSLATE_NOOP("articulation", "dshortfermata"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { ulongfermataSym,     QT_TRANSLATE_NOOP("articulation", "ulongfermata"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { dlongfermataSym,     QT_TRANSLATE_NOOP("articulation", "dlongfermata"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { uverylongfermataSym, QT_TRANSLATE_NOOP("articulation", "uverylongfermata"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { dverylongfermataSym, QT_TRANSLATE_NOOP("articulation", "dverylongfermata"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { thumbSym,            QT_TRANSLATE_NOOP("articulation", "thumb"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { sforzatoaccentSym,   QT_TRANSLATE_NOOP("articulation", "sforzato"),
            120, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { esprSym,             QT_TRANSLATE_NOOP("articulation", "espressivo"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { staccatoSym,         QT_TRANSLATE_NOOP("articulation", "staccato"),
            100,  50, ARTICULATION_SHOW_IN_PITCHED_STAFF  },
      { ustaccatissimoSym,   QT_TRANSLATE_NOOP("articulation", "ustaccatissimo"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF  },
      { dstaccatissimoSym,   QT_TRANSLATE_NOOP("articulation", "dstaccatissimo"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF  },
      { tenutoSym,           QT_TRANSLATE_NOOP("articulation", "tenuto"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { uportatoSym,         QT_TRANSLATE_NOOP("articulation", "uportato"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { dportatoSym,         QT_TRANSLATE_NOOP("articulation", "dportato"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { umarcatoSym,         QT_TRANSLATE_NOOP("articulation", "umarcato"),
            110, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { dmarcatoSym,         QT_TRANSLATE_NOOP("articulation", "dmarcato"),
            110, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { ouvertSym,           QT_TRANSLATE_NOOP("articulation", "ouvert"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { plusstopSym,         QT_TRANSLATE_NOOP("articulation", "plusstop"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { upbowSym,            QT_TRANSLATE_NOOP("articulation", "upbow"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { downbowSym,          QT_TRANSLATE_NOOP("articulation", "downbow"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { reverseturnSym,      QT_TRANSLATE_NOOP("articulation", "reverseturn"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { turnSym,             QT_TRANSLATE_NOOP("articulation", "turn"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { trillSym,            QT_TRANSLATE_NOOP("articulation", "trill"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { prallSym,            QT_TRANSLATE_NOOP("articulation", "prall"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { mordentSym,          QT_TRANSLATE_NOOP("articulation", "mordent"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { prallprallSym,       QT_TRANSLATE_NOOP("articulation", "prallprall"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { prallmordentSym,     QT_TRANSLATE_NOOP("articulation", "prallmordent"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { upprallSym,          QT_TRANSLATE_NOOP("articulation", "upprall"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
	{ downprallSym,        QT_TRANSLATE_NOOP("articulation", "downprall"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
	{ upmordentSym,        QT_TRANSLATE_NOOP("articulation", "upmordent"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
	{ downmordentSym,      QT_TRANSLATE_NOOP("articulation", "downmordent"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
	{ pralldownSym,      QT_TRANSLATE_NOOP("articulation", "pralldown"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
	{ prallupSym,      QT_TRANSLATE_NOOP("articulation", "prallup"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
	{ lineprallSym,      QT_TRANSLATE_NOOP("articulation", "lineprall"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
	{ schleiferSym, QT_TRANSLATE_NOOP("articulation", "schleifer"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { snappizzicatoSym,    QT_TRANSLATE_NOOP("articulation", "snappizzicato"),
            100, 100, ARTICULATION_SHOW_IN_PITCHED_STAFF | ARTICULATION_SHOW_IN_TABLATURE  },
      { letterTSym,    QT_TRANSLATE_NOOP("articulation", "tapping"),
            100, 100, ARTICULATION_SHOW_IN_TABLATURE  },
      { letterSSym,    QT_TRANSLATE_NOOP("articulation", "slapping"),
            100, 100, ARTICULATION_SHOW_IN_TABLATURE  },
      { letterPSym,    QT_TRANSLATE_NOOP("articulation", "popping"),
            100, 100, ARTICULATION_SHOW_IN_TABLATURE  },
	};

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(Score* s)
   : Symbol(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Articulation::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      setSym(articulationList[subtype()].sym);
      _anchor = ArticulationAnchor(score()->styleI(StyleIdx(ST_UfermataAnchor + subtype())));
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Articulation::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "idx")                         // obsolete
                  setSubtype(e.text().toInt());
            else if (tag == "channel")
                  _channelName = e.attribute("name");
            else if (tag == "anchor")
                  _anchor = ArticulationAnchor(e.text().toInt());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Articulation::write(Xml& xml) const
      {
      xml.stag("Articulation");
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      Element::writeProperties(xml);
      int t = subtype();
      if (score()->styleI(StyleIdx(ST_UfermataAnchor + t)) != int(_anchor))
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
//   setSubtype
//---------------------------------------------------------

void Articulation::setSubtype(const QString& s)
      {
      if (s[0].isDigit()) {         // for backward compatibility
            setSubtype(s.toInt());
            return;
            }
      int t = name2idx(s);
      setSubtype(t);
      _anchor = ArticulationAnchor(score()->styleI(StyleIdx(ST_UfermataAnchor + t)));
      }

//---------------------------------------------------------
//   idx2name
//---------------------------------------------------------

QString Articulation::idx2name(int idx)
      {
      return articulationList[idx].name;
      }

//---------------------------------------------------------
//   name2idx
//---------------------------------------------------------

int Articulation::name2idx(const QString& s)
      {
      for (int i = 0; i < ARTICULATIONS; ++i) {
            if (articulationList[i].name == s) {
                  return i;
                  }
            }
      return -1;
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
      symbols[score()->symIdx()][_sym].draw(painter, magS());
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

