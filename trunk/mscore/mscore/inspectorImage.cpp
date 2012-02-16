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

#include "inspectorImage.h"
#include "musescore.h"
#include "libmscore/image.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorImage
//---------------------------------------------------------

InspectorImage::InspectorImage(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);

      iList[0].t = P_AUTOSCALE;
      iList[0].w = b.autoscale;
      iList[0].r = b.resetAutoscale;

      iList[1].t  = P_SIZE;
      iList[1].sv = 0;
      iList[1].w  = b.sizeWidth;
      iList[1].r  = 0;

      iList[2].t  = P_SIZE;
      iList[2].sv = 1;
      iList[2].w  = b.sizeHeight;
      iList[2].r  = 0;

      iList[3].t = P_LOCK_ASPECT_RATIO;
      iList[3].w = b.lockAspectRatio;
      iList[3].r = b.resetLockAspectRatio;

      iList[4].t = P_SIZE_IS_SPATIUM;
      iList[4].w = b.sizeIsSpatium;
      iList[4].r = b.resetSizeIsSpatium;

      mapSignals();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorImage::valueChanged(int idx)
      {
      QDoubleSpinBox* b1 = b.sizeWidth;
      QDoubleSpinBox* b2 = b.sizeHeight;
      if (idx == 1 && b.lockAspectRatio->isChecked()) {
            // width was changed, fix height
            QSizeF sz   = inspector->element()->getProperty(P_SIZE).toSizeF();
            qreal ratio = sz.width() / sz.height();
            qreal h     = b1->value() / ratio;
            b2->blockSignals(true);
            b2->setValue(h);
            b2->blockSignals(false);
            }
      else if (idx == 2 && b.lockAspectRatio->isChecked()) {
            // height was changed, fix width
            QSizeF sz   = inspector->element()->getProperty(P_SIZE).toSizeF();
            qreal ratio = sz.width() / sz.height();
            qreal w     = b2->value() * ratio;
            b1->blockSignals(true);
            b1->setValue(w);
            b1->blockSignals(false);
            }
      else if (idx == 4) {
            QCheckBox* cb = static_cast<QCheckBox*>(iList[idx].w);
            qreal _spatium = inspector->element()->spatium();
            if (cb->isChecked()) {
                  b1->setSuffix("sp");
                  b2->setSuffix("sp");
                  b1->setValue(b1->value() * DPMM / _spatium);
                  b2->setValue(b2->value() * DPMM / _spatium);
                  }
            else {
                  b1->setSuffix("mm");
                  b2->setSuffix("mm");
                  b1->setValue(b1->value() * _spatium / DPMM);
                  b2->setValue(b2->value() * _spatium / DPMM);
                  }
            }
      InspectorBase::valueChanged(idx);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorImage::setElement(Element* e)
      {
      Image* image = static_cast<Image*>(e);
      QDoubleSpinBox* b1 = static_cast<QDoubleSpinBox*>(iList[1].w);
      QDoubleSpinBox* b2 = static_cast<QDoubleSpinBox*>(iList[2].w);
      if (image->sizeIsSpatium()) {
            b1->setSuffix("sp");
            b2->setSuffix("sp");
            }
      else {
            b1->setSuffix("mm");
            b2->setSuffix("mm");
            }
      InspectorBase::setElement(e);
      }

