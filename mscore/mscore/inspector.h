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

#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include "ui_inspector_element.h"
#include "ui_inspector_vbox.h"
#include "ui_inspector_hbox.h"
#include "ui_inspector_beam.h"
#include "ui_inspector_articulation.h"
#include "ui_inspector_spacer.h"
#include "ui_inspector_segment.h"
#include "ui_inspector_note.h"

class Element;
class Note;
class Inspector;
class Segment;

//---------------------------------------------------------
//   InspectorSegment
//---------------------------------------------------------

class InspectorSegment : public QWidget, Ui::InspectorSegment {
      Q_OBJECT
      Segment* segment;

   private slots:
      void resetLeadingSpaceClicked();
      void resetTrailingSpaceClicked();

      void leadingSpaceChanged(double);
      void trailingSpaceChanged(double);

   signals:
      void inspectorVisible(bool);
      void enableApply();

   public:
      InspectorSegment(QWidget* parent = 0);
      void setElement(Segment*);
      void apply();
      bool dirty() const;
      };

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

class InspectorNoteBase : public QWidget, Ui::InspectorNote {
      Q_OBJECT
      Note* note;

   private slots:
      void resetSmallClicked();
      void resetMirrorClicked();
      void resetDotPositionClicked();
      void resetOntimeOffsetClicked();
      void resetOfftimeOffsetClicked();

      void smallChanged(int);
      void mirrorHeadChanged(int);
      void dotPositionChanged(int);
      void ontimeOffsetChanged(int);
      void offtimeOffsetChanged(int);

   signals:
      void enableApply();

   public:
      InspectorNoteBase(QWidget* parent = 0);
      void setElement(Note*);
      void apply();
      bool dirty() const;
      };

//---------------------------------------------------------
//   InspectorElementElement
//---------------------------------------------------------

class InspectorElementElement : public QWidget, Ui::InspectorElement {
      Q_OBJECT

      Element* e;

   private slots:
      void resetColorClicked();
      void resetXClicked();
      void resetYClicked();
      void colorChanged(QColor);
      void offsetXChanged(double);
      void offsetYChanged(double);
      void visibleChanged(int);
      void resetVisibleClicked();

   signals:
      void enableApply();

   public:
      InspectorElementElement(QWidget* parent = 0);
      void setElement(Element*);
      void apply();
      bool dirty() const;
      };

//---------------------------------------------------------
//   InspectorElementBase
//---------------------------------------------------------

class InspectorElementBase : public QWidget {
      Q_OBJECT

   protected:
      QVBoxLayout* layout;
      Inspector* inspector;

   public:
      InspectorElementBase(QWidget* parent);
      virtual void setElement(Element*) = 0;
      virtual void apply() {}
      };

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

class InspectorElement : public InspectorElementBase {
      Q_OBJECT

      InspectorElementElement* ie;

   public:
      InspectorElement(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      };

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

class InspectorVBox : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorVBox vb;

      void block(bool val);

   private slots:
      void resetTopGap();
      void resetBottomGap();
      void resetLeftMargin();
      void resetRightMargin();
      void resetTopMargin();
      void resetBottomMargin();

   public:
      InspectorVBox(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      };

//---------------------------------------------------------
//   InspectorHBox
//---------------------------------------------------------

class InspectorHBox : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorHBox hb;

   private slots:
      void resetLeftGap();
      void resetRightGap();

   public:
      InspectorHBox(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      };

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

class InspectorArticulation : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorArticulation ar;

   public:
      InspectorArticulation(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      };

//---------------------------------------------------------
//   InspectorSpacer
//---------------------------------------------------------

class InspectorSpacer : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSpacer sp;

   public:
      InspectorSpacer(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      };

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

class InspectorNote : public InspectorElementBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      InspectorNoteBase* iNote;
      InspectorSegment* iSegment;
      QToolButton* dot1;
      QToolButton* dot2;
      QToolButton* dot3;

      bool dirty() const;

   private slots:
      void checkDirty();
      void dot1Clicked();
      void dot2Clicked();
      void dot3Clicked();

   public:
      InspectorNote(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      };

//---------------------------------------------------------
//   InspectorRest
//---------------------------------------------------------

class InspectorRest : public InspectorElementBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      InspectorSegment* iSegment;
      QCheckBox* small;

   public:
      InspectorRest(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      bool dirty() const;
      };

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

class InspectorClef : public InspectorElementBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      InspectorSegment* iSegment;

   public:
      InspectorClef(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      };

//---------------------------------------------------------
//   InspectorBeam
//---------------------------------------------------------

class InspectorBeam : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorBeam b;

      bool dirty() const;

   private slots:
      void resetDistributeClicked();
      void resetDirectionClicked();
      void directionActivated(int idx);
      void distributeToggled(bool val);

   public:
      InspectorBeam(QWidget* parent);
      virtual void setElement(Element*);
      virtual void apply();
      };

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

class Inspector : public QDockWidget {
      Q_OBJECT

      QVBoxLayout* layout;
      InspectorElementBase* ie;
      QPushButton* apply;
      Element* _element;

      virtual void closeEvent(QCloseEvent*);

   private slots:
      void applyClicked();

   signals:
      void inspectorVisible(bool);

   public slots:
      void enableApply(bool val = true) { apply->setEnabled(val); }
      void reset();

   public:
      Inspector(QWidget* parent = 0);
      void setElement(Element*);
      Element* element() const { return _element; }
      };

#endif

