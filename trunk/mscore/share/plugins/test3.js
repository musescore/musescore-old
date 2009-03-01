//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Test plugin
//
//  Copyright (C)2008 Werner Schweer and others
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

//
//    This is ECMAScript code (ECMA-262 aka "Java Script")
//

//---------------------------------------------------------
//    init
//    this function will be called on startup of
//    mscore
//---------------------------------------------------------

function init()
      {
      // print("test script init");
      }

//---------------------------------------------------------
//    run
//    this function will be called when activating the
//    plugin menu entry
//
//    global Variables:
//    pluginPath - contains the plugin path; file separator
//                 is "/"
//---------------------------------------------------------

function run()
      {
      var score = new Score();
      score.name = "Test-Score";
      score.appendPart("Piano");    // create two staff piano part
      score.appendMeasures(5);      // append five empty messages
      var cursor = new Cursor(score);
      cursor.staff = 0;
      cursor.voice = 0;
      cursor.rewind();
      for (var i = 0; i < 4; i += 1) {
            var chord  = new Chord();
            chord.tickLen = 480;
            var note   = new Note();
            note.pitch = 60 + i;
            chord.addNote(note);
            cursor.addChord(chord);
            cursor.next();
            }
      }

//---------------------------------------------------------
//    menu:  defines were the function will be placed
//           in the menu structure
//---------------------------------------------------------

var mscorePlugin = {
      menu: 'Plugins.CreateScore',
      init: init,
      run:  run
      };

mscorePlugin;

