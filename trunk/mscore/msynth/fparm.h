//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __FPARM_H__
#define __FPARM_H__

class Xml;
class Synth;

enum ParameterType { P_FLOAT, P_STRING };

//---------------------------------------------------------
//   Parameter
//---------------------------------------------------------

class Parameter {
   protected:
      int _id;
      QString _name;

   public:
      Parameter() {}
      virtual Parameter* clone() const = 0;
      Parameter(int i, const QString& n) : _id(i), _name(n) {}
      virtual ParameterType type() const = 0;
      virtual void write(Xml&) const = 0;
      const QString& name() const { return _name; }
      int id() const { return _id; }
      };

//---------------------------------------------------------
//   Fparm
//---------------------------------------------------------

class Fparm : public Parameter {
      float  _val, _min, _max;

   public:
      Fparm() : Parameter() {}
      Fparm(int i, const QString& n, float val, float min, float max)
         : Parameter(i, n), _val(val), _min(min), _max(max) {}
      Fparm(int i, const QString& n, float val) : Parameter(i, n), _val(val) {}
      virtual Parameter* clone() const { return new Fparm(*this); }

      ParameterType type() const { return P_FLOAT; }
      virtual void write(Xml&) const;
      float val() const      { return _val; }
      float min() const      { return _min; }
      float max() const      { return _max; }
      void setVal(float val) { _val = val; }
      void setMin(float val) { _min = val; }
      void setMax(float val) { _max = val; }
      void set(const QString& name, float val, float min, float max);
      };

//---------------------------------------------------------
//   Sparm
//---------------------------------------------------------

class Sparm : public Parameter {
      QString _val;

   public:
      Sparm() : Parameter() {}
      virtual Parameter* clone() const { return new Sparm(*this); }
      Sparm(int i, const QString& n, const QString& v)
         : Parameter(i, n), _val(v) {}
      ParameterType type() const { return P_STRING; }
      virtual void write(Xml&) const;
      QString val() const                          { return _val; }
      void setVal(const QString& s)                { _val = s; }
      void set(const QString& n, const QString& s) { _name = n; _val = s; }
      };

//---------------------------------------------------------
//   SynthParams
//    Synthesizer parameter as saved in score.
//---------------------------------------------------------

struct SynthParams {
      Synth* synth;
      QList<Parameter*> params;

      void write(Xml&) const;
      };

//---------------------------------------------------------
//   SyntiSettings
//---------------------------------------------------------

class SyntiSettings : public QList<SynthParams> {

   public:
      SyntiSettings() {}
      void write(Xml&) const;
      void read(QDomElement);
      };

#endif

