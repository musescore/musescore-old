//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id: xml.cpp,v 1.20 2006/04/05 08:15:12 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
      *this << "<?xml version=\"1.0\" encoding=\"utf8\"?>\n";
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

void Xml::tag(const char* name, QVariant data)
      {
      putLevel();
      switch(data.type()) {
            case QVariant::Bool:
            case QVariant::Char:
            case QVariant::Int:
                  *this << "<" << name << ">";
                  *this << data.toInt();
                  *this << "</" << name << ">\n";
                  break;
            case QVariant::Double:
                  *this << "<" << name << ">";
                  *this << data.value<double>();
                  *this << "</" << name << ">\n";
                  break;
            case QVariant::String:
                  *this << "<" << name << ">";
                  *this << xmlString(data.value<QString>());
                  *this << "</" << name << ">\n";
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

void Xml::tag(const char* name, const char* attributes, const QString& s)
      {
      putLevel();
      *this << "<" << name << " " << attributes << ">" << s << "</" << name << ">\n";
      }

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
//   readPoint
//---------------------------------------------------------

QPointF readPoint(QDomNode node)
      {
      QDomElement e = node.toElement();
      QPointF p;
      p.setX(e.attribute("x", "0.0").toDouble());
      p.setY(e.attribute("y", "0.0").toDouble());
      return p;
      }

//---------------------------------------------------------
//   readSize
//---------------------------------------------------------

QSizeF readSize(QDomNode node)
      {
      QDomElement e = node.toElement();
      QSizeF p;
      p.setWidth(e.attribute("w", "0.0").toDouble());
      p.setHeight(e.attribute("h", "0.0").toDouble());
      return p;
      }

//---------------------------------------------------------
//   domError
//---------------------------------------------------------

void domError(QDomNode node)
      {
      QDomElement e = node.toElement();
      QString tag(e.tagName());
      QString s;
      QDomNode dn(node);
      while (!dn.parentNode().isNull()) {
            dn = dn.parentNode();
            const QDomElement e = dn.toElement();
            const QString k(e.tagName());
            if (!s.isEmpty())
                  s += ":";
            s += k;
            }
      fprintf(stderr, "%s: Unknown Node <%s>, type %d\n",
         s.toLatin1().data(), tag.toLatin1().data(), node.nodeType());
      if (node.isText()) {
            fprintf(stderr, "  text node <%s>\n", node.toText().data().toLatin1().data());
            }
      }

//---------------------------------------------------------
//   domNotImplemented
//---------------------------------------------------------

void domNotImplemented(QDomNode node)
      {
      if (!debugMode)
            return;
      QDomElement e = node.toElement();
      QString tag(e.tagName());
      QString s;
      QDomNode dn(node);
      while (!dn.parentNode().isNull()) {
            dn = dn.parentNode();
            const QDomElement e = dn.toElement();
            const QString k(e.tagName());
            if (!s.isEmpty())
                  s += ":";
            s += k;
            }
      fprintf(stderr, "%s: Node not implemented: <%s>, type %d\n",
         s.toLatin1().data(), tag.toLatin1().data(), node.nodeType());
      if (node.isText()) {
            fprintf(stderr, "  text node <%s>\n", node.toText().data().toLatin1().data());
            }
      }

