//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "staff.h"

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_STAFF);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffText::write(Xml& xml) const
      {
      xml.stag("StaffText");
      if (!_midiActionName.isEmpty())
            xml.tagE(QString("midiAction name=\"%1\"").arg(_midiActionName));
      if (!_articulationName.isEmpty())
            xml.tagE(QString("articulationChange name=\"%1\"").arg(_articulationName));
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffText::read(QDomElement e)
      {
      _midiActionName.clear();
      _articulationName.clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "midiAction")
                  _midiActionName = e.attribute("name");
            else if (tag == "articulationChange")
                  _articulationName = e.attribute("name");
            else if (!Text::readProperties(e))
                  domError(e);
            }
      cursorPos = 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool StaffText::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void StaffText::propertyAction(const QString& s)
      {
      if (s == "props") {
            StaffTextProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(s);
      }
//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffText::layout(ScoreLayout* l)
      {
      Text::layout(l);
      Measure* m = (Measure*)parent();
      double y = track() != -1 ? m->system()->staff(track() / VOICES)->y() : 0.0;
      double x = (tick() != -1) ? m->tick2pos(tick()) : 0.0;
      setPos(ipos() + QPointF(x, y));
      }

//---------------------------------------------------------
//   StaffTextProperties
//---------------------------------------------------------

StaffTextProperties::StaffTextProperties(StaffText* st, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      staffText = st;

      Part* part = st->staff()->part();
      Instrument* i = part->instrument();
      foreach(Articulation* a, i->articulations) {
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
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void StaffTextProperties::saveValues()
      {
      if (articulationChange->isChecked()) {
            QListWidgetItem* i = articulationList->currentItem();
            if (i)
                  staffText->setArticulationName(i->text());
            }
      if (midiAction->isChecked()) {
            QListWidgetItem* i = midiActionList->currentItem();
            if (i)
                  staffText->setMidiActionName(i->text());
            }
      }
