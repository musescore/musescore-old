//=============================================================================
//  MusE Score
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

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "score.h"

//---------------------------------------------------------
//   ByteArrayClass
//---------------------------------------------------------

class ByteArrayClass : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const QByteArray &ba);
      static void fromScriptValue(const QScriptValue &obj, QByteArray &ba);

      QScriptString length;
      QScriptValue proto;
      QScriptValue ctor;

   public:
      ByteArrayClass(QScriptEngine* se);
      ~ByteArrayClass();
      QScriptValue constructor();
      QScriptValue newInstance(int size = 0);
      QScriptValue newInstance(const QByteArray& ba);
      QueryFlags queryProperty(const QScriptValue& object,
         const QScriptString& name, QueryFlags flags, uint* id);
      QScriptValue property(const QScriptValue& obhect,
         const QScriptString& name, uint id);
      void setProperty(QScriptValue& object, const QScriptString& name,
         uint id, const QScriptValue& value);
      QScriptValue::PropertyFlags propertyFlags(
         const QScriptValue& object, const QScriptString& name, uint id);
      QScriptClassPropertyIterator* newIterator(const QScriptValue& object);
      QString name() const;
      QScriptValue prototype() const;
      };

//---------------------------------------------------------
//   ByteArrayPrototype
//---------------------------------------------------------

class ByteArrayPrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      QByteArray *thisByteArray() const;

   public:
      ByteArrayPrototype(QObject *parent = 0) : QObject(parent) {}
      ~ByteArrayPrototype() {}

   public slots:
      void chop(int n);
      bool equals(const QByteArray &other);
      QByteArray left(int len) const;
      QByteArray mid(int pos, int len = -1) const;
      QScriptValue remove(int pos, int len);
      QByteArray right(int len) const;
      QByteArray simplified() const;
      QByteArray toBase64() const;
      QByteArray toLower() const;
      QByteArray toUpper() const;
      QByteArray trimmed() const;
      void truncate(int pos);
      QString toLatin1String() const;
      QScriptValue valueOf() const;
      };

typedef Score* ScorePtr;

//---------------------------------------------------------
//   ScScore
//    script proxy class for Score
//---------------------------------------------------------

class ScScore : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const ScorePtr& ba);
      static void fromScriptValue(const QScriptValue &obj, ScorePtr& ba);

      QScriptString scoreName, scoreStaves;
      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScScore(QScriptEngine* se);
      ~ScScore() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(const QString&);
      QScriptValue newInstance(const ScorePtr&);
      QueryFlags queryProperty(const QScriptValue& object,
         const QScriptString& name, QueryFlags flags, uint* id);
      QScriptValue property(const QScriptValue& obhect,
         const QScriptString& name, uint id);
      virtual void setProperty(QScriptValue& object, const QScriptString& name,
         uint id, const QScriptValue& value);
      QScriptValue::PropertyFlags propertyFlags(
         const QScriptValue& object, const QScriptString& name, uint id);
      QScriptClassPropertyIterator* newIterator(const QScriptValue& object);
      QString name() const           { return QLatin1String("Score"); }
      QScriptValue prototype() const { return proto; }
      };

//---------------------------------------------------------
//   ScScorePrototype
//---------------------------------------------------------

class ScScorePrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      Score* thisScore() const;

   public:
      ScScorePrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScScorePrototype() {}

   public slots:
      bool saveXml(const QString& name);
      bool saveMxl(const QString&);
      bool saveMidi(const QString&);
      bool savePng(const QString&);
      bool saveSvg(const QString&);
      bool saveLilypond(const QString&);
      };

#endif

