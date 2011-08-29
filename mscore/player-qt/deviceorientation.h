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

#ifndef __DEVICEORIENTATION_H__
#define __DEVICEORIENTATION_H__

//---------------------------------------------------------
//   DeviceOrientation
//---------------------------------------------------------

class DeviceOrientation : public QObject
      {
      Q_OBJECT
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

   protected:
      DeviceOrientation() { m_o = Landscape; }

   signals:
      void orientationChanged();

   public:
      Orientation orientation() const     { return m_o; }
      void setOrientation(Orientation o)  {
            if (o != m_o) {
                  m_o = o;
                  emit orientationChanged();
                  }
            }

      static DeviceOrientation* instance() {
            static DeviceOrientation* deviceOrientation = 0;
            if (!deviceOrientation)
                  deviceOrientation = new DeviceOrientation;
            return deviceOrientation;
            }
      };

QT_END_NAMESPACE

#endif
