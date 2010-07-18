//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "importgtp.h"
#include "score.h"
#include "measurebase.h"
#include "text.h"
#include "box.h"
#include "staff.h"
#include "part.h"
#include "measure.h"

//---------------------------------------------------------
//   errmsg
//---------------------------------------------------------

const char* GuitarPro::errmsg[] = {
      "no error",
      "unknown file format",
      "unexpected end of file",
      };

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

GuitarPro::GuitarPro()
      {
      }

GuitarPro::~GuitarPro()
      {
      }

//---------------------------------------------------------
//   skip
//---------------------------------------------------------

void GuitarPro::skip(qint64 len)
      {
      char c;
      while (len--)
            read(&c, 1);
      }

//---------------------------------------------------------
//    read
//---------------------------------------------------------

void GuitarPro::read(void* p, qint64 len)
      {
      if (len == 0)
            return;
      qint64 rv = f->read((char*)p, len);
      if (rv != len)
            throw GP_EOF;
      curPos += len;
      }

//---------------------------------------------------------
//   readChar
//---------------------------------------------------------

int GuitarPro::readChar()
      {
      char c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readUChar
//---------------------------------------------------------

int GuitarPro::readUChar()
      {
      uchar c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readPascalString
//---------------------------------------------------------

QString GuitarPro::readPascalString(int n)
      {
      uchar l;
      read(&l, 1);
      char s[l + 1];
      read(s, l);
      s[l] = 0;
      skip(n - l);
      return QString(s);
      }

//---------------------------------------------------------
//   readWordPascalString
//---------------------------------------------------------

QString GuitarPro::readWordPascalString()
      {
      int l = readDelphiInteger();
      char c[l+1];
      read(c, l);
      c[l] = 0;
      return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readDelphiInteger
//---------------------------------------------------------

int GuitarPro::readDelphiInteger()
      {
      uchar x;
      read(&x, 1);
      int r = x;
      read(&x, 1);
      r += x << 8;
      read(&x, 1);
      r += x << 16;
      read(&x, 1);
      r += x << 24;
      return r;
      }

//---------------------------------------------------------
//   readDelphiString
//---------------------------------------------------------

QString GuitarPro::readDelphiString()
      {
      int maxl = readDelphiInteger();
      uchar l;
      read(&l, 1);
      if (maxl != l + 1)
            printf("readDelphiString: first word doesn't match second byte");
      char c[l + 5];
      read(c, l);
      c[l] = 0;

      printf("read string <%s>\n", c);
      return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro::read(QFile* fp)
      {
      f      = fp;
      curPos = 0;
      QString s = readPascalString(30);
      printf("read <%s>\n", qPrintable(s));

      if (!s.startsWith("FICHIER GUITAR PRO "))
            throw GP_UNKNOWN_FORMAT;
      s = s.mid(20);

      int a = s.left(1).toInt();
      int b = s.mid(1).toInt();
      version = a * 100 + b;
      if (version != 101 && version != 102 && version != 103 && version != 104
         && version != 220 && version != 221 && version != 300
         && version != 400 && version != 406)
            throw GP_UNKNOWN_FORMAT;
printf("found GTP format version %d\n", version);

      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      copyright    = readDelphiString();
      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readDelphiInteger();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      uchar num;
      read(&num, 1);    // Shuffle rhythm feel
      if (version >= 400) {
            readDelphiInteger();
            for (int i = 0; i < GP_MAX_LYRIC_LINES; ++i) {
                  readDelphiInteger();
                  readWordPascalString();
                  }
            }
      int tempo = readDelphiInteger();
      printf("Tempo: %d\n", tempo);
      if (version >= 400) {
            uchar num;
            read(&num, 1);          // key
            readDelphiInteger();    // octave
            }
      else
            readDelphiInteger();    // key

      for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
            trackDefaults[i].patch   = readDelphiInteger();
            trackDefaults[i].volume  = readUChar();
            trackDefaults[i].pan     = readUChar();
            trackDefaults[i].chorus  = readUChar();
            trackDefaults[i].reverb  = readUChar();
            trackDefaults[i].phase   = readUChar();
            trackDefaults[i].tremolo = readUChar();
            uchar a, b;
            read(&a, 1);
            read(&b, 1);
            if (a != 0 || b != 0) {
                  printf("wrong byte padding\n");
                  }
            }
      numBars   = readDelphiInteger();
      numTracks = readDelphiInteger();

      printf("tracks %d  bars %d\n", numTracks, numBars);

      }

//---------------------------------------------------------
//   importGTP
//---------------------------------------------------------

bool Score::importGTP(const QString& name)
      {
      if (name.isEmpty())
            return false;
      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly))
            return false;

      GuitarPro gp;
      try {
            gp.read(&fp);
            }
      catch(GuitarPro::GuitarProError errNo) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Import GuitarPro"),
               QString("Load failed: ") + gp.error(errNo),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            fp.close();
            // avoid another error message box
            return true;
            }
      fp.close();

      MeasureBase* m;
      if (!_measures.first()) {
            m = new VBox(this);
            m->setTick(0);
            addMeasure(m);
            }
      else  {
            m = _measures.first();
            if (m->type() != VBOX) {
                  m = new VBox(this);
                  m->setTick(0);
                  addMeasure(m);
                  }
            }
      if (!gp.title.isEmpty()) {
            Text* s = new Text(this);
            s->setText(gp.title);
            s->setSubtype(TEXT_TITLE);
            s->setTextStyle(TEXT_STYLE_TITLE);
            m->add(s);
            }
      if (!gp.subtitle.isEmpty()) {
            Text* s = new Text(this);
            s->setText(gp.subtitle);
            s->setSubtype(TEXT_SUBTITLE);
            s->setTextStyle(TEXT_STYLE_SUBTITLE);
            m->add(s);
            }
      if (!gp.composer.isEmpty()) {
            Text* s = new Text(this);
            s->setText(gp.composer);
            s->setSubtype(TEXT_COMPOSER);
            s->setTextStyle(TEXT_STYLE_COMPOSER);
            m->add(s);
            }
      if (!gp.artist.isEmpty()) {
            Text* s = new Text(this);
            s->setText(gp.artist);
            s->setSubtype(TEXT_POET);
            s->setTextStyle(TEXT_STYLE_POET);
            m->add(s);
            }

//      album        = readDelphiString();
//      copyright    = readDelphiString();

      Part* part = new Part(this);
      for (int staffIdx = 0; staffIdx < gp.numTracks; ++staffIdx) {
            Staff* s = new Staff(this, part, staffIdx);
            part->insertStaff(s);
            _staves.push_back(s);
            }
      _parts.push_back(part);

      int tick = 0;
      Fraction ts(4,4);
      for (int i = 0; i < gp.numBars; ++i) {
            Measure* m = new Measure(this);
            m->setTick(tick);
            m->setTimesig(ts);
            m->setLen(ts);
            add(m);
            tick += ts.ticks();
            }

      _saved = false;
      _created = true;
      return true;
      }

