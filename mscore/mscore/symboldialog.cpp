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

//---------------------------------------------------------
//   createSymbolPalette
//---------------------------------------------------------

void SymbolDialog::createSymbolPalette()
      {
      sp = new Palette();
      sp->append(wholerestSym);
      sp->append(halfrestSym);
      sp->append(outsidewholerestSym);
      sp->append(outsidehalfrestSym);
      sp->append(longarestSym);
      sp->append(breverestSym);
      sp->append(quartrestSym);
      sp->append(eighthrestSym);
      sp->append(clasquartrestSym);
      sp->append(sixteenthrestSym);
      sp->append(thirtysecondrestSym);
      sp->append(sixtyfourthrestSym);
      sp->append(hundredtwentyeighthrestSym);
      sp->append(zeroSym);
      sp->append(oneSym);
      sp->append(twoSym);
      sp->append(threeSym);
      sp->append(fourSym);
      sp->append(fiveSym);
      sp->append(sixSym);
      sp->append(sevenSym);
      sp->append(eightSym);
      sp->append(nineSym);
      sp->append(sharpSym);
      sp->append(naturalSym);
      sp->append(flatSym);
      sp->append(flatflatSym);
      sp->append(sharpsharpSym);
      sp->append(rightparenSym);
      sp->append(leftparenSym);
      sp->append(dotSym);
      sp->append(brevisheadSym);
      sp->append(wholeheadSym);
      sp->append(halfheadSym);
      sp->append(quartheadSym);
      sp->append(wholediamondheadSym);
      sp->append(halfdiamondheadSym);
      sp->append(diamondheadSym);
      sp->append(wholetriangleheadSym);
      sp->append(halftriangleheadSym);
      sp->append(triangleheadSym);
      sp->append(wholeslashheadSym);
      sp->append(halfslashheadSym);
      sp->append(quartslashheadSym);
      sp->append(wholecrossedheadSym);
      sp->append(halfcrossedheadSym);
      sp->append(crossedheadSym);
      sp->append(xcircledheadSym);
      sp->append(ufermataSym);
      sp->append(dfermataSym);
      sp->append(thumbSym);
      sp->append(sforzatoaccentSym);
      sp->append(staccatoSym);
      sp->append(ustaccatissimoSym);
      sp->append(dstaccatissimoSym);
      sp->append(tenutoSym);
      sp->append(uportatoSym);
      sp->append(dportatoSym);
      sp->append(umarcatoSym);
      sp->append(dmarcatoSym);
      sp->append(ouvertSym);
      sp->append(plusstopSym);
      sp->append(upbowSym);
      sp->append(downbowSym);
      sp->append(reverseturnSym);
      sp->append(turnSym);
      sp->append(trillSym);
      sp->append(upedalheelSym);
      sp->append(dpedalheelSym);
      sp->append(upedaltoeSym);
      sp->append(dpedaltoeSym);
      sp->append(flageoletSym);
      sp->append(segnoSym);
      sp->append(codaSym);
      sp->append(rcommaSym);
      sp->append(lcommaSym);
      sp->append(arpeggioSym);
      sp->append(trillelementSym);
      sp->append(arpeggioarrowdownSym);
      sp->append(arpeggioarrowupSym);
      sp->append(trilelementSym);
      sp->append(prallSym);
      sp->append(mordentSym);
      sp->append(prallprallSym);
      sp->append(prallmordentSym);
      sp->append(upprallSym);
      sp->append(downprallSym);
      sp->append(upmordentSym);
      sp->append(downmordentSym);
      sp->append(lineprallSym);
      sp->append(pralldownSym);
      sp->append(prallupSym);
      sp->append(eighthflagSym);
      sp->append(sixteenthflagSym);
      sp->append(thirtysecondflagSym);
      sp->append(sixtyfourthflagSym);
      sp->append(deighthflagSym);
      sp->append(gracedashSym);
      sp->append(dgracedashSym);
      sp->append(dsixteenthflagSym);
      sp->append(dthirtysecondflagSym);
      sp->append(dsixtyfourthflagSym);
      sp->append(stemSym);
      sp->append(dstemSym);
      sp->append(altoclefSym);
      sp->append(caltoclefSym);
      sp->append(bassclefSym);
      sp->append(cbassclefSym);
      sp->append(trebleclefSym);
      sp->append(ctrebleclefSym);
      sp->append(percussionclefSym);
      sp->append(cpercussionclefSym);
      sp->append(tabclefSym);
      sp->append(ctabclefSym);
      sp->append(fourfourmeterSym);
      sp->append(allabreveSym);
      sp->append(pedalasteriskSym);
      sp->append(pedaldashSym);
      sp->append(pedaldotSym);
      sp->append(pedalPSym);
      sp->append(pedaldSym);
      sp->append(pedaleSym);
      sp->append(pedalPedSym);
      sp->append(accDiscantSym);
      sp->append(accDotSym);
      sp->append(accFreebaseSym);
      sp->append(accStdbaseSym);
      sp->append(accBayanbaseSym);
      sp->append(accSBSym);
      sp->append(accBBSym);
      sp->append(accOldEESym);
      sp->append(accOldEESSym);
      sp->append(wholedoheadSym);
      sp->append(halfdoheadSym);
      sp->append(doheadSym);
      sp->append(wholereheadSym);
      sp->append(halfreheadSym);
      sp->append(reheadSym);
      sp->append(wholemeheadSym);
      sp->append(halfmeheadSym);
      sp->append(meheadSym);
      sp->append(wholefaheadSym);
      sp->append(halffauheadSym);
      sp->append(fauheadSym);
      sp->append(halffadheadSym);
      sp->append(fadheadSym);
      sp->append(wholelaheadSym);
      sp->append(halflaheadSym);
      sp->append(laheadSym);
      sp->append(wholeteheadSym);
      sp->append(halfteheadSym);
      sp->append(letterfSym);
      sp->append(lettermSym);
      sp->append(letterpSym);
      sp->append(letterrSym);
      sp->append(lettersSym);
      sp->append(letterzSym);
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

      createSymbolPalette();

      sp->setAcceptDrops(true);
      sp->setDrawGrid(true);
      sp->setSelectable(true);

      connect(sp, SIGNAL(changed()), SLOT(setDirty()));
      connect(sp, SIGNAL(startDragElement(Element*)), SLOT(startDragElement(Element*)));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteElement()));
      sa->setWidget(sp);
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void SymbolDialog::setDirty()
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
      sp->add(idx, 0, QString());
      preferences.dirty = true;
      }

//---------------------------------------------------------
//   startDragElement
//---------------------------------------------------------

void SymbolDialog::startDragElement(Element* el)
      {
      el->setSystemFlag(systemFlag->isChecked());
      }

