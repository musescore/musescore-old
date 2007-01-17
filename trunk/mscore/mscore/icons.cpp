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

//---------------------------------------------------------
//   symPixmap
//---------------------------------------------------------

static QIcon symIcon(QChar code, int size = 20)
      {
      int width  = ICON_WIDTH;
      int height = ICON_HEIGHT;

      TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
      QFont f(s->family);
      f.setPixelSize(size);

      QFontMetricsF fm(f);
      QRectF bb(fm.boundingRect(code));

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
      painter.drawText(QPointF(x, y), code);
      painter.end();
      return QIcon(QPixmap::fromImage(image));
      }

enum {
      wholehead_Sym       = 0xe11b,
      halfhead_Sym       = 0xe11c,
      note4_Sym          = 0xe0fc,
      note8_Sym          = 0xe0f8,
      note16_Sym         = 0xe0f9,
      note32_Sym         = 0xe0fa,
      note64_Sym         = 0xe0fb,
      natural_Sym        = 0xe111,
      sharp_Sym          = 0xe10e,
      sharpsharp_Sym     = 0xe116,
      flat_Sym           = 0xe112,
      flatflat_Sym       = 0xe114,
      quartrest_Sym      = 0xe107,
      dot_Sym            = 0xe119,
      dotdot_Sym         = 0xe0fd,
      sforzatoaccent_Sym = 0xe151,
      staccato_Sym       = 0xe153,
      tenuto_Sym         = 0xe156,
      plus_Sym           = 0x2b,
      trebleclef_Sym     = 0xe18d,
      flip_Sym           = 0xe0fd
      };

//---------------------------------------------------------
//   genIcons
//    create some icons
//---------------------------------------------------------

void genIcons()
      {
      noteIcon           = symIcon(wholehead_Sym);
      note2Icon          = symIcon(halfhead_Sym);
      note4Icon          = symIcon(note4_Sym);
      note8Icon          = symIcon(note8_Sym);
      note16Icon         = symIcon(note16_Sym);
      note32Icon         = symIcon(note32_Sym);
      note64Icon         = symIcon(note64_Sym);
      naturalIcon        = symIcon(natural_Sym);
      sharpIcon          = symIcon(sharp_Sym);
      sharpsharpIcon     = symIcon(sharpsharp_Sym);
      flatIcon           = symIcon(flat_Sym);
      flatflatIcon       = symIcon(flatflat_Sym);
      quartrestIcon      = symIcon(quartrest_Sym);
      dotIcon            = symIcon(dot_Sym);
      dotdotIcon         = symIcon(dotdot_Sym);
      sforzatoaccentIcon = symIcon(sforzatoaccent_Sym);
      staccatoIcon       = symIcon(staccato_Sym);
      tenutoIcon         = symIcon(tenuto_Sym);
      plusIcon           = symIcon(plus_Sym);
      clefIcon           = symIcon(trebleclef_Sym, 14);

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
      }

