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

QIcon undoIcon, redoIcon, cutIcon, copyIcon, pasteIcon;
QIcon printIcon, clefIcon;
QIcon midiinIcon, speakerIcon, startIcon, playIcon, pauseIcon, repeatIcon;
QIcon sbeamIcon, mbeamIcon, nbeamIcon, beam32Icon;
QIcon fileOpenIcon, fileNewIcon, fileSaveIcon, fileSaveAsIcon;
QIcon exitIcon, viewmagIcon;
QIcon windowIcon;

//---------------------------------------------------------
//   symPixmap
//---------------------------------------------------------

QIcon symIcon(const Sym& sc, int size, int width, int height)
      {
      double iconSpatium = 1.3 * DPMM_DISPLAY;

      double ospatium = _spatium;
      _spatium  = iconSpatium * (size / 20.0);

      QRectF bb = sc.bbox();
      qreal w   = bb.width();
      qreal h   = bb.height();
      qreal x   = (width - w) / 2 - bb.x();
      qreal y   = ((height - h) / 2) - bb.y();

      QPixmap image(width, height);
      image.fill(QColor(255, 255, 255, 0));

      QPainter painter(&image);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      painter.setPen(QPen(QColor(0, 0, 0, 255)));
      sc.draw(painter, x, y);
      painter.end();
      _spatium = ospatium;

      return QIcon(image);
      }

SymCode wholehead_Sym      (0xe103, TEXT_STYLE_DYNAMICS1);
SymCode note2_Sym          (0xe102, TEXT_STYLE_DYNAMICS1);
SymCode note4_Sym          (0xe0fc, TEXT_STYLE_DYNAMICS1);
SymCode note8_Sym          (0xe0f8, TEXT_STYLE_DYNAMICS1);
SymCode note16_Sym         (0xe0f9, TEXT_STYLE_DYNAMICS1);
SymCode note32_Sym         (0xe0fa, TEXT_STYLE_DYNAMICS1);
SymCode note64_Sym         (0xe0fb, TEXT_STYLE_DYNAMICS1);
SymCode natural_Sym        (0xe111, TEXT_STYLE_DYNAMICS1);
SymCode sharp_Sym          (0xe10e, TEXT_STYLE_DYNAMICS1);
SymCode sharpsharp_Sym     (0xe116, TEXT_STYLE_DYNAMICS1);
SymCode flat_Sym           (0xe112, TEXT_STYLE_DYNAMICS1);
SymCode flatflat_Sym       (0xe114, TEXT_STYLE_DYNAMICS1);
SymCode quartrest_Sym      (0xe107, TEXT_STYLE_DYNAMICS1);
SymCode dot_Sym            (0xe0ff, TEXT_STYLE_DYNAMICS1);
SymCode dotdot_Sym         (0xe0fd, TEXT_STYLE_DYNAMICS1);
SymCode sforzatoaccent_Sym (0xe151, TEXT_STYLE_SYMBOL1);
SymCode staccato_Sym       (0xe153, TEXT_STYLE_SYMBOL1);
SymCode tenuto_Sym         (0xe156, TEXT_STYLE_SYMBOL1);
SymCode plus_Sym           (0x2b,   TEXT_STYLE_SYMBOL1);
SymCode trebleclef_Sym     (0xe18d, TEXT_STYLE_DYNAMICS1);
SymCode flip_Sym           (0xe0fd, TEXT_STYLE_DYNAMICS1);

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
      flipIcon = symIcon(symbols[flipSym]);

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
      sbeamIcon    = QIcon(":/data/sbeam.xpm");
      mbeamIcon    = QIcon(":/data/mbeam.xpm");
      nbeamIcon    = QIcon(":/data/nbeam.xpm");
      beam32Icon   = QIcon(":/data/beam32.xpm");
      fileOpenIcon = QIcon(":/data/fileopen.svg");
      fileNewIcon  = QIcon(":/data/filenew.svg");
      fileSaveIcon = QIcon(":/data/filesave.svg");
      fileSaveAsIcon = QIcon(":/data/filesaveas.svg");
      exitIcon     = QIcon(":/data/exit.svg");
      viewmagIcon  = QIcon(":/data/viewmag.xpm");
      repeatIcon   = QIcon(":/data/repeat.svg");
      }

