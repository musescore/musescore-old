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

#include "inspectorBeam.h"
#include "musescore.h"
#include "libmscore/beam.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorBeam
//---------------------------------------------------------

InspectorBeam::InspectorBeam(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);

      iList[0].t = P_STEM_DIRECTION;
      iList[0].w = b.direction;
      iList[0].r = b.resetDirection;

      iList[1].t = P_DISTRIBUTE;
      iList[1].w = b.distribute;
      iList[1].r = b.resetDistribute;

      iList[2].t = P_GROW_LEFT;
      iList[2].w = b.growLeft;
      iList[2].r = b.resetGrowLeft;

      iList[3].t = P_GROW_RIGHT;
      iList[3].w = b.growRight;
      iList[3].r = b.resetGrowRight;

      mapSignals();
      }

