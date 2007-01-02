//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: listedit.h,v 1.19 2006/03/30 07:32:34 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __LISTEDIT_H__
#define __LISTEDIT_H__

#include "ui_element.h"
#include "ui_note.h"
#include "ui_page.h"
#include "ui_text.h"
#include "ui_measure.h"
#include "ui_segment.h"
#include "ui_chord.h"
#include "ui_chordrest.h"
#include "ui_hairpin.h"
#include "ui_barline.h"
#include "ui_dynamic.h"
#include "ui_tuplet.h"

class ShowElementBase;
class Element;
class Page;
class DoubleLabel;
class Score;

class ShowNoteWidget;

//---------------------------------------------------------
//   PageListEditor
//---------------------------------------------------------

class PageListEditor : public QWidget {
      Q_OBJECT;

      Score* cs;
      QStackedWidget* stack;
      QTreeWidget* list;
      ShowElementBase* pagePanel;
      ShowElementBase* systemPanel;
      ShowElementBase* measurePanel;
      ShowElementBase* chordPanel;
      ShowElementBase* notePanel;
      ShowElementBase* restPanel;
      ShowElementBase* clefPanel;
      ShowElementBase* timesigPanel;
      ShowElementBase* keysigPanel;
      ShowElementBase* segmentView;
      ShowElementBase* textView;
      ShowElementBase* elementView;
      ShowElementBase* hairpinView;
      ShowElementBase* barLineView;
      ShowElementBase* dynamicView;
      ShowElementBase* tupletView;

      bool searchElement(QTreeWidgetItem* pi, Element* el);

   private slots:
      void itemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
      void itemExpanded(QTreeWidgetItem*);

   public slots:
      void setElement(Element*);

   public:
      PageListEditor(Score*);
	void updateList();
      };

//---------------------------------------------------------
//   MeasureListEditor
//---------------------------------------------------------

class MeasureListEditor : public QWidget {
      Q_OBJECT;

   private slots:
      // void itemChanged(QListViewItem*);

   public:
      MeasureListEditor();
      };

//---------------------------------------------------------
//   ShowElementBase
//---------------------------------------------------------

class ShowElementBase : public QWidget {
      Q_OBJECT;

      Ui::ElementBase eb;
      Element* el;

   private slots:
      void nextClicked();
      void previousClicked();
      void parentClicked();
      void anchorClicked();
      void offsetxChanged(double);
      void offsetyChanged(double);
      void selectedClicked(bool);
      void visibleClicked(bool);

   protected:
      QVBoxLayout* layout;

   signals:
      void elementChanged(Element*);

   public:
      ShowElementBase();
      virtual void setElement(Element*);
      Element* element() const { return el; }
      };

//---------------------------------------------------------
//   ShowPageWidget
//---------------------------------------------------------

class ShowPageWidget : public ShowElementBase {
      Q_OBJECT;

      Ui::PageBase pb;

   private slots:
      void itemClicked(QListWidgetItem*);

   public:
      ShowPageWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowSystemWidget
//---------------------------------------------------------

class ShowSystemWidget : public ShowElementBase {
      Q_OBJECT;

   public:
      ShowSystemWidget();
      };

//---------------------------------------------------------
//   MeasureView
//---------------------------------------------------------

class MeasureView : public ShowElementBase {
      Q_OBJECT;

      Ui::MeasureBase mb;

   public:
      MeasureView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowChordWidget
//---------------------------------------------------------

class ShowChordWidget : public ShowElementBase {
      Q_OBJECT;
      Ui::ChordRestBase crb;
      Ui::ChordBase cb;

   private slots:
      void hookClicked();
      void stemClicked();
      void directionChanged(int);
      void gotoAttribute(QListWidgetItem*);
      void gotoHelpline(QListWidgetItem*);
      void beamClicked();
      void tupletClicked();
      void upChanged(bool);
      void beamModeChanged(int);

   public:
      ShowChordWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   SegmentView
//---------------------------------------------------------

class SegmentView : public ShowElementBase {
      Q_OBJECT;
      Ui::SegmentBase sb;

   public:
      SegmentView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowNoteWidget
//---------------------------------------------------------

class ShowNoteWidget : public ShowElementBase {
      Q_OBJECT;

      Ui::NoteBase nb;

   public:
      ShowNoteWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowRestWidget
//---------------------------------------------------------

class ShowRestWidget : public ShowElementBase {
      Q_OBJECT;

      QSpinBox* segment;

   public:
      ShowRestWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowClefWidget
//---------------------------------------------------------

class ShowClefWidget : public ShowElementBase {
      Q_OBJECT;

      QSpinBox* idx;

   public:
      ShowClefWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowTimesigWidget
//---------------------------------------------------------

class ShowTimesigWidget : public ShowElementBase {
      Q_OBJECT;

   public:
      ShowTimesigWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowKeysigWidget
//---------------------------------------------------------

class ShowKeysigWidget : public ShowElementBase {
      Q_OBJECT;

   public:
      ShowKeysigWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TextView
//---------------------------------------------------------

class TextView : public ShowElementBase {
      Q_OBJECT;

      Ui::TextBase tb;

   public:
      TextView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   HairpinView
//---------------------------------------------------------

class HairpinView : public ShowElementBase {
      Q_OBJECT;

      Ui::HairpinBase hp;

   public:
      HairpinView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ElementView
//---------------------------------------------------------

class ElementView : public ShowElementBase {
      Q_OBJECT;

   public:
      ElementView();
      };

//---------------------------------------------------------
//   BarLineView
//---------------------------------------------------------

class BarLineView : public ShowElementBase {
      Q_OBJECT;

      Ui::BarLineBase bl;

   public:
      BarLineView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   DynamicView
//---------------------------------------------------------

class DynamicView : public ShowElementBase {
      Q_OBJECT;

      Ui::TextBase tb;
      Ui::DynamicBase bl;

   public:
      DynamicView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TupletView
//---------------------------------------------------------

class TupletView : public ShowElementBase {
      Q_OBJECT;

      Ui::TupletBase tb;

   signals:
      void itemClicked(Element*);

   private slots:
      void numberClicked();
      void elementClicked(QListWidgetItem*);

   public:
      TupletView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   DoubleLabel
//---------------------------------------------------------

class DoubleLabel : public QLabel {
      Q_OBJECT;

   public:
      DoubleLabel(QWidget* parent);
      void setValue(double);
      virtual QSize sizeHint() const;
      };

#endif

