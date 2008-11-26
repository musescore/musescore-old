//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: icons.h,v 1.8 2006/03/02 17:08:34 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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
extern QIcon brevisIcon;
extern QIcon noteIcon;
extern QIcon note2Icon;
extern QIcon note4Icon;
extern QIcon note8Icon;
extern QIcon note16Icon;
extern QIcon note32Icon;
extern QIcon note64Icon;

extern QIcon naturalIcon;
extern QIcon sharpIcon;
extern QIcon sharpsharpIcon;
extern QIcon flatIcon;
extern QIcon flatflatIcon;
extern QIcon staccatoIcon;
extern QIcon quartrestIcon;
extern QIcon dotIcon;
extern QIcon dotdotIcon;
extern QIcon sforzatoaccentIcon;
extern QIcon staccatoIcon;
extern QIcon tenutoIcon;
extern QIcon plusIcon;
extern QIcon flipIcon;
extern QIcon voiceIcons[];

extern QIcon undoIcon, redoIcon, cutIcon, copyIcon, pasteIcon;
extern QIcon printIcon, clefIcon;
extern QIcon midiinIcon, speakerIcon, startIcon, playIcon, pauseIcon, repeatIcon;
extern QIcon sbeamIcon, mbeamIcon, nbeamIcon, beam32Icon, abeamIcon;
extern QIcon fileOpenIcon, fileNewIcon, fileSaveIcon, fileSaveAsIcon;
extern QIcon exitIcon, viewmagIcon;
extern QIcon windowIcon;

extern QIcon acciaccaturaIcon, appoggiaturaIcon;
extern QIcon grace4Icon, grace16Icon, grace32Icon;

class Sym;

static const int ICON_HEIGHT = 24;
static const int ICON_WIDTH  = 16;

extern QIcon symIcon(const Sym&, int size, int width, int height);
extern QIcon noteEntryIcon;

#endif

