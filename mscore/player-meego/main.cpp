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

#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"

#include "config.h"
#include "libmscore/mscore.h"
#include "scoreview.h"
#include "omr/omr.h"
#include "seq.h"

bool debugMode = false;
QString revision;

#ifdef OMR
Omr::Omr(Score*) {}
void Omr::read(QDomElement) {}
void Omr::write(Xml&) const {}
#endif

//---------------------------------------------------------
//   main
//---------------------------------------------------------

Q_DECL_EXPORT int main(int argc, char *argv[])
      {
      QScopedPointer<QApplication> app(createApplication(argc, argv));

      PDPI = 120;
      DPI  = PDPI;
      DPMM  = DPI / INCH;
      MScore::init();
      seq = new Seq;
      if (!seq->init())
            qFatal("seq init failed");
      qDebug("audio ok");
      const char* uri = "MuseScore";
      qmlRegisterType<ScoreView>(uri, 1, 0, "ScoreView");

      QmlApplicationViewer viewer;
      viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
      viewer.setMainQmlFile(QLatin1String("qml/muplayer/main.qml"));
      viewer.showExpanded();

      qDebug("app exec");
      return app->exec();
      }

