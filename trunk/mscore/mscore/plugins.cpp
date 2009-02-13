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
#include "qscriptembeddeddebugger.h"
#include "scscore.h"
#include "scchord.h"
#include "sccursor.h"
#include "scnote.h"

Q_DECLARE_METATYPE(QByteArray*)
Q_DECLARE_METATYPE(ByteArrayClass*)

class ByteArrayClassPropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ByteArrayClassPropertyIterator(const QScriptValue &object);
      ~ByteArrayClassPropertyIterator();
      bool hasNext() const;
      void next();
      bool hasPrevious() const;
      void previous();
      void toFront();
      void toBack();
      QScriptString name() const;
      uint id() const;
      };

static qint32 toArrayIndex(const QString &str)
      {
      QByteArray bytes = str.toUtf8();
      char *eptr;
      quint32 pos = strtoul(bytes.constData(), &eptr, 10);
      if ((eptr == bytes.constData() + bytes.size()) && (QByteArray::number(pos) == bytes)) {
            return pos;
            }
      return -1;
      }

ByteArrayClass::ByteArrayClass(QScriptEngine *engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<QByteArray>(engine, toScriptValue, fromScriptValue);

      length = engine->toStringHandle(QLatin1String("length"));

      proto = engine->newQObject(new ByteArrayPrototype(this),
                                      QScriptEngine::QtOwnership,
                                      QScriptEngine::SkipMethodsInEnumeration
                                      | QScriptEngine::ExcludeSuperClassMethods
                                      | QScriptEngine::ExcludeSuperClassProperties);
      QScriptValue global = engine->globalObject();
      proto.setPrototype(global.property("Object").property("prototype"));

      ctor = engine->newFunction(construct);
      ctor.setData(qScriptValueFromValue(engine, this));
      }

ByteArrayClass::~ByteArrayClass()
      {
      }

QScriptClass::QueryFlags ByteArrayClass::queryProperty(const QScriptValue &object,
   const QScriptString &name, QueryFlags flags, uint *id)
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
      if (!ba)
            return 0;
      if (name == length) {
            return flags;
            }
      else {
            qint32 pos = toArrayIndex(name);
            if (pos == -1)
                  return 0;
            *id = pos;
            if ((flags & HandlesReadAccess) && (pos >= ba->size()))
                  flags &= ~HandlesReadAccess;
            return flags;
            }
      }

QScriptValue ByteArrayClass::property(const QScriptValue &object,
   const QScriptString &name, uint id)
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
      if (!ba)
            return QScriptValue();
      if (name == length) {
            return QScriptValue(engine(), ba->length());
            }
      else {
            qint32 pos = id;
            if ((pos < 0) || (pos >= ba->size()))
                  return QScriptValue();
            return QScriptValue(engine(), uint(ba->at(pos)) & 255);
            }
      return QScriptValue();
      }

void ByteArrayClass::setProperty(QScriptValue &object,
   const QScriptString &name, uint id, const QScriptValue &value)
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
      if (!ba)
            return;
      if (name == length) {
            ba->resize(value.toInt32());
            }
      else {
            qint32 pos = id;
            if (pos < 0)
                  return;
            if (ba->size() <= pos)
                  ba->resize(pos + 1);
            (*ba)[pos] = char(value.toInt32());
            }
      }

QScriptValue::PropertyFlags ByteArrayClass::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString &name, uint /*id*/)
      {
      if (name == length) {
            return QScriptValue::Undeletable | QScriptValue::SkipInEnumeration;
            }
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator *ByteArrayClass::newIterator(const QScriptValue &object)
      {
      return new ByteArrayClassPropertyIterator(object);
      }

QString ByteArrayClass::name() const
      {
      return QLatin1String("ByteArray");
      }

QScriptValue ByteArrayClass::prototype() const
      {
      return proto;
      }

QScriptValue ByteArrayClass::constructor()
      {
      return ctor;
      }

QScriptValue ByteArrayClass::newInstance(int size)
      {
      return newInstance(QByteArray(size, /*ch=*/0));
      }

QScriptValue ByteArrayClass::newInstance(const QByteArray &ba)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(ba));
      return engine()->newObject(this, data);
      }

QScriptValue ByteArrayClass::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ByteArrayClass *cls = qscriptvalue_cast<ByteArrayClass*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      int size = ctx->argument(0).toInt32();
      return cls->newInstance(size);
      }

QScriptValue ByteArrayClass::toScriptValue(QScriptEngine *eng, const QByteArray &ba)
      {
      QScriptValue ctor = eng->globalObject().property("ByteArray");
      ByteArrayClass *cls = qscriptvalue_cast<ByteArrayClass*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ByteArrayClass::fromScriptValue(const QScriptValue &obj, QByteArray &ba)
      {
      ba = qscriptvalue_cast<QByteArray>(obj.data());
      }

ByteArrayClassPropertyIterator::ByteArrayClassPropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

ByteArrayClassPropertyIterator::~ByteArrayClassPropertyIterator()
      {
      }

bool ByteArrayClassPropertyIterator::hasNext() const
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object().data());
      return m_index < ba->size();
      }

void ByteArrayClassPropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ByteArrayClassPropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ByteArrayClassPropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ByteArrayClassPropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ByteArrayClassPropertyIterator::toBack()
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object().data());
      m_index = ba->size();
      m_last = -1;
      }

QScriptString ByteArrayClassPropertyIterator::name() const
      {
      return QScriptString();
      }

uint ByteArrayClassPropertyIterator::id() const
      {
      return m_last;
      }

QByteArray *ByteArrayPrototype::thisByteArray() const
      {
      return qscriptvalue_cast<QByteArray*>(thisObject().data());
      }

void ByteArrayPrototype::chop(int n)
      {
      thisByteArray()->chop(n);
      }

bool ByteArrayPrototype::equals(const QByteArray &other)
      {
      return *thisByteArray() == other;
      }

QByteArray ByteArrayPrototype::left(int len) const
      {
      return thisByteArray()->left(len);
      }

QByteArray ByteArrayPrototype::mid(int pos, int len) const
      {
      return thisByteArray()->mid(pos, len);
      }

QScriptValue ByteArrayPrototype::remove(int pos, int len)
      {
      thisByteArray()->remove(pos, len);
      return thisObject();
      }

QByteArray ByteArrayPrototype::right(int len) const
      {
      return thisByteArray()->right(len);
      }

QByteArray ByteArrayPrototype::simplified() const
      {
      return thisByteArray()->simplified();
      }

QByteArray ByteArrayPrototype::toBase64() const
      {
      return thisByteArray()->toBase64();
      }

QByteArray ByteArrayPrototype::toLower() const
      {
      return thisByteArray()->toLower();
      }

QByteArray ByteArrayPrototype::toUpper() const
      {
      return thisByteArray()->toUpper();
      }

QByteArray ByteArrayPrototype::trimmed() const
      {
      return thisByteArray()->trimmed();
      }

void ByteArrayPrototype::truncate(int pos)
      {
      thisByteArray()->truncate(pos);
      }

QString ByteArrayPrototype::toLatin1String() const
      {
      return QString::fromLatin1(*thisByteArray());
      }

QScriptValue ByteArrayPrototype::valueOf() const
      {
      return thisObject().data();
      }

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
      importExtension("qt.core");
      importExtension("qt.gui");
      importExtension("qt.xml");
      importExtension("qt.network");
      importExtension("qt.uitools");
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

#ifdef HAS_SCRIPT_DEBUG
            if (scriptDebug) {
                  if (debugger == 0)
                        debugger = new QScriptEmbeddedDebugger();
                  debugger->attachTo(se);
                  debugger->breakAtFirstStatement();
                  }
#endif
#if QT_VERSION >= 0x040400
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
#endif
            }
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
      run.call();
      cs->end();
      }
