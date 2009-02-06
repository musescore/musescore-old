//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Test plugin 2
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
      // print("test2 script init");
      }

//---------------------------------------------------------
//    run
//    this function will be called when activating the
//    plugin menu entry
//---------------------------------------------------------

function run()
      {
      print("test2 script run");
      }


var mscorePlugin = {
      menu: 'Create.test',
      init: init,
      run:  run
      };

mscorePlugin;


