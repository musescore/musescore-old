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

#include "sparm_p.h"
#include "synti.h"
#include "mscore/xml.h"

//---------------------------------------------------------
//   SyntiParameterData
//---------------------------------------------------------

SyntiParameterData::SyntiParameterData()
      {
      _id   = -1;
      _type = P_FLOAT;
      _fval = 0.0;
      }

SyntiParameterData::SyntiParameterData(const QString& name, float val)
      {
      _id = -1;
      _name = name;
      _type = P_FLOAT;
      _fval = val;
      }

SyntiParameterData::SyntiParameterData(int i, const QString& name, float val)
      {
      _id   = i;
      _name = name;
      _type =  P_FLOAT;
      _fval = val;
      }

SyntiParameterData::SyntiParameterData(const QString& name, const QString& val)
      {
      _id   = -1;
      _name = name;
      _type =  P_STRING;
      _sval = val;
      }

SyntiParameterData::SyntiParameterData(int i, const QString& name, const QString& val)
      {
      _id   = i;
      _name = name;
      _type =  P_STRING;
      _sval = val;
      }

SyntiParameterData::SyntiParameterData(const SyntiParameterData& pd)
   : QSharedData(pd)
      {
      _id   = pd._id;
      _name = pd._name;
      _type = pd._type;
      switch(_type) {
            case P_FLOAT:
                  _fval = pd._fval;
                  _min  = pd._min;
                  _max  = pd._max;
                  break;
            case P_STRING:
                  _sval = pd._sval;
                  break;
            }
      }

SyntiParameter& SyntiParameter::operator=(const SyntiParameter& sp)
      {
      d = sp.d;
      return *this;
      }

bool SyntiParameterData::operator==(const SyntiParameterData& sp) const
      {
      if (_id == -1 ? (_name != sp._name) : (_id != sp._id))
            return false;
      switch(_type) {
            case P_FLOAT:
                  return qAbs(_fval - sp._fval) < 0.000001;
            case P_STRING:
                  return _sval == sp._sval;
            }
      return false;
      }

//---------------------------------------------------------
//   print
//    for debugging
//---------------------------------------------------------

void SyntiParameterData::print() const
      {
      if (_type == P_FLOAT)
            printf("<id=%d name=%s val=%f>", _id, qPrintable(_name), _fval);
      else if (_type == P_STRING)
            printf("<id=%d name=%s val=%s>", _id, qPrintable(_name), qPrintable(_sval));
      }

//---------------------------------------------------------
//   SyntiSettings::read
//---------------------------------------------------------

void SyntiState::read(XmlReader* r)
      {
      const xmlChar* tag;

      bool isFloat;
      while (r->readElement()) {
            if (r->tag() == "f")
                  isFloat = true;
            else if (r->tag() == "s")
                  isFloat = false;
            else
                  r->unknown();
            QString name;
            QString val;
            while (r->readAttribute()) {
                  if (r->tag() == "name")
                        name = r->stringValue();
                  else if (r->tag() == "val")
                        val = r->stringValue();
                  }
            if (isFloat) {
                  float f = val.toDouble();
                  append(SyntiParameter(name, f));
                  }
            else
                  append(SyntiParameter(name, val));
            }
      }

//---------------------------------------------------------
//   SyntiParameter
//---------------------------------------------------------

SyntiParameter::SyntiParameter()
      {
      d = new SyntiParameterData;
      }

SyntiParameter::SyntiParameter(const SyntiParameter& sp)
   : d(sp.d)
      {
      }

SyntiParameter::SyntiParameter(const QString& name, float val)
      {
      d = new SyntiParameterData(name, val);
      }

SyntiParameter::SyntiParameter(int id, const QString& name, float val)
      {
      d = new SyntiParameterData(id, name, val);
      }

SyntiParameter::SyntiParameter(const QString& name, const QString& val)
      {
      d = new SyntiParameterData(name, val);
      }

SyntiParameter::SyntiParameter(int id, const QString& name, const QString& val)
      {
      d = new SyntiParameterData(id, name, val);
      }

SyntiParameter::~SyntiParameter()
      {
      }

//---------------------------------------------------------
//   type
//---------------------------------------------------------

SyntiParameterType SyntiParameter::type() const
      {
      return d->_type;
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const QString& SyntiParameter::name() const
      {
      return d->_name;
      }

void SyntiParameter::setName(const QString& s)
      {
      d->_name = s;
      }

//---------------------------------------------------------
//   id
//---------------------------------------------------------

int SyntiParameter::id() const
      {
      return d->_id;
      }

//---------------------------------------------------------
//   setId
//---------------------------------------------------------

void SyntiParameter::setId(int v)
      {
      d->_id = v;
      }

//---------------------------------------------------------
//   sval
//---------------------------------------------------------

QString SyntiParameter::sval() const
      {
      return d->_sval;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void SyntiParameter::set(const QString& s)
      {
      d->_sval = s;
      }

void SyntiParameter::set(float v)
      {
      d->_fval = v;
      }

void SyntiParameter::set(const QString& s, float v, float min, float max)
      {
      d->_name = s;
      d->_fval = v;
      d->_min  = min;
      d->_max  = max;
      }

//---------------------------------------------------------
//   fval
//---------------------------------------------------------

float SyntiParameter::fval() const
      {
      return d->_fval;
      }

//---------------------------------------------------------
//   min
//---------------------------------------------------------

float SyntiParameter::min() const
      {
      return d->_min;
      }

//---------------------------------------------------------
//   max
//---------------------------------------------------------

float SyntiParameter::max() const
      {
      return d->_max;
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void SyntiParameter::setRange(float a, float b)
      {
      d->_min = a;
      d->_max = b;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool SyntiParameter::operator==(const SyntiParameter& sp) const
      {
      return d->operator==(*sp.d);
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void SyntiParameter::print() const
      {
      d->print();
      }

//---------------------------------------------------------
//   SyntiState
//---------------------------------------------------------

SyntiState::SyntiState()
      {
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool SyntiState::operator==(const SyntiState& st) const
      {
      int n = size();
      if (n != st.size())
            return false;
      for (int i = 0; i < n; ++i) {
            if (!(at(i) == st.at(i)))
                  return false;
            }
      return true;
      }

