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

void genIcons()
      {
      int iw = preferences.noteEntryIconWidth;
      int ih = preferences.noteEntryIconHeight;

      icons[longaUp_ICON]        = new QIcon(iconPath + "longaUp.svg");
      icons[brevis_ICON]         = new QIcon(iconPath + "brevis.svg");
      icons[note_ICON]           = new QIcon(iconPath + "note.svg");
      icons[note2_ICON]          = new QIcon(iconPath + "note2.svg");
      icons[note4_ICON]          = new QIcon(iconPath + "note4.svg");
      icons[note8_ICON]          = new QIcon(iconPath + "note8.svg");
      icons[note16_ICON]         = new QIcon(iconPath + "note16.svg");
      icons[note32_ICON]         = new QIcon(iconPath + "note32.svg");
      icons[note64_ICON]         = new QIcon(iconPath + "note64.svg");
      icons[natural_ICON]        = new QIcon(iconPath + "natural.svg");
      icons[sharp_ICON]          = new QIcon(iconPath + "sharp.svg");
      icons[sharpsharp_ICON]     = new QIcon(iconPath + "sharpsharp.svg");
      icons[flat_ICON]           = new QIcon(iconPath + "flat.svg");
      icons[flatflat_ICON]       = new QIcon(iconPath + "flatflat.svg");
      icons[quartrest_ICON]      = new QIcon(iconPath + "quartrest.svg");
      icons[dot_ICON]            = new QIcon(iconPath + "dot.svg");
      icons[dotdot_ICON]         = new QIcon(iconPath + "dotdot.svg");
      icons[sforzatoaccent_ICON] = new QIcon(iconPath + "sforzatoaccent.svg");
      icons[staccato_ICON]       = new QIcon(iconPath + "staccato.svg");
      icons[tenuto_ICON]         = new QIcon(iconPath + "tenuto.svg");
      icons[plus_ICON]           = new QIcon(iconPath + "plus.svg");
      icons[clef_ICON]           = new QIcon(iconPath + "clef.svg");
      icons[staccato_ICON]       = new QIcon(iconPath + "staccato.svg");

      static const char* vtext[VOICES] = { "1","2","3","4" };

      for (int i = 0; i < VOICES; ++i) {
            icons[voice1_ICON + i] = new QIcon;
            QPixmap image(iw, ih);
            QColor c(preferences.selectColor[i].light(180));
            image.fill(c);
            QPainter painter(&image);
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

      icons[cut_ICON]        = new QIcon(iconPath + "editcut.svg");
      icons[copy_ICON]       = new QIcon(iconPath + "editcopy.svg");
      icons[paste_ICON]      = new QIcon(iconPath + "editpaste.svg");
      icons[print_ICON]      = new QIcon(iconPath + "fileprint.svg");

      icons[acciaccatura_ICON] = new QIcon(iconPath + "acciaccatura.svg");
      icons[appoggiatura_ICON] = new QIcon(iconPath + "appoggiatura.svg");
      icons[flip_ICON]       = new QIcon(iconPath + "flip.svg");
      icons[undo_ICON]       = new QIcon(iconPath + "undo.svg");
      icons[redo_ICON]       = new QIcon(iconPath + "redo.svg");
      icons[midiin_ICON]     = new QIcon(iconPath + "midiin.svg");
      icons[speaker_ICON]    = new QIcon(iconPath + "speaker.svg");
      icons[start_ICON]      = new QIcon(iconPath + "start.svg");
      icons[play_ICON]       = new QIcon(iconPath + "play.svg");
      icons[sbeam_ICON]      = new QIcon(iconPath + "sbeam.svg");
      icons[mbeam_ICON]      = new QIcon(iconPath + "mbeam.svg");
      icons[nbeam_ICON]      = new QIcon(iconPath + "nbeam.svg");
      icons[beam32_ICON]     = new QIcon(iconPath + "beam32.svg");
      icons[beam64_ICON]     = new QIcon(iconPath + "beam64.svg");
      icons[abeam_ICON]      = new QIcon(iconPath + "abeam.svg");
      icons[fbeam1_ICON]     = new QIcon(iconPath + "fbeam1.svg");
      icons[fbeam2_ICON]     = new QIcon(iconPath + "fbeam2.svg");
      icons[fileOpen_ICON]   = new QIcon(iconPath + "fileopen.svg");
      icons[fileNew_ICON]    = new QIcon(iconPath + "filenew.svg");
      icons[fileSave_ICON]   = new QIcon(iconPath + "filesave.svg");
      icons[fileSaveAs_ICON] = new QIcon(iconPath + "filesaveas.svg");
      icons[exit_ICON]       = new QIcon(iconPath + "exit.svg");
      icons[viewmag_ICON]    = new QIcon(iconPath + "viewmag.xpm");
      icons[repeat_ICON]     = new QIcon(iconPath + "repeat.svg");
      icons[noteEntry_ICON]  = new QIcon(iconPath + "noteentry.svg");
      icons[grace4_ICON]     = new QIcon(iconPath + "grace4.svg");
      icons[grace16_ICON]    = new QIcon(iconPath + "grace16.svg");
      icons[grace32_ICON]    = new QIcon(iconPath + "grace32.svg");
      icons[keys_ICON]       = new QIcon(iconPath + "keyboard.svg");
      icons[tie_ICON]        = new QIcon(iconPath + "tie.svg");
      icons[window_ICON]     = new QIcon(iconPath + "mscore.xpm");

      icons[textBold_ICON]      = new QIcon(iconPath + "text_bold.svg");
      icons[textItalic_ICON]    = new QIcon(iconPath + "text_italic.svg");
      icons[textUnderline_ICON] = new QIcon(iconPath + "text_under.svg");
      icons[textLeft_ICON]      = new QIcon(iconPath + "text_left.svg");
      icons[textCenter_ICON]    = new QIcon(iconPath + "text_center.svg");
      icons[textRight_ICON]     = new QIcon(iconPath + "text_right.svg");
      icons[textSuper_ICON]     = new QIcon(iconPath + "superscript.svg");
      icons[textSub_ICON]       = new QIcon(iconPath + "subscript.svg");
      }

