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

#include "libmscore/score.h"
#include "libmscore/element.h"
#include "musescore.h"
#include "inspectorBase.h"
#include "inspector.h"

//---------------------------------------------------------
//   InspectorBase
//---------------------------------------------------------

InspectorBase::InspectorBase(QWidget* parent)
   : QWidget(parent)
      {
      resetMapper = new QSignalMapper(this);
      valueMapper = new QSignalMapper(this);

      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      inspector = static_cast<Inspector*>(parent);
      layout    = new QVBoxLayout;
      layout->setSizeConstraint(QLayout::SetNoConstraint);
      setLayout(layout);
      }

//---------------------------------------------------------
//   getValue
//---------------------------------------------------------

QVariant InspectorBase::getValue(int idx, qreal _spatium) const
      {
      const InspectorItem& ii = item(idx);
      QWidget* w        = ii.w;

      switch (propertyType(ii.t)) {
            case T_SREAL:     return w->property("value").toDouble() * _spatium;
            case T_REAL:      return w->property("value");
            case T_DIRECTION: return w->property("currentIndex");
            case T_BOOL:      return w->property("checked");
            default:          abort();
            }
      return QVariant();
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void InspectorBase::setValue(int idx, const QVariant& val, qreal _spatium)
      {
      const InspectorItem& ii = item(idx);
      QWidget* w        = ii.w;

      switch (propertyType(ii.t)) {
            case T_SREAL:
                  static_cast<QDoubleSpinBox*>(w)->setValue(val.toDouble() / _spatium);
                  break;
            case T_REAL:
                  static_cast<QDoubleSpinBox*>(w)->setValue(val.toDouble());
                  break;
            case T_DIRECTION:
                  static_cast<QComboBox*>(w)->setCurrentIndex(val.toInt());
                  break;
            case T_BOOL:
                  static_cast<QCheckBox*>(w)->setChecked(val.toBool());
                  break;
            default:
                  abort();
            }
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool InspectorBase::isDefault(int idx)
      {
      Element* e = inspector->element();
      const InspectorItem& ii = item(idx);

      P_ID id      = ii.t;
      QVariant val = getValue(idx, e->spatium());
      void* def    = e->propertyDefault(id);

      switch (propertyType(id)) {
            case T_SREAL:
            case T_REAL:      return val.toDouble() == *(qreal*)def;
            case T_DIRECTION: return val.toInt() == *(int*)def;
            case T_BOOL:      return val.toBool() == *(bool*)def;
            default:          abort();
            }
      }

//---------------------------------------------------------
//   dirty
//    return true if a property has changed
//---------------------------------------------------------

bool InspectorBase::dirty() const
      {
      Element* e = inspector->element();
      for (int i = 0; i < inspectorItems(); ++i) {
            if (e->getProperty(item(i).t) != getValue(i, e->spatium()))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBase::valueChanged(int idx)
      {
      if (item(idx).r)
            item(idx).r->setEnabled(!isDefault(idx));
      inspector->enableApply(dirty());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBase::setElement(Element* e)
      {
      for (int i = 0; i < inspectorItems(); ++i) {
            QWidget*     w = item(i).w;
            QToolButton* r = item(i).r;
            P_ID id        = item(i).t;
            QVariant val   = e->getProperty(id);

            w->blockSignals(true);
            setValue(i, val, e->spatium());
            w->blockSignals(false);

            if (r)
                  r->setEnabled(!isDefault(i));
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorBase::apply()
      {
      Element* e   = inspector->element();
      Score* score = e->score();

      score->startCmd();
      for (int i = 0; i < inspectorItems(); ++i) {
            QVariant val1 = e->getProperty(item(i).t);
            QVariant val2 = getValue(i, e->spatium());
            if (val1 != val2)
                  score->undoChangeProperty(e, item(i).t, val2);
            }
      score->setLayoutAll(true);
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   resetClicked
//---------------------------------------------------------

void InspectorBase::resetClicked(int i)
      {
      Element* e = inspector->element();
      P_ID id    = item(i).t;
      void* def  = e->propertyDefault(id);
      QWidget* w = item(i).w;

       switch (propertyType(id)) {
            case T_SREAL:
            case T_REAL:
                  static_cast<QDoubleSpinBox*>(w)->setValue(*(qreal*)def);
                  break;
            case T_DIRECTION:
                  static_cast<QComboBox*>(w)->setCurrentIndex(*(int*)def);
                  break;
            case T_BOOL:
                  static_cast<QCheckBox*>(w)->setChecked(*(bool*)def);
                  break;
            default:
                  abort();
            }
      }

//---------------------------------------------------------
//   mapSignals
//---------------------------------------------------------

void InspectorBase::mapSignals()
      {
      for (int i = 0; i < inspectorItems(); ++i) {
            QToolButton* b = item(i).r;
            if (b) {
                  connect(b, SIGNAL(clicked()), resetMapper, SLOT(map()));
                  resetMapper->setMapping(b, i);
                  }
            QWidget* w = item(i).w;
            valueMapper->setMapping(w, i);
            switch (propertyType(item(i).t)) {
                  case T_REAL:
                  case T_SREAL:
                        connect(w, SIGNAL(valueChanged(double)), valueMapper, SLOT(map()));
                        break;
                  case T_DIRECTION:
                        connect(w, SIGNAL(currentIndexChanged(int)), valueMapper, SLOT(map()));
                        break;
                  case T_BOOL:
                        connect(w, SIGNAL(toggled(bool)), valueMapper, SLOT(map()));
                        break;
                  default:
                        abort();
                  }
            }
      connect(resetMapper, SIGNAL(mapped(int)), SLOT(resetClicked(int)));
      connect(valueMapper, SIGNAL(mapped(int)), SLOT(valueChanged(int)));
      }
//
//  dummy
//
const InspectorItem& InspectorBase::item(int idx) const
      {
      static InspectorItem item;
      return item;
      }

