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
            setHtml(tr("<html><body><h2>no internet connection?</h2>"
               "</body></html>"));
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
            load(url);
      }

//---------------------------------------------------------
//   WebPage
//---------------------------------------------------------

WebPage::WebPage(MuseScore* mscore, QWidget* parent)
   : QDockWidget(parent)
      {
      QString tabPages[WEB_PAGECOUNT];

      tabPages[WEB_TUTORIALS] = tr("Tutorials");
      tabPages[WEB_NEWS]      = tr("News");
      tabPages[WEB_SCORELIB]  = tr("Scores");

      QWidget* w = new QWidget(this);
      setTitleBarWidget(w);
      titleBarWidget()->hide();

      QWidget* mainWidget = new QWidget(this);
      tab   = new QTabBar(mainWidget);
      stack = new QStackedWidget(mainWidget);
      QVBoxLayout* layout = new QVBoxLayout;
      layout->addWidget(tab);
      layout->addWidget(stack);
      mainWidget->setLayout(layout);

      setObjectName("webpage");
      setWindowTitle(tr("Web"));
      setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

      for (int i = 0; i < WEB_PAGECOUNT; ++i) {
            tab->addTab(tabPages[i]);
            web[i] = new MyWebView;
            stack->addWidget(web[i]);
            }

      setWidget(mainWidget);

      connect(tab, SIGNAL(currentChanged(int)), stack, SLOT(setCurrentIndex(int)));
      connect(tab, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
      }

//---------------------------------------------------------
//   setTab
//---------------------------------------------------------

void WebPage::setTab(int n)
      {
      if (tab->currentIndex() != n)
            tab->setCurrentIndex(n);
      else
            tabChanged(n);
      }

//---------------------------------------------------------
//   tabChanged
//---------------------------------------------------------

void WebPage::tabChanged(int n)
      {
printf("tabChanged %d\n", n);
      static const char* urls[WEB_PAGECOUNT];

      urls[WEB_SCORELIB]  = "http://musescore.com/sheetmusic";
      urls[WEB_TUTORIALS] = "http://musescore.org/musescore-panel/tutorials";
      urls[WEB_NEWS]      = "http://s.musescore.org/news.html";

      web[n]->load(QUrl(urls[n]));
      web[n]->setBusy();
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

      Score* score = new Score(mscore->defaultStyle());
      if (score->readScore(tmpName) != 0) {
            printf("readScore failed\n");
            delete score;
            return;
            }
      ScoreView::setScore(score);
      update();
      }


