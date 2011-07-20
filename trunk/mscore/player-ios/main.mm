#import <UIKit/UIKit.h>

#include "qmlapplicationviewer.h"

#include <QtGui/QApplication>
#include <QtCore/QtPlugin>
#include <QtDeclarative/QDeclarativeEngine>

Q_IMPORT_PLUGIN(UIKit)

//---------------------------------------------------------
//   qStringFromNSString
//---------------------------------------------------------

static QString qStringFromNSString(NSString *nsstring)
      {
	return QString::fromUtf8([nsstring UTF8String]);
      }

//---------------------------------------------------------
//   documentsDirectory
//---------------------------------------------------------

static QString documentsDirectory()
      {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	return qStringFromNSString(documentsDirectory);
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char *argv[])
      {
      NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

      QApplication app(argc, argv);
      QmlApplicationViewer viewer;
      viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
	viewer.engine()->setOfflineStoragePath(documentsDirectory());
      NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
      viewer.setMainQmlFile(qStringFromNSString([resourcePath stringByAppendingPathComponent:@"main.qml"]));
      viewer.showMaximized();
      int retVal = app.exec();
      [pool release];
      return retVal;
      }

