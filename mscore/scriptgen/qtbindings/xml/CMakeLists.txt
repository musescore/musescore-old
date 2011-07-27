#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2002-2008 by Werner Schweer and others
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

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

if (STATIC_SCRIPT_BINDINGS)
      set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Winvalid-pch -DQT_STATICPLUGIN")
      set(LIBTYPE "STATIC")
else (STATIC_SCRIPT_BINDINGS)
      set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Winvalid-pch")
      set(LIBTYPE "SHARED")
endif (STATIC_SCRIPT_BINDINGS)

if (GCC_VISIBILITY)
      set(CMAKE_CXX_FLAGS, ${CMAKE_CXX_FLAGS} "-fvisibility=hidden -fvisibility-inlines-hidden")
endif (GCC_VISIBILITY)

include_directories(${QT_INCLUDES})

add_library (
      qtscript_xml ${LIBTYPE}
      main.cpp
      qtscript_QDomAttr.cpp
      qtscript_QDomCDATASection.cpp
      qtscript_QDomCharacterData.cpp
      qtscript_QDomComment.cpp
      qtscript_QDomDocument.cpp
      qtscript_QDomDocumentFragment.cpp
      qtscript_QDomDocumentType.cpp
      qtscript_QDomElement.cpp
      qtscript_QDomEntity.cpp
      qtscript_QDomEntityReference.cpp
      qtscript_QDomImplementation.cpp
      qtscript_QDomNamedNodeMap.cpp
      qtscript_QDomNode.cpp
      qtscript_QDomNodeList.cpp
      qtscript_QDomNotation.cpp
      qtscript_QDomProcessingInstruction.cpp
      qtscript_QDomText.cpp
      qtscript_QXmlAttributes.cpp
      qtscript_QXmlContentHandler.cpp
      qtscript_QXmlDTDHandler.cpp
      qtscript_QXmlDeclHandler.cpp
      qtscript_QXmlDefaultHandler.cpp
      qtscript_QXmlEntityResolver.cpp
      qtscript_QXmlErrorHandler.cpp
      qtscript_QXmlInputSource.cpp
      qtscript_QXmlLexicalHandler.cpp
      qtscript_QXmlLocator.cpp
      qtscript_QXmlParseException.cpp
      qtscript_QXmlReader.cpp
      qtscript_QXmlSimpleReader.cpp
      qtscriptshell_QXmlAttributes.cpp
      qtscriptshell_QXmlContentHandler.cpp
      qtscriptshell_QXmlDTDHandler.cpp
      qtscriptshell_QXmlDeclHandler.cpp
      qtscriptshell_QXmlDefaultHandler.cpp
      qtscriptshell_QXmlEntityResolver.cpp
      qtscriptshell_QXmlErrorHandler.cpp
      qtscriptshell_QXmlInputSource.cpp
      qtscriptshell_QXmlLexicalHandler.cpp
      qtscriptshell_QXmlLocator.cpp
      qtscriptshell_QXmlReader.cpp
      qtscriptshell_QXmlSimpleReader.cpp
      )

set_target_properties(qtscript_xml
      PROPERTIES
      COMPILE_FLAGS
         "-D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_NO_DEBUG -include ${PROJECT_SOURCE_DIR}/all.h"
      LINK_FLAGS "-L${QT_LIBRARY_DIR}"
      )

if (STATIC_SCRIPT_BINDINGS)
else (STATIC_SCRIPT_BINDINGS)
   if (MINGW)
      target_link_libraries(
         qtscript_xml
         ${QT_mingw_LIBRARIES}
         )
      install( TARGETS qtscript_xml DESTINATION bin/script/ )
   else(MINGW)
      install( TARGETS qtscript_xml DESTINATION ${QT_PLUGINS_DIR}/script/ )
   endif(MINGW)
endif (STATIC_SCRIPT_BINDINGS)

