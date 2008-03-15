//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include <QtXml/QtXml>
#include "xml.h"


char* srcPathPrefix = "";
char* dstPathPrefix = "";

//---------------------------------------------------------
//   Manuals
//---------------------------------------------------------

struct Manuals {
      QString src;
      QString dst;
      QString idx;
      QString progDst;
      QString progIdx;
      };

Manuals manuals[] = {
      {  QString("doc-de.xml"),
         QString("de/man"),
         QString("de/man/reference.php"),
         QString("man/de"),
         QString("man/de/index.html")
         },
      {  QString("doc-en.xml"),
         QString("en/man"),
         QString("en/man/reference.php"),
         QString("man/en"),
         QString("man/en/index.html")
         }
      };

const QString deIndex("../web/de/reference.php");
const QString enIndex("../web/en/reference.php");
const QString indexFile("index.doc");

//---------------------------------------------------------
//   Index
//---------------------------------------------------------

struct Index {
      QString tag;
      QString target;
      Index(const QString& s1, const QString& s2) {
            tag = s1;
            target = s2;
            }
      };

QList<Index> idxList;

//---------------------------------------------------------
//   checkFile
//---------------------------------------------------------

void checkFile(int lan, const QString& name)
      {
      QString path;
      if (lan == 1)
            path = "../web/de/" + name;
      else
            path = "../web/en/" + name;
      QFileInfo fi(path);
      if (!fi.exists()) {
            printf("File <%s> does not exist\n", qPrintable(path));
            }
      }

//---------------------------------------------------------
//   sort
//---------------------------------------------------------

static bool sort(const Index& s1, const Index& s2)
      {
      return s1.tag.toLower() < s2.tag.toLower();
      }

//---------------------------------------------------------
//   genIndex
//    gen web index
//---------------------------------------------------------

void genIndex(const QString& of, const QList<Index> lst, int lang)
      {
      QFile ff(of);
      if (!ff.open(QIODevice::WriteOnly)) {
            printf("cannot open <%s>\n", qPrintable(of));
            return;
            }
      QTextStream os(&ff);
      os.setCodec("UTF-8");
      os << "<?php\n";
      os << "  $file=\"man/reference.php\";\n";
      os << "  $level=\"../..\";";
      os << "  require(\"../header.html\");\n";
      os << "  ?>\n";
      if (lang == 0)
            os << "<h4><a href=\"idx.php\">MuseScore</a> -- <a href=\"manual.php\">Dokumentation</a> -- Index</h4>\n";
      else
            os << "<h4><a href=\"idx.php\">MuseScore</a> -- <a href=\"manual.php\">Documentation</a> -- Index</h4>\n";
      os << "<table>\n";

      int columns = 3;
      int rows    = (lst.size() + columns - 1) / columns;

      for (int row = 0; row < rows; ++row) {
            os << "  <tr>\n";
            for (int col = 0; col < columns; ++col) {
                  os << "    <td>";
                  int idx = col * rows + row;
                  if (idx < lst.size()) {
                        QString ref = lst[idx].target + ".php";
                        // checkFile(ocol, ref);
                        os << QString("<a href=\"%1\">").arg(ref);
                        os << lst[idx].tag;
                        os << "</a>";
                        }
                  os << "</td>\n";
                  if ((col + 1) < columns) {
                        if (idx == 0)
                              os << "    <td>&nbsp;&nbsp;&nbsp;</td>\n";
                        else
                              os << "    <td/>\n";
                        }
                  }
            os << "  </tr>\n";
            }
      os << "</table>\n";
      os << "<?php require(\"../trailer.html\");  ?>\n";
      ff.close();
      }

//---------------------------------------------------------
//   genIndexProg
//---------------------------------------------------------

void genIndexProg(const QString& of, const QList<Index> lst, int lang)
      {
      QFile ff(of);
      if (!ff.open(QIODevice::WriteOnly)) {
            printf("cannot open <%s>\n", qPrintable(of));
            return;
            }
      QTextStream os(&ff);
      os.setCodec("UTF-8");
      os << "<html>\n";
      os << "<head>\n";
      os << "  <meta content=\"text/html; charset=UTF-8\">\n";
      os << "  </head>\n";
      os << "<body>\n";

      os << "<h4>MuseScore</a> -- Index</h4>\n";
      os << "<table>\n";

      int columns = 3;
      int rows    = (lst.size() + columns - 1) / columns;

      for (int row = 0; row < rows; ++row) {
            os << "  <tr>\n";
            for (int col = 0; col < columns; ++col) {
                  os << "    <td>";
                  int idx = col * rows + row;
                  if (idx < lst.size()) {
                        QString ref = lst[idx].target + ".html";
                        // checkFile(ocol, ref);
                        os << QString("<a href=\"%1\">").arg(ref);
                        os << lst[idx].tag;
                        os << "</a>";
                        }
                  os << "</td>\n";
                  if ((col + 1) < columns) {
                        if (idx == 0)
                              os << "    <td>&nbsp;&nbsp;&nbsp;</td>\n";
                        else
                              os << "    <td/>\n";
                        }
                  }
            os << "  </tr>\n";
            }
      os << "</table>\n";
      os << "</body>\n";
      os << "</html>\n";
      ff.close();
      }

//---------------------------------------------------------
//   addAttributes
//---------------------------------------------------------

void addAttributes(QDomElement e, QString& s)
      {
      QDomNamedNodeMap map = e.attributes();
      int n = map.size();
      for (int i = 0; i < n; ++i) {
            QDomAttr a = map.item(i).toAttr();
            s += QString(" %1=\"%2\"").arg(a.name()).arg(a.value());
            }
      }

//---------------------------------------------------------
//   genHtml
//---------------------------------------------------------

void genHtml(QDomElement e, Xml& xml)
      {
      QString s = e.tagName();

      if (s == "img" || s == "br") {
            addAttributes(e, s);
            xml.tagE(s);
            return;
            }
      if (s == "key") {
            xml << QString("&lt;<b>%1</b>&gt;").arg(e.text());
            return;
            }
      if (s == "sp") {
            xml << "&nbsp;&nbsp;";
            return;
            }

      if (s == "b" || s == "i")           // inline this tags
            xml << "<" << s << ">";
      else {
            addAttributes(e, s);
            xml.stag(s);
            }

      for (QDomNode ee = e.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
            if (ee.nodeType() == QDomNode::ElementNode)
                  genHtml(ee.toElement(), xml);
            else if (ee.nodeType() == QDomNode::TextNode)
                  xml << Xml::xmlString(ee.toText().data());
            }
      if (s == "b" || s == "i")           // inlined tags
            xml << "</" << s << ">";
      else
            xml.etag();
      }

//---------------------------------------------------------
//   genPage
//---------------------------------------------------------

void genPage(const QString& dir, QDomElement e, int lang)
      {
      QString name   = e.attribute("name");
      QString header = e.attribute("header");
      if (name.isEmpty()) {
            printf("genPage: empty name\n");
            return;
            }
      if (header.isEmpty()) {
            printf("genPage: empty header\n");
            return;
            }

      printf("create page %s\n", qPrintable(name));

      QString docFile = dir + "/" + name + ".php";
      QFile qf(docFile);
      if (!qf.open(QIODevice::WriteOnly)) {
            printf("cannot open <%s>\n", qPrintable(qf.fileName()));
            return;
            }
      Xml xml(&qf);
      xml << "<?php\n";
      xml << QString("  $file=\"man/%1.php\";\n").arg(name);
      xml << "  $level=\"../..\";";
      xml << "  require(\"../header.html\");\n";
      xml << "  ?>\n";
      if (lang == 0) {
            xml << QString("<h4><a href=\"../idx.php\">MuseScore</a> -- "
                   "<a href=\"../manual.php\">Dokumentation</a> -- "
                   "<a href=\"reference.php\">Index</a> -- %1</h4>\n").arg(header);
            }
      else if (lang == 1) {
            xml << QString("<h4><a href=\"../idx.php\">MuseScore</a> -- "
                   "<a href=\"../manual.php\">Documentation</a> -- "
                   "<a href=\"reference.php\">Index</a> -- %1</h4>\n").arg(header);
            }

      QDomNode ee = e;
      for (ee = ee.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
            if (ee.nodeType() == QDomNode::TextNode)
                  xml << Xml::xmlString(ee.toText().data());
            else if (ee.nodeType() == QDomNode::ElementNode) {
                  QDomElement de = ee.toElement();
                  if (de.tagName() == "index") {
                        idxList.append(Index(de.text(), name));
                        }
                  else
                        genHtml(ee.toElement(), xml);
                  }
            }

      xml << "<?php require(\"../trailer.html\");  ?>\n";
      qf.close();
      }

//---------------------------------------------------------
//   genProgPage
//---------------------------------------------------------

void genProgPage(const QString& dir, QDomElement e, int lang)
      {
      QString name   = e.attribute("name");
      QString header = e.attribute("header");
      if (name.isEmpty()) {
            printf("genPage: empty name\n");
            return;
            }
      if (header.isEmpty()) {
            printf("genPage: empty header\n");
            return;
            }
      printf("create page %s\n", qPrintable(name));
      QString docFile = dir + "/" + name + ".html";
      QFile qf(docFile);
      if (!qf.open(QIODevice::WriteOnly)) {
            printf("cannot open <%s>\n", qPrintable(qf.fileName()));
            return;
            }
      Xml xml(&qf);

      xml << "<html>\n";
      xml << "<head>\n";
      xml << "  <meta content=\"text/html; charset=UTF-8\">\n";
      xml << "  </head>\n";
      xml << "<body>\n";

      xml << QString("<h4>MuseScore -- <a href=\"index.html\">Index</a> -- %1</h4>\n").arg(header);

      QDomNode ee = e;
      for (ee = ee.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
            if (ee.nodeType() == QDomNode::TextNode)
                  xml << Xml::xmlString(ee.toText().data());
            else if (ee.nodeType() == QDomNode::ElementNode) {
                  QDomElement de = ee.toElement();
                  if (de.tagName() == "index") {
                        idxList.append(Index(de.text(), name));
                        }
                  else if (de.tagName() == "a") {
                        QString href = de.attribute("href");
                        if (!href.endsWith(".php")) {
                              genHtml(ee.toElement(), xml);
                              }
                        else {
                              xml << "<a href=\"" << href.left(href.size() - 4)
                                  << ".html\">";
                              for (QDomNode eee = ee.firstChild(); !eee.isNull(); eee = eee.nextSibling()) {
                                    if (eee.nodeType() == QDomNode::ElementNode)
                                          genHtml(eee.toElement(), xml);
                                    else if (eee.nodeType() == QDomNode::TextNode)
                                          xml << Xml::xmlString(eee.toText().data());
                                    }
                              xml << "</a>";
                              }
                        }
                  else if (de.tagName() == "img") {
                        QString src = de.attribute("src");
                        QFile f(srcPathPrefix + QString("/pic/") + src);
                        if (!f.exists())
                              printf("image <%s> does not exist\n", qPrintable(f.fileName()));
                        genHtml(ee.toElement(), xml);
                        }
                  else
                        genHtml(ee.toElement(), xml);
                  }
            }

      xml << "</body>\n";
      xml << "</html>\n";
      qf.close();
      }

//---------------------------------------------------------
//   genWeb
//---------------------------------------------------------

static void genWeb(const QString& dir, const QDomDocument& doc, int lang, bool web)
      {
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScoreManual") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "page") {
                              if (web)
                                    genPage(dir, ee, lang);
                              else
                                    genProgPage(dir, ee, lang);
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   createWebDoc
//---------------------------------------------------------

static int createWebDoc()
      {
      for (unsigned i = 0; i < sizeof(manuals)/sizeof(*manuals); ++i) {
            QString src = srcPathPrefix + QString("/") + manuals[i].src;
            QString dst = dstPathPrefix + QString("/") + manuals[i].dst;

            idxList.clear();

            QFile qf(src);
            if (!qf.open(QIODevice::ReadOnly)) {
                  printf("cannot open <%s>\n", qPrintable(qf.fileName()));
                  return -1;
                  }
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(&qf, false, &err, &line, &column)) {
                  printf("error reading doc file %s at line %d column %d: %s\n",
                     qPrintable(qf.fileName()), line, column, qPrintable(err));
                  return -1;
                  }
            docName = src;
            genWeb(dst, doc, i, true);
            qSort(idxList.begin(), idxList.end(), sort);

            QString idst = dstPathPrefix + QString("/") + manuals[i].idx;
            genIndex(idst, idxList, i);
            }
      return 0;
      }

//---------------------------------------------------------
//   createProgramDoc
//---------------------------------------------------------

static int createProgramDoc()
      {
      for (unsigned i = 0; i < sizeof(manuals)/sizeof(*manuals); ++i) {
            QString src = srcPathPrefix + QString("/") + manuals[i].src;
            QString dst = manuals[i].progDst;

            idxList.clear();

            QFile qf(src);
            if (!qf.open(QIODevice::ReadOnly)) {
                  printf("cannot open <%s>\n", qPrintable(qf.fileName()));
                  return -1;
                  }
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(&qf, false, &err, &line, &column)) {
                  printf("error reading doc file %s at line %d column %d: %s\n",
                     qPrintable(qf.fileName()), line, column, qPrintable(err));
                  return -1;
                  }
            docName = src;
            genWeb(dst, doc, i, false);
            qSort(idxList.begin(), idxList.end(), sort);
            genIndexProg(manuals[i].progIdx, idxList, i);
            }
      return 0;
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

void usage(const char* name, const char* txt)
      {
      printf("gendoc: usage: gendoc <options>\n");
      printf("  options:\n");
      printf("     -w       create web documentation (default)\n");
      printf("     -p       create program documentation\n");
      printf("     -sPath   source path prefix\n");
      printf("     -dPath   destination path prefix\n");
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      int c;
      int action = 0;
      while ((c = getopt(argc, argv, "wps:d:")) != EOF) {
            switch (c) {
                  case 'w':
                        action = 0;
                        break;
                  case 'p':
                        action = 1;
                        break;
                  case 's':
                        srcPathPrefix = optarg;
                        break;
                  case 'd':
                        dstPathPrefix = optarg;
                        break;
                  default:
                        usage(argv[0], "bad argument");
                        return -1;
                  }
            }
      switch(action) {
            case 0: return createWebDoc();
            case 1: return createProgramDoc();
            }
      }

