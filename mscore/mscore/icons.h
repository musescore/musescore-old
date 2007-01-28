//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: icons.h,v 1.8 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __ICONS_H__
#define __ICONS_H__

extern void genIcons();
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
extern QIcon midiinIcon, speakerIcon, startIcon, stopIcon, playIcon;
struct SymCode;

static const int ICON_HEIGHT = 24;
static const int ICON_WIDTH  = 16;

extern QIcon symIcon(const SymCode&, int size=20, int width=ICON_WIDTH, int height=ICON_HEIGHT);

#endif

