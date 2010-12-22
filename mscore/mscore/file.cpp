//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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
#include "scoreview.h"
#include "xml.h"
#include "element.h"
#include "note.h"
#include "rest.h"
#include "al/sig.h"
#include "clef.h"
#include "key.h"
#include "instrdialog.h"
#include "score.h"
#include "page.h"
#include "dynamics.h"
#include "file.h"
#include "style.h"
#include "al/tempo.h"
#include "select.h"
#include "preferences.h"
#include "input.h"
#include "playpanel.h"
#include "staff.h"
#include "part.h"
#include "utils.h"
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
#include "magbox.h"
#include "measure.h"
#include "undo.h"
#include "repeatlist.h"
#include "scoretab.h"
#include "beam.h"

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
      if (!fp.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: file not found:"),
               name,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return true;
            }
      if (loader(&fp)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: load failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
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
               tr("Save changes to the score \"%1\"\n"
               "before closing?").arg(s->name()),
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
         tr("All Supported Files (*.mscz *.mscx *.msc *.xml *.mxl *.mid *.midi *.kar *.md *.mgu *.MGU *.sgu *.SGU *.cap *.ove);;")+
         tr("MuseScore Files (*.mscz *.mscx *.msc);;")+
         tr("MusicXML Files (*.xml *.mxl);;")+
         tr("MIDI Files (*.mid *.midi *.kar);;")+
         tr("Muse Data Files (*.md);;")+
         tr("Capella Files (*.cap);;")+
         tr("BB Files <experimental> (*.mgu *.MGU *.sgu *.SGU);;")+
         tr("Overture Files <experimental> (*.ove);;")+
         tr("All Files (*)")
            )
         );
      if (fn.isEmpty())
            return;
      Score* score = new Score(defaultStyle);
      if(score->read(fn)) {
            setCurrentScoreView(appendScore(score));
            lastOpenPath = score->fileInfo()->path();
            writeSessionFile(false);
            }
      else {
            delete score;
            }
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

/**
 Save the current score.
 Handles the GUI's file-save action.
 */

void MuseScore::saveFile()
      {
      if (cs->saveFile(false)) {
            setWindowTitle("MuseScore: " + cs->name());
            int idx = scoreList.indexOf(cs);
            tab1->setTabText(idx, cs->name());
            tab2->setTabText(idx, cs->name());
            QString tmp = cs->tmpName();
            if (!tmp.isEmpty()) {
                  QFile f(tmp);
                  if (!f.remove())
                        printf("cannot remove temporary file <%s>\n", qPrintable(f.fileName()));
                  cs->setTmpName("");
                  }
            writeSessionFile(false);
            }
      }

/**
 If file has generated name, create a modal file save dialog
 and ask filename.
 Rename old file to backup file (.xxxx.msc?,).
 Default is to save score in .mscz format,
 Return true if OK and false on error.
 */

bool Score::saveFile(bool autosave)
      {
      if (created()) {
            QString selectedFilter;
            QString f1 = tr("Compressed MuseScore File (*.mscz)");
            QString f2 = tr("MuseScore File (*.mscx)");
            QString fn = QFileDialog::getSaveFileName(
               mscore, tr("MuseScore: Save Score"),
               QString("%1/%2.mscz").arg(preferences.workingDirectory).arg(info.baseName()),
               f1 + ";;" + f2,
               &selectedFilter
               );
            if (fn.isEmpty())
                  return false;
            info.setFile(fn);
            mscore->updateRecentScores(this);
            setCreated(false);
            }
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
            QString s(tr("The following file is locked: \n%1 \n\nTry saving to a different location."));
            QMessageBox::critical(mscore, tr("MuseScore: Save File"), s.arg(info.filePath()));
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
                  QMessageBox::critical(mscore, tr("MuseScore: Save File"), s);
                  return false;
                  }
            _undo->setClean();
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
            QString s = tr("Open Temp File\n") + tempName + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Save File"), s);
            return false;
            }
      try {
            if (suffix == "msc" || suffix == "mscx")
                  saveFile(&temp, autosave);
            else
                  saveCompressedFile(&temp, info, autosave);
            }
      catch (QString s) {
            QMessageBox::critical(mscore, tr("MuseScore: Save File failed: "), s);
            return false;
            }
      if (temp.error() != QFile::NoError)
            QMessageBox::critical(mscore, tr("MuseScore: Save File failed: "), temp.errorString());
      temp.close();

      //
      // step 2
      // remove old backup file if exists
      //
      QDir dir(info.path());
      QString backupName = QString(".") + info.fileName() + QString(",");
      if (dir.exists(backupName)) {
            if (!dir.remove(backupName)) {
                  QMessageBox::critical(mscore, tr("MuseScore: Save File"),
                     tr("removing old backup file ") + backupName + tr(" failed"));
                  }
            }

      //
      // step 3
      // rename old file into backup
      //
      QString name(info.filePath());
      if (dir.exists(name)) {
            if (!dir.rename(name, backupName)) {
                  QMessageBox::critical(mscore, tr("MuseScore: Save File"),
                     tr("renaming old file <")
                      + name + tr("> to backup <") + backupName + tr("> failed"));
                  }
            }

      //
      // step 4
      // rename temp name into file name
      //
      if (!QFile::rename(tempName, name)) {
            QMessageBox::critical(mscore, tr("MuseScore: Save File"),
               tr("renaming temp. file <") + tempName + tr("> to <") + name + tr("> failed:\n")
               + QString(strerror(errno)));
            return false;
            }
      // make file readable by all
      QFile::setPermissions(name, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser
         | QFile::ReadGroup | QFile::ReadOther);

      _undo->setClean();
      setClean(true);
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
      QStringList fl;
      fl.append(tr("Compressed MuseScore Format (*.mscz)"));
      fl.append(tr("MuseScore Format (*.mscx)"));
      fl.append(tr("MusicXML Format (*.xml)"));
      fl.append(tr("Compressed MusicXML Format (*.mxl)"));
      fl.append(tr("Standard MIDI File (*.mid)"));
      fl.append(tr("PDF File (*.pdf)"));
      fl.append(tr("PostScript File (*.ps)"));
      fl.append(tr("PNG Bitmap Graphic (*.png)"));
      fl.append(tr("Scalable Vector Graphic (*.svg)"));
      fl.append(tr("Lilypond Format (*.ly)"));
#ifdef HAS_AUDIOFILE
      fl.append(tr("Wave Audio (*.wav)"));
      fl.append(tr("Flac Audio (*.flac)"));
      fl.append(tr("Ogg Vorbis Audio (*.ogg)"));
#endif

      QString saveDialogTitle = saveCopy ? tr("MuseScore: Save a Copy") :
                                           tr("MuseScore: Save As");

      QSettings settings;
      if (mscore->lastSaveCopyDirectory.isEmpty())
            mscore->lastSaveCopyDirectory = settings.value("lastSaveCopyDirectory", preferences.workingDirectory).toString();
      if (mscore->lastSaveDirectory.isEmpty())
            mscore->lastSaveDirectory = settings.value("lastSaveDirectory", preferences.workingDirectory).toString();
      QString saveDirectory = saveCopy ?
            mscore->lastSaveCopyDirectory : mscore->lastSaveDirectory;

      QString selectedFilter;
      QString fn = QFileDialog::getSaveFileName(
               0,
               saveDialogTitle,
               QString("%1/%2.mscz").arg(saveDirectory).arg(info.baseName()),
               fl.join(";;"),
               &selectedFilter
               );
      if (fn.isEmpty())
            return false;

      QFileInfo fi(fn);
      if (saveCopy)
            mscore->lastSaveCopyDirectory = fi.absolutePath();
      else
            mscore->lastSaveDirectory = fi.absolutePath();

      QString ext;
      if (selectedFilter.isEmpty())
            ext = fi.suffix();
      else {
            int idx = fl.indexOf(selectedFilter);
            if (idx != -1) {
                  static const char* extensions[] = {
                        "mscz", "mscx", "xml", "mxl", "mid", "pdf", "ps", "png", "svg", "ly",
#ifdef HAS_AUDIOFILE
                        "wav", "flac", "ogg"
#endif
                        };
                  ext = extensions[idx];
                  }
            }
      if (ext.isEmpty()) {
            QMessageBox::critical(mscore, tr("MuseScore: Save As"), tr("cannot determine file type"));
            return false;
            }

      if (fi.suffix() != ext)
            fn += "." + ext;
      return saveAs(saveCopy, fn, ext);
      }

//---------------------------------------------------------
//   saveAs
//---------------------------------------------------------

bool Score::saveAs(bool saveCopy, const QString& path, const QString& ext)
      {
      QString suffix = "." + ext;
      QString fn(path);
      if (!fn.endsWith(suffix))
            fn += suffix;
      bool rv = false;
      if (ext == "mscx" || ext == "mscz") {
            // save as mscore *.msc[xz] file
            QFileInfo fi(fn);
            rv = true;
            try {
                  if (ext == "mscz")
                        saveCompressedFile(fi, false);
                  else
                        saveFile(fi, false);
                  }
            catch (QString s) {
                  rv = false;
                  QMessageBox::critical(mscore, tr("MuseScore: Save As"), s);
                  }
            if (rv && !saveCopy) {
                  fileInfo()->setFile(fn);
                  mscore->setWindowTitle("MuseScore: " + name());
                  _undo->setClean();
                  mscore->dirtyChanged(this);
                  setCreated(false);
                  mscore->updateRecentScores(this);
                  mscore->writeSessionFile(false);
                  }
            }
      else if (ext == "xml") {
            // save as MusicXML *.xml file
            rv = saveXml(fn);
            }
      else if (ext == "mxl") {
            // save as compressed MusicXML *.mxl file
            rv = saveMxl(fn);
            }
      else if (ext == "mid") {
            // save as midi file *.mid
            rv = saveMidi(fn);
            }
      else if (ext == "pdf") {
            // save as pdf file *.pdf
            rv = savePsPdf(fn, QPrinter::PdfFormat);
            }
      else if (ext == "ps") {
            // save as postscript file *.ps
            rv = savePsPdf(fn, QPrinter::PostScriptFormat);
            }
      else if (ext == "png") {
            // save as png file *.png
            rv = savePng(fn);
            }
      else if (ext == "svg") {
            // save as svg file *.svg
            rv = saveSvg(fn);
            }
      else if (ext == "ly") {
            // save as lilypond file *.ly
            rv = saveLilypond(fn);
            }
#ifdef HAS_AUDIOFILE
      else if (ext == "wav" || ext == "flac" || ext == "ogg")
            rv = saveAudio(fn, ext);
#endif
      else {
            fprintf(stderr, "internal error: unsupported extension <%s>\n",
               qPrintable(ext));
            return false;
            }
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
      if (pickupMeasure)
	 	       measures += 1;
      KeySigEvent ks = newWizard->keysig();

      Score* score = new Score(defaultStyle);
      score->setCreated(true);
//      score->layout();

      //
      //  create score from template
      //
      if (newWizard->useTemplate()) {
            if (!score->read(newWizard->templatePath())) {
                  QMessageBox::warning(0,
                     tr("MuseScore: failure"),
                     tr("Load template file ") + newWizard->templatePath() + tr(" failed"),
                     QString::null, QString::null, QString::null, 0, 1);
                  delete score;
                  return;
                  }
            score->fileInfo()->setFile(createDefaultName());

            int m = 0;
            for (Measure* mb = score->firstMeasure(); mb; mb = mb->nextMeasure()) {
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
            for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  Staff* staff = score->staff(staffIdx);
                  staff->keymap()->clear();
                  }

            int tracks = score->nstaves() * VOICES;
            for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
                  for (Segment* s = measure->first(); s;) {
                        Segment* ns = s->next();
                        if (
                              (s->subtype() == SegChordRest)
                           || (s->subtype() == SegClef)
                           || (s->subtype() == SegKeySig)
                           || (s->subtype() == SegGrace)
                           || (s->subtype() == SegBreath)
                           || (s->subtype() == SegTimeSig)
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
      if (!newWizard->title().isEmpty())
            score->fileInfo()->setFile(newWizard->title());
      for (int i = 0; i < measures; ++i) {
            Measure* measure = new Measure(score);
            score->measures()->add(measure);
            }

      Measure* lastMeasure = score->lastMeasure();
      if (lastMeasure && (lastMeasure->endBarLineType() == NORMAL_BAR))
            lastMeasure->setEndBarLineType(END_BAR, false);

      AL::TimeSigMap* sigmap = score->sigmap();
      if (pickupMeasure) {
            sigmap->add(0, AL::SigEvent(Fraction(pickupTimesigZ, pickupTimesigN), Fraction(timesigZ, timesigN)));
            int tick = score->sigmap()->ticksMeasure(0);
            sigmap->add(tick, AL::SigEvent(Fraction(timesigZ, timesigN)));
            score->firstMeasure()->setIrregular(true);
            }
      else {
            sigmap->add(0, AL::SigEvent(Fraction(timesigZ, timesigN)));
            }

      int tick = 0;
      for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
            mb->setTick(tick);
            if (mb->type() != MEASURE)
                  continue;
            Measure* measure = static_cast<Measure*>(mb);
            int ticks = sigmap->ticksMeasure(tick);
	      for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  Duration d(Duration::V_MEASURE);
                  if (tick == 0) {
                        TimeSig* ts = new TimeSig(score, timesigN, timesigZ);
                        ts->setTick(0);
                        ts->setTrack(staffIdx * VOICES);
                        Segment* s = measure->getSegment(ts);
                        s->add(ts);
                        Staff* staff = score->staff(staffIdx);
                        Part* part = staff->part();
                        if (!part->useDrumset()) {
                              //
                              // transpose key
                              //
                              KeySigEvent nKey = ks;
                              if (part->transpose().chromatic && !newWizard->useTemplate()) {
                                    int diff = -part->transpose().chromatic;
                                    nKey.setAccidentalType(transposeKey(nKey.accidentalType(), diff));
                                    }
                              if (nKey.accidentalType()) {
                                    (*(staff->keymap()))[0] = nKey;
                                    KeySig* keysig = new KeySig(score);
                                    keysig->setTrack(staffIdx * VOICES);
                                    keysig->setTick(0);
                                    keysig->setSubtype(nKey);
                                    s = measure->getSegment(keysig);
                                    s->add(keysig);
                                    }
                              }
                        if (pickupMeasure)
	                        d.setVal(ticks);
                        }
		      Rest* rest = new Rest(score, tick, d);
      	      rest->setTrack(staffIdx * VOICES);
	      	Segment* s = measure->getSegment(rest);
		      s->add(rest);
                  }
            tick += ticks;
            }
      score->fixTicks();
      //
      // select first rest
      //
      Measure* m = score->firstMeasure();
      for (Segment* s = m->first(); s; s = s->next()) {
            if (s->subtype() == SegChordRest) {
                  if (s->element(0)) {
                        score->select(s->element(0), SELECT_SINGLE, 0);
                        break;
                        }
                  }
            }

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
                  s->setTextStyle(TEXT_STYLE_TITLE);
                  s->setText(title);
                  measure->add(s);
                  }
            if (!subtitle.isEmpty()) {
                  Text* s = new Text(score);
                  s->setSubtype(TEXT_SUBTITLE);
                  s->setTextStyle(TEXT_STYLE_SUBTITLE);
                  s->setText(subtitle);
                  measure->add(s);
                  }
            if (!composer.isEmpty()) {
                  Text* s = new Text(score);
                  s->setSubtype(TEXT_COMPOSER);
                  s->setTextStyle(TEXT_STYLE_COMPOSER);
                  s->setText(composer);
                  measure->add(s);
                  }
            if (!poet.isEmpty()) {
                  Text* s = new Text(score);
                  s->setSubtype(TEXT_POET);
                  s->setTextStyle(TEXT_STYLE_POET);
                  s->setText(poet);
                  measure->add(s);
                  }
            }
      if (!copyright.isEmpty())
            score->setCopyright(copyright);

      score->rebuildMidiMapping();
      score->doLayout();
      setCurrentScoreView(appendScore(score));
      }

//---------------------------------------------------------
//   saveCompressedFile
//---------------------------------------------------------

void Score::saveCompressedFile(QFileInfo& info, bool autosave)
      {
      QString ext(".mscz");

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);

      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = tr("Open File\n") + info.filePath() + tr("\nfailed: ")
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
      Zip::ErrorCode ec = uz.createArchive(f);
      if (ec != Zip::Ok)
            throw (QString("Cannot create compressed musescore file"));

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
      ec = uz.createEntry("META-INF/container.xml", cbuf, dt);
      if (ec != Zip::Ok)
            throw(QString("Cannot add container.xml to zipfile '%1'\n").arg(info.filePath()));

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
            ec = uz.createEntry(dstPath, cbuf, dt);
            if (ec != Zip::Ok)
                  throw(QString("Cannot add <%1> to zipfile as <%1>\n").arg(srcPath).arg(dstPath));
            cbuf.close();
            ip->setPath(dstPath);   // image now has local path
            ++idx;
            }

      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      saveFile(&dbuf, autosave);
      dbuf.seek(0);
      ec = uz.createEntry(fn, dbuf, dt);
      if (ec != Zip::Ok)
            throw(QString("Cannot add %1 to zipfile '%2'").arg(fn).arg(info.filePath()));
      ec = uz.closeArchive();
      if (ec != Zip::Ok)
            throw(QString("Cannot close zipfile '%1'").arg(info.filePath()));
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

void Score::saveFile(QFileInfo& info, bool autosave)
      {
      QString ext(".mscx");

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = tr("Open File\n") + info.filePath() + tr("\nfailed: ")
               + QString(strerror(errno));
            throw(s);
            }
      saveFile(&fp, autosave);
      fp.close();
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
                              _style.load(ee, _mscVersion);
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
      _style.save(xml, false);     // save complete style
      foreach(TextStyle* ts, textStyles())
            ts->write(xml);

      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = tr("Write Style failed: ") + f.errorString();
            QMessageBox::critical(0, tr("MuseScore: Write Style"), s);
            }
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

extern QString revision;

void Score::saveFile(QIODevice* f, bool autosave)
      {
      Xml xml(f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.tag("programVersion", VERSION);
      xml.tag("programRevision", revision);
      write(xml, autosave);
      xml.etag();
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

      UnZip uz;
      UnZip::ErrorCode ec = uz.openArchive(name);
      if (ec != UnZip::Ok)
            return false;

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
            ec = uz.extractFile(ip->path(), &dbuf);
            if (ec != UnZip::Ok) {
                  printf("Cannot read <%s> from zipfile\n", qPrintable(ip->path()));
                  }
            else
                  ip->setLoaded(true);
            }

      if (rootfile.isEmpty()) {
            printf("can't find rootfile in: %s\n", qPrintable(name));
            return false;
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
            return false;
            }
      dbuf.close();
      docName = info.completeBaseName();
      return read(doc.documentElement());
      }

//---------------------------------------------------------
//   loadMsc
//    return false if file not found or error loading
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
      if (!f.open(QIODevice::ReadOnly))
            return false;

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString s;
            s.sprintf("error reading file %s at line %d column %d: %s\n",
               f.fileName().toLatin1().data(), line, column, err.toLatin1().data());

            QMessageBox::critical(mscore, tr("MuseScore: Read File"), s);
            return false;
            }
      f.close();
      docName = f.fileName();
      return read(doc.documentElement());
      }

//---------------------------------------------------------
//   read
//    return false on error
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
            if (_mscVersion > MSCVERSION) {
                  // incompatible version
                  QMessageBox::critical(0, tr("MuseScore"),
                     QT_TRANSLATE_NOOP("score", "Cannot read this score:\n"
                        "your version of MuseScore is too old.")
                     );
                  return false;
                  }

            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
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
                  else if (tag == "siglist")
                        _sigmap->read(ee, _fileDivision);
                  else if (tag == "tempolist")
                        _tempomap->read(ee, _fileDivision);
                  else if (tag == "programVersion") {
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

//                                                printf("Version %d.%d.%d   read %d.%d.%d\n",
//                                                   v1, v2, v3, rv1, rv2, rv3);

                                                int currentVersion = v1 * 10000 + v2 * 100 + v3;
                                                int readVersion = rv1 * 10000 + rv2 * 100 + v3;
                                                if (readVersion > currentVersion) {
                                                      printf("read future version\n");
                                                      }
                                                }
                                          }
                                    else
                                          printf("1cannot parse <%s>\n", qPrintable(val));
                                    }
                              }
                        else
                              printf("2cannot parse <%s>\n", VERSION);
                        }
                  else if (tag == "programRevision")
                        ;
                  else if (tag == "Mag")              // obsolete
                        ;
                  else if (tag == "MagIdx")           // obsolete
                        ;
                  else if (tag == "xoff") {           // obsolete
                        //_xoff = val.toDouble();
                        //if (_mscVersion >= 105)
                        //      _xoff *= DPMM;
                        }
                  else if (tag == "yoff") {           // obsolete
                        //_yoff = val.toDouble();
                        //if (_mscVersion >= 105)
                        //      _yoff *= DPMM;
                        }
                  else if (tag == "Spatium")
                        setSpatium (val.toDouble() * DPMM);
                  else if (tag == "Division")
                        _fileDivision = i;
                  else if (tag == "showInvisible")
                        _showInvisible = i;
                  else if (tag == "showFrames")
                        _showFrames = i;
                  else if (tag == "Style")
                        _style.load(ee, _mscVersion);
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
                  else if (tag == "rights") {   // obsolete
                        if (rights == 0) {
                              rights = new TextC(this);
                              rights->setSubtype(TEXT_COPYRIGHT);
                              rights->setTextStyle(TEXT_STYLE_COPYRIGHT);
                              }
                        if (mscVersion() <= 103)
                              rights->setHtml(val);
                        else
                              rights->setHtml(Xml::htmlToString(ee.firstChildElement()));
                        }
                  else if (tag == "copyright") {
                        if (rights == 0) {
                              rights = new TextC(this);
                              rights->setSubtype(TEXT_COPYRIGHT);
                              rights->setTextStyle(TEXT_STYLE_COPYRIGHT);
                              rights->read(ee);
                              }
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
                  else if (tag == "showFrames")
                        _showFrames = i;
                  else if (tag == "Symbols")    // obsolete
                        ;
                  else if (tag == "cursorTrack") {
                        if (i >= 0)
                              setInputTrack(i);
                        }
                  else if (tag == "Slur") {
                        Slur* slur = new Slur(this);
                        slur->read(ee);
                        slur->setTrack(-1);     // for backward compatibility
                        slur->setTick(-1);
                        add(slur);
                        }
                  else if (tag == "HairPin") {
                        Hairpin* hairpin = new Hairpin(this);
                        hairpin->setTick(curTick);
                        hairpin->read(ee);
                        add(hairpin);
                        }
                  else if (tag == "Ottava") {
                        Ottava* ottava = new Ottava(this);
                        ottava->setTick(curTick);
                        ottava->read(ee);
                        add(ottava);
                        }
                  else if (tag == "TextLine") {
                        TextLine* textLine = new TextLine(this);
                        textLine->setTick(curTick);
                        textLine->read(ee);
                        add(textLine);
                        }
                  else if (tag == "Volta") {
                        Volta* volta = new Volta(this);
                        volta->setTick(curTick);
                        volta->read(ee);
                        add(volta);
                        }
                  else if (tag == "Trill") {
                        Trill* trill = new Trill(this);
                        trill->setTick(curTick);
                        trill->read(ee);
                        add(trill);
                        }
                  else if (tag == "Pedal") {
                        Pedal* pedal = new Pedal(this);
                        pedal->setTick(curTick);
                        pedal->read(ee);
                        add(pedal);
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
                        _beams.append(beam);
                        }
                  else
                        domError(ee);
                  }
            }
      if (_mscVersion < 108)
            connectSlurs();

      connectTies();
      setInstrumentNames();

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
//DEBUG
//      _repeatList->unwind();
//      _repeatList->dump();

      return true;
      }

//---------------------------------------------------------
//   connectSlurs
//    helper routine for old msc versions
//    and MusicXml and Capella import
//---------------------------------------------------------

void Score::connectSlurs()
      {
      foreach (Element* e, _gel) {
            if (e->type() != SLUR)
                  continue;
            Slur* s = static_cast<Slur*>(e);
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
                  if (n1 == 0)
                        printf("  start at %d(%d) not found\n", s->tick(), s->track());
                  if (n2 == 0)
                        printf("  end at %d(%d) not found\n", s->tick2(), s->track2());
                  if ((n1 || n2) && (n1==n2))
                        printf("  start == end\n");
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
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Tuplet* t, *m->tuplets()) {
                  if (t->elements().empty()) {
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
      QPrinter* printerDev = static_cast<QPrinter*>(pdev);

      if (paperSizes[pageFormat()->size].qtsize == QPrinter::Custom) {
            printerDev->setPaperSize(QSizeF(pageFormat()->_width, pageFormat()->_height),
               QPrinter::Inch);
            }
      else
            printerDev->setPaperSize(paperSizes[pageFormat()->size].qtsize);

      printerDev->setOrientation(pageFormat()->landscape ? QPrinter::Landscape : QPrinter::Portrait);
      printerDev->setCreator("MuseScore Version: " VERSION);
      printerDev->setFullPage(true);
      printerDev->setColorMode(QPrinter::Color);

      printerDev->setDocName(name());
      printerDev->setDoubleSidedPrinting(pageFormat()->twosided);
      printerDev->setOutputFormat(QPrinter::NativeFormat);

#if defined(Q_WS_MAC) || defined(__MINGW32__)
      printerDev->setOutputFileName("");
#else
      // when setting this on windows platform, pd.exec() does not
      // show dialog
      printerDev->setOutputFileName(info.path() + "/" + name() + ".pdf");
#endif

      QPrintDialog pd(printerDev, 0);
      if (!pd.exec())
            return;
      print(printerDev);
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

      for (int copy = 0; copy < printer->numCopies(); ++copy) {
            const QList<Page*> pl = pages();
            int pages = pl.size();

            int fromPage = printer->fromPage() - 1;
            int toPage   = printer->toPage() - 1;
            if (fromPage < 0)
                  fromPage = 0;
            if ((toPage < 0) || (toPage >= pages))
                  toPage = pages - 1;

            bool firstPage = true;
            for (int n = fromPage; n <= toPage; ++n) {
                  if (!firstPage)
                        printer->newPage();
                  firstPage = false;
                  Page* page = pl.at(n);

                  QRectF fr = page->abbox();
                  QList<const Element*> ell = items(fr);
                  qStableSort(ell.begin(), ell.end(), elementLessThan);
                  
                  foreach(const Element* e, ell) {
                        e->itemDiscovered = 0;
                        if (!e->visible())
                              continue;
                        QPointF ap(e->canvasPos() - page->pos());
                        p.translate(ap);
                        p.setPen(QPen(e->color()));
                        e->draw(p);
                        p.translate(-ap);
                        }
                  }
            if ((copy + 1) < printer->numCopies())
                  printer->newPage();
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
      if (paperSizes[pageFormat()->size].qtsize == QPrinter::Custom) {
            p.setPaperSize(QSizeF(pageFormat()->_width, pageFormat()->_height),
               QPrinter::Inch);
            }
      else
            p.setPaperSize(paperSizes[pageFormat()->size].qtsize);

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
      QSvgGenerator printer;
      QPaintDevice* opdev = pdev;
      pdev = &printer;

      printer.setResolution(int(DPI));
      printer.setFileName(saveName);

      _printing = true;

      QPainter p(pdev);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      double mag = converterDpi / DPI;
      p.scale(mag, mag);

      QList<Element*> eel;
      foreach (Element* element, _gel)
            element->scanElements(&eel, collectElements);
      foreach (Beam* b, _beams)
            b->scanElements(&eel, collectElements);
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
            }

      _printing = false;
      p.end();
      pdev = opdev;
      return true;
      }



//---------------------------------------------------------
//   savePng
//    return true on success
//---------------------------------------------------------

bool Score::savePng(const QString& name)
      {
      return savePng(name, preferences.pngScreenShot, true, converterDpi, QImage::Format_ARGB32_Premultiplied );
      }

//---------------------------------------------------------
//   savePng with options
//    return true on success
//---------------------------------------------------------

bool Score::savePng(const QString& name, bool screenshot, bool transparent, double convDpi, QImage::Format format)
      {
      _printing = !screenshot;             // dont print page break symbols etc.

      bool rv = true;

      QImage::Format f;
      if (format != QImage::Format_Indexed8)
          f = format;
      else
          f = QImage::Format_ARGB32_Premultiplied;

      const QList<Page*>& pl = pages();
      int pages = pl.size();

      QList<Element*> eel;
      foreach (Element* element, _gel)
            element->scanElements(&eel, collectElements);
      foreach (Beam* b, _beams)
            b->scanElements(&eel, collectElements);
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

            QList<Element*> el;
            page->scanElements(&el, collectElements);
            foreach(const Element* e, el) {
                  if (!e->visible())
                        continue;
                  QPointF ap(e->canvasPos() - page->pos());
                  p.translate(ap);
                  p.setPen(QPen(e->color()));
                  e->draw(p);
                  p.translate(-ap);
                  }

            if( format == QImage::Format_Indexed8){
              //convert to grayscale & respect alpha
              QVector<QRgb> colorTable;
              colorTable.push_back(QColor(0, 0, 0, 0).rgba());
              if(!transparent){
                for (int i = 1; i < 256; i++)
                  colorTable.push_back(QColor(i, i, i).rgb());
              }else{
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
      _printing = false;
      return rv;
      }

