//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "webpage.h"
#include "mscore.h"

//---------------------------------------------------------
//   WebPage
//---------------------------------------------------------

WebPage::WebPage(MuseScore* mscore, QWidget* parent)
   : QDockWidget(parent)
      {
      setObjectName("webpage");
      setWindowTitle(tr("WebView"));
      setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

      QWebView* web = new QWebView(this);
      QWebPage* wp  = web->page();
      QWebFrame* frame = wp->mainFrame();
      frame->addToJavaScriptWindowObject("mscore", mscore);
      QWebSettings* s = wp->settings();
      s->setAttribute(QWebSettings::PluginsEnabled, true);
//      web->load(QUrl("http://s.musescore.org/tutorials.html"));
      web->load(QUrl("http://s.musescore.org/newscore.html"));
      setWidget(web);
      }

