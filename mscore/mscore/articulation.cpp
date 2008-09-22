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

#include "articulation.h"
#include "sym.h"

//---------------------------------------------------------
//   Articulation::articulationList
//---------------------------------------------------------

ArticulationInfo Articulation::articulationList[] = {
      { ufermataSym,       QString("ufermata"),        A_TOP_STAFF    },
      { dfermataSym,       QString("dfermata"),        A_BOTTOM_STAFF },
      { thumbSym,          QString("thumb"),           A_CHORD        },
      { sforzatoaccentSym, QString("sforzato"),        A_CHORD        },
      { esprSym,           QString("espressivo"),      A_CHORD        },
      { staccatoSym,       QString("staccato"),        A_CHORD        },
      { ustaccatissimoSym, QString("ustaccatissimo"),  A_CHORD        },
      { dstaccatissimoSym, QString("dstaccatissimo"),  A_CHORD        },
      { tenutoSym,         QString("tenuto"),          A_CHORD        },
      { uportatoSym,       QString("uportato"),        A_CHORD        },
      { dportatoSym,       QString("dportato"),        A_CHORD        },
      { umarcatoSym,       QString("umarcato"),        A_CHORD        },
      { dmarcatoSym,       QString("dmarcato"),        A_CHORD        },
      { ouvertSym,         QString("ouvert"),          A_CHORD        },
      { plusstopSym,       QString("plusstop"),        A_CHORD        },
      { upbowSym,          QString("upbow"),           A_TOP_STAFF    },
      { downbowSym,        QString("downbow"),         A_TOP_STAFF    },
      { reverseturnSym,    QString("reverseturn"),     A_TOP_STAFF    },
      { turnSym,           QString("turn"),            A_TOP_STAFF    },
      { trillSym,          QString("trill"),           A_TOP_STAFF    },
      { prallSym,          QString("prall"),           A_TOP_STAFF    },
      { mordentSym,        QString("mordent"),         A_TOP_STAFF    },
      { prallprallSym,     QString("prallprall"),      A_TOP_STAFF    },
      { prallmordentSym,   QString("prallmordent"),    A_TOP_STAFF    },
      { upprallSym,        QString("upprall"),         A_TOP_STAFF    },
	{ downprallSym,      QString("downprall"),       A_TOP_STAFF    },
	{ upmordentSym,      QString("upmordent"),       A_TOP_STAFF    },
	{ downmordentSym,    QString("downmordent"),     A_TOP_STAFF    },
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
      xml.stag("Attribute");
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      Element::writeProperties(xml);
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
      setSubtype(name2idx(s));
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Articulation::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Properties..."));
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
      for (int i = 0; i < NOTE_ATTRIBUTES; ++i) {
            if (articulationList[i].name == s) {
                  return i;
                  }
            }
      return -1;
      }


