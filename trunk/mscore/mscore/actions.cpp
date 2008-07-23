//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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

#include "mscore.h"
#include "icons.h"

//---------------------------------------------------------
//    initial list of shortcuts
//---------------------------------------------------------

Shortcut MuseScore::sc[] = {
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-open",
         QT_TR_NOOP("file open"),
         Qt::CTRL+Qt::Key_O,
         Qt::WindowShortcut,
         QT_TR_NOOP("Open"),
         QT_TR_NOOP("Load Score from File"),
         &fileOpenIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-save",
         QT_TR_NOOP("file save"),
         Qt::CTRL+Qt::Key_S,
         Qt::WindowShortcut,
         QT_TR_NOOP("Save"),
         QT_TR_NOOP("Save Score to File"),
         &fileSaveIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-save-as",
         QT_TR_NOOP("file save as"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Save As"),
         QT_TR_NOOP("Save Score to named File"),
         &fileSaveIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-close",
         QT_TR_NOOP("file close"),
         Qt::CTRL+Qt::Key_W,
         Qt::WindowShortcut,
         QT_TR_NOOP("Close"),
         QT_TR_NOOP("Close Current Score"),
         &fileSaveIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-new",
         QT_TR_NOOP("file new"),
         Qt::CTRL+Qt::Key_N,
         Qt::WindowShortcut,
         QT_TR_NOOP("New"),
         QT_TR_NOOP("Create new score"),
         &fileNewIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "print",
         QT_TR_NOOP("print"),
         Qt::CTRL+Qt::Key_P,
         Qt::WindowShortcut,
         QT_TR_NOOP("Print"),
         QT_TR_NOOP("Print Score"),
         &printIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "undo",
         QT_TR_NOOP("undo"),
         Qt::CTRL+Qt::Key_Z,        // QKeySequence::Undo,
         Qt::WindowShortcut,
         QT_TR_NOOP("Undo"),
         QT_TR_NOOP("undo last change"),
         &undoIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "redo",
         QT_TR_NOOP("redo"),
         Qt::CTRL+Qt::SHIFT+Qt::Key_Z,    // QKeySequence::Redo,
         Qt::WindowShortcut,
         QT_TR_NOOP("Redo"),
         QT_TR_NOOP("redo last undo"),
         &redoIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "cut",
         QT_TR_NOOP("cut"),
         Qt::CTRL+Qt::Key_X,        // QKeySequence::Cut,
         Qt::WindowShortcut,
         QT_TR_NOOP("Cut"),
         QT_TR_NOOP(""),
         &cutIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "copy",
         QT_TR_NOOP("copy"),
         Qt::CTRL+Qt::Key_C,        // QKeySequence::Copy,
         Qt::WindowShortcut,
         QT_TR_NOOP("Copy"),
         QT_TR_NOOP(""),
         &copyIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "paste",
         QT_TR_NOOP("paste"),
         Qt::CTRL+Qt::Key_V,        // QKeySequence::Paste,
         Qt::WindowShortcut,
         QT_TR_NOOP("Paste"),
         QT_TR_NOOP(""),
         &pasteIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "instruments",
         QT_TR_NOOP("show instruments dialog"),
         Qt::Key_I,
         Qt::WindowShortcut,
         QT_TR_NOOP("Instruments..."),
         QT_TR_NOOP("Show Instruments Dialog")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "clefs",
         QT_TR_NOOP("show clefs palette"),
         Qt::Key_Y,
         Qt::WindowShortcut,
         QT_TR_NOOP("Clef..."),
         QT_TR_NOOP("Show Clefs Palette"),
         &clefIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "keys",
         QT_TR_NOOP("show keys palette"),
         Qt::Key_K,
         Qt::WindowShortcut,
         QT_TR_NOOP("Key..."),
         QT_TR_NOOP("Show Keys Palette"),
         &sharpIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "symbols",
         QT_TR_NOOP("show symbols palette"),
         Qt::Key_Z,
         Qt::WindowShortcut,
         QT_TR_NOOP("Symbols..."),
         QT_TR_NOOP("Show Symbols Palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "times",
         QT_TR_NOOP("show time palette"),
         Qt::Key_T,
         Qt::WindowShortcut,
         QT_TR_NOOP("Time..."),
         QT_TR_NOOP("Show Time Palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "dynamics",
         QT_TR_NOOP("show dynamics palette"),
         Qt::Key_L,
         Qt::WindowShortcut,
         QT_TR_NOOP("Dynamics..."),
         QT_TR_NOOP("Show Dynamics Palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input",
         QT_TR_NOOP("note input"),
         Qt::Key_N,
         Qt::WindowShortcut,
         QT_TR_NOOP("Note Input"),
         QT_TR_NOOP("toggle note input mode N"),
         &noteEntryIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-spell",
         QT_TR_NOOP("pitch spell"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Pitch spell"),
         QT_TR_NOOP("Pitch spell")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval1",
         QT_TR_NOOP("enter unison above"),
         Qt::Key_1,
         Qt::WindowShortcut,
         QT_TR_NOOP("Unison above"),
         QT_TR_NOOP("Enter Unison above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval2",
         QT_TR_NOOP("enter second above"),
         Qt::Key_2,
         Qt::WindowShortcut,
         QT_TR_NOOP("Second above"),
         QT_TR_NOOP("Enter Second above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval3",
         QT_TR_NOOP("enter third above"),
         Qt::Key_3,
         Qt::WindowShortcut,
         QT_TR_NOOP("Third above"),
         QT_TR_NOOP("Enter Third above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval4",
         QT_TR_NOOP("enter fourth above"),
         Qt::Key_4,
         Qt::WindowShortcut,
         QT_TR_NOOP("Fourth above"),
         QT_TR_NOOP("Enter Fourth above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval5",
         QT_TR_NOOP("enter fifth above"),
         Qt::Key_5,
         Qt::WindowShortcut,
         QT_TR_NOOP("Fifth above"),
         QT_TR_NOOP("Enter Fifth above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval6",
         QT_TR_NOOP("enter sixth above"),
         Qt::Key_6,
         Qt::WindowShortcut,
         QT_TR_NOOP("Sixth above"),
         QT_TR_NOOP("Enter Sixth above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval7",
         QT_TR_NOOP("enter seventh above"),
         Qt::Key_7,
         Qt::WindowShortcut,
         QT_TR_NOOP("Seventh above"),
         QT_TR_NOOP("Enter Seventh above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval8",
         QT_TR_NOOP("enter octave above"),
         Qt::Key_8,
         Qt::WindowShortcut,
         QT_TR_NOOP("Octave above"),
         QT_TR_NOOP("Enter Octave above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval9",
         QT_TR_NOOP("enter ninth above"),
         Qt::Key_9,
         Qt::WindowShortcut,
         QT_TR_NOOP("Ninth above"),
         QT_TR_NOOP("Enter Ninth above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval-2",
         QT_TR_NOOP("enter second below"),
         Qt::SHIFT + Qt::Key_2,
         Qt::WindowShortcut,
         QT_TR_NOOP("Second below"),
         QT_TR_NOOP("Enter Second below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval-3",
         QT_TR_NOOP("enter third below"),
         Qt::SHIFT + Qt::Key_3,
         Qt::WindowShortcut,
         QT_TR_NOOP("Third below"),
         QT_TR_NOOP("Enter Third below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval-4",
         QT_TR_NOOP("enter fourth below"),
         Qt::SHIFT + Qt::Key_4,
         Qt::WindowShortcut,
         QT_TR_NOOP("Fourth below"),
         QT_TR_NOOP("Enter Fourth below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval-5",
         QT_TR_NOOP("enter fifth below"),
         Qt::SHIFT + Qt::Key_5,
         Qt::WindowShortcut,
         QT_TR_NOOP("Fifth below"),
         QT_TR_NOOP("Enter Fifth below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval-6",
         QT_TR_NOOP("enter sixth below"),
         Qt::SHIFT + Qt::Key_6,
         Qt::WindowShortcut,
         QT_TR_NOOP("Sixth below"),
         QT_TR_NOOP("Enter Sixth below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval-7",
         QT_TR_NOOP("enter seventh below"),
         Qt::SHIFT + Qt::Key_7,
         Qt::WindowShortcut,
         QT_TR_NOOP("Seventh below"),
         QT_TR_NOOP("Enter Seventh below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval-8",
         QT_TR_NOOP("enter octave below"),
         Qt::SHIFT + Qt::Key_8,
         Qt::WindowShortcut,
         QT_TR_NOOP("Octave below"),
         QT_TR_NOOP("Enter Octave below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "interval-9",
         QT_TR_NOOP("enter ninth below"),
         Qt::SHIFT + Qt::Key_9,
         Qt::WindowShortcut,
         QT_TR_NOOP("Ninth below"),
         QT_TR_NOOP("Enter Ninth below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-a",
         QT_TR_NOOP("enter note a"),
         Qt::Key_A,
         Qt::WindowShortcut,
         QT_TR_NOOP("A"),
         QT_TR_NOOP("Enter Note A")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-b",
         QT_TR_NOOP("enter note b"),
         Qt::Key_B,
         Qt::WindowShortcut,
         QT_TR_NOOP("B"),
         QT_TR_NOOP("Enter Note B")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-c",
         QT_TR_NOOP("enter note c"),
         Qt::Key_C,
         Qt::WindowShortcut,
         QT_TR_NOOP("C"),
         QT_TR_NOOP("Enter Note C")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-d",
         QT_TR_NOOP("enter note d"),
         Qt::Key_D,
         Qt::WindowShortcut,
         QT_TR_NOOP("D"),
         QT_TR_NOOP("Enter Note D")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-e",
         QT_TR_NOOP("enter note e"),
         Qt::Key_E,
         Qt::WindowShortcut,
         QT_TR_NOOP("E"),
         QT_TR_NOOP("Enter Note E")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-f",
         QT_TR_NOOP("enter note f"),
         Qt::Key_F,
         Qt::WindowShortcut,
         QT_TR_NOOP("F"),
         QT_TR_NOOP("Enter Note F")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-g",
         QT_TR_NOOP("enter note g"),
         Qt::Key_G,
         Qt::WindowShortcut,
         QT_TR_NOOP("G"),
         QT_TR_NOOP("Enter Note G")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-a",
         QT_TR_NOOP("add a to chord"),
         Qt::SHIFT + Qt::Key_A,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add A"),
         QT_TR_NOOP("Add note A to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-b",
         QT_TR_NOOP("add b to chord"),
         Qt::SHIFT + Qt::Key_B,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add B"),
         QT_TR_NOOP("Add note B to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-c",
         QT_TR_NOOP("add c to chord"),
         Qt::SHIFT + Qt::Key_C,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add C"),
         QT_TR_NOOP("Add note C to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-d",
         QT_TR_NOOP("add d to chord"),
         Qt::SHIFT + Qt::Key_D,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add D"),
         QT_TR_NOOP("Add note D to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-e",
         QT_TR_NOOP("add e to chord"),
         Qt::SHIFT + Qt::Key_E,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add E"),
         QT_TR_NOOP("Add note E to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-f",
         QT_TR_NOOP("add f to chord"),
         Qt::SHIFT + Qt::Key_F,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add F"),
         QT_TR_NOOP("Add note F to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-g",
         QT_TR_NOOP("add g to chord"),
         Qt::SHIFT + Qt::Key_G,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add G"),
         QT_TR_NOOP("Add note G to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest",
         QT_TR_NOOP("enter rest"),
         Qt::Key_Space,
         Qt::WindowShortcut,
         QT_TR_NOOP("rest"),
         QT_TR_NOOP("enter rest")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "stretch+",
         QT_TR_NOOP("more stretch"),
         Qt::SHIFT + Qt::Key_Plus,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add more stretch"),
         QT_TR_NOOP("Add more stretch to selected measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "stretch-",
         QT_TR_NOOP("less stretch"),
         Qt::SHIFT + Qt::Key_Minus,
         Qt::WindowShortcut,
         QT_TR_NOOP("Add less stretch"),
         QT_TR_NOOP("Add less stretch to selected measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "reset-beammode",
         QT_TR_NOOP("Reset Beam Mode"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Reset Beam Mode"),
         QT_TR_NOOP("Reset Beam Mode of selected measures")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "flip",
         QT_TR_NOOP("flip stem"),
         Qt::Key_X,
         Qt::WindowShortcut,
         QT_TR_NOOP("flip direction"),
         QT_TR_NOOP("flip direction"),
         &flipIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-up",
         QT_TR_NOOP("up"),
         Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("up"),
         QT_TR_NOOP("up")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-up-octave",
         QT_TR_NOOP("up+ctrl"),
         Qt::CTRL + Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("up+ctrl"),
         QT_TR_NOOP("up+ctrl")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "up-chord",
         QT_TR_NOOP("up note in chord"),
         Qt::ALT+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("up note in chord"),
         QT_TR_NOOP("goto higher pitched note in chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "top-chord",
         QT_TR_NOOP("goto top note in chord"),
         Qt::ALT+Qt::CTRL+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("top note in chord"),
         QT_TR_NOOP("goto top note in chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "move-up",
         QT_TR_NOOP("move up"),
         Qt::SHIFT+Qt::CTRL+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TR_NOOP("up+shift+ctrl"),
         QT_TR_NOOP("up+shift+ctrl")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-down",
         QT_TR_NOOP("pitch down"),
         Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("down"),
         QT_TR_NOOP("down")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-down-octave",
         QT_TR_NOOP("pitch down octave"),
         Qt::CTRL + Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("down+ctrl"),
         QT_TR_NOOP("down+ctrl")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "down-chord",
         QT_TR_NOOP("down note in chord"),
         Qt::ALT+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("down note in chord"),
         QT_TR_NOOP("goto lower pitched note in chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "bottom-chord",
         QT_TR_NOOP("goto bottom note in chord"),
         Qt::ALT+Qt::CTRL+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("bottom note in chord"),
         QT_TR_NOOP("goto bottom note in chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "move-down",
         QT_TR_NOOP("move down"),
         Qt::SHIFT+Qt::CTRL+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TR_NOOP("down+shift+ctrl"),
         QT_TR_NOOP("down+shift+ctrl")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-chord",
         QT_TR_NOOP("previous chord"),
         Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TR_NOOP("left"),
         QT_TR_NOOP("left")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-measure",
         QT_TR_NOOP("previous measure"),
         Qt::CTRL+Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TR_NOOP("left+ctrl"),
         QT_TR_NOOP("left+ctrl")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-chord",
         QT_TR_NOOP("next chord"),
         Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TR_NOOP("right"),
         QT_TR_NOOP("right")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-measure",
         QT_TR_NOOP("next measure"),
         Qt::CTRL+Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TR_NOOP("right+ctrl"),
         QT_TR_NOOP("right+ctrl")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-prev",
         QT_TR_NOOP("page-prev"),
         Qt::Key_PageUp,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-next",
         QT_TR_NOOP("page-next"),
         Qt::Key_PageDown,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-top",
         QT_TR_NOOP("page-top"),
         Qt::Key_Home,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-end",
         QT_TR_NOOP("page-end"),
         Qt::Key_End,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-tie",
         QT_TR_NOOP("add tie"),
         Qt::SHIFT+Qt::Key_S,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-slur",
         QT_TR_NOOP("add slur"),
         Qt::Key_S,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-hairpin",
         QT_TR_NOOP("crescendo"),
         Qt::Key_H,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-hairpin-reverse",
         QT_TR_NOOP("decrescendo"),
         Qt::SHIFT+Qt::Key_H,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "escape",
         QT_TR_NOOP("ESCAPE"),
         Qt::Key_Escape,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "delete",
         QT_TR_NOOP("delete"),
         Qt::Key_Delete,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-measure",
         QT_TR_NOOP("append measure"),
         Qt::CTRL+Qt::Key_B,
         Qt::WindowShortcut,
         QT_TR_NOOP("Append Measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-measures",
         QT_TR_NOOP("append measures"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Append Measures...")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-measure",
         QT_TR_NOOP("insert measure"),
         Qt::Key_Insert,
         Qt::WindowShortcut,
         QT_TR_NOOP("Insert Measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-measures",
         QT_TR_NOOP("insert measures"),
         Qt::CTRL+Qt::Key_Insert,
         Qt::WindowShortcut,
         QT_TR_NOOP("Insert Measures...")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-hbox",
         QT_TR_NOOP("Insert horizontal Frame"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Insert horizontal Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-vbox",
         QT_TR_NOOP("Insert vertical Frame"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Insert vertical Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-hbox",
         QT_TR_NOOP("Append horizontal Frame"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Append horizontal Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-vbox",
         QT_TR_NOOP("Append vertical Frame"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Append vertical Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "duplet",
         QT_TR_NOOP("duplet"),
         Qt::CTRL+Qt::Key_2,
         Qt::WindowShortcut,
         QT_TR_NOOP("Duplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "triplet",
         QT_TR_NOOP("triplet"),
         Qt::CTRL+Qt::Key_3,
         Qt::WindowShortcut,
         QT_TR_NOOP("Triplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "quadruplet",
         QT_TR_NOOP("quadruplet"),
         Qt::CTRL+Qt::Key_4,
         Qt::WindowShortcut,
         QT_TR_NOOP("Quadruplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "quintuplet",
         QT_TR_NOOP("Quintuplet"),
         Qt::CTRL+Qt::Key_5,
         Qt::WindowShortcut,
         QT_TR_NOOP("Quintuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "sextuplet",
         QT_TR_NOOP("Sextuplet"),
         Qt::CTRL+Qt::Key_6,
         Qt::WindowShortcut,
         QT_TR_NOOP("Sextuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "septuplet",
         QT_TR_NOOP("Septuplet"),
         Qt::CTRL+Qt::Key_7,
         Qt::WindowShortcut,
         QT_TR_NOOP("Septuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "octuplet",
         QT_TR_NOOP("Octuplet"),
         Qt::CTRL+Qt::Key_8,
         Qt::WindowShortcut,
         QT_TR_NOOP("Octuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "nonuplet",
         QT_TR_NOOP("Nonuplet"),
         Qt::CTRL+Qt::Key_9,
         Qt::WindowShortcut,
         QT_TR_NOOP("Nonuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "tuplet-dialog",
         QT_TR_NOOP("Other Tuplets"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Other Tuplets")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-1",
         QT_TR_NOOP("pad note 1/1"),
         Qt::ALT + Qt::Key_6,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/1"),
         QT_TR_NOOP("1/1"),
         &noteIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-2",
         QT_TR_NOOP("pad note 1/2"),
         Qt::ALT + Qt::Key_7,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/2"),
         QT_TR_NOOP("1/2"),
         &note2Icon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-4",
         QT_TR_NOOP("pad note 1/4"),
         Qt::ALT + Qt::Key_1,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/4"),
         QT_TR_NOOP("1/4"),
         &note4Icon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-8",
         QT_TR_NOOP("pad note 1/8"),
         Qt::ALT + Qt::Key_2,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/8"),
         QT_TR_NOOP("1/8"),
         &note8Icon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-16",
         QT_TR_NOOP("pad note 1/16"),
         Qt::ALT + Qt::Key_3,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/16"),
         QT_TR_NOOP("1/16"),
         &note16Icon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-32",
         QT_TR_NOOP("pad note 1/32"),
         Qt::ALT + Qt::Key_4,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/32"),
         QT_TR_NOOP("1/32"),
         &note32Icon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-64",
         QT_TR_NOOP("pad note 1/64"),
         Qt::ALT + Qt::Key_5,
         Qt::WindowShortcut,
         QT_TR_NOOP("1/64"),
         QT_TR_NOOP("1/64"),
         &note64Icon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-dot",
         QT_TR_NOOP("pad dot"),
         Qt::Key_Period,
         Qt::WindowShortcut,
         QT_TR_NOOP("dot"),
         QT_TR_NOOP("dot"),
         &dotIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-dotdot",
         QT_TR_NOOP("pad dot dot"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("dot dot"),
         QT_TR_NOOP("dot dot"),
         &dotdotIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-tie",
         QT_TR_NOOP("pad tie"),
         Qt::Key_Plus,
         Qt::WindowShortcut,
         QT_TR_NOOP("tie"),
         QT_TR_NOOP("tie"),
         &plusIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-rest",
         QT_TR_NOOP("pad rest"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("rest"),
         QT_TR_NOOP("rest"),
         &quartrestIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-sharp2",
         QT_TR_NOOP("double sharp"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("double sharp"),
         QT_TR_NOOP("double sharp"),
         &sharpsharpIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-sharp",
         QT_TR_NOOP("sharp"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("sharp"),
         QT_TR_NOOP("sharp"),
         &sharpIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-nat",
         QT_TR_NOOP("natural"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("natural"),
         QT_TR_NOOP("natural"),
         &naturalIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-flat",
         QT_TR_NOOP("flat"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("flat"),
         QT_TR_NOOP("flat"),
         &flatIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-flat2",
         QT_TR_NOOP("double flat"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("double flat"),
         QT_TR_NOOP("double flat"),
         &flatflatIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-acciaccatura",
         QT_TR_NOOP("acciaccatura"),
         Qt::Key_NumberSign,
         Qt::WindowShortcut,
         QT_TR_NOOP("acciaccatura"),
         QT_TR_NOOP("acciaccatura"),
         &acciaccaturaIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-appoggiatura",
         QT_TR_NOOP("appoggiatura"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("appoggiatura"),
         QT_TR_NOOP("appoggiatura"),
         &appoggiaturaIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-1",
         QT_TR_NOOP("voice 1"),
         QKeySequence(Qt::CTRL+Qt::Key_I, Qt::CTRL+Qt::Key_1),
         Qt::WindowShortcut,
         QT_TR_NOOP("voice 1"),
         QT_TR_NOOP("voice 1"),
         &voiceIcons[0]
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-2",
         QT_TR_NOOP("voice 2"),
         QKeySequence(Qt::CTRL+Qt::Key_I, Qt::CTRL+Qt::Key_2),
         Qt::WindowShortcut,
         QT_TR_NOOP("voice 2"),
         QT_TR_NOOP("voice 2"),
         &voiceIcons[1]
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-3",
         QT_TR_NOOP("voice 3"),
         QKeySequence(Qt::CTRL+Qt::Key_I, Qt::CTRL+Qt::Key_3),
         Qt::WindowShortcut,
         QT_TR_NOOP("voice 3"),
         QT_TR_NOOP("voice 3"),
         &voiceIcons[2]
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-4",
         QT_TR_NOOP("voice 4"),
         QKeySequence(Qt::CTRL+Qt::Key_I, Qt::CTRL+Qt::Key_4),
         Qt::WindowShortcut,
         QT_TR_NOOP("voice 4"),
         QT_TR_NOOP("voice 4"),
         &voiceIcons[3]
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "midi-on",
         QT_TR_NOOP("midi input in"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Enable Midi Input"),
         QT_TR_NOOP("Enable Midi Input"),
         &midiinIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "sound-on",
         QT_TR_NOOP("editing sound on"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Enable sound while editing"),
         QT_TR_NOOP("Enable sound while editing"),
         &speakerIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam-start",
         QT_TR_NOOP("beam start"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("beam start"),
         QT_TR_NOOP("beam start"),
         &sbeamIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam-mid",
         QT_TR_NOOP("beam mid"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("beam mid"),
         QT_TR_NOOP("beam mid"),
         &mbeamIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "no-beam",
         QT_TR_NOOP("no beam"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("no beam"),
         QT_TR_NOOP("no beam"),
         &nbeamIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam32",
         QT_TR_NOOP("beam 32"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("beam 32"),
         QT_TR_NOOP("beam 32"),
         &beam32Icon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "auto-beam",
         QT_TR_NOOP("auto beam"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("auto beam"),
         QT_TR_NOOP("auto beam"),
         &abeamIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-palette",
         QT_TR_NOOP("Palette"),
         Qt::Key_F9,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("Palette"),
         QT_TR_NOOP("Palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-playpanel",
         QT_TR_NOOP("Play Panel"),
         Qt::Key_F11,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("Play Panel"),
         QT_TR_NOOP("Play Panel")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-navigator",
         QT_TR_NOOP("Navigator"),
         Qt::Key_F12,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("Navigator"),
         QT_TR_NOOP("Navigator")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-mixer",
         QT_TR_NOOP("Mixer"),
         Qt::Key_F10,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("Mixer"),
         QT_TR_NOOP("Mixer")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "toggle-transport",
         QT_TR_NOOP("Transport Toolbar"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Transport"),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "toggle-noteinput",
         QT_TR_NOOP("Note Input Toolbar"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Note Input"),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "toggle-statusbar",
         QT_TR_NOOP("Status Bar"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Status Bar"),
         QT_TR_NOOP("Status Bar")
         ),

      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "quit",
         QT_TR_NOOP("Quit"),
         Qt::CTRL + Qt::Key_Q,
         Qt::WindowShortcut,
         QT_TR_NOOP("Quit"),
         QT_TR_NOOP("Quit"),
         &exitIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "mag",
         QT_TR_NOOP("Mag"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Mag"),
         QT_TR_NOOP("Zoom Canvas"),
         &viewmagIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "lyrics",
         QT_TR_NOOP("Lyrics"),
         Qt::CTRL + Qt::Key_L,
         Qt::WindowShortcut,
         QT_TR_NOOP("Lyrics"),
         QT_TR_NOOP("Lyrics")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "fingering",
         QT_TR_NOOP("Fingering Palette"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Fingering..."),
         QT_TR_NOOP("Fingering")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "tempo",
         QT_TR_NOOP("Tempo Palette"),
         Qt::CTRL+Qt::ALT + Qt::Key_T,
         Qt::WindowShortcut,
         QT_TR_NOOP("Tempo..."),
         QT_TR_NOOP("Tempo")
         ),
#if 0
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "metronome",
         QT_TR_NOOP("Metronome"),
         Qt::CTRL+Qt::ALT + Qt::Key_M,
         Qt::WindowShortcut,
         QT_TR_NOOP("Metronome"),
         QT_TR_NOOP("Metronome")
         ),
#endif
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "system-text",
         QT_TR_NOOP("Add System Text"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("System Text"),
         QT_TR_NOOP("Add System Text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "staff-text",
         QT_TR_NOOP("Add Staff Text"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Staff Text"),
         QT_TR_NOOP("Add Staff Text")
         ),
      Shortcut(
         STATE_NORMAL,
         "frame-text",
         QT_TR_NOOP("Add Text"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Frame Text"),
         QT_TR_NOOP("Add Text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "title-text",
         QT_TR_NOOP("Add Title"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Title"),
         QT_TR_NOOP("Add Title Text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "subtitle-text",
         QT_TR_NOOP("Add Subtitle"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Subtitle"),
         QT_TR_NOOP("Add Subtitle Text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "composer-text",
         QT_TR_NOOP("Add Composer"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Composer"),
         QT_TR_NOOP("Add Composer Text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "poet-text",
         QT_TR_NOOP("Add Poet"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Poet"),
         QT_TR_NOOP("Add Poet Text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "copyright-text",
         QT_TR_NOOP("Add Copyright"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Copyright"),
         QT_TR_NOOP("Add Copyright Text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-text",
         QT_TR_NOOP("Add Chord Name"),
         Qt::CTRL + Qt::Key_K,
         Qt::WindowShortcut,
         QT_TR_NOOP("Chord Name"),
         QT_TR_NOOP("Add Chord Text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rehearsalmark-text",
         QT_TR_NOOP("Add Rehearsal Mark"),
         Qt::CTRL + Qt::Key_M,
         Qt::WindowShortcut,
         QT_TR_NOOP("Rehearsal Mark"),
         QT_TR_NOOP("Add Rehearsal Mark")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "play",
         QT_TR_NOOP("player play"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Play"),
         QT_TR_NOOP("start sequencer play"),
         &playIcon
         ),
      Shortcut(
         STATE_PLAY,
         "pause",
         QT_TR_NOOP("toggle pause"),
         Qt::Key_Space,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("Pause"),
         QT_TR_NOOP("toggle pause"),
         &pauseIcon
         ),
      Shortcut(
         STATE_PLAY,
         "play-prev-chord",
         QT_TR_NOOP("previous chord"),
         Qt::Key_Left,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("left chord"),
         QT_TR_NOOP("left chord")
         ),
      Shortcut(
         STATE_PLAY,
         "play-prev-measure",
         QT_TR_NOOP("previous measure"),
         Qt::CTRL+Qt::Key_Left,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("previous measure"),
         QT_TR_NOOP("prev measure")
         ),
      Shortcut(
         STATE_PLAY,
         "play-next-chord",
         QT_TR_NOOP("next chord"),
         Qt::Key_Right,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("next chord"),
         QT_TR_NOOP("next chord")
         ),
      Shortcut(
         STATE_PLAY,
         "play-next-measure",
         QT_TR_NOOP("next measure"),
         Qt::CTRL+Qt::Key_Right,
         Qt::ApplicationShortcut,
         QT_TR_NOOP("next measure"),
         QT_TR_NOOP("next measure")
         ),
      Shortcut(
         STATE_PLAY,
         "seek-begin",
         QT_TR_NOOP("seek to begin"),
         Qt::Key_Home,
         Qt::ApplicationShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "rewind",
         QT_TR_NOOP("player rewind"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Rewind"),
         QT_TR_NOOP("rewind to start position"),
         &startIcon
         ),
      Shortcut(
         STATE_PLAY,
         "seek-end",
         QT_TR_NOOP("seek to end"),
         Qt::Key_End,
         Qt::WindowShortcut,
         QT_TR_NOOP(""),
         QT_TR_NOOP("")
         ),
      Shortcut(
         STATE_NORMAL,
         "repeat",
         QT_TR_NOOP("play repeats on"),
         Qt::CTRL+Qt::Key_R,
         Qt::WindowShortcut,
         QT_TR_NOOP("Repeat"),
         QT_TR_NOOP("play repeats on/off"),
         &repeatIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "load-style",
         QT_TR_NOOP("load style"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Load Style"),
         QT_TR_NOOP("Load Style"),
         &fileOpenIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "save-style",
         QT_TR_NOOP("save style"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Save Style"),
         QT_TR_NOOP("Save Style"),
         &fileSaveIcon
         ),
      Shortcut (
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "select-all",
         QT_TR_NOOP("select all"),
         Qt::CTRL+Qt::Key_A,
         Qt::WindowShortcut,
         QT_TR_NOOP("Select All"),
         QT_TR_NOOP("Select All")
         ),
      Shortcut (
         STATE_NORMAL,
         "transpose",
         QT_TR_NOOP("transpose"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Transpose..."),
         QT_TR_NOOP("Transpose")
         ),
      Shortcut (
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "clef-violin",
         QT_TR_NOOP("violin clef"),
         QKeySequence(Qt::CTRL+Qt::Key_Y, Qt::CTRL+Qt::Key_1),
         Qt::WindowShortcut,
         QT_TR_NOOP("violin clef"),
         QT_TR_NOOP("violin clef")
         ),
      Shortcut (
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "clef-bass",
         QT_TR_NOOP("bass clef"),
         QKeySequence(Qt::CTRL+Qt::Key_Y, Qt::CTRL+Qt::Key_2),
         Qt::WindowShortcut,
         QT_TR_NOOP("bass clef"),
         QT_TR_NOOP("bass clef")
         ),
      Shortcut (
         STATE_NORMAL,
         "voice-x12",
         QT_TR_NOOP("exchange voice 1-2"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("exchange voice 1-2"),
         QT_TR_NOOP("exchange voice 1-2")
         ),
      Shortcut (
         STATE_NORMAL,
         "voice-x13",
         QT_TR_NOOP("exchange voice 1-3"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("exchange voice 1-3"),
         QT_TR_NOOP("exchange voice 1-3")
         ),
      Shortcut (
         STATE_NORMAL,
         "voice-x14",
         QT_TR_NOOP("exchange voice 1-4"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("exchange voice 1-4"),
         QT_TR_NOOP("exchange voice 1-4")
         ),
      Shortcut (
         STATE_NORMAL,
         "voice-x23",
         QT_TR_NOOP("exchange voice 2-3"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("exchange voice 2-3"),
         QT_TR_NOOP("exchange voice 2-3")
         ),
      Shortcut (
         STATE_NORMAL,
         "voice-x24",
         QT_TR_NOOP("exchange voice 2-4"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("exchange voice 2-4"),
         QT_TR_NOOP("exchange voice 2-4")
         ),
      Shortcut (
         STATE_NORMAL,
         "voice-x34",
         QT_TR_NOOP("exchange voice 3-4"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("exchange voice 3-4"),
         QT_TR_NOOP("exchange voice 3-4")
         ),
      Shortcut (
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "concert-pitch",
         QT_TR_NOOP("display in concert pitch"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Display in Concert Pitch"),
         QT_TR_NOOP("Display in Concert Pitch")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "repeat-cmd",
         QT_TR_NOOP("repeat last command"),
         Qt::Key_R,
         Qt::WindowShortcut,
         QT_TR_NOOP("Repeat last command"),
         QT_TR_NOOP("Repeat last command"),
         &fileOpenIcon
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "edit-meta",
         QT_TR_NOOP("edit score meta data"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Meta Data"),
         QT_TR_NOOP("Meta Data")
         ),
      Shortcut(0, 0, 0),
      };

