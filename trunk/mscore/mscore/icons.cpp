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
QIcon printIcon;

//---------------------------------------------------------
//   symPixmap
//---------------------------------------------------------

static QIcon symIcon(int idx)
      {
      Sym sym(symbols[idx]);
      int width  = ICON_WIDTH;
      int height = ICON_HEIGHT;

      TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
      QFont f(s->family);
      f.setPixelSize(24);

      QFontMetricsF fm(f);
      QRectF bb(fm.boundingRect(sym.code()));

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
      painter.drawText(QPointF(x, y), QString(sym.code()));
      painter.end();
      return QIcon(QPixmap::fromImage(image));
      }

//---------------------------------------------------------
//   genIcons
//    create some icons
//---------------------------------------------------------

void genIcons()
      {
      noteIcon           = symIcon(wholeheadSym);
      note2Icon          = symIcon(halfheadSym);
      note4Icon          = symIcon(note4Sym);
      note8Icon          = symIcon(note8Sym);
      note16Icon         = symIcon(note16Sym);
      note32Icon         = symIcon(note32Sym);
      note64Icon         = symIcon(note64Sym);
      naturalIcon        = symIcon(naturalSym);
      sharpIcon          = symIcon(sharpSym);
      sharpsharpIcon     = symIcon(sharpsharpSym);
      flatIcon           = symIcon(flatSym);
      flatflatIcon       = symIcon(flatflatSym);
      quartrestIcon      = symIcon(quartrestSym);
      dotIcon            = symIcon(dotSym);
      dotdotIcon         = symIcon(dotdotSym);
      sforzatoaccentIcon = symIcon(sforzatoaccentSym);
      staccatoIcon       = symIcon(staccatoSym);
      tenutoIcon         = symIcon(tenutoSym);
      plusIcon           = symIcon(plusSym);

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
      flipIcon = symIcon(flipSym);

      undoIcon.addPixmap(QPixmap(undo_xpm));
      redoIcon.addPixmap(QPixmap(redo_xpm));
      cutIcon.addPixmap(QPixmap(editcut_xpm));
      copyIcon.addPixmap(QPixmap(editcopy_xpm));
      pasteIcon.addPixmap(QPixmap(editpaste_xpm));
      printIcon.addPixmap(QPixmap(fileprint_xpm));
      }

