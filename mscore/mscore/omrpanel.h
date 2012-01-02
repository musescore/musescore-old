//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __OMRPANEL_H__
#define __OMRPANEL_H__

class OmrView;

//---------------------------------------------------------
//   OmrPanel
//---------------------------------------------------------

class OmrPanel : public QDockWidget {
      Q_OBJECT

      QVBoxLayout* layout;
      OmrView* omrView;

      QCheckBox* showLines;
      QCheckBox* showBarlines;
      QCheckBox* showSlices;
      QCheckBox* showStaves;

      virtual void closeEvent(QCloseEvent*);

   private slots:
      void showBarlinesToggled(bool);
      void showLinesToggled(bool);
      void showSlicesToggled(bool);
      void showStavesToggled(bool);

   signals:
      void omrPanelVisible(bool);

   public slots:

   public:
      OmrPanel(QWidget* parent = 0);
      void setOmrView(OmrView*);
      };

#endif

