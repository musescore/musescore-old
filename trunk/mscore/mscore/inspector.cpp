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
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "musescore.h"
#include "libmscore/element.h"
#include "libmscore/score.h"
#include "libmscore/box.h"
#include "libmscore/undo.h"
#include "libmscore/spacer.h"

//---------------------------------------------------------
//   showInspector
//---------------------------------------------------------

void MuseScore::showInspector(bool visible)
      {
      QAction* a = getAction("inspector");
      if (visible) {
            if (!inspector) {
                  inspector = new Inspector();
                  QAction* a = getAction("toggle-palette");
                  connect(inspector, SIGNAL(inspectorVisible(bool)), a, SLOT(setChecked(bool)));
                  addDockWidget(Qt::RightDockWidgetArea, inspector);
                  }
            if (cs && cs->selection().element())
                  inspector->setElement(cs->selection().element());
            }
      if (inspector)
            inspector->setVisible(visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

Inspector::Inspector(QWidget* parent)
   : QDockWidget(tr("Inspector"), parent)
      {
      setObjectName("inspector");
      setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      QWidget* mainWidget = new QWidget;
      mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); //??
      setWidget(mainWidget);
      layout = new QVBoxLayout;
      mainWidget->setLayout(layout);
      ie        = 0;
      _element  = 0;
      QHBoxLayout* hbox = new QHBoxLayout;
      apply = new QPushButton;
      apply->setText(tr("Apply"));
      apply->setEnabled(false);
      hbox->addWidget(apply);
      layout->addStretch(10);
      layout->addLayout(hbox);
      connect(apply, SIGNAL(clicked()), SLOT(applyClicked()));
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Inspector::reset()
      {
      if (ie)
            ie->setElement(_element);
      apply->setEnabled(false);
      }

//---------------------------------------------------------
//   applyClicked
//---------------------------------------------------------

void Inspector::applyClicked()
      {
      if (ie)
            ie->apply();
      apply->setEnabled(false);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Inspector::setElement(Element* e)
      {
      if (e == 0 || _element == 0 || (e->type() != _element->type())) {
            if (ie)
                  delete ie;
            ie = 0;
            _element = e;
            apply->setEnabled(_element != 0);

            if (_element == 0)
                  return;
            switch(_element->type()) {
                  case FBOX:
                  case TBOX:
                  case VBOX:         ie = new InspectorVBox(this, this); break;
                  case HBOX:         ie = new InspectorHBox(this, this); break;
                  case ARTICULATION: ie = new InspectorArticulation(this, this); break;
                  case SPACER:       ie = new InspectorSpacer(this, this); break;
                  default:           ie = new InspectorElement(this, this); break;
                  }
            layout->insertWidget(0, ie);
            }
      _element = e;
      ie->setElement(_element);
      }

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

InspectorElement::InspectorElement(Inspector* i, QWidget* parent)
   : InspectorElementBase(i, parent)
      {
      QWidget* w = new QWidget;
      layout = new QVBoxLayout;
      setLayout(layout);

      ie.setupUi(w);
      layout->addWidget(w);
      connect(ie.x, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(ie.y, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorElement::setElement(Element* e)
      {
      qreal _spatium = e->score()->spatium();
      ie.elementName->setText(e->name());
      ie.x->setValue(e->pos().x() / _spatium);
      ie.y->setValue(e->pos().y() / _spatium);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorElement::apply()
      {
      Element* e      = inspector->element();
      Score* score    = e->score();
      qreal _spatium  = score->spatium();
      QPointF o(ie.x->value() * _spatium, ie.y->value() * _spatium);
      if (o != e->pos()) {
            score->startCmd();
            score->undo()->push(new ChangeUserOffset(e, o - e->ipos()));
            score->setLayoutAll(true);
            score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

InspectorVBox::InspectorVBox(Inspector* i, QWidget* parent)
   : InspectorElementBase(i, parent)
      {
      QWidget* w = new QWidget;
      layout = new QVBoxLayout;
      setLayout(layout);

      vb.setupUi(w);
      layout->addWidget(w);
      connect(vb.topGap,    SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(vb.bottomGap, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(vb.height,    SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorVBox::setElement(Element* e)
      {
      VBox* box = static_cast<VBox*>(e);
      qreal _spatium = e->score()->spatium();
      vb.elementName->setText(e->name());
      vb.topGap->blockSignals(true);
      vb.bottomGap->blockSignals(true);
      vb.height->blockSignals(true);
      vb.topGap->setValue(box->topGap() / _spatium);
      vb.bottomGap->setValue(box->bottomGap() / _spatium);
      vb.height->setValue(box->boxHeight().val());
      vb.topGap->blockSignals(false);
      vb.bottomGap->blockSignals(false);
      vb.height->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorVBox::apply()
      {
      VBox* box       = static_cast<VBox*>(inspector->element());
      Score* score    = box->score();
      qreal _spatium  = score->spatium();
      qreal topGap    = vb.topGap->value() * _spatium;
      qreal bottomGap = vb.bottomGap->value() * _spatium;
      Spatium height(vb.height->value());

      if (topGap != box->topGap() || bottomGap != box->bottomGap()
         || height != box->boxHeight()) {
            score->startCmd();
            score->undo()->push(new ChangeBoxProperties(box,
               box->leftMargin(), box->topMargin(), box->rightMargin(), box->bottomMargin(),
               height, box->boxWidth(),
               topGap, bottomGap
               ));
            score->setLayoutAll(true);
            score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   InspectorHBox
//---------------------------------------------------------

InspectorHBox::InspectorHBox(Inspector* i, QWidget* parent)
   : InspectorElementBase(i, parent)
      {
      QWidget* w = new QWidget;
      layout = new QVBoxLayout;
      setLayout(layout);

      hb.setupUi(w);
      layout->addWidget(w);
      connect(hb.topGap,    SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(hb.bottomGap, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(hb.width,     SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorHBox::setElement(Element* e)
      {
      HBox* box = static_cast<HBox*>(e);
      qreal _spatium = e->score()->spatium();
      hb.elementName->setText(e->name());

      hb.topGap->blockSignals(true);
      hb.bottomGap->blockSignals(true);
      hb.width->blockSignals(true);

      hb.topGap->setValue(box->topGap() / _spatium);
      hb.bottomGap->setValue(box->bottomGap() / _spatium);
      hb.width->setValue(box->boxHeight().val());

      hb.topGap->blockSignals(false);
      hb.bottomGap->blockSignals(false);
      hb.width->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorHBox::apply()
      {
      HBox* box       = static_cast<HBox*>(inspector->element());
      Score* score    = box->score();
      qreal _spatium  = score->spatium();
      qreal topGap    = hb.topGap->value() * _spatium;
      qreal bottomGap = hb.bottomGap->value() * _spatium;
      Spatium width(hb.width->value());

      if (topGap != box->topGap() || bottomGap != box->bottomGap()
         || width != box->boxWidth()) {
            score->startCmd();
            score->undo()->push(new ChangeBoxProperties(box,
               box->leftMargin(), box->topMargin(), box->rightMargin(), box->bottomMargin(),
               box->boxHeight(), width,
               topGap, bottomGap
               ));
            score->setLayoutAll(true);
            score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

InspectorArticulation::InspectorArticulation(Inspector* i, QWidget* parent)
   : InspectorElementBase(i, parent)
      {
      QWidget* w = new QWidget;
      layout = new QVBoxLayout;
      setLayout(layout);

      ar.setupUi(w);
      layout->addWidget(w);
      connect(ar.x, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(ar.y, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(ar.direction, SIGNAL(currentIndexChanged(int)), inspector, SLOT(enableApply()));
      connect(ar.anchor, SIGNAL(currentIndexChanged(int)), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorArticulation::setElement(Element* e)
      {
      Articulation* a = static_cast<Articulation*>(e);
      qreal _spatium = e->score()->spatium();
      ar.elementName->setText(e->name());
      ar.x->blockSignals(true);
      ar.y->blockSignals(true);
      ar.direction->blockSignals(true);
      ar.anchor->blockSignals(true);

      ar.x->setValue(a->pos().x() / _spatium);
      ar.y->setValue(a->pos().y() / _spatium);
      ar.direction->setCurrentIndex(int(a->direction()));
      ar.anchor->setCurrentIndex(int(a->anchor()));

      ar.x->blockSignals(false);
      ar.y->blockSignals(false);
      ar.direction->blockSignals(false);
      ar.anchor->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorArticulation::apply()
      {
      Articulation* a = static_cast<Articulation*>(inspector->element());
      Score* score    = a->score();
      qreal _spatium  = score->spatium();

      QPointF o(ar.x->value() * _spatium, ar.y->value() * _spatium);
      score->startCmd();
      if (o != a->pos())
            score->undo()->push(new ChangeUserOffset(a, o - a->ipos()));
      Direction d = Direction(ar.direction->currentIndex());
      ArticulationAnchor anchor = ArticulationAnchor(ar.anchor->currentIndex());
      if (anchor != a->anchor())
            score->undo()->push(new ChangeProperty(a, P_ARTICULATION_ANCHOR, int(anchor)));
      if (d != a->direction())
            score->undo()->push(new ChangeProperty(a, P_DIRECTION, int(d)));
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

InspectorSpacer::InspectorSpacer(Inspector* i, QWidget* parent)
   : InspectorElementBase(i, parent)
      {
      QWidget* w = new QWidget;
      layout = new QVBoxLayout;
      setLayout(layout);

      sp.setupUi(w);
      layout->addWidget(w);
      connect(sp.height, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorSpacer::setElement(Element* e)
      {
      Spacer* spacer = static_cast<Spacer*>(e);
      sp.elementName->setText(e->name());
      sp.height->setValue(spacer->gap() / spacer->spatium());
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorSpacer::apply()
      {
      Spacer* spacer = static_cast<Spacer*>(inspector->element());
      Score* score   = spacer->score();
      qreal space    = sp.height->value() * spacer->spatium();
      if (space != spacer->gap()) {
            score->startCmd();
            //TODO make undoable
            spacer->setGap(space);
            score->setLayoutAll(true);
            score->setDirty(true);

            score->endCmd();
            mscore->endCmd();
            }
      }
