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

QIcon* icons[ICONS];

//---------------------------------------------------------
//   symIcon
//    default size=20
//---------------------------------------------------------

QIcon* symIcon(const Sym& sc, int size, int width, int height)
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
      return new QIcon(image);
      }

//---------------------------------------------------------
//   genIcons
//    create some icons
//---------------------------------------------------------

void genIcons()
      {
      int iw = preferences.noteEntryIconWidth;
      int ih = preferences.noteEntryIconHeight;

      icons[longaUp_ICON]        = new QIcon(":/data/longaUp.svg");
      icons[brevis_ICON]         = new QIcon(":/data/brevis.svg");
      icons[note_ICON]           = new QIcon(":/data/note.svg");
      icons[note2_ICON]          = new QIcon(":/data/note2.svg");
      icons[note4_ICON]          = new QIcon(":/data/note4.svg");
      icons[note8_ICON]          = new QIcon(":/data/note8.svg");
      icons[note16_ICON]         = new QIcon(":/data/note16.svg");
      icons[note32_ICON]         = new QIcon(":/data/note32.svg");
      icons[note64_ICON]         = new QIcon(":/data/note64.svg");
      icons[natural_ICON]        = new QIcon(":/data/natural.svg");
      icons[sharp_ICON]          = new QIcon(":/data/sharp.svg");
      icons[sharpsharp_ICON]     = new QIcon(":/data/sharpsharp.svg");
      icons[flat_ICON]           = new QIcon(":/data/flat.svg");
      icons[flatflat_ICON]       = new QIcon(":/data/flatflat.svg");
      icons[quartrest_ICON]      = new QIcon(":/data/quartrest.svg");
      icons[dot_ICON]            = new QIcon(":/data/dot.svg");
      icons[dotdot_ICON]         = new QIcon(":/data/dotdot.svg");
      
      icons[sforzatoaccent_ICON] = symIcon(symbols[sforzatoaccentSym], 20, iw, ih);
      icons[staccato_ICON]       = symIcon(symbols[staccatoSym], 20, iw, ih);
      icons[tenuto_ICON]         = symIcon(symbols[tenutoSym], 20, iw, ih);
      icons[plus_ICON]           = symIcon(symbols[plusSym], 30, iw, ih);
      icons[clef_ICON]           = symIcon(symbols[trebleclefSym], 17, iw, ih);
      icons[staccato_ICON]       = symIcon(symbols[dotSym], 30, iw, ih);   //":/data/staccato.svg");

      icons[acciaccatura_ICON]   = new QIcon(":/data/acciaccatura.svg");
      icons[appoggiatura_ICON]   = new QIcon(":/data/appoggiatura.svg");

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

      icons[flip_ICON]    = new QIcon(":/data/flip.svg");
      icons[cut_ICON]     = new QIcon(QPixmap(editcut_xpm));
      icons[copy_ICON]    = new QIcon(QPixmap(editcopy_xpm));
      icons[paste_ICON]   = new QIcon(QPixmap(editpaste_xpm));
      icons[print_ICON]   = new QIcon(QPixmap(fileprint_xpm));

      icons[undo_ICON]       = new QIcon(":/data/undo.svg");
      icons[redo_ICON]       = new QIcon(":/data/redo.svg");
      icons[midiin_ICON]     = new QIcon(":/data/midiin.svg");
      icons[speaker_ICON]    = new QIcon(":/data/speaker.svg");
      icons[start_ICON]      = new QIcon(":/data/start.svg");
      icons[play_ICON]       = new QIcon(":/data/play.svg");
      icons[sbeam_ICON]      = new QIcon(":/data/sbeam.svg");
      icons[mbeam_ICON]      = new QIcon(":/data/mbeam.svg");
      icons[nbeam_ICON]      = new QIcon(":/data/nbeam.svg");
      icons[beam32_ICON]     = new QIcon(":/data/beam32.svg");
      icons[abeam_ICON]      = new QIcon(":/data/abeam.svg");
      icons[fileOpen_ICON]   = new QIcon(":/data/fileopen.svg");
      icons[fileNew_ICON]    = new QIcon(":/data/filenew.svg");
      icons[fileSave_ICON]   = new QIcon(":/data/filesave.svg");
      icons[fileSaveAs_ICON] = new QIcon(":/data/filesaveas.svg");
      icons[exit_ICON]       = new QIcon(":/data/exit.svg");
      icons[viewmag_ICON]    = new QIcon(":/data/viewmag.xpm");
      icons[repeat_ICON]     = new QIcon(":/data/repeat.svg");
      icons[noteEntry_ICON]  = new QIcon(":/data/noteentry.svg");
      icons[grace4_ICON]     = new QIcon(":/data/grace4.svg");
      icons[grace16_ICON]    = new QIcon(":/data/grace16.svg");
      icons[grace32_ICON]    = new QIcon(":/data/grace32.svg");
      icons[keys_ICON]       = new QIcon(":/data/keyboard.svg");
      icons[tie_ICON]        = new QIcon(":/data/tie.svg");
      icons[window_ICON]     = new QIcon(":/data/mscore.xpm");
      icons[community_ICON]     = new QIcon(":/data/community.svg");
      }

