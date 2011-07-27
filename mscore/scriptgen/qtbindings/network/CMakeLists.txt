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
      set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif (GCC_VISIBILITY)


include_directories(${QT_INCLUDES})

# only for QSsl
#      qtscript_QSslCertificate.cpp
#      qtscript_QSslCipher.cpp
#      qtscript_QSslConfiguration.cpp
#      qtscript_QSslError.cpp
#      qtscript_QSslKey.cpp
#      qtscriptshell_QSslSocket.cpp
#      qtscript_QSslSocket.cpp

add_library (
      qtscript_network ${LIBTYPE}
      main.cpp
      qtscript_QSsl.cpp
      qtscript_QAbstractSocket.cpp
      qtscript_QAuthenticator.cpp
      qtscript_QFtp.cpp
      qtscript_QHostAddress.cpp
      qtscript_QHostInfo.cpp
      qtscript_QHttp.cpp
      qtscript_QHttpRequestHeader.cpp
      qtscript_QHttpResponseHeader.cpp
      qtscript_QIPv6Address.cpp
      qtscript_QLocalServer.cpp
      qtscript_QLocalSocket.cpp
      qtscript_QNetworkAccessManager.cpp
      qtscript_QNetworkAddressEntry.cpp
      qtscript_QNetworkCookie.cpp
      qtscript_QNetworkCookieJar.cpp
      qtscript_QNetworkInterface.cpp
      qtscript_QNetworkProxy.cpp
      qtscript_QNetworkReply.cpp
      qtscript_QNetworkRequest.cpp
      qtscript_QTcpServer.cpp
      qtscript_QTcpSocket.cpp
      qtscript_QUdpSocket.cpp
      qtscript_QUrlInfo.cpp
      qtscriptshell_QAbstractSocket.cpp
      qtscriptshell_QFtp.cpp
      qtscriptshell_QHttp.cpp
      qtscriptshell_QHttpRequestHeader.cpp
      qtscriptshell_QHttpResponseHeader.cpp
      qtscriptshell_QIPv6Address.cpp
      qtscriptshell_QLocalServer.cpp
      qtscriptshell_QLocalSocket.cpp
      qtscriptshell_QNetworkAccessManager.cpp
      qtscriptshell_QNetworkCookieJar.cpp
      qtscriptshell_QNetworkReply.cpp
      qtscriptshell_QTcpServer.cpp
      qtscriptshell_QTcpSocket.cpp
      qtscriptshell_QUdpSocket.cpp
      qtscriptshell_QUrlInfo.cpp
      qtscriptshell_QNetworkProxyFactory.cpp
      qtscriptshell_QHttpHeader.cpp
      qtscriptshell_QAbstractNetworkCache.cpp
      qtscript_QNetworkProxyFactory.cpp
      qtscript_QNetworkProxyQuery.cpp
      qtscript_QNetworkCacheMetaData.cpp
      qtscript_QHttpHeader.cpp
      qtscript_QAbstractNetworkCache.cpp
      )

set_target_properties(
      qtscript_network
      PROPERTIES
      COMPILE_FLAGS
         "-D QT_NO_OPENSSL -D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_NO_DEBUG -include ${PROJECT_SOURCE_DIR}/all.h"
      LINK_FLAGS "-L${QT_LIBRARY_DIR}"
      )

if (NOT STATIC_SCRIPT_BINDINGS)
   if (MINGW)
      target_link_libraries(
         qtscript_network
         ${QT_mingw_LIBRARIES}
         )
      install( TARGETS qtscript_network DESTINATION bin/script/ )
   else(MINGW)
      install( TARGETS qtscript_network DESTINATION ${QT_PLUGINS_DIR}/script/ )
   endif(MINGW)
endif (NOT STATIC_SCRIPT_BINDINGS)

