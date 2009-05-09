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

#ifndef __SCCURSORMEASURE_H__
#define __SCCURSORMEASURE_H__

class Score;
class Measure;
class SCursorMeasure;

typedef Measure* MeasurePtr;


//---------------------------------------------------------
//   SCursorMeasure
//---------------------------------------------------------

class SCursorMeasure {
      Score* _score;
      Measure* _measure;

   public:
      SCursorMeasure(Score*);
      SCursorMeasure() {}
      Measure* measure() const    { return _measure;  }
      void setMeasure(Measure* m) { _measure = m;     }
      Score* score() const        { return _score;    }
      void rewind();
      };

//---------------------------------------------------------
//   ScSCursorMeasure
//---------------------------------------------------------

class ScSCursorMeasure : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const SCursorMeasure&);
      static void fromScriptValue(const QScriptValue &obj, SCursorMeasure&);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScSCursorMeasure(QScriptEngine* se);
      ~ScSCursorMeasure() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const SCursorMeasure&);
      QueryFlags queryProperty(const QScriptValue& object,
         const QScriptString& name, QueryFlags flags, uint* id);
      QScriptValue property(const QScriptValue& obhect,
         const QScriptString& name, uint id);
      virtual void setProperty(QScriptValue& object, const QScriptString& name,
         uint id, const QScriptValue& value);
      QScriptValue::PropertyFlags propertyFlags(
         const QScriptValue& object, const QScriptString& name, uint id);
      QScriptClassPropertyIterator* newIterator(const QScriptValue& object);
      QString name() const           { return QLatin1String("CursorMeasure"); }
      QScriptValue prototype() const { return proto; }
      };

//---------------------------------------------------------
//   ScSCursorMeasurePrototype
//---------------------------------------------------------

class ScSCursorMeasurePrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      SCursorMeasure* thisSCursorMeasure() const;

   public:
      ScSCursorMeasurePrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScSCursorMeasurePrototype() {}

   public slots:
      void rewind();
      bool eos() const;
      MeasurePtr measure();
      bool next();
      };

Q_DECLARE_METATYPE(SCursorMeasure)
Q_DECLARE_METATYPE(SCursorMeasure*)
Q_DECLARE_METATYPE(ScSCursorMeasure*)
#endif

