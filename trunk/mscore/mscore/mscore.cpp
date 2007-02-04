//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mscore.cpp,v 1.105 2006/09/15 09:34:57 wschweer Exp $
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

#include <signal.h>
#include <fenv.h>
#include "config.h"

#include "mscore.h"
#include "canvas.h"
#include "style.h"
#include "score.h"
#include "instrdialog.h"
#include "preferences.h"
#include "config.h"
#include "icons.h"
#include "textstyle.h"
#include "xml.h"
#include "seq.h"
#include "icons.h"
#include "tempo.h"
#include "padstate.h"
#include "pad.h"
#include "sym.h"

#include "data/filenew.xpm"
#include "data/fileopen.xpm"
#include "data/filesave.xpm"
#include "data/viewmag.xpm"

#include "padids.h"
#include "pad.h"
#include "alsa.h"
#include "pagesettings.h"
#include "listedit.h"
#include "editstyle.h"
#include "playpanel.h"
#include "page.h"

QTextStream cout(stdout);
QTextStream eout(stderr);

PadState padState;
QString mscoreGlobalShare;
QString language;

const char* magTable[] = {
     "25%", "50%", "75%", "100%", "150%", "200%", "400%", "800%", "1600%",
      QT_TRANSLATE_NOOP("magTable","PgeWidth"),
      QT_TRANSLATE_NOOP("magTable","Page"),
      QT_TRANSLATE_NOOP("magTable","DblPage"),
     };

const char* fileOpenText       = QT_TR_NOOP("Load Score from File");
const char* fileSaveText       = QT_TR_NOOP("Save Score to File");
const char* fileNewText        = QT_TR_NOOP("Create New Score");
const char* filePrintText      = QT_TR_NOOP("Print Score");
const char* infoStartButton    = QT_TR_NOOP("rewind to start position");
const char* infoStopButton     = QT_TR_NOOP("stop sequencer");
const char* infoPlayButton     = QT_TR_NOOP("start sequencer play");

static unsigned int startMag = 3;   // 100%, synchronize with canvas default

const char* eventRecordFile;

static bool haveMidi;

struct ProjectList {
      QString name;
      bool loaded;
      bool current;
      };
ProjectList projectList[PROJECT_LIST_LEN];

int appDpiX = 75;
int appDpiY = 75;
double DPI, DPMM;

QMap<QString, Shortcut*> shortcuts;

//---------------------------------------------------------
//    initial list of shortcuts
//---------------------------------------------------------

Shortcut MuseScore::sc[] = {
      Shortcut(
         "print",
         QT_TR_NOOP("print"),
         Qt::CTRL+Qt::Key_P,
         Qt::WindowShortcut,
         QT_TR_NOOP("Print"),
         QT_TR_NOOP("Print"),
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
         QT_TR_NOOP("Rewind"),
         &startIcon
         ),
      Shortcut(
         "stop",
         QT_TR_NOOP("player stop"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Stop"),
         QT_TR_NOOP("Stop"),
         &stopIcon
         ),
      Shortcut(
         "play",
         QT_TR_NOOP("plaser play"),
         0,
         Qt::WindowShortcut,
         QT_TR_NOOP("Play"),
         QT_TR_NOOP("Play"),
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
      Shortcut(0, 0, 0),
      };

//---------------------------------------------------------
//   getSharePath
//---------------------------------------------------------

static QString getSharePath()
      {
      return QString( INSTPREFIX "/share/" INSTALL_NAME);
      }

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion(const char* prog)
      {
      // fprintf(stderr, "%s: Linux Music Score Editor; Version %s\n", prog, VERSION);
      cout << prog << ": Linux Music Score Editor; Version " << VERSION << endl;
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* ev)
      {
      for (QList<Score*>::iterator i = scoreList.begin(); i != scoreList.end(); ++i) {
            Score* score = *i;
            if (score->dirty()) {
                  QString s(tr("%1 contains unsaved data\n"
                    "Save Current Score?"));
                  s = s.arg(score->projectName());
                  int n = QMessageBox::warning(this, tr("MuseScore"),
                     s,
                     tr("&Save"), tr("&Nosave"), tr("&Abort"), 0, 2);
                  if (n == 0)
                        saveFile();
                  else if (n == 2) {
                        ev->ignore();
                        return;
                        }
                  }
            }
      saveScoreList();
      seq->exit();
      ev->accept();
      //
      // close all toplevel windows
      //
      if (pageListEdit)
            pageListEdit->close();
      if (pad)
            pad->close();
      if (playPanel)
            playPanel->close();
      if (symbolPalette1)
            symbolPalette1->close();
      if (clefPalette)
            clefPalette->close();
      if (keyPalette)
            keyPalette->close();
      if (timePalette)
            timePalette->close();
      if (linePalette)
            linePalette->close();
      if (bracketPalette)
            bracketPalette->close();
      if (barPalette)
            barPalette->close();
      if (fingeringPalette)
            fingeringPalette->close();
      if (noteAttributesPalette)
            noteAttributesPalette->close();
      if (accidentalsPalette)
            accidentalsPalette->close();
      if (dynamicsPalette)
            dynamicsPalette->close();
      if (layoutBreakPalette)
            layoutBreakPalette->close();
      }

//---------------------------------------------------------
//   preferencesChanged
//---------------------------------------------------------

void MuseScore::preferencesChanged()
      {
      if (preferences.bgUseColor)
            canvas->setBackground(preferences.bgColor);
      else {
            QPixmap* pm = new QPixmap(preferences.bgWallpaper);
            canvas->setBackground(pm);
            }
      if (preferences.fgUseColor)
            canvas->setForeground(preferences.fgColor);
      else {
            QPixmap* pm = new QPixmap(preferences.fgWallpaper);
            if (pm == 0 || pm->isNull())
                  printf("no valid pixmap %s\n", preferences.fgWallpaper.toLatin1().data());
            canvas->setForeground(pm);
            }
      transportAction->setEnabled(seq->isRunning());
      transportId->setChecked(seq->isRunning());
      transportTools->setShown(seq->isRunning());
      getAction("midi-on")->setEnabled(preferences.enableMidiInput);
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      mscore = this;
      setIconSize(QSize(ICON_HEIGHT, ICON_HEIGHT));
      setWindowTitle(QString("MuseScore"));
      cs                    = 0;
      textStyleDialog       = 0;
      editStyleWin          = 0;
      instrList             = 0;
      playPanel             = 0;
      preferenceDialog      = 0;
      measuresDialog        = 0;
      iledit                = 0;
      pageListEdit          = 0;
      measureListEdit       = 0;
      symbolPalette1        = 0;
      clefPalette           = 0;
      keyPalette            = 0;
      timePalette           = 0;
      barPalette            = 0;
      fingeringPalette      = 0;
      linePalette           = 0;
      bracketPalette        = 0;
      dynamicsPalette       = 0;
      pageSettings          = 0;
      noteAttributesPalette = 0;
      accidentalsPalette    = 0;
      layoutBreakPalette    = 0;
      pad                   = 0;
      _midiinEnabled        = true;
      _speakerEnabled       = true;

      QAction* a;

      // otherwise unused actions:
      //   must be added somewere to work

      QActionGroup* ag = new QActionGroup(this);
      QStringList sl;
      sl << "page-prev" << "page-next" << "page-top" << "page-end"
         << "add-tie" << "add-slur" << "add-hairpin" << "add-hairpin-reverse"
         << "escape" << "delete" << "rest" << "pitch-up" << "pitch-down"
         << "pitch-up-octave" << "pitch-down-octave"
         << "move-up" << "move-down" << "up-chord" << "down-chord"
         << "top-chord" << "bottom-chord" << "next-chord" << "prev-chord"
         << "next-measure" << "prev-measure" << "print" << "undo"
         << "redo" << "append-measure" << "duole" << "triole" << "pentole"
         << "note-c" << "note-d" << "note-e" << "note-f" << "note-g"
         << "note-a" << "note-b"
         << "chord-c" << "chord-d" << "chord-e" << "chord-f" << "chord-g"
         << "chord-a" << "chord-b"
         << "stretch+" << "stretch-"
         << "instruments" << "clefs" << "keys" << "symbols" << "times" << "dynamics"
         << "cut" << "copy" << "paste"
         << "beam-start" << "beam-mid" << "no-beam" << "beam32"
         ;
      foreach(const QString s, sl) {
            QAction* a = getAction(s.toLatin1().data());
            addAction(a);
            ag->addAction(a);
            }
      connect(ag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      QWidget* mainWindow = new QWidget;
      layout = new QVBoxLayout;
      mainWindow->setLayout(layout);
      layout->setMargin(0);
      layout->setSpacing(0);
      tab = new TabBar;
      QHBoxLayout* hbox = new QHBoxLayout;
      hbox->addWidget(tab);
      hbox->addStretch(100);
      canvas = new Canvas;
      layout->addLayout(hbox);
      layout->addWidget(canvas);

      connect(tab, SIGNAL(currentChanged(int)), SLOT(setCurrentScore(int)));
      connect(tab, SIGNAL(doubleClick(int)), SLOT(removeTab(int)));

      QPixmap newIcon(filenew_xpm);
      QPixmap saveIcon(filesave_xpm);
      QPixmap openIcon(fileopen_xpm);
      QIcon exitIcon(":/data/exit.svg");
      QPixmap viewmagIcon(viewmag_xpm);

      QAction* whatsThis = QWhatsThis::createAction(this);

      //---------------------------------------------------
      //    Transport Action
      //---------------------------------------------------

      a  = getAction("midi-on");
      a->setCheckable(true);
      a->setEnabled(preferences.enableMidiInput);
      a->setChecked(_midiinEnabled);
      connect(a, SIGNAL(triggered(bool)), SLOT(midiinToggled(bool)));

      a = getAction("sound-on");
      a->setCheckable(true);
      a->setEnabled(preferences.playNotes);
      a->setChecked(_speakerEnabled);
      connect(a, SIGNAL(triggered(bool)), SLOT(speakerToggled(bool)));

      transportAction = new QActionGroup(this);
      transportAction->setExclusive(true);

      a = getAction("rewind");
      a->setWhatsThis(tr(infoStartButton));
      transportAction->addAction(a);
      connect(a, SIGNAL(triggered()), seq, SLOT(rewindStart()));

      a = getAction("stop");
      a->setCheckable(true);
      a->setChecked(true);
      a->setWhatsThis(tr(infoStopButton));
      transportAction->addAction(a);
      connect(a, SIGNAL(toggled(bool)), this, SLOT(setStop(bool)));

      a = getAction("play");
      a->setCheckable(true);
      a->setWhatsThis(tr(infoPlayButton));
      transportAction->addAction(a);
      connect(a, SIGNAL(toggled(bool)), this, SLOT(setPlay(bool)));

      //---------------------------------------------------
      //    File Action
      //---------------------------------------------------

      QAction* fileNewAction = new QAction(QIcon(newIcon), tr("&New"), this);
      fileNewAction->setToolTip(tr(fileNewText));

      QAction* fileOpenAction = new QAction(QIcon(openIcon), tr("&Open"), this);
      fileOpenAction->setToolTip(tr(fileOpenText));

      QAction* fileSaveAction = new QAction(QIcon(saveIcon), tr("&Save"), this);
      fileSaveAction->setToolTip(tr(fileSaveText));

      connect(fileNewAction,   SIGNAL(triggered()), SLOT(newFile()));
      connect(fileOpenAction,  SIGNAL(triggered()), SLOT(loadFile()));
      connect(fileSaveAction,  SIGNAL(triggered()), SLOT(saveFile()));

      //---------------------
      //    Tool Bar
      //---------------------

      fileTools = addToolBar(tr("File Operations"));
      fileTools->addAction(fileNewAction);
      fileTools->addAction(fileOpenAction);
      fileTools->addAction(fileSaveAction);
      fileTools->addAction(getAction("print"));
      fileTools->addAction(whatsThis);
      fileTools->addSeparator();
      fileTools->addAction(getAction("undo"));
      fileTools->addAction(getAction("redo"));
      fileTools->addSeparator();

      transportTools = addToolBar(tr("Transport Tools"));
      transportTools->addAction(getAction("sound-on"));
      transportTools->addAction(getAction("midi-on"));
      transportTools->addSeparator();
      transportTools->addAction(getAction("rewind"));
      transportTools->addAction(getAction("stop"));
      transportTools->addAction(getAction("play"));

      QAction* magAction = new QAction(QIcon(viewmagIcon), tr("Mag"), fileTools);
      magAction->setWhatsThis(tr("Zoom Canvas"));
      fileTools->addAction(magAction);
      connect(magAction, SIGNAL(triggered()), canvas, SLOT(magCanvas()));

      mag = new QComboBox(fileTools);
      mag->setToolTip(tr("Mag"));
      mag->setWhatsThis(tr("Zoom Canvas"));

      //
      // FIXME: the mag combobox is not editable but if we set
      //        setEditable(false) we also cannot display arbitrary
      //        mag values
      mag->setEditable(true);

      mag->setValidator(new QDoubleValidator(.05, 20.0, 2, mag));
      for (unsigned int i =  0; i < sizeof(magTable)/sizeof(*magTable); ++i) {
            mag->addItem(tr(magTable[i]), i);
            if (i == startMag)
                  mag->setCurrentIndex(i);
            }
      connect(mag, SIGNAL(activated(int)), SLOT(magChanged(int)));
      fileTools->addWidget(mag);
      addToolBarBreak();

      //-------------------------------
      //    Note Entry Tool Bar
      //-------------------------------

      entryTools = addToolBar(tr("Note Entry"));
      entryTools->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));

      QStringList sl1;
      sl1 << "pad-note-1" << "pad-note-2" << "pad-note-4" << "pad-note-8"
         << "pad-note-16" << "pad-note-32" << "pad-note-64" << "pad-dot"
         << "pad-tie" << "pad-rest" << "pad-sharp2" << "pad-sharp"
         << "pad-nat" << "pad-flat" << "pad-flat2";
      foreach(const QString s, sl1) {
            QAction* a = getAction(s.toLatin1().data());
            a->setCheckable(true);
            ag->addAction(a);
            entryTools->addAction(a);
            if (s == "pad-tie" || s == "pad-rest")
                  entryTools->addSeparator();
            }

      a = getAction("flip");
      ag->addAction(a);
      entryTools->addAction(a);
      entryTools->addSeparator();

      QStringList sl2;
      sl2 << "voice-1" << "voice-2" << "voice-3" << "voice-4";
      QActionGroup* vag = new QActionGroup(this);
      vag->setExclusive(true);
      foreach(const QString s, sl2) {
            QAction* a = getAction(s.toLatin1().data());
            a->setCheckable(true);
            vag->addAction(a);
            entryTools->addAction(a);
            }
      connect(vag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      //---------------------
      //    Menus
      //---------------------

      QMenuBar* mb = menuBar();

      //---------------------
      //    Menu File
      //---------------------

      QMenu* menuFile = mb->addMenu(tr("&File"));

      menuFile->addAction(fileNewAction);
      menuFile->addAction(fileOpenAction);
      openRecent = menuFile->addMenu(QIcon(openIcon), tr("Open &Recent"));
      connect(openRecent, SIGNAL(aboutToShow()), SLOT(openRecentMenu()));
      connect(openRecent, SIGNAL(triggered(QAction*)), SLOT(selectScore(QAction*)));
      menuFile->addSeparator();
      menuFile->addAction(fileSaveAction);
      menuFile->addAction(QIcon(saveIcon), tr("Save &As"), this, SLOT(saveAs()), Qt::CTRL + Qt::Key_A);
      menuFile->addSeparator();
      menuFile->addAction(QIcon(saveIcon), tr("Export Midi"),     this, SLOT(exportMidi()));
      menuFile->addAction(QIcon(saveIcon), tr("Export MusicXML"), this, SLOT(exportMusicXml()));
      menuFile->addAction(QIcon(openIcon), tr("Import Midi"),     this, SLOT(importMidi()));
      menuFile->addAction(QIcon(openIcon), tr("Import MusicXML"), this, SLOT(importMusicXml()));
      menuFile->addSeparator();
      menuFile->addAction(getAction("print"));
      menuFile->addSeparator();
      menuFile->addAction(exitIcon, tr("&Quit"), this, SLOT(close()), Qt::CTRL + Qt::Key_Q);

      //---------------------
      //    Menu Edit
      //---------------------

      menuEdit = mb->addMenu(tr("&Edit"));

      menuEdit->addAction(getAction("undo"));
      menuEdit->addAction(getAction("redo"));

      menuEdit->addSeparator();

      menuEdit->addAction(getAction("cut"));
      menuEdit->addAction(getAction("copy"));
      a = getAction("paste");
      a->setEnabled(false);
      menuEdit->addAction(a);
      selectionChanged(0);
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Instrument List..."), this, SLOT(startInstrumentListEditor()));
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Page List..."), this, SLOT(startPageListEditor()));
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Preferences..."), this, SLOT(startPreferenceDialog()));

      //---------------------
      //    Menu Create
      //---------------------

      QMenu* menuCreate = genCreateMenu();
      mb->addMenu(menuCreate);

      //---------------------
      //    Menu Notes
      //---------------------

      QMenu* menuNotes = mb->addMenu(tr("Notes"));

      a = getAction("note-input");
      connect(a, SIGNAL(triggered()), SLOT(startNoteEntry()));
      menuNotes->addAction(a);
      menuNotes->addSeparator();

      QMenu* menuAddPitch = new QMenu(tr("Add Note"));
      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            sprintf(buffer, "note-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }
      menuNotes->addMenu(menuAddPitch);

      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            sprintf(buffer, "chord-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }

      QMenu* menuAddIntervall = new QMenu(tr("Add Intervall"));
      for (int i = 1; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "intervall%d", i);
            a = getAction(buffer);
            ag->addAction(a);
            menuAddIntervall->addAction(a);
            }
      menuAddIntervall->addSeparator();
      for (int i = 2; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "intervall-%d", i);
            a = getAction(buffer);
            ag->addAction(a);
            menuAddIntervall->addAction(a);
            }
      menuNotes->addMenu(menuAddIntervall);

      QMenu* menuNtole = new QMenu(tr("Tuples"));
      menuNtole->addAction(getAction("duole"));
      menuNtole->addAction(getAction("triole"));
      menuNtole->addAction(getAction("pentole"));
      menuNotes->addMenu(menuNtole);

      //---------------------
      //    Menu Layout
      //---------------------

      QMenu* menuLayout = mb->addMenu(tr("&Layout"));

      menuLayout->addAction(tr("Page Settings..."), this, SLOT(showPageSettings()));
      menuLayout->addAction(tr("Reset Positions"),  this, SLOT(resetUserOffsets()));
      menuLayout->addAction(tr("Set Normal Staff Distances"),  canvas, SLOT(resetStaffOffsets()));
      menuLayout->addAction(getAction("stretch+"));
      menuLayout->addAction(getAction("stretch-"));

      menuLayout->addAction(tr("Reset Stretch"), this, SLOT(resetUserStretch()));
      menuLayout->addAction(tr("Breaks..."), this, SLOT(showLayoutBreakPalette()));

      //---------------------
      //    Menu Style
      //---------------------

      QMenu* menuStyle = mb->addMenu(tr("&Style"));
      menuStyle->addAction(tr("Edit Style..."), this, SLOT(editStyle()));
      menuStyle->addAction(tr("Edit Text Style..."), this, SLOT(editTextStyle()));
      menuStyle->addSeparator();
      menuStyle->addAction(QIcon(openIcon), tr("Load Style"), this, SLOT(loadStyle()));
      menuStyle->addAction(QIcon(saveIcon), tr("Save Style"), this, SLOT(saveStyle()));

      //---------------------
      //    Menu Display
      //---------------------

      menuDisplay = mb->addMenu(tr("&Display"));

      padId = new QAction(tr("Pad"), this);
      padId->setShortcut(Qt::Key_F10);
      padId->setShortcutContext(Qt::ApplicationShortcut);
      padId->setCheckable(true);
      connect(padId, SIGNAL(toggled(bool)), SLOT(showPad(bool)));
      menuDisplay->addAction(padId);

      playId = new QAction(tr("PlayPanel"), this);
      playId->setCheckable(true);
      playId->setShortcut(Qt::Key_F11);
      playId->setShortcutContext(Qt::ApplicationShortcut);
      connect(playId, SIGNAL(toggled(bool)), SLOT(showPlayPanel(bool)));
      menuDisplay->addAction(playId);

      navigatorId = new QAction(tr("Navigator"), this);
      navigatorId->setCheckable(true);
      navigatorId->setShortcut(Qt::Key_F12);
      navigatorId->setShortcutContext(Qt::ApplicationShortcut);
      connect(navigatorId, SIGNAL(toggled(bool)), SLOT(showNavigator(bool)));
      menuDisplay->addAction(navigatorId);

      menuDisplay->addSeparator();

      transportId = new QAction(tr("Transport Toolbar"), this);
      transportId->setCheckable(true);
      menuDisplay->addAction(transportId);
      connect(transportId, SIGNAL(toggled(bool)), transportTools, SLOT(setVisible(bool)));

      inputId = new QAction(tr("Note Input Toolbar"), this);
      inputId->setCheckable(true);
      inputId->setChecked(true);
      menuDisplay->addAction(inputId);
      connect(inputId, SIGNAL(toggled(bool)), entryTools, SLOT(setVisible(bool)));

      // if we have no sequencer, disable transport and play panel
      if (!seq->isRunning()) {
            transportId->setEnabled(false);
            playId->setEnabled(false);
            }

      menuDisplay->addSeparator();
      visibleId = menuDisplay->addAction(tr("Show Invisible"), this, SLOT(showInvisibleClicked()));
      visibleId->setCheckable(true);

      //---------------------
      //    Menu Help
      //---------------------

      mb->addSeparator();
      QMenu* menuHelp = mb->addMenu(tr("&Help"));

      menuHelp->addAction(tr("Manual"),  this, SLOT(helpBrowser()), Qt::Key_F1);
      menuHelp->addAction(tr("&About"),   this, SLOT(about()));
      menuHelp->addAction(tr("About&Qt"), this, SLOT(aboutQt()));
      menuHelp->addSeparator();
      menuHelp->addAction(whatsThis);

      setCentralWidget(mainWindow);
      connect(canvas, SIGNAL(magChanged()), SLOT(updateMag()));

      loadInstrumentTemplates();
      preferencesChanged();
      connect(seq, SIGNAL(started()), SLOT(seqStarted()));
      connect(seq, SIGNAL(stopped()), SLOT(seqStopped()));
      loadScoreList();

      QClipboard* cb = QApplication::clipboard();
      connect(cb, SIGNAL(dataChanged()), SLOT(clipboardChanged()));
      connect(cb, SIGNAL(selectionChanged()), SLOT(clipboardChanged()));
      }

//---------------------------------------------------------
//   magChanged
//---------------------------------------------------------

void MuseScore::magChanged(int idx)
      {
      qreal mag = canvas->mag();
      QSizeF s = canvas->fsize();
      switch(idx) {
            case 0:  mag = 0.25; break;
            case 1:  mag = 0.5;  break;
            case 2:  mag = 0.75; break;
            case 3:  mag = 1.0;  break;
            case 4:  mag = 1.5;  break;
            case 5:  mag = 2.0;  break;
            case 6:  mag = 4.0;  break;
            case 7:  mag = 8.0;  break;
            case 8:  mag = 16.0; break;
            case 9:      // page width
                  mag *= s.width() / (cs->pageFormat()->width() * DPI);
                  canvas->setOffset(0.0, 0.0);
                  break;
            case 10:     // page
                  {
                  double mag1 = s.width()  / (cs->pageFormat()->width() * DPI);
                  double mag2 = s.height() / (cs->pageFormat()->height() * DPI);
                  mag  *= (mag1 > mag2) ? mag2 : mag1;
                  canvas->setOffset(0.0, 0.0);
                  }
                  break;
            case 11:    // double page
                  {
                  double mag1 = s.width() / (cs->pageFormat()->width()*2*DPI+50.0);
                  double mag2 = s.height() / (cs->pageFormat()->height() * DPI);
                  mag  *= (mag1 > mag2) ? mag2 : mag1;
                  canvas->setOffset(0.0, 0.0);
                  }
                  break;
            case 12:    // original size
                  break;
            }
      canvas->setMag(mag);
      }

//---------------------------------------------------------
//   updateMag
//---------------------------------------------------------

void MuseScore::updateMag()
      {
      qreal val = canvas->mag();
      if (cs)
            cs->setMag(val);
      QString s;
      s.setNum(val * 100, 'f', 1);
      s += "%";
      mag->setEditText(s);
      }

//---------------------------------------------------------
//   padVisible
//---------------------------------------------------------

void MuseScore::padVisible(bool flag)
      {
      padId->setChecked(flag);
      }

//---------------------------------------------------------
//   navigatorVisible
//---------------------------------------------------------

void MuseScore::navigatorVisible(bool flag)
      {
      navigatorId->setChecked(flag);
      }

//---------------------------------------------------------
//   helpBrowser
//---------------------------------------------------------

void MuseScore::helpBrowser()
      {
      QString lang(getenv("LANG"));
      QFileInfo mscoreHelp(mscoreGlobalShare + QString("/doc/man-") + lang + QString(".pdf"));
      if (!mscoreHelp.isReadable()) {
            mscoreHelp.setFile(mscoreGlobalShare + QString("/doc/man-en.pdf"));
            if (!mscoreHelp.isReadable()) {
                  QString info(tr("MuseScore manual not found at: "));
                  info += mscoreHelp.filePath();
                  QMessageBox::critical(this, tr("MuseScore: Open Help"), info);
                  return;
                  }
            }
      QString url("file://" + mscoreHelp.filePath());
      QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   about
//---------------------------------------------------------

void MuseScore::about()
      {
      QMessageBox* mb = new QMessageBox();
      mb->setWindowTitle(QString("MuseScore"));
      mb->setText(tr("Linux Music Score Editor\n"
       "Version " VERSION "\n"
       "(C) Copyright 2002-2005 Werner Schweer and others\n"
       "see http://mscore.sourceforge.net/ for new versions\n"
       "and more information\n"
       "Published under the GNU Public Licence"));
      mb->exec();
      }

//---------------------------------------------------------
//   aboutQt
//---------------------------------------------------------

void MuseScore::aboutQt()
      {
      QMessageBox::aboutQt(this, QString("MuseScore"));
      }

//---------------------------------------------------------
//   openRecentMenu
//---------------------------------------------------------

void MuseScore::openRecentMenu()
      {
      genRecentPopup(openRecent);
      }

//---------------------------------------------------------
//   selectScore
//---------------------------------------------------------

void MuseScore::selectScore(QAction* action)
      {
      int id = action->data().toInt();
      if (id < 0)
            return;
      QString s = getScore(id).mid(2);
      if (s.isEmpty())
            return;
      Score* score = new Score();
      score->read(s);
      appendScore(score);
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void MuseScore::selectionChanged(int state)
      {
      getAction("cut")->setEnabled(state);
      getAction("copy")->setEnabled(state);
      }

//---------------------------------------------------------
//   appendScore
//    append score to project list
//---------------------------------------------------------

void MuseScore::appendScore(Score* score)
      {
      scoreList.push_back(score);
      tab->addTab(score->projectName());

      if (scoreList.size() > 1)
            tab->show();
      else
            tab->hide();

      QString name(score->filePath());
      for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
            if (projectList[i].name.isEmpty())
                  break;
            if (name == projectList[i].name) {
                  int dst = i;
                  int src = i+1;
                  int n   = PROJECT_LIST_LEN - i - 1;
                  for (int k = 0; k < n; ++k)
                        projectList[dst++] = projectList[src++];
                  projectList[dst].name = "";
                  break;
                  }
            }
      ProjectList* s = &projectList[PROJECT_LIST_LEN - 2];
      ProjectList* d = &projectList[PROJECT_LIST_LEN - 1];
      for (int i = 0; i < PROJECT_LIST_LEN-1; ++i)
            *d-- = *s--;
      projectList[0].name = name;
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage(const char* prog, const char*)
      {
      printVersion(prog);
      fprintf(stderr, "usage: %s flags scorefile\n   Flags:\n", prog);
      fprintf(stderr, "   -v        print version\n");
      fprintf(stderr, "   -d        debug mode\n");
      fprintf(stderr, "   -s        no internal synthesizer\n");
      fprintf(stderr, "   -m        no midi\n");
      fprintf(stderr, "   -L        layout debug\n");
      }

//---------------------------------------------------------
//   editTextStyle
//---------------------------------------------------------

void MuseScore::editTextStyle()
      {
      if (textStyleDialog == 0) {
            textStyleDialog = new TextStyleDialog(this);
            connect(textStyleDialog, SIGNAL(textStyleChanged()), SLOT(textStyleChanged()));
            }
      textStyleDialog->show();
      }

//---------------------------------------------------------
//   textStyleChanged
//---------------------------------------------------------

void MuseScore::textStyleChanged()
      {
      if (cs)
            cs->textStyleChanged();
      }

//---------------------------------------------------------
//   saveScoreList
//---------------------------------------------------------

void MuseScore::saveScoreList()
      {
      QFile f(QDir::homePath() + "/.mscorePrj");
      if (!f.open(QIODevice::WriteOnly))
            return;
      int n = scoreList.size();
      QTextStream out(&f);
      for (int i = PROJECT_LIST_LEN-1; i >= 0; --i) {
            if (projectList[i].name.isEmpty())
                  continue;
            bool loaded  = false;
            bool current = false;
            for (int k = 0; k < n; ++k) {
                  if (scoreList[k]->filePath() == projectList[i].name) {
                        loaded = true;
                        if (scoreList[k] == cs)
                              current = true;
                        break;
                        }
                  }
            if (current)
                  out << '+';
            else if (loaded)
                  out << '*';
            else
                  out << ' ';
            out << " " << projectList[i].name << '\n';
            }
      f.close();
      }

//---------------------------------------------------------
//   loadScoreList
//    read list of "Recent Scores"
//---------------------------------------------------------

void MuseScore::loadScoreList()
      {
      QFile f(QDir::homePath() + "/.mscorePrj");
      if (!f.open(QIODevice::ReadOnly))
            return;

      for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
            QByteArray buffer = f.readLine(512);
            int n = buffer.size();
            if (n <= 0)
                  break;
            if (n && buffer[n-1] == '\n')
                  buffer[n-1] = 0;
            if (strlen(buffer) >= 3) {
                  projectList[i].name    = QString(buffer.data() + 2);
                  projectList[i].current = (buffer[0] == '+');
                  projectList[i].loaded  = (buffer[0] != ' ');
                  }
            else
                  break;
            }
      f.close();
      }

//---------------------------------------------------------
//   genRecentPopup
//---------------------------------------------------------

void MuseScore::genRecentPopup(QMenu* popup) const
      {
      popup->clear();
      for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
            if (projectList[i].name.isEmpty())
                  break;
            const char* path = projectList[i].name.toLatin1().data();
            const char* p = strrchr(path, '/');
            if (p == 0)
                  p = path;
            else
                  ++p;
            QAction* action = popup->addAction(QString(p));
            action->setData(i);
            }
      }

//---------------------------------------------------------
//   getTopScore
//---------------------------------------------------------

QString MuseScore::getScore(int idx) const
      {
      if (idx >= PROJECT_LIST_LEN)
            idx = PROJECT_LIST_LEN - 1;
      return projectList[idx].name;
      }

//---------------------------------------------------------
//   setCurrentScore
//---------------------------------------------------------

void MuseScore::setCurrentScore(int idx)
      {
      if (tab->currentIndex() != idx) {
            tab->setCurrentIndex(idx);  // will call setCurrentScore() again
            return;
            }
      if (cs) {
            //
            // remember "global" values:
            //
            cs->setMag(canvas->mag());
            cs->setXoffset(canvas->xoffset());
            cs->setYoffset(canvas->yoffset());
            }

      cs = scoreList[idx];
      cs->clearViewer();
      cs->addViewer(canvas);

      getAction("undo")->setEnabled(!cs->undoEmpty());
      getAction("redo")->setEnabled(!cs->redoEmpty());
      visibleId->setChecked(cs->showInvisible());

      cs->setSpatium(cs->spatium());
      canvas->setMag(cs->mag());
      updateMag();
      canvas->setXoffset(cs->xoffset());
      canvas->setYoffset(cs->yoffset());

      setWindowTitle("MuseScore: " + cs->projectName());
      canvas->setScore(cs);
      seq->setScore(cs);
      if (playPanel)
            playPanel->setScore(cs);

      cs->setUpdateAll();
      cs->endCmd(false);
      cs->doLayout();   // DEBUG1

      connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
      }

//---------------------------------------------------------
//   signalHandler
//---------------------------------------------------------

#if 1
static void signalHandler(int)
      {
      printf("fp exception\n");
      abort();
      }
#endif


//---------------------------------------------------------
//   midiReceived
//---------------------------------------------------------

void MuseScore::midiReceived()
      {
      if (cs)
            cs->midiReceived();
      }

//---------------------------------------------------------
//   cmdAddTitle
//---------------------------------------------------------

void MuseScore::cmdAddTitle()
      {
      if (cs)
            cs->cmdAddTitle();
      }

//---------------------------------------------------------
//   cmdAddSubTitle
//---------------------------------------------------------

void MuseScore::cmdAddSubTitle()
      {
      if (cs)
            cs->cmdAddSubTitle();
      }

//---------------------------------------------------------
//   cmdAddComposer
//---------------------------------------------------------

void MuseScore::cmdAddComposer()
      {
      if (cs)
            cs->cmdAddComposer();
      }

//---------------------------------------------------------
//   cmdAddPoet
//---------------------------------------------------------

void MuseScore::cmdAddPoet()
      {
      if (cs)
            cs->cmdAddPoet();
      }

//---------------------------------------------------------
//   addLyrics
//---------------------------------------------------------

void MuseScore::addLyrics()
      {
      if (cs)
            cs->addLyrics();
      }

//---------------------------------------------------------
//   addExpression
//---------------------------------------------------------

void MuseScore::addExpression()
      {
      if (cs)
            cs->addExpression();
      }

//---------------------------------------------------------
//   addTechnik
//---------------------------------------------------------

void MuseScore::addTechnik()
      {
      if (cs)
            cs->addTechnik();
      }

//---------------------------------------------------------
//   addTempo
//---------------------------------------------------------

void MuseScore::addTempo()
      {
      if (cs)
            cs->addTempo();
      }

//---------------------------------------------------------
//   addMetronome
//---------------------------------------------------------

void MuseScore::addMetronome()
      {
      if (cs)
            cs->addMetronome();
      }

//---------------------------------------------------------
//   startNoteEntry
//---------------------------------------------------------

void MuseScore::startNoteEntry()
      {
      if (cs)
            cs->startNoteEntry();
      }

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void MuseScore::resetUserStretch()
      {
      if (cs)
            cs->resetUserStretch();
      }

//---------------------------------------------------------
//   resetUserOffsets
//---------------------------------------------------------

void MuseScore::resetUserOffsets()
      {
      if (cs)
            cs->resetUserOffsets();
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void MuseScore::midiNoteReceived(int pitch, bool chord)
      {
      if (cs)
            cs->midiNoteReceived(pitch, chord);
      }

//---------------------------------------------------------
//   showInvisibleClicked
//---------------------------------------------------------

void MuseScore::showInvisibleClicked()
      {
      if (cs)
            cs->setShowInvisible(visibleId->isChecked());
      }

//---------------------------------------------------------
//   showPageSettings
//---------------------------------------------------------

void MuseScore::showPageSettings()
      {
      if (pageSettings == 0) {
            pageSettings = new PageSettings();
            connect(pageSettings, SIGNAL(pageSettingsChanged()), SLOT(pageSettingsChanged()));
            }
      pageSettings->setScore(cs);
      pageSettings->show();
      pageSettings->raise();
      }

//---------------------------------------------------------
//   pageSettingsChanged
//---------------------------------------------------------

void MuseScore::pageSettingsChanged()
      {
      cs->pages()->update();
      canvas->setMag(cs->mag());
      cs->doLayout();
      cs->setUpdateAll();
      cs->textStyleChanged();    // fix text styles (center, right etc.)
      canvas->updateNavigator(true);
      cs->endCmd(false);
      }

//---------------------------------------------------------
//   startPageListEditor
//---------------------------------------------------------

void MuseScore::startPageListEditor()
      {
      if (pageListEdit == 0) {
            pageListEdit = new PageListEditor(cs);
            }
      pageListEdit->updateList();
      pageListEdit->show();
      }

//---------------------------------------------------------
//   showElementContext
//---------------------------------------------------------

void MuseScore::showElementContext(Element* el)
      {
      startPageListEditor();
      pageListEdit->setElement(el);
      }

//---------------------------------------------------------
//   editStyle
//---------------------------------------------------------

void MuseScore::editStyle()
      {
      if (editStyleWin == 0) {
            editStyleWin = new EditStyle(0);
            }
      editStyleWin->setValues(cs);
      editStyleWin->show();
      }

//---------------------------------------------------------
//   importMusicXml
//---------------------------------------------------------

void MuseScore::importMusicXml()
      {
      QString name = QFileDialog::getOpenFileName(
         this,
         tr("MuseScore: Import MusicXML"),
         ".",
         QString("MusicXml files (*.xml *.xml.gz *.xml.bz2);; All files (*)")
         );
      if (name.isEmpty())
            return;
      Score* score = new Score();
      score->read(name);
      appendScore(score);
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   showPlayPanel
//---------------------------------------------------------

void MuseScore::showPlayPanel(bool visible)
      {
      if (noSeq)
            return;

      if (playPanel == 0) {
            playPanel = new PlayPanel();
            connect(playPanel, SIGNAL(volChange(float)),    seq, SLOT(setVolume(float)));
            connect(playPanel, SIGNAL(relTempoChanged(int)),seq, SLOT(setRelTempo(int)));
            connect(playPanel, SIGNAL(posChange(int)),      seq, SLOT(setPos(int)));
            connect(playPanel, SIGNAL(rewindTriggered()),   seq, SLOT(rewindStart()));
            connect(playPanel, SIGNAL(closed()),                 SLOT(closePlayPanel()));
            connect(playPanel, SIGNAL(stopToggled(bool)),        SLOT(setStop(bool)));
            connect(playPanel, SIGNAL(playToggled(bool)),        SLOT(setPlay(bool)));

            bool playing = seq->isPlaying();
            playPanel->setStop(!playing);
            playPanel->setPlay(playing);
            playPanel->setVolume(seq->volume());
            playPanel->setTempo(cs->tempomap->tempo(0));
            playPanel->setRelTempo(cs->tempomap->relTempo());
            playPanel->enableSeek(!seq->isPlaying());
            playPanel->setEndpos(seq->getEndTick());
            playPanel->setScore(cs);
            }
      playPanel->setShown(visible);
      playId->setChecked(visible);
      }

//---------------------------------------------------------
//   closePlayPanel
//---------------------------------------------------------

void MuseScore::closePlayPanel()
      {
      playId->setChecked(false);
      }

//---------------------------------------------------------
//   showPad
//---------------------------------------------------------

void MuseScore::showPad(bool visible)
      {
      if (pad == 0) {
            pad = new Pad(0);
            connect(pad, SIGNAL(closed()), SLOT(closePad()));
            cs->setPadState();
            }
      pad->setShown(visible);
      padId->setChecked(visible);
      }

//---------------------------------------------------------
//   closePad
//---------------------------------------------------------

void MuseScore::closePad()
      {
      padId->setChecked(false);
      }

//---------------------------------------------------------
//   cmdAppendMeasures
//---------------------------------------------------------

void MuseScore::cmdAppendMeasures()
      {
      if (cs) {
		if (measuresDialog == 0)
                  measuresDialog = new MeasuresDialog;
            measuresDialog->show();
            }
      }

//---------------------------------------------------------
//   cmdAppendMeasures
//---------------------------------------------------------

void MuseScore::cmdAppendMeasures(int n)
      {
      if (cs) {
	      printf("append %d measures\n", n);
            cs->cmdAppendMeasures(n);
            }
      }

//---------------------------------------------------------
//   MeasuresDialog
//---------------------------------------------------------

MeasuresDialog::MeasuresDialog(QWidget* parent)
   : QDialog(parent)
	{
      setupUi(this);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MeasuresDialog::accept()
	{
	int n = measures->value();
	mscore->cmdAppendMeasures(n);
      done(1);
	}

//---------------------------------------------------------
//   midiinToggled
//---------------------------------------------------------

void MuseScore::midiinToggled(bool val)
      {
      _midiinEnabled = val;
      }

//---------------------------------------------------------
//   speakerToggled
//---------------------------------------------------------

void MuseScore::speakerToggled(bool val)
      {
      _speakerEnabled = val;
      }

//---------------------------------------------------------
//   midiinEnabled
//---------------------------------------------------------

bool MuseScore::midiinEnabled() const
      {
      return preferences.enableMidiInput && _midiinEnabled;
      }

//---------------------------------------------------------
//   playEnabled
//---------------------------------------------------------

bool MuseScore::playEnabled() const
      {
      return preferences.playNotes && _speakerEnabled;
      }

//---------------------------------------------------------
//   removeTab
//---------------------------------------------------------

void MuseScore::removeTab(int i)
      {
      int n = scoreList.size();
      if (n <= 1)
            return;
      QList<Score*>::iterator ii = scoreList.begin() + i;
      if (checkDirty(*ii))
            return;
      scoreList.erase(ii);
      tab->removeTab(i);
      cs = 0;
      if (i >= (n-1))
            i = 0;
      setCurrentScore(i);
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
//      feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
      signal(SIGFPE, signalHandler);

      padState.pitch = 60;
      setDefaultStyle();
      QApplication app(argc, argv);

#ifndef MINGW32
      appDpiX = QX11Info::appDpiX();
      appDpiY = QX11Info::appDpiY();
#endif
      DPI  = appDpiX;     // drawing resolution
      DPMM = DPI / INCH;      // dots/mm

      _spatium = 20.0 / 72.0 * DPI / 4.0;

      int c;
      while ((c = getopt(argc, argv, "vdLsm")) != EOF) {
            switch (c) {
                  case 'v':
                        printVersion(argv[0]);
                        return 0;
                  case 'd':
                        debugMode = true;
                        break;
                  case 'L':
                        layoutDebug = true;
                        break;
                  case 's':
                        noSeq = true;
                        break;
                  case 'm':
                        noMidi = true;
                        break;
                  default:
                        usage(argv[0], "bad argument");
                        return -1;
                  }
            }
      argc -= optind;
      ++argc;

      //
      // initialize shortcut hash table
      //
      for (unsigned i = 0;; ++i) {
            if (MuseScore::sc[i].xml == 0)
                  break;
            shortcuts[MuseScore::sc[i].xml] = new Shortcut(MuseScore::sc[i]);
            }

      haveMidi = !initMidi();
      preferences.read();

      QApplication::setFont(QFont(QString("helvetica"), 11, QFont::Normal));

      if (debugMode) {
            if (haveMidi)
                  printf("midi devices found\n");
            }
      mscoreGlobalShare = getSharePath();
      if (debugMode) {
            printf("global share: <%s>\n", mscoreGlobalShare.toLocal8Bit().data());
            }

      static QTranslator translator;
      QFile ft(":mscore.qm");
      if (ft.exists()) {
            if (debugMode)
                  printf("locale file found\n");
            if (translator.load(":/mscore.qm")) {
                  if (debugMode)
                        printf("locale file loaded\n");
                  }
            qApp->installTranslator(&translator);
            }
      else {
            if (debugMode) {
                  printf("locale file not found for locale <%s>\n",
                     QLocale::system().name().toLatin1().data());
                  }
            }

      //
      //  load internal fonts
      //
      int fontId = QFontDatabase::addApplicationFont(":/fonts/emmentaler_20.otf");
      if (fontId == -1) {
            fprintf(stderr, "Mscore: fatal error: cannot load internal font\n");
            exit(-1);
            }
      fontId = QFontDatabase::addApplicationFont(":/fonts/mscore1_20.otf");
      if (fontId == -1) {
            fprintf(stderr, "Mscore: fatal error: cannot load internal font\n");
            exit(-1);
            }

      seq = new Seq();
      if (!noSeq) {
            if (seq->init()) {
                  printf("sequencer init failed\n");
                  noSeq = true;
                  }
            }
      //
      // avoid font problems by overriding the environment
      //    fall back to "C" locale
      //
      setenv("LANG", "mops", 1);
      QLocale::setDefault(QLocale(QLocale::C));

      //-----------------------------------------
      //  sanity check
      //  check for score font
      //-----------------------------------------

#if 1
      QFont f;
      f.setFamily("Emmentaler");
      f.setPixelSize(20);
      QFontInfo fi(f);

      if (!fi.exactMatch()) {
            //
            // sometimes i do not get an exact match, but the
            // Emmentaler font is found in the font database.
            // I cannot find out why the font did not match. This
            // happens for example with current cvs version of fontconfig.
            //
            // produce some debugging output:
            //
            printf("Emmentaler not found (<%s><%d>)\n", fi.family().toLatin1().data(), fi.style());
            QFontDatabase fdb;
            QStringList families = fdb.families();
            foreach (QString family, families) {
                  if (family == "Emmentaler") {
                        printf("  found <%s>\n", family.toLatin1().data());
                        QStringList styles = fdb.styles(family);
                        foreach (QString style, styles) {
                              printf("    Style <%s>\n", style.toLatin1().data());

                              int w = fdb.weight(family, style);
                              printf("      weight %d\n", w);

                              bool b = fdb.isSmoothlyScalable(family, style);
                              printf("      smooth scalable %d\n", b);

                              b = fdb.isBitmapScalable(family, style);
                              printf("      bitmap scalable %d\n", b);

                              b = fdb.isScalable(family, style);
                              printf("      scalable %d\n", b);

                              b = fdb.isFixedPitch(family, style);
                              printf("      fixedPitch %d\n", b);

                              b = fdb.bold(family, style);
                              printf("      bold %d\n", b);

                              QList<int> sizes = fdb.smoothSizes(family, style);
                              printf("      sizes: ");
                              foreach (int s, sizes)
                                    printf("%d ", s);
                              printf("\n");
                              }
                        }
                  }
            }
#endif

      //-------------------------------
      //  load scores
      //-------------------------------

      initSymbols();
      genIcons();
      new MuseScore();

      int currentScore = 0;
      int idx = 0;
      bool scoreCreated = false;
      if (argc < 2) {
            switch (preferences.sessionStart) {
                  case LAST_SESSION:
                        for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
                              if (projectList[i].name.isEmpty())
                                    break;
                              if (projectList[i].loaded) {
                                    Score* score = new Score();
                                    scoreCreated = true;
                                    score->read(projectList[i].name);
                                    if (projectList[i].current)
                                          currentScore = idx;
                                    mscore->appendScore(score);
                                    ++idx;
                                    }
                              }
                        break;
                  case NEW_SESSION:
                        break;
                  case SCORE_SESSION:
                        Score* score = new Score();
                        scoreCreated = true;
                        score->read(preferences.startScore);
                        mscore->appendScore(score);
                        break;
                  }
            }
      else {
            while (argc > 1) {
                  QString name = argv[optind++];
                  --argc;
                  if (!name.isEmpty()) {
                        Score* score = new Score();
                        scoreCreated = true;
                        score->read(name);
                        mscore->appendScore(score);
                        }
                  }
            }

      if (!scoreCreated)
            // start with empty score:
            mscore->appendScore(new Score());

      mscore->resize(scoreSize);
      mscore->move(scorePos);
      mscore->setCurrentScore(currentScore);
      mscore->showNavigator(preferences.showNavigator);
      mscore->showPad(preferences.showPad);
      if (mscore->getKeyPad())
            mscore->getKeyPad()->move(preferences.padPos);
      mscore->showPlayPanel(preferences.showPlayPanel);
      if (mscore->getPlayPanel())
            mscore->getPlayPanel()->move(preferences.playPanelPos);

      int rfd = getMidiReadFd();
      if (rfd != -1) {
            QSocketNotifier* sn = new QSocketNotifier(rfd,
               QSocketNotifier::Read,  mscore);
            sn->connect(sn, SIGNAL(activated(int)), mscore, SLOT(midiReceived()));
            }
      mscore->getCanvas()->setFocus(Qt::OtherFocusReason);
      mscore->show();
      return qApp->exec();
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void MuseScore::cmd(QAction* a)
      {
      QString cmd(a->data().toString());

      if (cmd == "instruments")
            editInstrList();
      else if (cmd == "clefs")
            clefMenu();
      else if (cmd == "keys")
            keyMenu();
      else if (cmd == "symbols")
            symbolMenu1();
      else if (cmd == "times")
            timeMenu();
      else if (cmd == "dynamics")
            dynamicsMenu();
      else {
            if (cs)
                  cs->cmd(cmd);
            }
      }

//---------------------------------------------------------
//   clipboardChanged
//---------------------------------------------------------

void MuseScore::clipboardChanged()
      {
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (ms == 0)
            return;
      bool flag = ms->hasFormat("application/mscore/symbol")
            ||    ms->hasFormat("application/mscore/staff")
            ||    ms->hasFormat("application/mscore/system")
            ||    ms->hasFormat("application/mscore/symbols");
      // TODO: depends on selection state
      getAction("paste")->setEnabled(flag);
      }


