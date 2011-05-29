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
      setCursor(Qt::ArrowCursor);
      }

//---------------------------------------------------------
//   setBusy
//---------------------------------------------------------

void MyWebView::setBusy()
      {
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

enum { WEB_TUTORIALS, WEB_NEWS, WEB_SCORELIB };

      setObjectName("webpage");
      setWindowTitle(tr("Web"));
      setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

      tab->addTab(tr("Tutorials"));
      web1 = new MyWebView;
      stack->addWidget(web1);

      tab->addTab(tr("News"));
      web2 = new MyWebView;
      stack->addWidget(web2);

      tab->addTab(tr("Score Library"));
      web3 = new MyWebView;
      stack->addWidget(web3);

      setWidget(mainWidget);

      connect(tab, SIGNAL(currentChanged(int)), stack, SLOT(setCurrentIndex(int)));
      connect(tab, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
      }

//---------------------------------------------------------
//   tabChanged
//---------------------------------------------------------

void WebPage::tabChanged(int n)
      {
      switch (n) {
            case WEB_SCORELIB:
                  web1->load(QUrl("http://s.musescore.org/scoreview.html"));
                  web1->setBusy();
                  break;
            case WEB_TUTORIALS:
                  web2->load(QUrl("http://musescore.org/musescore-panel/tutorials"));
                  web2->setBusy();
                  break;
            case WEB_NEWS:
                  web3->load(QUrl("http://s.musescore.org/news.html"));
                  web3->setBusy();
                  break;
            }
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


