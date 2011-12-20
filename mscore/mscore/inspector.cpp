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
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/segment.h"
#include "libmscore/rest.h"
#include "libmscore/beam.h"
#include "libmscore/clef.h"

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
                  case VBOX:         ie = new InspectorVBox(this); break;
                  case HBOX:         ie = new InspectorHBox(this); break;
                  case ARTICULATION: ie = new InspectorArticulation(this); break;
                  case SPACER:       ie = new InspectorSpacer(this); break;
                  case NOTE:         ie = new InspectorNote(this); break;
                  case REST:         ie = new InspectorRest(this); break;
                  case CLEF:         ie = new InspectorClef(this); break;
                  case BEAM:         ie = new InspectorBeam(this); break;
                  default:           ie = new InspectorElement(this); break;
                  }
            layout->insertWidget(0, ie);
            }
      _element = e;
      ie->setElement(_element);
      apply->setEnabled(false);
      }

//---------------------------------------------------------
//   InspectorElementElement
//---------------------------------------------------------

InspectorElementElement::InspectorElementElement(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      connect(offsetX, SIGNAL(valueChanged(double)), SIGNAL(enableApply()));
      connect(offsetY, SIGNAL(valueChanged(double)), SIGNAL(enableApply()));
      connect(resetX,  SIGNAL(clicked()), SLOT(resetXClicked()));
      connect(resetY,  SIGNAL(clicked()), SLOT(resetYClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorElementElement::setElement(const Element* e)
      {
      elementName->setText(e->name());
      qreal _spatium = e->score()->spatium();
      offsetX->setValue(e->pos().x() / _spatium);
      offsetY->setValue(e->pos().y() / _spatium);
      }

//---------------------------------------------------------
//   resetXClicked
//---------------------------------------------------------

void InspectorElementElement::resetXClicked()
      {
      offsetX->setValue(0.0);
      }

//---------------------------------------------------------
//   resetTrailingSpace
//---------------------------------------------------------

void InspectorElementElement::resetYClicked()
      {
      offsetY->setValue(0.0);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorElementElement::apply(Element* e)
      {
      Score* score    = e->score();
      qreal _spatium  = score->spatium();
      QPointF o(offsetX->value() * _spatium, offsetY->value() * _spatium);
      if (o != e->pos())
            score->undo()->push(new ChangeUserOffset(e, o - e->ipos()));
      }

//---------------------------------------------------------
//   InspectorElementBase
//---------------------------------------------------------

InspectorElementBase::InspectorElementBase(QWidget* parent)
   : QWidget(parent)
      {
      inspector = static_cast<Inspector*>(parent);
      layout    = new QVBoxLayout;
      setLayout(layout);
      }

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

InspectorElement::InspectorElement(QWidget* parent)
   : InspectorElementBase(parent)
      {
      ie = new InspectorElementElement(this);
      layout->addWidget(ie);
      connect(ie, SIGNAL(enableApply()), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorElement::setElement(Element* e)
      {
      ie->setElement(e);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorElement::apply()
      {
      Element* e = inspector->element();
      Score* score = e->score();
      score->startCmd();
      ie->apply(e);
      score->setLayoutAll(true);
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

InspectorVBox::InspectorVBox(QWidget* parent)
   : InspectorElementBase(parent)
      {
      QWidget* w = new QWidget;

      vb.setupUi(w);
      layout->addWidget(w);
      connect(vb.topGap,    SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(vb.bottomGap, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(vb.height,    SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(vb.resetTopGap, SIGNAL(clicked()), SLOT(resetTopGap()));
      connect(vb.resetBottomGap, SIGNAL(clicked()), SLOT(resetBottomGap()));
      }

//---------------------------------------------------------
//   resetTopGap
//---------------------------------------------------------

void InspectorVBox::resetTopGap()
      {
      vb.topGap->setValue(0.0);
      }

//---------------------------------------------------------
//   resetBottomGap
//---------------------------------------------------------

void InspectorVBox::resetBottomGap()
      {
      vb.bottomGap->setValue(0.0);
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

InspectorHBox::InspectorHBox(QWidget* parent)
   : InspectorElementBase(parent)
      {
      QWidget* w = new QWidget;

      hb.setupUi(w);
      layout->addWidget(w);
      connect(hb.leftGap,  SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(hb.rightGap, SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(hb.width,    SIGNAL(valueChanged(double)), inspector, SLOT(enableApply()));
      connect(hb.resetLeftGap,  SIGNAL(clicked()), SLOT(resetLeftGap()));
      connect(hb.resetRightGap, SIGNAL(clicked()), SLOT(resetRightGap()));
      }

//---------------------------------------------------------
//   resetLeftGap
//---------------------------------------------------------

void InspectorHBox::resetLeftGap()
      {
      hb.leftGap->setValue(0.0);
      }

//---------------------------------------------------------
//   resetRightGap
//---------------------------------------------------------

void InspectorHBox::resetRightGap()
      {
      hb.rightGap->setValue(0.0);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorHBox::setElement(Element* e)
      {
      HBox* box = static_cast<HBox*>(e);
      qreal _spatium = e->score()->spatium();
      hb.elementName->setText(e->name());

      hb.leftGap->blockSignals(true);
      hb.rightGap->blockSignals(true);
      hb.width->blockSignals(true);

      hb.leftGap->setValue(box->topGap() / _spatium);
      hb.rightGap->setValue(box->bottomGap() / _spatium);
      hb.width->setValue(box->boxHeight().val());

      hb.leftGap->blockSignals(false);
      hb.rightGap->blockSignals(false);
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
      qreal leftGap   = hb.leftGap->value() * _spatium;
      qreal rightGap  = hb.rightGap->value() * _spatium;
      Spatium width(hb.width->value());

      if (leftGap != box->topGap() || rightGap != box->bottomGap()
         || width != box->boxWidth()) {
            score->startCmd();
            score->undo()->push(new ChangeBoxProperties(box,
               box->leftMargin(), box->topMargin(), box->rightMargin(), box->bottomMargin(),
               box->boxHeight(), width,
               leftGap, rightGap
               ));
            score->setLayoutAll(true);
            score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

InspectorArticulation::InspectorArticulation(QWidget* parent)
   : InspectorElementBase(parent)
      {
      QWidget* w = new QWidget;

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

InspectorSpacer::InspectorSpacer(QWidget* parent)
   : InspectorElementBase(parent)
      {
      QWidget* w = new QWidget;

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

//---------------------------------------------------------
//   InspectorSegment
//---------------------------------------------------------

InspectorSegment::InspectorSegment(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      connect(leadingSpace,       SIGNAL(valueChanged(double)), SIGNAL(enableApply()));
      connect(trailingSpace,      SIGNAL(valueChanged(double)), SIGNAL(enableApply()));
      connect(resetLeadingSpace,  SIGNAL(clicked()), SLOT(resetLeadingSpaceClicked()));
      connect(resetTrailingSpace, SIGNAL(clicked()), SLOT(resetTrailingSpaceClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorSegment::setElement(const Element* e)
      {
      const Segment* segment = static_cast<const Segment*>(e);
      leadingSpace->setValue(segment->extraLeadingSpace().val());
      trailingSpace->setValue(segment->extraTrailingSpace().val());
      }

//---------------------------------------------------------
//   resetLeadingSpace
//---------------------------------------------------------

void InspectorSegment::resetLeadingSpaceClicked()
      {
      leadingSpace->setValue(0.0);
      }

//---------------------------------------------------------
//   resetTrailingSpace
//---------------------------------------------------------

void InspectorSegment::resetTrailingSpaceClicked()
      {
      trailingSpace->setValue(0.0);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorSegment::apply(Element* e)
      {
      Segment* segment = static_cast<Segment*>(e);
      qreal val = leadingSpace->value();
      if (segment->extraLeadingSpace().val() != val)
            e->score()->undo()->push(new ChangeProperty(segment, P_LEADING_SPACE, val));
      val = trailingSpace->value();
      if (segment->extraTrailingSpace().val() != val)
            e->score()->undo()->push(new ChangeProperty(segment, P_TRAILING_SPACE, val));
      }

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

InspectorNote::InspectorNote(QWidget* parent)
   : InspectorElementBase(parent)
      {
      iElement = new InspectorElementElement(this);
      iSegment = new InspectorSegment(this);
      layout->addWidget(iElement);
      layout->addWidget(iSegment);
      connect(iElement, SIGNAL(enableApply()), inspector, SLOT(enableApply()));
      connect(iSegment, SIGNAL(enableApply()), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorNote::setElement(Element* e)
      {
      Note* note = static_cast<Note*>(e);
      Segment* segment = note->chord()->segment();

      iElement->setElement(e);
      iSegment->setElement(segment);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorNote::apply()
      {
      Note* note       = static_cast<Note*>(inspector->element());
      Segment* segment = note->chord()->segment();
      Score*  score    = note->score();
      score->startCmd();

      iElement->apply(note);
      iSegment->apply(segment);

      score->setLayoutAll(true);
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   InspectorRest
//---------------------------------------------------------

InspectorRest::InspectorRest(QWidget* parent)
   : InspectorElementBase(parent)
      {
      iElement = new InspectorElementElement(this);
      iSegment = new InspectorSegment(this);

      layout->addWidget(iElement);
      layout->addWidget(iSegment);
      connect(iElement, SIGNAL(enableApply()), inspector, SLOT(enableApply()));
      connect(iSegment, SIGNAL(enableApply()), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorRest::setElement(Element* e)
      {
      Rest* rest = static_cast<Rest*>(e);
      Segment* segment = rest->segment();

      iElement->setElement(rest);
      iSegment->setElement(segment);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorRest::apply()
      {
      Rest* rest       = static_cast<Rest*>(inspector->element());
      Segment* segment = rest->segment();
      Score* score     = rest->score();
      score->startCmd();

      iElement->apply(rest);
      iSegment->apply(segment);

      score->setLayoutAll(true);
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

InspectorClef::InspectorClef(QWidget* parent)
   : InspectorElementBase(parent)
      {
      iElement = new InspectorElementElement(this);
      iSegment = new InspectorSegment(this);

      layout->addWidget(iElement);
      layout->addWidget(iSegment);
      connect(iElement, SIGNAL(enableApply()), inspector, SLOT(enableApply()));
      connect(iSegment, SIGNAL(enableApply()), inspector, SLOT(enableApply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorClef::setElement(Element* e)
      {
      Clef* clef = static_cast<Clef*>(e);
      Segment* segment = clef->segment();

      iElement->setElement(clef);
      iSegment->setElement(segment);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorClef::apply()
      {
      Clef* clef       = static_cast<Clef*>(inspector->element());
      Segment* segment = clef->segment();
      Score* score     = clef->score();
      score->startCmd();

      iElement->apply(clef);
      iSegment->apply(segment);

      score->setLayoutAll(true);
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   InspectorBeam
//---------------------------------------------------------

InspectorBeam::InspectorBeam(QWidget* parent)
   : InspectorElementBase(parent)
      {
      QWidget* w = new QWidget;

      b.setupUi(w);
      layout->addWidget(w);
      connect(b.distribute, SIGNAL(toggled(bool)),  SLOT(distributeToggled(bool)));
      connect(b.direction,  SIGNAL(activated(int)), SLOT(directionActivated(int)));
      connect(b.resetDistribute, SIGNAL(clicked()), SLOT(resetDistributeClicked()));
      connect(b.resetDirection,  SIGNAL(clicked()), SLOT(resetDirectionClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBeam::setElement(Element* e)
      {
      Beam* beam = static_cast<Beam*>(e);

      b.distribute->blockSignals(true);
      b.distribute->setChecked(beam->distribute());
      b.distribute->blockSignals(false);
      b.direction->setCurrentIndex(beam->beamDirection());
      b.resetDirection->setEnabled(beam->beamDirection() != AUTO);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorBeam::apply()
      {
      Beam* beam   = static_cast<Beam*>(inspector->element());
      Score* score = beam->score();

      bool distribute = b.distribute->isChecked();
      Direction d     = Direction(b.direction->currentIndex());
      score->startCmd();
      if (beam->distribute() != distribute)
            score->undo()->push(new ChangeProperty(beam, P_DISTRIBUTE, distribute));
      if (beam->beamDirection() != d)
            score->undo()->push(new ChangeProperty(beam, P_DIRECTION, d));
      score->setLayoutAll(true);
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   resetDistributeClicked
//---------------------------------------------------------

void InspectorBeam::resetDistributeClicked()
      {
      b.distribute->setChecked(false);
      }

//---------------------------------------------------------
//   resetDirectionClicked
//---------------------------------------------------------

void InspectorBeam::resetDirectionClicked()
      {
      b.direction->setCurrentIndex(AUTO);
      }

//---------------------------------------------------------
//   directionActivated
//---------------------------------------------------------

void InspectorBeam::directionActivated(int idx)
      {
      b.resetDirection->setEnabled(Direction(idx) != AUTO);
      inspector->enableApply(dirty());
      }

//---------------------------------------------------------
//   distributeToggled
//---------------------------------------------------------

void InspectorBeam::distributeToggled(bool val)
      {
      b.resetDistribute->setEnabled(val);
      inspector->enableApply(dirty());
      }

//---------------------------------------------------------
//   dirty
//    return true if a property has changed
//---------------------------------------------------------

bool InspectorBeam::dirty() const
      {
      const Beam* beam = static_cast<Beam*>(inspector->element());
      bool distribute  = b.distribute->isChecked();
      Direction d      = Direction(b.direction->currentIndex());
      return (beam->distribute() != distribute)
             || (beam->beamDirection() != d);
      }

