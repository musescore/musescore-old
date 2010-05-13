//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __STAFFTEXT_H__
#define __STAFFTEXT_H__

#include "text.h"
#include "ui_stafftext.h"
#include "part.h"

//---------------------------------------------------------
//   ChannelActions
//---------------------------------------------------------

struct ChannelActions {
      int channel;
      QStringList midiActionNames;
      };

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

class StaffText : public Text  {
      Q_DECLARE_TR_FUNCTIONS(StaffText)
      QString _channelNames[4];
      QList<ChannelActions> _channelActions;

   public:
      StaffText(Score*);
      virtual StaffText* clone() const { return new StaffText(*this); }
      virtual ElementType type() const { return STAFF_TEXT; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      QString channelName(int voice) const                { return _channelNames[voice]; }
      void setChannelName(int v, const QString& s)        { _channelNames[v] = s;        }
      const QList<ChannelActions>* channelActions() const { return &_channelActions;    }
      QList<ChannelActions>* channelActions()             { return &_channelActions;    }
      };

//---------------------------------------------------------
//   StaffTextProperties
//    Dialog
//---------------------------------------------------------

class StaffTextProperties : public QDialog, public Ui::StaffTextProperties {
      Q_OBJECT

      StaffText* staffText;
      QToolButton* vb[4][4];
      QComboBox* channelCombo[4];
      void saveChannel(int channel);

   private slots:
      void saveValues();
      void channelItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
      void voiceButtonClicked(int);

   public:
      StaffTextProperties(StaffText*, QWidget* parent = 0);
      };

#endif
