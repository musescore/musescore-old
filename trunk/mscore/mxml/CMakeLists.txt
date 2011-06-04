#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2011 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

# set(CMAKE_CXX_FLAGS "-fpermissive")

add_library (
      mxml STATIC
      catalog.cpp
      chvalid.cpp
      dict.cpp
      encoding.cpp
      entities.cpp
      error.cpp
      globals.cpp
      hash.cpp
      list.cpp
      parser.cpp
      parserInternals.cpp
      pattern.cpp
      sax2.cpp
      threads.cpp
      tree.cpp
      uri.cpp
      valid.cpp
      xlink.cpp
      xmlIO.cpp
      xmlreader.cpp
      xmlstring.cpp
      xmlunicode.cpp
      )

