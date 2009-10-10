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

//---------------------------------------------------------
//   Articulation::articulationList
//---------------------------------------------------------

ArticulationInfo Articulation::articulationList[ARTICULATIONS] = {
      { ufermataSym,       QString("ufermata"),        100, 100  },
      { dfermataSym,       QString("dfermata"),        100, 100  },
      { thumbSym,          QString("thumb"),           100, 100  },
      { sforzatoaccentSym, QString("sforzato"),        120, 100  },
      { esprSym,           QString("espressivo"),      100, 100  },
      { staccatoSym,       QString("staccato"),        100,  50  },
      { ustaccatissimoSym, QString("ustaccatissimo"),  100, 100  },
      { dstaccatissimoSym, QString("dstaccatissimo"),  100, 100  },
      { tenutoSym,         QString("tenuto"),          100, 100  },
      { uportatoSym,       QString("uportato"),        100, 100  },
      { dportatoSym,       QString("dportato"),        100, 100  },
      { umarcatoSym,       QString("umarcato"),        110, 100  },
      { dmarcatoSym,       QString("dmarcato"),        110, 100  },
      { ouvertSym,         QString("ouvert"),          100, 100  },
      { plusstopSym,       QString("plusstop"),        100, 100  },
      { upbowSym,          QString("upbow"),           100, 100  },
      { downbowSym,        QString("downbow"),         100, 100  },
      { reverseturnSym,    QString("reverseturn"),     100, 100  },
      { turnSym,           QString("turn"),            100, 100  },
      { trillSym,          QString("trill"),           100, 100  },
      { prallSym,          QString("prall"),           100, 100  },
      { mordentSym,        QString("mordent"),         100, 100  },
      { prallprallSym,     QString("prallprall"),      100, 100  },
      { prallmordentSym,   QString("prallmordent"),    100, 100  },
      { upprallSym,        QString("upprall"),         100, 100  },
	{ downprallSym,      QString("downprall"),       100, 100  },
	{ upmordentSym,      QString("upmordent"),       100, 100  },
	{ downmordentSym,    QString("downmordent"),     100, 100  },
	};

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(Score* s)
   : Symbol(s)
      {
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

void Articulation::propertyAction(const QString& s)
      {
      if (s == "props") {
            ArticulationProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(s);
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
                  articulationList->addItem(tr("normal"));
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
