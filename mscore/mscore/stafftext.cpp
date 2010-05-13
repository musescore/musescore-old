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
      foreach(ChannelActions s, _channelActions) {
            int channel = s.channel;
            foreach(QString name, s.midiActionNames)
                  xml.tagE(QString("MidiAction channel=\"%1\" name=\"%2\"").arg(channel).arg(name));
            }
      for (int voice = 0; voice < VOICES; ++voice) {
            if (!_channelNames[voice].isEmpty())
                  xml.tagE(QString("channelSwitch voice=\"%1\" name=\"%2\"").arg(voice).arg(_channelNames[voice]));
            }
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffText::read(QDomElement e)
      {
      for (int voice = 0; voice < VOICES; ++voice)
            _channelNames[voice].clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "MidiAction") {
                  int channel = e.attribute("channel", 0).toInt();
                  QString name = e.attribute("name");
                  bool found = false;
                  int n = _channelActions.size();
                  for (int i = 0; i < n; ++i) {
                        ChannelActions* a = &_channelActions[i];
                        if (a->channel == channel) {
                              a->midiActionNames.append(name);
                              found = true;
                              break;
                              }
                        }
                  if (!found) {
                        ChannelActions a;
                        a.channel = channel;
                        a.midiActionNames.append(name);
                        _channelActions.append(a);
                        }
                  }
            else if (tag == "channelSwitch" || tag == "articulationChange") {
                  int voice = e.attribute("voice", "-1").toInt();
                  if (voice >= 0 && voice < VOICES)
                        _channelNames[voice] = e.attribute("name");
                  else if (voice == -1) {
                        // no voice applies channel to all voices for
                        // compatibility
                        for (int i = 0; i < VOICES; ++i)
                              _channelNames[i] = e.attribute("name");
                        }
                  }
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
//   initChannelCombo
//---------------------------------------------------------

static void initChannelCombo(QComboBox* cb, StaffText* st)
      {
      Part* part = st->staff()->part();
      foreach(const Channel& a, part->channel()) {
            if (a.name.isEmpty())
                  cb->addItem(cb->tr("normal"));
            else
                  cb->addItem(a.descr);
            }
      }

//---------------------------------------------------------
//   StaffTextProperties
//---------------------------------------------------------

StaffTextProperties::StaffTextProperties(StaffText* st, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      staffText = st;

      vb[0][0] = voice1_1;
      vb[0][1] = voice1_2;
      vb[0][2] = voice1_3;
      vb[0][3] = voice1_4;

      vb[1][0] = voice2_1;
      vb[1][1] = voice2_2;
      vb[1][2] = voice2_3;
      vb[1][3] = voice2_4;

      vb[2][0] = voice3_1;
      vb[2][1] = voice3_2;
      vb[2][2] = voice3_3;
      vb[2][3] = voice3_4;

      vb[3][0] = voice4_1;
      vb[3][1] = voice4_2;
      vb[3][2] = voice4_3;
      vb[3][3] = voice4_4;

      channelCombo[0] = channelCombo1;
      channelCombo[1] = channelCombo2;
      channelCombo[2] = channelCombo3;
      channelCombo[3] = channelCombo4;

      //---------------------------------------------------
      // setup "switch channel"
      //---------------------------------------------------

      for (int i = 0; i < 4; ++i)
            initChannelCombo(channelCombo[i], st);

      Part* part = st->staff()->part();
      int n = part->channel().size();
      int rows = 0;
      for (int voice = 0; voice < VOICES; ++voice) {
            if (st->channelName(voice).isEmpty())
                  continue;
            for (int i = 0; i < n; ++i) {
                  const Channel& a = part->channel(i);
                  if (a.name != st->channelName(voice))
                        continue;
                  int row = 0;
                  for (row = 0; row < rows; ++row) {
                        if (channelCombo[row]->currentIndex() == i) {
                              vb[voice][row]->setChecked(true);
                              break;
                              }
                        }
                  if (row == rows) {
                        vb[voice][rows]->setChecked(true);
                        channelCombo[row]->setCurrentIndex(i);
                        ++rows;
                        }
                  break;
                  }
            }
      QSignalMapper* mapper = new QSignalMapper(this);
      for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                  connect(vb[col][row], SIGNAL(clicked()), mapper, SLOT(map()));
                  mapper->setMapping(vb[col][row], (col << 8) + row);
                  }
            }
      connect(mapper, SIGNAL(mapped(int)), SLOT(voiceButtonClicked(int)));

      QTreeWidgetItem* selectedItem = 0;
      for (int i = 0; i < n; ++i) {
            const Channel& a = part->channel(i);
            QTreeWidgetItem* item = new QTreeWidgetItem(channelList);
            item->setData(0, Qt::UserRole, i);
            item->setText(0, a.name);
            item->setText(1, a.descr);
            if (i == 0)
                  selectedItem = item;
            }
      connect(channelList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
         SLOT(channelItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      channelList->setCurrentItem(selectedItem);
      }

//---------------------------------------------------------
//   voiceButtonClicked
//---------------------------------------------------------

void StaffTextProperties::voiceButtonClicked(int val)
      {
      int ccol = val >> 8;
      int crow = val & 0xff;
      for (int row = 0; row < 4; ++row) {
            if (row == crow)
                  continue;
            vb[ccol][row]->setChecked(false);
            }
      }

//---------------------------------------------------------
//   saveChannel
//---------------------------------------------------------

void StaffTextProperties::saveChannel(int channel)
      {
      QList<QTreeWidgetItem*> items = actionList->selectedItems();

      QList<ChannelActions>* ca = staffText->channelActions();
      int n = ca->size();
      for (int i = 0; i < n; ++i) {
            ChannelActions* a = &(*ca)[i];
            if (a->channel == channel) {
                  ca->removeAt(i);
                  break;
                  }
            }

      ChannelActions a;
      a.channel = channel;

      foreach(QTreeWidgetItem* item, items)
            a.midiActionNames.append(item->text(0));
      ca->append(a);
      }

//---------------------------------------------------------
//   channelItemChanged
//---------------------------------------------------------

void StaffTextProperties::channelItemChanged(QTreeWidgetItem* item, QTreeWidgetItem* pitem)
      {
      if (pitem)
            saveChannel(pitem->data(0, Qt::UserRole).toInt());
      if (item == 0)
            return;

      actionList->clear();
      Part* part = staffText->staff()->part();

      int channelIdx      = item->data(0, Qt::UserRole).toInt();
      Channel& channel    = part->channel(channelIdx);
      QString channelName = channel.name;

      foreach(const NamedEventList& e, part->midiActions()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(actionList);
            item->setText(0, e.name);
            item->setText(1, e.descr);
            }
      foreach(const NamedEventList& e, channel.midiActions) {
            QTreeWidgetItem* item = new QTreeWidgetItem(actionList);
            item->setText(0, e.name);
            item->setText(1, e.descr);
            }
      foreach(const ChannelActions& ca, *staffText->channelActions()) {
            if (ca.channel == channelIdx) {
                  foreach(QString s, ca.midiActionNames) {
                        QList<QTreeWidgetItem*> items = actionList->findItems(s, Qt::MatchExactly);
                        foreach(QTreeWidgetItem* item, items)
                              item->setSelected(true);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void StaffTextProperties::saveValues()
      {
      //
      // save channel switches
      //
      Part* part = staffText->staff()->part();
      for (int voice = 0; voice < VOICES; ++voice) {
            staffText->setChannelName(voice, QString());
            for (int row = 0; row < VOICES; ++row) {
                  if (vb[voice][row]->isChecked()) {
                        int idx = channelCombo[row]->currentIndex();
                        staffText->setChannelName(voice, part->channel()[idx].name);
                        break;
                        }
                  }
            }

      QTreeWidgetItem* pitem = channelList->currentItem();
      if (pitem)
            saveChannel(pitem->data(0, Qt::UserRole).toInt());

      staffText->score()->updateChannel();
      staffText->score()->setPlaylistDirty(true);
      }
