//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2007 Werner Schweer (ws@seh.de)
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

#include "mscore.h"
#include "icons.h"

//---------------------------------------------------------
//    initial list of shortcuts
//---------------------------------------------------------

Shortcut MuseScore::sc[] = {
      Shortcut(
         "file-open",
         QT_TR_NOOP("file open"),
         Qt::CTRL+Qt::Key_O,
         Qt::WindowShortcut,
         QT_TR_NOOP("Open Score"),
         QT_TR_NOOP("Load Score from File"),
         &fileOpenIcon
         ),
      Shortcut(
         "file-save",
         QT_TR_NOOP("file save"),
         Qt::CTRL+Qt::Key_S,
         Qt::WindowShortcut,
         QT_TR_NOOP("Save Score"),
         QT_TR_NOOP("Save Score to File"),
         &fileSaveIcon
         ),
      Shortcut(
         "file-save-as",
         QT_TR_NOOP("file save as"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Save Score As"),
         QT_TR_NOOP("Save Score to named File"),
         &fileSaveIcon
         ),
      Shortcut(
         "file-close",
         QT_TR_NOOP("file close"),
         Qt::CTRL+Qt::Key_W,
         Qt::WindowShortcut,
         QT_TR_NOOP("Close Score"),
         QT_TR_NOOP("Close Current Score"),
         &fileSaveIcon
         ),
      Shortcut(
         "file-new",
         QT_TR_NOOP("file new"),
         Qt::CTRL+Qt::Key_N,
         Qt::WindowShortcut,
         QT_TR_NOOP("New Score"),
         QT_TR_NOOP("Create New Score"),
         &fileNewIcon
         ),

      Shortcut(
         "print",
         QT_TR_NOOP("print"),
         Qt::CTRL+Qt::Key_P,
         Qt::WindowShortcut,
         QT_TR_NOOP("Print"),
         QT_TR_NOOP("Print Score"),
         &printIcon
         ),
      Shortcut(
         "undo",
         QT_TR_NOOP("undo"),
         Qt::CTRL+Qt::Key_Z,        // QKeySequence::Undo,
         Qt::WindowShortcut,
         QT_TR_NOOP("Undo"),
         QT_TR_NOOP("undo last change"),
         &undoIcon
         ),
      Shortcut(
         "redo",
         QT_TR_NOOP("redo"),
         Qt::CTRL+Qt::SHIFT+Qt::Key_Z,    // QKeySequence::Redo,
         Qt::WindowShortcut,
         QT_TR_NOOP("Redo"),
         QT_TR_NOOP("redo last undo"),
         &redoIcon
         ),
      Shortcut(
         "cut",
         QT_TR_NOOP("cut"),
         Qt::CTRL+Qt::Key_X,        // QKeySequence::Cut,
         Qt::WindowShortcut,
         QT_TR_NOOP("Cut"),
         QT_TR_NOOP(""),
         &cutIcon
         ),
      Shortcut(
         "copy",
         QT_TR_NOOP("copy"),
         Qt::CTRL+Qt::Key_C,        // QKeySequence::Copy,
         Qt::WindowShortcut,
         QT_TR_NOOP("Copy"),
         QT_TR_NOOP(""),
         &copyIcon
         ),
      Shortcut(
         "paste",
         QT_TR_NOOP("paste"),
         Qt::CTRL+Qt::Key_V,        // QKeySequence::Paste,
         Qt::WindowShortcut,
         QT_TR_NOOP("Paste"),
         QT_TR_NOOP(""),
         &pasteIcon
         ),
      Shortcut(
         "instruments",
         QT_TR_NOOP("show instruments dialog"),
         Qt::Key_I,
         Qt::WindowShortcut,
         QT_TR_NOOP("Instruments..."),
         QT_TR_NOOP("Show Instruments Dialog")
         ),
      Shortcut(
         "clefs",
         QT_TR_NOOP("show clefs palette"),
         Qt::Key_Y,
         Qt::WindowShortcut,
         QT_TR_NOOP("Clef..."),
         QT_TR_NOOP("Show Clefs Palette"),
         &clefIcon
         ),
      Shortcut(
         "keys",
         QT_TR_NOOP("show keys palette"),
         Qt::Key_K,
         Qt::WindowShortcut,
         QT_TR_NOOP("Key..."),
         QT_TR_NOOP("Show Keys Palette"),
         &sharpIcon
         ),
      Shortcut(
         "symbols",
         QT_TR_NOOP("show symbols palette"),
         Qt::Key_Z,
         Qt::WindowShortcut,
         QT_TR_NOOP("Symbols..."),
         QT_TR_NOOP("Show Symbols Palette")
         ),
      Shortcut(
         "times",
         QT_TR_NOOP("show time palette"),
         Qt::Key_T,
         Qt::WindowShortcut,
         QT_TR_NOOP("Time..."),
         QT_TR_NOOP("Show Time Palette")
         ),
      Shortcut(
         "dynamics",
         QT_TR_NOOP("show dynamics palette"),
         Qt::Key_L,
         Qt::WindowShortcut,
         QT_TR_NOOP("Dynamics..."),
         QT_TR_NOOP("Show Dynamics Palette")
         ),
      Shortcut(
         "note-input",
         QT_TR_NOOP("note input"),
         Qt::Key_N,
         Qt::WindowShortcut,
         QT_TR_NOOP("Input"),
         QT_TR_NOOP("Note Input Mode")
         ),
      Shortcut(
         "intervall1",
         QT_TR_NOOP("enter prime above"),
         Qt::Key_1,
         Qt::WindowShortcut,
         QT_TR_NOOP("Prime above"),
         QT_TR_NOOP("Enter Prime above")
         ),
      Shortcut(
         "intervall2",
         QT_TR_NOOP("enter sekunde above"),
         Qt::Key_2,
         Qt::WindowShortcut,
         QT_TR_NOOP("Sekunde above"),
         QT_TR_NOOP("Enter Sekunde above")
         ),
      Shortcut(
         "intervall3",
         QT_TR_NOOP("enter terz above"),
         Qt::Key_3,
         Qt::WindowShortcut,
         QT_TR_NOOP("Terz above"),
         QT_TR_NOOP("Enter Terz above")
         ),
      Shortcut(
         "intervall4",
         QT_TR_NOOP("enter quart above"),
         Qt::Key_4,
         Qt::WindowShortcut,
         QT_TR_NOOP("Quart above"),
         QT_TR_NOOP("Enter Quart above")
         ),
      Shortcut(
         "intervall5",
         QT_TR_NOOP("enter quint above"),
         Qt::Key_5,
         Qt::WindowShortcut,
         QT_TR_NOOP("Quint above"),
         QT_TR_NOOP("Enter Quint above")
         ),
      Shortcut(
         "intervall6",
         QT_TR_NOOP("enter sexte above"),
         Qt::Key_6,
         Qt::WindowShortcut,
         QT_TR_NOOP("Sexte above"),
         QT_TR_NOOP("Enter Sexte above")
         ),
      Shortcut(
         "intervall7",
         QT_TR_NOOP("enter septime above"),
         Qt::Key_7,
         Qt::WindowShortcut,
         QT_TR_NOOP("Septime above"),
         QT_TR_NOOP("Enter Septime above")
         ),
      Shortcut(
         "intervall8",
         QT_TR_NOOP("enter octave above"),
         Qt::Key_8,
         Qt::WindowShortcut,
         QT_TR_NOOP("Octave above"),
         QT_TR_NOOP("Enter Octave above")
         ),
      Shortcut(
         "intervall9",
         QT_TR_NOOP("enter None above"),
         Qt::Key_9,
         Qt::WindowShortcut,
         QT_TR_NOOP("None above"),
         QT_TR_NOOP("Enter None above")
         ),
      Shortcut(
         "intervall-2",
         QT_TR_NOOP("enter sekunde below"),
         Qt::SHIFT + Qt::Key_2,
         Qt::WindowShortcut,
         QT_TR_NOOP("Sekunde below"),
         QT_TR_NOOP("Enter Sekunde below")
         ),
      Shortcut(
         "intervall-3",
         QT_TR_NOOP("enter terz below"),
         Qt::SHIFT + Qt::Key_3,
         Qt::WindowShortcut,
         QT_TR_NOOP("Terz below"),
         QT_TR_NOOP("Enter Terz below")
         ),
      Shortcut(
         "intervall-4",
         QT_TR_NOOP("enter quart below"),
         Qt::SHIFT + Qt::Key_4,
         Qt::WindowShortcut,
         QT_TR_NOOP("Quart below"),
         QT_TR_NOOP("Enter Quart below")
         ),
      Shortcut(
         "intervall-5",
         QT_TR_NOOP("enter quint below"),
         Qt::SHIFT + Qt::Key_5,
         Qt::WindowShortcut,
         QT_TR_NOOP("Quint below"),
         QT_TR_NOOP("Enter Quint below")
         ),
      Shortcut(
         "intervall-6",
         QT_TR_NOOP("enter sexte below"),
         Qt::SHIFT + Qt::Key_6,
         Qt::WindowShortcut,
         QT_TR_NOOP("Sexte below"),
         QT_TR_NOOP("Enter Sexte below")
         ),
      Shortcut(
         "intervall-7",
         QT_TR_NOOP("enter septime below"),
         Qt::SHIFT + Qt::Key_7,
         Qt::WindowShortcut,
         QT_TR_NOOP("Septime below"),
         QT_TR_NOOP("Enter Septime below")
         ),
      Shortcut(
         "intervall-8",
         QT_TR_NOOP("enter octave below"),
         Qt::SHIFT + Qt::Key_8,
         Qt::WindowShortcut,
         QT_TR_NOOP("Octave below"),
         QT_TR_NOOP("Enter Octave below")
         ),
      Shortcut(
         "intervall-9",
         QT_TR_NOOP("enter None below"),
         Qt::SHIFT + Qt::Key_9,
         Qt::WindowShortcut,
         QT_TR_NOOP("None below"),
         QT_TR_NOOP("Enter None below")
         ),
      Shortcut(
         "note-a",
         QT_TR_NOOP("enter note a"),
         Qt::Key_A,
         Qt::WindowShortcut,
         QT_TR_NOOP("A"),
         QT_TR_NOOP("Enter Note A")
         ),
      Shortcut(
         "note-b",
         QT_TR_NOOP("enter note b"),
         Qt::Key_B,
         Qt::WindowShortcut,
         QT_TR_NOOP("B"),
         QT_TR_NOOP("Enter Note B")
         ),
      Shortcut(
         "note-c",
         QT_TR_NOOP("enter note c"),
         Qt::Key_C,
         Qt::WindowShortcut,
         QT_TR_NOOP("C"),
         QT_TR_NOOP("Enter Note C")
         ),
      Shortcut(
         "note-d",
         QT_TR_NOOP("enter note d"),
         Qt::Key_D,
         Qt::WindowShortcut,
         QT_TR_NOOP("D"),
         QT_TR_NOOP("Enter Note D")
         ),
      Shortcut(
         "note-e",
         QT_TR_NOOP("enter note e"),
         Qt::Key_E,
         Qt::WindowShortcut,
         QT_TR_NOOP("E"),
         QT_TR_NOOP("Enter Note E")
         ),
      Shortcut(
         "note-f",
         QT_TR_NOOP("enter note f"),
         Qt::Key_F,
         Qt::WindowShortcut,
         QT_TR_NOOP("F"),
         QT_TR_NOOP("Enter Note F")
         ),
      Shortcut(
         "note-g",
         QT_TR_NOOP("enter note g"),
         Qt::Key_G,
         Qt::WindowShortcut,
         QT_TR_NOOP("G"),
         QT_TR_NOOP("Enter Note G")
         ),
      Shortcut(
         "chord-a",
         QT_TR_NOOP("add a to chord"),
         Qt::SHIFT + Qt::Key_A,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add A"),
         QT_TR_NOOP("Add note A to chord")
         ),
      Shortcut(
         "chord-b",
         QT_TR_NOOP("add b to chord"),
         Qt::SHIFT + Qt::Key_B,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add B"),
         QT_TR_NOOP("Add note B to chord")
         ),
      Shortcut(
         "chord-c",
         QT_TR_NOOP("add c to chord"),
         Qt::SHIFT + Qt::Key_C,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add C"),
         QT_TR_NOOP("Add note C to chord")
         ),
      Shortcut(
         "chord-d",
         QT_TR_NOOP("add d to chord"),
         Qt::SHIFT + Qt::Key_D,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add D"),
         QT_TR_NOOP("Add note D to chord")
         ),
      Shortcut(
         "chord-e",
         QT_TR_NOOP("add e to chord"),
         Qt::SHIFT + Qt::Key_E,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add E"),
         QT_TR_NOOP("Add note E to chord")
         ),
      Shortcut(
         "chord-f",
         QT_TR_NOOP("add f to chord"),
         Qt::SHIFT + Qt::Key_F,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add F"),
         QT_TR_NOOP("Add note F to chord")
         ),
      Shortcut(
         "chord-g",
         QT_TR_NOOP("add g to chord"),
         Qt::SHIFT + Qt::Key_G,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add G"),
         QT_TR_NOOP("Add note G to chord")
         ),
      Shortcut(
         "rest",
         QT_TR_NOOP("enter rest"),
         Qt::Key_Space,
         Qt::WindowShortcut,
         QT_TR_NOOP("rest"),
         QT_TR_NOOP("enter rest")
         ),
      Shortcut(
         "stretch+",
         QT_TR_NOOP("more stretch"),
         Qt::Key_Plus,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add more stretch"),
         QT_TR_NOOP("Add more stretch to selected measure")
         ),
      Shortcut(
         "stretch-",
         QT_TR_NOOP("less stretch"),
         Qt::Key_Minus,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add less stretch"),
         QT_TR_NOOP("Add less stretch to selected measure")
         ),
      Shortcut(
         "flip",
         QT_TR_NOOP("flip stem"),
         Qt::Key_X,
         Qt::WindowShortcut,
         QT_TR_NOOP("flip direction"),
         QT_TR_NOOP("flip direction"),
         &flipIcon
         ),
      Shortcut(
         "pitch-up",
         QT_TR_NOOP("up"),
         Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("up"),
         QT_TR_NOOP("up")
         ),
      Shortcut(
         "pitch-up-octave",
         QT_TR_NOOP("up+ctrl"),
         Qt::CTRL + Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("up+ctrl"),
         QT_TR_NOOP("up+ctrl")
         ),
      Shortcut(
         "up-chord",
         QT_TR_NOOP("up note in chord"),
         Qt::ALT+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("up note in chord"),
         QT_TR_NOOP("goto higher pitched note in chord")
         ),
      Shortcut(
         "top-chord",
         QT_TR_NOOP("goto top note in chord"),
         Qt::ALT+Qt::CTRL+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("top note in chord"),
         QT_TR_NOOP("goto top note in chord")
         ),
      Shortcut(
         "move-up",
         QT_TR_NOOP("move up"),
         Qt::SHIFT+Qt::CTRL+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("up+shift+ctrl"),
         QT_TR_NOOP("up+shift+ctrl")
         ),
      Shortcut(
         "pitch-down",
         QT_TR_NOOP("pitch down"),
         Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("down"),
         QT_TR_NOOP("down")
         ),
      Shortcut(
         "pitch-down-octave",
         QT_TR_NOOP("pitch down octave"),
         Qt::CTRL + Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("down+ctrl"),
         QT_TR_NOOP("down+ctrl")
         ),
      Shortcut(
         "down-chord",
         QT_TR_NOOP("down note in chord"),
         Qt::ALT+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("down note in chord"),
         QT_TR_NOOP("goto lower pitched note in chord")
         ),
      Shortcut(
         "bottom-chord",
         QT_TR_NOOP("goto bottom note in chord"),
         Qt::ALT+Qt::CTRL+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("bottom note in chord"),
         QT_TR_NOOP("goto bottom note in chord")
         ),
      Shortcut(
         "move-down",
         QT_TR_NOOP("move down"),
         Qt::SHIFT+Qt::CTRL+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("down+shift+ctrl"),
         QT_TR_NOOP("down+shift+ctrl")
         ),
      Shortcut(
         "prev-chord",
         QT_TR_NOOP("previous chord"),
         Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TR_NOOP("left"),
         QT_TR_NOOP("left")
         ),
      Shortcut(
         "prev-measure",
         QT_TR_NOOP("previous measure"),
         Qt::CTRL+Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TR_NOOP("left+ctrl"),
         QT_TR_NOOP("left+ctrl")
         ),
      Shortcut(
         "next-chord",
         QT_TR_NOOP("next chord"),
         Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TR_NOOP("right"),
         QT_TR_NOOP("right")
         ),
      Shortcut(
         "next-measure",
         QT_TR_NOOP("next measure"),
         Qt::CTRL+Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TR_NOOP("right+ctrl"),
         QT_TR_NOOP("right+ctrl")
         ),
      Shortcut(
         "page-prev",
         QT_TR_NOOP("page-prev"),
         Qt::Key_PageUp,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "page-next",
         QT_TR_NOOP("page-next"),
         Qt::Key_PageDown,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "page-top",
         QT_TR_NOOP("page-top"),
         Qt::Key_Home,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "page-end",
         QT_TR_NOOP("page-end"),
         Qt::Key_End,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "add-tie",
         QT_TR_NOOP("add tie"),
         Qt::SHIFT+Qt::Key_S,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "add-slur",
         QT_TR_NOOP("add slur"),
         Qt::Key_S,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "add-hairpin",
         QT_TR_NOOP("crescendo"),
         Qt::Key_H,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "add-hairpin-reverse",
         QT_TR_NOOP("decrescendo"),
         Qt::SHIFT+Qt::Key_H,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "escape",
         QT_TR_NOOP("ESCAPE"),
         Qt::Key_Escape,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "delete",
         QT_TR_NOOP("delete"),
         Qt::Key_Delete,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "append-measure",
         QT_TR_NOOP("append measure"),
         Qt::CTRL+Qt::Key_B,
         Qt::WindowShortcut,
         QT_TR_NOOP("Measure")
         ),
      Shortcut(
         "duole",
         QT_TR_NOOP("duole"),
         Qt::CTRL+Qt::Key_2,
         Qt::WindowShortcut,
         QT_TR_NOOP("Duole")
         ),
      Shortcut(
         "triole",
         QT_TR_NOOP("triole"),
         Qt::CTRL+Qt::Key_3,
         Qt::WindowShortcut,
         QT_TR_NOOP("Triole")
         ),
      Shortcut(
         "pentole",
         QT_TR_NOOP("pentole"),
         Qt::CTRL+Qt::Key_5,
         Qt::WindowShortcut,
         QT_TR_NOOP("Pentole")
         ),
      Shortcut(
         "pad-note-1",
         QT_TR_NOOP("pad note 1/1"),
         Qt::ALT + Qt::Key_6,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/1"),
         QT_TR_NOOP("1/1"),
         &noteIcon
         ),
      Shortcut(
         "pad-note-2",
         QT_TR_NOOP("pad note 1/2"),
         Qt::ALT + Qt::Key_7,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/2"),
         QT_TR_NOOP("1/2"),
         &note2Icon
         ),
      Shortcut(
         "pad-note-4",
         QT_TR_NOOP("pad note 1/4"),
         Qt::ALT + Qt::Key_1,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/4"),
         QT_TR_NOOP("1/4"),
         &note4Icon
         ),
      Shortcut(
         "pad-note-8",
         QT_TR_NOOP("pad note 1/8"),
         Qt::ALT + Qt::Key_2,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/8"),
         QT_TR_NOOP("1/8"),
         &note8Icon
         ),
      Shortcut(
         "pad-note-16",
         QT_TR_NOOP("pad note 1/16"),
         Qt::ALT + Qt::Key_3,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/16"),
         QT_TR_NOOP("1/16"),
         &note16Icon
         ),
      Shortcut(
         "pad-note-32",
         QT_TR_NOOP("pad note 1/32"),
         Qt::ALT + Qt::Key_4,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/32"),
         QT_TR_NOOP("1/32"),
         &note32Icon
         ),
      Shortcut(
         "pad-note-64",
         QT_TR_NOOP("pad note 1/64"),
         Qt::ALT + Qt::Key_5,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/64"),
         QT_TR_NOOP("1/64"),
         &note64Icon
         ),
      Shortcut(
         "pad-dot",
         QT_TR_NOOP("pad dot"),
         Qt::ALT + Qt::Key_8,
         Qt::WindowShortcut,
         QT_TR_NOOP("dot"),
         QT_TR_NOOP("dot"),
         &dotIcon
         ),
      Shortcut(
         "pad-tie",
         QT_TR_NOOP("pad tie"),
         Qt::ALT + Qt::Key_9,
         Qt::WindowShortcut,
         QT_TR_NOOP("tie"),
         QT_TR_NOOP("tie"),
         &plusIcon
         ),
      Shortcut(
         "pad-rest",
         QT_TR_NOOP("pad rest"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("rest"),
         QT_TR_NOOP("rest"),
         &quartrestIcon
         ),
      Shortcut(
         "pad-sharp2",
         QT_TR_NOOP("double sharp"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("double sharp"),
         QT_TR_NOOP("double sharp"),
         &sharpsharpIcon
         ),
      Shortcut(
         "pad-sharp",
         QT_TR_NOOP("sharp"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("sharp"),
         QT_TR_NOOP("sharp"),
         &sharpIcon
         ),
      Shortcut(
         "pad-nat",
         QT_TR_NOOP("natural"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("natural"),
         QT_TR_NOOP("natural"),
         &naturalIcon
         ),
      Shortcut(
         "pad-flat",
         QT_TR_NOOP("flat"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("flat"),
         QT_TR_NOOP("flat"),
         &flatIcon
         ),
      Shortcut(
         "pad-flat2",
         QT_TR_NOOP("double flat"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("double flat"),
         QT_TR_NOOP("double flat"),
         &flatflatIcon
         ),
      Shortcut(
         "voice-1",
         QT_TR_NOOP("voice 1"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("voice 1"),
         QT_TR_NOOP("voice 1"),
         &voiceIcons[0]
         ),
      Shortcut(
         "voice-2",
         QT_TR_NOOP("voice 2"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("voice 2"),
         QT_TR_NOOP("voice 2"),
         &voiceIcons[1]
         ),
      Shortcut(
         "voice-3",
         QT_TR_NOOP("voice 3"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("voice 3"),
         QT_TR_NOOP("voice 3"),
         &voiceIcons[2]
         ),
      Shortcut(
         "voice-4",
         QT_TR_NOOP("voice 4"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("voice 4"),
         QT_TR_NOOP("voice 4"),
         &voiceIcons[3]
         ),
      Shortcut(
         "midi-on",
         QT_TR_NOOP("midi input in"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Enable Midi Input"),
         QT_TR_NOOP("Enable Midi Input"),
         &midiinIcon
         ),
      Shortcut(
         "sound-on",
         QT_TR_NOOP("editing sound on"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Enable sound while editing"),
         QT_TR_NOOP("Enable sound while editing"),
         &speakerIcon
         ),
      Shortcut(
         "rewind",
         QT_TR_NOOP("player rewind"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Rewind"),
         QT_TR_NOOP("rewind to start position"),
         &startIcon
         ),
      Shortcut(
         "stop",
         QT_TR_NOOP("player stop"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Stop"),
         QT_TR_NOOP("stop sequencer"),
         &stopIcon
         ),
      Shortcut(
         "play",
         QT_TR_NOOP("plaser play"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Play"),
         QT_TR_NOOP("start sequencer play"),
         &playIcon
         ),
      Shortcut(
         "beam-start",
         QT_TR_NOOP("beam start"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("beam start"),
         QT_TR_NOOP("beam start"),
         &sbeamIcon
         ),
      Shortcut(
         "beam-mid",
         QT_TR_NOOP("beam mid"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("beam mid"),
         QT_TR_NOOP("beam mid"),
         &mbeamIcon
         ),
      Shortcut(
         "no-beam",
         QT_TR_NOOP("no beam"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("no beam"),
         QT_TR_NOOP("no beam"),
         &nbeamIcon
         ),
      Shortcut(
         "beam32",
         QT_TR_NOOP("beam 32"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("beam 32"),
         QT_TR_NOOP("beam 32"),
         &beam32Icon
         ),
      Shortcut(
         "toggle-pad",
         QT_TR_NOOP("Pad"),
         Qt::Key_F10,
         Qt::ApplicationShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "toggle-playpanel",
         QT_TR_NOOP("Play Panel"),
         Qt::Key_F11,
         Qt::ApplicationShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "toggle-navigator",
         QT_TR_NOOP("Navigator"),
         Qt::Key_F12,
         Qt::ApplicationShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "toggle-transport",
         QT_TR_NOOP("Transport Toolbar"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "toggle-noteinput",
         QT_TR_NOOP("Note Input Toolbar"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         "toggle-statusbar",
         QT_TR_NOOP("Status Bar"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),

      Shortcut(
         "export-midi",
         QT_TR_NOOP("Export Midi"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Export Midi"),
         QT_TR_NOOP("Export Midi"),
         &fileSaveIcon
         ),
      Shortcut(
         "export-xml",
         QT_TR_NOOP("Export MusicXML"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Export MusicXML"),
         QT_TR_NOOP("Export MusicXML"),
         &fileSaveIcon
         ),
      Shortcut(
         "import-midi",
         QT_TR_NOOP("Import Midi"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Import Midi"),
         QT_TR_NOOP("Import Midi"),
         &fileOpenIcon
         ),
      Shortcut(
         "import-xml",
         QT_TR_NOOP("Import MusicXML"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Import MusicXML"),
         QT_TR_NOOP("Import MusicXML"),
         &fileOpenIcon
         ),
      Shortcut(
         "quit",
         QT_TR_NOOP("Quit"),
         Qt::CTRL + Qt::Key_Q,
         Qt::WindowShortcut,
         QT_TR_NOOP("Quit"),
         QT_TR_NOOP("Quit"),
         &exitIcon
         ),
      Shortcut(
         "mag",
         QT_TR_NOOP("Mag"),
         Qt::CTRL + Qt::Key_Q,
         Qt::WindowShortcut,
         QT_TR_NOOP("Mag"),
         QT_TR_NOOP("Zoom Canvas"),
         &viewmagIcon
         ),
      Shortcut(0, 0, 0),
      };

