//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "mscore.h"
#include "globals.h"
#include "score.h"
#include "script.h"
#include "config.h"
#include "qscriptembeddeddebugger.h"

//---------------------------------------------------------
//   registerPlugin
//---------------------------------------------------------

void MuseScore::registerPlugin(const QString& pluginPath)
      {
      QFile f(pluginPath);
      if (!f.open(QIODevice::ReadOnly)) {
            if (debugMode)
                  printf("Loading Plugin <%s> failed\n", qPrintable(pluginPath));
            return;
            }
      if (debugMode)
            printf("Register Plugin <%s>\n", qPrintable(pluginPath));

      QScriptEngine se(0);
      QScriptValue val  = se.evaluate(f.readAll(), pluginPath);
      f.close();
      QScriptValue init = val.property("init");
      if (!init.isFunction()) {
            printf("Load plugin: no init function found\n");
            return;
            }
      QScriptValue run = val.property("run");
      if (!run.isFunction()) {
            printf("Load plugin: no run function found\n");
            return;
            }
      int pluginIdx = plugins.size();
      plugins.append(pluginPath);

      init.call();
      QString menu = val.property("menu").toString();
      if (menu.isEmpty()) {
            printf("Load plugin: no menu property\n");
            return;
            }
      QStringList ml   = menu.split(".", QString::SkipEmptyParts);
      int n            = ml.size();
      QWidget* curMenu = menuBar();

      for(int i = 0; i < n; ++i) {
            QString m  = ml[i];
            bool found = false;
            QList<QObject*> ol = curMenu->children();
            foreach(QObject* o, ol) {
                  QMenu* menu = qobject_cast<QMenu*>(o);
                  if (!menu)
                        continue;
                  if (menu->objectName() == m) {
                        curMenu = menu;
                        found = true;
                        break;
                        }
                  }
            if (!found) {
                  if (i == 0) {
                        curMenu = new QMenu(m, menuBar());
                        menuBar()->insertMenu(menuBar()->actions().back(), (QMenu*)curMenu);
                        }
                  else if (i + 1 == n) {
                        QAction* a = ((QMenu*)curMenu)->addAction(m);
                        connect(a, SIGNAL(triggered()), pluginMapper, SLOT(map()));
                        pluginMapper->setMapping(a, pluginIdx);
                        }
                  else
                        curMenu = ((QMenu*)curMenu)->addMenu(m);
                  }
            }
      }

//---------------------------------------------------------
//   loadPlugins
//---------------------------------------------------------

void MuseScore::loadPlugins()
      {
      pluginMapper = new QSignalMapper(this);
      connect(pluginMapper, SIGNAL(mapped(int)), SLOT(pluginTriggered(int)));

      QDir pluginDir(mscoreGlobalShare + "plugins");
      QDirIterator it(pluginDir, QDirIterator::Subdirectories);
      while (it.hasNext()) {
            QFileInfo fi = it.fileInfo();
            if (fi.isFile()) {
                  QString path(fi.filePath());
                  if (path.endsWith(".js"))
                        registerPlugin(path);
                  }
            it.next();
            }
      }

//---------------------------------------------------------
//   pluginTriggered
//---------------------------------------------------------

void MuseScore::pluginTriggered(int idx)
      {
      QString pluginPath = plugins[idx];
      QFile f(pluginPath);
      if (!f.open(QIODevice::ReadOnly)) {
            if (debugMode)
                  printf("Loading Plugin <%s> failed\n", qPrintable(pluginPath));
            return;
            }
      if (debugMode)
            printf("Run Plugin <%s>\n", qPrintable(pluginPath));
      QScriptEngine se(0);
      se.importExtension("qt.core");
      se.importExtension("qt.gui");
      se.importExtension("qt.xml");
      se.importExtension("qt.network");
      se.importExtension("qt.uitools");

#ifdef HAS_SCRIPT_DEBUG
      QScriptEmbeddedDebugger debugger;
      if (scriptDebug) {
            debugger.attachTo(&se);
            debugger.breakAtFirstStatement();
            }
#endif

#if QT_VERSION >= 0x040400
      if (debugMode) {
            QStringList sl = se.availableExtensions();
            printf("available:\n");
            foreach(QString s, sl)
                  printf("  <%s>\n", qPrintable(s));

            sl = se.importedExtensions();
            printf("imported:\n");
            foreach(QString s, sl)
                  printf("  <%s>\n", qPrintable(s));
            }
#endif

      QScriptValue v = se.newQObject(cs);
      se.globalObject().setProperty("score", v);
      v = se.newVariant(division);
      se.globalObject().setProperty("division", v);
      QFileInfo fi(f);
      v = se.newVariant(fi.absolutePath());
      se.globalObject().setProperty("pluginPath", v);

      QScriptValue val = se.evaluate(f.readAll(), pluginPath);
      f.close();
      QScriptValue run = val.property("run");
      if (!run.isFunction()) {
            printf("Run plugin: no run function found\n");
            return;
            }
      run.call();

#ifdef HAS_SCRIPT_DEBUG
      if (scriptDebug) {
            debugger.detach();
            }
#endif
      }


