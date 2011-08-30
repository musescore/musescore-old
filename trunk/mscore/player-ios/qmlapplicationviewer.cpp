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

#include "qmlapplicationviewer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtDeclarative/QtDeclarative>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>

#include "libmscore/mscore.h"
#include "player-qt/scoreview.h"
#include "omr/omr.h"
#include "seq.h"

#include "player-qt/runtime.h"

bool debugMode = false;
QString revision;
Runtime* runtimeInstance;

//---------------------------------------------------------
//   QmlApplicationViewerPrivate
//---------------------------------------------------------

class QmlApplicationViewerPrivate
      {
      QString mainQmlFile;
      friend class QmlApplicationViewer;
      static QString adjustPath(const QString &path);
      };

//---------------------------------------------------------
//   adjustPath
//---------------------------------------------------------

QString QmlApplicationViewerPrivate::adjustPath(const QString &path)
      {
      if (!QDir::isAbsolutePath(path))
            return QCoreApplication::applicationDirPath()
               + QLatin1String("/../Resources/") + path;
      return path;
      }

//---------------------------------------------------------
//   QmlApplicationViewer
//---------------------------------------------------------

QmlApplicationViewer::QmlApplicationViewer(QWidget *parent)
   : QDeclarativeView(parent), m_d(new QmlApplicationViewerPrivate)
      {
      PDPI = 100;
      DPI  = PDPI;
      DPMM = DPI / INCH;

      runtimeInstance = new Runtime;

      MScore::init();
      seq = new Seq;
      seq->init();

      qmlRegisterType<ScoreView>("MuseScore", 1, 0, "ScoreView");

      QDeclarativeContext* ctxt = rootContext();
      ctxt->setContextProperty(QLatin1String("runtime"), runtimeInstance);

      // registering only for exposing the Runtime::Orientation enum
      qmlRegisterUncreatableType<Runtime>("Qt",      4, 7, "Orientation", QString());
      qmlRegisterUncreatableType<Runtime>("QtQuick", 1, 0, "Orientation", QString());

      connect(engine(), SIGNAL(quit()), SLOT(close()));
      setResizeMode(QDeclarativeView::SizeRootObjectToView);
      }

QmlApplicationViewer::~QmlApplicationViewer()
      {
      delete m_d;
      }

void QmlApplicationViewer::setMainQmlFile(const QString &file)
      {
      m_d->mainQmlFile = QmlApplicationViewerPrivate::adjustPath(file);
      setSource(QUrl::fromLocalFile(m_d->mainQmlFile));
      }

void QmlApplicationViewer::addImportPath(const QString &path)
      {
      engine()->addImportPath(QmlApplicationViewerPrivate::adjustPath(path));
      }

