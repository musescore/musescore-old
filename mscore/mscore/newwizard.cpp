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

#include "newwizard.h"
#include "mscore.h"

//---------------------------------------------------------
//   NewWizardPage1
//---------------------------------------------------------

NewWizardPage1::NewWizardPage1(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("This wizard lets you create a new score"));
      rb1 = new QRadioButton(tr("Create new score from template"));
      rb2 = new QRadioButton(tr("Create new score from scratch"));
      rb2->setChecked(true);
      registerField("useTemplate", rb1, "checked");
      connect(rb1, SIGNAL(toggled(bool)), SLOT(rb1Toggled(bool)));
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(rb1, 0, 0);
      grid->addWidget(rb2, 1, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   NewWizardPage4
//---------------------------------------------------------

NewWizardPage4::NewWizardPage4(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Select Template File:"));

      QStringList nameFilter;
      nameFilter.append("*.msc");

      model = new QDirModel;
      model->setNameFilters(nameFilter);

      tree  = new QTreeView;
      tree->setModel(model);
      tree->header()->hideSection(1);
      tree->header()->hideSection(2);
      tree->header()->hideSection(3);

      QString path(mscoreGlobalShare);
      path += "/templates";
      tree->setRootIndex(model->index(path));
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(tree, 0, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   NewWizard
//---------------------------------------------------------

NewWizard::NewWizard(QWidget* parent)
   : QWizard(parent)
      {
      setPixmap(QWizard::LogoPixmap, QPixmap(":/data/mscore.png"));
      setWindowTitle(tr("MuseScore: Create New Score"));
      setOption(QWizard::HaveFinishButtonOnEarlyPages);

      p1 = new NewWizardPage1;

      p2 = new QWizardPage;
      p2->setTitle(tr("Create New Score"));
      p2->setSubTitle(tr("In this page you create a set of instruments. Each instrument"
                         " is represented by one or more staves"));

      p3 = new QWizardPage;
      p3->setTitle(tr("Create New Score"));
      p3->setSubTitle(tr("Create Time Signature"));
      p3->setFinalPage(true);

      p4 = new NewWizardPage4;
      p4->setFinalPage(true);

      setPage(Page_Type, p1);
      setPage(Page_Instruments, p2);
      setPage(Page_Timesig, p3);
      setPage(Page_Template, p4);
      }

//---------------------------------------------------------
//   rb1Toggled
//---------------------------------------------------------

void NewWizardPage1::rb1Toggled(bool val)
      {
      printf("rb1 toggled %d\n", val);
      }

//---------------------------------------------------------
//   nextId
//---------------------------------------------------------

int NewWizard::nextId() const
      {
      bool useTemplate = field("useTemplate").toBool();
      printf("nextIdx use template %d\n", useTemplate);

      switch(currentId()) {
            case Page_Type:
                  if (useTemplate)
                        return Page_Template;
                  return Page_Instruments;
            case Page_Instruments:
                  return Page_Timesig;
            case Page_Timesig:
            default:
                  return -1;
            }
      }

