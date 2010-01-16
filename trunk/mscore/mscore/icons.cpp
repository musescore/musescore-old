//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

QIcon longaUpIcon;
QIcon brevisIcon;
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
QIcon midiinIcon, speakerIcon, startIcon, playIcon, repeatIcon;
QIcon sbeamIcon, mbeamIcon, nbeamIcon, beam32Icon, abeamIcon;
QIcon fileOpenIcon, fileNewIcon, fileSaveIcon, fileSaveAsIcon;
QIcon exitIcon, viewmagIcon;
QIcon windowIcon;

QIcon acciaccaturaIcon, appoggiaturaIcon;
QIcon grace4Icon, grace16Icon, grace32Icon;
QIcon keysIcon;

//---------------------------------------------------------
//   symIcon
//    default size=20
//---------------------------------------------------------

QIcon symIcon(const Sym& sc, int size, int width, int height)
      {
      double mag  = .6 * (size/20.0) * PDPI / DPI;
      QRectF bb   = sc.bbox(mag);

      qreal w   = bb.width();
      qreal h   = bb.height();
      qreal x   = (width  - w) * .5 - bb.x();
      qreal y   = (height - h) * .5 - bb.y();

      QWidget wi;

      QPixmap image(width, height);
      QColor bg(wi.palette().brush(QPalette::Normal, QPalette::Window).color());

      image.fill(QColor(255, 255, 255, 0));
      QPainter painter(&image);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      painter.setFont(sc.font());

      QPen pen(wi.palette().brush(QPalette::Normal, QPalette::Text).color());

      painter.setPen(pen);
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
      int iw = preferences.noteEntryIconWidth;
      int ih = preferences.noteEntryIconHeight;

      longaUpIcon        = symIcon(symbols[longaupSym], 20, iw, ih);
      brevisIcon         = symIcon(symbols[brevisheadSym], 20, iw, ih);
      noteIcon           = symIcon(symbols[wholeheadSym], 20, iw, ih);
      note2Icon          = symIcon(symbols[note2Sym], 20, iw, ih);
      note4Icon          = symIcon(symbols[note4Sym], 20, iw, ih);
      note8Icon          = symIcon(symbols[note8Sym], 20, iw, ih);
      note16Icon         = symIcon(symbols[note16Sym], 20, iw, ih);
      note32Icon         = symIcon(symbols[note32Sym], 20, iw, ih);
      note64Icon         = symIcon(symbols[note64Sym], 20, iw, ih);
      naturalIcon        = symIcon(symbols[naturalSym], 30, iw, ih);
      sharpIcon          = symIcon(symbols[sharpSym], 30, iw, ih);
      sharpsharpIcon     = symIcon(symbols[sharpsharpSym], 30, iw, ih);
      flatIcon           = symIcon(symbols[flatSym], 30, iw, ih);
      flatflatIcon       = symIcon(symbols[flatflatSym], 30, iw, ih);
      quartrestIcon      = symIcon(symbols[rest4Sym], 30, iw, ih);
      dotIcon            = symIcon(symbols[dotSym], 30, iw, ih);
      dotdotIcon         = symIcon(symbols[dotdotSym], 30, iw, ih);
      sforzatoaccentIcon = symIcon(symbols[sforzatoaccentSym], 20, iw, ih);
      staccatoIcon       = symIcon(symbols[staccatoSym], 20, iw, ih);
      tenutoIcon         = symIcon(symbols[tenutoSym], 20, iw, ih);
      plusIcon           = symIcon(symbols[plusSym], 30, iw, ih);
      clefIcon           = symIcon(symbols[trebleclefSym], 17, iw, ih);
      staccatoIcon       = symIcon(symbols[dotSym], 30, iw, ih);   //":/data/staccato.svg");

      acciaccaturaIcon   = QIcon(":/data/acciaccatura.svg");
      appoggiaturaIcon   = QIcon(":/data/appoggiatura.svg");

      static const char* vtext[VOICES] = { "1","2","3","4" };

      for (int i = 0; i < VOICES; ++i) {
            QPixmap image(iw, ih);
            QColor c(preferences.selectColor[i].light(180));
            image.fill(c);
            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, iw, ih), Qt::AlignCenter, vtext[i]);
            painter.end();
            voiceIcons[i].addPixmap(image);

            painter.begin(&image);
            c = QColor(preferences.selectColor[i].light(140));
            painter.fillRect(0, 0, iw, ih, c);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, iw, ih), Qt::AlignCenter, vtext[i]);
            painter.end();
            voiceIcons[i].addPixmap(image, QIcon::Normal, QIcon::On);
            }
      flipIcon = QIcon(":/data/flip.svg");

      cutIcon.addPixmap(QPixmap(editcut_xpm));
      copyIcon.addPixmap(QPixmap(editcopy_xpm));
      pasteIcon.addPixmap(QPixmap(editpaste_xpm));
      printIcon.addPixmap(QPixmap(fileprint_xpm));

      undoIcon       = QIcon(":/data/undo.svg");
      redoIcon       = QIcon(":/data/redo.svg");
      midiinIcon     = QIcon(":/data/midiin.svg");
      speakerIcon    = QIcon(":/data/speaker.svg");
      startIcon      = QIcon(":/data/start.svg");
      playIcon       = QIcon(":/data/play.svg");
      sbeamIcon      = QIcon(":/data/sbeam.svg");
      mbeamIcon      = QIcon(":/data/mbeam.svg");
      nbeamIcon      = QIcon(":/data/nbeam.svg");
      beam32Icon     = QIcon(":/data/beam32.svg");
      abeamIcon      = QIcon(":/data/abeam.svg");
      fileOpenIcon   = QIcon(":/data/fileopen.svg");
      fileNewIcon    = QIcon(":/data/filenew.svg");
      fileSaveIcon   = QIcon(":/data/filesave.svg");
      fileSaveAsIcon = QIcon(":/data/filesaveas.svg");
      exitIcon       = QIcon(":/data/exit.svg");
      viewmagIcon    = QIcon(":/data/viewmag.xpm");
      repeatIcon     = QIcon(":/data/repeat.svg");
      noteEntryIcon  = QIcon(":/data/noteentry.svg");
      grace4Icon     = QIcon(":/data/grace4.svg");
      grace16Icon    = QIcon(":/data/grace16.svg");
      grace32Icon    = QIcon(":/data/grace32.svg");
      keysIcon       = QIcon(":/data/keyboard.svg");
      }

