//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PROPERTY_H__
#define __PROPERTY_H__

//------------------------------------------------------------------------
//   Element Properties
//    accessible through
//    virtual QVariant Element::getProperty(int propertyId)
//    virtual void Element::setProperty(int propertyId, const QVariant&)
//------------------------------------------------------------------------

enum P_TYPE {
      P_SUBTYPE,
      P_SELECTED,
      P_COLOR,
      P_VISIBLE,
      P_SMALL,
      P_SHOW_COURTESY,
      P_LINE_TYPE,
      P_PITCH,
      P_TPC,
      P_HEAD_TYPE,
      P_HEAD_GROUP,
      P_VELO_TYPE,
      P_VELO_OFFSET,
      P_ARTICULATION_ANCHOR,
      P_DIRECTION,
      P_STEM_DIRECTION,
      P_NO_STEM,
      P_SLUR_DIRECTION,
      P_LEADING_SPACE,
      P_TRAILING_SPACE,
      P_DISTRIBUTE,
      P_MIRROR_HEAD,
      P_DOT_POSITION,
      P_ONTIME_OFFSET,
      P_OFFTIME_OFFSET,
      P_TUNING,
      P_PAUSE,
      P_BARLINE_SPAN,
      P_USER_OFF,
      P_FRET,
      P_STRING,
      P_GHOST,
      P_END
      };

enum P_DATA_TYPE {
      T_SUBTYPE,
      T_BOOL,
      T_INT,
      T_REAL,
      T_POINT,
      T_COLOR,
      T_DIRECTION,      // enum Direction
      T_DIRECTION_H,    // enum DirectionH
      T_LAYOUT_BREAK,
      T_VALUE_TYPE
      };

extern void setProperty(P_DATA_TYPE, void*, const QString& value);
extern void setProperty(P_DATA_TYPE, void*, const QVariant& value);
extern QVariant getProperty(P_DATA_TYPE, void*);
extern QVariant getProperty(P_DATA_TYPE type, const QDomElement& e);

//---------------------------------------------------------
//   template Property
//---------------------------------------------------------

template <class T>
class Property {
   public:
      int id;
      P_DATA_TYPE type;
      const char* name;     // xml name of property
      void* (T::*data)();   // member function returns pointer to data
      void* defaultVal;     // pointer to default data

      void setProperty(T* c, const QDomElement& e) {
            QVariant v = ::getProperty(type, e);
            ::setProperty(type, ((*c).*(data))(), v);
            }
      };

//---------------------------------------------------------
//   property
//---------------------------------------------------------

template <class T>
Property<T>* property(Property<T>* list, int id)
      {
      for (int i = 0; ; ++i) {
            if (list[i].id == P_END)
                  break;
            else if (list[i].id == id)
                  return &list[i];
            }
      return 0;
      }

template <class T>
Property<T>* property(Property<T>* list, const QString& name)
      {
      for (int i = 0; ; ++i) {
            if (list[i].id == P_END)
                  break;
            else if (list[i].name == name)
                  return &list[i];
            }
      return 0;
      }

#include "xml.h"

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

#define WRITE_PROPERTIES(T) \
      for (int i = 0;; ++i) {  \
            const Property<T>& p = propertyList[i]; \
            if (p.id == P_END) \
                  break;       \
            xml.tag(p.name, p.type, ((*(T*)this).*(p.data))(), p.defaultVal); \
            }
#endif

