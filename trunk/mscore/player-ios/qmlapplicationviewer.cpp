#include "qmlapplicationviewer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include "libmscore/mscore.h"
#include "player-qt/scoreview.h"
#include "omr/omr.h"
#include "seq.h"

bool debugMode = false;
QString revision;

// dummies:

// Omr::Omr(Score*) {}
// void Omr::write(Xml&) const {}
// void Omr::read(QDomElement) {}

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

      MScore::init();
      seq = new Seq;
      seq->init();

      qmlRegisterType<ScoreView>("MuseScore", 1, 0, "ScoreView");

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

void QmlApplicationViewer::setOrientation(ScreenOrientation orientation)
      {
      printf("setOrientation %d\n", int(orientation));
      }

void QmlApplicationViewer::showExpanded()
      {
      show();
      }

