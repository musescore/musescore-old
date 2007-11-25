//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "symboldialog.h"
#include "palette.h"
#include "mscore.h"
#include "sym.h"
#include "style.h"
#include "element.h"
#include "symbol.h"
#include "preferences.h"

Palette* symbolPalette = 0;

static const int ROWS = 11;
static const int COLUMNS = 16;

//---------------------------------------------------------
//   createSymbolPalette
//---------------------------------------------------------

void createSymbolPalette()
      {
      symbolPalette = new Palette(ROWS, COLUMNS);
      symbolPalette->addObject(0, wholerestSym);
      symbolPalette->addObject(1, halfrestSym);
      symbolPalette->addObject(2, outsidewholerestSym);
      symbolPalette->addObject(3, outsidehalfrestSym);
      symbolPalette->addObject(4, longarestSym);
      symbolPalette->addObject(5, breverestSym);
      symbolPalette->addObject(6, quartrestSym);
      symbolPalette->addObject(7, eighthrestSym);
      symbolPalette->addObject(8, clasquartrestSym);
      symbolPalette->addObject(9, sixteenthrestSym);
      symbolPalette->addObject(10, thirtysecondrestSym);
      symbolPalette->addObject(11, sixtyfourthrestSym);
      symbolPalette->addObject(12, hundredtwentyeighthrestSym);
      symbolPalette->addObject(16, zeroSym);
      symbolPalette->addObject(17, oneSym);
      symbolPalette->addObject(18, twoSym);
      symbolPalette->addObject(19, threeSym);
      symbolPalette->addObject(20, fourSym);
      symbolPalette->addObject(21, fiveSym);
      symbolPalette->addObject(22, sixSym);
      symbolPalette->addObject(23, sevenSym);
      symbolPalette->addObject(24, eightSym);
      symbolPalette->addObject(25, nineSym);
      symbolPalette->addObject(31, sharpSym);
      symbolPalette->addObject(32, naturalSym);
      symbolPalette->addObject(33, flatSym);
      symbolPalette->addObject(34, flatflatSym);
      symbolPalette->addObject(35, sharpsharpSym);
      symbolPalette->addObject(36, rightparenSym);
      symbolPalette->addObject(37, leftparenSym);
      symbolPalette->addObject(38, dotSym);
      symbolPalette->addObject(39, brevisheadSym);
      symbolPalette->addObject(40, wholeheadSym);
      symbolPalette->addObject(41, halfheadSym);
      symbolPalette->addObject(42, quartheadSym);
      symbolPalette->addObject(43, wholediamondheadSym);
      symbolPalette->addObject(44, halfdiamondheadSym);
      symbolPalette->addObject(45, diamondheadSym);
      symbolPalette->addObject(46, wholetriangleheadSym);
      symbolPalette->addObject(47, halftriangleheadSym);
      symbolPalette->addObject(48, triangleheadSym);
      symbolPalette->addObject(49, wholeslashheadSym);
      symbolPalette->addObject(50, halfslashheadSym);
      symbolPalette->addObject(51, quartslashheadSym);
      symbolPalette->addObject(52, wholecrossedheadSym);
      symbolPalette->addObject(53, halfcrossedheadSym);
      symbolPalette->addObject(54, crossedheadSym);
      symbolPalette->addObject(55, xcircledheadSym);
      symbolPalette->addObject(57, ufermataSym);
      symbolPalette->addObject(58, dfermataSym);
      symbolPalette->addObject(59, thumbSym);
      symbolPalette->addObject(60, sforzatoaccentSym);
      symbolPalette->addObject(61, staccatoSym);
      symbolPalette->addObject(62, ustaccatissimoSym);
      symbolPalette->addObject(63, dstaccatissimoSym);
      symbolPalette->addObject(64, tenutoSym);
      symbolPalette->addObject(65, uportatoSym);
      symbolPalette->addObject(66, dportatoSym);
      symbolPalette->addObject(67, umarcatoSym);
      symbolPalette->addObject(68, dmarcatoSym);
      symbolPalette->addObject(69, ouvertSym);
      symbolPalette->addObject(70, plusstopSym);
      symbolPalette->addObject(71, upbowSym);
      symbolPalette->addObject(72, downbowSym);
      symbolPalette->addObject(73, reverseturnSym);
      symbolPalette->addObject(74, turnSym);
      symbolPalette->addObject(75, trillSym);
      symbolPalette->addObject(76, upedalheelSym);
      symbolPalette->addObject(77, dpedalheelSym);
      symbolPalette->addObject(78, upedaltoeSym);
      symbolPalette->addObject(79, dpedaltoeSym);
      symbolPalette->addObject(80, flageoletSym);
      symbolPalette->addObject(81, segnoSym);
      symbolPalette->addObject(82, codaSym);
      symbolPalette->addObject(83, rcommaSym);
      symbolPalette->addObject(84, lcommaSym);
      symbolPalette->addObject(85, arpeggioSym);
      symbolPalette->addObject(86, trillelementSym);
      symbolPalette->addObject(87, arpeggioarrowdownSym);
      symbolPalette->addObject(88, arpeggioarrowupSym);
      symbolPalette->addObject(89, trilelementSym);
      symbolPalette->addObject(90, prallSym);
      symbolPalette->addObject(91, mordentSym);
      symbolPalette->addObject(92, prallprallSym);
      symbolPalette->addObject(93, prallmordentSym);
      symbolPalette->addObject(94, upprallSym);
      symbolPalette->addObject(95, downprallSym);
      symbolPalette->addObject(96, upmordentSym);
      symbolPalette->addObject(97, downmordentSym);
      symbolPalette->addObject(98, lineprallSym);
      symbolPalette->addObject(99, pralldownSym);
      symbolPalette->addObject(101, prallupSym);
      symbolPalette->addObject(102, eighthflagSym);
      symbolPalette->addObject(103, sixteenthflagSym);
      symbolPalette->addObject(104, thirtysecondflagSym);
      symbolPalette->addObject(105, sixtyfourthflagSym);
      symbolPalette->addObject(106, deighthflagSym);
      symbolPalette->addObject(107, gracedashSym);
      symbolPalette->addObject(108, dgracedashSym);
      symbolPalette->addObject(109, dsixteenthflagSym);
      symbolPalette->addObject(110, dthirtysecondflagSym);
      symbolPalette->addObject(111, dsixtyfourthflagSym);
      symbolPalette->addObject(112, stemSym);
      symbolPalette->addObject(113, dstemSym);
      symbolPalette->addObject(114, altoclefSym);
      symbolPalette->addObject(115, caltoclefSym);
      symbolPalette->addObject(116, bassclefSym);
      symbolPalette->addObject(117, cbassclefSym);
      symbolPalette->addObject(118, trebleclefSym);
      symbolPalette->addObject(119, ctrebleclefSym);
      symbolPalette->addObject(120, percussionclefSym);
      symbolPalette->addObject(121, cpercussionclefSym);
      symbolPalette->addObject(122, tabclefSym);
      symbolPalette->addObject(123, ctabclefSym);
      symbolPalette->addObject(124, fourfourmeterSym);
      symbolPalette->addObject(125, allabreveSym);
      symbolPalette->addObject(126, pedalasteriskSym);
      symbolPalette->addObject(127, pedaldashSym);
      symbolPalette->addObject(128, pedaldotSym);
      symbolPalette->addObject(129, pedalPSym);
      symbolPalette->addObject(130, pedaldSym);
      symbolPalette->addObject(131, pedaleSym);
      symbolPalette->addObject(132, pedalPedSym);
      symbolPalette->addObject(133, accDiscantSym);
      symbolPalette->addObject(134, accDotSym);
      symbolPalette->addObject(135, accFreebaseSym);
      symbolPalette->addObject(136, accStdbaseSym);
      symbolPalette->addObject(137, accBayanbaseSym);
      symbolPalette->addObject(138, accSBSym);
      symbolPalette->addObject(139, accBBSym);
      symbolPalette->addObject(140, accOldEESym);
      symbolPalette->addObject(141, accOldEESSym);
      symbolPalette->addObject(142, wholedoheadSym);
      symbolPalette->addObject(143, halfdoheadSym);
      symbolPalette->addObject(144, doheadSym);
      symbolPalette->addObject(145, wholereheadSym);
      symbolPalette->addObject(146, halfreheadSym);
      symbolPalette->addObject(147, reheadSym);
      symbolPalette->addObject(148, wholemeheadSym);
      symbolPalette->addObject(149, halfmeheadSym);
      symbolPalette->addObject(150, meheadSym);
      symbolPalette->addObject(151, wholefaheadSym);
      symbolPalette->addObject(152, halffauheadSym);
      symbolPalette->addObject(152, fauheadSym);
      symbolPalette->addObject(153, halffadheadSym);
      symbolPalette->addObject(154, fadheadSym);
      symbolPalette->addObject(155, wholelaheadSym);
      symbolPalette->addObject(156, halflaheadSym);
      symbolPalette->addObject(157, laheadSym);
      symbolPalette->addObject(158, wholeteheadSym);
      symbolPalette->addObject(159, halfteheadSym);
      symbolPalette->addObject(160, letterfSym);
      symbolPalette->addObject(161, lettermSym);
      symbolPalette->addObject(162, letterpSym);
      symbolPalette->addObject(163, letterrSym);
      symbolPalette->addObject(164, lettersSym);
      symbolPalette->addObject(165, letterzSym);
      }

//---------------------------------------------------------
//   SymbolDialog
//---------------------------------------------------------

SymbolDialog::SymbolDialog(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Symbols"));
      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      QScrollArea* sa = new QScrollArea;
      l->addWidget(sa);

      QButtonGroup* bg = new QButtonGroup();
      bg->addButton(anchorPage, ANCHOR_PARENT);
      bg->addButton(anchorTime, ANCHOR_STAFF);
      bg->addButton(anchorNote, ANCHOR_PARENT);
      bg->addButton(anchorSystem, ANCHOR_MEASURE);
      connect(bg, SIGNAL(buttonClicked(int)), SLOT(anchorClicked(int)));
      anchorPage->setChecked(true);

      if (symbolPalette == 0)
            createSymbolPalette();
      sp = symbolPalette;
      sp->setAcceptDrops(true);
      sp->setDrawGrid(true);
      sp->setShowSelection(true);

      connect(sp, SIGNAL(droppedElement(Element*)), SLOT(elementDropped(Element*)));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteElement()));
      sa->setWidget(sp);
      }

//---------------------------------------------------------
//   anchorClicked
//---------------------------------------------------------

void SymbolDialog::anchorClicked(int val)
      {
      Anchor anchor = (Anchor)val;
      int rows    = sp->getRows();
      int columns = sp->getColumns();

      for (int i = 0; i < rows * columns; ++i) {
            Element* e = sp->element(i);
            if (e && e->type() == SYMBOL)
                  ((Symbol*)e)->setAnchor(anchor);
            }
      }

//---------------------------------------------------------
//   elementDropped
//---------------------------------------------------------

void SymbolDialog::elementDropped(Element*)
      {
      preferences.dirty = true;
      }

//---------------------------------------------------------
//   deleteElement
//---------------------------------------------------------

void SymbolDialog::deleteElement()
      {
      int idx = sp->getSelectedIdx();
      if (idx == -1)
            return;
      sp->addObject(idx, 0, QString());
      preferences.dirty = true;
      }
