//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: file.cpp,v 1.70 2006/04/12 14:58:10 wschweer Exp $
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

/**
 File handling: loading and saving.
 */

#include "globals.h"
#include "config.h"
#include "mscore.h"
#include "canvas.h"
#include "xml.h"
#include "element.h"
#include "note.h"
#include "rest.h"
#include "sig.h"
#include "clef.h"
#include "key.h"
#include "instrdialog.h"
#include "score.h"
#include "page.h"
#include "dynamics.h"
#include "file.h"
#include "style.h"
#include "tempo.h"
#include "select.h"
#include "padstate.h"
#include "preferences.h"
#include "input.h"
#include "playpanel.h"
#include "staff.h"
#include "part.h"
#include "utils.h"
#include "layout.h"
#include "barline.h"
#include "palette.h"
#include "symboldialog.h"
#include "slur.h"
#include "hairpin.h"
#include "ottava.h"
#include "textline.h"
#include "pedal.h"
#include "trill.h"
#include "volta.h"
#include "newwizard.h"
#include "timesig.h"
#include "box.h"
#include "excerpt.h"
#include "system.h"
#include "tuplet.h"
#include "keysig.h"
#include "zip.h"
#include "unzip.h"

//---------------------------------------------------------
//   load
//    return true on error
//---------------------------------------------------------

/**
 Create a modal file open dialog with caption \a caption,
 working directory \a base
 and filter \a ext.
 If a file is selected, load it.
 Return false if OK and true on error.
 */

bool LoadFile::load(QWidget* parent, const QString& base, const QString& ext,
   const QString& caption)
      {
      error = "";
      _name = QFileDialog::getOpenFileName(parent, caption, base, ext);

      if (_name.isEmpty())
            return true;
      QFileInfo info(_name);

      if (info.suffix().isEmpty()) {
            _name += ext;
            info.setFile(_name);
            }
      return load(_name);
      }

/**
 Load file \a name.
 Display message box with error if loading fails.
 Return false if OK and true on error.
 */

bool LoadFile::load(const QString& name)
      {
      if (name.isEmpty())
            return true;

      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly))
            return true;
      if (loader(&fp)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: load failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            printf("load failed: %s\n", qPrintable(error));
            }
      fp.close();
      return false;
      }

//---------------------------------------------------------
//   checkDirty
//    if dirty, save score
//    return true on cancel
//---------------------------------------------------------

bool MuseScore::checkDirty(Score* s)
      {
      if (s->dirty()) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("The score \"%1\"contains unsaved data\n"
               "Save Current Score?").arg(s->name()),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save) {
                  if (s->isSavable()) {
                        if (!s->saveFile(false))
                              return true;
                        }
                  else {
                        if (!s->saveAs(false))
                              return true;
                        }

                  }
            else if (n == QMessageBox::Cancel)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   clearScore
//---------------------------------------------------------

void MuseScore::clearScore()
      {
      setDefaultStyle();
      cs->clear();
      cs->sel->clear();
      canvas->clearScore();
      }

//---------------------------------------------------------
//   loadFile
//---------------------------------------------------------

/**
 Create a modal file open dialog.
 If a file is selected, load it.
 Handles the GUI's file-open action.
 */

void MuseScore::loadFile()
      {
      QString fn = QFileDialog::getOpenFileName(
         this,
         tr("MuseScore: Load Score"),
         lastOpenPath,
         tr("All Supported Files (*.mscz *.msc *.xml *.mxl *.mid *.kar *.md *.ly *.mgu *.MGU *.sgu *.SGU);;"
            "MuseScore Files (*.mscz *.msc);;"
            "MusicXML Files (*.xml *.mxl);;"
            "MIDI Files (*.mid *.kar);;"
            "Muse Data Files (*.md);;"
            "LilyPond Files <experimental> (*.ly);;"
            "BB Files <experimental> (*.mgu *.MGU *.sgu *.SGU);;"
            "All Files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      Score* score = new Score();
      score->read(fn);
      appendScore(score);
      lastOpenPath = score->fileInfo()->path();
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

/**
 Save the current score.
 Handles the GUI's file-save action.
 Return true if OK and false on error.
 */

bool MuseScore::saveFile()
      {
      bool val = cs->saveFile(false);
      setWindowTitle("MuseScore: " + cs->name());
      tab->setTabText(tab->currentIndex(), cs->name());
      return val;
      }

/**
 If file has generated name, create a modal file save dialog
 and ask filename.
 Rename old file to backup file (.xxxx.msc,).
 Default is to save score in .msc format,
 but (compressed) MusicXML files are also saved correctly.
 Return true if OK and false on error.
 */

bool Score::saveFile(bool autosave)
      {
      if (created()) {
            QString fn = QFileDialog::getSaveFileName(
               mscore, tr("MuseScore: Save Score"),
               QString("./%1.mscz").arg(name()),
               tr("Compressed MuseScore File (*.mscz);;"
                  "MuseScore File (*.msc);;")
               );
            if (fn.isEmpty())
                  return false;
            fileInfo()->setFile(fn);
            setCreated(false);
            }

      // if file was already saved in this session
      // save but don't overwrite backup again

      if (saved()) {
            bool rv = false;
            if (fileInfo()->suffix() == "mxl")
                  rv = saveMxl(fileInfo()->absoluteFilePath());
            else if (fileInfo()->suffix() == "xml")
                  rv = saveXml(fileInfo()->absoluteFilePath());
            else if (fileInfo()->suffix() == "msc")
                  rv = saveFile(*fileInfo(), autosave);
            else
                  rv = saveCompressedFile(*fileInfo(), autosave);
            if (rv)
                  setDirty(false);
            return rv;
            }
      //
      // step 1
      // save into temporary file
      //

      QTemporaryFile temp(info.path() + "/msXXXXXX");
      temp.setAutoRemove(false);
      if (!temp.open()) {
            QString s = tr("Open Temp File\n") + temp.fileName() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open File"), s);
            return false;
            }
      bool rv = false;
      if (info.suffix() == "mxl")
            rv = saveMxl(temp.fileName());
      else if (info.suffix() == "xml")
            rv = saveXml(temp.fileName());
      else if (info.suffix() == "msc")
            rv = saveFile(&temp, autosave);
      else
            rv = saveCompressedFile(&temp, info, autosave);
      if (!rv)
            return false;

      //
      // step 2
      // remove old backup file if exists
      //
      QDir dir(info.path());
      QString backupName = QString(".") + info.fileName() + QString(",");
      dir.remove(backupName);

      //
      // step 3
      // rename old file into backup
      //
      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + QString(".mscz"));
      QString name(info.filePath());
      dir.rename(name, backupName);

      //
      // step 4
      // rename temp name into file name
      //
      temp.rename(name);
      setDirty(false);
      setSaved(true);
      return true;
      }

//---------------------------------------------------------
//   saveAs
//    return true on success
//---------------------------------------------------------

/**
 Save the current score using a different name or type.
 Handles the GUI's file-save-as and file-save-a-copy actions.
 The saveCopy flag, if true, does not change the name of the active score nor marks it clean.
 Return true if OK and false on error.
 */

bool Score::saveAs(bool saveCopy)
      {
      QString selectedFilter;
      QStringList fl;

      fl.append(tr("Compressed MuseScore Format (*.mscz)"));
      fl.append(tr("MuseScore Format (*.msc)"));
      fl.append(tr("MusicXML Format (*.xml)"));
      fl.append(tr("Compressed MusicXML Format (*.mxl)"));
      fl.append(tr("Standard MIDI File (*.mid)"));
      fl.append(tr("PDF File (*.pdf)"));
      fl.append(tr("Postscript File (*.ps)"));
      fl.append(tr("PNG Bitmap Graphic (*.png)"));
      fl.append(tr("Scalable Vector Graphic (*.svg)"));
      fl.append(tr("Lilypond Format (*.ly)"));

      QString saveDialogTitle = saveCopy ? tr("MuseScore: Save a Copy") :
                                           tr("MuseScore: Save As");
      QString saveDirectory = saveCopy ? preferences.lastSaveCopyDirectory :
                                         preferences.lastSaveDirectory;
      QString fn = QFileDialog::getSaveFileName(
         0, saveDialogTitle,
         saveDirectory,
         fl.join(";;"),
         &selectedFilter
         );
      if (fn.isEmpty())
            return false;

      bool rv = false;
      if (selectedFilter == fl[0] || selectedFilter == fl[1]) {
            // save as mscore *.msc(z) file
            if (selectedFilter == fl[0] && !fn.endsWith(".mscz"))
                  fn.append(".mscz");
            else if (selectedFilter == fl[1] && !fn.endsWith(".msc"))
                  fn.append(".msc");
            QFileInfo fi(fn);
            if (selectedFilter == fl[0])
                  rv = saveCompressedFile(fi, false);
            else
                  rv = saveFile(fi, false);
            if (rv && !saveCopy) {
                  fileInfo()->setFile(fn);
                  mscore->setWindowTitle("MuseScore: " + name());
                  setDirty(false);
                  mscore->dirtyChanged(this);
                  setCreated(false);
                  }
            }
      else if (selectedFilter == fl[2]) {
            // save as MusicXML *.xml file
            if (!fn.endsWith(".xml"))
                  fn.append(".xml");
            rv = saveXml(fn);
            }
      else if (selectedFilter == fl[3]) {
            // save as compressed MusicXML *.mxl file
            if (!fn.endsWith(".mxl"))
                  fn.append(".mxl");
            rv = saveMxl(fn);
            }
      else if (selectedFilter == fl[4]) {
            // save as midi file *.mid
            if (!fn.endsWith(".mid"))
                  fn.append(".mid");
            rv = saveMidi(fn);
            }
      else if (selectedFilter == fl[5]) {
            // save as pdf file *.pdf
            if (!fn.endsWith(".pdf"))
                  fn.append(".pdf");
            rv = savePsPdf(fn, QPrinter::PdfFormat);
            }
      else if (selectedFilter == fl[6]) {
            // save as postscript file *.ps
            if (!fn.endsWith(".ps"))
                  fn.append(".ps");
            rv = savePsPdf(fn, QPrinter::PostScriptFormat);
            }
      else if (selectedFilter == fl[7]) {
            // save as png file *.png
            if (!fn.endsWith(".png"))
                  fn.append(".png");
            rv = savePng(fn);
            }
      else if (selectedFilter == fl[8]) {
            // save as svg file *.svg
            if (!fn.endsWith(".svg"))
                  fn.append(".svg");
            rv = saveSvg(fn);
            }
      else if (selectedFilter == fl[9]) {
            // save as lilypond file *.ly
            if (!fn.endsWith(".ly"))
                  fn.append(".ly");
            rv = saveLilypond(fn);
            }

      // after a successful saveas (compressed) MusicXML, clear the "dirty" flag
      if (rv && (fn.endsWith(".xml") || fn.endsWith(".mxl")) && !saveCopy)
            setDirty(false);

      QFileInfo fi(fn);
      if (saveCopy)
            preferences.lastSaveCopyDirectory = fi.absolutePath();
      else
            preferences.lastSaveDirectory = fi.absolutePath();
      return rv;
      }

//---------------------------------------------------------
//   createDefaultName
//---------------------------------------------------------

QString MuseScore::createDefaultName() const
      {
      QString name(tr("Untitled"));
      int n;
      for (n = 1; ; ++n) {
            bool nameExists = false;
            QString tmpName;
            if (n == 1)
                  tmpName = name;
            else
                  tmpName = QString("%1-%2").arg(name).arg(n);
            foreach(Score* s, scoreList) {
                  if (s->name() == tmpName) {
                        nameExists = true;
                        break;
                        }
                  }
            if (!nameExists) {
                  name = tmpName;
                  break;
                  }
            }
      return name;
      }

//---------------------------------------------------------
//   newFile
//    create new score
//---------------------------------------------------------

void MuseScore::newFile()
      {
      if (newWizard == 0)
            newWizard = new NewWizard(this);
      newWizard->restart();
      if (newWizard->exec() != QDialog::Accepted)
            return;
      int measures = newWizard->measures();
      int timesigZ, timesigN;
      int pickupTimesigZ, pickupTimesigN;
      newWizard->timesig(&timesigZ, &timesigN);
      bool pickupMeasure = newWizard->pickupMeasure(&pickupTimesigZ, &pickupTimesigN);
      char ks = newWizard->keysig();

      Score* score = new Score;
      score->setCreated(true);

      //
      //  create score from template
      //
      if (newWizard->useTemplate()) {
            score->read(newWizard->templatePath());
            score->fileInfo()->setFile(createDefaultName());

            int m = 0;
            for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
                  if (mb->type() == MEASURE)
                        ++m;
                  }
            if (m < measures)
                  measures -= m;
            else
                  measures = 0;
            //
            // remove all notes & rests
            //
            int tracks = score->nstaves() * VOICES;
            for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* measure = static_cast<Measure*>(mb);
                  for (Segment* s = measure->first(); s;) {
                        Segment* ns = s->next();
                        if (
                              (s->subtype() == Segment::SegChordRest)
                           || (s->subtype() == Segment::SegClef)
                           || (s->subtype() == Segment::SegTimeSig)
                           ) {
                              for (int track = 0; track < tracks; ++track) {
                                    if (s->element(track))
                                          delete s->element(track);
                                    }
                              measure->remove(s);
                              }
                        s = ns;
                        }
                  }
            }
      //
      //  create new score from scratch
      //
      else {
            score->fileInfo()->setFile(createDefaultName());
            newWizard->createInstruments(score);
            }
      for (int i = 0; i < measures; ++i) {
            Measure* measure = new Measure(score);
            score->measures()->add(measure);
            }

      Measure* lastMeasure = static_cast<Measure*>(score->measures()->last());
      if ((lastMeasure->type() == MEASURE) && (lastMeasure->endBarLineType() == NORMAL_BAR))
            lastMeasure->setEndBarLineType(END_BAR, false);

      SigList* sigmap = score->getSigmap();
      if (pickupMeasure) {
            sigmap->add(0, SigEvent(pickupTimesigZ, pickupTimesigN, timesigZ, timesigN));
            int tick = score->getSigmap()->ticksMeasure(0);
            sigmap->add(tick, SigEvent(timesigZ, timesigN));
            }
      else {
            sigmap->add(0, SigEvent(timesigZ, timesigN));
            }

      int tick = 0;
      for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
            mb->setTick(tick);
            if (mb->type() != MEASURE)
                  continue;
            Measure* measure = static_cast<Measure*>(mb);
            int ticks = sigmap->ticksMeasure(tick);
	      for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  int len = 0;
                  if (tick == 0) {
                        TimeSig* ts = new TimeSig(score, timesigN, timesigZ);
                        ts->setTick(0);
                        ts->setTrack(staffIdx * VOICES);
                        Segment* s = measure->getSegment(ts);
                        s->add(ts);
                        Staff* staff = score->staff(staffIdx);
                        Part* part = staff->part();
                        Instrument* instrument = part->instrument();
                        //
                        // transpose key
                        //
                        int nKey = ks;
                        if (instrument->pitchOffset && !newWizard->useTemplate()) {
                              int diff = -instrument->pitchOffset;
                              nKey = transposeKey(nKey, diff);
                              }
                        if (nKey) {
                              (*(staff->keymap()))[0] = nKey;
                              KeySig* keysig = new KeySig(score);
                              keysig->setTrack(staffIdx * VOICES);
                              keysig->setTick(0);
                              keysig->setSig(0, nKey);
                              s = measure->getSegment(keysig);
                              s->add(keysig);
                              }
                        if (pickupMeasure)
	                        len = ticks;
                        }
		      Rest* rest = new Rest(score, tick, len);
      	      rest->setTrack(staffIdx * VOICES);
	      	Segment* s = measure->getSegment(rest);
		      s->add(rest);
                  }
            tick += ticks;
            }
      score->fixTicks();

      QString title     = newWizard->title();
      QString subtitle  = newWizard->subtitle();
      QString composer  = newWizard->composer();
      QString poet      = newWizard->poet();
      QString copyright = newWizard->copyright();
      if (!title.isEmpty() || !subtitle.isEmpty() || !composer.isEmpty() || !poet.isEmpty()) {
            MeasureBase* measure = score->measures()->first();
            if (measure->type() != VBOX) {
                  MeasureBase* nm = new VBox(score);
                  nm->setTick(0);
                  nm->setNext(measure);
                  score->measures()->add(nm);
                  measure = nm;
                  }
            if (!title.isEmpty()) {
                  Text* s = new Text(score);
                  s->setSubtype(TEXT_TITLE);
                  s->setText(title);
                  measure->add(s);
                  }
            if (!subtitle.isEmpty()) {
                  Text* s = new Text(score);
                  s->setSubtype(TEXT_SUBTITLE);
                  s->setText(subtitle);
                  measure->add(s);
                  }
            if (!composer.isEmpty()) {
                  Text* s = new Text(score);
                  s->setSubtype(TEXT_COMPOSER);
                  s->setText(composer);
                  measure->add(s);
                  }
            if (!poet.isEmpty()) {
                  Text* s = new Text(score);
                  s->setSubtype(TEXT_POET);
                  s->setText(poet);
                  measure->add(s);
                  }
            }
      if (!copyright.isEmpty())
            score->setCopyright(copyright);

      appendScore(score);
      score->rebuildMidiMapping();
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   saveCompressedFile
//    return true on success
//---------------------------------------------------------

bool Score::saveCompressedFile(QFileInfo& info, bool autosave)
      {
      QString ext(".mscz");

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);

      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = tr("Open File\n") + info.filePath() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(0, tr("MuseScore: Open File"), s);
            return false;
            }
      bool rv = saveCompressedFile(&fp, info, autosave);
//      fp.close();
      return rv;
      }

//---------------------------------------------------------
//   saveCompressedFile
//---------------------------------------------------------

bool Score::saveCompressedFile(QIODevice* f, QFileInfo& info, bool autosave)
      {
      Zip uz;
      Zip::ErrorCode ec = uz.createArchive(f);
      if (ec != Zip::Ok) {
            printf("Cannot create compressed musescore file\n");
            return false;
            }

      QDateTime dt;
      if (debugMode)
            dt = QDateTime(QDate(2007, 9, 10), QTime(12, 0));
      else
            dt = QDateTime::currentDateTime();

      QString fn = info.completeBaseName() + ".msc";
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      Xml xml(&cbuf);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(fn));
      xml.etag();
      xml.etag();
      xml.etag();
      cbuf.seek(0);
      ec = uz.createEntry("META-INF/container.xml", cbuf, dt);
      if (ec != Zip::Ok) {
            printf("Cannot add container.xml to zipfile '%s'\n", qPrintable(info.filePath()));
            return false;
            }

      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      bool rv = saveFile(&dbuf, autosave);
      dbuf.seek(0);
      ec = uz.createEntry(fn, dbuf, dt);
      if (ec != Zip::Ok) {
            printf("Cannot add %s to zipfile '%s'\n", qPrintable(fn), qPrintable(info.filePath()));
            return false;
            }
      ec = uz.closeArchive();
      if (ec != Zip::Ok) {
            printf("Cannot close zipfile '%s'\n", qPrintable(info.filePath()));
            return false;
            }
      return rv;
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

bool Score::saveFile(QFileInfo& info, bool autosave)
      {
      QString ext(".msc");

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = tr("Open File\n") + info.filePath() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(0, tr("MuseScore: Open File"), s);
            return false;
            }
      bool rv = saveFile(&fp, autosave);
      fp.close();
      return rv;
      }

//---------------------------------------------------------
//   StaffLines::write
//---------------------------------------------------------

void StaffLines::write(Xml& xml) const
      {
      xml.stag("Staff");
      if (lines() != 5)
            xml.tag("lines", lines());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   StaffLines::read
//---------------------------------------------------------

void StaffLines::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "lines")
                  setLines(e.text().toInt());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

void Score::loadStyle()
      {
      QString fn = QFileDialog::getOpenFileName(
         0, QWidget::tr("MuseScore: Load Style"),
         QString("."),
            QWidget::tr("MuseScore Styles (*.mss);;"
            "All Files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      QFile f(fn);
      if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load Style failed:"),
               QString(strerror(errno)),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            }
      loadStyle(&f);
      }

//---------------------------------------------------------
//   loadStyle
//    return true on error
//---------------------------------------------------------

bool Score::loadStyle(QFile* qf)
      {
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(qf, false, &err, &line, &column)) {
            QString error;
            error.sprintf("error reading style file %s at line %d column %d: %s\n",
               qf->fileName().toLatin1().data(), line, column, err.toLatin1().data());
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load Style failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return true;
            }
      docName = qf->fileName();
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  QString version = e.attribute(QString("version"));
                  QStringList sl = version.split('.');
                  _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "Style")
                              _style->load(ee, _mscVersion);
                        else if (tag == "TextStyle") {
                              QString name = ee.attribute("name");
                              TextStyle* s = 0;
                              foreach(TextStyle* ts, textStyles()) {
                                    if (ts->name == name) {
                                          s = ts;
                                          break;
                                          }
                                    }
                              if (s == 0) {
                                    printf("new TextStyle <%s>\n", qPrintable(name));
                                    }
                              else
                                    s->read(ee);
                              }
                        else
                              domError(ee);
                        }
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void Score::saveStyle()
      {
      QString name = QFileDialog::getSaveFileName(
         0, tr("MuseScore: Save Style"),
         ".",
         tr("MuseScore Style File (*.mss)")
         );
      if (name.isEmpty())
            return;
      QString ext(".mss");
      QFileInfo info(name);

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile f(info.filePath());
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = tr("Open Style File\n") + f.fileName() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open Style file"), s);
            return;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      _style->save(xml);
      foreach(TextStyle* ts, textStyles())
            ts->write(xml);

      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = QString("Write Style failed: ") + f.errorString();
            QMessageBox::critical(0, tr("MuseScore: Write Style"), s);
            }
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

bool Score::saveFile(QIODevice* f, bool autosave)
      {
      Xml xml(f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");

      xml.tag("Mag",  mag());
      xml.tag("xoff", xoffset() / DPMM);
      xml.tag("yoff", yoffset() / DPMM);

      if (::symbolPalette)
            ::symbolPalette->write(xml, "Symbols");

      write(xml, autosave);
      xml.etag();
#if 0
      if (f->error() != QFile::NoError) {
            QString s = QString("Write File failed: ") + f->errorString();
            QMessageBox::critical(this, tr("MuseScore: Write File"), s);
            return false;
            }
#endif
      return true;
      }

//---------------------------------------------------------
//   loadCompressedMsc
//    return true on error
//---------------------------------------------------------

bool Score::loadCompressedMsc(QString name)
      {
      QString ext(".mscz");

      info.setFile(name);
      if (info.suffix().isEmpty()) {
            name += ext;
            info.setFile(name);
            }

      UnZip uz;
      UnZip::ErrorCode ec = uz.openArchive(name);
      if (ec != UnZip::Ok)
            return true;

      QBuffer cbuf;
      cbuf.open(QIODevice::WriteOnly);
      ec = uz.extractFile("META-INF/container.xml", &cbuf);

      QDomDocument container;
      int line, column;
      QString err;
      if (!container.setContent(cbuf.data(), false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            printf("error: %s\n", qPrintable(error));
            return true;
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
                        if (eee.tagName() == "rootfile") {
                              if (rootfile.isEmpty())
                                    rootfile = eee.attribute(QString("full-path"));
                              }
                        else
                              domError(eee);
                        }
                  }
            }
      if (rootfile.isEmpty()) {
            printf("can't find rootfile in: %s\n", qPrintable(name));
            return true;
            }

      QBuffer dbuf;
      dbuf.open(QIODevice::WriteOnly);
      ec = uz.extractFile(rootfile, &dbuf);

      QDomDocument doc;
      if (!doc.setContent(dbuf.data(), false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            printf("error: %s\n", qPrintable(error));
            return true;
            }
      dbuf.close();
      docName = info.completeBaseName();
      return read(doc.documentElement());
      }

//---------------------------------------------------------
//   loadMsc
//    return true if file not found or error loading
//---------------------------------------------------------

bool Score::loadMsc(QString name)
      {
      QString ext(".msc");

      info.setFile(name);
      if (info.suffix().isEmpty()) {
            name += ext;
            info.setFile(name);
            }
      QFile f(name);
      if (!f.open(QIODevice::ReadOnly))
            return true;

      QDomDocument doc;
      int line, column;
      QString err;
      QXmlSimpleReader reader;
      QXmlInputSource  source(&f);
      if (!doc.setContent(&source, &reader, &err, &line, &column)) {
            QString s;
            s.sprintf("error reading file %s at line %d column %d: %s\n",
               f.fileName().toLatin1().data(), line, column, err.toLatin1().data());

            QMessageBox::critical(mscore, tr("MuseScore: Read File"), s);
            return true;
            }
      f.close();
      docName = f.fileName();
      return read(doc.documentElement());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool Score::read(QDomElement e)
      {
      _fileDivision = 384;   // for compatibility with old mscore files

      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() != "museScore")
                  continue;
            QString version = e.attribute(QString("version"));
            QStringList sl = version.split('.');
            _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  curTrack = -1;
                  QString tag(ee.tagName());
                  QString val(ee.text());
                  int i = val.toInt();
                  if (tag == "Staff")
                        readStaff(ee);
                  else if (tag == "siglist")
                        sigmap->read(ee, division, _fileDivision);
                  else if (tag == "tempolist")
                        tempomap->read(ee, this);
                  else if (tag == "Mag")
                        setMag(val.toDouble());
                  else if (tag == "xoff") {
                        if (_mscVersion < 105)
                              setXoffset(val.toDouble());
                        else
                              setXoffset(val.toDouble() * DPMM);
                        }
                  else if (tag == "yoff") {
                        if (_mscVersion < 105)
                              setYoffset(val.toDouble());
                        else
                              setYoffset(val.toDouble() * DPMM);
                        }
                  else if (tag == "Spatium")
                        setSpatium (val.toDouble() * DPMM);
                  else if (tag == "Division")
                        _fileDivision = i;
                  else if (tag == "showInvisible")
                        _showInvisible = i;
                  else if (tag == "Style")
                        _style->load(ee, _mscVersion);
                  else if (tag == "TextStyle") {
                        QString name = ee.attribute("name");
                        TextStyle* s = 0;
                        foreach(TextStyle* ts, textStyles()) {
                              if (ts->name == name) {
                                    s = ts;
                                    break;
                                    }
                              }
                        if (s == 0) {
                              printf("new TextStyle <%s>\n", qPrintable(name));
                              }
                        else
                              s->read(ee);
                        }
                  else if (tag == "page-layout")
                        pageFormat()->read(ee);
                  else if (tag == "rights") {
                        if (rights == 0) {
                              rights = new QTextDocument(0);
                              rights->setUseDesignMetrics(true);
                              }
                        if (mscVersion() <= 103)
                              rights->setHtml(val);
                        else
                              rights->setHtml(Xml::htmlToString(ee.firstChildElement()));
                        }
                  else if (tag == "movement-number")
                        _movementNumber = val;
                  else if (tag == "movement-title")
                        _movementTitle = val;
                  else if (tag == "work-number")
                        _workNumber = val;
                  else if (tag == "work-title")
                        _workTitle = val;
                  else if (tag == "source")
                        _source = val;
                  else if (tag == "Part") {
                        Part* part = new Part(this);
                        part->read(ee);
                        _parts.push_back(part);
                        }
                  else if (tag == "showInvisible")
                        _showInvisible = i;
                  else if (tag == "Symbols") {
                        if (::symbolPalette == 0)
                              createSymbolPalette();
                        ::symbolPalette->read(ee);
                        }
                  else if (tag == "cursorTrack") {
                        if (i >= 0)
                              setInputTrack(i);
                        }
                  else if (tag == "Slur") {
                        Slur* slur = new Slur(this);
                        slur->read(ee);
                        _layout->add(slur);
                        }
                  else if (tag == "HairPin") {
                        Hairpin* hairpin = new Hairpin(this);
                        hairpin->read(ee);
                        _layout->add(hairpin);
                        }
                  else if (tag == "Ottava") {
                        Ottava* ottava = new Ottava(this);
                        ottava->read(ee);
                        _layout->add(ottava);
                        }
                  else if (tag == "TextLine") {
                        TextLine* textLine = new TextLine(this);
                        textLine->read(ee);
                        _layout->add(textLine);
                        }
                  else if (tag == "Volta") {
                        Volta* volta = new Volta(this);
                        volta->read(ee);
                        _layout->add(volta);
                        }
                  else if (tag == "Trill") {
                        Trill* trill = new Trill(this);
                        trill->read(ee);
                        _layout->add(trill);
                        }
                  else if (tag == "Pedal") {
                        Pedal* pedal = new Pedal(this);
                        pedal->read(ee);
                        _layout->add(pedal);
                        }
                  else if (tag == "Excerpt") {
                        Excerpt* e = new Excerpt(this);
                        e->read(ee);
                        _excerpts.append(e);
                        }
                  else
                        domError(ee);
                  }
            }
      if (_mscVersion < 108)
            connectSlurs();

      _layout->connectTies();
//      _layout->searchHiddenNotes();
      _layout->setInstrumentNames();

      searchSelectedElements();
      _fileDivision = division;
      return false;
      }

//---------------------------------------------------------
//   connectSlurs
//    helper routine for old msc versions and MusicXml import
//---------------------------------------------------------

void Score::connectSlurs()
      {
      foreach(Element* e, _gel) {
            if (e->type() != SLUR)
                  continue;
            Slur* s = (Slur*)e;
            Element* n1 = searchNote(s->tick(), s->track());
            Element* n2 = searchNote(s->tick2(), s->track2());
            if (n1 == 0 || n2 == 0) {
                  printf("  not found\n");
                  // TODO: remove SLur
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
                        printf("   n2 is %s\n", n2->name());
                  }
            }
      }

//---------------------------------------------------------
//   checkSlurs
//    helper routine to check for sanity slurs
//---------------------------------------------------------

void Score::checkSlurs()
      {
      foreach(Element* e, _gel) {
            if (e->type() != SLUR)
                  continue;
            Slur* s = (Slur*)e;
            Element* n1 = s->startElement();
            Element* n2 = s->endElement();
            if (n1 == 0 || n2 == 0 || n1 == n2) {
                  printf("unconnected slur: removing\n");
                  if (n1) {
                        ((ChordRest*)n1)->removeSlurFor(s);
                        ((ChordRest*)n1)->removeSlurBack(s);
                        }
                  int idx = _gel.indexOf(s);
                  _gel.removeAt(idx);
                  }
            }
      }

//---------------------------------------------------------
//   checkTuplets
//    helper routine to check for tuplet sanity
//---------------------------------------------------------

void Score::checkTuplets()
      {
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            foreach(Tuplet* t, *m->tuplets()) {
                  if (t->elements()->empty()) {
                        printf("empty tuplet: removing\n");
                        m->tuplets()->removeAll(t);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   printFile
//---------------------------------------------------------

void Score::printFile()
      {
      pdev->setPageSize(paperSizes[pageFormat()->size].qtsize);
      pdev->setOrientation(pageFormat()->landscape ? QPrinter::Landscape : QPrinter::Portrait);
      pdev->setCreator("MuseScore Version: " VERSION);
      pdev->setFullPage(true);
      pdev->setColorMode(QPrinter::Color);

      pdev->setDocName(name());
      pdev->setDoubleSidedPrinting(pageFormat()->twosided);

#ifndef __MINGW32__
      // when setting this on windows platform, pd.exec() does not
      // show dialog
      pdev->setOutputFileName(info.path() + "/" + name() + ".pdf");
#endif

      QPrintDialog pd(pdev, 0);
      if (!pd.exec()) {
            return;
            }
      print(pdev);
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void Score::print(QPrinter* printer)
      {
      _printing = true;
      QPainter p(printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      double mag = printer->logicalDpiX() / DPI;
      p.scale(mag, mag);

      QList<const Element*> el;
      foreach (const Element* element, _gel)
            element->collectElements(el);
      for (MeasureBase* m = _measures.first(); m; m = m->next())
            m->collectElements(el);

      const QList<Page*> pl = _layout->pages();
      int pages = pl.size();
      for (int n = 0; n < pages; ++n) {
            if (n)
                  printer->newPage();
            const Page* page = pl.at(n);
            page->collectElements(el);
            for (int i = 0; i < el.size(); ++i) {
                  const Element* e = el[i];
                  if (!e->visible())
                        continue;
                  QPointF ap(e->canvasPos() - page->pos());
                  p.translate(ap);
                  p.setPen(QPen(e->color()));
                  e->draw(p);
                  p.translate(-ap);
                  }
            }
      p.end();
      _printing = false;
      }

//---------------------------------------------------------
//   savePsPdf
//---------------------------------------------------------

bool Score::savePsPdf(const QString& saveName, QPrinter::OutputFormat format)
      {
      QPrinter p(QPrinter::HighResolution);
      p.setPageSize(paperSizes[pageFormat()->size].qtsize);
      p.setOrientation(pageFormat()->landscape ? QPrinter::Landscape : QPrinter::Portrait);
      p.setCreator("MuseScore Version: " VERSION);
      p.setFullPage(true);
      p.setColorMode(QPrinter::Color);
      p.setDocName(name());
      p.setDoubleSidedPrinting(pageFormat()->twosided);
      p.setOutputFormat(format);
      p.setOutputFileName(saveName);
      print(&p);
      return true;
      }

//---------------------------------------------------------
//   saveSvg
//---------------------------------------------------------

bool Score::saveSvg(const QString& saveName)
      {
      QRectF r = canvas()->lassoRect();
      double x = r.x();
      double y = r.y();
      double w = r.width();
      double h = r.height();

      QSvgGenerator printer;
      printer.setFileName(saveName);
      printer.setSize(QSize(lrint(w), lrint(h)));

      _printing = true;
      QPainter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      p.setClipRect(QRect(0, 0, lrint(w), lrint(h)));
      p.setClipping(true);

      QPointF offset(x, y);

      QList<const Element*> el;
      foreach(const Page* page, _layout->pages()) {
            el.clear();
            page->collectElements(el);
            foreach(System* system, *page->systems()) {
                  foreach(MeasureBase* m, system->measures()) {
                        m->collectElements(el);
                        }
                  }
            for (int i = 0; i < el.size(); ++i) {
                  const Element* e = el[i];
                  if (!e->visible())
                        continue;
                  QPointF ap(e->canvasPos() - offset);
                  p.translate(ap);
                  p.setPen(QPen(e->color()));
                  e->draw(p);
                  p.translate(-ap);
                  }
            }

      _printing = false;
      p.end();
      return true;
      }

//---------------------------------------------------------
//   savePng
//    return true on success
//---------------------------------------------------------

bool Score::savePng(const QString& name)
      {
      if (!preferences.pngScreenShot)
            _printing = true;             // dont print page break symbols etc.

      bool rv = true;
      if (!canvas()->lassoRect().isEmpty() && preferences.pngScreenShot) {
            // this is a special hack to export only the canvas lasso selection
            // into png (screen shot mode)

            QRectF r = canvas()->matrix().mapRect(canvas()->lassoRect());

            int w = lrint(r.width()  * converterDpi / DPI);
            int h = lrint(r.height() * converterDpi / DPI);

            QImage printer(w, h, QImage::Format_ARGB32_Premultiplied);
            printer.setDotsPerMeterX(lrint(DPMM * 1000.0));
            printer.setDotsPerMeterY(lrint(DPMM * 1000.0));
//          printer.fill(-1);     // white background
            printer.fill(0);      // transparent background
            double m = converterDpi / PDPI;
            QPainter p(&printer);
            canvas()->paintLasso(p, m);
            rv = printer.save(name, "png");
            }
      else {
            const QList<Page*>& pl = _layout->pages();
            int pages = pl.size();

            QList<const Element*> eel;
            foreach (const Element* element, _gel)
                  element->collectElements(eel);
            for (MeasureBase* m = _measures.first(); m; m = m->next())
                  m->collectElements(eel);

            for (int pageNumber = 0; pageNumber < pages; ++pageNumber) {
                  const Page* page = pl.at(pageNumber);
                  QList<const Element*> el;
                  page->collectElements(el);

                  QRectF r = page->abbox();
                  int w = lrint(r.width()  * converterDpi / DPI);
                  int h = lrint(r.height() * converterDpi / DPI);

                  QImage printer(w, h, QImage::Format_ARGB32_Premultiplied);
                  printer.setDotsPerMeterX(lrint(DPMM * 1000.0));
                  printer.setDotsPerMeterY(lrint(DPMM * 1000.0));
                  printer.fill(0);      // transparent background

                  double mag = converterDpi / DPI;
                  QPainter p(&printer);
                  p.setRenderHint(QPainter::Antialiasing, true);
                  p.setRenderHint(QPainter::TextAntialiasing, true);
                  p.scale(mag, mag);

                  foreach(const Element* e, eel) {
                        if (!e->visible())
                              continue;
                        QPointF ap(e->canvasPos() - page->pos());
                        p.translate(ap);
                        p.setPen(QPen(e->color()));
                        e->draw(p);
                        p.translate(-ap);
                        }

                  foreach(const Element* e, el) {
                        if (!e->visible())
                              continue;
                        QPointF ap(e->canvasPos() - page->pos());
                        p.translate(ap);
                        p.setPen(QPen(e->color()));
                        e->draw(p);
                        p.translate(-ap);
                        }

                  QString fileName(name);
                  if (fileName.endsWith(".png"))
                        fileName = fileName.left(fileName.size() - 4);
                  fileName += QString("-%1.png").arg(pageNumber+1);

                  rv = printer.save(fileName, "png");
                  if (!rv)
                        break;
                  }
            }
      _printing = false;
      return rv;
      }

