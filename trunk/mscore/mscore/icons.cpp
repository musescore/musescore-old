//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: icons.cpp,v 1.16 2006/03/02 17:08:34 wschweer Exp $
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

#include "globals.h"
#include "icons.h"
#include "score.h"
#include "painter.h"
#include "style.h"
#include "preferences.h"
#include "sym.h"

#include "data/undo.xpm"
#include "data/redo.xpm"
#include "data/editcut.xpm"
#include "data/editcopy.xpm"
#include "data/editpaste.xpm"
#include "data/fileprint.xpm"
#include "data/midiin.xpm"
#include "data/speaker.xpm"

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
QIcon midiinIcon, speakerIcon, startIcon, stopIcon, playIcon;
QIcon sbeamIcon, mbeamIcon, nbeamIcon, beam32Icon;
QIcon fileOpenIcon, fileNewIcon, fileSaveIcon;
QIcon exitIcon, viewmagIcon;

//---------------------------------------------------------
//   symPixmap
//---------------------------------------------------------

QIcon symIcon(const SymCode& sc, int size, int width, int height)
      {
      TextStyle* s = &textStyles[sc.style];
      QFont f(s->family);
      f.setPixelSize(size);

      QFontMetricsF fm(f);
      QRectF bb(fm.boundingRect(sc.code));

      qreal w   = bb.width();
      qreal h   = bb.height();
      qreal x   = (width - w) / 2;
      qreal y   = ((height - h) / 2) - bb.y();

      QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
      image.fill(qRgba(0, 0, 0, 0));

      Painter painter(&image);
      painter.setFont(f);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      painter.setPen(QPen(QColor(0, 0, 0, 255)));
      painter.drawText(QPointF(x, y), sc.code);
      painter.end();
      return QIcon(QPixmap::fromImage(image));
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
      noteIcon           = symIcon(wholehead_Sym);
      note2Icon          = symIcon(note2_Sym);
      note4Icon          = symIcon(note4_Sym);
      note8Icon          = symIcon(note8_Sym);
      note16Icon         = symIcon(note16_Sym);
      note32Icon         = symIcon(note32_Sym);
      note64Icon         = symIcon(note64_Sym);
      naturalIcon        = symIcon(natural_Sym, 30);
      sharpIcon          = symIcon(sharp_Sym, 30);
      sharpsharpIcon     = symIcon(sharpsharp_Sym, 30);
      flatIcon           = symIcon(flat_Sym, 30);
      flatflatIcon       = symIcon(flatflat_Sym, 30);
      quartrestIcon      = symIcon(quartrest_Sym, 30);
      dotIcon            = symIcon(dot_Sym, 30);
      dotdotIcon         = symIcon(dotdot_Sym, 30);
      sforzatoaccentIcon = symIcon(sforzatoaccent_Sym);
      staccatoIcon       = symIcon(staccato_Sym);
      tenutoIcon         = symIcon(tenuto_Sym);
      plusIcon           = symIcon(plus_Sym, 30);
      clefIcon           = symIcon(trebleclef_Sym, 17);

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
      flipIcon = symIcon(flip_Sym);

      undoIcon.addPixmap(QPixmap(undo_xpm));
      redoIcon.addPixmap(QPixmap(redo_xpm));
      cutIcon.addPixmap(QPixmap(editcut_xpm));
      copyIcon.addPixmap(QPixmap(editcopy_xpm));
      pasteIcon.addPixmap(QPixmap(editpaste_xpm));
      printIcon.addPixmap(QPixmap(fileprint_xpm));

      midiinIcon.addPixmap(QPixmap(midiin_xpm));
      speakerIcon.addPixmap(QPixmap(speaker_xpm));
      startIcon    = QIcon(":/data/start.svg");
      stopIcon     = QIcon(":/data/stop.svg");
      playIcon     = QIcon(":/data/play.svg");
      sbeamIcon    = QIcon(":/data/sbeam.xpm");
      mbeamIcon    = QIcon(":/data/mbeam.xpm");
      nbeamIcon    = QIcon(":/data/nbeam.xpm");
      beam32Icon   = QIcon(":/data/beam32.xpm");
      fileOpenIcon = QIcon(":/data/fileopen.xpm");
      fileNewIcon  = QIcon(":/data/filenew.xpm");
      fileSaveIcon = QIcon(":/data/filesave.xpm");
      exitIcon     = QIcon(":/data/exit.svg");
      viewmagIcon  = QIcon(":/data/viewmag.xpm");
      }

