//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __RUNTIME_H__
#define __RUNTIME_H__

//---------------------------------------------------------
//   Runtime
//---------------------------------------------------------

class Runtime : public QObject {
      Q_OBJECT
      Q_PROPERTY(Orientation orientation READ orientation NOTIFY orientationChanged)
      Q_ENUMS(Orientation)

   public:
      enum Orientation {
            UnknownOrientation,
            Portrait,
            Landscape,
            PortraitInverted,
            LandscapeInverted
            };
   private:
      Orientation m_o;

   public:
      Runtime(QObject* parent = 0) : QObject(parent) { m_o = Portrait;}

      Orientation orientation() const { return m_o; }

      void setOrientation(Orientation o)  {
            if (o != m_o) {
                  m_o = o;
                  emit orientationChanged();
                  }
            }

   Q_SIGNALS:
      void orientationChanged();
      };

extern Runtime* runtimeInstance;
#endif

