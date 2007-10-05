//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: file.cpp,v 1.70 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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
#include "pad.h"
#include "layout.h"
#include "barline.h"
#include "palette.h"
#include "symboldialog.h"
#include "slur.h"

double printerMag = 1.0;

//---------------------------------------------------------
//   readInstrument
//---------------------------------------------------------

static void readInstrument(const QString& group, QDomElement e)
      {
      InstrumentTemplate t;

      t.group       = group;
      t.staves      = 1;
      for (int i = 0; i < MAX_STAVES; ++i) {
            t.clefIdx[i] = 0;
            t.staffLines[i] = 5;
            t.smallStaff[i] = false;
            }
      t.bracket     = -1;
      t.midiProgram = 0;
      t.minPitch    = 0;
      t.maxPitch    = 127;
      t.transpose   = 0;
      t.useDrumset  = false;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "name")
                  t.name = val;
            else if (tag == "short-name")
                  t.shortName = val;
            else if (tag == "staves")
                  t.staves = i;
            else if (tag == "clef") {
                  int idx = e.attribute("staff", "1").toInt() - 1;
                  if (idx >= MAX_STAVES)
                        idx = MAX_STAVES-1;
                  t.clefIdx[idx] = i;
                  }
            else if (tag == "stafflines") {
                  int idx = e.attribute("staff", "1").toInt() - 1;
                  if (idx >= MAX_STAVES)
                        idx = MAX_STAVES-1;
                  t.staffLines[idx] = i;
                  }
            else if (tag == "stafflines") {
                  int idx = e.attribute("staff", "1").toInt() - 1;
                  if (idx >= MAX_STAVES)
                        idx = MAX_STAVES-1;
                  t.smallStaff[idx] = i;
                  }
            else if (tag == "bracket")
                  t.bracket = i;
            else if (tag == "minPitch")
                  t.minPitch = i;
            else if (tag == "maxPitch")
                  t.maxPitch = i;
            else if (tag == "transpose")
                  t.transpose = i;
            else if (tag == "drumset") {
                  t.useDrumset = i;
                  }
            else
                  domError(e);
            }
      instrumentTemplates.push_back(new InstrumentTemplate(t));
      }

//---------------------------------------------------------
//   readInstrumentGroup
//---------------------------------------------------------

static void readInstrumentGroup(QDomElement e)
      {
      QString group = e.attribute("name");

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "instrument")
                  readInstrument(group, e);
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   getFile
//    return true on error
//---------------------------------------------------------

bool LoadFile::load(QWidget* parent, const QString& base, const QString& ext,
   const QString& caption)
      {
      error = "";
      _name = QFileDialog::getOpenFileName(parent, caption, base, ext);

      if (_name.isEmpty())
            return true;
      QFileInfo info(_name);

      if (info.completeSuffix() == QString("")) {
            _name += ext;
            info.setFile(_name);
            }
      return load(_name);
      }

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
            }
      fp.close();
      return false;
      }

//---------------------------------------------------------
//   checkDirty
//    if dirty, save score
//    return true on abort
//---------------------------------------------------------

bool MuseScore::checkDirty(Score* s)
      {
      if (s->dirty()) {
            int n = QMessageBox::warning(this, tr("MuseScore"),
               tr("The current Score contains unsaved data\n"
               "Save Current Score?"),
               tr("&Save"), tr("&Nosave"), tr("&Abort"), 0, 2);
            if (n == 0) {
                  if (s->isSavable())
                        s->saveFile();
                  else {
                        if (!saveAs())
                              return true;
                        }

                  }
            else if (n == 2)
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

void MuseScore::loadFile()
      {
      QString fn = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Load Score"),
         QString("."),
         tr("MuseScore Files (*.msc);;"
            "MusicXml Files (*.xml);;"
            "Standard Midi File Files (*.mid);;"
            "Muse Data Files (*.md);;"
            "Lilypond Files (*.ly);;"
            "All files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      Score* score = new Score();
      score->read(fn);
      appendScore(score);
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveFile()
      {
      bool val = cs->saveFile();
      setWindowTitle("MuseScore: " + cs->projectName());
      tab->setTabText(tab->currentIndex(), cs->projectName());
      return val;
      }

bool Score::saveFile()
      {
      if (created()) {
            QString fn = QFileDialog::getSaveFileName(
               mscore, tr("MuseScore: Save Score"),
               QString("./%1.msc").arg(projectName()),
               QString("*.msc")
               );
            if (fn.isEmpty())
                  return false;
            fileInfo()->setFile(fn);
            setCreated(false);
            }

      // if file was already saved in this session
      // dont overwrite backup again

      if (saved()) {
            bool rv = mscore->saveFile(*fileInfo());
            if (rv)
                  setDirty(false);
            return rv;
            }
      //
      // step 1
      // save into temporary file
      //
      QFileInfo* qf = fileInfo();
      QTemporaryFile temp(qf->path() + "/msXXXXXX");
      temp.setAutoRemove(false);
      if (!temp.open()) {
            QString s = tr("Open Temp File\n") + temp.fileName() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open File"), s);
            return false;
            }
      bool rv = mscore->saveFile(&temp);

      if (!rv)
            return false;

      //
      // step 2
      // remove old backup file if exists
      //
      QDir dir(qf->path());
      QString backupName = QString(".") + qf->fileName() + QString(",");
      dir.remove(backupName);

      //
      // step 3
      // rename old file into backup
      //
      QString name(qf->filePath());
      if (qf->completeSuffix() == "")
            name += QString(".msc");
      dir.rename(name, backupName);

      //
      // step 4
      // rename temp name into file name
      //
      temp.rename(name);
//      temp.close();

      setDirty(false);
      setSaved(true);
      return true;
      }

//---------------------------------------------------------
//   saveAs
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveAs()
      {
      if (!cs)
            return false;
      QString selectedFilter;
      QStringList fl;

      fl.append(tr("MuseScore Format (*.msc)"));
      fl.append(tr("MusicXml Format (*.xml)"));
      fl.append(tr("Standard Midi File (*.mid)"));
      fl.append(tr("PDF File (*.pdf)"));
      fl.append(tr("Postscript File (*.ps)"));
      fl.append(tr("PNG Bitmap Graphic (*.png)"));
      fl.append(tr("Scalable Vector Graphic (*.svg)"));
      fl.append(tr("Lilypond Format (*.ly)"));

      QString fn = QFileDialog::getSaveFileName(
         this, tr("MuseScore: Save As"),
         ".",
         fl.join(";;"),
         &selectedFilter
         );
      if (fn.isEmpty())
            return false;

      if (selectedFilter == fl[0]) {
            // save as mscore *.msc file
            if (!fn.endsWith(".msc"))
                  fn.append(".msc");
            QFileInfo fi(fn);
            return saveFile(fi);
            }
      if (selectedFilter == fl[1]) {
            // save as MusicXML *.xml file
            if (!fn.endsWith(".xml"))
                  fn.append(".xml");
            return cs->saveXml(fn);
            }
      if (selectedFilter == fl[2]) {
            // save as midi file *.mid
            if (!fn.endsWith(".mid"))
                  fn.append(".mid");
            return cs->saveMidi(fn);
            }
      if (selectedFilter == fl[3]) {
            // save as pdf file *.pdf
            if (!fn.endsWith(".pdf"))
                  fn.append(".pdf");
            return cs->savePdf(fn);
            }
      if (selectedFilter == fl[4]) {
            // save as postscript file *.ps
            if (!fn.endsWith(".ps"))
                  fn.append(".ps");
            return cs->savePs(fn);
            }
      if (selectedFilter == fl[5]) {
            // save as png file *.png
            if (!fn.endsWith(".png"))
                  fn.append(".png");
            return cs->savePng(fn);
            }
      if (selectedFilter == fl[6]) {
            // save as svg file *.svg
            if (!fn.endsWith(".svg"))
                  fn.append(".svg");
            return cs->saveSvg(fn);
            }
      if (selectedFilter == fl[7]) {
            // save as lilypond file *.ly
            if (!fn.endsWith(".ly"))
                  fn.append(".ly");
            return cs->saveLilypond(fn);
            }
      return false;
      }

//---------------------------------------------------------
//   createDefaultName
//---------------------------------------------------------

QString MuseScore::createDefaultName() const
      {
      QString name(tr("untitled"));
      int n;
      for (n = 1; ; ++n) {
            bool nameExists = false;
            QString tmpName;
            if (n == 1)
                  tmpName = name;
            else
                  tmpName = QString("%1-%2").arg(name).arg(n);
            foreach(Score* s, scoreList) {
                  if (s->projectName() == tmpName) {
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
//   newFileFromTemplate
//    create new score
//---------------------------------------------------------

void MuseScore::newFileFromTemplate()
      {
      QString path(mscoreGlobalShare);
      path += "/templates";
      QString fn = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Load Template"),
         path,
         "Score templates (*.msc);; Any files (*)"
         );
      Score* score = new Score;
      if (!fn.isEmpty())
            score->read(fn);
      score->fileInfo()->setFile(createDefaultName());
      score->setCreated(true);
      appendScore(score);
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   newFile
//    create new score
//---------------------------------------------------------

void MuseScore::newFile()
      {
      Score* score = new Score;
      score->fileInfo()->setFile(createDefaultName());
      score->setCreated(true);
      appendScore(score);
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveFile(QFileInfo& info)
      {
      QString ext(".msc");

      if (info.completeSuffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile fp(info.filePath());
      if (!fp.open(QIODevice::WriteOnly)) {
            QString s = tr("Open File\n") + info.filePath() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(this, tr("MuseScore: Open File"), s);
            return false;
            }
      bool rv = saveFile(&fp);
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

void MuseScore::loadStyle()
      {
      QString fn = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Load Style"),
         QString("."),
         tr("MuseScore Styles (*.mss);;"
            "All files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      QFile f(fn);
      if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: load Style failed:"),
               QString(strerror(errno)),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            }
      loadStyle(&f);
      }

//---------------------------------------------------------
//   loadStyle
//    return true on error
//---------------------------------------------------------

bool MuseScore::loadStyle(QFile* qf)
      {
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(qf, false, &err, &line, &column)) {
            QString error;
            error.sprintf("error reading style file %s at line %d column %d: %s\n",
               qf->fileName().toLatin1().data(), line, column, err.toLatin1().data());
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: load Style failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return true;
            }

      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  // QString version = e.attribute(QString("version"));
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "Style")
                              cs->style()->loadStyle(ee);
                        else if (tag == "TextStyle") {
                              QString name = ee.attribute("name");
                              TextStyle* s = 0;
                              foreach(TextStyle* ts, cs->textStyles()) {
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

void MuseScore::saveStyle()
      {
      QString name = QFileDialog::getSaveFileName(
         this, tr("MuseScore: Save Style"),
         ".",
         tr("MuseScore style file (*.mss)")
         );
      if (name.isEmpty())
            return;
      QString ext(".mss");
      QFileInfo info(name);

      if (info.completeSuffix().isEmpty())
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
      xml.stag("museScore version=\"1.0\"");
      cs->style()->saveStyle(xml);
      foreach(TextStyle* ts, cs->textStyles())
            ts->write(xml);

      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = QString("Write Style failed: ") + f.errorString();
            QMessageBox::critical(this, tr("MuseScore: Write Style"), s);
            }
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveFile(QFile* f)
      {
      Xml xml(f);
      xml.header();
      xml.stag("museScore version=\"1.0\"");

      xml.tag("Spatium", _spatium / DPMM);
      xml.tag("Division", division);
      xml.tag("Mag",  canvas->mag());
      xml.tag("xoff", canvas->xoffset());
      xml.tag("yoff", canvas->yoffset());

      if (::symbolPalette)
            ::symbolPalette->write(xml, "Symbols");

      cs->write(xml);

      xml.etag();
      if (f->error() != QFile::NoError) {
            QString s = QString("Write File failed: ") + f->errorString();
            QMessageBox::critical(this, tr("MuseScore: Write File"), s);
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   loadFile
//    return true on error
//---------------------------------------------------------

bool Score::loadFile(QFile* qf)
      {
      QDomDocument doc;
      int line, column;
      QString err;
      QXmlSimpleReader reader;
      QXmlInputSource  source(qf);
      if (!doc.setContent(&source, &reader, &err, &line, &column)) {
            QString s;
            s.sprintf("error reading file %s at line %d column %d: %s\n",
               qf->fileName().toLatin1().data(), line, column, err.toLatin1().data());

            QMessageBox::critical(mscore, tr("MuseScore: Read File"), s);
            return true;
            }

      _fileDivision = 384;   // for compatibility with old mscore files

      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
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
                        else if (tag == "xoff")
                              setXoffset(val.toDouble());
                        else if (tag == "yoff")
                              setYoffset(val.toDouble());
                        else if (tag == "Spatium")
                              setSpatium (val.toDouble() * DPMM);
                        else if (tag == "Division")
                              _fileDivision = i;
                        else if (tag == "showInvisible")
                              _showInvisible = i;
                        else if (tag == "Style")
                              _style->loadStyle(ee);
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
                        else if (tag == "instrument-group")
                              readInstrumentGroup(ee);
                        else if (tag == "rights") {
                              if (rights == 0)
                                    rights = new QTextDocument(0);
                              rights->setHtml(val);
                              }
                        else if (tag == "movement-number")
                              movementNumber = val;
                        else if (tag == "movement-title")
                              movementTitle = val;
                        else if (tag == "Part") {
                              Part* part = new Part(this);
                              part->read(this, ee);
                              parts()->push_back(part);
                              }
                        else if (tag == "showInvisible")
                              _showInvisible = i;
                        else if (tag == "Symbols") {
                              if (::symbolPalette == 0)
                                    createSymbolPalette();
                              ::symbolPalette->read(ee);
                              }
                        else if (tag == "cursorTrack")
                              _is.track = i;
                        else if (tag == "cursorStaff")      // obsolete
                              _is.track = i * VOICES;
                        else if (tag == "cursorVoice")      // obsolete
                              _is.track += i;
                        else if (tag == "Slur") {
                              Slur* slur = new Slur(this);
                              slur->setTick(0);
                              slur->setStaff(0);
                              slur->read(ee);
                              _layout->add(slur);
                              }
                        else
                              domError(ee);
                        }
                  }
            }
      _layout->connectTies();
      searchSelectedElements();
      _fileDivision = division;
      return false;
      }

//---------------------------------------------------------
//   loadMsc
//    return true if file not found or error loading
//---------------------------------------------------------

bool Score::loadMsc(QString name)
      {
      QString ext(".msc");

      info.setFile(name);
      if (info.completeSuffix() == "") {
            name += ext;
            info.setFile(name);
            }
      QFile f(name);
      if (!f.open(QIODevice::ReadOnly))
            return true;
      bool rv = loadFile(&f);
      f.close();
      return rv;
      }

//---------------------------------------------------------
//   loadInstrumentTemplates
//---------------------------------------------------------

void MuseScore::loadInstrumentTemplates()
      {
      QString lang(QLocale::system().name().left(2));
      QString instrTemplates = mscoreGlobalShare + "/templates/instruments_" + lang + ".xml";
      QFileInfo info(instrTemplates);
      if (!info.isReadable()) {
            instrTemplates = mscoreGlobalShare + "/templates/instruments.xml";
            info.setFile(instrTemplates);
            if (!info.isReadable()) {
                  instrTemplates = ":/data/instruments.xml";
                  info.setFile(instrTemplates);
                  }
            }
      if (!info.isReadable()) {
            fprintf(stderr, "cannot find instrument templates <%s>\n", instrTemplates.toLatin1().data());
            return;
            }

      QFile qf(instrTemplates);
      if (!qf.open(QIODevice::ReadOnly))
            return;

      QDomDocument doc;
      int line, column;
      QString err;
      bool rv = doc.setContent(&qf, false, &err, &line, &column);
      qf.close();

      if (!rv) {
            QString s;
            s.sprintf("error reading file %s at line %d column %d: %s\n",
               instrTemplates.toLatin1().data(), line, column, err.toLatin1().data());

            QMessageBox::critical(mscore, tr("MuseScore: Read File"), s);
            return;
            }

      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "instrument-group")
                              readInstrumentGroup(ee);
                        else
                              domError(ee);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ElementList::write(Xml& xml) const
      {
      for (ciElement ie = begin(); ie != end(); ++ie)
            (*ie)->write(xml);
      }

//---------------------------------------------------------
//   printFile
//---------------------------------------------------------

void Score::printFile()
      {
      //
      // HighResolution gives higher output quality
      // but layout may be slightly different

      QPrinter printer(QPrinter::HighResolution);
//      QPrinter printer;

      printer.setPageSize(paperSizes[pageFormat()->size].qtsize);
      printer.setOrientation(pageFormat()->landscape ? QPrinter::Landscape : QPrinter::Portrait);
      printer.setCreator("MuseScore Version: " VERSION);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
      printer.setDocName(projectName());
      printer.setDoubleSidedPrinting(pageFormat()->twosided);

      QPrintDialog pd(&printer, 0);
      if (!pd.exec())
            return;
      print(&printer);
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

      qreal oldSpatium = _spatium;
      double oldDPI    = DPI;
      DPI              = printer->logicalDpiX();         // drawing resolution

//HACK:
printerMag = DPI / oldDPI;

      DPMM             = DPI / INCH;                     // dots/mm
      setSpatium(_spatium * DPI / oldDPI);
      QPaintDevice* oldPaintDevice = mainLayout()->paintDevice();
      mainLayout()->setPaintDevice(printer);
      doLayout();

      ElementList el;
      for (ciPage ip = _layout->pages()->begin();;) {
            Page* page = *ip;
            el.clear();
            page->collectElements(el);
            foreach(System* system, *page->systems()) {
                  foreach(Measure* m, system->measures()) {
                        m->collectElements(el);
                        }
                  }
            for (int i = 0; i < el.size(); ++i) {
                  Element* e = el.at(i);
                  if (!e->visible())
                        continue;
                  QPointF ap(e->canvasPos() - page->pos());
                  p.translate(ap);
                  p.setPen(QPen(e->color()));
                  e->draw(p);
                  p.translate(-ap);
                  }
            ++ip;
            if (ip == _layout->pages()->end())
                  break;
            printer->newPage();
            }
      p.end();
      _printing = false;
printerMag = 1.0;
      DPI       = oldDPI;
      DPMM      = DPI / INCH;                     // dots/mm
      setSpatium(oldSpatium);
      mainLayout()->setPaintDevice(oldPaintDevice);
      doLayout();
      }

//---------------------------------------------------------
//   savePdf
//---------------------------------------------------------

bool Score::savePdf(const QString& name)
      {
      //
      // HighResolution gives higher output quality
      // but layout may be slightly different

      QPrinter printer(QPrinter::HighResolution);
      // QPrinter printer;
      printer.setPageSize(paperSizes[pageFormat()->size].qtsize);
      printer.setOrientation(pageFormat()->landscape ? QPrinter::Landscape : QPrinter::Portrait);
      printer.setCreator("MuseScore Version: " VERSION);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
      printer.setDocName(projectName());
      printer.setDoubleSidedPrinting(pageFormat()->twosided);
      printer.setOutputFormat(QPrinter::PdfFormat);
      printer.setOutputFileName(name);

      print(&printer);
      return true;
      }

//---------------------------------------------------------
//   savePs
//---------------------------------------------------------

bool Score::savePs(const QString& name)
      {
      //
      // HighResolution gives higher output quality
      // but layout may be slightly different

      QPrinter printer(QPrinter::HighResolution);
      // QPrinter printer;
      printer.setPageSize(paperSizes[pageFormat()->size].qtsize);
      printer.setOrientation(pageFormat()->landscape ? QPrinter::Landscape : QPrinter::Portrait);
      printer.setCreator("MuseScore Version: " VERSION);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
      printer.setDocName(projectName());
      printer.setDoubleSidedPrinting(pageFormat()->twosided);
      printer.setOutputFormat(QPrinter::PostScriptFormat);
      printer.setOutputFileName(name);

      print(&printer);
      return true;
      }

//---------------------------------------------------------
//   saveSvg
//---------------------------------------------------------

bool Score::saveSvg(const QString& name)
      {
      QRectF r = canvas()->lassoRect();
      double x = r.x();
      double y = r.y();
      double w = r.width();
      double h = r.height();

      QSvgGenerator printer;
      printer.setFileName(name);
      printer.setSize(QSize(lrint(w), lrint(h)));

      _printing = true;
      QPainter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      p.setClipRect(QRect(0, 0, lrint(w), lrint(h)));
      p.setClipping(true);

      QPointF offset(x, y);

      ElementList el;
      foreach(Page* page, *_layout->pages()) {
            el.clear();
            page->collectElements(el);
            foreach(System* system, *page->systems()) {
                  foreach(Measure* m, system->measures()) {
                        m->collectElements(el);
                        }
                  }
            for (int i = 0; i < el.size(); ++i) {
                  Element* e = el.at(i);
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
//---------------------------------------------------------

bool Score::savePng(const QString& name)
      {
      QRectF r = canvas()->matrix().mapRect(canvas()->lassoRect());
      double w = r.width();
      double h = r.height();
#if 0
      // scaling of fonts does not work

      QImage printer(lrint(w), lrint(h), QImage::Format_ARGB32_Premultiplied);
      printer.setDotsPerMeterX(lrint(DPMM * 1000.0));
      printer.setDotsPerMeterY(lrint(DPMM * 1000.0));
#else
      QPixmap printer(lrint(w), lrint(h));
#endif
      printer.fill(QColor(0, 0, 0, 0));

      QPainter p(&printer);
      QPaintDevice* oldPaintDevice = mainLayout()->paintDevice();
      mainLayout()->setPaintDevice(&printer);

      doLayout();

      canvas()->paintLasso(p);
      bool rv = printer.save(name, "png");

      mainLayout()->setPaintDevice(oldPaintDevice);
      doLayout();
      return rv;
      }

