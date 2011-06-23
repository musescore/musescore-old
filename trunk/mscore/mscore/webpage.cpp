//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  The webview is shown on startup with a local file inviting user 
//  to start connecting with the community. They can press start and 
//  MuseScore will go online. If no connection, display a can't connect message
//  On next startup, if no connection, the panel is closed. If connection, the
//  MuseScore goes online directly. If the autoclose panel is reopen, the user
//  can retry; retry should not close the panel.
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
#include "musescore.h"
#include "preferences.h"
#include "libmscore/score.h"
#include "libmscore/mscore.cpp"


//---------------------------------------------------------
//   MyWebPage
//---------------------------------------------------------

MyWebPage::MyWebPage(QObject *parent)
   : QWebPage(parent)
      {
      // Enable plugin support
      settings()->setAttribute(QWebSettings::PluginsEnabled, true);
      }

//---------------------------------------------------------
//   createPlugin
//---------------------------------------------------------

QObject* MyWebPage::createPlugin(
   const QString &classid,
   const QUrl &url,
   const QStringList &paramNames,
   const QStringList &paramValues)
      {
      // Create the widget using QUiLoader.
      // This means that the widgets don't need to be registered
      // with the meta object system.
      // On the other hand, non-gui objects can't be created this
      // way. When we'd like to create non-visual objects in
      // Html to use them via JavaScript, we'd use a different
      // mechanism than this.
#if 0
      if (classid == "WebScoreView") {
            WebScoreView* sv = new WebScoreView(view());
            int idx = paramNames.indexOf("score");
            if (idx != -1) {
                  QString score = paramValues[idx];
                  sv->setScore(paramValues[idx]);
                  }
            else {
                  printf("create WebScoreView: property score not found(%d)\n",
                     paramNames.size());
                  }
            return sv;
            }
#endif
      return 0;

      /*QUiLoader loader;
      return loader.createWidget(classid, view());*/
      }

//---------------------------------------------------------
//   MyWebView
//---------------------------------------------------------

MyWebView::MyWebView(QWidget *parent):
   QWebView(parent),
   m_page(this)
      {
      // Set the page of our own PageView class, MyPageView,
      // because only objects of this class will handle
      // object-tags correctly.

      m_page.setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
      
      setPage(&m_page);
      progressBar = 0;
      connect(this, SIGNAL(linkClicked(const QUrl&)), SLOT(link(const QUrl&)));
      }

//---------------------------------------------------------
//   stopBusy
//---------------------------------------------------------

void MyWebView::stopBusy(bool val, bool close)
      {
      if (!val) {
            setHtml(tr("<html><head>"
                 "<script type=\"text/javascript\">"
                  "function closePermanently() { mscore.closeWebPanelPermanently(); return false;}"
                  "</script>"
                  "<link rel=\"stylesheet\" href=\"data/webview.css\" type=\"text/css\" /></head>"
            "<body>"
            "<div id=\"content\">"
            "<div id=\"middle\">"
            "  <div class=\"title\" align=\"center\"><h2>Could not<br /> connect</h2></div>"
            "  <ul><li>To connect with the community, <br /> you need to have internet <br /> connection enabled</li></ul>"
            "  <div align=\"center\"><a class=\"button\" href=\"#\" onclick=\"return panel.load();\">Retry</a></div>"
            "  <div align=\"center\"><a class=\"close\" href=\"#\" onclick=\"return closePermanently();\">Close this permanently</div>"
            "</div></div>"
            "</body></html>"), QUrl("qrc:/"));
            if(!preferences.firstStartWeb && close)
                  mscore->showWebPanel(false);
            }
      disconnect(this, SIGNAL(loadProgress(int)), progressBar, SLOT(setValue(int)));
      mscore->hideProgressBar();
      setCursor(Qt::ArrowCursor);
      if(val) {
            preferences.firstStartWeb = false;
            preferences.dirty = true;
            }
      }

void MyWebView::stopBusyAndClose(bool val)
      {
      stopBusy(val, true);
      }

void MyWebView::stopBusyStatic(bool val) 
      {
      stopBusy(val, false);
      }

//---------------------------------------------------------
//   setBusy
//---------------------------------------------------------

void MyWebView::setBusy()
      {
      progressBar = mscore->showProgressBar();
      progressBar->setRange(0, 100);
      progressBar->setValue(0);
      connect(this, SIGNAL(loadProgress(int)), progressBar, SLOT(setValue(int)));
      setCursor(Qt::WaitCursor);
      }

//---------------------------------------------------------
//   link
//---------------------------------------------------------

void MyWebView::link(const QUrl& url)
      {
      QString path(url.path());
      QFileInfo fi(path);
      if (fi.suffix() == "mscz")
            mscore->loadFile(url);
      else
            QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   WebPage
//---------------------------------------------------------

WebPageDockWidget::WebPageDockWidget(MuseScore* mscore, QWidget* parent)
   : QDockWidget(parent)
      {
      setWindowTitle("MuseScore Connect");
      setFloating(false);
      setFeatures(QDockWidget::DockWidgetClosable);
      
      setObjectName("webpage");
      setAllowedAreas(Qt::LeftDockWidgetArea);

      web = new MyWebView;
      web->setContextMenuPolicy(Qt::PreventContextMenu);
      QWebFrame* frame = web->webPage()->mainFrame();
      connect(frame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addToJavascript()));
            
      /*web->load(QUrl("http://localhost/webview/test.html"));
      web->setBusy();*/
      if(preferences.firstStartWeb) {
            connect(web, SIGNAL(loadFinished(bool)), web, SLOT(stopBusyStatic(bool)));
            web->setBusy();
            web->setHtml("<html><head>"
                  "<script type=\"text/javascript\">"
                  "      function closePermanently() { mscore.closeWebPanelPermanently(); return false; }</script>"
                  "      <link rel=\"stylesheet\" href=\"data/webview.css\" type=\"text/css\" /></head>"
                  "<body>"
                  "<div id=\"content\">"
                  "<div id=\"middle\">"
                  "  <div class=\"title\" align=\"center\"><h2>Connect with the <br /> Community</h2></div>"
                  "  <ul><li>Find help</li>"
                  "  <li>Improve your skills</li>"
                  "  <li>Read the latest news</li>"
                  "  <li>Download free sheet music</li></ul>"
                  "  <div align=\"center\"><a class=\"button\" href=\"#\" onClick=\"return panel.load();\">Start</a></div>"
                  "  <div align=\"center\"><a class=\"close\" href=\"#\" onclick=\"return closePermanently();\">Close this permanently</div>"
                  "</div></div>"
                  "</body></html>", QUrl("qrc:/"));
            }
      else{
            //And not load ! 
            connect(web, SIGNAL(loadFinished(bool)), web, SLOT(stopBusyAndClose(bool)));
            web->setBusy();
            web->load(QUrl(webUrl));
            }
      setWidget(web);
      }

void WebPageDockWidget::addToJavascript() 
      {
      QWebFrame* frame = web->webPage()->mainFrame();
      frame->addToJavaScriptWindowObject("panel", this);
      frame->addToJavaScriptWindowObject("mscore", mscore);
      }

void WebPageDockWidget::load()
      {
      connect(web, SIGNAL(loadFinished(bool)), web, SLOT(stopBusyStatic(bool)));
      web->setBusy();
      web->load(QUrl(webUrl));
      }

#if 0
//---------------------------------------------------------
//   WebScoreView
//---------------------------------------------------------

WebScoreView::WebScoreView(QWidget* parent)
   : ScoreView(parent)
      {
      networkManager = 0;
      }

WebScoreView::WebScoreView(const WebScoreView& wsv)
   : ScoreView((QWidget*)(wsv.parent()))
      {
      networkManager = 0;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void WebScoreView::setScore(const QString& url)
      {
      if (!networkManager) {
            networkManager = new QNetworkAccessManager(this);
            connect(networkManager, SIGNAL(finished(QNetworkReply*)),
               SLOT(networkFinished(QNetworkReply*)));
            }
      networkManager->get(QNetworkRequest(QUrl(url)));
      }

//---------------------------------------------------------
//   networkFinished
//---------------------------------------------------------

void WebScoreView::networkFinished(QNetworkReply* reply)
      {
      if (reply->error() != QNetworkReply::NoError) {
            printf("Error while checking update [%s]\n", qPrintable(reply->errorString()));
            return;
            }
      QByteArray ha = reply->rawHeader("Content-Disposition");
      QString s(ha);
      QString name;
      QRegExp re(".*filename=\"(.*)\"");
      if (s.isEmpty() || re.indexIn(s) == -1)
            name = "unknown.mscz";
      else
            name = re.cap(1);

      QByteArray data = reply->readAll();
      QString tmpName = "/tmp/" + name;
      QFile f(tmpName);
      f.open(QIODevice::WriteOnly);
      f.write(data);
      f.close();

      Score* score = mscore->readScore(tmpName);
      if (!score) {
            printf("readScore failed\n");
            return;
            }
      ScoreView::setScore(score);
      update();
      }

#endif

