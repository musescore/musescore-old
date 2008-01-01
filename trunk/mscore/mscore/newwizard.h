//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: part.cpp,v 1.14 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2008 Werner Schweer and others
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


//---------------------------------------------------------
//   NewWizardPage1
//---------------------------------------------------------

class NewWizardPage1 : public QWizardPage {
      Q_OBJECT

      QRadioButton* rb1;
      QRadioButton* rb2;

   private slots:
      void rb1Toggled(bool);

   public:
      NewWizardPage1(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   NewWizardPage4
//---------------------------------------------------------

class NewWizardPage4 : public QWizardPage {
      Q_OBJECT

      QDirModel* model;
      QTreeView* tree;

   private slots:

   public:
      NewWizardPage4(QWidget* parent = 0);
      };


//---------------------------------------------------------
//   NewWizard
//---------------------------------------------------------

class NewWizard : public QWizard {
      Q_OBJECT

      QWizardPage* p1;
      QWizardPage* p2;
      QWizardPage* p3;
      QWizardPage* p4;

   public:
      NewWizard(QWidget* parent = 0);
      friend class QWizardPage;
      virtual int nextId() const;

      enum { Page_Type, Page_Instruments, Page_Timesig, Page_Template };
      };

#endif

