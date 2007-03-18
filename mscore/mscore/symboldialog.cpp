//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer (ws@seh.de)
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

static const int ROWS = 11;
static const int COLUMNS = 16;

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
      QScrollArea* symbolPalette = new QScrollArea;
      l->addWidget(symbolPalette);

      QButtonGroup* bg = new QButtonGroup();
      bg->addButton(anchorPage, ANCHOR_PAGE);
      bg->addButton(anchorTime, ANCHOR_TICK);
      bg->addButton(anchorNote, ANCHOR_NOTE);
      bg->addButton(anchorSystem, ANCHOR_SYSTEM);
      connect(bg, SIGNAL(buttonClicked(int)), SLOT(anchorClicked(int)));
      anchorPage->setChecked(true);

      sp = new SymbolPalette(ROWS, COLUMNS);
      symbolPalette->setWidget(sp);
      sp->addObject(0, wholerestSym);
      sp->addObject(1, halfrestSym);
      sp->addObject(2, outsidewholerestSym);
      sp->addObject(3, outsidehalfrestSym);
      sp->addObject(4, longarestSym);
      sp->addObject(5, breverestSym);
      sp->addObject(6, quartrestSym);
      sp->addObject(7, eighthrestSym);
      sp->addObject(8, clasquartrestSym);
      sp->addObject(9, sixteenthrestSym);
      sp->addObject(10, thirtysecondrestSym);
      sp->addObject(11, sixtyfourthrestSym);
      sp->addObject(12, hundredtwentyeighthrestSym);
      sp->addObject(16, zeroSym);
      sp->addObject(17, oneSym);
      sp->addObject(18, twoSym);
      sp->addObject(19, threeSym);
      sp->addObject(20, fourSym);
      sp->addObject(21, fiveSym);
      sp->addObject(22, sixSym);
      sp->addObject(23, sevenSym);
      sp->addObject(24, eightSym);
      sp->addObject(25, nineSym);
      sp->addObject(31, sharpSym);
      sp->addObject(32, naturalSym);
      sp->addObject(33, flatSym);
      sp->addObject(34, flatflatSym);
      sp->addObject(35, sharpsharpSym);
      sp->addObject(36, rightparenSym);
      sp->addObject(37, leftparenSym);
      sp->addObject(38, dotSym);
      sp->addObject(39, brevisheadSym);
      sp->addObject(40, wholeheadSym);
      sp->addObject(41, halfheadSym);
      sp->addObject(42, quartheadSym);
      sp->addObject(43, wholediamondheadSym);
      sp->addObject(44, halfdiamondheadSym);
      sp->addObject(45, diamondheadSym);
      sp->addObject(46, wholetriangleheadSym);
      sp->addObject(47, halftriangleheadSym);
      sp->addObject(48, triangleheadSym);
      sp->addObject(49, wholeslashheadSym);
      sp->addObject(50, halfslashheadSym);
      sp->addObject(51, quartslashheadSym);
      sp->addObject(52, wholecrossedheadSym);
      sp->addObject(53, halfcrossedheadSym);
      sp->addObject(54, crossedheadSym);
      sp->addObject(55, xcircledheadSym);

      sp->addObject(57, ufermataSym);
      sp->addObject(58, dfermataSym);
      sp->addObject(59, thumbSym);
      sp->addObject(60, sforzatoaccentSym);
      sp->addObject(61, staccatoSym);
      sp->addObject(62, ustaccatissimoSym);
      sp->addObject(63, dstaccatissimoSym);
      sp->addObject(64, tenutoSym);
      sp->addObject(65, uportatoSym);
      sp->addObject(66, dportatoSym);
      sp->addObject(67, umarcatoSym);
      sp->addObject(68, dmarcatoSym);
      sp->addObject(69, ouvertSym);
      sp->addObject(70, plusstopSym);
      sp->addObject(71, upbowSym);
      sp->addObject(72, downbowSym);
      sp->addObject(73, reverseturnSym);
      sp->addObject(74, turnSym);
      sp->addObject(75, trillSym);
      sp->addObject(76, upedalheelSym);
      sp->addObject(77, dpedalheelSym);
      sp->addObject(78, upedaltoeSym);
      sp->addObject(79, dpedaltoeSym);
      sp->addObject(80, flageoletSym);
      sp->addObject(81, segnoSym);
      sp->addObject(82, codaSym);
      sp->addObject(83, rcommaSym);
      sp->addObject(84, lcommaSym);
      sp->addObject(85, arpeggioSym);
      sp->addObject(86, trillelementSym);
      sp->addObject(87, arpeggioarrowdownSym);
      sp->addObject(88, arpeggioarrowupSym);
      sp->addObject(89, trilelementSym);
      sp->addObject(90, prallSym);
      sp->addObject(91, mordentSym);
      sp->addObject(92, prallprallSym);
      sp->addObject(93, prallmordentSym);
      sp->addObject(94, upprallSym);
      sp->addObject(95, downprallSym);
      sp->addObject(96, upmordentSym);
      sp->addObject(97, downmordentSym);
      sp->addObject(98, lineprallSym);
      sp->addObject(99, pralldownSym);
      sp->addObject(101, prallupSym);
      sp->addObject(102, eighthflagSym);
      sp->addObject(103, sixteenthflagSym);
      sp->addObject(104, thirtysecondflagSym);
      sp->addObject(105, sixtyfourthflagSym);
      sp->addObject(106, deighthflagSym);
      sp->addObject(107, gracedashSym);
      sp->addObject(108, dgracedashSym);
      sp->addObject(109, dsixteenthflagSym);
      sp->addObject(110, dthirtysecondflagSym);
      sp->addObject(111, dsixtyfourthflagSym);
      sp->addObject(112, stemSym);
      sp->addObject(113, dstemSym);
      sp->addObject(114, altoclefSym);
      sp->addObject(115, caltoclefSym);
      sp->addObject(116, bassclefSym);
      sp->addObject(117, cbassclefSym);
      sp->addObject(118, trebleclefSym);
      sp->addObject(119, ctrebleclefSym);
      sp->addObject(120, percussionclefSym);
      sp->addObject(121, cpercussionclefSym);
      sp->addObject(122, tabclefSym);
      sp->addObject(123, ctabclefSym);
      sp->addObject(124, fourfourmeterSym);
      sp->addObject(125, allabreveSym);
      sp->addObject(126, pedalasteriskSym);
      sp->addObject(127, pedaldashSym);
      sp->addObject(128, pedaldotSym);
      sp->addObject(129, pedalPSym);
      sp->addObject(130, pedaldSym);
      sp->addObject(131, pedaleSym);
      sp->addObject(132, pedalPedSym);
      sp->addObject(133, accDiscantSym);
      sp->addObject(134, accDotSym);
      sp->addObject(135, accFreebaseSym);
      sp->addObject(136, accStdbaseSym);
      sp->addObject(137, accBayanbaseSym);
      sp->addObject(138, accSBSym);
      sp->addObject(139, accBBSym);
      sp->addObject(140, accOldEESym);
      sp->addObject(141, accOldEESSym);
      sp->addObject(142, wholedoheadSym);
      sp->addObject(143, halfdoheadSym);
      sp->addObject(144, doheadSym);
      sp->addObject(145, wholereheadSym);
      sp->addObject(146, halfreheadSym);
      sp->addObject(147, reheadSym);
      sp->addObject(148, wholemeheadSym);
      sp->addObject(149, halfmeheadSym);
      sp->addObject(150, meheadSym);
      sp->addObject(151, wholefaheadSym);
      sp->addObject(152, halffauheadSym);
      sp->addObject(152, fauheadSym);
      sp->addObject(153, halffadheadSym);
      sp->addObject(154, fadheadSym);
      sp->addObject(155, wholelaheadSym);
      sp->addObject(156, halflaheadSym);
      sp->addObject(157, laheadSym);
      sp->addObject(158, wholeteheadSym);
      sp->addObject(159, halfteheadSym);
      sp->addObject(160, letterfSym);
      sp->addObject(161, lettermSym);
      sp->addObject(162, letterpSym);
      sp->addObject(163, letterrSym);
      sp->addObject(164, lettersSym);
      sp->addObject(165, letterzSym);
      }

//---------------------------------------------------------
//   anchorClicked
//---------------------------------------------------------

void SymbolDialog::anchorClicked(int val)
      {
      Anchor anchor = (Anchor)val;
      for (int i = 0; i < ROWS * COLUMNS; ++i) {
            Element* e = sp->element(i);
            if (e && e->type() == SYMBOL)
                  ((Symbol*)e)->setAnchor(anchor);
            }
      }

