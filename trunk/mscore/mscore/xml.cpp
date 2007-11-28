//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id: xml.cpp,v 1.20 2006/04/05 08:15:12 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer and others
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

#include "xml.h"
#include "globals.h"

QString docName;

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

Xml::Xml()
      {
      stack.clear();
      curTick = 0;
      }

Xml::Xml(QIODevice* device)
   : QTextStream(device)
      {
      setCodec("utf8");
      stack.clear();
      curTick = 0;
      }

//---------------------------------------------------------
//   putLevel
//---------------------------------------------------------

void Xml::putLevel()
      {
      int level = stack.size();
      for (int i = 0; i < level * 2; ++i)
            *this << ' ';
      }

//---------------------------------------------------------
//   header
//---------------------------------------------------------

void Xml::header()
      {
      *this << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      }

//---------------------------------------------------------
//   stag
//    <mops attribute="value">
//---------------------------------------------------------

void Xml::stag(const QString& s)
      {
      putLevel();
      *this << '<' << s << '>' << endl;
      stack.append(s.split(' ')[0]);
      }

//---------------------------------------------------------
//   etag
//    </mops>
//---------------------------------------------------------

void Xml::etag()
      {
      putLevel();
      *this << "</" << stack.takeLast() << '>' << endl;
      }

//---------------------------------------------------------
//   tagE
//    <mops attribute="value"/>
//---------------------------------------------------------

void Xml::tagE(const char* format, ...)
      {
      va_list args;
      va_start(args, format);
      putLevel();
      *this << '<';
    	char buffer[BS];
      vsnprintf(buffer, BS, format, args);
    	*this << buffer;
      va_end(args);
      *this << "/>" << endl;
      }

//---------------------------------------------------------
//   tagE
//---------------------------------------------------------

void Xml::tagE(const QString& s)
      {
      putLevel();
      *this << '<' << s << "/>\n";
      }

//---------------------------------------------------------
//   ntag
//    <mops> without newline
//---------------------------------------------------------

void Xml::ntag(const char* name)
      {
      putLevel();
      *this << "<" << name << ">";
      }

//---------------------------------------------------------
//   netag
//    </mops>     without indentation
//---------------------------------------------------------

void Xml::netag(const char* s)
      {
      *this << "</" << s << '>' << endl;
      }

//---------------------------------------------------------
//   tag
//    <mops>value</mops>
//---------------------------------------------------------

void Xml::tag(const QString& name, QVariant data)
      {
      QString ename(name.split(' ')[0]);

      putLevel();
      switch(data.type()) {
            case QVariant::Bool:
            case QVariant::Char:
            case QVariant::Int:
                  *this << "<" << name << ">";
                  *this << data.toInt();
                  *this << "</" << ename << ">\n";
                  break;
            case QVariant::Double:
                  *this << "<" << name << ">";
                  *this << data.value<double>();
                  *this << "</" << ename << ">\n";
                  break;
            case QVariant::String:
                  *this << "<" << name << ">";
                  *this << xmlString(data.value<QString>());
                  *this << "</" << ename << ">\n";
                  break;
            case QVariant::Color:
                  {
                  QColor color(data.value<QColor>());
                  *this << QString("<%1 r=\"%2\" g=\"%3\" b=\"%4\"/>\n").arg(name).arg(color.red()).arg(color.green()).arg(color.blue());
                  }
                  break;
            case QVariant::Rect:
                  {
                  QRect r(data.value<QRect>());
                  *this << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg(name).arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
                  }
                  break;
            case QVariant::PointF:
                  {
                  QPointF p(data.value<QPointF>());
                  *this << QString("<%1 x=\"%2\" y=\"%3\"/>\n").arg(name).arg(p.x()).arg(p.y());
                  }
                  break;
            case QVariant::SizeF:
                  {
                  QSizeF p(data.value<QSizeF>());
                  *this << QString("<%1 w=\"%2\" h=\"%3\"/>\n").arg(name).arg(p.width()).arg(p.height());
                  }
                  break;
            default:
                  printf("Xml::tag: unsupported type %d\n", data.type());
                  break;
            }
      }

#if 0
void Xml::tag(const char* name, const QString& attributes, const QString& s)
      {
      putLevel();
      *this << "<" << name << " " << attributes << ">" << s << "</" << name << ">\n";
      }
#endif

void Xml::tag(const char* name, const QWidget* g)
      {
      tag(name, QRect(g->pos(), g->size()));
      }

//---------------------------------------------------------
//   xmlString
//---------------------------------------------------------

QString Xml::xmlString(const QString& ss)
      {
      QString s(ss);
      s.replace('&',  "&amp;");
      s.replace('<',  "&lt;");
      s.replace('>',  "&gt;");
      s.replace('\'', "&apos;");
      s.replace('"',  "&quot;");
      return s;
      }

 //---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Xml::dump(int len, const unsigned char* p)
      {
      putLevel();
      int col = 0;
      setFieldWidth(5);
      setNumberFlags(numberFlags() | QTextStream::ShowBase);
      setIntegerBase(16);
      for (int i = 0; i < len; ++i, ++col) {
            if (col >= 16) {
                  setFieldWidth(0);
                  *this << endl;
                  col = 0;
                  putLevel();
                  setFieldWidth(5);
                  }
            *this << (p[i] & 0xff);
            }
      if (col)
            *this << endl << dec;
      setFieldWidth(0);
      setIntegerBase(10);
      }

//---------------------------------------------------------
//   readPoint
//---------------------------------------------------------

QPointF readPoint(QDomElement e)
      {
      QPointF p;
      p.setX(e.attribute("x", "0.0").toDouble());
      p.setY(e.attribute("y", "0.0").toDouble());
      return p;
      }

//---------------------------------------------------------
//   readSize
//---------------------------------------------------------

QSizeF readSize(QDomElement e)
      {
      QSizeF p;
      p.setWidth(e.attribute("w", "0.0").toDouble());
      p.setHeight(e.attribute("h", "0.0").toDouble());
      return p;
      }

//---------------------------------------------------------
//   printDomElementPath
//---------------------------------------------------------

static QString domElementPath(QDomElement e)
      {
      QString s;
      QDomNode dn(e);
      while (!dn.parentNode().isNull()) {
            dn = dn.parentNode();
            const QDomElement e = dn.toElement();
            const QString k(e.tagName());
            if (!s.isEmpty())
                  s += ":";
            s += k;
            }
      return s;
      }

//---------------------------------------------------------
//   domError
//---------------------------------------------------------

void domError(QDomElement e)
      {
      QString s = domElementPath(e);
      if (!docName.isEmpty())
            fprintf(stderr, "<%s>:", qPrintable(docName));
      fprintf(stderr, "%s: Unknown Node <%s>, type %d\n",
         qPrintable(s), qPrintable(e.tagName()), e.nodeType());
      if (e.isText())
            fprintf(stderr, "  text node <%s>\n", qPrintable(e.toText().data()));
      }

//---------------------------------------------------------
//   domNotImplemented
//---------------------------------------------------------

void domNotImplemented(QDomElement e)
      {
      if (!debugMode)
            return;
      QString s = domElementPath(e);
      if (!docName.isEmpty())
            fprintf(stderr, "<%s>:", qPrintable(docName));
      fprintf(stderr, "%s: Node not implemented: <%s>, type %d\n",
         qPrintable(s), qPrintable(e.tagName()), e.nodeType());
      if (e.isText())
            fprintf(stderr, "  text node <%s>\n", qPrintable(e.toText().data()));
      }

//---------------------------------------------------------
//   htmlToString
//---------------------------------------------------------

void Xml::htmlToString(QDomElement e, int level, QString* s)
      {
      *s += QString("<%1").arg(e.tagName());
      QDomNamedNodeMap map = e.attributes();
      int n = map.size();
      for (int i = 0; i < n; ++i) {
            QDomAttr a = map.item(i).toAttr();
            *s += QString(" %1=\"%2\"").arg(a.name()).arg(a.value());
            }
      *s += ">";
      ++level;
      for (QDomNode ee = e.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
            if (ee.nodeType() == QDomNode::ElementNode)
                  htmlToString(ee.toElement(), level, s);
            else if (ee.nodeType() == QDomNode::TextNode)
                  *s += ee.toText().data();
            }
      *s += QString("</%1>").arg(e.tagName());
      --level;
      }

QString Xml::htmlToString(QDomElement e)
      {
      QString s;
      htmlToString(e, 0, &s);
      return s;
      }

