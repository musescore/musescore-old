//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "score.h"
#include "xml.h"
#include "element.h"
#include "measure.h"
#include "segment.h"
#include "slur.h"
#include "chordrest.h"
#include "tuplet.h"
#include "beam.h"
#include "revisions.h"
#include "page.h"
#include "part.h"
#include "staff.h"
#include "keysig.h"
#include "clef.h"
#include "text.h"
#include "ottava.h"
#include "volta.h"
#include "painter.h"
#include "excerpt.h"
#include "zarchive/zarchive.h"
#include "diff/diff_match_patch.h"
#include "mscore.h"
#include "stafftype.h"
#include "omr/omr.h"
#include "omr/omrpage.h"
#include "al/sig.h"
#include "undo.h"

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(Xml& xml, bool /*autosave*/)
      {
      slurs.clear();
      xml.stag("Score");

      if (_omr && xml.writeOmr)
            _omr->write(xml);
      if (_showOmr && xml.writeOmr)
            xml.tag("showOmr", _showOmr);

      for (int i = 0; i < 32; ++i) {
            if (!_layerTags[i].isEmpty()) {
                  xml.tag(QString("LayerTag id=\"%1\" tag=\"%2\"").arg(i).arg(_layerTags[i]),
                     _layerTagComments[i]);
                  }
            }
      int n = _layer.size();
      for (int i = 1; i < n; ++i) {       // dont save default variant
            const Layer& l = _layer[i];
            xml.tagE(QString("Layer name=\"%1\" mask=\"%2\"").arg(l.name).arg(l.tags));
            }
      xml.tag("currentLayer", _currentLayer);

      _syntiState.write(xml);

      if (pageNumberOffset())
            xml.tag("page-offset", pageNumberOffset());
      xml.tag("Division", AL::division);
      xml.curTrack = -1;

      _style.save(xml, true);      // save only differences to buildin style

      if (!parentScore()) {
            int idx = 0;
            foreach(StaffType* st, _staffTypes) {
                  if ((idx >= STAFF_TYPES) || !st->isEqual(*::staffTypes[idx]))
                        st->write(xml, idx);
                  ++idx;
                  }
            }
      xml.tag("showInvisible", _showInvisible);
      xml.tag("showUnprintable", _showUnprintable);
      xml.tag("showFrames", _showFrames);
      pageFormat()->write(xml);

      QMapIterator<QString, QString> i(_metaTags);
      while (i.hasNext()) {
            i.next();
            if (!i.value().isEmpty())
                  xml.tag(QString("metaTag name=\"%1\"").arg(i.key()), i.value());
            }

      foreach(KeySig* ks, customKeysigs)
            ks->write(xml);
      foreach(const Part* part, _parts)
            part->write(xml);

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Tuplet* tuplet, *m->tuplets())
                  tuplet->setId(xml.tupletId++);
            }
      xml.curTrack = 0;
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1));
            xml.curTick  = 0;
            xml.curTrack = staffIdx * VOICES;
            for (MeasureBase* m = first(); m; m = m->next()) {
                  if (m->type() == MEASURE || staffIdx == 0)
                        m->write(xml, staffIdx, staffIdx == 0);
                  if (m->type() == MEASURE)
                        xml.curTick = m->tick() + m->ticks();
                  }
            xml.etag();
            }
      xml.curTrack = -1;
      xml.tag("cursorTrack", _is.track());
      foreach(Excerpt* excerpt, _excerpts)
            excerpt->score()->write(xml, false);       // recursion
      if (parentScore())
            xml.tag("name", name());
      xml.etag();
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void Score::readStaff(QDomElement e)
      {
      MeasureBase* mb = first();
      int staff       = e.attribute("id", "1").toInt() - 1;
      curTick         = 0;
      curTrack        = staff * VOICES;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());

            if (tag == "Measure") {
                  Measure* measure = 0;
                  if (staff == 0) {
                        measure = new Measure(this);
                        measure->setTick(curTick);
                        add(measure);
                        if (_mscVersion < 115) {
                              const AL::SigEvent& ev = sigmap()->timesig(measure->tick());
                              measure->setLen(ev.timesig());
                              measure->setTimesig(ev.nominal());
                              }
                        else {
                              //
                              // inherit timesig from previous measure
                              //
                              Measure* m = measure->prevMeasure();
                              Fraction f(m ? m->timesig() : Fraction(4,4));
                              measure->setLen(f);
                              measure->setTimesig(f);
                              }
                        }
                  else {
                        while (mb) {
                              if (mb->type() != MEASURE) {
                                    mb = mb->next();
                                    }
                              else {
                                    measure = (Measure*)mb;
                                    mb      = mb->next();
                                    break;
                                    }
                              }
                        if (measure == 0) {
                              printf("Score::readStaff(): missing measure!\n");
                              measure = new Measure(this);
                              measure->setTick(curTick);
                              add(measure);
                              }
                        }
                  measure->read(e, staff);
                  curTick = measure->tick() + measure->ticks();
                  }
            else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                  MeasureBase* mb = static_cast<MeasureBase*>(Element::name2Element(tag, this));
                  mb->read(e);
                  mb->setTick(curTick);
                  add(mb);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   saveFile
///   If file has generated name, create a modal file save dialog
///   and ask filename.
///   Rename old file to backup file (.xxxx.msc?,).
///   Default is to save score in .mscz format,
///   Return true if OK and false on error.
//---------------------------------------------------------

bool Score::saveFile(bool autosave)
      {
      QString suffix = info.suffix();
      if ((suffix != "mscx") && (suffix != "mscz")) {
            QString s = info.filePath();
            if (!suffix.isEmpty())
                  s = s.left(s.size() - suffix.size());
            else
                  s += ".";
            if (suffix == "msc")
                  suffix = "mscx";        // silently change to mscx
            else
                  suffix = "mscz";
            s += suffix;
            info.setFile(s);
            }

      if (info.exists() && !info.isWritable()) {
            QString s = QT_TRANSLATE_NOOP("file", "The following file is locked: \n%1 \n\nTry saving to a different location.");
            MScore::lastError = s.arg(info.filePath());
            return false;
            }

      // if file was already saved in this session
      // save but don't overwrite backup again

      if (saved()) {
            try {
                  if (suffix == "msc" || suffix == "mscx")
                        saveFile(info, autosave);
                  else
                        saveCompressedFile(info, autosave);
                  }
            catch (QString s) {
                  MScore::lastError = s;
                  return false;
                  }
            undo()->setClean();
            setClean(true);
            return true;
            }
      //
      // step 1
      // save into temporary file to prevent partially overwriting
      // the original file in case of "disc full"
      //

      QString tempName = info.filePath() + QString(".temp");
      QFile temp(tempName);
      if (!temp.open(QIODevice::WriteOnly)) {
            MScore::lastError = QT_TRANSLATE_NOOP("file", "Open Temp File\n")
               + tempName + QT_TRANSLATE_NOOP("file", "\nfailed: ") + QString(strerror(errno));
            return false;
            }
      try {
            if (suffix == "msc" || suffix == "mscx")
                  saveFile(&temp, false, autosave);
            else
                  saveCompressedFile(&temp, info, autosave);
            }
      catch (QString s) {
            MScore::lastError = s;
            return false;
            }
      if (temp.error() != QFile::NoError) {
            MScore::lastError = QT_TRANSLATE_NOOP("file",
               "MuseScore: Save File failed: ") + temp.errorString();
            temp.close();
            return false;
            }
      temp.close();

      //
      // step 2
      // remove old backup file if exists
      //
      QDir dir(info.path());
      QString backupName = QString(".") + info.fileName() + QString(",");
      if (dir.exists(backupName)) {
            if (!dir.remove(backupName)) {
//                  QMessageBox::critical(mscore, tr("MuseScore: Save File"),
//                     tr("removing old backup file ") + backupName + tr(" failed"));
                  }
            }

      //
      // step 3
      // rename old file into backup
      //
      QString name(info.filePath());
      if (dir.exists(name)) {
            if (!dir.rename(name, backupName)) {
//                  QMessageBox::critical(mscore, tr("MuseScore: Save File"),
//                     tr("renaming old file <")
//                      + name + tr("> to backup <") + backupName + tr("> failed"));
                  }
            }

      //
      // step 4
      // rename temp name into file name
      //
      if (!QFile::rename(tempName, name)) {
            MScore::lastError = QT_TRANSLATE_NOOP("file", "renaming temp. file <")
               + tempName + QT_TRANSLATE_NOOP("file", "> to <") + name
               + QT_TRANSLATE_NOOP("file", "> failed:\n")
               + QString(strerror(errno));
            return false;
            }
      // make file readable by all
      QFile::setPermissions(name, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser
         | QFile::ReadGroup | QFile::ReadOther);

      undo()->setClean();
      setClean(true);
      setSaved(true);
      return true;
      }

//---------------------------------------------------------
//   saveCompressedFile
//---------------------------------------------------------

void Score::saveCompressedFile(QFileInfo& info, bool autosave)
      {
      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ".mscz");

      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = QString("Open File\n") + info.filePath() + QString("\nfailed: ")
               + QString(strerror(errno));
            throw(s);
            }
      saveCompressedFile(&fp, info, autosave);
      fp.close();
      }

//---------------------------------------------------------
//   saveCompressedFile
//    file is already opened
//---------------------------------------------------------

void Score::saveCompressedFile(QIODevice* f, QFileInfo& info, bool autosave)
      {
      Zip uz;
      if (!uz.createArchive(f))
            throw (QString("Cannot create compressed musescore file: " + uz.errorString()));

      QDateTime dt;
      if (debugMode)
            dt = QDateTime(QDate(2007, 9, 10), QTime(12, 0));
      else
            dt = QDateTime::currentDateTime();

      QString fn = info.completeBaseName() + ".mscx";
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      Xml xml(&cbuf);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(Xml::xmlString(fn)));
      xml.etag();
      int idx = 1;
      foreach(ImagePath* ip, imagePathList) {
            if (!ip->isUsed())
                  continue;
            QString srcPath = ip->path();
            QFileInfo fi(srcPath);
            QString suffix = fi.suffix();
            QString dstPath = QString("Pictures/pic%1.%2").arg(idx).arg(suffix);
            xml.tag("file", dstPath);
            ++idx;
            }


      xml.etag();
      xml.etag();
      cbuf.seek(0);
      if (!uz.createEntry("META-INF/container.xml", cbuf, dt))
            throw(QString("Cannot add container.xml to zipfile '%1': ").arg(info.filePath())
               + uz.errorString());

      // save images
      idx = 1;
      foreach(ImagePath* ip, imagePathList) {
            if (!ip->isUsed())
                  continue;
            QString srcPath = ip->path();
            QFileInfo fi(srcPath);
            QString suffix = fi.suffix();
            QString dstPath = QString("Pictures/pic%1.%2").arg(idx).arg(suffix);
            QBuffer cbuf;
            QByteArray ba;
            if (!ip->loaded()) {
                  QFile inFile(srcPath);
                  if (!inFile.open(QIODevice::ReadOnly))
                        throw(QString("cannot open picture file"));
                  ip->buffer().setData(inFile.readAll());
                  inFile.close();
                  ip->setLoaded(true);
                  }
            cbuf.setBuffer(&(ip->buffer().buffer()));
            if (!cbuf.open(QIODevice::ReadOnly))
                  throw(QString("cannot open buffer cbuf"));
            if (!uz.createEntry(dstPath, cbuf, dt, false, false, 0))
                  throw(QString("Cannot add <%1> to zipfile as <%1>\n").arg(srcPath).arg(dstPath));
            cbuf.close();
            ip->setPath(dstPath);   // image now has local path
            ++idx;
            }
#ifdef OMR
      //
      // save OMR page images
      //
      if (_omr) {
            int n = _omr->numPages();
if (n)
      printf("save OMR-pages %d\n", n);
            for (int i = 0; i < n; ++i) {
                  QString path = QString("OmrPages/page%1.png").arg(i+1);
                  QBuffer cbuf;
                  OmrPage* page = _omr->page(i);
                  QImage image = page->image();
                  if (!image.save(&cbuf, "PNG"))
                        throw(QString("cannot create image"));
                  if (!cbuf.open(QIODevice::ReadOnly))
                        throw(QString("cannot open buffer cbuf"));
                  if (!uz.createEntry(path, cbuf, dt, false, true, 0))
                        throw(QString("Cannot add <%1> to zipfile\n").arg(path));
                  cbuf.close();
                  }
            }
#endif

      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      saveFile(&dbuf, true, autosave);
      dbuf.seek(0);
      if (!uz.createEntry(fn, dbuf, dt))
            throw(QString("Cannot add %1 to zipfile '%2'").arg(fn).arg(info.filePath()));
      if (!uz.closeArchive())
            throw(QString("Cannot close zipfile '%1'").arg(info.filePath()));
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

void Score::saveFile(QFileInfo& info, bool autosave)
      {
      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ".mscx");
      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = QString("Open File\n") + info.filePath() + QString("\nfailed: ")
               + QString(strerror(errno));
            throw(s);
            }
      saveFile(&fp, false, autosave);
      fp.close();
      }

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

bool Score::loadStyle(const QString& fn)
      {
      QFile f(fn);
      if (f.open(QIODevice::ReadOnly)) {
            Style st(*MScore::defaultStyle());
            if (st.load(&f)) {
                  _undo->push(new ChangeStyle(this, st));
                  return true;
                  }
            }
      MScore::lastError = strerror(errno);
      return false;
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

bool Score::saveStyle(const QString& name)
      {
      QString ext(".mss");
      QFileInfo info(name);

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile f(info.filePath());
      if (!f.open(QIODevice::WriteOnly)) {
            MScore::lastError = QT_TRANSLATE_NOOP("file", "Open Style File\n")
               + f.fileName() + QT_TRANSLATE_NOOP("file", "\nfailed: ")
               + QString(strerror(errno));
            return false;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      _style.save(xml, false);     // save complete style
      xml.etag();
      if (f.error() != QFile::NoError) {
            MScore::lastError = QT_TRANSLATE_NOOP("file", "Write Style failed: ")
               + f.errorString();
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

extern QString revision;

void Score::saveFile(QIODevice* f, bool msczFormat, bool autosave)
      {
      Xml xml(f);
      xml.writeOmr = msczFormat;
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.tag("programVersion", VERSION);
      xml.tag("programRevision", revision);
      write(xml, autosave);
      xml.etag();
      if (!parentScore())
            _revisions->write(xml);
      }

//---------------------------------------------------------
//   loadCompressedMsc
//    return false on error
//---------------------------------------------------------

bool Score::loadCompressedMsc(QString name)
      {
      QString ext(".mscz");

      info.setFile(name);
      if (info.suffix().isEmpty()) {
            name += ext;
            info.setFile(name);
            }

      Unzip uz;
      if (!uz.openArchive(name))
            return false;

      QBuffer cbuf;
      cbuf.open(QIODevice::WriteOnly);
      uz.extractFile("META-INF/container.xml", &cbuf);

      QDomDocument container;
      int line, column;
      QString err;
      if (!container.setContent(cbuf.data(), false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            printf("error: %s\n", qPrintable(error));
            return false;
            }

      // extract first rootfile
      QString rootfile = "";
      for (QDomElement e = container.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() != "container") {
                  domError(e);
                  continue;
                  }
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() != "rootfiles") {
                        domError(ee);
                        continue;
                        }
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        QString tag(eee.tagName());
                        QString val(eee.text());

                        if (tag == "rootfile") {
                              if (rootfile.isEmpty())
                                    rootfile = eee.attribute(QString("full-path"));
                              }
                        else if (tag == "file") {
                              ImagePath* ip = new ImagePath(val);
                              imagePathList.append(ip);
                              }
                        else
                              domError(eee);
                        }
                  }
            }
      //
      // load images
      //
      foreach(ImagePath* ip, imagePathList) {
            QBuffer& dbuf = ip->buffer();
            dbuf.open(QIODevice::WriteOnly);
            if (!uz.extractFile(ip->path(), &dbuf))
                  printf("Cannot read <%s> from zipfile\n", qPrintable(ip->path()));
            else
                  ip->setLoaded(true);
            }
      if (rootfile.isEmpty()) {
            printf("can't find rootfile in: %s\n", qPrintable(name));
            return false;
            }

      QBuffer dbuf;
      dbuf.open(QIODevice::WriteOnly);
      uz.extractFile(rootfile, &dbuf);

      QDomDocument doc;
      if (!doc.setContent(dbuf.data(), false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            printf("error: %s\n", qPrintable(error));
            return false;
            }
      dbuf.close();
      docName = info.completeBaseName();
      bool retval = read1(doc.documentElement());

#ifdef OMR
      //
      // load OMR page images
      //
      if (_omr) {
            int n = _omr->numPages();
            for (int i = 0; i < n; ++i) {
                  QBuffer dbuf;
                  dbuf.open(QIODevice::WriteOnly);
                  QString path = QString("OmrPages/page%1.png").arg(i+1);
                  if (!uz.extractFile(path, &dbuf))
                        printf("Cannot read <%s> from zipfile\n", qPrintable(path));
                  else  {
                        OmrPage* page = _omr->page(i);
                        QImage image;
                        if (image.loadFromData(dbuf.data(), "PNG")) {
                              page->setImage(image);
                              // printf("read image %s\n", qPrintable(path));
                              }
                        else
                              printf("load image failed\n");
                        }
                  }
            }
#endif
      return retval;
      }

//---------------------------------------------------------
//   loadMsc
//    return true on success
//---------------------------------------------------------

bool Score::loadMsc(QString name)
      {
      QString ext(".mscx");

      info.setFile(name);
      if (info.suffix().isEmpty()) {
            name += ext;
            info.setFile(name);
            }
      QFile f(name);
      if (!f.open(QIODevice::ReadOnly)) {
            MScore::lastError = f.errorString();
            return false;
            }

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString s = QT_TRANSLATE_NOOP("file", "error reading file %1 at line %2 column %3: %4\n");
            MScore::lastError = s.arg(f.fileName()).arg(line).arg(column).arg(err);
            return false;
            }
      f.close();
      docName = f.fileName();
      return read1(doc.documentElement());
      }

//---------------------------------------------------------
//   parseVersion
//---------------------------------------------------------

void Score::parseVersion(const QString& val)
      {
      QRegExp re("(\\d+)\\.(\\d+)\\.(\\d+)");
      int v1, v2, v3, rv1, rv2, rv3;
      if (re.indexIn(VERSION) != -1) {
            QStringList sl = re.capturedTexts();
            if (sl.size() == 4) {
                  v1 = sl[1].toInt();
                  v2 = sl[2].toInt();
                  v3 = sl[3].toInt();
                  if (re.indexIn(val) != -1) {
                        sl = re.capturedTexts();
                        if (sl.size() == 4) {
                              rv1 = sl[1].toInt();
                              rv2 = sl[2].toInt();
                              rv3 = sl[3].toInt();

                              int currentVersion = v1 * 10000 + v2 * 100 + v3;
                              int readVersion = rv1 * 10000 + rv2 * 100 + rv3;
                              if (readVersion > currentVersion) {
                                    printf("read future version\n");
                                    }
                              }
                        }
                  else {
                        QRegExp re1("(\\d+)\\.(\\d+)");
                        if (re1.indexIn(val) != -1) {
                              sl = re.capturedTexts();
                              if (sl.size() == 3) {
                                    rv1 = sl[1].toInt();
                                    rv2 = sl[2].toInt();

                                    int currentVersion = v1 * 10000 + v2 * 100 + v3;
                                    int readVersion = rv1 * 10000 + rv2 * 100;
                                    if (readVersion > currentVersion) {
                                          printf("read future version\n");
                                          }
                                    }
                              }
                        else
                              printf("1cannot parse <%s>\n", qPrintable(val));
                        }
                  }
            }
      else
            printf("2cannot parse <%s>\n", VERSION);
      }

//---------------------------------------------------------
//   read1
//    return true on success
//---------------------------------------------------------

bool Score::read1(QDomElement e)
      {
      _elinks.clear();
      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  QString version = e.attribute("version");
                  QStringList sl = version.split('.');
                  _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
                  if (_mscVersion > MSCVERSION) {
                        // incompatible version
                        MScore::lastError =
                           QT_TRANSLATE_NOOP("score", "Cannot read this score:\n"
                           "your version of MuseScore is too old.");
                        return false;
                        }
                  if (_mscVersion < 117) {
                        bool rv = read(e);
                        return rv;
                        }
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "programVersion")
                              parseVersion(val);
                        else if (tag == "programRevision")
                              ;
                        else if (tag == "Score") {
                              read(ee);
                              }
                        else if (tag == "Revision") {
                              Revision* revision = new Revision;
                              revision->read(e);
                              _revisions->add(revision);
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
      int id = 1;
      foreach(LinkedElements* le, _elinks)
            le->setLid(id++);
      _elinks.clear();
      _mscVersion = MSCVERSION;     // for later drag & drop usage
      return true;
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool Score::read(QDomElement dScore)
      {
      _fileDivision = 384;   // for compatibility with old mscore files
      slurs.clear();

      if (parentScore())
            setMscVersion(parentScore()->mscVersion());
      dScore = dScore.firstChildElement();
      for (QDomElement ee = dScore; !ee.isNull(); ee = ee.nextSiblingElement()) {
            curTrack = -1;
            QString tag(ee.tagName());
            QString val(ee.text());
            int i = val.toInt();
            if (tag == "Staff")
                  readStaff(ee);
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(this);
                  ks->read(ee);
                  customKeysigs.append(ks);
                  }
            else if (tag == "StaffType") {
                  int idx        = ee.attribute("idx").toInt();
                  StaffType* ost = _staffTypes.value(idx);
                  StaffType* st;
                  if (ost)
                        st = ost;
                  else {
                        QString group  = ee.attribute("group", "pitched");
                        if (group == "percussion")
                              st  = new StaffTypePercussion();
                        else if (group == "tablature")
                              st  = new StaffTypeTablature();
                        else
                              st  = new StaffTypePitched();
                        }
                  st->read(ee);
                  if (idx < _staffTypes.size())
                        _staffTypes[idx] = st;
                  else
                        _staffTypes.append(st);
                  }
            else if (tag == "siglist")
                  _sigmap->read(ee, _fileDivision);
            else if (tag == "tempolist")        // obsolete
                  ;           // tempomap()->read(ee, _fileDivision);
            else if (tag == "programVersion")
                  parseVersion(val);
            else if (tag == "programRevision")
                  ;
            else if (tag == "Mag" || tag == "MagIdx" || tag == "xoff" || tag == "yoff") {
                  // obsolete
                  ;
                  }
#ifdef OMR
            else if (tag == "Omr") {
                  _omr = new Omr(this);
                  _omr->read(ee);
/*                  if (!_omr->read()) {
                        delete _omr;
                        _omr = 0;
                        }
                  */

                  }
#endif
            else if (tag == "showOmr")
                  _showOmr = i;
            else if (tag == "LayerTag") {
                  int id = ee.attribute("id").toInt();
                  QString tag = ee.attribute("tag");
                  if (id >= 0 && id < 32) {
                        _layerTags[id] = tag;
                        _layerTagComments[id] = val;
                        }
                  }
            else if (tag == "Layer") {
                  Layer layer;
                  layer.name = ee.attribute("name");
                  layer.tags = ee.attribute("mask").toUInt();
                  _layer.append(layer);
                  }
            else if (tag == "currentLayer")
                  _currentLayer = val.toInt();
            else if (tag == "SyntiSettings") {
                  _syntiState.clear();
                  _syntiState.read(ee);
                  //
                  // check for soundfont,
                  // add default soundfont if none found
                  // (for compatibility with old scores)
                  //
                  bool hasSoundfont = false;
                  foreach(const SyntiParameter& sp, _syntiState) {
                        if (sp.name() == "soundfont") {
                              QFileInfo fi(sp.sval());
                              if(fi.exists())
                                    hasSoundfont = true;
                              }
                        }
                  if (!hasSoundfont)
                        _syntiState.append(SyntiParameter("soundfont", MScore::soundFont));
                  }
            else if (tag == "Spatium")
                  _style.setSpatium (val.toDouble() * DPMM); // obsolete, moved to Style
            else if (tag == "page-offset")            // obsolete, moved to Score
                  setPageNumberOffset(i);
            else if (tag == "Division")
                  _fileDivision = i;
            else if (tag == "showInvisible")
                  _showInvisible = i;
            else if (tag == "showUnprintable")
                  _showUnprintable = i;
            else if (tag == "showFrames")
                  _showFrames = i;
            else if (tag == "Style")
                  _style.load(ee);
            else if (tag == "TextStyle") {      // obsolete: is now part of style
                  TextStyle s;
                  s.read(ee);
                  // settings for _reloff::x and _reloff::y in old formats
                  // is now included in style; setting them to 0 fixes most
                  // cases of backward compatibility
                  s.setRxoff(0);
                  s.setRyoff(0);
                  _style.setTextStyle(s);
                  }
            else if (tag == "page-layout")
                  pageFormat()->read(ee, this);
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read(ee);
                  setMetaTag("copyright", text->getText());
                  delete text;
                  }
            else if (tag == "movement-number")
                  setMetaTag("movementNumber", val);
            else if (tag == "movement-title")
                  setMetaTag("movementTitle", val);
            else if (tag == "work-number")
                  setMetaTag("workNumber", val);
            else if (tag == "work-title")
                  setMetaTag("workTitle", val);
            else if (tag == "source")
                  setMetaTag("source", val);
            else if (tag == "metaTag") {
                  QString name = ee.attribute("name");
                  setMetaTag(name, val);
                  }
            else if (tag == "Part") {
                  Part* part = new Part(this);
                  part->read(ee);
                  _parts.push_back(part);
                  }
            else if (tag == "Symbols")    // obsolete
                  ;
            else if (tag == "cursorTrack") {
                  if (i >= 0)
                        setInputTrack(i);
                  }
            else if (tag == "Slur") {
                  Slur* slur = new Slur(this);
                  slur->read(ee);
                  slurs.append(slur);
                  }
            else if ((_mscVersion < 116) &&     // skip and process in II. pass
               ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Pedal"))) {
                  ;
                  }
            else if (tag == "Excerpt") {
                  Excerpt* e = new Excerpt(this);
                  e->read(ee);
                  _excerpts.append(e);
                  }
            else if (tag == "Beam") {
                  Beam* beam = new Beam(this);
                  beam->read(ee);
                  beam->setParent(0);
                  // _beams.append(beam);
                  }
            else if (tag == "Score") {          // recursion
                  Score* s = new Score(style());
                  s->setParentScore(this);
                  s->read(ee);
                  addExcerpt(s);
                  s->setExcerptsChanged(true);
                  }
            else if (tag == "name")
                  setName(val);
            else
                  domError(ee);
            }

      if (_mscVersion < 108)
            connectSlurs();

      if (_mscVersion < 121) {            // 115
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  Staff* s = _staves[staffIdx];
                  int track = staffIdx * VOICES;

                  ClefList* cl = s->clefList();
                  for (ciClefEvent i = cl->constBegin(); i != cl->constEnd(); ++i) {
                        int tick = i.key();
                        ClefType clefId = i.value()._concertClef;
                        Measure* m = tick2measure(tick);
                        if ((tick == m->tick()) && m->prevMeasure())
                              m = m->prevMeasure();
                        Segment* seg = m->getSegment(SegClef, tick);
                        if (seg->element(track))
                              static_cast<Clef*>(seg->element(track))->setGenerated(false);
                        else {
                              Clef* clef = new Clef(this);
                              clef->setClefType(clefId);
                              clef->setTrack(track);
                              clef->setParent(seg);
                              clef->setGenerated(false);
                              seg->add(clef);
                              }
                        }

                  KeyList* km = s->keymap();
                  for (ciKeyList i = km->begin(); i != km->end(); ++i) {
                        int tick = i->first;
                        KeySigEvent ke = i->second;
                        Measure* m = tick2measure(tick);
                        Segment* seg = m->getSegment(SegKeySig, tick);
                        if (seg->element(track))
                              static_cast<KeySig*>(seg->element(track))->setGenerated(false);
                        else {
                              KeySig* ks = keySigFactory(ke);
                              ks->setParent(seg);
                              ks->setTrack(track);
                              ks->setGenerated(false);
                              seg->add(ks);
                              }
                        }
                  }
            // TODO: free cleflist
            }
      if (_mscVersion < 116) {
            //
            // scan spanner in a II. pass
            //
            for (QDomElement ee = dScore; !ee.isNull(); ee = ee.nextSiblingElement()) {
                  QString tag(ee.tagName());
                  QString val(ee.text());
                  if (   (tag == "HairPin")
                      || (tag == "Ottava")
                      || (tag == "TextLine")
                      || (tag == "Volta")
                      || (tag == "Trill")
                      || (tag == "Pedal")) {
                        Spanner* s = static_cast<Spanner*>(Element::name2Element(tag, this));
                        s->setTrack(0);
                        s->read(ee);
                        int tick2 = s->__tick2();
                        Segment* s1 = tick2segment(curTick);
                        Segment* s2 = tick2segment(tick2);
                        if (s1 == 0 || s2 == 0) {
                              printf("cannot place %s at tick %d - %d\n",
                                 s->name(), s->__tick1(), tick2);
                              }
                        else {
                              s->setStartElement(s1);
                              Measure* m = s2->measure();
                              if (s->anchor() == ANCHOR_MEASURE && tick2 == m->tick()) {
                                    // anchor to EndBarLine segment of previous measure:
                                    m  = m->prevMeasure();
                                    s2 = m->getSegment(SegEndBarLine, tick2);
                                    }
                              s->setEndElement(s2);
                              s1->add(s);
                              }
                        if (s->type() == VOLTA) {
                              // fix volta position
                              Volta* volta = static_cast<Volta*>(s);
                              int n = volta->spannerSegments().size();
                              for (int i = 0; i < n; ++i) {
                                    LineSegment* seg = volta->segmentAt(i);
                                    if (!seg->userOff().isNull())
                                          seg->setUserYoffset(seg->userOff().y() + 0.5 * spatium());
                                    }
                              }
                        else if (s->type() == OTTAVA) {
                              // fix ottava position
                              Ottava* volta = static_cast<Ottava*>(s);
                              int n = volta->spannerSegments().size();
                              for (int i = 0; i < n; ++i) {
                                    LineSegment* seg = volta->segmentAt(i);
                                    if (!seg->userOff().isNull())
                                          seg->setUserYoffset(seg->userOff().y() - styleP(ST_ottavaY));
                                    }
                              }
                        }
                  }
            }

      // check slurs
      foreach(Slur* slur, slurs) {
            if (!slur->startElement() || !slur->endElement()) {
                  printf("incomplete Slur\n");
                  if (slur->startElement()) {
                        printf("  front %d\n", static_cast<ChordRest*>(slur->startElement())->tick());
                        static_cast<ChordRest*>(slur->startElement())->removeSlurFor(slur);
                        }
                  if (slur->endElement()) {
                        printf("  back %d\n", static_cast<ChordRest*>(slur->endElement())->tick());
                        static_cast<ChordRest*>(slur->endElement())->removeSlurBack(slur);
                        }
                  }
            else {
                  ChordRest* cr1 = (ChordRest*)(slur->startElement());
                  ChordRest* cr2 = (ChordRest*)(slur->endElement());
                  if (cr1->tick() > cr2->tick()) {
                        printf("Slur invalid start-end tick %d-%d\n", cr1->tick(), cr2->tick());
                        slur->setStartElement(cr2);
                        slur->setEndElement(cr1);
                        }
                  int n1 = 0;
                  int n2 = 0;
                  foreach(Slur* s, cr1->slurFor()) {
                        if (s == slur)
                              ++n1;
                        }
                  foreach(Slur* s, cr2->slurBack()) {
                        if (s == slur)
                              ++n2;
                        }
                  if (n1 != 1 || n2 != 1) {
                        printf("Slur references bad: %d %d\n", n1, n2);
                        }
                  }
            }
      slurs.clear();
      connectTies();

      searchSelectedElements();

      _fileDivision = AL::division;

      //
      //    sanity check for barLineSpan
      //
      foreach(Staff* staff, _staves) {
            int barLineSpan = staff->barLineSpan();
            int idx = staffIdx(staff);
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  printf("bad span: idx %d  span %d staves %d\n", idx, barLineSpan, n);
                  staff->setBarLineSpan(n - idx);
                  }
            }

      if (_omr == 0)
            _showOmr = false;

      if (_mscVersion < 103) {
            foreach(Staff* staff, _staves) {
                  Part* part = staff->part();
                  if (part->staves()->size() == 1)
                        staff->setBarLineSpan(1);
                  else {
                        if (staff == part->staves()->front())
                              staff->setBarLineSpan(part->staves()->size());
                        else
                              staff->setBarLineSpan(0);
                        }
                  }
            }

      if (_mscVersion < 117) {
            // create excerpts
            foreach(Excerpt* excerpt, _excerpts) {
                  Score* nscore = ::createExcerpt(*excerpt->parts());
                  if (nscore) {
                        nscore->setParentScore(this);
                        nscore->setName(excerpt->title());
                        nscore->rebuildMidiMapping();
                        nscore->updateChannel();
                        nscore->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
                        nscore->doLayout();
                        excerpt->setScore(nscore);
                        }
                  }
            }
      renumberMeasures();
      rebuildMidiMapping();
      updateChannel();
      return true;
      }

//---------------------------------------------------------
//   connectSlurs
//    helper routine for old msc versions
//    and MusicXml and Capella import
//---------------------------------------------------------

void Score::connectSlurs()
      {
#if 0
      foreach (Slur* s, slurs) {
            Element* n1 = searchNote(s->tick(), s->track());
            Element* n2 = searchNote(s->tick2(), s->track2());
            if (n1 == 0 || n2 == 0) {
                  printf("connectSlurs: position not found\n");
                  // remove in checkSlurs
                  }
            else {
                  if (n1->isChordRest()) {
                        ((ChordRest*)n1)->addSlurFor(s);
                        s->setStartElement(n1);
                        }
                  else
                        printf("   n1 is %s\n", n1->name());
                  if (n2->isChordRest()) {
                        ((ChordRest*)n2)->addSlurBack(s);
                        s->setEndElement(n2);
                        }
                  else
                        printf("connectSlurs: n2 is %s\n", n2->name());
                  }
            }
#endif
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void Score::print(Painter* painter, int pageNo)
      {
      _printing  = true;
      int offset = pageNumberOffset();
      Page* page = pages().at(pageNo);
      QRectF fr  = page->abbox();

      QList<const Element*> ell = page->items(fr);
      qStableSort(ell.begin(), ell.end(), elementLessThan);
      foreach(const Element* e, ell) {
            e->itemDiscovered = 0;
            if (!e->visible())
                  continue;
            painter->save();
            painter->translate(e->canvasPos() - page->pos());
            painter->setPenColor(e->color());
            e->draw(painter);
            painter->restore();
            }
      _printing = false;
      }

//---------------------------------------------------------
//   saveSvg
//---------------------------------------------------------

bool Score::saveSvg(const QString& saveName)
      {
#if 0 // TODO-LIB
      QSvgGenerator printer;
      printer.setResolution(int(DPI));
      printer.setFileName(saveName);

      _printing = true;

      QPainter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      double mag = converterDpi / DPI;
      p.scale(mag, mag);
      PainterQt painter(&p, 0);

      QList<Element*> eel;
      for (MeasureBase* m = _measures.first(); m; m = m->next()) {
            // skip multi measure rests
            if (m->type() == MEASURE) {
                  Measure* mm = static_cast<Measure*>(m);
                  if (mm->multiMeasure() < 0)
                        continue;
                  }
            m->scanElements(&eel, collectElements);
            }
      QList<const Element*> el;
      foreach(Page* page, pages()) {
            el.clear();
            page->scanElements(&el, collectElements);
            foreach(const Element* e, eel) {
                  if (!e->visible())
                        continue;
                  p.save();
                  p.translate(e->canvasPos() - page->pos());
                  p.setPen(QPen(e->color()));
                  e->draw(&painter);
                  p.restore();
                  }
            foreach(const Element* e, el) {
                  if (!e->visible())
                        continue;
                  p.save();
                  p.translate(e->canvasPos() - page->pos());
                  p.setPen(QPen(e->color()));
                  e->draw(&painter);
                  p.restore();
                  }
            }

      _printing = false;
      p.end();
#endif
      return true;
      }

//---------------------------------------------------------
//   savePng
//    return true on success
//---------------------------------------------------------

bool Score::savePng(const QString& name)
      {
//TODO-LIB      return savePng(name, false, true, converterDpi, QImage::Format_ARGB32_Premultiplied );
      return false;
      }

//---------------------------------------------------------
//   savePng with options
//    return true on success
//---------------------------------------------------------

bool Score::savePng(const QString& name, bool screenshot, bool transparent, double convDpi, QImage::Format format)
      {
      bool rv = true;
#if 0
      _printing = !screenshot;             // dont print page break symbols etc.


      QImage::Format f;
      if (format != QImage::Format_Indexed8)
          f = format;
      else
          f = QImage::Format_ARGB32_Premultiplied;

      const QList<Page*>& pl = pages();
      int pages = pl.size();

      QList<Element*> eel;
//      foreach (Beam* b, _beams)
//            b->scanElements(&eel, collectElements);
      for (MeasureBase* m = _measures.first(); m; m = m->next()) {
            // skip multi measure rests
            if (m->type() == MEASURE) {
                  Measure* mm = static_cast<Measure*>(m);
                  if (mm->multiMeasure() < 0)
                        continue;
                  }
            m->scanElements(&eel, collectElements);
            }
      int padding = QString("%1").arg(pages).size();
      for (int pageNumber = 0; pageNumber < pages; ++pageNumber) {
            Page* page = pl.at(pageNumber);

            QRectF r = page->abbox();
            int w = lrint(r.width()  * convDpi / DPI);
            int h = lrint(r.height() * convDpi / DPI);

            QImage printer(w, h, f);

            printer.setDotsPerMeterX(lrint(DPMM * 1000.0));
            printer.setDotsPerMeterY(lrint(DPMM * 1000.0));

            printer.fill(transparent ? 0 : 0xffffffff);

            double mag = convDpi / DPI;
            QPainter p(&printer);
            PainterQt painter(&p, 0);

            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::TextAntialiasing, true);
            p.scale(mag, mag);

            foreach(const Element* e, eel) {
                  if (!e->visible())
                        continue;
                  QPointF ap(e->canvasPos() - page->pos());
                  p.translate(ap);
                  p.setPen(QPen(e->color()));
                  e->draw(&painter);
                  p.translate(-ap);
                  }

            QList<Element*> el;
            page->scanElements(&el, collectElements);
            foreach(const Element* e, el) {
                  if (!e->visible())
                        continue;
                  QPointF ap(e->canvasPos() - page->pos());
                  p.translate(ap);
                  p.setPen(QPen(e->color()));
                  e->draw(&painter);
                  p.translate(-ap);
                  }

            if (format == QImage::Format_Indexed8) {
                  //convert to grayscale & respect alpha
                  QVector<QRgb> colorTable;
                  colorTable.push_back(QColor(0, 0, 0, 0).rgba());
                  if (!transparent) {
                        for (int i = 1; i < 256; i++)
                              colorTable.push_back(QColor(i, i, i).rgb());
                        }
                  else {
                        for (int i = 1; i < 256; i++)
                              colorTable.push_back(QColor(0, 0, 0, i).rgba());
                        }
                  printer = printer.convertToFormat(QImage::Format_Indexed8, colorTable);
                  }

            QString fileName(name);
            if (fileName.endsWith(".png"))
                  fileName = fileName.left(fileName.size() - 4);
            fileName += QString("-%1.png").arg(pageNumber+1, padding, 10, QLatin1Char('0'));

            rv = printer.save(fileName, "png");
            if (!rv)
                  break;
            }
#endif
      _printing = false;
      return rv;
      }

//---------------------------------------------------------
//   readCompressedToBuffer
//---------------------------------------------------------

QByteArray Score::readCompressedToBuffer()
      {
      QBuffer cbuf;
      Unzip uz;
      if (!uz.openArchive(filePath()))
            return QByteArray();

      cbuf.open(QIODevice::WriteOnly);
      uz.extractFile("META-INF/container.xml", &cbuf);

      QDomDocument container;
      int line, column;
      QString err;
      if (!container.setContent(cbuf.data(), false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            printf("error: %s\n", qPrintable(error));
            return QByteArray();
            }

      // extract first rootfile
      QString rootfile = "";
      for (QDomElement e = container.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() != "container") {
                  domError(e);
                  continue;
                  }
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() != "rootfiles") {
                        domError(ee);
                        continue;
                        }
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        QString tag(eee.tagName());
                        QString val(eee.text());

                        if (tag == "rootfile") {
                              if (rootfile.isEmpty())
                                    rootfile = eee.attribute(QString("full-path"));
                              }
                        else if (tag == "file") {
                              ImagePath* ip = new ImagePath(val);
                              imagePathList.append(ip);
                              }
                        else
                              domError(eee);
                        }
                  }
            }
      //
      // load images
      //
      foreach(ImagePath* ip, imagePathList) {
            QBuffer& dbuf = ip->buffer();
            dbuf.open(QIODevice::WriteOnly);
            if (!uz.extractFile(ip->path(), &dbuf))
                  printf("Cannot read <%s> from zipfile\n", qPrintable(ip->path()));
            else
                  ip->setLoaded(true);
            }

      if (rootfile.isEmpty()) {
            printf("can't find rootfile in: %s\n", qPrintable(filePath()));
            return QByteArray();
            }

      QBuffer dbuf;
      dbuf.open(QIODevice::WriteOnly);
      uz.extractFile(rootfile, &dbuf);
      dbuf.close();
      return dbuf.data();
      }

//---------------------------------------------------------
//   readToBuffer
//---------------------------------------------------------

QByteArray Score::readToBuffer()
      {
      QByteArray ba;
      QString cs  = info.suffix();

      if (cs == "mscz") {
            ba = readCompressedToBuffer();
            }
      if (cs.toLower() == "msc" || cs.toLower() == "mscx") {
            QFile f(filePath());
            if (f.open(QIODevice::ReadOnly)) {
                  ba = f.readAll();
                  f.close();
                  }
            }
      return ba;
      }

//---------------------------------------------------------
//   createRevision
//---------------------------------------------------------

void Score::createRevision()
      {
#if 0
printf("createRevision\n");
      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      saveFile(&dbuf, false, false);
      dbuf.close();

      QByteArray ba1 = readToBuffer();

      QString os = QString::fromUtf8(ba1.data(), ba1.size());
      QString ns = QString::fromUtf8(dbuf.buffer().data(), dbuf.buffer().size());

      diff_match_patch dmp;
      Revision* r = new Revision();
      r->setDiff(dmp.patch_toText(dmp.patch_make(ns, os)));
      r->setId("1");
      _revisions->add(r);

//      printf("patch:\n%s\n==========\n", qPrintable(patch));
      //
#endif
      }

//---------------------------------------------------------
//   writeSegments
//---------------------------------------------------------

void Score::writeSegments(Xml& xml, const Measure* m, int strack, int etrack, Segment* fs, Segment* ls,
   bool writeSystemElements)
      {
      for (int track = strack; track < etrack; ++track) {
            for (Segment* segment = fs; segment && segment != ls; segment = segment->next1()) {
                  Element* e = segment->element(track);
                  //
                  // special case: - barline span > 1
                  //               - part (excerpt) staff starts after
                  //                 barline element
                  bool needTick = segment->tick() != xml.curTick;
                  if ((segment->subtype() == SegEndBarLine)
                     && (e == 0)
                     && writeSystemElements
                     && ((track % VOICES) == 0)) {
                        // search barline:
                        for (int idx = track - VOICES; idx >= 0; idx -= VOICES) {
                              if (segment->element(idx)) {
                                    int oDiff = xml.trackDiff;
                                    xml.trackDiff = idx;          // staffIdx should be zero
                                    segment->element(idx)->write(xml);
                                    xml.trackDiff = oDiff;
                                    break;
                                    }
                              }
                        }
                  foreach(Element* e, segment->annotations()) {
                        if (e->track() != track || e->generated())
                              continue;
                        if (needTick) {
                              xml.tag("tick", segment->tick());
                              xml.curTick = segment->tick();
                              needTick = false;
                              }
                        e->write(xml);
                        }
                  foreach(Spanner* e, segment->spannerFor()) {
                        if (e->track() == track && !e->generated()) {
                              if (needTick) {
                                    xml.tag("tick", segment->tick());
                                    xml.curTick = segment->tick();
                                    needTick = false;
                                    }
                              e->setId(++xml.spannerId);
                              e->write(xml);
                              }
                        }
                  foreach(Spanner* e, segment->spannerBack()) {
                        if (e->track() == track && !e->generated()) {
                              if (needTick) {
                                    xml.tag("tick", segment->tick());
                                    xml.curTick = segment->tick();
                                    needTick = false;
                                    }
                              xml.tagE(QString("endSpanner id=\"%1\"").arg(e->id()));
                              }
                        }
                  if (e && !e->generated()) {
                        if (needTick) {
                              xml.tag("tick", segment->tick());
                              xml.curTick = segment->tick();
                              needTick = false;
                              }
                        if (e->isChordRest()) {
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              Beam* beam = cr->beam();
                              if (beam && !beam->generated() && beam->elements().front() == cr) {
                                    beam->setId(xml.beamId++);
                                    beam->write(xml);
                                    }
                              Tuplet* tuplet = cr->tuplet();
                              if (tuplet && tuplet->elements().front() == cr) {
                                    tuplet->setId(xml.tupletId++);
                                    tuplet->write(xml);
                                    }
                              foreach(Slur* slur, cr->slurFor()) {
                                    bool found = false;
                                    foreach(Slur* slur1, slurs) {
                                          if (slur1 == slur) {
                                                found = true;
                                                break;
                                                }
                                          }
                                    if (!found) {
                                          slur->setId(xml.slurId++);
                                          slurs.append(slur);
                                          slur->write(xml);
                                          }
                                    }
                              foreach(Slur* slur, cr->slurBack()) {
                                    bool found = false;
                                    foreach(Slur* slur1, slurs) {
                                          if (slur1 == slur) {
                                                found = true;
                                                break;
                                                }
                                          }
                                    if (!found) {
                                          slur->setId(xml.slurId++);
                                          slurs.append(slur);
                                          slur->write(xml);
                                          }
                                    }
                              }
                        if ((segment->subtype() == SegEndBarLine) && m && (m->multiMeasure() > 0)) {
                              xml.stag("BarLine");
                              xml.tag("subtype", m->endBarLineType());
                              xml.tag("visible", m->endBarLineVisible());
                              xml.etag();
                              }
                        else
                              e->write(xml);
                        }
                  }
            }
      }

