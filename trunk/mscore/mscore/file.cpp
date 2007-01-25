//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: file.cpp,v 1.70 2006/04/12 14:58:10 wschweer Exp $
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
#include "painter.h"
#include "pad.h"
#include "layout.h"
#include "barline.h"

//---------------------------------------------------------
//   readInstrument
//---------------------------------------------------------

static void readInstrument(const QString& group, QDomNode node)
      {
      int clef[MAX_STAVES];
      for (int i = 0; i < MAX_STAVES; ++i)
            clef[i] = 0;
      QString name;
      QString shortName;
      int staves = 1;
      int clefIdx = 0;
      int bracket = -1;

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "name")
                  name = val;
            else if (tag == "short-name")
                  shortName = val;
            else if (tag == "staves")
                  staves = i;
            else if (tag == "clef") {
                  clef[clefIdx] = i;
                  ++clefIdx;
                  if (clefIdx >= MAX_STAVES)
                        clefIdx = MAX_STAVES-1;
                  }
            else if (tag == "bracket")
                  bracket = i;
            else
                  domError(node);
            }

      instrumentTemplates.push_back(InstrumentTemplate(group, name, shortName,
         staves, clef, bracket));
      }

//---------------------------------------------------------
//   readInstrumentGroup
//---------------------------------------------------------

static void readInstrumentGroup(QDomNode node)
      {
      QDomElement e = node.toElement();
      QString group = e.attribute("name");

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "instrument")
                  readInstrument(group, node);
            else
                  domError(node);
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
      QString zip;

      if (info.completeSuffix() == QString("")) {
            _name += ext;
            info.setFile(_name);
            }
      QFile fp(_name);
      if (!fp.open(QIODevice::ReadOnly))
            return true;
      if (loader(&fp)) {
            QMessageBox::warning(parent,
               QWidget::tr("MuseScore: load failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            }
      fp.close();
      return false;
      }

bool LoadFile::load(const QString& name)
      {
      if (name.isEmpty())
            return true;
      QString zip;

      QFileInfo info(name);
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
//   SaveFile
//---------------------------------------------------------

bool SaveFile::save(QWidget* p, const QString& base, const QString& ext,
   const QString& caption)
      {
      parent = p;
      QString pattern("*");
      pattern += ext;
      _name = QFileDialog::getSaveFileName(parent, caption, base, pattern);

      if (_name.isEmpty())
            return true;
      QFileInfo info(_name);

      if (info.completeSuffix() == QString("")) {
            _name += ext;
            info.setFile(_name);
            }
      f.setFileName(_name);
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = QWidget::tr("Open File\n") + _name + QWidget::tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(parent, QWidget::tr("MuseScore: Open File"), s);
            return true;
            }
      saver();
      bool notOk = f.error() != QFile::NoError;
      if (notOk) {
            QString s = QString("Write failed: ") + QString(strerror(errno));
            QMessageBox::critical(parent, QWidget::tr("MuseScore: Write"), s);
            }
      f.close();
      return notOk;
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
            if (n == 0)
                  s->saveFile();
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
         QString("Score files (*.msc);; MusicXml files (*.xml);; All files (*)")
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
      if (projectName().isEmpty() || projectName() == tr("untitled")
         || ((fileInfo()->completeSuffix() != "msc"))) {
            QString fn = QFileDialog::getSaveFileName(
               mscore, tr("MuseScore: Save Score"),
               QString("."),
               QString("*.msc")
               );
            if (fn.isEmpty())
                  return false;
            fileInfo()->setFile(fn);
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
      temp.open();
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
      QString fn = QFileDialog::getSaveFileName(
         this, tr("MuseScore: Save As"),
         QString("."),
         QString("*.msc")
         );
      if (fn.isEmpty())
            return false;
      QFileInfo fi(fn);
      return saveFile(fi);
      }

//---------------------------------------------------------
//   newFile
//    create new score
//---------------------------------------------------------

void MuseScore::newFile()
      {
      QString path(mscoreGlobalShare);
      path += "/templates";
      QString fn = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Load Template"),
         path,
         "Score templates (*.msc);; Any files (*)"
         );
      Score* score;
      if (cs->projectName() == "untitled" && !cs->dirty()) {
            cs->clear();
            score = cs;
            }
      else
            score = new Score;
      if (!fn.isEmpty())
            score->read(fn);
      score->fileInfo()->setFile(QString("untitled"));

      if (score != cs) {
            appendScore(score);
            tab->setCurrentIndex(scoreList.size() - 1);
            }
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
//   SStaff::write
//---------------------------------------------------------

void SStaff::write(Xml& xml) const
      {
      xml.stag("Staff");
      xml.tag("lines", lines);
      Element::writeProperties(xml);
      xml.etag("Staff");
      }

//---------------------------------------------------------
//   SStaff::read
//---------------------------------------------------------

void SStaff::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "lines")
                  lines = e.text().toInt();
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   Symbol::write
//---------------------------------------------------------

void Symbol::write(Xml& xml) const
      {
      xml.stag("Symbol");
      xml.tag("name", symbols[_sym].name());
//      xml.tag("style", _sym->textStyle());
      xml.tag("x", pos().x());
      xml.tag("y", pos().y());
      Element::writeProperties(xml);
      xml.etag("Symbol");
      }

//---------------------------------------------------------
//   Symbol::read
//---------------------------------------------------------

void Symbol::read(QDomNode node)
      {
      QPointF pos;
      int s = -1;

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "name") {
                  for (int i = 0; i < symbols.size(); ++i) {
                        if (val == symbols[i].name()) {
                              s = i;
                              break;
                              }
                        }
                  if (s == -1) {
                        printf("unknown symbol <%s>\n", val.toLocal8Bit().data());
                        s = 0;
                        }
                  }
            else if (Element::readProperties(node))
                  ;
            else if (tag == "x")
                  pos.setX(val.toDouble());
            else if (tag == "y")
                  pos.setY(val.toDouble());
            else
                  domError(node);
            }
      if (s == -1) {
            printf("unknown symbol\n");
            s = 0;
            }
      setPos(pos);
      setSym(s);
      }

//---------------------------------------------------------
//   readGeometry
//---------------------------------------------------------

void Score::readGeometry(QDomNode node)
      {
      QDomElement e = node.toElement();
      scorePos.setX(e.attribute("x", "0").toInt());
      scorePos.setY(e.attribute("y", "0").toInt());
      scoreSize.setWidth(e.attribute("w", "0").toInt());
      scoreSize.setHeight(e.attribute("h", "0").toInt());
      }

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

class LoadStyle : public LoadFile {
   public:
      virtual bool loader(QFile* f);
      };

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

void MuseScore::loadStyle()
      {
      LoadStyle ls;
      ls.load(this, QString("."), QString("*.mss"), tr("MuseScore: load Style"));
      }

//---------------------------------------------------------
//   loader
//    return true on error
//---------------------------------------------------------

bool LoadStyle::loader(QFile* qf)
      {
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(qf, false, &err, &line, &column)) {
            error.sprintf("error reading file %s at line %d column %d: %s\n",
               name().toLatin1().data(), line, column, err.toLatin1().data());
            return true;
            }

      for (QDomNode node = doc.documentElement(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            if (e.tagName() == "museScore") {
                  // QString version = e.attribute(QString("version"));
                  for (QDomNode n = node.firstChild(); !n.isNull();  n = n.nextSibling()) {
                        e = n.toElement();
                        if (e.isNull())
                              continue;
                        QString tag(e.tagName());
                        QString val(e.text());
                        if (tag == "Style")
                              loadStyle(n);
                        else
                              domError(node);
                        }
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

class SaveStyle : public SaveFile {
   public:
      virtual bool saver();
      };

void MuseScore::saveStyle()
      {
      SaveStyle ls;
      ls.save(this, QString("."), QString(".mss"), tr("MuseScore: Save Style"));
      }

bool SaveStyle::saver()
      {
      Xml xml(&f);
      xml.header("museScore");
      xml.stag("museScore version=\"1.0\"");

      ::saveStyle(xml);

      xml.etag("museScore");
      return false;
      }

//---------------------------------------------------------
//   saveFile
//    return true on success
//---------------------------------------------------------

bool MuseScore::saveFile(QFile* f)
      {
      Xml xml(f);
      xml.header("museScore");
      xml.stag("museScore version=\"1.0\"");

      xml.tag("Geometry", mscore);
      xml.tag("Spatium", _spatium / DPMM);
      xml.tag("Division", division);
      xml.tag("Mag",  canvas->mag());
      xml.tag("xoff", canvas->xoffset());
      xml.tag("yoff", canvas->yoffset());

      ::saveStyle(xml);       // should style really saved with score?

      cs->write(xml);

      xml.tag("cursorStaff", cis->staff);
      xml.tag("cursorVoice", cis->voice);

      xml.etag("museScore");
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
//      reader.setFeature("http://xml.org/sax/features/namespaces", false);
//      reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", true);
      QXmlInputSource  source(qf);
      if (!doc.setContent(&source, &reader, &err, &line, &column)) {
            QString s;
            s.sprintf("error reading file %s at line %d column %d: %s\n",
               qf->fileName().toLatin1().data(), line, column, err.toLatin1().data());

            QMessageBox::critical(mscore, tr("MuseScore: Read File"), s);
            return true;
            }

      _fileDivision = 384;   // for compatibility with old mscore files

      for (QDomNode node = doc.documentElement(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            if (e.tagName() == "museScore") {
                  for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
                        e = n.toElement();
                        if (e.isNull())
                              continue;
                        QString tag(e.tagName());
                        QString val(e.text());
                        int i = val.toInt();
                        if (tag == "Staff")
                              readStaff(n);
                        else if (tag == "siglist")
                              sigmap->read(n, this);
                        else if (tag == "keylist")
                              keymap->read(n, this);
                        else if (tag == "tempolist")
                              tempomap->read(n, this);
                        else if (tag == "cursorStaff")
                              cis->staff = i;
                        else if (tag == "cursorVoice")
                              cis->voice = i;
                        else if (tag == "Geometry")
                              readGeometry(n);
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
                              ::loadStyle(n);
                        else if (tag == "page-layout")
                              pageFormat()->read(n);
                        else if (tag == "instrument-group")
                              readInstrumentGroup(n);
                        else if (tag == "rights")
                              rights = val;
                        else if (tag == "movement-number")
                              movementNumber = val;
                        else if (tag == "movement-title")
                              movementTitle = val;
                        else if (tag == "Part") {
                              Part* part = new Part(this);
                              part->read(this, n);
                              parts()->push_back(part);
                              }
                        else if (tag == "showInvisible")
                              _showInvisible = i;
                        else
                              domError(e);
                        }
                  }
            }
      // fixTicks();       //DEBUG

      //
      // create missing barlines
      //
      for (Measure* m = _layout->first(); m; m = m->next()) {
            //
            // dont create barline if next measure is
            //    a begin reapeat measure
            //
            if (m->next() && m->next()->startRepeat())
                  continue;
            int n = nstaves();
            for (int i = 0; i < n; ++i) {
                  Staff* sp = staff(i);
                  if (!sp->isTop())
                        continue;
                  MStaffList* sl = m->staffList();
                  MStaff& s = (*sl)[i];
                  if (s.endBarLine != 0)
                        continue;
                  BarLine* barLine = new BarLine(this);
                  barLine->setStaff(sp);
                  barLine->setSubtype(NORMAL_BAR);
                  m->add(barLine);
                  }
            }
      connectTies();
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
      QString instrTemplates = mscoreGlobalShare + "/templates/instruments_" + language.left(2) + ".xml";
      QFileInfo info(instrTemplates);
      if (!info.isReadable()) {
            instrTemplates = mscoreGlobalShare + "/templates/instruments.xml";
            info.setFile(instrTemplates);
            }
      if (!info.isReadable()) {
            fprintf(stderr, "cannot find instrument templates <%s>\n", instrTemplates.toLatin1().data());
            return;
            }
      QString zip;

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

      for (QDomNode node = doc.documentElement(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            if (e.tagName() == "museScore") {
                  for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
                        e = n.toElement();
                        if (e.isNull())
                              continue;
                        QString tag(e.tagName());
                        QString val(e.text());
                        if (tag == "instrument-group")
                              readInstrumentGroup(n);
                        else
                              domError(e);
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
      QPrinter printer(QPrinter::HighResolution);
      // QPrinter printer;
      printer.setPageSize(paperSizes[pageFormat()->size].qtsize);
      printer.setOrientation(pageFormat()->landscape ? QPrinter::Landscape : QPrinter::Portrait);
      printer.setCreator("MuseScore Version: " VERSION);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);

      QPrintDialog pd(&printer, 0);
      if (!pd.exec())
            return;

      Painter p(&printer);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setClipRect(QRectF(0.0, 0.0, 1000000.0, 1000000.0));

      qreal oldSpatium = _spatium;
      double oldDPI = DPI;
      DPI  = printer.logicalDpiX();          // drawing resolution
      DPMM = DPI / INCH;                     // dots/mm
      setSpatium(_spatium * DPI / oldDPI);
      QPaintDevice* oldPaintDevice = scoreLayout()->paintDevice();
      scoreLayout()->setPaintDevice(&printer);

      doLayout();

      p.setPrint(true);

      for (ciPage ip = pages()->begin();;) {
            Page* page = *ip;
            page->draw(p);
            ++ip;
            if (ip == pages()->end())
                  break;
            printer.newPage();
            }
      p.end();

      DPI = oldDPI;
      DPMM = DPI / INCH;                     // dots/mm
      scoreLayout()->setPaintDevice(oldPaintDevice);
      setSpatium(oldSpatium);
      doLayout();
      }

