//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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
#include "script.h"
#include "config.h"

#if (QT_VERSION < 0x040500)
#include "qscriptembeddeddebugger.h"
#endif

#include "scscore.h"
#include "scchord.h"
#include "sccursor.h"
#include "scnote.h"
#include "sctext.h"
#include "scbytearray.h"

//---------------------------------------------------------
//   registerPlugin
//---------------------------------------------------------

void MuseScore::registerPlugin(const QString& pluginPath)
      {
      if (plugins.contains(pluginPath))
            return;

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
      if (se.hasUncaughtException()) {
            QScriptValue sv = se.uncaughtException();
#if 0
            printf("Load plugin <%s>: line %d: %s\n",
               qPrintable(pluginPath),
               se.uncaughtExceptionLineNumber(),
               qPrintable(sv.toString()));
#endif
            QMessageBox::critical(0, "MuseScore Error",
               QString("Error loading plugin\n"
                  "\"%1\" line %2:\n"
                  "%3").arg(pluginPath)
                     .arg(se.uncaughtExceptionLineNumber())
                     .arg(sv.toString())
               );
            return;
            }

      f.close();
      QScriptValue init = val.property("init");
      if (!init.isFunction()) {
            printf("Load plugin <%s>: no init function found\n", qPrintable(pluginPath));
            return;
            }
      QScriptValue run = val.property("run");
      if (!run.isFunction()) {
            printf("Load plugin <%s>: no run function found\n", qPrintable(pluginPath));
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
                  if (debugMode)
                        printf("check menu <%s><%s>\n", qPrintable(menu->objectName()), qPrintable(m));
                  if (menu->objectName() == m) {
                        curMenu = menu;
                        found = true;
                        if (debugMode)
                              printf("  found\n");
                        break;
                        }
                  }
            if (!found) {
                  if (i == 0) {
                        curMenu = new QMenu(m, menuBar());
                        curMenu->setObjectName(m);
                        menuBar()->insertMenu(menuBar()->actions().back(), (QMenu*)curMenu);
                        if (debugMode)
                              printf("add Menu <%s>\n", qPrintable(m));
                        }
                  else if (i + 1 == n) {
                        QAction* a = ((QMenu*)curMenu)->addAction(m);
                        connect(a, SIGNAL(triggered()), pluginMapper, SLOT(map()));
                        pluginMapper->setMapping(a, pluginIdx);
                        if (debugMode)
                              printf("add action <%s>\n", qPrintable(m));
                        }
                  else {
                        curMenu = ((QMenu*)curMenu)->addMenu(m);
                        if (debugMode)
                              printf("add menu <%s>\n", qPrintable(m));
                        }
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
      if (debugMode)
            printf("Plugin Path <%s>\n", qPrintable(mscoreGlobalShare + "plugins"));
      QDirIterator it(pluginDir, QDirIterator::Subdirectories);
      while (it.hasNext()) {
            it.next();
            QFileInfo fi = it.fileInfo();
            if (fi.isFile()) {
                  QString path(fi.filePath());
                  if (path.endsWith(".js"))
                        registerPlugin(path);
                  }
            }
      }

//---------------------------------------------------------
//   ScriptEngine
//---------------------------------------------------------

ScriptEngine::ScriptEngine()
   : QScriptEngine()
      {
#ifdef HAS_SCRIPT_INTERFACE
      const char* xts[] = {
            "qt.core", "qt.gui", "qt.xml", "qt.network", "qt.uitools"
            };
      for (unsigned i = 0; i < sizeof(xts)/sizeof(*xts); ++i) {
            importExtension(xts[i]);
            if (hasUncaughtException()) {
                  QScriptValue val = uncaughtException();
                  printf("%s\n", qPrintable(val.toString()));
                  }
            }
#endif

      baClass = new ByteArrayClass(this);
      globalObject().setProperty("ByteArray", baClass->constructor());

      scoreClass = new ScScore(this);
      globalObject().setProperty("Score", scoreClass->constructor());

      cursorClass = new ScSCursor(this);
      globalObject().setProperty("Cursor", cursorClass->constructor());

      ScChord* chordClass = new ScChord(this);
      globalObject().setProperty("Chord", chordClass->constructor());

      ScNote* noteClass = new ScNote(this);
      globalObject().setProperty("Note", noteClass->constructor());

      ScText* textClass = new ScText(this);
      globalObject().setProperty("Text", textClass->constructor());

      QScriptValue v = newVariant(division);
      globalObject().setProperty("division", v);
      }

//---------------------------------------------------------
//   pluginTriggered
//---------------------------------------------------------

void MuseScore::pluginTriggered(int idx)
      {
      QString pp = plugins[idx];
      QFile f(pp);
      if (!f.open(QIODevice::ReadOnly)) {
            if (debugMode)
                  printf("Loading Plugin <%s> failed\n", qPrintable(pp));
            return;
            }
      if (debugMode)
            printf("Run Plugin <%s>\n", qPrintable(pp));
      if (se == 0) {
            se = new ScriptEngine();
            if (debugMode) {
                  // > qt4.4 required
                  QStringList lp = qApp->libraryPaths();
                  foreach(const QString& s, lp)
                        printf("lib path <%s>\n", qPrintable(s));

                  QStringList sl = se->availableExtensions();
                  printf("available:\n");
                  foreach(const QString& s, sl)
                        printf("  <%s>\n", qPrintable(s));

                  sl = se->importedExtensions();
                  printf("imported:\n");
                  foreach(const QString& s, sl)
                        printf("  <%s>\n", qPrintable(s));
                  }
            }
#ifdef HAS_SCRIPT_DEBUG
      if (scriptDebug) {
            if (debugger == 0) {
                  debugger = new QScriptEngineDebugger();
                  debugger->attachTo(se);
                  }
            debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
            }
#endif
      QScriptValue v = se->getScoreClass()->newInstance(cs);
      se->globalObject().setProperty("curScore", v);

      SCursor c;
      v = se->getCursorClass()->newInstance(c);
      se->globalObject().setProperty("curCursor", v);

      QFileInfo fi(f);
      pluginPath = fi.absolutePath();
      v = se->newVariant(pluginPath);
      se->globalObject().setProperty("pluginPath", v);

      QScriptValue val = se->evaluate(f.readAll(), pp);
      f.close();
      QScriptValue run = val.property("run");
      if (!run.isFunction()) {
            printf("Run plugin: no run function found\n");
            return;
            }

      foreach(Score* s, scoreList)
            s->startCmd();
      run.call();
      foreach(Score* s, scoreList)
            s->endCmd();
      cs->end();
      }

