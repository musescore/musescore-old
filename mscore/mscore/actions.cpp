//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "musescore.h"
#include "libmscore/score.h"      // states
#include "icons.h"

//---------------------------------------------------------
//    initial list of shortcuts
//---------------------------------------------------------

Shortcut MuseScore::sc[] = {
      Shortcut(
         STATE_ALL,
         0,
         "local-help",
         QT_TRANSLATE_NOOP("action","Local handbook"),            // Appears in Edit > Preferences > Shortcuts
         Qt::Key_F1,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Local &Handbook..."),        // Appears in menu
         QT_TRANSLATE_NOOP("action","Show local handbook")        // Appears if you use Help > What's This?
         ),
      Shortcut(
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         0,
         "file-open",
         QT_TRANSLATE_NOOP("action","File open"),
         QKeySequence::Open,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","&Open..."),
         QT_TRANSLATE_NOOP("action","Load score from file"),
         fileOpen_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         0,
         "file-save",
         QT_TRANSLATE_NOOP("action","File save"),
         QKeySequence::Save,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","&Save"),
         QT_TRANSLATE_NOOP("action","Save score to file"),
          fileSave_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         A_SCORE,
         "file-save-as",
         QT_TRANSLATE_NOOP("action","File save as"),
         QKeySequence::SaveAs,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Save &As..."),
         QT_TRANSLATE_NOOP("action","Save score under a new file name"),
          fileSaveAs_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         A_SCORE,
         "file-save-selection",
         QT_TRANSLATE_NOOP("action","Save Selection"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Save Selection..."),
         QT_TRANSLATE_NOOP("action","Save current selection as new score"),
          fileSaveAs_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         A_SCORE,
         "file-save-a-copy",
         QT_TRANSLATE_NOOP("action","File save a copy"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Save a Copy..."),
         QT_TRANSLATE_NOOP("action","Save a copy of the score in addition to the current file")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         A_SCORE,
         "file-export",
         QT_TRANSLATE_NOOP("action","Export Score"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Export Score..."),
         QT_TRANSLATE_NOOP("action","Save a copy of the score in various formats"),
          fileSave_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         A_SCORE,
         "file-reload",
         QT_TRANSLATE_NOOP("action","File reload"),
         0,                         // no shortcut, its an destructive non undoable operation
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Reload"),
         QT_TRANSLATE_NOOP("action","Reload score from file")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         0,
         "file-close",
         QT_TRANSLATE_NOOP("action","File close"),
         QKeySequence::Close,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","&Close"),
         QT_TRANSLATE_NOOP("action","Close current score")
         ),
      Shortcut(
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         0,
         "file-new",
         QT_TRANSLATE_NOOP("action","File new"),
         Qt::CTRL+Qt::Key_N,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","&New..."),
         QT_TRANSLATE_NOOP("action","Create new score"),
          fileNew_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         A_SCORE,
         "print",
         QT_TRANSLATE_NOOP("action","Print"),
         Qt::CTRL+Qt::Key_P,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","&Print..."),
         QT_TRANSLATE_NOOP("action","Print score"),
          print_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "undo",
         QT_TRANSLATE_NOOP("action","Undo"),
         QKeySequence::Undo,      // Qt::CTRL+Qt::Key_Z, // QKeySequence::Undo,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","&Undo"),
         QT_TRANSLATE_NOOP("action","Undo last change"),
          undo_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "redo",
         QT_TRANSLATE_NOOP("action","Redo"),
         QKeySequence::Redo,      // Qt::CTRL+Qt::SHIFT+Qt::Key_Z,    // QKeySequence::Redo,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","&Redo"),
         QT_TRANSLATE_NOOP("action","Redo last undo"),
          redo_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "cut",
         QT_TRANSLATE_NOOP("action","Cut"),
         Qt::CTRL+Qt::Key_X,        // QKeySequence::Cut,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Cu&t"),
         QT_TRANSLATE_NOOP("action",""),
          cut_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_FOTO,
         0,
         "copy",
         QT_TRANSLATE_NOOP("action","Copy"),
         Qt::CTRL+Qt::Key_C,        // QKeySequence::Copy,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","&Copy"),
         QT_TRANSLATE_NOOP("action",""),
          copy_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "paste",
         QT_TRANSLATE_NOOP("action","Paste"),
         Qt::CTRL+Qt::Key_V,        //  QKeySequence::Paste,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","&Paste"),
         QT_TRANSLATE_NOOP("action",""),
          paste_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "instruments",
         QT_TRANSLATE_NOOP("action","Show instruments dialog"),
         Qt::Key_I,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Instruments..."),
         QT_TRANSLATE_NOOP("action","Show instruments dialog")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "clefs",
         QT_TRANSLATE_NOOP("action","Show clefs palette"),
         Qt::Key_Y,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Clefs..."),
         QT_TRANSLATE_NOOP("action","Show clefs palette"),
          clef_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "keys",
         QT_TRANSLATE_NOOP("action","Show keys signatures palette"),
         Qt::Key_K,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Key Signatures..."),
         QT_TRANSLATE_NOOP("action","Show key signatures palette"),
          sharp_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "symbols",
         QT_TRANSLATE_NOOP("action","Show symbols palette"),
         Qt::Key_Z,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Symbols..."),
         QT_TRANSLATE_NOOP("action","Show symbols palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "times",
         QT_TRANSLATE_NOOP("action","Show time signatures palette"),
         Qt::Key_T,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Time Signatures..."),
         QT_TRANSLATE_NOOP("action","Show time signatures palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "dynamics",
         QT_TRANSLATE_NOOP("action","Show dynamics palette"),
         Qt::Key_L,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Dynamics..."),
         QT_TRANSLATE_NOOP("action","Show dynamics palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-input",
         QT_TRANSLATE_NOOP("action","Note input mode"),
         Qt::Key_N,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Note Input"),
         QT_TRANSLATE_NOOP("action","Note input mode"),
          noteEntry_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pitch-spell",
         QT_TRANSLATE_NOOP("action","Pitch spell"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Pitch Spell"),
         QT_TRANSLATE_NOOP("action","Pitch spell")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval1",
         QT_TRANSLATE_NOOP("action","Enter unison above"),
         Qt::ALT + Qt::Key_1,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Unison Above"),
         QT_TRANSLATE_NOOP("action","Enter unison above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval2",
         QT_TRANSLATE_NOOP("action","Enter second above"),
         Qt::ALT + Qt::Key_2,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Second Above"),
         QT_TRANSLATE_NOOP("action","Enter second above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval3",
         QT_TRANSLATE_NOOP("action","Enter third above"),
         Qt::ALT + Qt::Key_3,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Third Above"),
         QT_TRANSLATE_NOOP("action","Enter third above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval4",
         QT_TRANSLATE_NOOP("action","Enter fourth above"),
         Qt::ALT + Qt::Key_4,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Fourth Above"),
         QT_TRANSLATE_NOOP("action","Enter fourth above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval5",
         QT_TRANSLATE_NOOP("action","Enter fifth above"),
         Qt::ALT + Qt::Key_5,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Fifth Above"),
         QT_TRANSLATE_NOOP("action","Enter fifth above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval6",
         QT_TRANSLATE_NOOP("action","Enter sixth above"),
         Qt::ALT + Qt::Key_6,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Sixth Above"),
         QT_TRANSLATE_NOOP("action","Enter sixth above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval7",
         QT_TRANSLATE_NOOP("action","Enter seventh above"),
         Qt::ALT + Qt::Key_7,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Seventh Above"),
         QT_TRANSLATE_NOOP("action","Enter seventh above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval8",
         QT_TRANSLATE_NOOP("action","Enter octave above"),
         Qt::ALT + Qt::Key_8,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Octave Above"),
         QT_TRANSLATE_NOOP("action","Enter octave above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval9",
         QT_TRANSLATE_NOOP("action","Enter ninth above"),
         Qt::ALT + Qt::Key_9,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Ninth Above"),
         QT_TRANSLATE_NOOP("action","Enter ninth above")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval-2",
         QT_TRANSLATE_NOOP("action","Enter second below"),
         Qt::SHIFT + Qt::Key_2,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Second Below"),
         QT_TRANSLATE_NOOP("action","Enter second below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval-3",
         QT_TRANSLATE_NOOP("action","Enter third below"),
         Qt::SHIFT + Qt::Key_3,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Third Below"),
         QT_TRANSLATE_NOOP("action","Enter third below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval-4",
         QT_TRANSLATE_NOOP("action","Enter fourth below"),
         Qt::SHIFT + Qt::Key_4,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Fourth Below"),
         QT_TRANSLATE_NOOP("action","Enter fourth below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval-5",
         QT_TRANSLATE_NOOP("action","Enter fifth below"),
         Qt::SHIFT + Qt::Key_5,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Fifth Below"),
         QT_TRANSLATE_NOOP("action","Enter fifth below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval-6",
         QT_TRANSLATE_NOOP("action","Enter sixth below"),
         Qt::SHIFT + Qt::Key_6,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Sixth Below"),
         QT_TRANSLATE_NOOP("action","Enter sixth below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval-7",
         QT_TRANSLATE_NOOP("action","Enter seventh below"),
         Qt::SHIFT + Qt::Key_7,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Seventh Below"),
         QT_TRANSLATE_NOOP("action","Enter seventh below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval-8",
         QT_TRANSLATE_NOOP("action","Enter octave below"),
         Qt::SHIFT + Qt::Key_8,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Octave Below"),
         QT_TRANSLATE_NOOP("action","Enter octave below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "interval-9",
         QT_TRANSLATE_NOOP("action","Enter ninth below"),
         Qt::SHIFT + Qt::Key_9,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Ninth Below"),
         QT_TRANSLATE_NOOP("action","Enter ninth below")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-a",
         QT_TRANSLATE_NOOP("action","Enter note A"),
         Qt::Key_A,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","A"),
         QT_TRANSLATE_NOOP("action","Enter note A")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-b",
         QT_TRANSLATE_NOOP("action","Enter note B"),
         Qt::Key_B,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","B"),
         QT_TRANSLATE_NOOP("action","Enter note B")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-c",
         QT_TRANSLATE_NOOP("action","Enter note C"),
         Qt::Key_C,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","C"),
         QT_TRANSLATE_NOOP("action","Enter note C")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-d",
         QT_TRANSLATE_NOOP("action","Enter note D"),
         Qt::Key_D,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","D"),
         QT_TRANSLATE_NOOP("action","Enter note D")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-e",
         QT_TRANSLATE_NOOP("action","Enter note E"),
         Qt::Key_E,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","E"),
         QT_TRANSLATE_NOOP("action","Enter note E")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-f",
         QT_TRANSLATE_NOOP("action","Enter note F"),
         Qt::Key_F,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","F"),
         QT_TRANSLATE_NOOP("action","Enter note F")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-g",
         QT_TRANSLATE_NOOP("action","Enter note G"),
         Qt::Key_G,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","G"),
         QT_TRANSLATE_NOOP("action","Enter note G")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-a",
         QT_TRANSLATE_NOOP("action","Add note A to chord"),
         Qt::SHIFT + Qt::Key_A,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add A"),
         QT_TRANSLATE_NOOP("action","Add note A to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-b",
         QT_TRANSLATE_NOOP("action","Add note B to chord"),
         Qt::SHIFT + Qt::Key_B,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add B"),
         QT_TRANSLATE_NOOP("action","Add note B to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-c",
         QT_TRANSLATE_NOOP("action","Add note C to chord"),
         Qt::SHIFT + Qt::Key_C,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add C"),
         QT_TRANSLATE_NOOP("action","Add note C to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-d",
         QT_TRANSLATE_NOOP("action","Add note D to chord"),
         Qt::SHIFT + Qt::Key_D,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add D"),
         QT_TRANSLATE_NOOP("action","Add note D to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-e",
         QT_TRANSLATE_NOOP("action","Add note E to chord"),
         Qt::SHIFT + Qt::Key_E,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add E"),
         QT_TRANSLATE_NOOP("action","Add note E to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-f",
         QT_TRANSLATE_NOOP("action","Add note F to chord"),
         Qt::SHIFT + Qt::Key_F,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add F"),
         QT_TRANSLATE_NOOP("action","Add note F to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-g",
         QT_TRANSLATE_NOOP("action","Add note G to chord"),
         Qt::SHIFT + Qt::Key_G,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add G"),
         QT_TRANSLATE_NOOP("action","Add note G to chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-a",
         QT_TRANSLATE_NOOP("action","Insert note A"),
         Qt::CTRL + Qt::SHIFT + Qt::Key_A,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert A"),
         QT_TRANSLATE_NOOP("action","Insert note A")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-b",
         QT_TRANSLATE_NOOP("action","Insert note B"),
         Qt::CTRL + Qt::SHIFT + Qt::Key_B,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert B"),
         QT_TRANSLATE_NOOP("action","Insert note B")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-c",
         QT_TRANSLATE_NOOP("action","Insert note C"),
         Qt::CTRL + Qt::SHIFT + Qt::Key_C,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert C"),
         QT_TRANSLATE_NOOP("action","Insert note C")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-d",
         QT_TRANSLATE_NOOP("action","Insert note D"),
         Qt::CTRL + Qt::SHIFT + Qt::Key_D,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert D"),
         QT_TRANSLATE_NOOP("action","Insert note D")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-e",
         QT_TRANSLATE_NOOP("action","Insert note E"),
         Qt::CTRL + Qt::SHIFT + Qt::Key_E,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert E"),
         QT_TRANSLATE_NOOP("action","Insert note E")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-f",
         QT_TRANSLATE_NOOP("action","Insert note F"),
         Qt::CTRL + Qt::SHIFT + Qt::Key_F,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert F"),
         QT_TRANSLATE_NOOP("action","Insert note F")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-g",
         QT_TRANSLATE_NOOP("action","Insert note G"),
         Qt::CTRL + Qt::SHIFT + Qt::Key_G,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert G"),
         QT_TRANSLATE_NOOP("action","Insert note G")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest",
         QT_TRANSLATE_NOOP("action","Enter rest"),
         Qt::Key_0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Rest"),
         QT_TRANSLATE_NOOP("action","Enter rest"),
          quartrest_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "add-staccato",
         QT_TRANSLATE_NOOP("action","Add staccato"),
         Qt::SHIFT+Qt::Key_Period,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Staccato"),
         QT_TRANSLATE_NOOP("action","Add staccato")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "add-tenuto",
         QT_TRANSLATE_NOOP("action","Add tenuto"),
         Qt::Key_Underscore,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Tenuto"),
         QT_TRANSLATE_NOOP("action","Add tenuto")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "add-trill",
         QT_TRANSLATE_NOOP("action","Add trill"),
	   0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Trill"),
         QT_TRANSLATE_NOOP("action","Add trill")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "stretch+",
         QT_TRANSLATE_NOOP("action","More stretch"),
         Qt::Key_BraceRight,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add More Stretch"),
         QT_TRANSLATE_NOOP("action","Add more stretch to selected measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "stretch-",
         QT_TRANSLATE_NOOP("action","Less stretch"),
         Qt::Key_BraceLeft,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add Less Stretch"),
         QT_TRANSLATE_NOOP("action","Add less stretch to selected measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "reset-beammode",
         QT_TRANSLATE_NOOP("action","Reset Beam Mode"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Reset Beam Mode"),
         QT_TRANSLATE_NOOP("action","Reset beam mode of selected measures")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "flip",
         QT_TRANSLATE_NOOP("action","Flip direction"),
         Qt::Key_X,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Flip Direction"),
         QT_TRANSLATE_NOOP("action","Flip direction"),
          flip_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pitch-up",
         QT_TRANSLATE_NOOP("action","Pitch up"),
         Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Up"),
         QT_TRANSLATE_NOOP("action","Pitch up")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pitch-up-diatonic",
         QT_TRANSLATE_NOOP("action","Diatonic pitch up"),
         Qt::SHIFT+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Diatonic up"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch up")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pitch-up-octave",
         QT_TRANSLATE_NOOP("action","Pitch up octave"),
         Qt::CTRL + Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Up Octave"),
         QT_TRANSLATE_NOOP("action","Pitch up by an octave")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "up-chord",
         QT_TRANSLATE_NOOP("action","Go to higher pitched note in chord"),
         Qt::ALT+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Up Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to higher pitched note in chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "top-chord",
         QT_TRANSLATE_NOOP("action","Go to top note in chord"),
         Qt::ALT+Qt::CTRL+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Top Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to top note in chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "move-up",
         QT_TRANSLATE_NOOP("action","Move up"),
         Qt::SHIFT+Qt::CTRL+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","up+shift+ctrl"),
         QT_TRANSLATE_NOOP("action","up+shift+ctrl")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pitch-down",
         QT_TRANSLATE_NOOP("action","Pitch down"),
         Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Down"),
         QT_TRANSLATE_NOOP("action","Pitch down")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pitch-down-diatonic",
         QT_TRANSLATE_NOOP("action","Diatonic pitch down"),
         Qt::SHIFT+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Diatonic down"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch down")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pitch-down-octave",
         QT_TRANSLATE_NOOP("action","Pitch down octave"),
         Qt::CTRL + Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Down octave"),
         QT_TRANSLATE_NOOP("action","Pitch down by an octave")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "down-chord",
         QT_TRANSLATE_NOOP("action","Go to lower pitched note in chord"),
         Qt::ALT+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Down Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to lower pitched note in chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "bottom-chord",
         QT_TRANSLATE_NOOP("action","Go to bottom note in chord"),
         Qt::ALT+Qt::CTRL+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Bottom Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to bottom note in chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "move-down",
         QT_TRANSLATE_NOOP("action","Move down"),
         Qt::SHIFT+Qt::CTRL+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","down+shift+ctrl"),
         QT_TRANSLATE_NOOP("action","down+shift+ctrl")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "prev-chord",
         QT_TRANSLATE_NOOP("action","Previous chord"),
         Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Previous Chord"),
         QT_TRANSLATE_NOOP("action","Previous chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "prev-measure",
         QT_TRANSLATE_NOOP("action","Previous measure"),
         Qt::CTRL+Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Previous Measure"),
         QT_TRANSLATE_NOOP("action","Previous measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "next-chord",
         QT_TRANSLATE_NOOP("action","Next chord"),
         Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Next Chord"),
         QT_TRANSLATE_NOOP("action","Next chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "next-measure",
         QT_TRANSLATE_NOOP("action","Next measure"),
         Qt::CTRL+Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Next Measure"),
         QT_TRANSLATE_NOOP("action","Next measure")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "select-prev-chord",
         QT_TRANSLATE_NOOP("action","Add previous chord to selection"),
         Qt::SHIFT+Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","left+shift"),
         QT_TRANSLATE_NOOP("action","left+shift")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "select-prev-measure",
         QT_TRANSLATE_NOOP("action","Select to beginning of measure"),
         Qt::CTRL+Qt::SHIFT+Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","left+ctrl"),
         QT_TRANSLATE_NOOP("action","left+ctrl")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "select-next-chord",
         QT_TRANSLATE_NOOP("action","Add next chord to selection"),
         Qt::SHIFT+Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","right+shift"),
         QT_TRANSLATE_NOOP("action","right+shift")
         ),
      Shortcut(
         STATE_NOTE_ENTRY,
         0,
         "move-right",
         QT_TRANSLATE_NOOP("action","Move chord/rest right"),
         Qt::SHIFT+Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Move chord/rest right"),
         QT_TRANSLATE_NOOP("action","Move chord/rest right")
         ),
      Shortcut(
         STATE_NOTE_ENTRY,
         0,
         "move-left",
         QT_TRANSLATE_NOOP("action","Move chord/rest left"),
         Qt::SHIFT+Qt::Key_Left,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Move chord/rest left"),
         QT_TRANSLATE_NOOP("action","Move chord/rest left")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "select-next-measure",
         QT_TRANSLATE_NOOP("action","Select to end of measure"),
         Qt::CTRL+Qt::SHIFT+Qt::Key_Right,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","right+ctrl+shift"),
         QT_TRANSLATE_NOOP("action","right+ctrl+shift")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "select-begin-line",
         QT_TRANSLATE_NOOP("action","Select to beginning of line"),
         Qt::SHIFT+Qt::Key_Home,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","shift+home"),
         QT_TRANSLATE_NOOP("action","shift+home")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "select-end-line",
         QT_TRANSLATE_NOOP("action","Select to end of line"),
         Qt::SHIFT+Qt::Key_End,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","shift+end"),
         QT_TRANSLATE_NOOP("action","shift+end")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "select-begin-score",
         QT_TRANSLATE_NOOP("action","Select to beginning of score"),
         Qt::CTRL+Qt::SHIFT+Qt::Key_Home,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","ctrl+shift+home"),
         QT_TRANSLATE_NOOP("action","ctrl+shift+home")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "select-end-score",
         QT_TRANSLATE_NOOP("action","Select to end of score"),
         Qt::CTRL+Qt::SHIFT+Qt::Key_End,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","ctrl+shift+end"),
         QT_TRANSLATE_NOOP("action","ctrl+shift+end")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-staff-above",
         QT_TRANSLATE_NOOP("action","Add staff above to selection"),
         Qt::ALT+Qt::SHIFT+Qt::Key_Up,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add staff above to selection"),
         QT_TRANSLATE_NOOP("action","Add staff above to selection")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-staff-below",
         QT_TRANSLATE_NOOP("action","Add staff below to selection"),
         Qt::ALT+Qt::SHIFT+Qt::Key_Down,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add staff below to selection"),
         QT_TRANSLATE_NOOP("action","Add staff below to selection")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "page-prev",
         QT_TRANSLATE_NOOP("action","Page: previous"),
         Qt::Key_PageUp,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "page-next",
         QT_TRANSLATE_NOOP("action","Page: next"),
         Qt::Key_PageDown,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "page-top",
         QT_TRANSLATE_NOOP("action","Page: top"),
         Qt::Key_Home,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "page-end",
         QT_TRANSLATE_NOOP("action","Page: end"),
         Qt::Key_End,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "add-slur",
         QT_TRANSLATE_NOOP("action","Add slur"),
         Qt::Key_S,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "add-hairpin",
         QT_TRANSLATE_NOOP("action","Crescendo"),
         Qt::Key_H,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "add-hairpin-reverse",
         QT_TRANSLATE_NOOP("action","Decrescendo"),
         Qt::SHIFT+Qt::Key_H,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY | STATE_SEARCH | STATE_FOTO,
         0,
         "escape",
         QT_TRANSLATE_NOOP("action","Escape"),
         Qt::Key_Escape,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         A_CMD,
         "delete",
         QT_TRANSLATE_NOOP("action","Delete"),
         Qt::Key_Delete,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Delete"),
         QT_TRANSLATE_NOOP("action","Delete contents of the selected measures")
         ),
      Shortcut(
         STATE_NORMAL,
         A_CMD,
         "time-delete",
         QT_TRANSLATE_NOOP("action","Timewise delete"),
         0, // Qt::CTRL + Qt::Key_Delete,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Timewise Delete"),
         QT_TRANSLATE_NOOP("action","Delete element and duration")
         ),
      Shortcut(
         STATE_NORMAL,
         A_CMD,
         "delete-measures",
         QT_TRANSLATE_NOOP("action","Delete selected measures"),
         Qt::CTRL + Qt::Key_Delete,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Delete Selected Measures"),
         QT_TRANSLATE_NOOP("action","Delete selected measures")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-measure",
         QT_TRANSLATE_NOOP("action","Append one measure"),      // Appears in Edit > Preferences > Shortcuts
         Qt::CTRL+Qt::Key_B,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Append One Measure")       // Appears in menu
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-measures",
         QT_TRANSLATE_NOOP("action","Append measures"),
         Qt::CTRL+Qt::SHIFT+Qt::Key_B,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Append Measures...")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-measure",
         QT_TRANSLATE_NOOP("action","Insert one measure"),
         Qt::Key_Insert,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert One Measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-measures",
         QT_TRANSLATE_NOOP("action","Insert measures"),
         Qt::CTRL+Qt::Key_Insert,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert Measures...")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-hbox",
         QT_TRANSLATE_NOOP("action","Insert horizontal frame"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert Horizontal Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-textframe",
         QT_TRANSLATE_NOOP("action","Insert text frame"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert Text Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-textframe",
         QT_TRANSLATE_NOOP("action","Append text frame"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Append Text Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-fretframe",
         QT_TRANSLATE_NOOP("action","Insert fret diagram frame"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert Fret Diagram Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-vbox",
         QT_TRANSLATE_NOOP("action","Insert vertical frame"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert Vertical Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-hbox",
         QT_TRANSLATE_NOOP("action","Append horizontal frame"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Append Horizontal Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-vbox",
         QT_TRANSLATE_NOOP("action","Append vertical frame"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Append Vertical Frame")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "duplet",
         QT_TRANSLATE_NOOP("action","Duplet"),
         Qt::CTRL+Qt::Key_2,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Duplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "triplet",
         QT_TRANSLATE_NOOP("action","Triplet"),
         Qt::CTRL+Qt::Key_3,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Triplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "quadruplet",
         QT_TRANSLATE_NOOP("action","Quadruplet"),
         Qt::CTRL+Qt::Key_4,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Quadruplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "quintuplet",
         QT_TRANSLATE_NOOP("action","Quintuplet"),
         Qt::CTRL+Qt::Key_5,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Quintuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "sextuplet",
         QT_TRANSLATE_NOOP("action","Sextuplet"),
         Qt::CTRL+Qt::Key_6,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Sextuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "septuplet",
         QT_TRANSLATE_NOOP("action","Septuplet"),
         Qt::CTRL+Qt::Key_7,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Septuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "octuplet",
         QT_TRANSLATE_NOOP("action","Octuplet"),
         Qt::CTRL+Qt::Key_8,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Octuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "nonuplet",
         QT_TRANSLATE_NOOP("action","Nonuplet"),
         Qt::CTRL+Qt::Key_9,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Nonuplet")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "tuplet-dialog",
         QT_TRANSLATE_NOOP("action","Other tuplets"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Other...")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "note-longa",
         QT_TRANSLATE_NOOP("action","Note duration: longa"),
         Qt::Key_9,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Longa"),
         QT_TRANSLATE_NOOP("action","Longa"),
          longaUp_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "note-breve",
         QT_TRANSLATE_NOOP("action","Note duration: breve"),
         Qt::Key_8,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Double whole note (breve)"),
         QT_TRANSLATE_NOOP("action","Double whole note (breve)"),
          brevis_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-note-1",
         QT_TRANSLATE_NOOP("action","Note duration: whole"),
         Qt::Key_7,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Whole note (semibreve)"),
         QT_TRANSLATE_NOOP("action","Whole note (semibreve)"),
          note_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-note-2",
         QT_TRANSLATE_NOOP("action","Note duration: half"),
         Qt::Key_6,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Half note (minim)"),
         QT_TRANSLATE_NOOP("action","Half note (minim)"),
          note2_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-note-4",
         QT_TRANSLATE_NOOP("action","Note duration: quarter"),
         Qt::Key_5,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Quarter note (crochet)"),
         QT_TRANSLATE_NOOP("action","Quarter note (crochet)"),
          note4_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-note-8",
         QT_TRANSLATE_NOOP("action","Note duration: 8th"),
         Qt::Key_4,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Eighth note (quaver)"),
         QT_TRANSLATE_NOOP("action","Eighth note (quaver)"),
          note8_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-note-16",
         QT_TRANSLATE_NOOP("action","Note duration: 16th"),
         Qt::Key_3,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","16th note (semiquaver)"),
         QT_TRANSLATE_NOOP("action","16th note (semiquaver)"),
          note16_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-note-32",
         QT_TRANSLATE_NOOP("action","Note duration: 32nd"),
         Qt::Key_2,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","32nd note (hemisemiquaver)"),
         QT_TRANSLATE_NOOP("action","32nd note (hemisemiquaver)"),
          note32_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-note-64",
         QT_TRANSLATE_NOOP("action","Note duration: 64th"),
         Qt::Key_1,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","64th note (hemidemisemiquaver)"),
         QT_TRANSLATE_NOOP("action","64th note (hemidemisemiquaver)"),
          note64_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-note-128",
         QT_TRANSLATE_NOOP("action","Note duration: 128th"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","128th note"),
         QT_TRANSLATE_NOOP("action","128th note"),
          note128_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-dot",
         QT_TRANSLATE_NOOP("action","Note duration: augmentation dot"),
         Qt::Key_Period,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Augmentation dot"),
         QT_TRANSLATE_NOOP("action","Augmentation dot"),
          dot_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-dotdot",
         QT_TRANSLATE_NOOP("action","Note duration: double augmentation dot"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Double augmentation dot"),
         QT_TRANSLATE_NOOP("action","Double augmentation dot"),
          dotdot_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "tie",
         QT_TRANSLATE_NOOP("action","Note duration: tie"),
         Qt::Key_Plus,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Tie"),
         QT_TRANSLATE_NOOP("action","Tie"),
         tie_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "pad-rest",
         QT_TRANSLATE_NOOP("action","Note entry: rest"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Rest"),
         QT_TRANSLATE_NOOP("action","Rest"),
          quartrest_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "sharp2",
         QT_TRANSLATE_NOOP("action","Note entry: double sharp"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Double sharp"),
         QT_TRANSLATE_NOOP("action","Double sharp"),
          sharpsharp_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "sharp",
         QT_TRANSLATE_NOOP("action","Note entry: sharp"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Sharp"),
         QT_TRANSLATE_NOOP("action","Sharp"),
          sharp_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "nat",
         QT_TRANSLATE_NOOP("action","Note entry: natural"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Natural"),
         QT_TRANSLATE_NOOP("action","Natural"),
          natural_ICON
         ),
       Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "flat",
         QT_TRANSLATE_NOOP("action","Note entry: flat"),
         Qt::Key_Minus,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Flat"),
         QT_TRANSLATE_NOOP("action","Flat"),
          flat_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "flat2",
         QT_TRANSLATE_NOOP("action","Note entry: double flat"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Double flat"),
         QT_TRANSLATE_NOOP("action","Double flat"),
          flatflat_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "acciaccatura",
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
         Qt::Key_Slash,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
          acciaccatura_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "appoggiatura",
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
          appoggiatura_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
	 /* no stroke: 4th*/
        "grace4",
        QT_TRANSLATE_NOOP("action","Grace: quarter"),
        0,
        Qt::WindowShortcut,
        QT_TRANSLATE_NOOP("action","Grace: quarter"),
        QT_TRANSLATE_NOOP("action","Grace: quarter"),
         grace4_ICON
        ),
     Shortcut(
        STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
      /* no stroke: 16th*/
        "grace16",
        QT_TRANSLATE_NOOP("action","Grace: 16th"),
        0,
        Qt::WindowShortcut,
        QT_TRANSLATE_NOOP("action","Grace: 16th"),
        QT_TRANSLATE_NOOP("action","Grace: 16th"),
         grace16_ICON
        ),
     Shortcut(
        STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
      /* no stroke: 32th*/
        "grace32",
        QT_TRANSLATE_NOOP("action","Grace: 32nd"),
        0,
        Qt::WindowShortcut,
        QT_TRANSLATE_NOOP("action","Grace: 32nd"),
        QT_TRANSLATE_NOOP("action","Grace: 32nd"),
         grace32_ICON
        ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "voice-1",
         QT_TRANSLATE_NOOP("action","Voice 1"),
         QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_1),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Voice 1"),
         QT_TRANSLATE_NOOP("action","Voice 1"),
          voice1_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "voice-2",
         QT_TRANSLATE_NOOP("action","Voice 2"),
         QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_2),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Voice 2"),
         QT_TRANSLATE_NOOP("action","Voice 2"),
          voice2_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "voice-3",
         QT_TRANSLATE_NOOP("action","Voice 3"),
         QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_3),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Voice 3"),
         QT_TRANSLATE_NOOP("action","Voice 3"),
          voice3_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "voice-4",
         QT_TRANSLATE_NOOP("action","Voice 4"),
         QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_4),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Voice 4"),
         QT_TRANSLATE_NOOP("action","Voice 4"),
          voice4_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "midi-on",
         QT_TRANSLATE_NOOP("action","MIDI input"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Enable MIDI input"),
         QT_TRANSLATE_NOOP("action","Enable MIDI input"),
          midiin_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "sound-on",
         QT_TRANSLATE_NOOP("action","Editing sound on"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Enable sound while editing"),
         QT_TRANSLATE_NOOP("action","Enable sound while editing"),
          speaker_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "beam-start",
         QT_TRANSLATE_NOOP("action","Beam start"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Beam start"),
         QT_TRANSLATE_NOOP("action","Beam start"),
          sbeam_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "beam-mid",
         QT_TRANSLATE_NOOP("action","Beam middle"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Beam middle"),
         QT_TRANSLATE_NOOP("action","Beam middle"),
          mbeam_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "no-beam",
         QT_TRANSLATE_NOOP("action","No beam"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","No beam"),
         QT_TRANSLATE_NOOP("action","No beam"),
          nbeam_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "beam32",
         QT_TRANSLATE_NOOP("action","Beam 32nd sub"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Beam 32nd sub"),
         QT_TRANSLATE_NOOP("action","Beam 32nd sub"),
          beam32_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "beam64",
         QT_TRANSLATE_NOOP("action","Beam 64th sub"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Beam 64th sub"),
         QT_TRANSLATE_NOOP("action","Beam 64th sub"),
          beam64_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "auto-beam",
         QT_TRANSLATE_NOOP("action","Auto beam"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Auto beam"),
         QT_TRANSLATE_NOOP("action","Auto beam"),
          abeam_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "fbeam1",
         QT_TRANSLATE_NOOP("action","Feathered beam, slower"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Feathered Beam, Slower"),
         QT_TRANSLATE_NOOP("action","Feathered beam, slower"),
          fbeam1_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "fbeam2",
         QT_TRANSLATE_NOOP("action","Feathered beam, faster"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Feathered Beam, Faster"),
         QT_TRANSLATE_NOOP("action","Feathered beam, faster"),
          fbeam2_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         0,
         "toggle-palette",
         QT_TRANSLATE_NOOP("action","Palette"),
#ifdef Q_WS_MAC
		     Qt::CTRL+Qt::ALT+Qt::Key_K,
#else
		     Qt::Key_F9,
#endif
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Palette"),
         QT_TRANSLATE_NOOP("action","Palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         0,
         "toggle-playpanel",
         QT_TRANSLATE_NOOP("action","Play panel"),
#ifdef Q_WS_MAC
		 Qt::CTRL+Qt::ALT+Qt::Key_P,
#else
		 Qt::Key_F11,
#endif
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Play Panel"),
         QT_TRANSLATE_NOOP("action","Play panel")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         0,
         "toggle-navigator",
         QT_TRANSLATE_NOOP("action","Navigator"),
#ifdef Q_WS_MAC
		 Qt::CTRL+Qt::ALT+Qt::Key_N,
#else
		 Qt::Key_F12,
#endif
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Navigator"),
         QT_TRANSLATE_NOOP("action","Navigator")
         ),
      Shortcut(
#ifdef Q_WS_MAC
         //Avoid conflict with M in text
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
#else
	   STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
#endif
         0,
         "toggle-mixer",
         QT_TRANSLATE_NOOP("action","Mixer"),
#ifdef Q_WS_MAC
         //Cmd + M is for minimize to dock on mac
		 Qt::Key_M,
#else
		 Qt::Key_F10,
#endif
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Mixer"),
         QT_TRANSLATE_NOOP("action","Mixer")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "toggle-transport",
         QT_TRANSLATE_NOOP("action","Transport toolbar"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Transport"),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "toggle-noteinput",
         QT_TRANSLATE_NOOP("action","Note input toolbar"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Note Input"),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "toggle-statusbar",
         QT_TRANSLATE_NOOP("action","Status bar"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Status Bar"),
         QT_TRANSLATE_NOOP("action","Status bar")
         ),

      Shortcut(
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY | STATE_FOTO,
         0,
         "quit",
         QT_TRANSLATE_NOOP("action","Quit"),
         Qt::CTRL + Qt::Key_Q,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Quit"),
         QT_TRANSLATE_NOOP("action","Quit"),
          exit_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "mag",
         QT_TRANSLATE_NOOP("action","Zoom canvas"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Zoom Canvas"),
         QT_TRANSLATE_NOOP("action","Zoom canvas"),
          viewmag_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "lyrics",
         QT_TRANSLATE_NOOP("action","Lyrics"),
         Qt::CTRL + Qt::Key_L,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Lyrics"),
         QT_TRANSLATE_NOOP("action","Lyrics")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "fingering",
         QT_TRANSLATE_NOOP("action","Fingering palette"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Fingering..."),
         QT_TRANSLATE_NOOP("action","Fingering palette")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "tempo",
         QT_TRANSLATE_NOOP("action","Tempo"),
         Qt::CTRL+Qt::ALT + Qt::Key_T,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Tempo..."),
         QT_TRANSLATE_NOOP("action","Tempo")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "system-text",
         QT_TRANSLATE_NOOP("action","Add system text"),
         Qt::CTRL + Qt::SHIFT + Qt::Key_T,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","System Text"),
         QT_TRANSLATE_NOOP("action","Add system text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "staff-text",
         QT_TRANSLATE_NOOP("action","Add staff text"),
         Qt::CTRL + Qt::Key_T,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Staff Text"),
         QT_TRANSLATE_NOOP("action","Add staff text")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "frame-text",
         QT_TRANSLATE_NOOP("action","Add text"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Frame Text"),
         QT_TRANSLATE_NOOP("action","Add text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "title-text",
         QT_TRANSLATE_NOOP("action","Add title text"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Title"),
         QT_TRANSLATE_NOOP("action","Add title text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "subtitle-text",
         QT_TRANSLATE_NOOP("action","Add subtitle text"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Subtitle"),
         QT_TRANSLATE_NOOP("action","Add subtitle text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "composer-text",
         QT_TRANSLATE_NOOP("action","Add composer text"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Composer"),
         QT_TRANSLATE_NOOP("action","Add composer text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "poet-text",
         QT_TRANSLATE_NOOP("action","Add lyricist text"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Lyricist"),
         QT_TRANSLATE_NOOP("action","Add lyricist text")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-text",
         QT_TRANSLATE_NOOP("action","Add chord name"),
         Qt::CTRL + Qt::Key_K,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Chord Name"),
         QT_TRANSLATE_NOOP("action","Add chord name")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "harmony-properties",
         QT_TRANSLATE_NOOP("action","Show harmony properties for chord"),
         Qt::SHIFT+Qt::Key_K,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Harmony Properties"),
         QT_TRANSLATE_NOOP("action","Show harmony properties for chord")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rehearsalmark-text",
         QT_TRANSLATE_NOOP("action","Add rehearsal mark"),
         Qt::CTRL + Qt::Key_M,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Rehearsal Mark"),
         QT_TRANSLATE_NOOP("action","Add rehearsal mark")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "picture",
         QT_TRANSLATE_NOOP("action","Add picture"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Picture"),
         QT_TRANSLATE_NOOP("action","Add picture")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "play",
         QT_TRANSLATE_NOOP("action","Player play"),
         Qt::Key_Space,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Play"),
         QT_TRANSLATE_NOOP("action","Start or stop playback"),
          play_ICON
         ),
      Shortcut(
         STATE_PLAY,
         0,
         "play-prev-chord",
         QT_TRANSLATE_NOOP("action","Previous chord"),
         Qt::Key_Left,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Previous chord"),
         QT_TRANSLATE_NOOP("action","Previous chord")
         ),
      Shortcut(
         STATE_PLAY,
         0,
         "play-prev-measure",
         QT_TRANSLATE_NOOP("action","Previous measure"),
         Qt::CTRL+Qt::Key_Left,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Previous measure"),
         QT_TRANSLATE_NOOP("action","Previous measure")
         ),
      Shortcut(
         STATE_PLAY,
         0,
         "play-next-chord",
         QT_TRANSLATE_NOOP("action","Next chord"),
         Qt::Key_Right,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Next chord"),
         QT_TRANSLATE_NOOP("action","Next chord")
         ),
      Shortcut(
         STATE_PLAY,
         0,
         "play-next-measure",
         QT_TRANSLATE_NOOP("action","Next measure"),
         Qt::CTRL+Qt::Key_Right,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Next measure"),
         QT_TRANSLATE_NOOP("action","Next measure")
         ),
      Shortcut(
         STATE_PLAY,
         0,
         "seek-begin",
         QT_TRANSLATE_NOOP("action","Player seek to begin"),
         Qt::Key_Home,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         0,
         "rewind",
         QT_TRANSLATE_NOOP("action","Player rewind"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Rewind"),
         QT_TRANSLATE_NOOP("action","Rewind to start position"),
          start_ICON
         ),
      Shortcut(
         STATE_PLAY,
         0,
         "seek-end",
         QT_TRANSLATE_NOOP("action","Player seek to end"),
         Qt::Key_End,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action",""),
         QT_TRANSLATE_NOOP("action","")
         ),
      Shortcut(
         STATE_NORMAL,
         A_SCORE,
         "repeat",
         QT_TRANSLATE_NOOP("action","Play repeats on/off"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Repeat"),
         QT_TRANSLATE_NOOP("action","Play repeats on/off"),
          repeat_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         A_SCORE,
         "pan",
         QT_TRANSLATE_NOOP("action","Pan score while playing on/off"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Pan"),
         QT_TRANSLATE_NOOP("action","Pan score while playing on/off"),
          pan_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         A_SCORE,
         "load-style",
         QT_TRANSLATE_NOOP("action","Load style"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Load Style..."),
         QT_TRANSLATE_NOOP("action","Load style"),
          fileOpen_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         A_SCORE,
         "save-style",
         QT_TRANSLATE_NOOP("action","Save style"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Save Style..."),
         QT_TRANSLATE_NOOP("action","Save style"),
          fileSave_ICON
         ),
      Shortcut (
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         A_CMD,
         "select-all",
         QT_TRANSLATE_NOOP("action","Select all"),
         Qt::CTRL+Qt::Key_A,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Select A&ll"),
         QT_TRANSLATE_NOOP("action","Select all")
         ),
      Shortcut (
         STATE_NORMAL,
         0,
         "transpose",
         QT_TRANSLATE_NOOP("action","Transpose"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","&Transpose..."),
         QT_TRANSLATE_NOOP("action","Transpose")
         ),
      Shortcut (
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "clef-violin",
         QT_TRANSLATE_NOOP("action","Violin clef"),
         QKeySequence(Qt::CTRL+Qt::Key_Y, Qt::CTRL+Qt::Key_1),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Violin Clef"),
         QT_TRANSLATE_NOOP("action","Violin clef")
         ),
      Shortcut (
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "clef-bass",
         QT_TRANSLATE_NOOP("action","Bass clef"),
         QKeySequence(Qt::CTRL+Qt::Key_Y, Qt::CTRL+Qt::Key_2),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Bass Clef"),
         QT_TRANSLATE_NOOP("action","Bass clef")
         ),
      Shortcut (
         STATE_NORMAL,
         A_CMD,
         "voice-x12",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-2"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-2"),
         QT_TRANSLATE_NOOP("action","Exchange voice 1-2")
         ),
      Shortcut (
         STATE_NORMAL,
         A_CMD,
         "voice-x13",
         QT_TRANSLATE_NOOP("action","Exchange voice 1-3"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-3"),
         QT_TRANSLATE_NOOP("action","Exchange voice 1-3")
         ),
      Shortcut (
         STATE_NORMAL,
         A_CMD,
         "voice-x14",
         QT_TRANSLATE_NOOP("action","Exchange voice 1-4"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-4"),
         QT_TRANSLATE_NOOP("action","Exchange voice 1-4")
         ),
      Shortcut (
         STATE_NORMAL,
         A_CMD,
         "voice-x23",
         QT_TRANSLATE_NOOP("action","Exchange voice 2-3"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Exchange Voice 2-3"),
         QT_TRANSLATE_NOOP("action","Exchange voice 2-3")
         ),
      Shortcut (
         STATE_NORMAL,
         A_CMD,
         "voice-x24",
         QT_TRANSLATE_NOOP("action","Exchange voice 2-4"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Exchange Voice 2-4"),
         QT_TRANSLATE_NOOP("action","Exchange voice 2-4")
         ),
      Shortcut (
         STATE_NORMAL,
         A_CMD,
         "voice-x34",
         QT_TRANSLATE_NOOP("action","Exchange voice 3-4"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Exchange Voice 3-4"),
         QT_TRANSLATE_NOOP("action","Exchange voice 3-4")
         ),
      Shortcut (
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "concert-pitch",
         QT_TRANSLATE_NOOP("action","Display in concert pitch"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Concert Pitch"),
         QT_TRANSLATE_NOOP("action","Display in concert pitch")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "repeat-cmd",
         QT_TRANSLATE_NOOP("action","Repeat last command"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Repeat Last Command"),
         QT_TRANSLATE_NOOP("action","Repeat last command"),
          fileOpen_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_SCORE,
         "edit-meta",
         QT_TRANSLATE_NOOP("action","Edit score meta data"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Meta Data..."),
         QT_TRANSLATE_NOOP("action","Edit score meta data")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "system-break",
         QT_TRANSLATE_NOOP("action","Toggle system break"),
         Qt::Key_Return,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Toggle System Break"),
         QT_TRANSLATE_NOOP("action","Toggle system break")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "page-break",
         QT_TRANSLATE_NOOP("action","Toggle page break"),
         Qt::CTRL+Qt::Key_Return,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Toggle Page Break"),
         QT_TRANSLATE_NOOP("action","Toggle page break")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "section-break",
         QT_TRANSLATE_NOOP("action","Toggle section break"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Toggle Section Break"),
         QT_TRANSLATE_NOOP("action","Toggle section break")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "edit-element",
         QT_TRANSLATE_NOOP("action","Edit element"),
         Qt::CTRL+Qt::Key_E,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Edit Element"),
         QT_TRANSLATE_NOOP("action","Edit element")
         ),
      Shortcut(
         STATE_NORMAL | STATE_EDIT,
         0,
         "reset",
         QT_TRANSLATE_NOOP("action","Reset user settings"),
         Qt::CTRL+Qt::Key_R,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Reset"),
         QT_TRANSLATE_NOOP("action","Reset user settings")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "debugger",
         QT_TRANSLATE_NOOP("action","Show debugger"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Debugger"),
         QT_TRANSLATE_NOOP("action","Show debugger")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "reset-stretch",
         QT_TRANSLATE_NOOP("action","Reset measure stretch"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Reset Stretch"),
         QT_TRANSLATE_NOOP("action","Reset measure stretch")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_SCORE,
         "show-invisible",
         QT_TRANSLATE_NOOP("action","Show invisible"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Show Invisible"),
         QT_TRANSLATE_NOOP("action","Show invisible")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_SCORE,
         "show-unprintable",
         QT_TRANSLATE_NOOP("action","Show unprintable"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Show unprintable"),
         QT_TRANSLATE_NOOP("action","Show unprintable")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_SCORE,
         "show-frames",
         QT_TRANSLATE_NOOP("action","Show frames"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Show Frames"),
         QT_TRANSLATE_NOOP("action","Show frames")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_SCORE,
         "show-pageborders",
         QT_TRANSLATE_NOOP("action","Show Page Margins"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Show Page Margins"),
         QT_TRANSLATE_NOOP("action","Show Page Margins")
         ),
      Shortcut(
         STATE_EDIT,
         0,
         "show-keys",
         QT_TRANSLATE_NOOP("action","Insert text symbol"),
         Qt::Key_F2,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Insert Text Symbol..."),
         QT_TRANSLATE_NOOP("action","Insert special characters and text symbols"),
          keys_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "script-debug",
         QT_TRANSLATE_NOOP("action","Enable script debugger"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Enable Script Debugger"),
         QT_TRANSLATE_NOOP("action","Enable script debugger")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest-1",
         QT_TRANSLATE_NOOP("action","Note entry: whole rest"),
         QKeySequence(Qt::SHIFT+Qt::Key_R, Qt::SHIFT+Qt::Key_S),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Whole rest"),
         QT_TRANSLATE_NOOP("action","Whole rest")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest-2",
         QT_TRANSLATE_NOOP("action","Note entry: half rest"),
         QKeySequence(Qt::SHIFT+Qt::Key_R, Qt::SHIFT+Qt::Key_M),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Half rest"),
         QT_TRANSLATE_NOOP("action","Half rest")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest-4",
         QT_TRANSLATE_NOOP("action","Note entry: quarter rest"),
         QKeySequence(Qt::SHIFT+Qt::Key_R, Qt::SHIFT+Qt::Key_R),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Quarter rest"),
         QT_TRANSLATE_NOOP("action","Quarter rest")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest-8",
         QT_TRANSLATE_NOOP("action","Note entry: 8th rest"),
         QKeySequence(Qt::SHIFT+Qt::Key_R, Qt::SHIFT+Qt::Key_Q),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","8th rest"),
         QT_TRANSLATE_NOOP("action","8th rest")
         ),
      Shortcut(                     // mapped to undo in note entry mode
         STATE_NOTE_ENTRY,
         0,
         "backspace",
         QT_TRANSLATE_NOOP("action","Backspace"),
         QKeySequence(Qt::Key_Backspace),    // QKeySequence::Back,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Backspace"),
         QT_TRANSLATE_NOOP("action","Backspace")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_SEARCH,
         0,
         "find",
         QT_TRANSLATE_NOOP("action","Search"),
         QKeySequence::Find,        // Qt::CTRL + Qt::Key_F,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Search"),
         QT_TRANSLATE_NOOP("action","Search")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "zoomin",
         QT_TRANSLATE_NOOP("action","Zoom in"),
         Qt::CTRL + Qt::Key_Plus,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Zoom In")
         ),
      Shortcut(
         // conflicts with Ctrl+- in edit mode to enter lyrics hyphen
         // STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,

         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "zoomout",
         QT_TRANSLATE_NOOP("action","Zoom out"),
         Qt::CTRL + Qt::Key_Minus,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Zoom Out")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "mirror-note",
         QT_TRANSLATE_NOOP("action","Mirror note head"),
         Qt::SHIFT + Qt::Key_X,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Mirror note head"),
         QT_TRANSLATE_NOOP("action","Mirror note head"),
          flip_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "edit-style",
         QT_TRANSLATE_NOOP("action","Edit general style"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Edit General Style..."),
         QT_TRANSLATE_NOOP("action","Edit general style")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "edit-text-style",
         QT_TRANSLATE_NOOP("action","Edit text style"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Edit Text Style..."),
         QT_TRANSLATE_NOOP("action","Edit text style")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "edit-harmony",
         QT_TRANSLATE_NOOP("action","Edit chord style"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Edit Chord Style..."),
         QT_TRANSLATE_NOOP("action","Edit chord style")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-similar",
         QT_TRANSLATE_NOOP("action","Select all similar elements"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","All Similar Elements"),
         QT_TRANSLATE_NOOP("action","Select all similar elements")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-similar-staff",
         QT_TRANSLATE_NOOP("action","Select all similar elements in same staff"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","All Similar Elements in Same Staff"),
         QT_TRANSLATE_NOOP("action","Select all similar elements in same staff")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "synth-control",
         QT_TRANSLATE_NOOP("action","Synthesizer"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Synthesizer"),
         QT_TRANSLATE_NOOP("action","Synthesizer")
         ),
      Shortcut(
         STATE_NOTE_ENTRY,
         A_CMD,
         "double-duration",
         QT_TRANSLATE_NOOP("action","Double duration"),
         QKeySequence(Qt::Key_W),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Double duration"),
         QT_TRANSLATE_NOOP("action","Double duration")
         ),
      Shortcut(
         STATE_NOTE_ENTRY,
         A_CMD,
         "half-duration",
         QT_TRANSLATE_NOOP("action","Half duration"),
         QKeySequence(Qt::Key_Q),
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Half duration"),
         QT_TRANSLATE_NOOP("action","Half duration")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "repeat-sel",
         QT_TRANSLATE_NOOP("action","Repeat selection"),
         Qt::Key_R,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Repeat selection"),
         QT_TRANSLATE_NOOP("action","Repeat selection"),
          fileOpen_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "follow",
         QT_TRANSLATE_NOOP("action","Follow song"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Follow Song"),
         QT_TRANSLATE_NOOP("action","Follow song")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "split-h",
         QT_TRANSLATE_NOOP("action","Display documents side by side"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Documents Side by Side"),
         QT_TRANSLATE_NOOP("action","Display documents side by side")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "split-v",
         QT_TRANSLATE_NOOP("action","Display documents stacked"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Documents Stacked"),
         QT_TRANSLATE_NOOP("action","Display documents stacked")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "parts",
         QT_TRANSLATE_NOOP("action","Parts..."),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Parts..."),
         QT_TRANSLATE_NOOP("action","Parts...")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "enh-up",
         QT_TRANSLATE_NOOP("action","Enharmonic up"),
         Qt::Key_J,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Enharmonic up"),
         QT_TRANSLATE_NOOP("action","Enharmonic up")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "enh-down",
         QT_TRANSLATE_NOOP("action","Enharmonic down"),
         Qt::CTRL+Qt::Key_J,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Enharmonic down"),
         QT_TRANSLATE_NOOP("action","Enharmonic down")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "revision",
         QT_TRANSLATE_NOOP("action","Create new revision"),
	   Qt::CTRL+Qt::Key_F11,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Create new revision"),
         QT_TRANSLATE_NOOP("action","Create new revision")
         ),
      Shortcut(
         STATE_NORMAL | STATE_FOTO,
         0,
         "fotomode",
         QT_TRANSLATE_NOOP("action","Toggle foto mode"),
	   0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Toggle foto mode"),
         QT_TRANSLATE_NOOP("action","Toggle foto mode"),
         fotomode_ICON
         ),
      Shortcut(
         STATE_EDIT,
         0,
         "toggle-styled",
         QT_TRANSLATE_NOOP("action","Toggle styled"),
	   0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Styled"),
         QT_TRANSLATE_NOOP("action","Toggle styled")
         ),
      Shortcut(
         STATE_ALL,
         A_CMD,
         "add-audio",
         QT_TRANSLATE_NOOP("action","Add audio track"),
	   0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Add Audio Track"),
         QT_TRANSLATE_NOOP("action","Add audio track")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "show-omr",
         QT_TRANSLATE_NOOP("action","Show score image"),
         Qt::Key_O,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Show OMR image"),
         QT_TRANSLATE_NOOP("action","Show OMR image")
         ),
      Shortcut(
         STATE_ALL,
         0,
         "fullscreen",
         QT_TRANSLATE_NOOP("action","Full screen"),
	   Qt::CTRL + Qt::Key_U,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","F&ull Screen"),
         QT_TRANSLATE_NOOP("action","F&ull screen")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "hraster",
         QT_TRANSLATE_NOOP("action","Enable horizontal raster"),
	   0,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Enable horizontal raster"),
         QT_TRANSLATE_NOOP("action","Enable horizontal raster"),
         hraster_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "vraster",
         QT_TRANSLATE_NOOP("action","Enable vertical raster"),
	   0,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Enable vertical raster"),
         QT_TRANSLATE_NOOP("action","Enable vertical raster"),
         vraster_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "config-raster",
         QT_TRANSLATE_NOOP("action","Configure raster"),
	   0,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Configure raster"),
         QT_TRANSLATE_NOOP("action","Configure raster")
         ),
      Shortcut(
         STATE_NOTE_ENTRY,
         A_CMD,
         "repitch",
         QT_TRANSLATE_NOOP("action","Re-pitch mode"),
	   Qt::CTRL + Qt::SHIFT + Qt::Key_I,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Re-Pitch Mode"),
         QT_TRANSLATE_NOOP("action","Replace pitches without changing rhythms"),
         repitch_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "stack-down",
         QT_TRANSLATE_NOOP("action","Stack down"),
         Qt::SHIFT + Qt::Key_Z,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Stack down"),
         QT_TRANSLATE_NOOP("action","Stack down")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "toogle-piano",
         QT_TRANSLATE_NOOP("action","Piano Keyboard"),
         Qt::Key_P,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Piano Keyboard"),
         QT_TRANSLATE_NOOP("action","Piano Keyboard")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY | STATE_FOTO,
         0,
         "media",
         QT_TRANSLATE_NOOP("action","Show media dialog"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Additional Media"),
         QT_TRANSLATE_NOOP("action","Show media dialog")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "split-measure",
         QT_TRANSLATE_NOOP("action","Split Measure"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Split Measure"),
         QT_TRANSLATE_NOOP("action","Split Measure")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "join-measure",
         QT_TRANSLATE_NOOP("action","Join Measure"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Join Measure"),
         QT_TRANSLATE_NOOP("action","Join Measure")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY | STATE_FOTO,
         0,
         "page-settings",
         QT_TRANSLATE_NOOP("action","Page Settings"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Page Settings..."),
         QT_TRANSLATE_NOOP("action","Page Settings")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "album",
         QT_TRANSLATE_NOOP("action","Album"),
         0,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Album..."),
         QT_TRANSLATE_NOOP("action","Album")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "layer",
         QT_TRANSLATE_NOOP("action","Layer"),
         0,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Layer..."),
         QT_TRANSLATE_NOOP("action","Layer")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "next-score",
         QT_TRANSLATE_NOOP("action","next score"),
         QKeySequence::NextChild,
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","next score"),
         QT_TRANSLATE_NOOP("action","next score")
         ),
      Shortcut(
         STATE_NORMAL,
         0,
         "previous-score",
         QT_TRANSLATE_NOOP("action","previous score"),
#if defined(Q_WS_WIN)
          // Qt bug on windows : http://bugreports.qt.nokia.com/browse/QTBUG-15746
         Qt::CTRL+Qt::SHIFT+Qt::Key_Tab,
#else
         QKeySequence::PreviousChild,
#endif
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","previous score"),
         QT_TRANSLATE_NOOP("action","previous score")
         ),
      Shortcut(
         STATE_INIT | STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY | STATE_SEARCH | STATE_FOTO,
         0,
         "musescore-connect",
         "MuseScore Connect",
         Qt::Key_F7,
         Qt::ApplicationShortcut,
         "MuseScore Connect",
         "MuseScore Connect",
         community_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT,
         0,
         "inspector",
         QT_TRANSLATE_NOOP("action","Show Inspector"),
         Qt::Key_F8,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","inspector"),
         QT_TRANSLATE_NOOP("action","Show inspector")
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT,
         0,
         "metronome",
         QT_TRANSLATE_NOOP("action","Metronome"),
         0,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","metronome"),
         QT_TRANSLATE_NOOP("action","toggle metronome"),
         metronome_ICON
         ),
      Shortcut(
         STATE_NORMAL | STATE_NOTE_ENTRY,
         A_CMD,
         "figured-bass",
         QT_TRANSLATE_NOOP("action","Figured Bass"),
         Qt::SHIFT + Qt::Key_L,
         Qt::WindowShortcut,
         QT_TRANSLATE_NOOP("action","Figured Bass"),
         QT_TRANSLATE_NOOP("action","Figured Bass")
         ),
      // xml==0  marks end of list
      Shortcut(0, 0, 0, 0, QKeySequence::UnknownKey)
      };

