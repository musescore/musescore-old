//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ELEMENTLAYOUT_H__
#define __ELEMENTLAYOUT_H__

#include "mscore.h"

class Element;
class Xml;

//---------------------------------------------------------
//   ElementLayout
//---------------------------------------------------------

class ElementLayout {

   protected:
      Align  _align;
      double _xoff, _yoff;
      OffsetType _offsetType;
      QPointF _reloff;

   public:
      ElementLayout();
      ElementLayout(Align a, double xo, double yo, OffsetType ot, double rx, double ry)
         : _align(a), _xoff(xo), _yoff(yo), _offsetType(ot), _reloff(rx, ry) {}

      Align align() const                   { return _align;        }
      OffsetType offsetType() const         { return _offsetType;   }
      double xoff() const                   { return _xoff;         }
      double yoff() const                   { return _yoff;         }
      QPointF reloff() const                { return _reloff;       }
      void setReloff(const QPointF& val)    { _reloff = val;        }
      void setAlign(Align val)              { _align  = val;        }
      void setXoff(double val)              { _xoff   = val;        }
      void setYoff(double val)              { _yoff   = val;        }
      void setOffsetType(OffsetType val)    { _offsetType = val;    }
      void layout(Element*) const;
      void writeProperties(Xml& xml) const;
      bool readProperties(QDomElement e);
      void setRxoff(double v)               { _reloff.rx() = v; }
      void setRyoff(double v)               { _reloff.ry() = v; }
      double rxoff() const                  { return _reloff.x(); }
      double ryoff() const                  { return _reloff.y(); }
      };

#endif

