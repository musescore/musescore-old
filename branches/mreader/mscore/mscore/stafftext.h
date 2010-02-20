//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
//   StaffText
//---------------------------------------------------------

class StaffText : public Text  {
      Q_DECLARE_TR_FUNCTIONS(StaffText)
      QString _channelName;
      QString _midiActionName;

   public:
      StaffText(Score*);
      virtual StaffText* clone() const { return new StaffText(*this); }
      virtual ElementType type() const { return STAFF_TEXT; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      QString channelName() const                { return _channelName;      }
      QString midiActionName() const             { return _midiActionName;   }
      void setChannelName(const QString& s)      { _channelName = s;         }
      void setMidiActionName(const QString& s)   { _midiActionName = s;      }
      };

//---------------------------------------------------------
//   StaffTextProperties
//    Dialog
//---------------------------------------------------------

class StaffTextProperties : public QDialog, public Ui::StaffTextProperties {
      Q_OBJECT

      StaffText* staffText;

   private slots:
      void saveValues();

   public:
      StaffTextProperties(StaffText*, QWidget* parent = 0);
      };

#endif
