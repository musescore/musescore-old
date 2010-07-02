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

#include "articulation.h"
#include "sym.h"
#include "score.h"
#include "chordrest.h"
#include "system.h"
#include "measure.h"

//---------------------------------------------------------
//   Articulation::articulationList
//---------------------------------------------------------

ArticulationInfo Articulation::articulationList[ARTICULATIONS] = {
      { ufermataSym,       QT_TRANSLATE_NOOP("articulation", "ufermata"),        100, 100  },
      { dfermataSym,       QT_TRANSLATE_NOOP("articulation", "dfermata"),        100, 100  },
      { ushortfermataSym,    QT_TRANSLATE_NOOP("articulation", "ushortfermata"),    100, 100  },
      { dshortfermataSym,    QT_TRANSLATE_NOOP("articulation", "dshortfermata"),    100, 100  },
      { ulongfermataSym,     QT_TRANSLATE_NOOP("articulation", "ulongfermata"),     100, 100  },
      { dlongfermataSym,     QT_TRANSLATE_NOOP("articulation", "dlongfermata"),     100, 100  },
      { uverylongfermataSym, QT_TRANSLATE_NOOP("articulation", "uverylongfermata"), 100, 100  },
      { dverylongfermataSym, QT_TRANSLATE_NOOP("articulation", "dverylongfermata"), 100, 100  },
      { thumbSym,          QT_TRANSLATE_NOOP("articulation", "thumb"),           100, 100  },
      { sforzatoaccentSym, QT_TRANSLATE_NOOP("articulation", "sforzato"),        120, 100  },
      { esprSym,           QT_TRANSLATE_NOOP("articulation", "espressivo"),      100, 100  },
      { staccatoSym,       QT_TRANSLATE_NOOP("articulation", "staccato"),        100,  50  },
      { ustaccatissimoSym, QT_TRANSLATE_NOOP("articulation", "ustaccatissimo"),  100, 100  },
      { dstaccatissimoSym, QT_TRANSLATE_NOOP("articulation", "dstaccatissimo"),  100, 100  },
      { tenutoSym,         QT_TRANSLATE_NOOP("articulation", "tenuto"),          100, 100  },
      { uportatoSym,       QT_TRANSLATE_NOOP("articulation", "uportato"),        100, 100  },
      { dportatoSym,       QT_TRANSLATE_NOOP("articulation", "dportato"),        100, 100  },
      { umarcatoSym,       QT_TRANSLATE_NOOP("articulation", "umarcato"),        110, 100  },
      { dmarcatoSym,       QT_TRANSLATE_NOOP("articulation", "dmarcato"),        110, 100  },
      { ouvertSym,         QT_TRANSLATE_NOOP("articulation", "ouvert"),          100, 100  },
      { plusstopSym,       QT_TRANSLATE_NOOP("articulation", "plusstop"),        100, 100  },
      { upbowSym,          QT_TRANSLATE_NOOP("articulation", "upbow"),           100, 100  },
      { downbowSym,        QT_TRANSLATE_NOOP("articulation", "downbow"),         100, 100  },
      { reverseturnSym,    QT_TRANSLATE_NOOP("articulation", "reverseturn"),     100, 100  },
      { turnSym,           QT_TRANSLATE_NOOP("articulation", "turn"),            100, 100  },
      { trillSym,          QT_TRANSLATE_NOOP("articulation", "trill"),           100, 100  },
      { prallSym,          QT_TRANSLATE_NOOP("articulation", "prall"),           100, 100  },
      { mordentSym,        QT_TRANSLATE_NOOP("articulation", "mordent"),         100, 100  },
      { prallprallSym,     QT_TRANSLATE_NOOP("articulation", "prallprall"),      100, 100  },
      { prallmordentSym,   QT_TRANSLATE_NOOP("articulation", "prallmordent"),    100, 100  },
      { upprallSym,        QT_TRANSLATE_NOOP("articulation", "upprall"),         100, 100  },
	{ downprallSym,      QT_TRANSLATE_NOOP("articulation", "downprall"),       100, 100  },
	{ upmordentSym,      QT_TRANSLATE_NOOP("articulation", "upmordent"),       100, 100  },
	{ downmordentSym,    QT_TRANSLATE_NOOP("articulation", "downmordent"),     100, 100  },
      { snappizzicatoSym,  QT_TRANSLATE_NOOP("articulation", "snappizzicato"),   100, 100  },
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
//   genPropertyMenu
//---------------------------------------------------------

bool Articulation::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Articulation Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Articulation::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            ArticulationProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   ArticulationProperties
//---------------------------------------------------------

ArticulationProperties::ArticulationProperties(Articulation* na, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      noteAttribute = na;

#if 0
      Part* part = st->staff()->part();
      Instrument* i = part->instrument();
      foreach(Channel* a, i->articulations) {
            if (a->name.isEmpty())
                  articulationList->addItem("normal");
            else
                  articulationList->addItem(a->name);
            }

      foreach(const NamedEventList& e, i->midiActions)
            midiActionList->addItem(e.name);

      articulationChange->setChecked(!st->articulationName().isEmpty());
      midiAction->setChecked(!st->midiActionName().isEmpty());

      if (!st->articulationName().isEmpty()) {
            QList<QListWidgetItem*> wl = articulationList
               ->findItems(st->articulationName(), Qt::MatchExactly);
            if (!wl.isEmpty())
                  articulationList->setCurrentRow(articulationList->row(wl[0]));
            }
      if (!st->midiActionName().isEmpty()) {
            QList<QListWidgetItem*> wl = midiActionList
               ->findItems(st->midiActionName(), Qt::MatchExactly);
            if (!wl.isEmpty())
                  midiActionList->setCurrentRow(midiActionList->row(wl[0]));
            }
#endif
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void ArticulationProperties::saveValues()
      {
#if 0
      if (articulationChange->isChecked()) {
            QListWidgetItem* i = articulationList->currentItem();
            if (i)
                  staffText->setChannelName(i->text());
            }
      if (midiAction->isChecked()) {
            QListWidgetItem* i = midiActionList->currentItem();
            if (i)
                  staffText->setMidiActionName(i->text());
            }
#endif
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
//   canvasPos
//---------------------------------------------------------

QPointF Articulation::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      ChordRest* cr = chordRest();
      if (cr == 0 || cr->parent() == 0)
            return pos();
      Measure* m = cr->measure();
      if (m == 0)
            return pos();
      System* system = m->system();
      if (system == 0)
            return pos();
      double yp = y() + system->staff(staffIdx() + cr->staffMove())->y() + system->y();
      return QPointF(xp, yp);
      }
