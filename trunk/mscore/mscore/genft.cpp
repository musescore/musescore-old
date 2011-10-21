//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

//
//    q+d hack to create an xml font description file from
//    lilipond tables embedded in mscore-20.ttf
//
//    usage: genft mscore-20.ttf > symbols.xml
//

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/tttables.h>
#include "xml.h"

QMap<int, int> codemap;
QMap<QString, int> namemap;

struct Glyph {
      QString name;
      int code;
      QPointF attach;
      QRectF bbox;
      };

QList<Glyph> glyphs;

//---------------------------------------------------------
//   getTable
//---------------------------------------------------------

static char* getTable(char* t, FT_Face face)
      {
      FT_ULong tag = FT_MAKE_TAG(t[0], t[1], t[2], t[3]);
      FT_ULong length = 0;
      int error = FT_Load_Sfnt_Table(face, tag, 0, NULL, &length);
      if (error) {
            qDebug("genft: cannot load table LILC\n");
            exit(-3);
            }
      FT_Byte* buffer = (FT_Byte*)malloc(length + 1);
      error = FT_Load_Sfnt_Table(face, tag, 0, buffer, &length);
      buffer[length] = 0;
      if (error) {
            qDebug("genft: cannot load font table LILC\n");
            exit(4);
            }
      return (char*)buffer;
      }

//---------------------------------------------------------
//   parseLILC
//    (rests.0 .
//    ((bbox . (-0.000000 -3.125000 7.500000 0.000000))
//    (subfont . "feta20")
//    (subfont-index . 33)
//    (attachment . (7.500000 . 0.000000))))
//---------------------------------------------------------

static void parseLILC(char* buffer)
      {
      QString s(buffer);
      QStringList sl = s.split("\n");
      QRegExp ra("\\(attachment\\s\\.\\s\\(([0-9\\+\\-\\.]{1,})\\s\\.\\s([0-9\\+\\-\\.]{1,})\\)\\)\\)\\)");
      QRegExp rb("\\(\\(bbox\\s\\.\\s\\(([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\)\\)");
      int n = sl.size();
      for (int i = 0; (i+4) < n; i += 5) {
            Glyph g;
            QString s = sl[i];
            int nn = s.size();
            s = s.mid(1, nn - 3);
            g.name = s;
            int idx = 0;
            if (namemap.contains(s))
                  idx = namemap[s];
            else
                  qDebug("genft: <%s> not in map\n", qPrintable(s));
            int code = 0;
            if (codemap.contains(idx))
                  code = codemap[idx];
            else
                  qDebug("codemap has no index %d\n", idx);
            g.code = code;

            s = sl[i+4];
            int val = ra.indexIn(s);
            if (val == -1 || ra.numCaptures() != 2) {
                  qDebug("bad reg expr a\n");
                  exit(-5);
                  }
            g.attach.rx() = ra.cap(1).toDouble();
            g.attach.ry() = -ra.cap(2).toDouble();

            s = sl[i+1];
            val = rb.indexIn(s);
            if (val == -1 || rb.numCaptures() != 4) {
                  qDebug("bad reg expr b\n");
                  exit(-5);
                  }
            double a = rb.cap(1).toDouble();
            double b = rb.cap(2).toDouble();
            double c = rb.cap(3).toDouble();
            double d = rb.cap(4).toDouble();
            g.bbox = QRectF(a, -d, c - a, d - b);
            glyphs.append(g);
            }
      }


//---------------------------------------------------------
//   genXml
//---------------------------------------------------------

static void genXml()
      {
      QFile f;
      f.open(stdout, QFile::WriteOnly);
      AL::Xml xml(&f);
      xml.header();
      xml.stag("museScore");
      foreach(const Glyph& g, glyphs) {
            xml.stag("Glyph");
            xml.tag("name",   g.name);
            xml.tag("code",   QString("0x%1").arg(g.code, 0, 16));
            xml.tag("attach", g.attach);
            xml.tag("bbox",   g.bbox);
            xml.etag();
            }
      xml.etag();
      f.close();
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int, char* argv[])
      {
      FT_Library library;

      if (FT_Init_FreeType(&library)) {
            qDebug("init free type library failed\n");
            exit(-1);
            }
      FT_Face face;
      int error = FT_New_Face(library, argv[1], 0, &face);
      if (error) {
            qDebug("open font failed <%s>\n", argv[1]);
            exit(-2);
            }

      FT_Select_Charmap(face, FT_ENCODING_UNICODE);

      FT_ULong charcode;
      FT_UInt gindex;

      for (charcode = FT_Get_First_Char(face, &gindex); gindex;
         charcode = FT_Get_Next_Char(face, charcode, &gindex)) {
            char name[256];
            FT_Get_Glyph_Name(face, gindex, name, 256);
            codemap[gindex] = charcode;
            namemap[name] = gindex;
            }

      char* p = getTable((char*)"LILC", face);
      parseLILC(p);
      // p = getTable("LILY", face);      // global values, not used now
      // p = getTable("LILF", face);      // subfont table, not used now
      genXml();
      return 0;
      }

