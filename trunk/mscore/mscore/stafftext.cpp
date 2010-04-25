//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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
      setTextStyle(TEXT_STYLE_STAFF);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffText::write(Xml& xml) const
      {
      xml.stag("StaffText");
      foreach(QString s, _midiActionNames)
            xml.tagE(QString("MidiAction name=\"%1\"").arg(s));
      if (!_channelName.isEmpty())
            xml.tagE(QString("channelSwitch name=\"%1\"").arg(_channelName));
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffText::read(QDomElement e)
      {
      _midiActionNames.clear();
      _channelName.clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "MidiAction")
                  _midiActionNames.append(e.attribute("name"));
            else if (tag == "channelSwitch" || tag == "articulationChange")
                  _channelName = e.attribute("name");
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
      Text::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Staff Text Properties..."));
      a->setData("sprops");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void StaffText::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "sprops") {
            StaffTextProperties rp(this);
            rp.exec();
            }
      else
            Text::propertyAction(viewer, s);
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
      foreach(const Channel& a, part->channel()) {
            if (a.name.isEmpty())
                  channelList->addItem(tr("normal"));
            else
                  channelList->addItem(a.name);
            }

      foreach(const NamedEventList& e, part->midiActions())
            midiActionList->addItem(e.name);

      channel->setChecked(!st->channelName().isEmpty());
      midiAction->setChecked(!st->midiActionNames()->isEmpty());

      if (!st->channelName().isEmpty()) {
            QList<QListWidgetItem*> wl = channelList->findItems(st->channelName(), Qt::MatchExactly);
            if (!wl.isEmpty())
                  channelList->setCurrentRow(channelList->row(wl[0]));
            }
      foreach(QString s, *st->midiActionNames()) {
            QList<QListWidgetItem*> wl = midiActionList->findItems(s, Qt::MatchExactly);
            foreach(QListWidgetItem* item, wl)
                  item->setSelected(true);
            }
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void StaffTextProperties::saveValues()
      {
      if (channel->isChecked()) {
            QListWidgetItem* i = channelList->currentItem();
            if (i)
                  staffText->setChannelName(i->text());
            }
      if (midiAction->isChecked()) {
            staffText->midiActionNames()->clear();
            for (int i = 0; i < midiActionList->count(); ++i) {
                  QListWidgetItem* item = midiActionList->item(i);
                  if (item->isSelected())
                        staffText->midiActionNames()->append(item->text());
                  }
            }
      }
