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

#include "omrpanel.h"
#include "musescore.h"
#include "scoreview.h"
#include "omr/omrview.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   showOmrPanel
//---------------------------------------------------------

void MuseScore::showOmrPanel(bool visible)
      {
      QAction* a = getAction("omr");
      if (visible) {
            if (!omrPanel) {
                  omrPanel = new OmrPanel();
                  connect(omrPanel, SIGNAL(omrPanelVisible(bool)), a, SLOT(setChecked(bool)));
                  addDockWidget(Qt::RightDockWidgetArea, omrPanel);
                  }
            }
      if (omrPanel)
            omrPanel->setVisible(visible);
      if (omrPanel && visible) {
            if (cv && cv->omrView())
                  omrPanel->setOmrView(cv->omrView());
            else
                  omrPanel->setOmrView(0);
            }
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   OmrPanel
//---------------------------------------------------------

OmrPanel::OmrPanel(QWidget* parent)
   : QDockWidget(tr("Omr Panel"), parent)
      {
      setObjectName("omrpanel");
      setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      QWidget* mainWidget = new QWidget;
      mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      setWidget(mainWidget);
      layout = new QVBoxLayout;
      mainWidget->setLayout(layout);
      QHBoxLayout* hbox = new QHBoxLayout;

      showLines    = new QCheckBox;
      showLines->setText(tr("Show Lines"));

      showBarlines = new QCheckBox;
      showBarlines->setText(tr("Mark Barlines"));

      showSlices   = new QCheckBox;
      showSlices->setText(tr("Mark Slices"));

      showStaves   = new QCheckBox;
      showStaves->setText(tr("Mark Staves"));

      setOmrView(0);

      connect(showBarlines, SIGNAL(toggled(bool)), SLOT(showBarlinesToggled(bool)));
      connect(showLines,    SIGNAL(toggled(bool)), SLOT(showLinesToggled(bool)));
      connect(showSlices,   SIGNAL(toggled(bool)), SLOT(showSlicesToggled(bool)));
      connect(showStaves,   SIGNAL(toggled(bool)), SLOT(showStavesToggled(bool)));

      layout->addWidget(showLines);
      layout->addWidget(showBarlines);
      layout->addWidget(showSlices);
      layout->addWidget(showStaves);
      layout->addStretch(10);
      layout->addLayout(hbox);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void OmrPanel::closeEvent(QCloseEvent* ev)
      {
      emit omrPanelVisible(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   showBarlinesToggled
//---------------------------------------------------------

void OmrPanel::showBarlinesToggled(bool val)
      {
      if (omrView)
            omrView->setShowBarlines(val);
      }

//---------------------------------------------------------
//   showLinesToggled
//---------------------------------------------------------

void OmrPanel::showLinesToggled(bool val)
      {
      if (omrView)
            omrView->setShowLines(val);
      }

//---------------------------------------------------------
//   showSlicesToggled
//---------------------------------------------------------

void OmrPanel::showSlicesToggled(bool val)
      {
      if (omrView)
            omrView->setShowSlices(val);
      }

//---------------------------------------------------------
//   showStavesToggled
//---------------------------------------------------------

void OmrPanel::showStavesToggled(bool val)
      {
      if (omrView)
            omrView->setShowStaves(val);
      }

//---------------------------------------------------------
//   setOmr
//---------------------------------------------------------

void OmrPanel::setOmrView(OmrView* v)
      {
      omrView = v;
      if (omrView) {
            showBarlines->blockSignals(true);
            showLines->blockSignals(true);
            showSlices->blockSignals(true);
            showStaves->blockSignals(true);

            showBarlines->setEnabled(true);
            showBarlines->setChecked(omrView->showBarlines());

            showLines->setEnabled(true);
            showLines->setChecked(omrView->showLines());

            showSlices->setEnabled(true);
            showSlices->setChecked(omrView->showSlices());

            showStaves->setEnabled(true);
            showStaves->setChecked(omrView->showStaves());

            showBarlines->blockSignals(false);
            showLines->blockSignals(false);
            showSlices->blockSignals(false);
            showStaves->blockSignals(false);
            }
      else {
            showBarlines->setEnabled(false);
            showLines->setEnabled(false);
            showSlices->setEnabled(false);
            showStaves->setEnabled(false);
            }
      }


