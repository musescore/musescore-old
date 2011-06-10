//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: file.cpp 3708 2010-11-16 09:54:31Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer
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

#include <QtCore/QStringList>
#include <QtCore/QRegExp>

/**
 File handling: loading and saving.
 */

#include "globals.h"
#include "element.h"
#include "note.h"
#include "rest.h"
#include "m-al/sig.h"
#include "clef.h"
#include "key.h"
#include "score.h"
#include "page.h"
#include "dynamics.h"
#include "style.h"
#include "m-al/tempo.h"
#include "select.h"
#include "preferences.h"
#include "staff.h"
#include "part.h"
#include "utils.h"
#include "barline.h"
#include "slur.h"
#include "hairpin.h"
#include "ottava.h"
#include "textline.h"
#include "pedal.h"
#include "trill.h"
#include "volta.h"
#include "timesig.h"
#include "box.h"
#include "excerpt.h"
#include "system.h"
#include "tuplet.h"
#include "keysig.h"
#include "zarchive/zarchive.h"
#include "measure.h"
#include "repeatlist.h"
#include "beam.h"
#include "stafftype.h"
#include "seq.h"
#include "lyrics.h"
#include "segment.h"
#include "tempotext.h"
#include "m-al/xml.h"

#define VERSION "2.0.0"

extern int division;

//---------------------------------------------------------
//   loadCompressedMsc
//    return false on error
//---------------------------------------------------------

bool Score::loadCompressedMsc(QString name)
      {
      info.setFile(name);

      Unzip uz;
      if (!uz.openArchive(name))
            return false;

      QBuffer cbuf;
      cbuf.open(QIODevice::WriteOnly);
      uz.extractFile("META-INF/container.xml", &cbuf);

      QString rootfile = "";
      XmlReader r(cbuf.data().data(), cbuf.data().size());
      if (!r.readElement())
            return false;
      if (r.tag() != "container") {
            r.unknown();
            return false;
            }
      if (!r.readElement())
            return false;
      if (r.tag() != "rootfiles") {
            r.unknown();
            return false;
            }
      QString val;
      while (r.readElement()) {
            if (r.tag() == "rootfile") {
                  while (r.readAttribute()) {
                        if (rootfile.isEmpty() && (r.tag() == "full-path"))
                              rootfile = r.stringValue();
                        }
                  }
            else if (r.readString("file", &val)) {
                  ImagePath* ip = new ImagePath(val);
                  imagePathList.append(ip);
                  }
            else
                  r.unknown();
            }
      //
      // load images
      //
      foreach(ImagePath* ip, imagePathList) {
            QBuffer& dbuf = ip->buffer();
            dbuf.open(QIODevice::WriteOnly);
            if (uz.extractFile(ip->path(), &dbuf))
                  ip->setLoaded(true);
            }

      if (rootfile.isEmpty())
            return false;

      QBuffer dbuf;
      dbuf.open(QIODevice::WriteOnly);
      uz.extractFile(rootfile, &dbuf);

      XmlReader reader(dbuf.data().data(), dbuf.data().size());
      dbuf.close();
      return read1(&reader);
      }

//---------------------------------------------------------
//   loadMsc
//    return false if file not found or error loading
//---------------------------------------------------------

bool Score::loadMsc(QString name)
      {
      info.setFile(name);

      QFile f(name);
      if (!f.open(QIODevice::ReadOnly))
            return false;

      QByteArray ba = f.readAll();
      XmlReader reader(ba.data(), ba.size());
      f.close();
      return read1(&reader);
      }

//---------------------------------------------------------
//   checkProgramVersion
//---------------------------------------------------------

bool Score::checkProgramVersion(const QString& version)
      {
      QStringList sl = version.split('.');
      _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
      if (_mscVersion > MSCVERSION) {
            // incompatible version
            printf("incompatible version %d > %d\n", _mscVersion, MSCVERSION);
            return false;
            }
      printf("version %d\n", _mscVersion);
      return true;
      }

//---------------------------------------------------------
//   checkScoreVersion
//---------------------------------------------------------

bool Score::checkScoreVersion(const QString& version)
      {
      QRegExp re("(\\d+)\\.(\\d+)\\.(\\d+)");
      int v1, v2, v3, rv1, rv2, rv3;
      if (re.indexIn(VERSION) != -1) {
            QStringList sl = re.capturedTexts();
            if (sl.size() == 4) {
                  v1 = sl[1].toInt();
                  v2 = sl[2].toInt();
                  v3 = sl[3].toInt();
                  if (re.indexIn(version) != -1) {
                        sl = re.capturedTexts();
                        if (sl.size() == 4) {
                              rv1 = sl[1].toInt();
                              rv2 = sl[2].toInt();
                              rv3 = sl[3].toInt();
                              int currentVersion = v1 * 10000 + v2 * 100 + v3;
                              int readVersion    = rv1 * 10000 + rv2 * 100 + rv3;
                              if (readVersion > currentVersion) {
                                    printf("read future version %d > %d\n", readVersion, currentVersion);
                                    return false;
                                    }
                              }
                        }
                  else {
                        printf("1cannot parse <%s>\n", qPrintable(version));
                        return false;
                        }
                  }
            }
      else
            printf("2cannot parse <%s>\n", VERSION);
      return true;
      }

//---------------------------------------------------------
//   read1
//---------------------------------------------------------

bool Score::read1(XmlReader* r)
      {
      _elinks.clear();

      while (r->readElement()) {
            if (r->tag() == "museScore") {
                  while (r->readAttribute()) {
                        if (r->tag() == "version") {
                              if (!checkProgramVersion(r->stringValue()))
                                    return false;
                              }
                        }
                  break;
                  }
            else
                  r->unknown();
            }
      while (r->readElement()) {
            QString data;
            if (r->readString("programVersion", &data)) {
                  if (!checkScoreVersion(data))
                        return false;
                  }
            else if (r->readString("programRevision", &data))
                  ;
            else if (r->tag() == "Score")
                  read(r);
            else
                  r->unknown();
            }
      return true;
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool Score::read(XmlReader* r)
      {
      _fileDivision = 384;   // for compatibility with old mscore files
      slurs.clear();

      if (parentScore())
            setMscVersion(parentScore()->mscVersion());

      curTrack = -1;
      QString data;
      qreal val;
      int i;
      while (r->readElement()) {
            MString8 tag(r->tag());
            if (tag == "Staff")
                  readStaff(r);
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(this);
                  ks->read(r);
                  customKeysigs.append(ks);
                  }
            else if (tag == "StaffType") {
                  int idx = 0;
                  while (r->readAttribute()) {
                        if (r->tag() == "idx")
                              idx = r->intValue();
                        }
                  StaffType* ost = _staffTypes.value(idx);
                  StaffType* st  = ost ? new StaffType(*ost) : new StaffType;
                  st->read(r);
                  if (_staffTypes.value(idx)) {            // if there is already a stafftype
                        if (_staffTypes[idx]->modified())  // if it belongs to Score()
                              delete _staffTypes[idx];
                        _staffTypes[idx] = st;
                        }
                  else
                        _staffTypes.append(st);
                  st->setModified(true);
                  }
            else if (tag == "siglist")
                  _sigmap->read(r, _fileDivision);
            else if (r->readString("programVersion", &data)) {
                  if (!checkScoreVersion(data))
                        return false;
                  }
            else if (r->readString("programRevision", &data))
                  ;
            else if (r->readString("showOmr", &data))
                  ;
            else if (tag == "SyntiSettings") {
                  _syntiState.clear();
                  _syntiState.read(r);
                  }
            else if (r->readReal("Spatium", &val))
                  setSpatium (val * DPMM);
            else if (r->readInt("Division", &_fileDivision))
                  ;
            else if (r->readBool("showInvisible", &_showInvisible))
                  ;
            else if (r->readBool("showFrames", &_showFrames))
                  ;
            else if (tag == "Style")
                  _style.load(r);
            else if (tag == "page-layout")
                  pageFormat()->read(r);
            else if (r->readString("copyright", &data) || r->readString("rights", &data))
                  setMetaTag("copyright", data);
            else if (r->readString("movement-number", &data))
                  setMetaTag("movementNumber", data);
            else if (r->readString("movement-title", &data))
                  setMetaTag("movementTitle", data);
            else if (r->readString("work-number", &data))
                  setMetaTag("workNumber", data);
            else if (r->readString("work-title", &data))
                  setMetaTag("workTitle", data);
            else if (r->readString("source", &data))
                  setMetaTag("source", data);
//            else if (tag == "metaTag") {
//                  QString name = ee.attribute("name");
//                  setMetaTag(name, val);
//                  }
            else if (tag == "Part") {
                  Part* part = new Part(this);
                  part->read(r);
                  _parts.push_back(part);
                  }
            else if (r->readBool("showInvisible", &_showInvisible))
                  ;
            else if (r->readBool("showFrames", &_showFrames))
                  ;
            else if (tag == "Slur") {
                  Slur* slur = new Slur(this);
                  slur->read(r);
                  slurs.append(slur);
                  }
            else if (tag == "Excerpt")
                  r->skipElement((const xmlChar*)"Excerpt");
            else if (tag == "Beam") {
                  Beam* beam = new Beam(this);
                  beam->read(r);
                  beam->setParent(0);
                  _beams.append(beam);
                  }
            else if (tag == "Score") {          // recursion
                  Score* s = new Score();
                  s->setStyle(style());
                  s->setParentScore(this);
                  s->read(r);
//TODOplayer                  addExcerpt(s);
                  }
            else if (r->readString("name", &data))
                  setName(data);
            else if (r->readInt("cursorTrack", &i))
                  ;
            else
                  r->unknown();
            }

      // check slurs
      foreach(Slur* slur, slurs) {
            if (!slur->startElement() || !slur->endElement()) {
                  if (slur->startElement()) {
                        // printf("  front %d\n", static_cast<ChordRest*>(slur->startElement())->tick());
                        static_cast<ChordRest*>(slur->startElement())->removeSlurFor(slur);
                        }
                  if (slur->endElement()) {
                        // printf("  back %d\n", static_cast<ChordRest*>(slur->endElement())->tick());
                        static_cast<ChordRest*>(slur->endElement())->removeSlurBack(slur);
                        }
                  }
            }
      slurs.clear();
      connectTies();
      setInstrumentNames();

      _fileDivision = division;

      //
      //    sanity check for barLineSpan
      //
      foreach(Staff* staff, _staves) {
            int barLineSpan = staff->barLineSpan();
            int idx = staffIdx(staff);
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  // printf("bad span: idx %d  span %d staves %d\n", idx, barLineSpan, n);
                  staff->setBarLineSpan(n - idx);
                  }
            }

      renumberMeasures();
      rebuildMidiMapping();
      updateChannel();
      return true;
      }

