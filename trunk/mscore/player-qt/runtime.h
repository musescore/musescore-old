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

#include "deviceorientation.h"

//---------------------------------------------------------
//   Runtime
//---------------------------------------------------------

class Runtime : public QObject {
      Q_OBJECT
      Q_PROPERTY(DeviceOrientation::Orientation orientation READ orientation NOTIFY orientationChanged)

      Runtime(QObject* parent = 0)
         : QObject(parent)
            {
            connect(DeviceOrientation::instance(), SIGNAL(orientationChanged()),
               this, SIGNAL(orientationChanged()));
            }

   public:
      static Runtime* instance() {
            static Runtime* instance = 0;
            if (!instance)
                  instance = new Runtime;
            return instance;
            }
      DeviceOrientation::Orientation orientation() const {
            return DeviceOrientation::instance()->orientation();
            }

   Q_SIGNALS:
      void orientationChanged();
      };

#endif

