//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __DYNAMICS_H__
#define __DYNAMICS_H__

#include "text.h"
#include "mscore.h"

class Measure;
class Segment;

//---------------------------------------------------------
//   Dyn
//---------------------------------------------------------

struct Dyn {
      int velocity;           ///< associated midi velocity (0-127, -1 = none)
      bool accent;            ///< if true add velocity to current chord velocity
      const char* tag;

      Dyn(int velo, bool a, const char* t)
         : velocity(velo), accent(a), tag(t) {}
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

class Dynamic : public Text {
      Q_DECLARE_TR_FUNCTIONS(Dynamic)

      mutable QPointF dragOffset;
      int _velocity;          // associated midi velocity 0-127
      DynamicType _dynType;

   public:
      Dynamic(Score*);
      Dynamic(const Dynamic&);
      virtual Dynamic* clone() const   { return new Dynamic(*this); }
      virtual ElementType type() const { return DYNAMIC; }
      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual void layout();

      virtual void setSubtype(int val);
      virtual void setSubtype(const QString&);
      virtual const QString subtypeName() const;

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);

      virtual bool isEditable() const { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void endEdit();
      virtual void toDefault();

      void resetType();
      void setVelocity(int v);
      int velocity() const;
      DynamicType dynType() const    { return _dynType; }
      void setDynType(DynamicType t) { _dynType = t;    }


      virtual QLineF dragAnchor() const;
      };

extern Dyn dynList[];
#endif
