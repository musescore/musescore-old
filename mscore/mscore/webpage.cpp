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
#include "mscore.h"
#include "preferences.h"
#include "score.h"


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
      
      //set cookie jar for persistent cookies
      CookieJar* jar = new CookieJar(QString(dataPath + "/cookies.txt"));
      page()->networkAccessManager()->setCookieJar(jar);

      progressBar = 0;
      connect(this, SIGNAL(linkClicked(const QUrl&)), SLOT(link(const QUrl&)));
      }

//---------------------------------------------------------
//   stopBusy
//---------------------------------------------------------

void MyWebView::stopBusy(bool val, bool close)
      {
      if (!val) {
            setHtml(QString("<html><head>"
                 "<script type=\"text/javascript\">"
                  "function closePermanently() { mscore.closeWebPanelPermanently(); return false;}"
                  "</script>"
                  "<link rel=\"stylesheet\" href=\"data/webview.css\" type=\"text/css\" /></head>"
            "<body>"
            "<div id=\"content\">"
            "<div id=\"middle\">"
            "  <div class=\"title\" align=\"center\"><h2>%1</h2></div>"
            "  <ul><li>%2</li></ul>"
            "  <div align=\"center\"><a class=\"button\" href=\"#\" onclick=\"return panel.load();\">%3</a></div>"
            "  <div align=\"center\"><a class=\"close\" href=\"#\" onclick=\"return closePermanently();\">%4</div>"
            "</div></div>"
            "</body></html>")
            .arg(tr("Could not<br /> connect"))
            .arg(tr("To connect with the community, <br /> you need to have internet <br /> connection enabled"))
            .arg(tr("Retry"))
            .arg(tr("Close this permanently"))
            , QUrl("qrc:/"));
            if(!preferences.firstStartWeb && close)
                  mscore->showWebPanel(false);
            }
      disconnect(this, SIGNAL(loadProgress(int)), progressBar, SLOT(setValue(int)));
      mscore->hideProgressBar();
      setCursor(Qt::ArrowCursor);
      }

void MyWebView::stopBusyAndClose(bool val)
      {
      stopBusy(val, true);
      }

void MyWebView::stopBusyAndFirst(bool val)
      {
      stopBusy(val, false);
      if(val && preferences.firstStartWeb) {
            preferences.firstStartWeb = false;
            preferences.dirty = true;
            }
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
//   sizeHint
//---------------------------------------------------------
      
QSize	MyWebView::sizeHint() const 
      {
      return QSize(300 , 300);
      }

//---------------------------------------------------------
//   WebPageDockWidget
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
            
      if(preferences.firstStartWeb) {
            connect(web, SIGNAL(loadFinished(bool)), web, SLOT(stopBusyStatic(bool)));
            web->setBusy();
            web->setHtml(QString("<html><head>"
                  "<script type=\"text/javascript\">"
                  "      function closePermanently() { mscore.closeWebPanelPermanently(); return false; }</script>"
                  "      <link rel=\"stylesheet\" href=\"data/webview.css\" type=\"text/css\" /></head>"
                  "<body>"
                  "<div id=\"content\">"
                  "<div id=\"middle\">"
                  "  <div class=\"title\" align=\"center\"><h2>%1</h2></div>"
                  "  <ul><li>%2</li>"
                  "  <li>%3</li>"
                  "  <li>%4</li>"
                  "  <li>%5</li></ul>"
                  "  <div align=\"center\"><a class=\"button\" href=\"#\" onClick=\"return panel.load();\">%6</a></div>"
                  "  <div align=\"center\"><a class=\"close\" href=\"#\" onclick=\"return closePermanently();\">%7</div>"
                  "</div></div>"
                  "</body></html>")
                  .arg(tr("Connect with the <br /> Community"))
                  .arg(tr("Find help"))
                  .arg(tr("Improve your skills"))
                  .arg(tr("Read the latest news"))
                  .arg(tr("Download free sheet music"))
                  .arg(tr("Start"))
                  .arg(tr("Close this permanently"))
                  , QUrl("qrc:/"));
            }
      else{
            //And not load ! 
            connect(web, SIGNAL(loadFinished(bool)), web, SLOT(stopBusyAndClose(bool)));
            web->setBusy();
            web->load(QUrl(webUrl()));
            }

      setWidget(web);
      }

//---------------------------------------------------------
//   addToJavascript
//---------------------------------------------------------

void WebPageDockWidget::addToJavascript() 
      {
      QWebFrame* frame = web->webPage()->mainFrame();
      frame->addToJavaScriptWindowObject("panel", this);
      frame->addToJavaScriptWindowObject("mscore", mscore);
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void WebPageDockWidget::load()
      {
      connect(web, SIGNAL(loadFinished(bool)), web, SLOT(stopBusyAndFirst(bool)));
      web->setBusy();
      web->load(QUrl(webUrl()));
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------
QString WebPageDockWidget::webUrl()
    { 
    static const char* staticUrl = "http://cdn.musescore.com/connect.html";
    return QString("%1?language=%2").arg(staticUrl).arg(mscore->getLocaleISOCode()); 
    }      


//---------------------------------------------------------
//   CookieJar
//
//   Once the QNetworkCookieJar object is deleted, all cookies it held will be 
//   discarded as well. If you want to save the cookies, you should derive from 
//   this class and implement the saving to disk to your own storage format. 
//   (From QNetworkCookieJar documentation.)
//---------------------------------------------------------

CookieJar::CookieJar(QString path, QObject *parent) 
    : QNetworkCookieJar(parent)
      {
      file = path;
      QFile cookieFile(this->file);

      if (cookieFile.exists() && cookieFile.open(QIODevice::ReadOnly)) {
            QList<QNetworkCookie> list;
            QByteArray line;

            while(!(line = cookieFile.readLine()).isNull()) {
                  list.append(QNetworkCookie::parseCookies(line));
                  }
            setAllCookies(list); 
            }
      else {
            qWarning() << "Can't open "<< this->file << " to read cookies"; 
            }
      }

//---------------------------------------------------------
//   ~CookieJar
//---------------------------------------------------------

CookieJar::~CookieJar()
      {
      QList <QNetworkCookie> cookieList; 
      cookieList = allCookies();
      
      QFile file(this->file);

      if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Can't open "<< this->file << " to save cookies";
            return;
            }
                  
      QTextStream out(&file);
      for(int i = 0 ; i < cookieList.size() ; i++) {
                //get cookie data
                QNetworkCookie cookie = cookieList.at(i);
                if (!cookie.isSessionCookie()) {
                      QByteArray line =  cookieList.at(i).toRawForm(QNetworkCookie::Full);
                      out << line << "\n";
                      }
            }
      file.close();
      }

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
      
      Score* score = new Score(defaultStyle);
      if(!score->read(tmpName)) {
            printf("readScore failed\n");
            delete score;
            return;
            }

      ScoreView::setScore(score);
      update();
      }


