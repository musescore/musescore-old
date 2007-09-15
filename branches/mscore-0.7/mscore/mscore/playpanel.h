//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: playpanel.h,v 1.8 2006/03/02 17:08:41 wschweer Exp $
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

      Score* cs;
      virtual void closeEvent(QCloseEvent*);

   private slots:
      void volumeChanged(int);
      void posChanged(int);

   signals:
      void relTempoChanged(int);
      void posChange(int);
      void volChange(float);
      void closed();

   public:
      PlayPanel(QWidget* parent = 0);
      void heartBeat(int rpos, int apos);

      void setTempo(double);
      void setRelTempo(int);

      void setVolume(float);
      void setEndpos(int);
      void setScore(Score* s);
      };

#endif

