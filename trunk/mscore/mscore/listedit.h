//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: listedit.h,v 1.19 2006/03/30 07:32:34 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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
#include "ui_slurtie.h"
#include "ui_slur.h"
#include "ui_line.h"
#include "ui_linesegment.h"
#include "ui_lyrics.h"

class ShowElementBase;
class Element;
class Page;
class DoubleLabel;
class Score;
class BSymbol;
class ElementItem;

class ShowNoteWidget;
// class QSplitter;

//---------------------------------------------------------
//   PageListEditor
//---------------------------------------------------------

class PageListEditor : public QWidget {
      Q_OBJECT;

      QStackedWidget* stack;
      QTreeWidget* list;
      QSplitter* split;

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
      ShowElementBase* slurView;
      ShowElementBase* tieView;
      ShowElementBase* voltaView;
      ShowElementBase* voltaSegmentView;
      ShowElementBase* lyricsView;

      bool searchElement(QTreeWidgetItem* pi, Element* el);
      void addSymbol(ElementItem* parent, BSymbol* bs);

   protected:
      Score* cs;

   private slots:
      void itemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
      void itemExpanded(QTreeWidgetItem*);
      void layoutScore();

   public slots:
      void setElement(Element*);

   public:
      PageListEditor(QWidget* parent = 0);
      void writeSettings();
	void updateList(Score*);
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
      void parentClicked();
      void offsetxChanged(double);
      void offsetyChanged(double);
      void selectedClicked(bool);
      void visibleClicked(bool);
      void gotoElement(QListWidgetItem* ai);

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

   private slots:
      void elementClicked(QTreeWidgetItem* item);

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

   private slots:
      void tieForClicked();
      void tieBackClicked();
      void accidentalClicked();
      void tpcChanged(int);

   signals:
      void scoreChanged();

   public:
      ShowNoteWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowRestWidget
//---------------------------------------------------------

class ShowRestWidget : public ShowElementBase {
      Q_OBJECT;

      Ui::ChordRestBase crb;

      QSpinBox* segment;

   private slots:
      void tupletClicked();
      void beamClicked();

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

   private slots:
      void textChanged();

   signals:
      void scoreChanged();

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
      void scoreChanged();

   private slots:
      void numberClicked();
      void elementClicked(QTreeWidgetItem*);
      void hasNumberToggled(bool);
      void hasLineToggled(bool);

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

//---------------------------------------------------------
//   SlurView
//---------------------------------------------------------

class SlurView : public ShowElementBase {
      Q_OBJECT;

      Ui::SlurTieBase st;
      Ui::SlurBase sb;

   private slots:
      void segmentClicked(QTreeWidgetItem* item);

   public:
      SlurView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TieView
//---------------------------------------------------------

class TieView : public ShowElementBase {
      Q_OBJECT;

      Ui::SlurTieBase st;

   private slots:
      void segmentClicked(QTreeWidgetItem* item);

   public:
      TieView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   VoltaView
//---------------------------------------------------------

class VoltaView : public ShowElementBase {
      Q_OBJECT;

      Ui::SLineBase lb;

   private slots:
      void segmentClicked(QTreeWidgetItem* item);

   public:
      VoltaView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   VoltaSegmentView
//---------------------------------------------------------

class VoltaSegmentView : public ShowElementBase {
      Q_OBJECT;

      Ui::LineSegmentBase lb;

   public:
      VoltaSegmentView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   VoltaSegmentView
//---------------------------------------------------------

class LyricsView : public ShowElementBase {
      Q_OBJECT;

      Ui::LyricsBase lb;

   public:
      LyricsView();
      virtual void setElement(Element*);
      };

#endif

