//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __ICONS_H__
#define __ICONS_H__

enum {
      ICON_ACCIACCATURA, ICON_APPOGGIATURA, ICON_GRACE4, ICON_GRACE16, ICON_GRACE32,
      ICON_SBEAM, ICON_MBEAM, ICON_NBEAM, ICON_BEAM32, ICON_AUTOBEAM
      };

extern void genIcons();

enum { longaUp_ICON, brevis_ICON, note_ICON, note2_ICON, note4_ICON, note8_ICON, note16_ICON,
      note32_ICON, note64_ICON, natural_ICON, sharp_ICON, sharpsharp_ICON, flat_ICON, flatflat_ICON,
      staccato_ICON, quartrest_ICON, dot_ICON, dotdot_ICON, sforzatoaccent_ICON,
      tenuto_ICON, plus_ICON, flip_ICON, voice1_ICON, voice2_ICON, voice3_ICON, voice4_ICON,
      undo_ICON, redo_ICON, cut_ICON, copy_ICON, paste_ICON, print_ICON, clef_ICON,
      midiin_ICON, speaker_ICON, start_ICON, play_ICON, repeat_ICON, sbeam_ICON, mbeam_ICON,
      nbeam_ICON, beam32_ICON, abeam_ICON, fileOpen_ICON, fileNew_ICON, fileSave_ICON,
      fileSaveAs_ICON, exit_ICON, viewmag_ICON, window_ICON, acciaccatura_ICON, appoggiatura_ICON,
      grace4_ICON, grace16_ICON, grace32_ICON, noteEntry_ICON, keys_ICON, tie_ICON, community_ICON,
      ICONS
      };
extern QIcon* icons[ICONS];

class Sym;

static const int ICON_HEIGHT = 30;
static const int ICON_WIDTH  = 24;

extern QIcon* symIcon(const Sym&, int size, int width, int height);

#endif

