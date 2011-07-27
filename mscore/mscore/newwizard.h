//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __NEWWIZARD_H__
#define __NEWWIZARD_H__

#include "ui_instrwizard.h"
#include "ui_timesigwizard.h"
#include "ui_newwizard.h"

#include "key.h"

class Score;
class Palette;

//---------------------------------------------------------
//   InstrumentWizard
//---------------------------------------------------------

class InstrumentWizard : public QWidget, private Ui::InstrumentWizard {
      Q_OBJECT

   private slots:
      void on_addButton_clicked();
      void on_partiturList_itemSelectionChanged();
      void on_instrumentList_itemSelectionChanged();
      void on_instrumentList_itemActivated(QTreeWidgetItem* item, int);
      void on_removeButton_clicked();
      void on_upButton_clicked();
      void on_downButton_clicked();
      void on_aboveButton_clicked();
      void on_belowButton_clicked();
      void buildTemplateList();

   signals:
      void completeChanged(bool);

   public:
      InstrumentWizard(QWidget* parent = 0);
      void createInstruments(Score*);
      void init();
      };

//---------------------------------------------------------
//   TimesigWizard
//---------------------------------------------------------

class TimesigWizard : public QWidget, private Ui::TimesigWizard {
      Q_OBJECT

   public:
      TimesigWizard(QWidget* parent = 0);
      int measures() const;
      void timesig(int* z, int* n) const;
      bool pickup(int* z, int* n) const;
      };

//---------------------------------------------------------
//   TitleWizard
//---------------------------------------------------------

class TitleWizard : public QWidget, public Ui::NewWizard {
      Q_OBJECT

   public:
      TitleWizard(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   NewWizardPage1
//---------------------------------------------------------

class NewWizardPage1 : public QWizardPage {
      Q_OBJECT

      TitleWizard* w;

   public:
      NewWizardPage1(QWidget* parent = 0);
      QString title() const              { return w->title->text();      }
      QString subtitle() const           { return w->subtitle->text();   }
      QString composer() const           { return w->composer->text();   }
      QString poet() const               { return w->poet->text();       }
      QString copyright() const          { return w->copyright->text();  }
      virtual void initializePage();
      };

//---------------------------------------------------------
//   NewWizardPage2
//---------------------------------------------------------

class NewWizardPage2 : public QWizardPage {
      Q_OBJECT

      bool complete;
      InstrumentWizard* w;

   public slots:
      void setComplete(bool);

   public:
      NewWizardPage2(QWidget* parent = 0);
      virtual bool isComplete() const  { return complete; }
      void createInstruments(Score* s) { w->createInstruments(s); }
      virtual void initializePage();
      };

//---------------------------------------------------------
//   NewWizardPage3
//---------------------------------------------------------

class NewWizardPage3 : public QWizardPage {
      Q_OBJECT

      TimesigWizard* w;

   public:
      NewWizardPage3(QWidget* parent = 0);
      int measures() const                     { return w->measures();          }
      void timesig(int* z, int* n) const       { w->timesig(z, n);              }
      bool pickupMeasure(int* z, int* n) const { return w->pickup(z, n); }
      };

//---------------------------------------------------------
//   NewWizardPage4
//---------------------------------------------------------

class NewWizardPage4 : public QWizardPage {
      Q_OBJECT

      QDirModel* model;
      QTreeView* tree;

   private slots:
      void templateChanged(const QItemSelection&, const QItemSelection&);

   public:
      NewWizardPage4(QWidget* parent = 0);
      virtual bool isComplete() const;
      QString templatePath() const;
      virtual void initializePage();
      };

//---------------------------------------------------------
//   NewWizardPage5
//---------------------------------------------------------

class NewWizardPage5 : public QWizardPage {
      Q_OBJECT

      Palette* sp;

   public:
      NewWizardPage5(QWidget* parent = 0);
      virtual bool isComplete() const { return true; }
      KeySigEvent keysig() const;
      void init();
      };

//---------------------------------------------------------
//   NewWizard
//---------------------------------------------------------

class NewWizard : public QWizard {
      Q_OBJECT

      NewWizardPage1* p1;
      NewWizardPage2* p2;
      NewWizardPage3* p3;
      NewWizardPage4* p4;
      NewWizardPage5* p5;

   public:
      NewWizard(QWidget* parent = 0);
      friend class QWizardPage;
      virtual int nextId() const;

      enum { Page_Type, Page_Instruments, Page_Template, Page_Keysig, Page_Timesig};

      QString templatePath() const       { return p4->templatePath(); }
      bool useTemplate() const;
      int measures() const               { return p3->measures();    }
      void timesig(int* z, int* n) const { p3->timesig(z, n);        }
      void createInstruments(Score* s)   { p2->createInstruments(s); }
      QString title() const              { return p1->title();       }
      QString subtitle() const           { return p1->subtitle();    }
      QString composer() const           { return p1->composer();    }
      QString poet() const               { return p1->poet();        }
      QString copyright() const          { return p1->copyright();   }
      KeySigEvent keysig() const                 { return p5->keysig();      }
      bool pickupMeasure(int* z, int* n) const { return p3->pickupMeasure(z, n); }
      };

#endif

