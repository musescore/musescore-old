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
#include "score.h"
#include "script.h"

//---------------------------------------------------------
//   WrapperQMessageBox
//---------------------------------------------------------

WrapperQMessageBox::WrapperQMessageBox(QWidget* parent)
   : QMessageBox(parent)
      {
      }

//---------------------------------------------------------
//   qscript_call
//---------------------------------------------------------

QScriptValue WrapperQMessageBox::qscript_call(QWidget* parent)
      {
      QMessageBox* const iface = new WrapperQMessageBox(parent);
      return engine()->newQObject(iface, QScriptEngine::AutoOwnership);
      }

//---------------------------------------------------------
//   loadPlugins
//---------------------------------------------------------

void MuseScore::loadPlugins()
      {
      QDir pluginDir(mscoreGlobalShare + "plugins");
      QStringList nameFilters;
      nameFilters << "*.js";
      QStringList pluginList = pluginDir.entryList(nameFilters, QDir::Files, QDir::Name);
      QScriptEngine se(0);
      QSignalMapper* mapper = new QSignalMapper(this);
      connect(mapper, SIGNAL(mapped(int)), SLOT(pluginTriggered(int)));

      foreach(QString plugin, pluginList) {
            QString pluginPath(pluginDir.path() + "/" + plugin);
            QFile f(pluginPath);
            if (!f.open(QIODevice::ReadOnly)) {
                  if (debugMode)
                        printf("Loading Plugin <%s> failed\n", qPrintable(pluginPath));
                  continue;
                  }
            if (debugMode)
                  printf("Load Plugin <%s>\n", qPrintable(pluginPath));
            QScriptValue val  = se.evaluate(f.readAll(), plugin);
            f.close();
            QScriptValue init = val.property("init");
            if (!init.isFunction()) {
                  printf("Load plugin: no init function found\n");
                  continue;
                  }
            QScriptValue run = val.property("run");
            if (!run.isFunction()) {
                  printf("Load plugin: no run function found\n");
                  continue;
                  }
            int pluginIdx = plugins.size();
            plugins.append(pluginPath);

            init.call();
            QString menu = val.property("menu").toString();
            if (menu.isEmpty()) {
                  printf("Load plugin: no menu property\n");
                  continue;
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
                              connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
                              mapper->setMapping(a, pluginIdx);
                              }
                        else
                              curMenu = ((QMenu*)curMenu)->addMenu(m);
                        }
                  }
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

      QScriptValue v = se.newQObject(cs);
      se.globalObject().setProperty("score", v);
      v = se.newVariant(division);
      se.globalObject().setProperty("division", v);
      se.globalObject().setProperty("QMessageBox", se.newQObject(new WrapperQMessageBox, QScriptEngine::AutoOwnership));

      QScriptValue val = se.evaluate(f.readAll(), pluginPath);
      f.close();
      QScriptValue run = val.property("run");
      if (!run.isFunction()) {
            printf("Run plugin: no run function found\n");
            return;
            }
      run.call();
      }


