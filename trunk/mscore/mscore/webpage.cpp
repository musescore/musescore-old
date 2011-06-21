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
#include "musescore.h"
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
      QWebFrame* frame = m_page.mainFrame();
      frame->addToJavaScriptWindowObject("mscore", mscore);
      setPage(&m_page);
      progressBar = 0;
      connect(this, SIGNAL(linkClicked(const QUrl&)), SLOT(link(const QUrl&)));
      connect(this, SIGNAL(loadFinished(bool)), SLOT(stopBusy(bool)));
      }

//---------------------------------------------------------
//   stopBusy
//---------------------------------------------------------

void MyWebView::stopBusy(bool val)
      {
      if (!val)
            setHtml(tr("<HTML><BODY BGCOLOR=\"#FFFFFF\"><H2>no internet connection?</H2>"
               "</BODY></HTML>"));
      disconnect(this, SIGNAL(loadProgress(int)), progressBar, SLOT(setValue(int)));
      mscore->hideProgressBar();
      setCursor(Qt::ArrowCursor);
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

      //QWidget* w = new QWidget(this);
      setWindowTitle (tr("MuseScore Universe"));
      /*setTitleBarWidget(w);
      titleBarWidget()->hide();*/
      setFloating(false);
      setFeatures(QDockWidget::DockWidgetClosable);

      QWidget* mainWidget = new QWidget(this);
      
      QVBoxLayout* layout = new QVBoxLayout;
      mainWidget->setLayout(layout);

      setObjectName("webpage");
      setAllowedAreas(Qt::LeftDockWidgetArea);
      const char* url = "http://cdn.musescore.org/universe.html";
      
      web = new MyWebView;
      layout->addWidget(web);
      web->load(QUrl(url));
      web->setBusy();
      
      setWidget(mainWidget);
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

      Score* score = new Score(MScore::defaultStyle());
      if (score->readScore(tmpName) != 0) {
            printf("readScore failed\n");
            delete score;
            return;
            }
      ScoreView::setScore(score);
      update();
      }


