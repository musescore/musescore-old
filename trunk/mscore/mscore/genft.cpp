
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/tttables.h>
#include "al/xml.h"

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
            fprintf(stderr, "cannot load table LILC\n");
            exit(-3);
            }
      FT_Byte* buffer = (FT_Byte*)malloc(length + 1);
      error = FT_Load_Sfnt_Table(face, tag, 0, buffer, &length);
      buffer[length] = 0;
      if (error) {
            fprintf(stderr, "cannot load font table LILC\n");
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
//      printf("strings %d %d\n", sl.size(), sl.size() % 5);
      QRegExp ra("\\(attachment\\s\\.\\s\\(([0-9\\+\\-\\.]{1,})\\s\\.\\s([0-9\\+\\-\\.]{1,})\\)\\)\\)\\)");
      QRegExp rb("\\(\\(bbox\\s\\.\\s\\(([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\s([0-9\\+\\-\\.]{1,})\\)\\)");
      int n = sl.size();
      for (int i = 0; (i+4) < n; i += 5) {
            Glyph g;
            QString s = sl[i];
            int nn = s.size();
            s = s.mid(1, nn - 3);
//            printf("<%s>\n", qPrintable(s));
            g.name = s;
            int idx = 0;
            if (namemap.contains(s))
                  idx = namemap[s];
            else
                  fprintf(stderr, "<%s> not in map\n", qPrintable(s));
            int code = 0;
            if (codemap.contains(idx))
                  code = codemap[idx];
            else
                  fprintf(stderr, "codemap has no index %d\n", idx);
            g.code = code;

            s = sl[i+4];
            int val = ra.indexIn(s);
            if (val == -1 || ra.numCaptures() != 2) {
                  printf("bad reg expr a\n");
                  exit(-5);
                  }
            g.attach.rx() = ra.cap(1).toDouble();
            g.attach.ry() = ra.cap(2).toDouble();

            s = sl[i+1];
            val = rb.indexIn(s);
            if (val == -1 || rb.numCaptures() != 4) {
                  printf("bad reg expr b\n");
                  exit(-5);
                  }
            g.bbox = QRectF(rb.cap(1).toDouble(), rb.cap(2).toDouble(), rb.cap(3).toDouble(), rb.cap(4).toDouble());
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
            fprintf(stderr, "init free type library failed\n");
            exit(-1);
            }
      FT_Face face;
      int error = FT_New_Face(library, argv[1], 0, &face);
      if (error) {
            fprintf(stderr, "open font failed <%s>\n", argv[1]);
            exit(-2);
            }

//      FT_CharMap current_cmap = face->charmap;
      FT_Select_Charmap(face, FT_ENCODING_UNICODE);

      FT_ULong charcode;
      FT_UInt gindex;

      for (charcode = FT_Get_First_Char(face, &gindex); gindex;
         charcode = FT_Get_Next_Char(face, charcode, &gindex)) {
            char name[256];
            FT_Get_Glyph_Name(face, gindex, name, 256);
            // printf("index %3d code 0x%04x <%s>\n", gindex, charcode, name);
            codemap[gindex] = charcode;
            namemap[name] = gindex;
            }

      char* p = getTable("LILC", face);
      parseLILC(p);
      p = getTable("LILY", face);
//      printf("<%s>\n", p);
      p = getTable("LILF", face);
//      printf("<%s>\n", p);
      genXml();
      return 0;
      }

