//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: icons.cpp,v 1.16 2006/03/02 17:08:34 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "globals.h"
#include "icons.h"
#include "score.h"
#include "style.h"
#include "preferences.h"
#include "sym.h"

#include "data/editcut.xpm"
#include "data/editcopy.xpm"
#include "data/editpaste.xpm"
#include "data/fileprint.xpm"

QIcon noteIcon;
QIcon note2Icon;
QIcon note4Icon;
QIcon note8Icon;
QIcon note16Icon;
QIcon note32Icon;
QIcon note64Icon;

QIcon naturalIcon;
QIcon sharpIcon;
QIcon sharpsharpIcon;
QIcon flatIcon;
QIcon flatflatIcon;
QIcon quartrestIcon;
QIcon dotIcon;
QIcon dotdotIcon;
QIcon sforzatoaccentIcon;
QIcon staccatoIcon;
QIcon tenutoIcon;
QIcon plusIcon;
QIcon flipIcon;
QIcon voiceIcons[VOICES];
QIcon noteEntryIcon;

QIcon undoIcon, redoIcon, cutIcon, copyIcon, pasteIcon;
QIcon printIcon, clefIcon;
QIcon midiinIcon, speakerIcon, startIcon, playIcon, pauseIcon, repeatIcon;
QIcon sbeamIcon, mbeamIcon, nbeamIcon, beam32Icon, abeamIcon;
QIcon fileOpenIcon, fileNewIcon, fileSaveIcon, fileSaveAsIcon;
QIcon exitIcon, viewmagIcon;
QIcon windowIcon;

QIcon acciaccaturaIcon, appoggiaturaIcon;
QIcon grace4Icon, grace16Icon, grace32Icon;

//---------------------------------------------------------
//   symIcon
//    default size=20
//---------------------------------------------------------

QIcon symIcon(const Sym& sc, int size, int width, int height)
      {
      double mag = (size/20.0) * 0.6 * (_spatium / (SPATIUM20 * DPI)) * PDPI / DPI;
      QRectF bb = sc.bbox(mag);
      qreal w   = bb.width();
      qreal h   = bb.height();
      qreal x   = (width  - w) * .5 - bb.x();
      qreal y   = (height - h) * .5 - bb.y();

      QPixmap image(width, height);
      image.fill(QColor(255, 255, 255, 0));
      QPainter painter(&image);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      painter.setFont(sc.font());
      painter.setPen(QPen(QColor(0, 0, 0, 255)));
      sc.draw(painter, mag, x, y);
      painter.end();
      return QIcon(image);
      }

//---------------------------------------------------------
//   genIcons
//    create some icons
//---------------------------------------------------------

void genIcons()
      {
      noteIcon           = symIcon(symbols[wholeheadSym]);
      note2Icon          = symIcon(symbols[note2Sym]);
      note4Icon          = symIcon(symbols[note4Sym]);
      note8Icon          = symIcon(symbols[note8Sym]);
      note16Icon         = symIcon(symbols[note16Sym]);
      note32Icon         = symIcon(symbols[note32Sym]);
      note64Icon         = symIcon(symbols[note64Sym]);
      naturalIcon        = symIcon(symbols[naturalSym], 30);
      sharpIcon          = symIcon(symbols[sharpSym], 30);
      sharpsharpIcon     = symIcon(symbols[sharpsharpSym], 30);
      flatIcon           = symIcon(symbols[flatSym], 30);
      flatflatIcon       = symIcon(symbols[flatflatSym], 30);
      quartrestIcon      = symIcon(symbols[quartrestSym], 30);
      dotIcon            = symIcon(symbols[dotSym], 30);
      dotdotIcon         = symIcon(symbols[dotdotSym], 30);
      sforzatoaccentIcon = symIcon(symbols[sforzatoaccentSym]);
      staccatoIcon       = symIcon(symbols[staccatoSym]);
      tenutoIcon         = symIcon(symbols[tenutoSym]);
      plusIcon           = symIcon(symbols[plusSym], 30);
      clefIcon           = symIcon(symbols[trebleclefSym], 17);
      staccatoIcon       = symIcon(symbols[dotSym], 30);   //":/data/staccato.svg");

      acciaccaturaIcon   = QIcon(":/data/acciaccatura.svg");
      appoggiaturaIcon   = QIcon(":/data/appoggiatura.svg");

      static const char* vtext[VOICES] = { "1","2","3","4" };
      for (int i = 0; i < VOICES; ++i) {
            QPixmap image(ICON_WIDTH, ICON_HEIGHT);
            QColor c(preferences.selectColor[i].light(180));
            image.fill(c);
            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, ICON_WIDTH, ICON_HEIGHT), Qt::AlignCenter, vtext[i]);
            painter.end();
            voiceIcons[i].addPixmap(image);

            painter.begin(&image);
            c = QColor(preferences.selectColor[i].light(140));
            painter.fillRect(0, 0, ICON_WIDTH, ICON_HEIGHT, c);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, ICON_WIDTH, ICON_HEIGHT), Qt::AlignCenter, vtext[i]);
            painter.end();
            voiceIcons[i].addPixmap(image, QIcon::Normal, QIcon::On);
            }
      flipIcon = QIcon(":/data/flip.svg");

      cutIcon.addPixmap(QPixmap(editcut_xpm));
      copyIcon.addPixmap(QPixmap(editcopy_xpm));
      pasteIcon.addPixmap(QPixmap(editpaste_xpm));
      printIcon.addPixmap(QPixmap(fileprint_xpm));

      undoIcon     = QIcon(":/data/undo.svg");
      redoIcon     = QIcon(":/data/redo.svg");
      midiinIcon   = QIcon(":/data/midiin.svg");
      speakerIcon  = QIcon(":/data/speaker.svg");
      startIcon    = QIcon(":/data/start.svg");
      playIcon     = QIcon(":/data/play.svg");
      pauseIcon    = QIcon(":/data/pause.svg");
      sbeamIcon    = QIcon(":/data/sbeam.svg");
      mbeamIcon    = QIcon(":/data/mbeam.svg");
      nbeamIcon    = QIcon(":/data/nbeam.svg");
      beam32Icon   = QIcon(":/data/beam32.svg");
      abeamIcon    = QIcon(":/data/abeam.svg");
      fileOpenIcon = QIcon(":/data/fileopen.svg");
      fileNewIcon  = QIcon(":/data/filenew.svg");
      fileSaveIcon = QIcon(":/data/filesave.svg");
      fileSaveAsIcon = QIcon(":/data/filesaveas.svg");
      exitIcon       = QIcon(":/data/exit.svg");
      viewmagIcon    = QIcon(":/data/viewmag.xpm");
      repeatIcon     = QIcon(":/data/repeat.svg");
      noteEntryIcon  = QIcon(":/data/noteentry.svg");
      grace4Icon     = QIcon(":/data/grace4.svg");
      grace16Icon    = QIcon(":/data/grace16.svg");
      grace32Icon    = QIcon(":/data/grace32.svg");
      }

