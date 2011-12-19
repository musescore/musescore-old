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
      P_COLOR,
      P_VISIBLE,
      P_SMALL,
      P_SHOW_COURTESY,
      P_LINE_TYPE,
      P_TPC,
      P_ARTICULATION_ANCHOR,
      P_DIRECTION,
      P_STEM_DIRECTION,
      P_SLUR_DIRECTION,
      P_LEADING_SPACE,
      P_TRAILING_SPACE,
      P_DISTRIBUTE,
      };

enum P_DATA_TYPE {
      T_VARIANT,
      T_DIRECTION       // enum Direction
      };

//---------------------------------------------------------
//   template Property
//---------------------------------------------------------

template <class T>
class Property {
   public:
      int id;
      P_DATA_TYPE type;
      const char* name;                   // xml name of property
      QVariant (T::*get)() const;         // pointer to getter function
      void (T::*set)(const QVariant&);    // pointer to setter function
      };

#endif

