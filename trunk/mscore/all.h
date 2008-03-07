//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id: allqt.h,v 1.24 2006/03/02 17:08:30 wschweer Exp $
//
//  Copyright (C) 2004-2006 Werner Schweer (ws@seh.de)
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

#ifndef __ALLQT_H__
#define __ALLQT_H__

#include <stdio.h>
#include <values.h>
#include <map>
#include <cmath>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <QtXml/QtXml>
#include <QtCore/QPointF>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QDateTime>
#include <QtCore/QtGlobal>
#include <QtCore/QtDebug>

#if QT_VERSION >= 0x040400
#include <QtCore/QAtomicInt>
#endif

#include <QtGui/QPainterPath>
#include <QtGui/QScrollArea>
#include <QtGui/QToolBar>
#include <QtGui/QWhatsThis>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>
#include <QtGui/QDockWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPrinter>
#include <QtGui/QPainter>
#include <QtGui/QComboBox>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QMainWindow>
#include <QtGui/QFileDialog>
#include <QtGui/QPrintDialog>
#include <QtGui/QColorDialog>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSplitter>
#ifndef __MINGW32__
#include <QtGui/QX11Info>
#endif
#include <QtGui/QFontDatabase>
#include <QtGui/QApplication>
#include <QtGui/QToolTip>
#include <QtCore/QProcess>
#include <QtGui/QDesktopServices>
#include <QtGui/QTextDocument>
#include <QtGui/QTextCursor>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QToolBox>
#include <QtGui/QToolButton>
#include <QtGui/QTextBlock>
#include <QtGui/QTextFrameFormat>
#include <QtGui/QClipboard>
#include <QtGui/QStatusBar>
#include <QtGui/QSplashScreen>
#include <QtGui/QPushButton>
#include <QtGui/QStylePainter>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QWizard>
#include <QtGui/QRadioButton>
#include <QtGui/QDirModel>
#include <QtGui/QHeaderView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QSvgGenerator>

#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>
#endif

