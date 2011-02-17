//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer et al.
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

#include "config.h"
#include "globals.h"
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
#include "stafftype.h"
#include "seq.h"
#include "revisions.h"
#include "lyrics.h"
#include "segment.h"
#include "tempotext.h"
#include "sym.h"
#include "painter.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif

#include "diff/diff_match_patch.h"

//---------------------------------------------------------
//   readScoreError
//---------------------------------------------------------

void MuseScore::readScoreError(int rv) const
      {
      if (rv == 1) {
            QMessageBox::critical(0,
               tr("MuseScore: Load error"),
               QString(tr("Cannot read file: file: %1").arg(strerror(errno)))
               );
            }
      else {
            QMessageBox::critical(0,
               tr("MuseScore: Load error"),
               tr("unsupported file extension")
               );
            }
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
      QString fn = getOpenScoreName(
         lastOpenPath,
#ifdef OMR
         tr("All Supported Files (*.mscz *.mscx *.msc *.xml *.mxl *.mid *.midi *.kar *.md *.mgu *.MGU *.sgu *.SGU *.cap *.pdf *.ove *.bww *.GTP *.GP3 *.GP4);;")+
#else
         tr("All Supported Files (*.mscz *.mscx *.msc *.xml *.mxl *.mid *.midi *.kar *.md *.mgu *.MGU *.sgu *.SGU *.cap *.ove *.bww *.GTP *.GP3 *.GP4);;")+
#endif
         tr("MuseScore Files (*.mscz *.mscx *.msc);;")+
         tr("MusicXML Files (*.xml *.mxl);;")+
         tr("MIDI Files (*.mid *.midi *.kar);;")+
         tr("Muse Data Files (*.md);;")+
         tr("Capella Files (*.cap);;")+
         tr("BB Files <experimental> (*.mgu *.MGU *.sgu *.SGU);;")+
#ifdef OMR
         tr("PDF Files <experimental omr> (*.pdf);;")+
#endif
         tr("Overture Files <experimental> (*.ove);;")+
         tr("Bagpipe Music Writer Files <experimental> (*.bww);;")+
         tr("Guitar Pro (*.GTP *.GP3 *.GP4 *.GP5);;")+
         tr("All Files (*)")
         );
      if (fn.isEmpty())
            return;
      Score* score = new Score(_defaultStyle);
      int rv = score->readScore(fn);
      if (rv == 0) {
            setCurrentScoreView(appendScore(score));
            lastOpenPath = score->fileInfo()->path();
            writeSessionFile(false);
            }
      else {
            readScoreError(rv);
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
      if (cs == 0)
            return;
      cs->setSyntiState(synti->state());
      if (cs->saveFile(false)) {
            setWindowTitle("MuseScore: " + cs->name());
            int idx = scoreList.indexOf(cs);
            tab1->setTabText(idx, cs->name());
            if (tab2)
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

//---------------------------------------------------------
//   createDefaultFileName
//---------------------------------------------------------

QString Score::createDefaultFileName()
      {
      QString fn = info.baseName();
      Text* t = getText(TEXT_TITLE);
      if (t)
            fn = t->getText();
      //
      // special characters in filenames are a constant source
      // of trouble, this replaces some of them common in german:
      //
      fn = fn.replace(QChar(' '),  "_");
      fn = fn.replace(QChar('\n'), "_");
      fn = fn.replace(QChar(0xe4), "ae");
      fn = fn.replace(QChar(0xf6), "oe");
      fn = fn.replace(QChar(0xfc), "ue");
      fn = fn.replace(QChar(0xdf), "ss");
      fn = fn.replace(QChar(0xc4), "Ae");
      fn = fn.replace(QChar(0xd6), "Oe");
      fn = fn.replace(QChar(0xdc), "Ue");
      return fn;
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
      if (created()) {
            QString selectedFilter;
            QString name = createDefaultFileName();
            QString f1 = tr("Compressed MuseScore File (*.mscz)");
            QString f2 = tr("MuseScore File (*.mscx)");

            QString fname   = QString("%1.mscz").arg(name);
            QString filter = f1 + ";;" + f2;
            QString fn     = mscore->getSaveScoreName(
               tr("MuseScore: Save Score"),
               fname,
               filter,
               &selectedFilter
               );
            if (fn.isEmpty())
                  return false;
            info.setFile(fn);
            mscore->updateRecentScores(this);
            setCreated(false);
            mscore->writeSessionFile(false);
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

      undo()->setClean();
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

      if (saveDirectory.isEmpty()) {
            saveDirectory = preferences.workingDirectory;
            }

      QString selectedFilter;
      QString name    = QString("%1.mscz").arg(info.baseName());
      QString filter = fl.join(";;");
      QString fn = mscore->getSaveScoreName(
         saveDialogTitle, name, filter, &selectedFilter);
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
                  undo()->setClean();
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
      int pickupTimesigZ, pickupTimesigN;
      int measures       = newWizard->measures();
      Fraction timesig   = newWizard->timesig();
      bool pickupMeasure = newWizard->pickupMeasure(&pickupTimesigZ, &pickupTimesigN);
      if (pickupMeasure)
            measures += 1;
      KeySigEvent ks     = newWizard->keysig();

      Score* score = new Score(_defaultStyle);
      score->setCreated(true);

      //
      //  create score from template
      //
      if (newWizard->useTemplate()) {
            int rv = score->readScore(newWizard->templatePath());
            if (rv != 0) {
                  readScoreError(rv);
#if 0
                  QMessageBox::warning(0,
                     tr("MuseScore: failure"),
                     tr("Load template file ") + newWizard->templatePath() + tr(" failed"),
                     QString::null, QString::null, QString::null, 0, 1);
#endif
                  delete score;
                  return;
                  }
            score->fileInfo()->setFile(createDefaultName());

            int m = 0;
            for (Measure* mb = score->firstMeasure(); mb; mb = mb->nextMeasure()) {
                  if (mb->type() == MEASURE)
                        ++m;
                  }
            //
            // remove all notes & rests
            //
            score->deselectAll();
            for (Segment* s = score->firstMeasure()->first(); s;) {
                  Segment* ns = s->next1();
                  if (s->subtype() == SegChordRest && s->tick() == 0) {
                        int tracks = s->elist().size();
                        for (int track = 0; track < tracks; ++track) {
                              delete s->element(track);
                              s->setElement(track, 0);
                              }
                        }
                  else if (
                     (s->subtype() == SegChordRest)
                     || (s->subtype() == SegClef)
                     || (s->subtype() == SegKeySig)
                     || (s->subtype() == SegGrace)
                     || (s->subtype() == SegBreath)
                     ) {
                        s->measure()->remove(s);
                        delete s;
                        }
                  s = ns;
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
      Measure* pm = score->firstMeasure();
      for (int i = 0; i < measures; ++i) {
            Measure* m;
            if (pm) {
                  m  = pm;
                  pm = pm->nextMeasure();
                  }
            else {
                  m = new Measure(score);
                  score->measures()->add(m);
                  }
            m->setTimesig(timesig);
            m->setLen(timesig);
            if (pickupMeasure) {
                  if (i == 0) {
                        m->setIrregular(true);        // dont count pickup measure
                        m->setLen(Fraction(pickupTimesigZ, pickupTimesigN));
                        }
                  else if (i == (measures - 1)) {
                        // last measure is shorter
                        m->setLen(timesig - Fraction(pickupTimesigZ, pickupTimesigN));
                        }
                  }
            if (i == (measures - 1))
                  m->setEndBarLineType(END_BAR, false);
            else
                  m->setEndBarLineType(NORMAL_BAR, true);
            }

      int tick = 0;
      for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
            mb->setTick(tick);
            if (mb->type() != MEASURE)
                  continue;
            Measure* measure = static_cast<Measure*>(mb);
            int ticks = measure->ticks();
	      for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  Staff* staff = score->staff(staffIdx);
                  if (tick == 0) {
                        if (!staff->useTablature()) {
                              TimeSig* ts = new TimeSig(score, timesig);
                              ts->setTrack(staffIdx * VOICES);
                              Segment* s = measure->getSegment(ts, 0);
                              s->add(ts);
                              }
                        Part* part = staff->part();
                        if (!part->instr()->useDrumset()) {
                              //
                              // transpose key
                              //
                              KeySigEvent nKey = ks;
                              if (part->instr()->transpose().chromatic && !newWizard->useTemplate()) {
                                    int diff = -part->instr()->transpose().chromatic;
                                    nKey.setAccidentalType(transposeKey(nKey.accidentalType(), diff));
                                    }
                              if (nKey.accidentalType()) {
                                    (*(staff->keymap()))[0] = nKey;
                                    KeySig* keysig = new KeySig(score);
                                    keysig->setTrack(staffIdx * VOICES);
                                    keysig->setKeySigEvent(nKey);
                                    Segment* s = measure->getSegment(keysig, 0);
                                    s->add(keysig);
                                    }
                              }
                        Clef* clef = new Clef(score);
                        clef->setClefType(staff->clef(0));
                        clef->setTrack(staffIdx * VOICES);
                        Segment* segment = measure->getSegment(SegClef, 0);
                        segment->add(clef);
                        }
                  if (staff->primaryStaff()) {
                        if (measure->timesig() != measure->len()) {
                              int tick = measure->tick();
                              QList<Duration> dList = toDurationList(measure->len(), false);
                              if (!dList.isEmpty()) {
                                    foreach(Duration d, dList) {
		                              Rest* rest = new Rest(score, d);
                                          rest->setDuration(d.fraction());
            	                        rest->setTrack(staffIdx * VOICES);
	            	                  Segment* s = measure->getSegment(rest, tick);
		                              s->add(rest);
                                          tick += rest->ticks();
                                          }
                                    }
                              }
                        else {
		                  Rest* rest = new Rest(score, Duration(Duration::V_MEASURE));
                              rest->setDuration(measure->len());
            	            rest->setTrack(staffIdx * VOICES);
	            	      Segment* s = measure->getSegment(rest, tick);
		                  s->add(rest);
                              }
                        }
                  }
            tick += ticks;
            }
      score->fixTicks();
      //
      // ceate linked staves
      //
      foreach(Staff* staff, score->staves()) {
            if (!staff->linkedStaves())
                  continue;
            foreach(Staff* lstaff, staff->linkedStaves()->staves()) {
                  if (staff != lstaff) {
                        cloneStaff(staff, lstaff);
                        }
                  }
            }
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
                  score->setMetaTag("workTitle", title);
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
      if (newWizard->createTempo()) {
            double tempo = newWizard->tempo();
            TempoText* tt = new TempoText(score);

            QString s = symbols[0][note4Sym].toString();
            tt->setText(QString("%1 = %2").arg(s).arg(tempo));
            tt->setTempo(tempo/60.0);
            tt->setTrack(0);
            Segment* seg = score->firstMeasure()->first(SegChordRest);
            seg->add(tt);
            score->tempomap()->changeTempo(0, tempo);
            }
      if (!copyright.isEmpty())
            score->setMetaTag("copyright", copyright);

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
#ifdef OMR
      //
      // save OMR page images
      //
      if (_omr) {
            int n = _omr->numPages();
            for (int i = 0; i < n; ++i) {
                  QString path = QString("OmrPages/page%1.png").arg(i+1);
                  QBuffer cbuf;
                  OmrPage* page = _omr->page(i);
                  QImage image = page->image();
                  if (!image.save(&cbuf, "PNG"))
                        throw(QString("cannot create image"));
                  if (!cbuf.open(QIODevice::ReadOnly))
                        throw(QString("cannot open buffer cbuf"));
                  ec = uz.createEntry(path, cbuf, dt);
                  if (ec != Zip::Ok)
                        throw(QString("Cannot add <%1> to zipfile\n").arg(path));
                  cbuf.close();
                  }
            }
#endif

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
      QString fn = mscore->getStyleFilename(true);
      if (fn.isEmpty())
            return;

      QFile f(fn);
      if (f.open(QIODevice::ReadOnly)) {
            Style st(*mscore->defaultStyle());
            if (st.load(&f)) {
                  _undo->push(new ChangeStyle(this, st));
                  return;
                  }
            }
      else {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load Style failed:"),
               QString(strerror(errno)),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            }
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void Score::saveStyle()
      {
      QString name = mscore->getStyleFilename(false);
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
                  ec = uz.extractFile(path, &dbuf);
                  if (ec != UnZip::Ok) {
                        printf("Cannot read <%s> from zipfile\n", qPrintable(path));
                        }
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
      return read1(doc.documentElement());
      }

//---------------------------------------------------------
//   read1
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
                        QMessageBox::critical(0, tr("MuseScore"),
                           QT_TRANSLATE_NOOP("score", "Cannot read this score:\n"
                           "your version of MuseScore is too old.")
                           );
                        return false;
                        }
                  if (_mscVersion < 117) {
                        bool rv = read(e);
                        if (rv)
                              mscore->updateRecentScores(this);
                        return rv;
                        }
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "programVersion") {
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
                                          else
                                                printf("1cannot parse <%s>\n", qPrintable(val));
                                          }
                                    }
                              else
                                    printf("2cannot parse <%s>\n", VERSION);
                              }
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
                        st = ost->clone();
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
                  if (ost) {            // if there is already a stafftype
                        if (ost->modified())  // if it belongs to Score()
                              delete ost;
                        _staffTypes[idx] = st;
                        }
                  else
                        _staffTypes.append(st);
                  st->setModified(true);
                  }
            else if (tag == "siglist")
                  _sigmap->read(ee, _fileDivision);
            else if (tag == "tempolist")        // obsolete
                  ;           // tempomap()->read(ee, _fileDivision);
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
                              hasSoundfont = true;
                              break;
                              }
                        }
                  if (!hasSoundfont)
                        _syntiState.append(SyntiParameter("soundfont", preferences.soundFont));
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
                  pageFormat()->read(ee);
            else if (tag == "copyright" || tag == "rights")
                  setMetaTag("copyright", val);
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
                  _beams.append(beam);
                  }
            else if (tag == "Score") {          // recursion
                  Score* s = new Score(style());
                  s->setParentScore(this);
                  s->read(ee);
                  addExcerpt(s);
                  }
            else if (tag == "name")
                  setName(val);
            else
                  domError(ee);
            }

      if (_mscVersion < 108)
            connectSlurs();

      if (_mscVersion < 115) {
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  Staff* s = _staves[staffIdx];
                  int track = staffIdx * VOICES;

                  ClefList* cl = s->clefList();
                  for (ciClefEvent i = cl->begin(); i != cl->end(); ++i) {
                        int tick = i->first;
                        ClefType clefId = i->second._concertClef;
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
                        Segment* s1 = tick2segment(curTick);
                        Segment* s2 = tick2segment(s->__tick2());
                        if (s1 == 0 || s2 == 0) {
                              printf("cannot place %s at tick %d - %d\n",
                                 s->name(), s->__tick1(), s->__tick2());
                              }
                        else {
                              s->setStartElement(s1);
                              s->setEndElement(s2);
                              s1->add(s);
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
            }
      slurs.clear();
      connectTies();
//      setInstrumentNames();

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
                  nscore->setParentScore(this);
                  nscore->setName(excerpt->title());
                  nscore->rebuildMidiMapping();
                  nscore->updateChannel();
                  nscore->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
                  nscore->doLayout();
                  excerpt->setScore(nscore);
                  }
            }
      renumberMeasures();
      checkScore();
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
//   printFile
//---------------------------------------------------------

void Score::printFile()
      {
      QPrinter printerDev(QPrinter::HighResolution);

      if (paperSizes[pageFormat()->size].qtsize >= int(QPrinter::Custom)) {
            printerDev.setPaperSize(QSizeF(pageFormat()->_width, pageFormat()->_height),
               QPrinter::Inch);
            }
      else
            printerDev.setPaperSize(QPrinter::PageSize(paperSizes[pageFormat()->size].qtsize));

      printerDev.setOrientation(pageFormat()->landscape ? QPrinter::Landscape : QPrinter::Portrait);
      printerDev.setCreator("MuseScore Version: " VERSION);
      printerDev.setFullPage(true);
      printerDev.setColorMode(QPrinter::Color);

      printerDev.setDocName(name());
      printerDev.setDoubleSidedPrinting(pageFormat()->twosided);
      printerDev.setOutputFormat(QPrinter::NativeFormat);

#if defined(Q_WS_MAC) || defined(__MINGW32__)
      printerDev.setOutputFileName("");
#else
      // when setting this on windows platform, pd.exec() does not
      // show dialog
      printerDev.setOutputFileName(info.path() + "/" + name() + ".pdf");
#endif

      QPrintDialog pd(&printerDev, 0);
      if (!pd.exec())
            return;
      print(&printerDev);
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void Score::print(QPrinter* printer)
      {
      _printing = true;
      QPainter p(printer);
      Painter painter(&p, 0);

      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      double mag = printer->logicalDpiX() / DPI;
      p.scale(mag, mag);

      for (int copy = 0; copy < printer->numCopies(); ++copy) {
            const QList<Page*> pl = pages();
            int pages = pl.size();

            int offset = pageFormat()->pageOffset();
            int fromPage = printer->fromPage() - 1 - offset;
            int toPage   = printer->toPage() - 1 - offset;
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
                  QList<const Element*> ell = page->items(fr);
                  qStableSort(ell.begin(), ell.end(), elementLessThan);
                  foreach(const Element* e, ell) {
                        e->itemDiscovered = 0;
                        if (!e->visible())
                              continue;
                        p.save();
                        p.translate(e->canvasPos() - page->pos());
                        p.setPen(QPen(e->color()));
                        e->draw(&painter);
                        p.restore();
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
      if (paperSizes[pageFormat()->size].qtsize >= int(QPrinter::Custom)) {
            p.setPaperSize(QSizeF(pageFormat()->_width, pageFormat()->_height),
               QPrinter::Inch);
            }
      else
            p.setPaperSize(QPrinter::PageSize(paperSizes[pageFormat()->size].qtsize));

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
      printer.setResolution(int(DPI));
      printer.setFileName(saveName);

      _printing = true;

      QPainter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      double mag = converterDpi / DPI;
      p.scale(mag, mag);
      Painter painter(&p, 0);

      QList<Element*> eel;
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
      return true;
      }

//---------------------------------------------------------
//   savePng
//    return true on success
//---------------------------------------------------------

bool Score::savePng(const QString& name)
      {
      return savePng(name, false, true, converterDpi, QImage::Format_ARGB32_Premultiplied );
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
            Painter painter(&p, 0);

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
      _printing = false;
      return rv;
      }

//---------------------------------------------------------
//   readCompressedToBuffer
//---------------------------------------------------------

QByteArray Score::readCompressedToBuffer()
      {
      QBuffer cbuf;
      UnZip uz;
      UnZip::ErrorCode ec = uz.openArchive(filePath());
      if (ec != UnZip::Ok)
            return QByteArray();

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
            ec = uz.extractFile(ip->path(), &dbuf);
            if (ec != UnZip::Ok) {
                  printf("Cannot read <%s> from zipfile\n", qPrintable(ip->path()));
                  }
            else
                  ip->setLoaded(true);
            }

      if (rootfile.isEmpty()) {
            printf("can't find rootfile in: %s\n", qPrintable(filePath()));
            return QByteArray();
            }

      QBuffer dbuf;
      dbuf.open(QIODevice::WriteOnly);
      ec = uz.extractFile(rootfile, &dbuf);
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
            if (!f.open(QIODevice::ReadOnly))
                  return false;
            ba = f.readAll();
            f.close();
            }
      return ba;
      }

//---------------------------------------------------------
//   createRevision
//---------------------------------------------------------

void Score::createRevision()
      {
printf("createRevision\n");
      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      saveFile(&dbuf, false);
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
                  //
                  // write new slurs for all voices
                  // (this allows for slurs crossing voices)
                  //
                  if (((track % VOICES) == 0)
                     && (segment->subtype() & (SegChordRest | SegGrace))) {
                        for (int i = 0; i < VOICES; ++i) {
                              Element* e = segment->element(track + i);
                              if (e) {
                                    ChordRest* cr = static_cast<ChordRest*>(e);
                                    foreach(Slur* slur, cr->slurFor()) {
                                          slur->setId(xml.slurId++);
                                          slur->write(xml);
                                          }
                                    }
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
                              if (beam && beam->elements().front() == cr)
                                    beam->write(xml);
                              Tuplet* tuplet = cr->tuplet();
                              if (tuplet && tuplet->elements().front() == cr) {
                                    tuplet->setId(xml.tupletId++);
                                    tuplet->write(xml);
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

//---------------------------------------------------------
//   addAudioTrack
//---------------------------------------------------------

void Score::addAudioTrack()
      {
      // TODO
      }

#if NATIVE_FILEDIALOG

//---------------------------------------------------------
//   getSoundFont
//---------------------------------------------------------

QString MuseScore::getSoundFont(const QString& d)
      {
      QString s = QFileDialog::getOpenFileName(
         mscore,
         MuseScore::tr("Choose Synthesizer SoundFont"),
         d,
         MuseScore::tr("SoundFont Files (*.sf2 *.SF2);;All (*)")
         );
      return s;
      }

//---------------------------------------------------------
//   getOpenScoreName
//---------------------------------------------------------

QString MuseScore::getOpenScoreName(QString& dir, const QString& filter)
      {
      return QFileDialog::getOpenFileName(this,
         tr("MuseScore: Load Score"), dir, filter);
      }

//---------------------------------------------------------
//   getSaveScoreName
//---------------------------------------------------------

QString MuseScore::getSaveScoreName(const QString& title,
   QString& name, const QString& filter, QString* selectedFilter)
      {
      QString selectedFilter;
      QString fn = QFileDialog::getSaveFileName(this,
               title,
               name,
               filter,
               selectedFilter
               );
      return fn;
      }

//---------------------------------------------------------
//   getStyleFilename
//---------------------------------------------------------

QString MuseScore::getStyleFilename(bool open)
      {
      QString fn;
      if (open) {
            fn = QFileDialog::getOpenFileName(
               this, tr("MuseScore: Load Style"),
               QString("."),
               tr("MuseScore Styles (*.mss);;" "All Files (*)")
               );
            }
      else {
            fn = QFileDialog::getSaveFileName(
               this, tr("MuseScore: Save Style"),
               QString("."),
               tr("MuseScore Style File (*.mss)")
               );
            }
      return fn;
      }

#else // NATIVE_FILEDIALOG

//---------------------------------------------------------
//   getOpenFileName
//---------------------------------------------------------

QString MuseScore::getOpenScoreName(QString& dir, const QString& filter)
      {
      QFileInfo myScores(preferences.myScoresPath);
      if (myScores.isRelative())
            myScores.setFile(QDir::home(), preferences.myScoresPath);
      if (loadScoreDialog == 0) {
            loadScoreDialog = new QFileDialog(this);
            loadScoreDialog->setFileMode(QFileDialog::ExistingFile);
            loadScoreDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadScoreDialog->setWindowTitle(tr("MuseScore: Load Score"));

            // setup side bar urls
            QList<QUrl> urls;
            QString home = QDir::homePath();
            urls.append(QUrl::fromLocalFile(home));
            urls.append(QUrl::fromLocalFile(myScores.absoluteFilePath()));
            urls.append(QUrl::fromLocalFile(QDir::currentPath()));
            urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/demos"));
            loadScoreDialog->setSidebarUrls(urls);
            QSettings settings;
            loadScoreDialog->restoreState(settings.value("loadScoreDialog").toByteArray());
            }
      loadScoreDialog->setNameFilter(filter);
      loadScoreDialog->setDirectory(dir);

      QStringList result;
      if (loadScoreDialog->exec()) {
            result = loadScoreDialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getSaveScoreName
//---------------------------------------------------------

QString MuseScore::getSaveScoreName(const QString& title,
   QString& name, const QString& filter, QString* selectedFilter)
      {
      QFileInfo myScores(preferences.myScoresPath);
      if (myScores.isRelative())
            myScores.setFile(QDir::home(), preferences.myScoresPath);
      if (saveScoreDialog == 0) {
            saveScoreDialog = new QFileDialog(this);
            saveScoreDialog->setFileMode(QFileDialog::AnyFile);
            saveScoreDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
            saveScoreDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            saveScoreDialog->setLabelText(QFileDialog::Accept, tr("Save"));
            // setup side bar urls
            QList<QUrl> urls;
            QString home = QDir::homePath();
            urls.append(QUrl::fromLocalFile(home));
            urls.append(QUrl::fromLocalFile(myScores.absoluteFilePath()));
            urls.append(QUrl::fromLocalFile(QDir::currentPath()));
            saveScoreDialog->setSidebarUrls(urls);
            QSettings settings;
            saveScoreDialog->restoreState(settings.value("saveScoreDialog").toByteArray());
            }
      saveScoreDialog->setWindowTitle(title);
      saveScoreDialog->setNameFilter(filter);
      // saveScoreDialog->setDirectory(name);
      saveScoreDialog->selectFile(name);
      QStringList result;
      if (saveScoreDialog->exec()) {
            result = saveScoreDialog->selectedFiles();
            *selectedFilter = saveScoreDialog->selectedNameFilter();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getStyleFilename
//---------------------------------------------------------

QString MuseScore::getStyleFilename(bool open)
      {
      QFileInfo myStyles(preferences.myStylesPath);
      if (myStyles.isRelative())
            myStyles.setFile(QDir::home(), preferences.myStylesPath);
      QFileDialog* dialog;
      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(myStyles.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));

      if (open) {
            if (loadStyleDialog == 0) {
                  loadStyleDialog = new QFileDialog(this);
                  loadStyleDialog->setFileMode(QFileDialog::ExistingFile);
                  loadStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  loadStyleDialog->setWindowTitle(tr("MuseScore: Load Style"));
                  loadStyleDialog->setNameFilter(tr("MuseScore Style File (*.mss)"));
                  loadStyleDialog->setDirectory(".");

                  // setup side bar urls
                  urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/styles"));
                  loadStyleDialog->setSidebarUrls(urls);
                  QSettings settings;
                  loadStyleDialog->restoreState(settings.value("loadStyleDialog").toByteArray());
                  }
            dialog = loadStyleDialog;
            }
      else {
            if (saveStyleDialog == 0) {
                  saveStyleDialog = new QFileDialog(this);
                  saveStyleDialog->setFileMode(QFileDialog::AnyFile);
                  saveStyleDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
                  saveStyleDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                  saveStyleDialog->setLabelText(QFileDialog::Accept, tr("Save"));
                  saveStyleDialog->setWindowTitle(tr("MuseScore: Save Style"));
                  saveStyleDialog->setNameFilter(tr("MuseScore Style File (*.mss)"));
                  saveStyleDialog->setDirectory(".");

                  // setup side bar urls
                  saveStyleDialog->setSidebarUrls(urls);
                  QSettings settings;
                  saveStyleDialog->restoreState(settings.value("saveStyleDialog").toByteArray());
                  }
            dialog = saveStyleDialog;
            }
      if (dialog->exec()) {
            QStringList result = dialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

//---------------------------------------------------------
//   getSoundFont
//---------------------------------------------------------

QString MuseScore::getSoundFont(const QString& d)
      {
      if (loadSoundFontDialog == 0) {
            loadSoundFontDialog = new QFileDialog(this);
            loadSoundFontDialog->setFileMode(QFileDialog::ExistingFile);
            loadSoundFontDialog->setOption(QFileDialog::DontUseNativeDialog, true);
            loadSoundFontDialog->setWindowTitle(tr("MuseScore: Choose Synthesizer SoundFont"));
            loadSoundFontDialog->setNameFilter(tr("SoundFont Files (*.sf2 *.SF2);;All (*)"));
            loadSoundFontDialog->setDirectory(d);

            QSettings settings;
            loadSoundFontDialog->restoreState(settings.value("mySoundFonts").toByteArray());
            }

      //
      // setup side bar urls
      //
      QFileInfo mySoundFonts(preferences.mySoundFontsPath);
      if (mySoundFonts.isRelative())
            mySoundFonts.setFile(QDir::home(), preferences.mySoundFontsPath);

      QList<QUrl> urls;
      QString home = QDir::homePath();
      urls.append(QUrl::fromLocalFile(home));
      urls.append(QUrl::fromLocalFile(mySoundFonts.absoluteFilePath()));
      urls.append(QUrl::fromLocalFile(QDir::currentPath()));
      urls.append(QUrl::fromLocalFile(mscoreGlobalShare+"/sound"));
      loadSoundFontDialog->setSidebarUrls(urls);

      if (loadSoundFontDialog->exec()) {
            QStringList result = loadSoundFontDialog->selectedFiles();
            return result.front();
            }
      return QString();
      }

#endif  // NATIVE_FILEDIALOG

