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

#ifndef __SCSCORE_H__
#define __SCSCORE_H__

#include "score.h"

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

Q_DECLARE_METATYPE(ScorePtr)
Q_DECLARE_METATYPE(ScorePtr*)
Q_DECLARE_METATYPE(ScScore*)

#endif

