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

extern QString iconPath;
QIcon* icons[ICONS];

//---------------------------------------------------------
//   genIcons
//    create some icons
//---------------------------------------------------------

static const char* iconNames[] = {
      "longaUp.svg",
      "brevis.svg",
      "note.svg",
      "note2.svg",
      "note4.svg",
      "note8.svg",
      "note16.svg",
      "note32.svg",
      "note64.svg",
      "natural.svg",
      "sharp.svg",
      "sharpsharp.svg",
      "flat.svg",
      "flatflat.svg",
      "staccato.svg",
      "quartrest.svg",
      "dot.svg",
      "dotdot.svg",
      "sforzatoaccent.svg",
      "tenuto.svg",
      "plus.svg",
      "flip.svg",
      "undo.svg",
      "redo.svg",
      "editcut.svg",
      "editcopy.svg",
      "editpaste.svg",
      "fileprint.svg",
      "clef.svg",
      "midiin.svg",
      "speaker.svg",
      "start.svg",
      "play.svg",
      "repeat.svg",
      "sbeam.svg",
      "mbeam.svg",
      "nbeam.svg",
      "beam32.svg",
      "beam64.svg",
      "abeam.svg",
      "fbeam1.svg",
      "fbeam2.svg",
      "fileopen.svg",
      "filenew.svg",
      "filesave.svg",
      "filesaveas.svg",
      "exit.svg",
      "viewmag.svg",
      "mscore.xpm",
      "acciaccatura.svg",
      "appoggiatura.svg",
      "grace4.svg",
      "grace16.svg",
      "grace32.svg",
      "noteentry.svg",
      "keyboard.svg",
      "tie.svg",
      "text_bold.svg",
      "text_italic.svg",
      "text_under.svg",
      "text_left.svg",
      "text_center.svg",
      "text_right.svg",
      "text_top.svg",
      "text_bottom.svg",
      "text_vcenter.svg",
      "superscript.svg",
      "subscript.svg",
      "fotomode.svg"
      };

void genIcons()
      {
      for (int i = 0; i < voice1_ICON; ++i) {
            icons[i] = new QIcon(iconPath + iconNames[i]);
//            if (icons[i]->isNull() || icons[i]->pixmap(12).isNull()) {
//                  printf("cannot load Icon <%s>\n", iconNames[i]);
//                  }
            }

      static const char* vtext[VOICES] = { "1","2","3","4" };
      int iw = 16;
      int ih = 24;
      for (int i = 0; i < VOICES; ++i) {
            icons[voice1_ICON + i] = new QIcon;
            QPixmap image(iw, ih);
            QColor c(preferences.selectColor[i].light(180));
            image.fill(c);
            QPainter painter(&image);
            painter.setFont(QFont("FreeSans", 8));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, iw, ih), Qt::AlignCenter, vtext[i]);
            painter.end();
            icons[voice1_ICON +i]->addPixmap(image);

            painter.begin(&image);
            c = QColor(preferences.selectColor[i].light(140));
            painter.fillRect(0, 0, iw, ih, c);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, iw, ih), Qt::AlignCenter, vtext[i]);
            painter.end();
            icons[voice1_ICON + i]->addPixmap(image, QIcon::Normal, QIcon::On);
            }
      }
