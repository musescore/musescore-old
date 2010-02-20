#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2002-2008 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

if (STATIC_SCRIPT_BINDINGS)
      set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Winvalid-pch -DQT_STATICPLUGIN")
      set(LIBTYPE "STATIC")
else (STATIC_SCRIPT_BINDINGS)
      set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Winvalid-pch")
      set(LIBTYPE "SHARED")
endif (STATIC_SCRIPT_BINDINGS)

if (GCC_VISIBILITY)
      set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif (GCC_VISIBILITY)

include_directories(${QT_INCLUDES})

add_library ( qtscript_core ${LIBTYPE}
      main.cpp
      qtscript_QBasicTimer.cpp
      qtscript_QBitArray.cpp
      qtscript_QBuffer.cpp
      qtscript_QByteArray.cpp
      qtscript_QByteArrayMatcher.cpp
      qtscript_QChildEvent.cpp
      qtscript_QCoreApplication.cpp
      qtscript_QCryptographicHash.cpp
      qtscript_QDataStream.cpp
      qtscript_QDir.cpp
      qtscript_QDirIterator.cpp
      qtscript_QDynamicPropertyChangeEvent.cpp
      qtscript_QEvent.cpp
      qtscript_QEventLoop.cpp
      qtscript_QFile.cpp
      qtscript_QFileInfo.cpp
      qtscript_QFileSystemWatcher.cpp
      qtscript_QFuture.cpp
      qtscript_QFutureIterator.cpp
      qtscript_QFutureSynchronizer.cpp
      qtscript_QFutureSynchronizerVoid.cpp
      qtscript_QFutureVoid.cpp
      qtscript_QFutureWatcher.cpp
      qtscript_QFutureWatcherVoid.cpp
      qtscript_QIODevice.cpp
      qtscript_QLibraryInfo.cpp
      qtscript_QLocale.cpp
      qtscript_QMutex.cpp
      qtscript_QObject.cpp
      qtscript_QPoint.cpp
      qtscript_QPointF.cpp
      qtscript_QProcess.cpp
      qtscript_QRect.cpp
      qtscript_QRectF.cpp
      qtscript_QRunnable.cpp
      qtscript_QSemaphore.cpp
      qtscript_QSettings.cpp
      qtscript_QSignalMapper.cpp
      qtscript_QSize.cpp
      qtscript_QSizeF.cpp
      qtscript_QSocketNotifier.cpp
      qtscript_QStringMatcher.cpp
      qtscript_QSystemSemaphore.cpp
      qtscript_QTemporaryFile.cpp
      qtscript_QTextBoundaryFinder.cpp
      qtscript_QTextCodec.cpp
      qtscript_QTextDecoder.cpp
      qtscript_QTextEncoder.cpp
      qtscript_QTextStream.cpp
      qtscript_QThreadPool.cpp
      qtscript_QTime.cpp
      qtscript_QTimeLine.cpp
      qtscript_QTimer.cpp
      qtscript_QTimerEvent.cpp
      qtscript_QTranslator.cpp
      qtscript_QUrl.cpp
      qtscript_QUuid.cpp
      qtscript_QWaitCondition.cpp
      qtscript_QXmlStreamAttribute.cpp
      qtscript_QXmlStreamAttributes.cpp
      qtscript_QXmlStreamEntityDeclaration.cpp
      qtscript_QXmlStreamEntityResolver.cpp
      qtscript_QXmlStreamNamespaceDeclaration.cpp
      qtscript_QXmlStreamNotationDeclaration.cpp
      qtscript_QXmlStreamReader.cpp
      qtscript_QXmlStreamWriter.cpp
      qtscript_Qt.cpp
      qtscript_QtConcurrent.cpp
      qtscriptshell_QBuffer.cpp
      qtscriptshell_QChildEvent.cpp
      qtscriptshell_QCoreApplication.cpp
      qtscriptshell_QDataStream.cpp
      qtscriptshell_QDirIterator.cpp
      qtscriptshell_QEvent.cpp
      qtscriptshell_QEventLoop.cpp
      qtscriptshell_QFile.cpp
      qtscriptshell_QFileSystemWatcher.cpp
      qtscriptshell_QFutureSynchronizer.cpp
      qtscriptshell_QFutureSynchronizerVoid.cpp
      qtscriptshell_QIODevice.cpp
      qtscriptshell_QObject.cpp
      qtscriptshell_QProcess.cpp
      qtscriptshell_QRunnable.cpp
      qtscriptshell_QSettings.cpp
      qtscriptshell_QSignalMapper.cpp
      qtscriptshell_QSocketNotifier.cpp
      qtscriptshell_QTemporaryFile.cpp
      qtscriptshell_QTextCodec.cpp
      qtscriptshell_QTextStream.cpp
      qtscriptshell_QThreadPool.cpp
      qtscriptshell_QTimeLine.cpp
      qtscriptshell_QTimer.cpp
      qtscriptshell_QTimerEvent.cpp
      qtscriptshell_QTranslator.cpp
      qtscriptshell_QUuid.cpp
      qtscriptshell_QXmlStreamAttributes.cpp
      qtscriptshell_QXmlStreamEntityResolver.cpp

      qtscript_QAbstractItemModel.cpp
      qtscript_QAbstractListModel.cpp
      qtscript_QAbstractTableModel.cpp
      qtscriptshell_QTextCodecPlugin.cpp
      qtscriptshell_QMimeData.cpp
      qtscriptshell_QAbstractItemModel.cpp
      qtscriptshell_QAbstractListModel.cpp
      qtscriptshell_QAbstractTableModel.cpp
      qtscript_QTextCodecPlugin.cpp
      qtscript_QPersistentModelIndex.cpp
      qtscript_QMimeData.cpp
      qtscript_QModelIndex.cpp
      )

set_target_properties(qtscript_core
      PROPERTIES
      COMPILE_FLAGS
         "-D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_SHARED -DQT_NO_DEBUG -include ${PROJECT_SOURCE_DIR}/all.h"
      LINK_FLAGS "-L${QT_LIBRARY_DIR}"
      )

if (STATIC_SCRIPT_BINDINGS)
else (STATIC_SCRIPT_BINDINGS)
   if (MINGW)
      target_link_libraries(
         qtscript_core
         ${QT_mingw_LIBRARIES}
         )
      install( TARGETS qtscript_core DESTINATION bin/script/ )
   else(MINGW)
      install( TARGETS qtscript_core DESTINATION ${QT_PLUGINS_DIR}/script/ )
   endif(MINGW)
endif (STATIC_SCRIPT_BINDINGS)

