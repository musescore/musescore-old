//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __MREADER_H__
#define __MREADER_H__

#include "ui_mreader.h"

class ScanView;

//---------------------------------------------------------
//   MuseReader
//---------------------------------------------------------

class MuseReader : public QMainWindow, public Ui::MuseReader {
      Q_OBJECT

      QString lastOpenPath;
      ScanView* _scanView;
      QSpinBox* pageNumber;
      QLabel* maxPages;
      QLabel* xLabel;
      QLabel* yLabel;

      virtual void closeEvent(QCloseEvent*);
      void writeSettings();
      bool save(const QString& fn);

   private slots:
      void load();
      void save();

   public:
      MuseReader(QWidget* parent = 0);
      void readSettings();
      bool load(const QString& fn);
      };

#endif

