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

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

Xml::Xml()
      {
      level = 0;
      curTick = 0;
      }

Xml::Xml(QIODevice* device)
   : QTextStream(device)
      {
      setCodec("utf8");
      level = 0;
      curTick = 0;
      }

//---------------------------------------------------------
//   putLevel
//---------------------------------------------------------

void Xml::putLevel()
      {
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
      ++level;
      }

//---------------------------------------------------------
//   etag
//    </mops>
//---------------------------------------------------------

void Xml::etag(const char* s)
      {
      putLevel();
      *this << "</" << s << '>' << endl;
      --level;
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

void Xml::tag(const char* name, int val)
      {
      putLevel();
      *this << "<" << name << ">" << val << "</" << name << ">\n";
      }

void Xml::tag(const char* name, QChar val)
      {
      putLevel();
      *this << "<" << name << ">" << val << "</" << name << ">\n";
      }

void Xml::tag(const char* name, double val)
      {
      putLevel();
      QString s("<%1>%2</%3>\n");
      *this << s.arg(name).arg(val).arg(name);
      }

void Xml::tag(const char* name, float val)
      {
      putLevel();
      *this << QString("<%1>%2</%3>\n").arg(name).arg(val).arg(name);
      }

void Xml::tag(const char* name, const char* s)
      {
      tag(name, QString(s));
      }

void Xml::tag(const char* name, const QString& s)
      {
      putLevel();
      *this << "<" << name << ">";
      *this << xmlString(s) << "</" << name << ">\n";
      }

void Xml::tag(const char* name, const char* attributes, const QString& s)
      {
      putLevel();
      *this << "<" << name << " " << attributes << ">" << s << "</" << name << ">\n";
      }

void Xml::tag(const char* name, const QColor& color)
      {
      putLevel();
    	char buffer[BS];
      snprintf(buffer, BS, "<%s r=\"%d\" g=\"%d\" b=\"%d\"/>\n",
	    name, color.red(), color.green(), color.blue());
    	*this << buffer;
      }

void Xml::tag(const char* name, const QWidget* g)
      {
      tag(name, QRect(g->pos(), g->size()));
      }

void Xml::tag(const char* name, const QRect& r)
      {
      putLevel();
   	*this << "<" << name;
    	char buffer[BS];
      snprintf(buffer, BS, " x=\"%d\" y=\"%d\" w=\"%d\" h=\"%d\" />\n",
         r.x(), r.y(), r.width(), r.height());
    	*this << buffer;
      }

void Xml::tag(const char* const name, const QPointF& p)
      {
      putLevel();
      QString s("<%1 x=\"%2\" y=\"%3\"/>\n");
      *this << s.arg(name).arg(p.x()).arg(p.y());
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

