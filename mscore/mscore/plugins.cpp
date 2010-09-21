//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009-2010 Werner Schweer and others
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
#include "score.h"
#include "undo.h"
#include "globals.h"
#include "script.h"
#include "config.h"
#include "chord.h"
#include "note.h"
#include "utils.h"
#include "sccursor.h"

Q_DECLARE_METATYPE(Score*);
Q_DECLARE_METATYPE(SCursor*);

//---------------------------------------------------------
//   registerPlugin
//---------------------------------------------------------

void MuseScore::registerPlugin(const QString& pluginPath)
      {
      QFileInfo np(pluginPath);
      QString baseName = np.baseName();

      foreach(QString s, plugins) {
            QFileInfo fi(s);
            if (fi.baseName() == baseName) {
                  if (debugMode)
                        printf("  Plugin <%s> already registered\n", qPrintable(pluginPath));
                  return;
                  }
            }

      QFile f(pluginPath);
      if (!f.open(QIODevice::ReadOnly)) {
            if (debugMode)
                  printf("Loading Plugin <%s> failed\n", qPrintable(pluginPath));
            return;
            }
      if (debugMode)
            printf("Register Plugin <%s>\n", qPrintable(pluginPath));

      if (se == 0) {
            se = new ScriptEngine();
            se->installTranslatorFunctions();
            }
            
      //load translation
      QFileInfo fi(pluginPath);
      QString pPath = fi.absolutePath();
      QSettings settings;
      QString lName = settings.value("language", "system").toString();
      if (lName.toLower() == "system")
            lName = QLocale::system().name();
      QTranslator* translator = new QTranslator;
      if(translator->load("locale_"+lName, pPath+"/translations"))
            qApp->installTranslator(translator);
           
      QScriptValue val  = se->evaluate(f.readAll(), pluginPath);
      if (se->hasUncaughtException()) {
            QScriptValue sv = se->uncaughtException();
#if 0
            printf("Load plugin <%s>: line %d: %s\n",
               qPrintable(pluginPath),
               se->uncaughtExceptionLineNumber(),
               qPrintable(sv.toString()));
#endif
            QMessageBox::critical(0, "MuseScore Error",
               tr("Error loading plugin\n"
                  "\"%1\" line %2:\n"
                  "%3").arg(pluginPath)
                     .arg(se->uncaughtExceptionLineNumber())
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
      int majorVersion = val.property("majorVersion").toInt32();
      int minorVersion = val.property("minorVersion").toInt32();
      if (majorVersion) {
            if (majorVersion != SCRIPT_MAJOR_VERSION) {
                  QString s = tr("Script\n%1\nis incompatible with current interface");
                  QMessageBox::warning(0,
                     QWidget::tr("MuseScore: register script plugin:"),
                     s.arg(pluginPath),
                     QString::null, QString::null, QString::null, 0, 1);
                  }
            else if (minorVersion > SCRIPT_MINOR_VERSION) {
                  printf("Your MuseScore version may be too old to run script <%s> (minor version %d > %d)\n",
                     qPrintable(pluginPath), minorVersion, SCRIPT_MAJOR_VERSION);
                  QString s = tr("MuseScore is too old to run script\n%1");
                  QMessageBox::warning(0,
                     QWidget::tr("MuseScore: register script plugin:"),
                     s.arg(pluginPath),
                     QString::null, QString::null, QString::null, 0, 1);
                  }
            }

      int pluginIdx = plugins.size();
      plugins.append(pluginPath);
      
      //give access to pluginPath in init
      se->globalObject().setProperty("pluginPath", se->newVariant(pPath));
      
      init.call();
      QString menu = val.property("menu").toString();
      QString context = fi.baseName();
      menu = qApp->translate(qPrintable(context), qPrintable(menu));
      if (menu.isEmpty()) {
            printf("Load plugin: no menu property\n");
            return;
            }

      if (!pluginMapper)
            return;

      QStringList ml;
      QString s;
      bool escape = false;
      foreach (QChar c, menu) {
            if (escape) {
                  escape = false;
                  s += c;
                  }
            else {
                  if (c == '\\')
                        escape = true;
                  else {
                        if (c == '.') {
                              ml += s;
                              s = "";
                              }
                        else {
                              s += c;
                              }
                        }
                  }
            }
      if (!s.isEmpty())
            ml += s;

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
                  if (menu->objectName() == m || menu->title() == m) {
                        curMenu = menu;
                        found = true;
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
                        QStringList sl = m.split(":");
                        QAction* a = 0;
                        QMenu* cm = static_cast<QMenu*>(curMenu);
                        if (sl.size() == 2) {
                              QList<QAction*> al = cm->actions();
                              QAction* ba = 0;
                              foreach(QAction* ia, al) {
                                    if (ia->text() == sl[0]) {
                                          ba = ia;
                                          break;
                                          }
                                    }
                              a = new QAction(sl[1], 0);
                              cm->insertAction(ba, a);
                              }
                        else {
                              a = cm->addAction(m);
                              }
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
//   registerPlugin
//---------------------------------------------------------

void MuseScore::registerPlugin(QAction* a)
      {
      if (!pluginMapper) {
            printf("registerPlugin: no pluginMapper\n");
            return;
            }
      int pluginIdx = plugins.size() - 1; // plugin is already appended
      connect(a, SIGNAL(triggered()), pluginMapper, SLOT(map()));
      pluginMapper->setMapping(a, pluginIdx);
printf("registerPlugin: add action idx %d\n", pluginIdx);
      }

//---------------------------------------------------------
//   loadPlugins
//---------------------------------------------------------

void MuseScore::loadPlugins()
      {
      pluginMapper = new QSignalMapper(this);
      connect(pluginMapper, SIGNAL(mapped(int)), SLOT(pluginTriggered(int)));
      loadPluginDir(dataPath + "/plugins");
      loadPluginDir(mscoreGlobalShare + "plugins");
      }

void MuseScore::loadPluginDir(const QString& pluginPath)
      {
      if (debugMode)
            printf("Plugin Path <%s>\n", qPrintable(pluginPath));
      QDir pluginDir(pluginPath);
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
//   unloadPlugins
//---------------------------------------------------------

void MuseScore::unloadPlugins()
      {
      for(int idx = 0; idx < plugins.size() ; idx++){
          pluginExecuteFunction(idx, "onClose");
          }
      }

//---------------------------------------------------------
//   loadPlugin
//---------------------------------------------------------

bool MuseScore::loadPlugin(const QString& filename)
      {
      bool result = false;

      QDir pluginDir(mscoreGlobalShare + "plugins");
      if (debugMode)
            printf("Plugin Path <%s>\n", qPrintable(mscoreGlobalShare + "plugins"));

      if (filename.endsWith(".js")){
            QFileInfo fi(pluginDir, filename);
            if (fi.exists()) {
                  QString path(fi.filePath());
                  registerPlugin(path);
                  result = true;
                  }
            }
      return result;
      }

//---------------------------------------------------------
//   ScriptEngine
//---------------------------------------------------------

ScriptEngine::ScriptEngine()
   : QScriptEngine()
      {
      static const char* xts[] = {
            "qt.core", "qt.gui", "qt.xml", "qt.network", "qt.uitools"
            };
      for (unsigned i = 0; i < sizeof(xts)/sizeof(*xts); ++i) {
            importExtension(xts[i]);
            if (hasUncaughtException()) {
                  QScriptValue val = uncaughtException();
                  printf("%s\n", qPrintable(val.toString()));
                  }
            }

      //
      // create MuseScore bindings
      //

      globalObject().setProperty("Cursor",    create_Cursor_class(this),  QScriptValue::SkipInEnumeration);
      globalObject().setProperty("Score",     create_Score_class(this),   QScriptValue::SkipInEnumeration);
      globalObject().setProperty("Note",      create_Note_class(this),    QScriptValue::SkipInEnumeration);
      globalObject().setProperty("Chord",     create_Chord_class(this),   QScriptValue::SkipInEnumeration);
      globalObject().setProperty("Rest",      create_Rest_class(this),    QScriptValue::SkipInEnumeration);
      globalObject().setProperty("Harmony",   create_Harmony_class(this), QScriptValue::SkipInEnumeration);
      globalObject().setProperty("Text",      create_Text_class(this),    QScriptValue::SkipInEnumeration);
      globalObject().setProperty("Measure",   create_Measure_class(this), QScriptValue::SkipInEnumeration);
      globalObject().setProperty("Part",      create_Part_class(this),    QScriptValue::SkipInEnumeration);
      globalObject().setProperty("PageFormat",create_PageFormat_class(this),    QScriptValue::SkipInEnumeration);

      globalObject().setProperty("mscore",              newQObject(mscore));
      globalObject().setProperty("division",            newVariant(AL::division));
      globalObject().setProperty("mscoreVersion",       newVariant(version()));
      globalObject().setProperty("mscoreMajorVersion",  newVariant(majorVersion()));
      globalObject().setProperty("mscoreMinorVersion",  newVariant(minorVersion()));
      globalObject().setProperty("mscoreUpdateVersion", newVariant(updateVersion()));
      globalObject().setProperty("mscoreDPI",			newVariant(DPI));
      //globalObject().setProperty("localeName",          newVariant(lName));
      }

//---------------------------------------------------------
//   pluginTriggered
//---------------------------------------------------------

void MuseScore::pluginTriggered(int idx)
      {
      pluginExecuteFunction(idx, "run");
      }


void MuseScore::pluginExecuteFunction(int idx, const char* functionName)
      {
      QString pp = plugins[idx];
      QFile f(pp);
      if (!f.open(QIODevice::ReadOnly)) {
            if (debugMode)
                  printf("Loading Plugin <%s> failed\n", qPrintable(pp));
            return;
            }
      if (debugMode)
            printf("Run Plugin <%s> : <%s>\n", qPrintable(pp), functionName);
      if (se == 0) {
            se = new ScriptEngine();
            se->installTranslatorFunctions();
            if (debugMode) {
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
      if (scriptDebug) {
            if (debugger == 0) {
                  debugger = new QScriptEngineDebugger();
                  debugger->attachTo(se);
                  }
            debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
            }

      se->globalObject().setProperty("curScore", se->newVariant(qVariantFromValue(cs)));

      QFileInfo fi(f);
      pluginPath = fi.absolutePath();
      se->globalObject().setProperty("pluginPath", se->newVariant(pluginPath));

      QScriptValue val = se->evaluate(f.readAll(), pp);
      f.close();
      QScriptValue run = val.property(functionName);
      if (!run.isFunction()) {
            if (debugMode)
                printf("Execute plugin: no %s function found\n", functionName);
            return;
            }

      foreach(Score* s, scoreList)
            s->startCmd();
      run.call();
      if (se->hasUncaughtException()) {
            QScriptValue sv = se->uncaughtException();
            QMessageBox::critical(0, "MuseScore Error",
               tr("Error loading plugin\n"
                  "\"%1\" line %2:\n"
                  "%3").arg(pluginPath)
                     .arg(se->uncaughtExceptionLineNumber())
                     .arg(sv.toString())
               );
            }
      foreach(Score* s, scoreList)
          s->endCmd();
      if(cs)
          cs->end();
      }
