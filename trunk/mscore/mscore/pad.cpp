//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: pad.cpp,v 1.14 2006/03/02 17:08:40 wschweer Exp $
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

#include "pad.h"
#include "padids.h"
#include "sym.h"
#include "mscore.h"

#include "data/sbeam.xpm"
#include "data/mbeam.xpm"
#include "data/nbeam.xpm"
#include "data/beam32.xpm"
#include "icons.h"

const int PMW = 32;

PadEntry padTrans[PAD_LAYOUTS][PAD_KEYS] = {
      //----------------------------------------------------
      {
      { 0, PAD_REST,     QT_TRANSLATE_NOOP("Pad","rest") },
      { 0, PAD_NOTE32,   QT_TRANSLATE_NOOP("Pad","1/32") },
      { 0, PAD_NOTE16,   QT_TRANSLATE_NOOP("Pad","1/16") },
      { 0, PAD_NOTE8,    QT_TRANSLATE_NOOP("Pad","1/8") },
      { 0, PAD_NOTE4,    QT_TRANSLATE_NOOP("Pad","1/4") },
      { 0, PAD_NOTE2,    QT_TRANSLATE_NOOP("Pad","1/1") },
      { 0, PAD_NOTE1,    QT_TRANSLATE_NOOP("Pad","1/1") },
      { 0, PAD_NAT,      QT_TRANSLATE_NOOP("Pad","natural") },
      { 0, PAD_SHARP,    QT_TRANSLATE_NOOP("Pad","sharp") },
      { 0, PAD_FLAT,     QT_TRANSLATE_NOOP("Pad","flat") },
      { 0, PAD_ESCAPE,   QT_TRANSLATE_NOOP("Pad","escape") },
      { 0, PAD_AKZENT,   QT_TRANSLATE_NOOP("Pad","accent") },
      { 0, PAD_STACCATO, QT_TRANSLATE_NOOP("Pad","staccato") },
      { 0, PAD_TENUTO,   QT_TRANSLATE_NOOP("Pad","tenuto") },
      { 0, PAD_TIE,      QT_TRANSLATE_NOOP("Pad","tie") },
      { 0, PAD_EDIT,     ""    },
      { 0, PAD_DOT,      QT_TRANSLATE_NOOP("Pad","dot") },

      { 0, PAD_N0, 0 },
      { 0, PAD_N1, 0 },
      { 0, PAD_N2, 0 },
      { 0, PAD_N3, 0 },
      { 0, PAD_N4, 0 },
      },
      //----------------------------------------------------
      {
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, PAD_ESCAPE,   QT_TRANSLATE_NOOP("Pad","escape") },
      { 0, PAD_AKZENT,   QT_TRANSLATE_NOOP("Pad","accent") },
      { 0, PAD_STACCATO, QT_TRANSLATE_NOOP("Pad","staccato") },
      { 0, PAD_TENUTO,   QT_TRANSLATE_NOOP("Pad","tenuto") },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1,0 },

      { 0, PAD_N0, 0 },
      { 0, PAD_N1, 0 },
      { 0, PAD_N2, 0 },
      { 0, PAD_N3, 0 },
      { 0, PAD_N4, 0 },
      },
      //----------------------------------------------------
      {
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, PAD_NAT,    QT_TRANSLATE_NOOP("Pad","natural") },
      { 0, PAD_SHARP,  QT_TRANSLATE_NOOP("Pad","sharp") },
      { 0, PAD_FLAT,   QT_TRANSLATE_NOOP("Pad","flat") },
      { 0, PAD_ESCAPE, QT_TRANSLATE_NOOP("Pad","escape") },
      { 0, PAD_SHARP2, QT_TRANSLATE_NOOP("Pad","double sharp") },
      { 0, PAD_FLAT2,  QT_TRANSLATE_NOOP("Pad","double flat") },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, PAD_N0, 0 },
      { 0, PAD_N1, 0 },
      { 0, PAD_N2, 0 },
      { 0, PAD_N3, 0 },
      { 0, PAD_N4, 0 },
      },
      //----------------------------------------------------
      {
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, PAD_BEAM_START, QT_TRANSLATE_NOOP("Pad","beam start") },
      { 0, PAD_BEAM_MID,   QT_TRANSLATE_NOOP("Pad","beam mid") },
      { 0, PAD_BEAM_NO,    QT_TRANSLATE_NOOP("Pad","no beam") },
      { 0, PAD_ESCAPE,     QT_TRANSLATE_NOOP("Pad","escape") },
      { 0, PAD_BEAM32,     QT_TRANSLATE_NOOP("Pad","beam32") },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, PAD_N0, 0 },
      { 0, PAD_N1, 0 },
      { 0, PAD_N2, 0 },
      { 0, PAD_N3, 0 },
      { 0, PAD_N4, 0 },
      },
      //----------------------------------------------------
      {
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, PAD_ESCAPE,  QT_TRANSLATE_NOOP("Pad","escape") },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, -1, 0 },
      { 0, PAD_N0, 0 },
      { 0, PAD_N1, 0 },
      { 0, PAD_N2, 0 },
      { 0, PAD_N3, 0 },
      { 0, PAD_N4, 0 },
      }
      //----------------------------------------------------
      };

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Pad::mouseMoveEvent(QMouseEvent* ev)
      {
      move(ev->globalX()-dx, ev->globalY() - dy);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Pad::mousePressEvent(QMouseEvent* ev)
      {
      dx = ev->globalX() - x();
      dy = ev->globalY() - y();
      }

//---------------------------------------------------------
//   setButton
//---------------------------------------------------------

static void setButton(QToolButton* b, bool flag)
      {
      b->blockSignals(true);
      b->setChecked(flag);
      b->blockSignals(false);
      }

//---------------------------------------------------------
//   setOn
//---------------------------------------------------------

void Pad::setOn(bool flag, int cmd)
      {
      int id = -1;
      for (int i = 0; i < PAD_KEYS; ++i) {
            if (padTrans[_padNo][i].cmd == cmd) {
                  id = i;
                  break;
                  }
            }
// printf("setOn %d %d\n", id, flag);

      switch (id) {
            case -1:        break;
            case PAD_0:     setButton(button0, flag); break;
            case PAD_1:     setButton(button1, flag); break;
            case PAD_2:     setButton(button2, flag); break;
            case PAD_3:     setButton(button3, flag); break;
            case PAD_4:     setButton(button4, flag); break;
            case PAD_5:     setButton(button5, flag); break;
            case PAD_6:     setButton(button6, flag); break;
            case PAD_7:     setButton(button7, flag); break;
            case PAD_8:     setButton(button8, flag); break;
            case PAD_9:     setButton(button9, flag); break;
            case PAD_COMMA: setButton(buttonComma, flag); break;
            case PAD_MINUS: setButton(buttonMinus, flag); break;
            case PAD_PLUS:  setButton(buttonPlus, flag); break;
            case PAD_ENTER: setButton(buttonEnter, flag); break;
            case PAD_NUM:   setButton(buttonEscape, flag); break;
            case PAD_DIV:   setButton(buttonDivision, flag); break;
            case PAD_MULT:  setButton(buttonMultiplication, flag); break;

/*            case PAD_N0:    setButton(buttonPad1, flag); break;
            case PAD_N1:    setButton(buttonPad2, flag); break;
            case PAD_N2:    setButton(buttonPad3, flag); break;
            case PAD_N3:    setButton(buttonPad4, flag); break;
            case PAD_N4:    setButton(buttonPad5, flag); break;
*/
            }
      }

//---------------------------------------------------------
//   keyPressEvent
//    try to route all key events to scanvas
//---------------------------------------------------------

bool Pad::event(QEvent* ev)
      {
      if (ev->type() != QEvent::KeyPress)
            return QWidget::event(ev);
      QKeyEvent* e = (QKeyEvent*)ev;
      emit keyEvent(e);
      return false;
      }

//---------------------------------------------------------
//   setPadNo
//---------------------------------------------------------

void Pad::setPadNo(int n)
      {
      _padNo = n;
      blockSignals(true);

      QList<QAbstractButton*> bl = bg->buttons();
      foreach(QAbstractButton* b, bl) {
            int id = bg->id(b);
            if (b->isCheckable())
                  b->setChecked(false);
            int cmd = padTrans[_padNo][id].cmd;
            b->setEnabled(cmd != -1);
            if (padTrans[_padNo][id].icon)
                  b->setIcon(*padTrans[_padNo][id].icon);
            else if (id < PAD_N0) {
                  QPixmap pm(PMW, PMW);
                  QPainter painter(&pm);
                  pm.fill(b->palette().color(QPalette::Background));
                  b->setIcon(pm);
                  painter.end();
                  }
            b->setToolTip(QString(padTrans[_padNo][id].help));
            }
      buttonPad1->setChecked(n == 0);
      buttonPad2->setChecked(n == 1);
      buttonPad3->setChecked(n == 2);
      buttonPad4->setChecked(n == 3);
      buttonPad5->setChecked(n == 4);
      blockSignals(false);
      }

//---------------------------------------------------------
//   Pad
//---------------------------------------------------------

Pad::Pad(QWidget* parent)
   : QWidget(parent)
      {
      setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
      setupUi(this);

      setCursor(Qt::PointingHandCursor);

      bg = new QButtonGroup(this);
      bg->setExclusive(false);
      bg->addButton(buttonPad1, PAD_N0);
      bg->addButton(buttonPad2, PAD_N1);
      bg->addButton(buttonPad3, PAD_N2);
      bg->addButton(buttonPad4, PAD_N3);
      bg->addButton(buttonPad5, PAD_N4);

      bg->addButton(button0,    PAD_0);
      bg->addButton(button1,    PAD_1);
      bg->addButton(button2,    PAD_2);
      bg->addButton(button3,    PAD_3);
      bg->addButton(button4,    PAD_4);
      bg->addButton(button5,    PAD_5);
      bg->addButton(button6,    PAD_6);
      bg->addButton(button7,    PAD_7);
      bg->addButton(button8,    PAD_8);
      bg->addButton(button9,    PAD_9);
      bg->addButton(buttonPlus, PAD_PLUS);
      bg->addButton(buttonEnter,          PAD_ENTER);
      bg->addButton(buttonComma,          PAD_COMMA);
      bg->addButton(buttonMinus,          PAD_MINUS);
      bg->addButton(buttonMultiplication, PAD_MULT);
      bg->addButton(buttonDivision,       PAD_DIV);

      connect(bg, SIGNAL(buttonClicked(int)), mscore, SLOT(keyPadToggled(int)));

      QIcon* sbeam  = new QIcon(QPixmap(sbeam_xpm));
      QIcon* mbeam  = new QIcon(QPixmap(mbeam_xpm));
      QIcon* nbeam  = new QIcon(QPixmap(nbeam_xpm));
      QIcon* beam32 = new QIcon(QPixmap(beam32_xpm));

      padTrans[0][PAD_0].icon     = &quartrestIcon;
      padTrans[0][PAD_1].icon     = &note32Icon;
      padTrans[0][PAD_2].icon     = &note16Icon;
      padTrans[0][PAD_3].icon     = &note8Icon;
      padTrans[0][PAD_4].icon     = &note4Icon;
      padTrans[0][PAD_5].icon     = &note2Icon;
      padTrans[0][PAD_6].icon     = &noteIcon;
      padTrans[0][PAD_7].icon     = &naturalIcon;
      padTrans[0][PAD_8].icon     = &sharpIcon;
      padTrans[0][PAD_9].icon     = &flatIcon;
      padTrans[0][PAD_COMMA].icon = &dotIcon;
      padTrans[0][PAD_DIV].icon   = &sforzatoaccentIcon;
      padTrans[0][PAD_MULT].icon  = &staccatoIcon;
      padTrans[0][PAD_MINUS].icon = &tenutoIcon;

      padTrans[1][PAD_DIV].icon   = &sforzatoaccentIcon;
      padTrans[1][PAD_MULT].icon  = &staccatoIcon;
      padTrans[1][PAD_MINUS].icon = &tenutoIcon;

      padTrans[2][PAD_DIV].icon   = &sharpsharpIcon;
      padTrans[2][PAD_MULT].icon  = &flatflatIcon;
      padTrans[2][PAD_7].icon     = &naturalIcon;
      padTrans[2][PAD_8].icon     = &sharpIcon;
      padTrans[2][PAD_9].icon     = &flatIcon;

      padTrans[3][PAD_7].icon     = sbeam;
      padTrans[3][PAD_8].icon     = mbeam;
      padTrans[3][PAD_9].icon     = nbeam;
      padTrans[3][PAD_DIV].icon   = beam32;

      setPadNo(0);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Pad::closeEvent(QCloseEvent* ev)
      {
      emit close();
      QWidget::closeEvent(ev);
      }

