//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
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

#ifndef __ELEMENTLAYOUT_H__
#define __ELEMENTLAYOUT_H__

#include <QtCore/QPointF>
#include "globals.h"

class Element;
class Xml;
class XmlReader;

//---------------------------------------------------------
//   ElementLayout
//---------------------------------------------------------

class ElementLayout {

   protected:
      Align  _align;
      qreal _xoff, _yoff;
      OffsetType _offsetType;
      QPointF _reloff;

   public:
      ElementLayout();
      ElementLayout(Align a, qreal xo, qreal yo, OffsetType ot, qreal rx, qreal ry)
         : _align(a), _xoff(xo), _yoff(yo), _offsetType(ot), _reloff(rx, ry) {}

      Align align() const                   { return _align;        }
      OffsetType offsetType() const         { return _offsetType;   }
      qreal xoff() const                   { return _xoff;         }
      qreal yoff() const                   { return _yoff;         }
      QPointF reloff() const                { return _reloff;       }
      void setReloff(const QPointF& val)    { _reloff = val;        }
      void setAlign(Align val)              { _align  = val;        }
      void setXoff(qreal val)              { _xoff   = val;        }
      void setYoff(qreal val)              { _yoff   = val;        }
      void setOffsetType(OffsetType val)    { _offsetType = val;    }
      void layout(Element*) const;
      bool readProperties(XmlReader* e);
      void setRxoff(qreal v)               { _reloff.rx() = v; }
      void setRyoff(qreal v)               { _reloff.ry() = v; }
      qreal rxoff() const                  { return _reloff.x(); }
      qreal ryoff() const                  { return _reloff.y(); }
      };

#endif

