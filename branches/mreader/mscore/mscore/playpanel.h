//=============================================================================
//  MuseScore
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

#ifndef __PLAYPANEL_H__
#define __PLAYPANEL_H__

#include "ui_playpanel.h"

class Score;

//---------------------------------------------------------
//   PlayPanel
//---------------------------------------------------------

class PlayPanel : public QWidget, private Ui::PlayPanelBase {
      Q_OBJECT
      int cachedTickPosition;
      int cachedTimePosition;

      Score* cs;
      virtual void closeEvent(QCloseEvent*);

   private slots:
      void volumeChanged(double,int);
      void posChanged(int);
      void swingStyleChanged(int);

   signals:
      void relTempoChanged(double,int);
      void posChange(int);
      void volChange(float);      
      void closed();

   public slots:
      void setVolume(float);

   public:
      PlayPanel(QWidget* parent = 0);
      void heartBeat(int rpos, int apos);
      void heartBeat2(int sec);

      void setTempo(double);
      void setRelTempo(int);

      void setEndpos(int);
      void setScore(Score* s);
      };

#endif

