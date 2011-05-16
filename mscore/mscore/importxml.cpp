//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
 MusicXML import.
 */

#include "config.h"
#include "mscore.h"
#include "musicxml.h"
#include "file.h"
#include "score.h"
#include "rest.h"
#include "chord.h"
#include "al/sig.h"
#include "key.h"
#include "clef.h"
#include "note.h"
#include "element.h"
#include "sym.h"
#include "slur.h"
#include "hairpin.h"
#include "tuplet.h"
#include "segment.h"
#include "dynamics.h"
#include "page.h"
#include "staff.h"
#include "part.h"
#include "measure.h"
#include "style.h"
#include "bracket.h"
#include "timesig.h"
#include "xml.h"
#include "barline.h"
#include "lyrics.h"
#include "volta.h"
#include "textline.h"
#include "keysig.h"
#include "pitchspelling.h"
#include "layoutbreak.h"
#include "tremolo.h"
#include "box.h"
#include "repeat.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "unzip.h"
#include "harmony.h"
#include "tempotext.h"
#include "articulation.h"
#include "arpeggio.h"
#include "glissando.h"
#include "breath.h"
#include "al/tempo.h"

//---------------------------------------------------------
//   xmlSetPitch
//---------------------------------------------------------

/**
 Convert MusicXML \a step / \a alter / \a octave to midi pitch,
 set pitch and tpc.
 */

static void xmlSetPitch(Note* n, char step, int alter, int octave, Ottava* ottava, int track)
      {
//      printf("xmlSetPitch(n=%p, st=%c, alter=%d, octave=%d)\n",
//             n, step, alter, octave);
      int istep = step - 'A';
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };
      if (istep < 0 || istep > 6) {
            printf("xmlSetPitch: illegal pitch %d, <%c>\n", istep, step);
            return;
            }
      int pitch = table[istep] + alter + (octave+1) * 12;

      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 127;

      if (ottava != 0 && ottava->track() == track)
            pitch -= ottava->pitchShift();

      n->setPitch(pitch);

      //                        a  b  c  d  e  f  g
      static int table1[7]  = { 5, 6, 0, 1, 2, 3, 4 };
      int tpc  = step2tpc(table1[istep], alter);
      // alternativ: tpc = step2tpc((istep + 5) % 7, alter);      // rotate istep 5 steps
      n->setTpc(tpc);
      }

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

/**
 MusicXml constructor.
 */

MusicXml::MusicXml(QDomDocument* d)
      {
      doc = d;
      maxLyrics = 0;
      lastVolta = 0;
      beamMode = BEAM_NO;
      }

//---------------------------------------------------------
//   LoadMusicXml
//---------------------------------------------------------

/**
 Loader for MusicXml files.
 */

class LoadMusicXml : public LoadFile {
      QDomDocument* _doc;

   public:
      LoadMusicXml() {
            _doc = new QDomDocument();
            }
      ~LoadMusicXml() {
            delete _doc;
            }
      virtual bool loader(QFile* f);
      QDomDocument* doc() const { return _doc; }
      };

//---------------------------------------------------------
//   loader
//---------------------------------------------------------

/**
 Load MusicXML file \a qf, return true if OK and false on error.
 */

bool LoadMusicXml::loader(QFile* qf)
      {
      int line, column;
      QString err;
      if (!_doc->setContent(qf, false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            error = err + "\n at line " + ln + " column " + col;
            return false;
            }
      docName = qf->fileName();
      return true;
      }

//---------------------------------------------------------
//   LoadCompressedMusicXml
//---------------------------------------------------------

/**
 Loader for compressed MusicXml files.
 */

class LoadCompressedMusicXml : public LoadFile {
      QDomDocument* _doc;

   public:
      LoadCompressedMusicXml() {
            _doc = new QDomDocument();
            }
      ~LoadCompressedMusicXml() {
            delete _doc;
            }
      virtual bool loader(QFile* f);
      QDomDocument* doc() const { return _doc; }
      };

//---------------------------------------------------------
//   loader
//---------------------------------------------------------

/**
 Load compressed MusicXML file \a qf, return false if OK and true on error.
 */

bool LoadCompressedMusicXml::loader(QFile* qf)
      {
      UnZip uz;
      UnZip::ErrorCode ec;
      ec = uz.openArchive(qf->fileName());
      if (ec != UnZip::Ok) {
            error = "Unable to open archive(" + qf->fileName() + "):\n" + uz.formatError(ec);
            return true;
            }

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
            error = err + "\n at line " + ln + " column " + col;
            printf("error: %s\n", error.toLatin1().data());
            return true;
            }

      // extract first rootfile
      QString rootfile = "";
      for (QDomElement e = container.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "container") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "rootfiles") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    if (eee.tagName() == "rootfile") {
                                          if (rootfile == "")
                                                rootfile = eee.attribute(QString("full-path"));
                                          }
                                    else
                                          domError(eee);
                                    }
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
      if (rootfile == "") {
            printf("can't find rootfile in: %s\n", qf->fileName().toLatin1().data());
            return true;
            }

      QBuffer dbuf;
      dbuf.open(QIODevice::WriteOnly);
      ec = uz.extractFile(rootfile, &dbuf);

      if (!_doc->setContent(dbuf.data(), false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            error = err + "\n at line " + ln + " column " + col;
            printf("error: %s\n", qPrintable(error));
            return true;
            }
      docName = qf->fileName();
      return false;
      }

//---------------------------------------------------------
//   importMusicXml
//---------------------------------------------------------

/**
 Import MusicXML file \a name into the Score.
 return false on error
 */

bool Score::importMusicXml(const QString& name)
      {
      LoadMusicXml lx;
      if (!lx.load(name))
            return false;
      setSaved(false);
      MusicXml musicxml(lx.doc());
      musicxml.import(this);
      connectTies();
      layoutAll = true;
      _created = false;
      return true;
      }

//---------------------------------------------------------
//   importCompressedMusicXml
//---------------------------------------------------------

/**
 Import compressed MusicXML file \a name into the Score.
 return false on error
 */

bool Score::importCompressedMusicXml(const QString& name)
      {
      LoadCompressedMusicXml lx;
      if (!lx.load(name))
            return false;
      setSaved(false);
      MusicXml musicxml(lx.doc());
      musicxml.import(this);
      connectTies();
      layoutAll = true;
      _created = false;
      return true;
      }

//---------------------------------------------------------
//   import
//      scorePartwise
//        part-list
//        part
//        work
//        identification
//---------------------------------------------------------

/**
 Parse the MusicXML file, which must be in score-partwise format.
 */

void MusicXml::import(Score* s)
      {
      score  = s;
      tie    = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            slur[i] = 0;
      for (int i = 0; i < MAX_BRACKETS; ++i)
            bracket[i] = 0;
      tuplet = 0;
      ottava = 0;
      trill = 0;
      pedal = 0;
      tremStart = 0;

      // TODO only if multi-measure rests used ???
      score->style().set(ST_createMultiMeasureRests, true);

      for (QDomElement e = doc->documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "score-partwise")
                  scorePartwise(e.firstChildElement());
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

static void addText(VBox* & vbx, Score* s, QString strTxt, int sbtp, int stl)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s);
            text->setSubtype(sbtp);
            text->setTextStyle(stl);
            text->setText(strTxt);
            if (vbx == 0)
                  vbx = new VBox(s);
            vbx->add(text);
            }
      }

//---------------------------------------------------------
//   doCredits
//---------------------------------------------------------

/**
 Create Text elements for the credits read from MusicXML credit-words elements.
 If no credits are found, create credits from meta data.
 */

void MusicXml::doCredits()
      {
//      printf("MusicXml::doCredits()\n");
      PageFormat* pf = score->pageFormat();
//      printf("page format w=%g h=%g spatium=%g DPMM=%g DPI=%g\n",
//             pf->width(), pf->height(), score->spatium(), DPMM, DPI);
      // page width and height in tenths
      const double pw  = pf->width() * 10 * DPI / score->spatium();
      const double ph  = pf->height() * 10 * DPI / score->spatium();
      const int pw1 = (int) (pw / 3);
      const int pw2 = (int) (pw * 2 / 3);
      const int ph2 = (int) (ph / 2);
//      printf("page format w=%g h=%g\n", pw, ph);
//      printf("page format pw1=%d pw2=%d ph2=%d\n", pw1, pw2, ph2);
      // dump the credits
/*
      for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
            CreditWords* w = *ci;
            printf("credit-words defx=%g defy=%g just=%s hal=%s val=%s words=%s\n",
                  w->defaultX,
                  w->defaultY,
                  w->justify.toUtf8().data(),
                  w->hAlign.toUtf8().data(),
                  w->vAlign.toUtf8().data(),
                  w->words.toUtf8().data());
            }
*/
      // apply simple heuristics using only default x and y
      // to recognize the meaning of credit words
      CreditWords* crwTitle = 0;
      CreditWords* crwSubTitle = 0;
      CreditWords* crwComposer = 0;
      CreditWords* crwPoet = 0;
      CreditWords* crwCopyRight = 0;
      // title is highest above middle of the page that is in the middle column
      // it is done first because subtitle detection depends on the title
      for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
            CreditWords* w = *ci;
            double defx = w->defaultX;
            double defy = w->defaultY;
            if (defy > ph2 && pw1 < defx && defx < pw2) {
                  // found a possible title
                  if (!crwTitle || defy > crwTitle->defaultY) crwTitle = w;
                  }
            }
      // subtitle is highest above middle of the page that is
      // in the middle column and is not the title
      for (ciCreditWords ci = credits.begin(); ci != credits.end(); ++ci) {
            CreditWords* w = *ci;
            double defx = w->defaultX;
            double defy = w->defaultY;
            if (defy > ph2 && pw1 < defx && defx < pw2) {
                  // found a possible subtitle
                  if ((!crwSubTitle || defy > crwSubTitle->defaultY)
                      && w != crwTitle)
                        crwSubTitle = w;
                  }
            // composer is above middle of the page and in the right column
            if (defy > ph2 && pw2 < defx) {
                  // found composer
                  if (!crwComposer) crwComposer = w;
                  }
            // poet is above middle of the page and in the left column
            if (defy > ph2 && defx < pw1) {
                  // found poet
                  if (!crwPoet) crwPoet = w;
                  }
            // copyright is below middle of the page and in the middle column
            if (defy < ph2 && pw1 < defx && defx < pw2) {
                  // found copyright
                  if (!crwCopyRight) crwCopyRight = w;
                  }
            } // end for (ciCreditWords ...
/*
      if (crwTitle) printf("title='%s'\n", crwTitle->words.toUtf8().data());
      if (crwSubTitle) printf("subtitle='%s'\n", crwSubTitle->words.toUtf8().data());
      if (crwComposer) printf("composer='%s'\n", crwComposer->words.toUtf8().data());
      if (crwPoet) printf("poet='%s'\n", crwPoet->words.toUtf8().data());
      if (crwCopyRight) printf("copyright='%s'\n", crwCopyRight->words.toUtf8().data());
*/

      if (crwTitle || crwSubTitle || crwComposer || crwPoet || crwCopyRight)
            score->setCreditsRead(true);

      QString strTitle;
      QString strSubTitle;
      QString strComposer;
      QString strPoet;
      QString strTranslator;

      if (score->creditsRead()) {
            if (crwTitle) strTitle = crwTitle->words;
            if (crwSubTitle) strSubTitle = crwSubTitle->words;
            if (crwComposer) strComposer = crwComposer->words;
            if (crwPoet) strPoet = crwPoet->words;
            }
      else {
            if (!(score->movementTitle().isEmpty() && score->workTitle().isEmpty())) {
                  strTitle = score->movementTitle();
                  if (strTitle.isEmpty())
                        strTitle = score->workTitle();
                  }
            if (!(score->movementNumber().isEmpty() && score->workNumber().isEmpty())) {
                  strSubTitle = score->movementNumber();
                  if (strSubTitle.isEmpty())
                        strSubTitle = score->workNumber();
                  }
            if (!composer.isEmpty()) strComposer = composer;
            if (!poet.isEmpty()) strPoet = poet;
            if (!translator.isEmpty()) strTranslator = translator;
            }

      VBox* vbox  = 0;
      addText(vbox, score, strTitle, TEXT_TITLE, TEXT_STYLE_TITLE);
      addText(vbox, score, strSubTitle, TEXT_SUBTITLE, TEXT_STYLE_SUBTITLE);
      addText(vbox, score, strComposer, TEXT_COMPOSER, TEXT_STYLE_COMPOSER);
      addText(vbox, score, strPoet, TEXT_POET, TEXT_STYLE_POET);
      addText(vbox, score, strTranslator, TEXT_TRANSLATOR, TEXT_STYLE_TRANSLATOR);
      if (vbox) {
            vbox->setTick(0);
            score->measures()->add(vbox);
            }
      if (crwCopyRight) score->setCopyright(crwCopyRight->words);
      }

//---------------------------------------------------------
//   scorePartwise
//---------------------------------------------------------

/**
 Read the MusicXML score-partwise element.
 */

void MusicXml::scorePartwise(QDomElement ee)
      {
      // In a first pass collect all Parts in case the part-list does not
      // list them all. Incomplete part-list's are generated by some versions
      // of finale

      for (QDomElement e = ee; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "part") {
                  QString id = e.attribute(QString("id"));
                  Part* part = new Part(score);
                  part->setId(id);
                  score->appendPart(part);
                  Staff* staff = new Staff(score, part, 0);
                  part->staves()->push_back(staff);
                  score->staves().push_back(staff);
                  }
            }
      for (QDomElement e = ee; !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "part-list")
                  xmlPartList(e.firstChildElement());
            else if (tag == "part")
                  xmlPart(e.firstChildElement(), e.attribute(QString("id")));
            else if (tag == "work") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "work-number")
                              score->setWorkNumber(ee.text());
                        else if (ee.tagName() == "work-title")
                              score->setWorkTitle(ee.text());
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "identification") {
                  // TODO: this is metadata !
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "creator") {
                              // type is an arbitrary label
                              QString type = ee.attribute(QString("type"));
                              QString str = ee.text();
                              MusicXmlCreator* crt = new MusicXmlCreator(type, str);
                              score->addCreator(crt);
                              if (type == "composer")
                                    composer = str;
                              else if (type == "poet") //not in dtd ?
                                    poet = str;
                              else if (type == "lyricist")
                                    poet = str;
                              else if (type == "translator")
                                    translator = str;
                              else if (type == "transcriber")
                                    ;
                              else
                                    printf("unknown creator <%s>\n", type.toLatin1().data());
                              }
                        else if (ee.tagName() == "rights")
                              score->setmxmlRights(ee.text());
                        else if (ee.tagName() == "encoding")
                              domNotImplemented(ee);
                        else if (ee.tagName() == "source")
                              score->setSource(ee.text());
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "defaults") {
                double millimeter = score->spatium()/10.0;
                double tenths = 1.0;
                QDomElement pageLayoutElement;
                for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                    QString tag(ee.tagName());
                    if (tag == "scaling") {
                        for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                            QString tag(eee.tagName());
                            if (tag == "millimeters")
                                millimeter = eee.text().toDouble();
                            else if (tag == "tenths")
                                tenths = eee.text().toDouble();
                            else
                                domError(eee);
                        }
                        double _spatium = DPMM * (millimeter * 10.0 / tenths);
                        score->setSpatium(_spatium);
                    }
                    else if (tag == "page-layout"){
                        pageLayoutElement = ee;
                    }
                    else if (tag == "system-layout") {
                        for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                            QString tag(eee.tagName());
                            Spatium val(eee.text().toDouble() / 10.0);
                            if (tag == "system-margins")
                                ;
                            else if (tag == "system-distance") {
                                score->style().set(ST_systemDistance, val);
                            }
                            else if (tag == "top-system-distance")
                                ;
                            else
                                domError(eee);
                        }
                    }
                    else if (tag == "staff-layout") {
                        for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                            QString tag(eee.tagName());
                            Spatium val(eee.text().toDouble() / 10.0);
                            if (tag == "staff-distance")
                                score->style().set(ST_staffDistance, val);
                            else
                                domError(eee);
                        }
                    }
                    else if (tag == "music-font")
                        domNotImplemented(ee);
                    else if (tag == "word-font")
                        domNotImplemented(ee);
                    else if (tag == "lyric-font")
                        domNotImplemented(ee);
                    else
                        domError(ee);
                }

                /*QMessageBox::warning(0,
                                     QWidget::tr("MuseScore: load XML"),
                                     QString("Val: ") + QString("%1").arg(QString::number(tenths,'f',2)) + " " + QString("%1").arg(QString::number(millimeter,'f',2)) + " " + QString("%1").arg(QString::number(INCH,'f',2)),
                                     QString::null, QWidget::tr("Quit"), QString::null, 0, 1);*/
                score->pageFormat()->readMusicXML(pageLayoutElement, millimeter / (tenths * INCH) );
                score->setDefaultsRead(true); // TODO only if actually succeeded ?
            }
            else if (tag == "movement-number")
                  score->setMovementNumber(e.text());
            else if (tag == "movement-title")
                  score->setMovementTitle(e.text());
            else if (tag == "credit") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "credit-words") {
                              double defaultx    = ee.attribute(QString("default-x")).toDouble();
                              double defaulty    = ee.attribute(QString("default-y")).toDouble();
                              QString justify = ee.attribute(QString("justify"));
                              QString halign  = ee.attribute(QString("halign"));
                              QString valign  = ee.attribute(QString("valign"));
                              QString crwords = ee.text();
                              CreditWords* cw = new CreditWords(defaultx, defaulty, justify, halign, valign, crwords);
                              credits.append(cw);
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }

      // add bracket where required
      const QList<Part*>* il = score->parts();
      // add bracket to multi-staff parts
      /* LVIFIX TODO is this necessary ?
      for (int idx = 0; idx < il->size(); ++idx) {
            Part* part = il->at(idx);
            printf("part %d staves=%d\n", idx, part->nstaves());
            if (part->nstaves() > 1)
                  part->staff(0)->addBracket(BracketItem(BRACKET_AKKOLADE, part->nstaves()));
            }
      */
      for (int i = 0; i < (int) partGroupList.size(); i++) {
            MusicXmlPartGroup* pg = partGroupList[i];
            // determine span in staves
            int stavesSpan = 0;
            for (int j = 0; j < pg->span; j++)
                  stavesSpan += il->at(pg->start + j)->nstaves();
            // and add bracket
            il->at(pg->start)->staff(0)->addBracket(BracketItem(pg->type, stavesSpan));
                  if(pg->barlineSpan)
                	  il->at(pg->start)->staff(0)->setBarLineSpan(pg->span);
            }
      }



//---------------------------------------------------------
//   partGroupStart
//---------------------------------------------------------

/**
 Store part-group start with number \a n, first part \a p and symbol / \a s in the partGroups
 array for later reference, as at this time insufficient information is available to be able
 to generate the brackets.
 */

static void partGroupStart(MusicXmlPartGroup* (&pgs)[MAX_PART_GROUPS], int n, int p, QString s, bool barlineSpan)
      {
//      printf("partGroupStart number=%d part=%d symbol=%s\n", n, p, s.toLatin1().data());
      if (n < 0 || n >= MAX_PART_GROUPS) {
            printf("illegal part-group number: %d\n", n);
            return;
            }

      if (pgs[n]) {
            printf("part-group number=%d already active\n", n);
            return;
            }

      int bracketType = NO_BRACKET;
      if (s == "")
            ; //ignore
      else if (s == "brace")
            bracketType = BRACKET_AKKOLADE;
      else if (s == "bracket")
            bracketType = BRACKET_NORMAL;
      else {
            printf("part-group symbol=%s not supported\n", s.toLatin1().data());
            return;
            }

      MusicXmlPartGroup* pg = new MusicXmlPartGroup;
      pg->span = 0;
      pg->start = p;
      pg->barlineSpan = barlineSpan,
      pg->type = bracketType;
      pgs[n] = pg;
      }

//---------------------------------------------------------
//   partGroupStop
//---------------------------------------------------------

/**
 Handle part-group stop with number \a n and part \a p.

 For part group n, the start part, span (in parts) and type are now known.
 To generate brackets, the span in staves must also be known.
 */

static void partGroupStop(MusicXmlPartGroup* (&pgs)[MAX_PART_GROUPS], int n, int p,
                          std::vector<MusicXmlPartGroup*> &pgl)
      {
//      printf("partGroupStop number=%d part=%d\n", n, p);
      if (n < 0 || n >= MAX_PART_GROUPS) {
            printf("illegal part-group number: %d\n", n);
            return;
            }

      if (!pgs[n]) {
            printf("part-group number=%d not active\n", n);
            return;
            }

      pgs[n]->span = p - pgs[n]->start;
//      printf("part-group number=%d start=%d span=%d type=%d\n",
//             n, pgs[n]->start, pgs[n]->span, pgs[n]->type);
      pgl.push_back(pgs[n]);
      pgs[n] = 0;
      }

//---------------------------------------------------------
//   xmlPartList
//---------------------------------------------------------

/**
 Read the MusicXML part-list element.
 */

void MusicXml::xmlPartList(QDomElement e)
      {
      int scoreParts = 0;
      bool barlineSpan = false;
      MusicXmlPartGroup* partGroups[MAX_PART_GROUPS];
      for (int i = 0; i < MAX_PART_GROUPS; ++i)
            partGroups[i] = 0;

      for (;!e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "score-part") {
                  xmlScorePart(e.firstChildElement(), e.attribute(QString("id")));
                  scoreParts++;
                  }
            else if (e.tagName() == "part-group") {
                  int number = e.attribute(QString("number")).toInt() - 1;
                  QString symbol = "";
                  QString type = e.attribute(QString("type"));
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "group-symbol")
                              symbol = ee.text();
                        else if(ee.tagName() == "group-barline"){
                              if(ee.text() == "yes")
                                    barlineSpan = true;
                        }else
                              domError(ee);
                        }
                  if (type == "start")
                        partGroupStart(partGroups, number, scoreParts, symbol, barlineSpan);
                  else if (type == "stop")
                        partGroupStop(partGroups, number, scoreParts, partGroupList);
                  else
                        printf("Import MusicXml:xmlPartList: part-group type '%s' not supported\n",
                               type.toLatin1().data());
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   xmlScorePart
//---------------------------------------------------------

/**
 Read the MusicXML score-part element.
 */

void MusicXml::xmlScorePart(QDomElement e, QString id)
      {
      Part* part = 0;
      foreach(Part* p, *score->parts()) {
            if (p->id() == id) {
                  part = p;
                  break;
                  }
            }
      if (part == 0) {
            printf("Import MusicXml:xmlScorePart: cannot find part %s\n", qPrintable(id));
            exit(-1);
            }

// printf("create track id:<%s>\n", id.toLatin1().data());

      for (;!e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "part-name") {
                  // OK? (ws) Yes it should be ok.part-name is display in front of staff in finale. (la)
                  part->setLongName(e.text());
                  part->setTrackName(e.text());
                  }
            else if (e.tagName() == "part-abbreviation") {
            	  part->setShortName(e.text());
                  }
            else if (e.tagName() == "score-instrument") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "instrument-name") {
                              // part-name or instrument-name?
                              if (part->longName()->getText().isEmpty())
                                    part->setLongName(ee.text());
                              }
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "midi-instrument") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "midi-channel")
                              part->setMidiChannel(ee.text().toInt() - 1);
                        else if (ee.tagName() == "midi-program")
                              part->setMidiProgram(ee.text().toInt() - 1);
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
/*      score->parts()->push_back(part);
      Staff* staff = new Staff(score, part, 0);
      part->staves()->push_back(staff);
      score->staves().push_back(staff);
 */
      }

//---------------------------------------------------------
//   xmlPart
//---------------------------------------------------------

/**
 Read the MusicXML part element.
 */

void MusicXml::xmlPart(QDomElement e, QString id)
      {
      Part* part = 0;
      foreach(Part* p, *score->parts()) {
            if (p->id() == id) {
                  part = p;
                  break;
                  }
            }
      if (part == 0) {
            printf("Import MusicXml:xmlPart: cannot find part %s\n", id.toLatin1().data());
            exit(-1);
            }
      tick                  = 0;
      maxtick               = 0;
      lastMeasureLen        = 0;
      multiMeasureRestCount = 0;
      startMultiMeasureRest = false;

      // initialize voice list
      // for (int i = 0; i < part->nstaves(); ++i) {
      for (int i = 0; i < MAX_STAVES; ++i)
            voicelist[i].clear();

      if (!score->measures()->first()) {
            doCredits();
            }

      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "measure") {
                  xmlMeasure(part, e, e.attribute(QString("number")).toInt()-1);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   stringToInt
//---------------------------------------------------------

/**
 Convert a string in \a s into an int. Set *res to true iff conversion was
 successful. \a s may end with ".0", as is generated by Audiveris 3.2 and up,
 in elements <divisions>, <duration>, <alter> and <sound> attributes
 dynamics and tempo.
 In case of error val return a default value of 0. A negative number is an error.
 Note that non-integer values cannot be handled by mscore.
 */

static int stringToInt(const QString& s, bool* ok)
      {
      int res = 0;
      QString str = s;
      if (s.endsWith(".0"))
            str = s.left(s.size() - 2);
      res = str.toInt(ok);
      return res;
      }


//---------------------------------------------------------
//   xmlMeasure
//---------------------------------------------------------

/**
 Read the MusicXML measure element.
 */

Measure* MusicXml::xmlMeasure(Part* part, QDomElement e, int number)
      {
      int staves = score->nstaves();
      int staff = score->staffIdx(part);

      if (staves == 0) {
            printf("MusicXml::xmlMeasure no staves!\n");
            return 0;
            }

      // search measure for tick
      Measure* measure = 0;
      Measure* lastMeasure = 0;
      for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            lastMeasure = m;
            if (m->tick() == tick) {
                  measure = m;
                  break;
                  }
            }
      if (!measure) {
            //
            // DEBUG:
            if (lastMeasure && lastMeasure->tick() > tick) {
                  printf("Measure at position %d not found!\n", tick);
                  }
            measure  = new Measure(score);
            measure->setTick(tick);
            measure->setNo(number);
            score->measures()->add(measure);
      }else{
            int pstaves = part->nstaves();
            for (int i = 0; i < pstaves; ++i) {
                Staff* reals = score->staff(staff+i);
                measure->mstaff(staff+i)->lines->setLines(reals->lines());
            }
      }

// must remember volta to handle <ending type="discontinue">
//      Volta* lastVolta = 0;       // ws: move to global to allow for voltas spanning more
                                    //     than one measure

      QString implicit = e.attribute("implicit", "no");
      if (implicit == "yes")
            measure->setIrregular(true);


      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "attributes")
                  xmlAttributes(measure, staff, e.firstChildElement());
            else if (e.tagName() == "note")
                  xmlNote(measure, staff, e.firstChildElement());
            else if (e.tagName() == "backup") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "duration") {
                              bool ok;
                              int val = stringToInt(ee.text(), &ok);
                              if (!ok) {
                                    printf("MusicXml-Import: bad backup duration value: <%s>\n",
                                       qPrintable(ee.text()));
                                    }
                              if (val == 0)     // neuratron scanner produces sometimes 0 !?
                                    val = 1;
                              val = (val * AL::division) / divisions;
                              tick -= val;
                              lastLen = val;    // ?
                              }
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "direction") {
                  direction(measure, staff, e);
                  }
            else if (e.tagName() == "print") {
                  QString newSystem = e.attribute("new-system", "no");
                  QString newPage   = e.attribute("new-page", "no");
                  //
                  // in MScore the break happens _after_ the marked measure:
                  //
                  Measure* pm = (Measure*)(measure->prev());      // TODO: MeasureBase
                  if (pm == 0)
                        printf("ImportXml: warning: break on first measure\n");
                  else {
                        if (newSystem == "yes" || newPage == "yes") {
                              LayoutBreak* lb = new LayoutBreak(score);
                              lb->setTrack(staff * VOICES);
                              lb->setSubtype(
                                 newSystem == "yes" ? LAYOUT_BREAK_LINE : LAYOUT_BREAK_PAGE
                                 );
                              pm->add(lb);
                              }
                        }
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "system-layout") {
                              }
                        else if (ee.tagName() == "staff-layout") {
                              }
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "forward") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "duration") {
                              bool ok;
                              int val = stringToInt(ee.text(), &ok);
                              if (!ok) {
                                    printf("MusicXml-Import: bad forward duration value: <%s>\n",
                                       qPrintable(ee.text()));
                                    }
                              val = val * AL::division / divisions;
                              tick += val;
                              if (tick > maxtick)
                                    maxtick = tick;
                              lastLen = val;    // ?
                              }
                        else if (ee.tagName() == "voice")
                              ;
                        else if (ee.tagName() == "staff")
                              ;
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "barline") {
                  QString loc = e.attribute("location", "right");
                  QString barStyle;
                  QString endingNumber;
                  QString endingType;
                  QString repeat;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "bar-style")
                              barStyle = ee.text();
                        else if (ee.tagName() == "ending") {
                              endingNumber = ee.attribute("number");
                              endingType   = ee.attribute("type");
                              }
                        else if (ee.tagName() == "repeat")
                              repeat = ee.attribute("direction");
                        else
                              domError(ee);
                        }
                  if ((barStyle != "") || (repeat != "")) {
                        BarLine* barLine = new BarLine(score);
                        bool visible = true;
                        if (barStyle == "light-heavy" && repeat == "backward") {
                              barLine->setSubtype(END_REPEAT);
                              }
                        else if (barStyle == "heavy-light" && repeat == "forward") {
                              barLine->setSubtype(START_REPEAT);
                              }
                        else if (barStyle == "light-heavy" && repeat.isEmpty())
                              barLine->setSubtype(END_BAR);
                        else if (barStyle == "regular")
                              barLine->setSubtype(NORMAL_BAR);
                        else if (barStyle == "dotted")
                              barLine->setSubtype(BROKEN_BAR);
                        else if (barStyle == "light-light")
                              barLine->setSubtype(DOUBLE_BAR);
      /*                  else if (barStyle == "heavy-light")
                              ;
                        else if (barStyle == "heavy-heavy")
                              ;
      */
                        else if (barStyle == "none"){
                              barLine->setSubtype(NORMAL_BAR);
                              visible = false;
                        }
                        else if (barStyle == "") {
                              if (repeat == "backward")
                                    barLine->setSubtype(END_REPEAT);
                              else if (repeat == "forward")
                                    barLine->setSubtype(START_REPEAT);
                              else
                                    printf("ImportXml: warning: empty bar type\n");
                              }
                        else
                              printf("unsupported bar type <%s>\n", barStyle.toLatin1().data());
                        barLine->setTrack(staff * VOICES);
                        if (barLine->subtype() == START_REPEAT) {
                              measure->setRepeatFlags(RepeatStart);
                              }
                        else if (barLine->subtype() == END_REPEAT) {
                              measure->setRepeatFlags(RepeatEnd);
                              }
                        else
                              measure->setEndBarLineType(barLine->subtype(), false, visible);
                        }
                  if (!(endingNumber.isEmpty() && endingType.isEmpty())) {
                        if (endingNumber.isEmpty())
                              printf("ImportXml: warning: empty ending number\n");
                        else if (endingType.isEmpty())
                              printf("ImportXml: warning: empty ending type\n");
                        else {
                              int iEendingNumber = endingNumber.toInt();
                              if (iEendingNumber <= 0)
                                    printf("ImportXml: warning: unsupported ending number <%s>\n",
                                            endingNumber.toLatin1().data());
                              else {
                                    if (endingType == "start") {
                                          Volta* volta = new Volta(score);
                                          volta->setTrack(staff * VOICES);
                                          volta->setTick(tick);
                                          volta->setText(endingNumber);
                                          // LVIFIX TODO also support endings "1, 2" and "1 - 3"
                                          volta->endings().clear();
                                          volta->endings().append(iEendingNumber);
                                          lastVolta = volta;
                                          }
                                    else if (endingType == "stop") {
                                          if (lastVolta) {
												lastVolta->setTick2(tick);
												lastVolta->setSubtype(Volta::VOLTA_CLOSED);
                                                score->add(lastVolta);
												lastVolta = 0;
                                                }
                                          else {
                                                printf("lastVolta == 0 on stop\n");
                                                }
                                          }
                                    else if (endingType == "discontinue") {
                                          if (lastVolta) {
                                                lastVolta->setTick2(tick);
                                                lastVolta->setSubtype(Volta::VOLTA_OPEN);
												score->add(lastVolta);
                                                lastVolta = 0;
                                                }
                                          else {
                                                printf("lastVolta == 0 on discontinue\n");
                                                }
                                          }
                                    else
                                          printf("ImportXml: warning: unsupported ending type <%s>\n",
                                                  endingType.toLatin1().data());
                                    }
                              }
                        }
                  }
            else if (e.tagName() == "sound")
                  domNotImplemented(e);
            else if (e.tagName() == "harmony")
                  xmlHarmony(e, tick, measure);
            else
                  domError(e);
            }
      staves         = part->nstaves();
      int measureLen = maxtick - measure->tick();

      if (lastMeasureLen != measureLen) {
            AL::TimeSigMap* sigmap = score->sigmap();
            int tick        = measure->tick();
            AL::SigEvent se = sigmap->timesig(tick);

            if (measureLen != sigmap->ticksMeasure(tick)) {
                  AL::SigEvent se = sigmap->timesig(tick);

                  Fraction f = se.getNominal();
// printf("Add Sig %d  len %d:  %s\n", tick, measureLen, qPrintable(f.print()));
                  score->sigmap()->add(tick, measureLen, f);
                  int tm = AL::ticks_measure(se.fraction());
                  if (tm != measureLen) {
                        if (!measure->irregular()) {
                            /* MusicXML's implicit means "don't print measure number",
                              set it only if explicitly requested, not when the measure length
                              is not what is expected. See MozartTrio.xml measures 12..13.
                            */
                            // measure->setIrregular(true);
                            /*
                              printf("irregular Measure %d Len %d at %d   lastLen: %d -> should be: %d (tm=%d)\n",
                                 number, measure->tick(), maxtick,
                                 lastMeasureLen, measureLen, tm);
                              */
                              }
                        }
                  }
            }
      lastMeasureLen = measureLen;
      tick = maxtick;

      // multi-measure rest handling:
      // all normal measures get setBreakMultiMeasureRest(true)
      // the first measure in a multi-measure rest gets setBreakMultiMeasureRest(true)
      // all other measures in a multi-measure rest get setBreakMultiMeasureRest(false)
      // and count down the remaining number of measures
      if (startMultiMeasureRest) {
            measure->setBreakMultiMeasureRest(true);
            startMultiMeasureRest = false;
            }
      else {
            if (multiMeasureRestCount > 0) {
                  // measure is continuation of a multi-measure rest
                  measure->setBreakMultiMeasureRest(false);
                  --multiMeasureRestCount;
                  }
            else
                  measure->setBreakMultiMeasureRest(true);
            }

      return measure;
      }

//---------------------------------------------------------
//   setSLinePlacement -- helper for direction
//---------------------------------------------------------

static void setSLinePlacement(SLine* sli, float s, const QString pl, bool hasYoff, qreal yoff)
      {
//      printf("setSLinePlacement s=%g pl='%s' hasy=%d yoff=%g\n",
//             s, qPrintable(pl), hasYoff, yoff
//            );
      float offs = 0.0;
      if (hasYoff) offs = yoff;
      else {
            if (pl == "above")
                  offs = -3;
            else if (pl == "below")
                  offs = 8;
            else
                  printf("setSLinePlacement invalid placement '%s'\n", qPrintable(pl));
            }
      LineSegment* ls = sli->lineSegments().front();
      ls->setUserOff(QPointF(0, offs * s));
      }

//---------------------------------------------------------
//   metronome
//---------------------------------------------------------

static void addSymbolToText(const SymCode& s, QTextCursor* cur)
      {
      QTextCharFormat oFormat = cur->charFormat();
      if (s.fontId >= 0) {
            QTextCharFormat oFormat = cur->charFormat();
            QTextCharFormat nFormat(oFormat);
            nFormat.setFontFamily(fontId2font(s.fontId).family());
            cur->setCharFormat(nFormat);
            cur->insertText(s.code);
            cur->setCharFormat(oFormat);
            }
      else
            cur->insertText(s.code);
      }

/**
 Read the MusicXML metronome element.
 */

/*
          <metronome parentheses="yes">
            <beat-unit>quarter</beat-unit>
            <beat-unit-dot/>
            <per-minute>50</per-minute>
            </metronome>
          <metronome parentheses="yes">
            <beat-unit>quarter</beat-unit>
            <beat-unit-dot/>
            <beat-unit>half</beat-unit>
            <beat-unit-dot/>
            </metronome>
*/

static void metronome(QDomElement e, Text* t)
      {
      bool textAdded = false;
      QTextDocument* d = t->doc();
      QTextCursor c(d);
      c.movePosition(QTextCursor::EndOfLine);
      QString parenth = e.attribute("parentheses");
      if (parenth == "yes") c.insertText("(");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "beat-unit") {
                  if (textAdded) c.insertText(" = ");
                  QString txt = e.text();
                  if (txt == "breve") addSymbolToText(SymCode(0xe100, 1), &c);
                  else if (txt == "whole") addSymbolToText(SymCode(0xe101, 1), &c);
                  else if (txt == "half") addSymbolToText(SymCode(0xe104, 1), &c);
                  else if (txt == "quarter") addSymbolToText(SymCode(0xe105, 1), &c);
                  else if (txt == "eighth") addSymbolToText(SymCode(0xe106, 1), &c);
                  else if (txt == "16th") addSymbolToText(SymCode(0xe107, 1), &c);
                  else if (txt == "32nd") addSymbolToText(SymCode(0xe108, 1), &c);
                  else if (txt == "64th") addSymbolToText(SymCode(0xe109, 1), &c);
                  else c.insertText(e.text());
                  textAdded = true;
                  }
            else if (e.tagName() == "beat-unit-dot") {
                  addSymbolToText(SymCode(0xe10a, 1), &c);
                  textAdded = true;
                  }
            else if (e.tagName() == "per-minute") {
                  if (textAdded) c.insertText(" = ");
                  c.insertText(e.text());
                  textAdded = true;
                  }
            else
                  domError(e);
            } // for (e = e.firstChildElement(); ...
      if (parenth == "yes") c.insertText(")");
      }

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

/**
 Read the MusicXML direction element.
 */

// LVI FIXME: introduce offset concept to mscore.
// offset changes only the print position (not the tick), but unlike relative-x
// it is expressed in terms of divisions (MusicXML direction.dtd)
// even though the DTD does not mention it, practically speaking
// offset and relative-x are mutually exclusive

// Typical example:
// <direction placement="above">
//   <direction-type>
//      <words>Fine</words>
//      </direction-type>
//   <sound fine="yes"/>
//   </direction>

// Note: multiple direction-type (and multiple words) elements may be present,
// and at least one direction-type must be present but sound is optional and
// at most one can be present.

void MusicXml::direction(Measure* measure, int staff, QDomElement e)
      {
//      printf("MusicXml::direction\n");
      QString placement = e.attribute("placement");

      QString dirType;
      QString type;
      QString txt;
      QString lang;
      QString fontWeight = "";
      QString fontStyle = "";
      QString fontSize = "";
      int offset = 0;
      int rstaff = 0;
      QStringList dynamics;
      int spread = 0;
      qreal rx = 0.0;
      qreal ry = 0.0;
      qreal yoffset = 0.0; // actually this is default-y
      qreal xoffset = 0.0;
      bool hasYoffset = false;
      QString tempo = "";
      QString rehearsal = "";
      QString sndCapo = "";
      QString sndCoda = "";
      QString sndDacapo = "";
      QString sndDalsegno = "";
      QString sndSegno = "";
      QString sndFine = "";
      bool coda = false;
      bool segno = false;
      int ottavasize = 0;
      bool pedalLine = false;
      int number = 1;
      QString lineEnd;
      qreal endLength = 0;
      QString lineType;
      QDomElement metrEl;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "direction-type") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        dirType = ee.tagName();
                        ry      = ee.attribute(QString("relative-y"), "0").toDouble() * -.1;
                        rx      = ee.attribute(QString("relative-x"), "0").toDouble() * .1;
                        yoffset = ee.attribute("default-y").toDouble(&hasYoffset) * -0.1;
                        xoffset = ee.attribute("default-x", "0.0").toDouble() * 0.1;
                        if (dirType == "words") {
                              txt        = ee.text();
                              lang       = ee.attribute(QString("xml:lang"), "it");
                              fontWeight = ee.attribute(QString("font-weight"));
                              fontSize   = ee.attribute(QString("font-size"));
                              fontStyle  = ee.attribute(QString("font-style"));
                              }
                        else if (dirType == "rehearsal") {
                              rehearsal = ee.text();
                              }
                        else if (dirType == "pedal") {
                              type = ee.attribute(QString("type"));
                              pedalLine = ee.attribute(QString("line")) == "yes";
                              }
                        else if (dirType == "dynamics") {
                              QDomElement eee = ee.firstChildElement();
                              if (!eee.isNull()) {
                                    if (eee.tagName() == "other-dynamics")
                                          dynamics.push_back(eee.text());
                                    else
                                          dynamics.push_back(eee.tagName());
                                    }
                              }
                        else if (dirType == "wedge") {
                              type   = ee.attribute(QString("type"));
                              spread = ee.attribute(QString("spread"), "0").toInt();
                              }
                        else if (dirType == "dashes")
                              domNotImplemented(ee);
                        else if (dirType == "bracket") {
                              type      = ee.attribute(QString("type"));
                              number    = ee.attribute(QString("number"), "1").toInt();
                              lineEnd   = ee.attribute(QString("line-end"), "none");
                              endLength = ee.attribute(QString("end-length"), "0").toDouble() * 0.1;
                              lineType  = ee.attribute(QString("line-type"), "solid");
                              }
                        else if (dirType == "metronome")
                              metrEl = ee;
                        else if (dirType == "octave-shift") {
                              type       = ee.attribute(QString("type"));
                              ottavasize = ee.attribute(QString("size"), "0").toInt();
                              }
                        else if (dirType == "coda")
                              coda = true;
                        else if (dirType == "segno")
                              segno = true;
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "sound") {
                  // attr: dynamics, tempo
// LVIFIX: TODO coda and segno should be numbered uniquely
                  sndCapo = e.attribute("capo");
                  sndCoda = e.attribute("coda");
                  sndDacapo = e.attribute("dacapo");
                  sndDalsegno = e.attribute("dalsegno");
                  sndFine = e.attribute("fine");
                  sndSegno = e.attribute("segno");
                  tempo = e.attribute("tempo");
                  }
            else if (e.tagName() == "offset")
                  offset = (e.text().toInt() * AL::division)/divisions;
            else if (e.tagName() == "staff") {
                  // DEBUG: <staff>0</staff>
                  rstaff = e.text().toInt() - 1;
                  if (rstaff < 0)         // ???
                        rstaff = 0;
                  }
            else
                  domError(e);
            } // for (e = e.firstChildElement(); ...

/*
      printf(" tempo=%s txt=%s metrEl=%s coda=%d segno=%d sndCapo=%s sndCoda=%s"
             " sndDacapo=%s sndDalsegno=%s sndFine=%s sndSegno=%s\n",
             tempo.toLatin1().data(),
             txt.toLatin1().data(),
             qPrintable(metrEl.tagName()),
             coda,
             segno,
             sndCapo.toLatin1().data(),
             sndCoda.toLatin1().data(),
             sndDacapo.toLatin1().data(),
             sndDalsegno.toLatin1().data(),
             sndFine.toLatin1().data(),
             sndSegno.toLatin1().data()
            );
*/

      // Try to recognize the various repeats
      QString repeat = "";
      // Easy cases first
      if (coda) repeat = "coda";
      if (segno) repeat = "segno";
      // As sound may be missing, next do a wild-card match with known repeat texts
      QString lowerTxt = txt.toLower();
      QRegExp daCapo("d\\.? *c\\.?|da *capo");
      QRegExp daCapoAlFine("d\\.? *c\\.? *al *fine|da *capo *al *fine");
      QRegExp daCapoAlCoda("d\\.? *c\\.? *al *coda|da *capo *al *coda");
      QRegExp dalSegno("d\\.? *s\\.?|d[ae]l *segno");
      QRegExp dalSegnoAlFine("d\\.? *s\\.? *al *fine|d[ae]l *segno *al *fine");
      QRegExp dalSegnoAlCoda("d\\.? *s\\.? *al *coda|d[ae]l *segno *al *coda");
      QRegExp fine("fine");
      QRegExp toCoda("to *coda");
      if (daCapo.exactMatch(lowerTxt)) repeat = "daCapo";
      if (daCapoAlFine.exactMatch(lowerTxt)) repeat = "daCapoAlFine";
      if (daCapoAlCoda.exactMatch(lowerTxt)) repeat = "daCapoAlCoda";
      if (dalSegno.exactMatch(lowerTxt)) repeat = "dalSegno";
      if (dalSegnoAlFine.exactMatch(lowerTxt)) repeat = "dalSegnoAlFine";
      if (dalSegnoAlCoda.exactMatch(lowerTxt)) repeat = "dalSegnoAlCoda";
      if (fine.exactMatch(lowerTxt)) repeat = "fine";
      if (toCoda.exactMatch(lowerTxt)) repeat = "toCoda";
      // If that did not work, try to recognize a sound attribute
      if (repeat == "" && sndCoda != "") repeat = "coda";
      if (repeat == "" && sndDacapo != "") repeat = "daCapo";
      if (repeat == "" && sndDalsegno != "") repeat = "dalSegno";
      if (repeat == "" && sndFine != "") repeat = "fine";
      if (repeat == "" && sndSegno != "") repeat = "segno";
      // If a repeat was found, assume words is no longer needed
      if (repeat != "") txt = "";

/*
      printf(" txt=%s repeat=%s\n",
             txt.toLatin1().data(),
             repeat.toLatin1().data()
            );
*/

      if (repeat != "") {
            Element* e = 0;
            if (repeat == "segno") {
                  e = new Marker(score);
                  ((Marker*)e)->setMarkerType(MARKER_SEGNO);
                  }
            else if (repeat == "coda") {
                  Marker* m = new Marker(score);
                  m->setMarkerType(MARKER_CODA);
                  e = m;
                  }
            else if (repeat == "fine") {
                  Marker* m = new Marker(score);
                  m->setMarkerType(MARKER_FINE);
                  e = m;
                  }
            else if (repeat == "toCoda") {
                  Marker* m = new Marker(score);
                  m->setMarkerType(MARKER_TOCODA);
                  e = m;
                  }
            else if (repeat == "daCapo") {
                  Jump* jp = new Jump(score);
                  jp->setJumpType(JUMP_DC);
                  e = jp;
                  }
            else if (repeat == "daCapoAlCoda") {
                  Jump* jp = new Jump(score);
                  jp->setJumpType(JUMP_DC_AL_CODA);
                  e = jp;
                  }
            else if (repeat == "daCapoAlFine") {
                  Jump* jp = new Jump(score);
                  jp->setJumpType(JUMP_DC_AL_FINE);
                  e = jp;
                  }
            else if (repeat == "dalSegno") {
                  Jump* jp = new Jump(score);
                  jp->setJumpType(JUMP_DS);
                  e = jp;
                  }
            else if (repeat == "dalSegnoAlCoda") {
                  Jump* jp = new Jump(score);
                  jp->setJumpType(JUMP_DS_AL_CODA);
                  e = jp;
                  }
            else if (repeat == "dalSegnoAlFine") {
                  Jump* jp = new Jump(score);
                  jp->setJumpType(JUMP_DS_AL_FINE);
                  e = jp;
                  }
            if (e) {
                  e->setTrack((staff + rstaff) * VOICES);
                  measure->add(e);
                  }
            }

      if ((dirType == "words" && txt != "") || dirType == "metronome") {
/*
            printf("words txt='%s' metrEl='%s' tempo='%s' pl='%s' hasyoffs=%d fsz='%s' fst='%s' fw='%s'\n",
                    txt.toUtf8().data(),
                    qPrintable(metrEl.tagName()),
                    tempo.toUtf8().data(),
                    placement.toUtf8().data(),
                    hasYoffset,
                    fontSize.toUtf8().data(),
                    fontStyle.toUtf8().data(),
                    fontWeight.toUtf8().data()
                  );
*/
            Text* t;
            if (tempo != "") {
                  t = new TempoText(score);
                  double tpo = tempo.toDouble()/60.0;
                  ((TempoText*) t)->setTempo(tpo);
                  AL::TempoMap* tl = score->tempomap();
                  if(tl) tl->addTempo(tick, tpo);
                  }
            else {
                  t = new Text(score);
                  t->setTextStyle(TEXT_STYLE_TECHNIK);
                  }
            t->setTick(tick);
            if (fontSize != "" || fontStyle != "" || fontWeight != "") {
                  QFont f = t->defaultFont();
                  if (fontSize != "") {
                        bool ok = true;
                        float size = fontSize.toFloat();
                        if (ok) f.setPointSizeF(size);
                        }
                  f.setItalic(fontStyle == "italic");
                  f.setBold(fontWeight == "bold");
                  t->setDefaultFont(f);
                  }
            t->setText(txt);
            if (metrEl.tagName() != "") metronome(metrEl, t);
            if (hasYoffset) t->setYoff(yoffset);
            else t->setAbove(placement == "above");
            t->setUserOff(QPointF(rx, ry));
            t->setMxmlOff(offset);
            if (tempo != "")
                  t->setTrack(0); //Track 0 for tempo text (system text)
            else
                  t->setTrack((staff + rstaff) * VOICES);
            measure->add(t);
            }
      else if (dirType == "rehearsal") {
            Text* t = new Text(score);
            t->setSubtype(TEXT_REHEARSAL_MARK);
            t->setTextStyle(TEXT_STYLE_REHEARSAL_MARK);
            t->setTick(tick);
            t->setText(rehearsal);
            if (hasYoffset) t->setYoff(yoffset);
            else t->setAbove(placement == "above");
            t->setUserOff(QPointF(rx, ry));
            t->setMxmlOff(offset);
            t->setTrack((staff + rstaff) * VOICES);
            measure->add(t);
            }
      else if (dirType == "pedal") {
            if (pedalLine) {
                  if (type == "start") {
                        if (pedal) {
                              printf("overlapping pedal lines not supported\n");
                              delete pedal;
                              pedal = 0;
                              }
                        else {
                              pedal = new Pedal(score);
                              pedal->setTrack((staff + rstaff) * VOICES);
                              pedal->setTick(tick);
                              if (placement == "") placement = "below";
                              setSLinePlacement(pedal,
                                          score->spatium(), placement,
                                          hasYoffset, yoffset);
                              }
                        }
                  else if (type == "stop") {
                        if (!pedal) {
                              printf("pedal line stop without start\n");
                              }
                        else {
                              pedal->setTick2(tick);
                              score->add(pedal);
                              pedal = 0;
                              }
                        }
                  else
                        printf("unknown pedal %s\n", type.toLatin1().data());
                  }
            else {
                  Symbol* s = new Symbol(score);
                  s->setAlign(ALIGN_LEFT | ALIGN_BASELINE);
                  s->setOffsetType(OFFSET_SPATIUM);
                  s->setTick(tick);
                  if (type == "start")
                        s->setSym(pedalPedSym);
                  else if (type == "stop")
                        s->setSym(pedalasteriskSym);
                  else
                        printf("unknown pedal %s\n", type.toLatin1().data());
                  if (hasYoffset) s->setYoff(yoffset);
                  else s->setAbove(placement == "above");
                  s->setUserOff(QPointF(rx, ry));
                  s->setMxmlOff(offset);
                  s->setTrack((staff + rstaff) * VOICES);
                  measure->add(s);
                  }
            }
      else if (dirType == "dynamics") {
            // more than one dynamic ???
            // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
            for (QStringList::Iterator it = dynamics.begin(); it != dynamics.end(); ++it ) {
                  Dynamic* dyn = new Dynamic(score);
                  dyn->setSubtype(*it);
                  if (hasYoffset) dyn->setYoff(yoffset);
                  else dyn->setAbove(placement == "above");
                  dyn->setUserOff(QPointF(rx, ry));
                  dyn->setMxmlOff(offset);
                  dyn->setTrack((staff + rstaff) * VOICES);
                  dyn->setTick(tick);
                  measure->add(dyn);
                  }
            }
      else if (dirType == "wedge") {
            bool above = (placement == "above");
            if (type == "crescendo")
                  addWedge(0, tick, rx, ry, above, hasYoffset, yoffset, 0);
            else if (type == "stop")
                  genWedge(0, tick, measure, staff+rstaff);
            else if (type == "diminuendo")
                  addWedge(0, tick, rx, ry, above, hasYoffset, yoffset, 1);
            else
                  printf("unknown wedge type: %s\n", type.toLatin1().data());
            }
      else if (dirType == "bracket") {
            int n = number-1;
            TextLine* b = bracket[n];
            if (type == "start") {
                  if (b) {
                        printf("overlapping bracket with same number?\n");
                        delete b;
                        bracket[n] = 0;
                        }
                  else {
                        b = new TextLine(score);

                        // what does placement affect?
                        //yoffset += (placement == "above" ? 0.0 : 5.0);
                        // store for later to set in segment
                        // b->setUserOff(QPointF(rx + xoffset, ry + yoffset));
                        b->setMxmlOff(offset);
                        if (placement == "") placement = "above"; // set default
                        setSLinePlacement(b,
                                          score->spatium(), placement,
                                          hasYoffset, yoffset);

                        // TODO: MuseScore doesn't support hooks at beginning of lines

                        // hack: assume there was a words element before the bracket
                        if (!txt.isEmpty()) {
                              b->setBeginText(txt);
                              }

                        if (lineType == "solid")
                              b->setLineStyle(Qt::SolidLine);
                        else if (lineType == "dashed")
                              b->setLineStyle(Qt::DashLine);
                        else if (lineType == "dotted")
                              b->setLineStyle(Qt::DotLine);
                        else
                              printf("unsupported line-type: %s\n", lineType.toLatin1().data());

                        b->setTrack((staff + rstaff) * VOICES);
                        b->setTick(tick);
                        bracket[n] = b;
                        }
                  }
            else if (type == "stop") {
                  if (!b) {
                        printf("bracket stop without start, number %d\n", number);
                        }
                  else {
                        b->setTick2(tick);
                        // TODO: MuseScore doesn't support lines which start and end on different staves
                        /*
                        QPointF userOff = b->userOff();
                        b->add(b->createLineSegment());

                        b->setUserOff(QPointF()); // restore the offset
                        b->setMxmlOff2(offset);
                        LineSegment* ls1 = b->lineSegments().front();
                        LineSegment* ls2 = b->lineSegments().back();
                        // what does placement affect?
                        //yoffset += (placement == "above" ? 0.0 : 5.0);
                        ls1->setUserOff(userOff);
                        ls2->setUserOff2(QPointF(rx + xoffset, ry + yoffset));
                        */
                        b->setEndHook(lineEnd != "none");
                        if (lineEnd == "up")
                              b->setEndHookHeight(-1 * b->endHookHeight());
                        score->add(b);
                        bracket[n] = 0;
                        }
                  }
            }
      else if (dirType == "octave-shift") {
            if (type == "down") {
                  if (ottava) {
                        printf("overlapping octave-shift not supported\n");
                        delete ottava;
                        ottava = 0;
                        }
                  else {
                        ottava = new Ottava(score);
                        ottava->setTrack((staff + rstaff) * VOICES);
                        ottava->setTick(tick);
                        if (ottavasize == 8)
                              ottava->setSubtype(0);
                        else if (ottavasize == 15)
                              ottava->setSubtype(1);
                        else {
                              printf("unknown octave-shift size %d\n", ottavasize);
                              delete ottava;
                              ottava = 0;
                              }
                        if (placement == "") placement = "above"; // set default
                        if (ottava) setSLinePlacement(ottava,
                                          score->spatium(), placement,
                                          hasYoffset, yoffset);
                        }
                  }
            else if (type == "up") {
                  if (ottava) {
                        printf("overlapping octave-shift not supported\n");
                        delete ottava;
                        ottava = 0;
                        }
                  else {
                        ottava = new Ottava(score);
                        ottava->setTrack((staff + rstaff) * VOICES);
                        ottava->setTick(tick);
                        if (ottavasize == 8)
                              ottava->setSubtype(2);
                        else if (ottavasize == 15)
                              ottava->setSubtype(3);
                        else {
                              printf("unknown octave-shift size %d\n", ottavasize);
                              delete ottava;
                              ottava = 0;
                              }
                        if (placement == "") placement = "below"; // set default
                        if (ottava) setSLinePlacement(ottava,
                                          score->spatium(), placement,
                                          hasYoffset, yoffset);
                        }
                  }
            else if (type == "stop") {
                  if (!ottava) {
                        printf("octave-shift stop without start\n");
                        }
                  else {
                        ottava->setTick2(tick);
                        score->add(ottava);
                        ottava = 0;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   xmlAttributes
//---------------------------------------------------------

/**
 Read the MusicXML attributes element.
 */

// Standard order of attributes as written by Dolet for Finale is divisions,
// key, time, staves and clef(s). For the first measure this means number of
// staves is unknown when the time attributes is read. As this translates
// into a time signature that must be inserted into every staff of the
// part, delay insertion of time signatures until after all attributes
// have been read.

void MusicXml::xmlAttributes(Measure* measure, int staff, QDomElement e)
      {
      QString beats = "";
      QString beatType = "";
      QString timeSymbol = "";

      for (;!e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "divisions") {
                  bool ok;
                  divisions = stringToInt(e.text(), &ok);
                  if (!ok) {
                        printf("MusicXml-Import: bad divisions value: <%s>\n",
                           qPrintable(e.text()));
                              divisions = 4;
                        }
                  }
            else if (e.tagName() == "key") {
                  int number = e.attribute(QString("number"), "-1").toInt();
                  QString printObject(e.attribute("print-object", "yes"));
                  int staffIdx = staff;
                  if (number != -1)
                        staffIdx += number - 1;
                  KeySigEvent key;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "fifths")
                              key.setAccidentalType(ee.text().toInt());
                        else if (ee.tagName() == "mode")
                              domNotImplemented(ee);
                        else
                              domError(ee);
                        }
                  if (number == -1) {
                        //
                        //   apply key to all staves in the part
                        //   ws: number of staves is not always known at this
                        //       point, so we have to set key signature when
                        //       we see the "staves" tag
                        //
                        int staves = score->part(staff)->nstaves();
                        // apply to all staves in part
                        for (int i = 0; i < staves; ++i) {
                              KeySigEvent oldkey = score->staff(staffIdx+i)->keymap()->key(tick);
                              if (oldkey != key) {
                                    // new key differs from key in effect at this tick
                                    (*score->staff(staffIdx+i)->keymap())[tick] = key;
                                    KeySig* keysig = new KeySig(score);
                                    keysig->setTick(tick);
                                    keysig->setTrack((staffIdx + i) * VOICES);
                                    keysig->setSubtype(key);
                                    keysig->setVisible(printObject == "yes");
                                    Segment* s = measure->getSegment(keysig);
                                    s->add(keysig);
                                    }
                              }
                        }
                  else {
                        //
                        //    apply key to staff(staffIdx) only
                        //
                        KeySigEvent oldkey = score->staff(staffIdx)->keymap()->key(tick);
                        if (oldkey != key) {
                              // new key differs from key in effect at this tick
                              (*score->staff(staffIdx)->keymap())[tick] = key;
                              KeySig* keysig = new KeySig(score);
                              keysig->setTick(tick);
                              keysig->setTrack(staffIdx * VOICES);
                              keysig->setSubtype(key);
                              keysig->setVisible(printObject == "yes");
                              Segment* s = measure->getSegment(keysig);
                              s->add(keysig);
                              }
                        }
                  }
            else if (e.tagName() == "time") {
                  timeSymbol = e.attribute("symbol");
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "beats")
                              beats = ee.text();
                        else if (ee.tagName() == "beat-type") {
                              beatType = ee.text();
                              }
                        else if(ee.tagName() == "senza-misura")
                              ;
                        else
                              domError(ee);
                        }
                  }
            else if (e.tagName() == "clef")
                  xmlClef(e, staff, measure);
            else if (e.tagName() == "staves") {
                  int staves = e.text().toInt();
                  Part* part = score->part(staff);
                  part->setStaves(staves);
                  Staff* st = part->staff(0);
                  if (st && staves == 2) {
                        st->setBracket(0, BRACKET_AKKOLADE);
                        st->setBracketSpan(0, 2);
                        st->setBarLineSpan(2); //seems to be default in musicXML
                        }
                  // set key signature
                  KeySigEvent key = score->staff(staff)->keymap()->key(tick);
                  for (int i = 1; i < staves; ++i) {
                        KeySigEvent oldkey = score->staff(staff+i)->keymap()->key(tick);
                        if (oldkey != key)
                              (*score->staff(staff+i)->keymap())[tick] = key;
                        }
                  }
            else if (e.tagName() == "staff-details"){
                  int number  = e.attribute(QString("number"), "-1").toInt();
                  int staffIdx = staff;
                  if (number != -1)
                        staffIdx += number - 1;
                  int stafflines = 5;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "staff-lines")
                              stafflines = ee.text().toInt();
                        else
                              domNotImplemented(ee);
                  }

                  if (number == -1){
                      int staves = score->part(staff)->nstaves();
                      for (int i = 0; i < staves; ++i) {
                            score->staff(staffIdx+i)->setLines(stafflines);
                            measure->mstaff(staffIdx+i)->lines->setLines(stafflines);
                      }
                  }else{
                      score->staff(staffIdx)->setLines(stafflines);
                      measure->mstaff(staffIdx)->lines->setLines(stafflines);
                  }
            }
            else if (e.tagName() == "instruments")
                  domNotImplemented(e);
            else if (e.tagName() == "transpose") {
                  Interval interval;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        int i = ee.text().toInt();
                        if (ee.tagName() == "diatonic")
                              interval.diatonic = i;
                        else if (ee.tagName() == "chromatic")
                              interval.chromatic = i;
                        else
                              domError(ee);
                        }
                  score->part(staff)->setTranspose(interval);
                  }
            else if (e.tagName() == "measure-style")
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "multiple-rest") {
                              int multipleRest = ee.text().toInt();
                              if (multipleRest > 1) {
                                    multiMeasureRestCount = multipleRest - 1;
                                    startMultiMeasureRest = true;
                                    }
                              else
                                    printf("ImportMusicXml: multiple-rest %d not supported\n",
                                       multipleRest);
                              }
                        else
                              domError(ee);
                        }
            else
                  domError(e);
            }
      if (beats != "" && beatType != "") {
            // determine if timesig is valid
            int st = 0;  // timesig subtype calculated
            int bts[4];  // the beats (max 4 separated by "+") as integer
            int btp = 0; // beat-type as integer
            if (beats == "2" && beatType == "2" && timeSymbol == "cut") {
                  st = TSIG_ALLA_BREVE;
                  }
            else if (beats == "4" && beatType == "4" && timeSymbol == "common") {
                  st = TSIG_FOUR_FOUR;
                  }
            else  {
                  if (!timeSymbol.isEmpty()) {
                        printf("ImportMusicXml: time symbol <%s> not recognized with beats=%s and beat-type=%s\n",
                           qPrintable(timeSymbol), qPrintable(beats), qPrintable(beatType));
                        }

                  btp = beatType.toInt();
                  QStringList list = beats.split("+");
                  for (int i = 0; i < 4; i++) bts[i] = 0;
                  for (int i = 0; i < list.size() && i < 4; i++) {
                        bts[i] = list.at(i).toInt();
                        }
                  // the beat type and at least one beat must be non-zero
                  if (btp && (bts[0] || bts[1] || bts[2] || bts[3])) {
                        TimeSig ts = TimeSig(score, btp, bts[0], bts[1], bts[2], bts[3]);
                        st = ts.subtype();
                        }
                  }
            if (st) {
                  // add timesig to all staves
                  score->sigmap()->add(tick, TimeSig::getSig(st));
                  Part* part = score->part(staff);
                  int staves = part->nstaves();
                  for (int i = 0; i < staves; ++i) {
                        TimeSig* timesig = new TimeSig(score);
                        timesig->setTick(tick);
                        timesig->setSubtype(st);
                        timesig->setTrack((staff + i) * VOICES);
                        Segment* s = measure->getSegment(timesig);
                        s->add(timesig);
                        }
                  }
            else
                  printf("unknown time signature, beats=<%s> beat-type=<%s> symbol=<%s>\n",
                         beats.toLatin1().data(), beatType.toLatin1().data(),
                         timeSymbol.toLatin1().data());
            }
      }

//---------------------------------------------------------
//   xmlLyric
//---------------------------------------------------------

void MusicXml::xmlLyric(Measure* measure, int staff, QDomElement e)
      {
      int lyricNo = e.attribute(QString("number"), "1").toInt() - 1;
      if (lyricNo > MAX_LYRICS)
            printf("too much lyrics (>%d)\n", MAX_LYRICS);
      Lyrics* l = new Lyrics(score);
      l->setNo(lyricNo);
      l->setTick(tick);

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "syllabic") {
                  if (e.text() == "single")
                        l->setSyllabic(Lyrics::SINGLE);
                  else if (e.text() == "begin")
                        l->setSyllabic(Lyrics::BEGIN);
                  else if (e.text() == "end")
                        l->setSyllabic(Lyrics::END);
                  else if (e.text() == "middle")
                        l->setSyllabic(Lyrics::MIDDLE);
                  else
                        printf("unknown syllabic %s\n", qPrintable(e.text()));
                  }
            else if (e.tagName() == "text")
                  l->setText(l->getText()+e.text());
            else if (e.tagName() == "elision")
                  if (e.text().isEmpty()){
                        l->setText(l->getText()+" ");
                  }else{
                        l->setText(l->getText()+e.text());
                  }
            else if (e.tagName() == "extend")
                  ;
            else if (e.tagName() == "end-line")
                  ;
            else if (e.tagName() == "end-paragraph")
                  ;
            else
                  domError(e);
            }
      l->setTrack(staff * VOICES);
      Segment* segment = measure->getSegment(l);
      segment->add(l);
      }

//---------------------------------------------------------
//   hasElem
//---------------------------------------------------------

/**
 Determine if \a e has a child \a tagname.
 */

static bool hasElem(const QDomElement e, const QString& tagname)
      {
      return !e.elementsByTagName(tagname).isEmpty();
      }

//---------------------------------------------------------
//   nrOfGraceSegsReq
//---------------------------------------------------------

/**
 Determine the number of grace segments required for the grace note represented by \a n.
 */

static int nrOfGraceSegsReq(QDomNode n)
      {
      if (!n.isElement() || n.nodeName() != "note") return 0;
      QDomElement e = n.toElement();
      if (!hasElem(e, "grace")) return 0;
      int nsegs = 0;
      // when counting starts in a grace chord but not at the first note,
      // compensate for the missed first note
      if (hasElem(e, "chord")) nsegs = 1;
      // count the number of grace chords
      // i.e the number of notes with grace but without chord child elements
      for (; !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag != "note") {
                  ;
//                  printf("nrOfGraceSegsReq: found non-note tag '%s'\n",
//                         qPrintable(tag));
                  }
            if (!hasElem(e, "grace"))
                  // non-grace note found, done
                  return nsegs;
            if (!hasElem(e, "chord"))
                  // first note of grace chord found
                  ++nsegs;
            }
      return 0;
      }

//---------------------------------------------------------
//   xmlNote
//---------------------------------------------------------

/**
 Read a MusicXML note.

 \a Staff is the number of first staff of the part this note belongs to.
 */

void MusicXml::xmlNote(Measure* measure, int staff, QDomElement e)
      {
//      printf("xmlNote start tick=%d\n", tick);
      QDomNode pn = e.parentNode();
      voice = 0;
      move  = 0;

      int duration = 0;
      bool rest    = false;
      int relStaff = 0;
      BeamMode bm  = BEAM_AUTO;
      Direction sd = AUTO;
      int dots     = 0;
      bool grace   = false;
      QString arpeggioType;
      QString fermataType;
      QString glissandoType;
      QString tupletType;
      QString tupletPlacement;
      QString tupletBracket;
      QString step;
      QString fingering;
      QString pluck;
      QString string;
      QString graceSlash;
      QString wavyLineType;
      int alter  = 0;
      int octave = 4;
      int accidental = 0;
      bool editorial = false;
      Duration durationType(Duration::V_INVALID);
      bool trillMark = false;
      QString strongAccentType;
      bool accent = false;
      bool breathmark = false;
      bool staccatissimo = false;
      bool staccato = false;
      bool tenuto = false;
      bool turn = false;
      bool mordent = false;
      bool invertedMordent = false;
      bool invertedTurn = false;
      bool stopped = false;
      bool upbow = false;
      bool downbow = false;
      int actualNotes = 1;
      int normalNotes = 1;
      int tremolo = 0;
      QString tremoloType;
      int headGroup = 0;
      bool noStem = false;
      QString placement;
      QStringList dynamics;
      qreal rx = 0.0;
      qreal ry = 0.0;
      qreal yoffset = 0.0; // actually this is default-y
      qreal xoffset = 0.0;
      bool hasYoffset = false;
      QColor noteheadColor = QColor::Invalid;

      QString printObject = "yes";
      if (pn.isElement() && pn.nodeName() == "note") {
            QDomElement pne = pn.toElement();
            printObject = pne.attribute("print-object", "yes");
            }

      for (; !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString s(e.text());

            if (tag == "pitch") {
                  step   = "C";
                  alter  = 0;
                  octave = 4;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "step")          // A-G
                              step = ee.text();
                        else if (ee.tagName() == "alter") {    // -1=flat 1=sharp (0.5=quarter sharp)
                              bool ok;
                              alter = stringToInt(ee.text(), &ok); // fractions not supported by mscore
                              if (!ok || alter < -2 || alter > 2) {
                                    printf("ImportXml: bad 'alter' value: %s at line %d col %d\n",
                                          qPrintable(ee.text()), ee.lineNumber(), ee.columnNumber());
                                    alter = 0;
                                    }
                              }
                        else if (ee.tagName() == "octave")   // 0-9 4=middle C
                              octave = ee.text().toInt();
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "unpitched") {
                  //
                  // TODO: check semantic
                  //
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "display-step")          // A-G
                              step = ee.text();
                        else if (ee.tagName() == "display-octave")   // 0-9 4=middle C
                              octave = ee.text().toInt();
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "duration") {
                  bool ok;
                  duration = stringToInt(s, &ok);
                  if (!ok) {
                        printf("MusicXml-Import: bad note duration value: <%s>\n",
                           qPrintable(s));
                              duration = 1;
                        }
                  }
            else if (tag == "type")
                  durationType = Duration(s);
            else if (tag == "chord") {
                  if (!grace)
                        tick -= lastLen;
                  }
            else if (tag == "voice")
                  voice = s.toInt() - 1;
            else if (tag == "stem") {
                  if (s == "up")
                        sd = UP;
                  else if (s == "down")
                        sd = DOWN;
                  else if (s == "none")
                        noStem = true;
                  else if (s == "double")
                        ;
                  else
                        printf("unknown stem direction %s\n", e.text().toLatin1().data());
                  }
            else if (tag == "staff")
                  relStaff = s.toInt() - 1;
            else if (tag == "beam") {
                  int beamNo = e.attribute(QString("number"), "1").toInt();
                  if (beamNo == 1) {
                        if (s == "begin")
                              bm = BEAM_BEGIN;
                        else if (s == "end")
                              bm = BEAM_END;
                        else if (s == "continue")
                              bm = BEAM_MID;
                        else if (s == "backward hook")
                              ;
                        else if (s == "forward hook")
                              ;
                        else
                              printf("unknown beam keyword <%s>\n", s.toLatin1().data());
                        }
                  }
            else if (tag == "rest") {
                  rest = true;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "display-step")          // A-G
                              step = ee.text();
                        else if (ee.tagName() == "display-octave")   // 0-9 4=middle C
                              octave = ee.text().toInt();
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "lyric")
                  xmlLyric(measure, relStaff + staff, e);
            else if (tag == "dot")
                  ++dots;
            else if (tag == "accidental") {
                  if (e.attribute(QString("editorial")) == "yes")
                        editorial = true;
                  if (s == "natural")
                        accidental = 5;
                  else if (s == "flat")
                        accidental = 2;
                  else if (s == "sharp")
                        accidental = 1;
                  else if (s == "double-sharp")
                        accidental = 3;
                  else if (s == "sharp-sharp")
                        accidental = 3;
                  else if (s == "natural-flat")
                        ;
                  else if (s == "quarter-flat")
                        accidental = 19;
                  else if (s == "quarter-sharp")
                        accidental = 22;
                  else if (s == "three-quarters-flat")
                        accidental = 18;
                  else if (s == "three-quarters-sharp")
                        accidental = 25;
                  else if (s == "flat-flat")
                        accidental = 4;
                  else if (s == "natural-sharp")
                        ;
                  else
                        printf("unknown accidental %s\n", s.toLatin1().data());
                  }
            else if (tag == "notations") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "slur") {
                              int slurNo   = ee.attribute(QString("number"), "1").toInt() - 1;
                              QString slurType = ee.attribute(QString("type"));

                              int trk = (staff + relStaff) * VOICES;
                              if (slurType == "start") {
                                    bool endSlur = false;
                                    if (slur[slurNo] == 0)
                                          slur[slurNo] = new Slur(score);
                                    else
                                          endSlur = true;
                                    QString pl = ee.attribute(QString("placement"));
                                    if (pl == "above")
                                          slur[slurNo]->setSlurDirection(UP);
                                    else if (pl == "below")
                                          slur[slurNo]->setSlurDirection(DOWN);
                                    slur[slurNo]->setStart(tick, trk + voice);
                                    slur[slurNo]->setTrack((staff + relStaff) * VOICES);
                                    score->add(slur[slurNo]);
                                    if (endSlur)
                                          slur[slurNo] = 0;
                                    }
                              else if (slurType == "stop") {
                                    if (slur[slurNo] == 0) {
                                          slur[slurNo] = new Slur(score);
                                          slur[slurNo]->setEnd(tick, trk + voice);
                                          }
                                    else {
                                          slur[slurNo]->setEnd(tick, trk + voice);
                                          slur[slurNo] = 0;
                                          }
                                    }
                              else
                                    printf("unknown slur type %s\n", slurType.toLatin1().data());
                              }
                        else if (ee.tagName() == "tied") {
                              QString tiedType = ee.attribute(QString("type"));
                              if (tiedType == "start") {
                                    if (tie) {
                                          printf("Tie already active\n");
                                          }
                                    else {
                                          tie = new Tie(score);
                                          }
                                    QString tiedOrientation = e.attribute("orientation", "auto");
                                    if (tiedOrientation == "over")
                                          tie->setSlurDirection(UP);
                                    else if (tiedOrientation == "under")
                                          tie->setSlurDirection(DOWN);
                                    else if (tiedOrientation == "auto")
                                          ;
                                    else
                                          printf("unknown tied orientation: %s\n", tiedOrientation.toLatin1().data());
                                          ;
                                    }
                              else if (tiedType == "stop")
                                    ;
                              else
                                    printf("unknown tied type %s\n", tiedType.toLatin1().data());
                              }
                        else if (ee.tagName() == "tuplet") {
                              tupletType      = ee.attribute(QString("type"));
                              tupletPlacement = ee.attribute("placement");
                              tupletBracket   = ee.attribute("bracket");
                              }
                        else if (ee.tagName() == "dynamics") {
                              placement = ee.attribute("placement");
                              ry        = ee.attribute(QString("relative-y"), "0").toDouble() * -.1;
                              rx        = ee.attribute(QString("relative-x"), "0").toDouble() * .1;
                              yoffset   = ee.attribute("default-y").toDouble(&hasYoffset) * -0.1;
                              xoffset   = ee.attribute("default-x", "0.0").toDouble() * 0.1;
                              QDomElement eee = ee.firstChildElement();
                              if (!eee.isNull()) {
                                    if (eee.tagName() == "other-dynamics")
                                          dynamics.push_back(eee.text());
                                    else
                                          dynamics.push_back(eee.tagName());
                                    }
                              }
                        else if (ee.tagName() == "articulations") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    if (eee.tagName() == "accent")
                                          accent = true;
                                    else if (eee.tagName() == "breath-mark")
                                          breathmark = true;
                                    else if (eee.tagName() == "staccatissimo")
                                          staccatissimo = true;
                                    else if (eee.tagName() == "staccato")
                                          staccato = true;
                                    else if (eee.tagName() == "strong-accent")
                                          strongAccentType = eee.attribute(QString("type"));
                                    else if (eee.tagName() == "tenuto")
                                          tenuto = true;
                                    else
                                          domError(eee);
                                    }
                              }
                        else if (ee.tagName() == "fermata") {
                              fermataType = ee.attribute(QString("type"));
                              }
                        else if (ee.tagName() == "ornaments") {
                              // <trill-mark placement="above"/>
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    if (eee.tagName() == "trill-mark")
                                          trillMark = true;
                                    else if (eee.tagName() == "wavy-line")
                                          wavyLineType = eee.attribute(QString("type"));
                                    else if (eee.tagName() == "inverted-turn")
                                          invertedTurn = true;
                                    else if (eee.tagName() == "turn")
                                          turn = true;
                                    else if (eee.tagName() == "inverted-mordent")
                                          invertedMordent = true;
                                    else if (eee.tagName() == "mordent")
                                          mordent = true;
                                    else if (eee.tagName() == "tremolo") {
                                          tremolo = eee.text().toInt();
                                          tremoloType = eee.attribute(QString("type"));
                                          }
                                    else if (eee.tagName() == "accidental-mark")
                                          domNotImplemented(eee);
                                    else if (eee.tagName() == "delayed-turn")
                                          domNotImplemented(eee);
                                    else
                                          domError(eee);
                                    }
                              }
                        else if (ee.tagName() == "technical") {
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    if (eee.tagName() == "fingering")
                                          fingering = eee.text();
                                    else if (eee.tagName() == "fret")
                                          domNotImplemented(eee);
                                    else if (eee.tagName() == "pluck")
                                          pluck = eee.text();
                                    else if (eee.tagName() == "string")
                                          string = eee.text();
                                    else if (eee.tagName() == "pull-off")
                                          domNotImplemented(eee);
                                    else if (eee.tagName() == "stopped")
                                          stopped = true;
                                    else if (eee.tagName() == "up-bow")
                                          upbow = true;
                                    else if (eee.tagName() == "down-bow")
                                          downbow = true;
                                    else
                                          domError(eee);
                                    }
                              }
                        else if (ee.tagName() == "arpeggiate") {
                              arpeggioType = ee.attribute(QString("direction"));
                              if (arpeggioType == "") arpeggioType = "none";
                              }
                        else if (ee.tagName() == "non-arpeggiate")
                              arpeggioType = "non-arpeggiate";
                        // glissando and slide are added to the "stop" chord only
                        else if (ee.tagName() == "glissando") {
                              if (ee.attribute("type") == "stop") glissandoType = "glissando";
                              }
                        else if (ee.tagName() == "slide") {
                              if (ee.attribute("type") == "stop") glissandoType = "slide";
                              }
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "tie") {
                  QString tieType = e.attribute(QString("type"));
                  if (tieType == "start")
                        ;
                  else if (tieType == "stop")
                        ;
                  else
                        printf("unknown tie type %s\n", tieType.toLatin1().data());
                  }
            else if (tag == "grace") {
                  grace = true;
                  graceSlash = e.attribute(QString("slash"));
                  }
            else if (tag == "time-modification") {  // tuplets
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "actual-notes")
                              actualNotes = ee.text().toInt();
                        else if (ee.tagName() == "normal-notes")
                              normalNotes = ee.text().toInt();
                        else if (ee.tagName() == "normal-type")
                              domNotImplemented(ee);
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "notehead") {
                  if (s == "slash")
                        headGroup = 5;
                  else if (s == "triangle")
                        headGroup = 3;
                  else if (s == "diamond")
                        headGroup = 2;
                  else if (s == "x")
                        headGroup = 1;
                  else if (s == "circle-x")
                        headGroup = 6;
                  else if (s == "do")
                        headGroup = 7;
                  else if (s == "re")
                        headGroup = 8;
                  else if (s == "mi")
                        headGroup = 4;
                  else if (s == "fa")
                        headGroup = 9;
                  else if (s == "la")
                        headGroup = 10;
                  else if (s == "ti")
                        headGroup = 11;
                  else if (s == "normal")
                        ;
                  else
                        printf("unknown notehead %s\n", qPrintable(s));
                  QString color = e.attribute(QString("color"), 0);
                  if (color != 0)
                        noteheadColor = QColor(color);
                  }
            else if (tag == "instrument")
                  domNotImplemented(e);
            else if (tag == "cue")
                  domNotImplemented(e);
            else
                  domError(e);
            }

      // Musicxml voices are counted for all staffs of an
      // instrument. They are not limited. In mscore voices are associated
      // with a staff. Every staff can have at most VOICES voices.
      // The following lines map musicXml voices to mscore voices.
      // If a voice crosses two staffs, this is expressed with the
      // "move" parameter in mscore.

      // Musicxml voices are unique within a part, but not across parts.
      // LVIFIX: check: Thus the search for a given MusicXML voice number should be restricted
      // the the staves of the part it belongs to.

//      printf("voice mapper before: relStaff=%d voice=%d", relStaff, voice);
      int found = false;
      for (int s = 0; s < MAX_STAVES; ++s) {
            int v = 0;
            for (std::vector<int>::iterator i = voicelist[s].begin(); i != voicelist[s].end(); ++i, ++v) {
                  if (*i == voice) {
                        int d = relStaff - s;
                        relStaff = s;
                        move += d;
                        voice = v;
//                        printf(" -> found at s=%d\n", s);
                        found = true;
                        break;
                        }
                  }
            } // for (int s ...
      if (!found) {
            if (voicelist[relStaff].size() >= unsigned(VOICES))
                  printf("ImportMusicXml: too many voices (staff %d, relStaff %d, %d >= %d)\n",
                         staff, relStaff, voicelist[relStaff].size(), VOICES);
            else {
                  voicelist[relStaff].push_back(voice);
//                  printf(" -> append %d to voicelist[%d]\n", voice, relStaff);
                  voice = voicelist[relStaff].size() -1;
                  }
            }
//      printf("after: relStaff=%d move=%d voice=%d\n", relStaff, move, voice);

      int track = (staff + relStaff) * VOICES + voice;
//      printf("staff=%d relStaff=%d VOICES=%d voice=%d track=%d\n",
//             staff, relStaff, VOICES, voice, track);

      int ticks = (AL::division * duration) / divisions;

      if (!grace && tick + ticks > maxtick)
            maxtick = tick + ticks;

//      printf("%s at %d voice %d dur = %d, beat %d/%d div %d pitch %d ticks %d\n",
//         rest ? "Rest" : "Note", tick, voice, duration, beats, beatType,
//         divisions, 0 /* pitch */, ticks);

      ChordRest* cr = 0;

      if (rest) {
            // whole measure rests do not have a "type" element
            int len = ticks;
            if (durationType.type() == Duration::V_INVALID) {
                  durationType.setType(Duration::V_MEASURE);
                  len = 0;
                  }
            cr = new Rest(score, tick, durationType);
            cr->setDots(dots);
            if (beamMode == BEAM_BEGIN || beamMode == BEAM_MID)
                  cr->setBeamMode(BEAM_MID);
            else
                  cr->setBeamMode(BEAM_NO);
            cr->setTrack(track);
            ((Rest*)cr)->setStaffMove(move);
            Segment* s = measure->getSegment(cr);
            s->add(cr);
            cr->setVisible(printObject == "yes");
            if (step != "" && 0 <= octave && octave <= 9) {
                  // printf("rest step=%s oct=%d", qPrintable(step), octave);
                  int clef = cr->staff()->clefList()->clef(tick);
                  int po = clefTable[clef].pitchOffset;
                  int istep = step[0].toAscii() - 'A';
                  // printf(" clef=%d po=%d istep=%d\n", clef, po, istep);
                  if (istep < 0 || istep > 6) {
                        printf("rest: illegal display-step %d, <%s>\n", istep, qPrintable(step));
                        }
                  else {
                        //                        a  b  c  d  e  f  g
                        static int table2[7]  = { 5, 6, 0, 1, 2, 3, 4 };
                        int dp = 7 * (octave + 2) + table2[istep];
                        // printf("dp=%d\n", dp);
                        cr->setUserYoffset((po - dp + 3) * score->spatium() / 2);
                        }
                  }
            }
      else {
            char c     = step[0].toLatin1();
            Note* note = new Note(score);
            note->setHeadGroup(headGroup);
            note->setTrack(track);
            note->setColor(noteheadColor);
            // note->setStaffMove(move);

            if (!fingering.isEmpty()) {
                  Text* f = new Text(score);
                  f->setSubtype(TEXT_FINGERING);
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->setText(fingering);
                  note->add(f);
                  }

            if (!pluck.isEmpty()) {
                  Text* f = new Text(score);
                  f->setSubtype(TEXT_FINGERING);
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->setText(pluck);
                  note->add(f);
                  }

            if (!string.isEmpty()) {
                  Text* f = new Text(score);
                  f->setSubtype(TEXT_STRING_NUMBER);
                  f->setTextStyle(TEXT_STYLE_STRING_NUMBER);
                  f->setText(string);
                  note->add(f);
                  }

            if (tie) {
                  note->setTieFor(tie);
                  tie->setStartNote(note);
                  tie->setTrack(track);
                  tie = 0;
                  }

//            if (grace)
//                  printf(" grace: nrOfGraceSegsReq: %d\n", nrOfGraceSegsReq(pn));
            int gl = nrOfGraceSegsReq(pn);
            cr = measure->findChord(tick, track, grace);
            if (cr == 0) {
                  SegmentType st = SegChordRest;
                  cr = new Chord(score);
                  cr->setTick(tick);
                  cr->setBeamMode(bm);
                  cr->setTrack(track);
//                  printf(" grace=%d\n", grace);
                  if (grace) {
                        NoteType nt = NOTE_APPOGGIATURA;
                        if (graceSlash == "yes")
                              nt = NOTE_ACCIACCATURA;
                        ((Chord*)cr)->setNoteType(nt);
                        // the subtraction causes a grace at tick=0 to fail
                        // cr->setTick(tick - (AL::division / 2));
                        cr->setTick(tick);
                        if (durationType.type() == Duration::V_QUARTER) {
                              ((Chord*)cr)->setNoteType(NOTE_GRACE4);
                              cr->setDuration(Duration::V_QUARTER);
                              }
                        else if (durationType.type() == Duration::V_16TH) {
                              ((Chord*)cr)->setNoteType(NOTE_GRACE16);
                              cr->setDuration(Duration::V_16TH);
                              }
                        else if (durationType.type() == Duration::V_32ND) {
                              ((Chord*)cr)->setNoteType(NOTE_GRACE32);
                              cr->setDuration(Duration::V_32ND);
                              }
                        else
                              cr->setDuration(Duration::V_EIGHT);
                        st = SegGrace;
                        }
                  else {
                        if (durationType.type() == Duration::V_INVALID)
                              durationType.setType(Duration::V_QUARTER);
                        cr->setDuration(durationType);
                        }
                  cr->setDots(dots);
//                  printf(" cr->tick()=%d ", cr->tick());
                  Segment* s = measure->getSegment(st, cr->tick(), gl);
                  s->add(cr);
                  }
//            printf(" cr->tick()=%d", cr->tick());
            cr->setStaffMove(move);

            // pitch must be set before adding note to chord as note
            // is inserted into pitch sorted list (ws)

            xmlSetPitch(note, c, alter, octave, ottava, track);
            cr->add(note);

            ((Chord*)cr)->setNoStem(noStem);

//            printf("staff for new note: %p (staff=%d, relStaff=%d)\n",
//                   score->staff(staff + relStaff), staff, relStaff);
            // LVIFIX: accidental handling is ugly, replace magic numbers by constants
            if (1 <= accidental &&  accidental <= 5 && editorial)
                  note->setUserAccidental(accidental + 0x8000);
            // LVIFIX: quarter tone accidentals support is "drawing only"
            if (accidental == 18
                || accidental == 19
                || accidental == 22
                || accidental == 25)
                  note->setAccidentalType(accidental);

            if (cr->beamMode() == BEAM_NO)
                  cr->setBeamMode(bm);
            // remember beam mode last non-grace note
            // bm == BEAM_AUTO means no <beam> was found
            if (!grace && bm != BEAM_AUTO)
                  beamMode = bm;
            ((Chord*)cr)->setStemDirection(sd);

            note->setVisible(printObject == "yes");
            }
      if (!fermataType.isEmpty()) {
            Articulation* f = new Articulation(score);
            if (fermataType == "upright") {
                  f->setSubtype(UfermataSym);
                  cr->add(f);
                  }
            else if (fermataType == "inverted") {
                  f->setSubtype(DfermataSym);
                  f->setUserYoffset(5.3); // force below note (albeit by brute force)
                  cr->add(f);
                  }
            else {
                  printf("unknown fermata type %s\n", fermataType.toLatin1().data());
                  delete f;
                  }
            }
      if (!arpeggioType.isEmpty()) {
            Arpeggio* a = new Arpeggio(score);
            if (arpeggioType == "none")
                  a->setSubtype(0);
            else if (arpeggioType == "up")
                  a->setSubtype(1);
            else if (arpeggioType == "down")
                  a->setSubtype(2);
            else if (arpeggioType == "non-arpeggiate")
                  a->setSubtype(3);
            else {
                  printf("unknown arpeggio type %s\n", arpeggioType.toLatin1().data());
                  delete a;
                  a = 0;
                  }
            if ((static_cast<Chord*>(cr))->arpeggio()) {
                  // there can be only one
                  delete a;
                  a = 0;
                  }
            else
                  cr->add(a);
            }
      if (!glissandoType.isEmpty()) {
            Glissando* g = new Glissando(score);
            if (glissandoType == "slide")
                  g->setSubtype(0);
            else if (glissandoType == "glissando")
                  g->setSubtype(1);
            else {
                  printf("unknown glissando type %s\n", glissandoType.toLatin1().data());
                  delete g;
                  g = 0;
                  }
            if ((static_cast<Chord*>(cr))->glissando()) {
                  // there can be only one
                  delete g;
                  g = 0;
                  }
            else
                  cr->add(g);
            }
      if (!strongAccentType.isEmpty()) {
            Articulation* na = new Articulation(score);
            if (strongAccentType == "up") {
                  na->setSubtype(UmarcatoSym);
                  cr->add(na);
                  }
            else if (strongAccentType == "down") {
                  na->setSubtype(DmarcatoSym);
//                  f->setUserYoffset(5.3); // force below note (albeit by brute force)
                  cr->add(na);
                  }
            else {
                  printf("unknown mercato type %s\n", strongAccentType.toLatin1().data());
                  delete na;
                  }
            }
      bool wavyLineStart = false;
      if (!wavyLineType.isEmpty()) {
            if (wavyLineType == "start") {
                  if (trill) {
                        printf("overlapping wavy-line not supported\n");
                        delete trill;
                        trill = 0;
                        }
                  else {
                        trill = new Trill(score);
                        trill->setTrack((staff + relStaff) * VOICES);
                        trill->setTick(tick);
                        wavyLineStart = true;
                        }
                  }
            else if (wavyLineType == "stop") {
                  if (!trill) {
                        printf("wavy-line stop without start\n");
                        }
                  else {
                        trill->setTick2(tick+ticks);
                        score->add(trill);
                        trill = 0;
                        }
                  }
            else
                  printf("unknown wavy-line type %s\n", wavyLineType.toLatin1().data());
            }
      // note that mscore wavy line already implicitly includes a trillsym
      // so don't add an additional one
      if (trillMark && !wavyLineStart) {
            Articulation* na = new Articulation(score);
            na->setSubtype(TrillSym);
            cr->add(na);
            }
      if (invertedTurn) {
            Articulation* na = new Articulation(score);
            na->setSubtype(ReverseturnSym);
            cr->add(na);
            }
      if (turn) {
            Articulation* na = new Articulation(score);
            na->setSubtype(TurnSym);
            cr->add(na);
            }
      if (mordent) {
            Articulation* na = new Articulation(score);
            na->setSubtype(MordentSym);
            cr->add(na);
            }
      if (invertedMordent) {
            Articulation* na = new Articulation(score);
            na->setSubtype(PrallSym);
            cr->add(na);
            }
      if (accent) {
            Articulation* na = new Articulation(score);
            na->setSubtype(SforzatoaccentSym);
            cr->add(na);
            }
      if (staccatissimo) {
            Articulation* na = new Articulation(score);
            na->setSubtype(UstaccatissimoSym);
            cr->add(na);
            }
      if (staccato) {
            Articulation* na = new Articulation(score);
            na->setSubtype(StaccatoSym);
            cr->add(na);
            }
      if (tenuto) {
            Articulation* na = new Articulation(score);
            na->setSubtype(TenutoSym);
            cr->add(na);
            }
      if (stopped) {
            Articulation* na = new Articulation(score);
            na->setSubtype(PlusstopSym);
            cr->add(na);
            }
      if (upbow) {
            Articulation* na = new Articulation(score);
            na->setSubtype(UpbowSym);
            cr->add(na);
            }
      if (downbow) {
            Articulation* na = new Articulation(score);
            na->setSubtype(DownbowSym);
            cr->add(na);
            }
      if (breathmark) {
            Breath* b = new Breath(score);
            b->setTick(tick);
            b->setTrack((staff + relStaff) * VOICES + voice);
            Segment* seg = measure->getSegment(SegBreath, tick);
            seg->add(b);
            }
      if (!tupletType.isEmpty()) {
            if (tupletType == "start") {
                  tuplet = new Tuplet(score);
                  tuplet->setTrack((staff + relStaff) * VOICES);
                  tuplet->setRatio(Fraction(actualNotes, normalNotes));
                  tuplet->setTick(tick);

                  // type, placement

                  measure->add(tuplet);
                  }
            else if (tupletType == "stop") {
                  if (tuplet) {
                        cr->setTuplet(tuplet);
                        tuplet->add(cr);
                        int totalDuration = 0;
                        foreach(DurationElement* de, tuplet->elements()) {
                              if (de->type() == CHORD || de->type() == REST){
                                    totalDuration+=de->tickLen();
                                    }
                              }
                        if(totalDuration && normalNotes){
                              Duration d;
                              d.setVal(totalDuration);
                              tuplet->setFraction(d.fraction()); 
                              Duration d2;
                              d2.setVal(totalDuration/normalNotes);
                              tuplet->setBaseLen(d2.fraction());
                              tuplet = 0;
                              }
                        else{
                              printf("MusicXML::import: tuplet stop but bad duration\n");
                              }
                        }
                  else
                        printf("MusicXML::import: tuplet stop without tuplet start\n");
                  }
            else
                  printf("unknown tuplet type %s\n", tupletType.toLatin1().data());
            }
      if (tuplet) {
            cr->setTuplet(tuplet);
            tuplet->add(cr);
            }
      if (tremolo) {
            // printf("tremolo=%d tremoloType='%s'\n", tremolo, qPrintable(tremoloType));
            if (tremolo == 1 || tremolo == 2 || tremolo == 3) {
                  if (tremoloType == "" || tremoloType == "single") {
                        Tremolo * t = new Tremolo(score);
                        if (tremolo == 1) t->setSubtype(TREMOLO_1);
                        if (tremolo == 2) t->setSubtype(TREMOLO_2);
                        if (tremolo == 3) t->setSubtype(TREMOLO_3);
                        cr->add(t);
                        }
                  else if (tremoloType == "start") {
                        if (tremStart) printf("MusicXML::import: double tremolo start\n");
                        tremStart = static_cast<Chord*>(cr);
                        }
                  else if (tremoloType == "stop") {
                        if (tremStart) {
                              Tremolo * t = new Tremolo(score);
                              if (tremolo == 1) t->setSubtype(3);
                              if (tremolo == 2) t->setSubtype(4);
                              if (tremolo == 3) t->setSubtype(5);
                              t->setChords(tremStart, static_cast<Chord*>(cr));
                              cr->add(t);
                              }
                        else printf("MusicXML::import: double tremolo stop w/o start\n");
                        tremStart = 0;
                        }
                  }
            else
                  printf("unknown tremolo type %d\n", tremolo);
            }

      // more than one dynamic ???
      // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
      // TODO remove duplicate code (see MusicXml::direction)
      for (QStringList::Iterator it = dynamics.begin(); it != dynamics.end(); ++it ) {
            Dynamic* dyn = new Dynamic(score);
            dyn->setSubtype(*it);
            if (hasYoffset) dyn->setYoff(yoffset);
            else dyn->setAbove(placement == "above");
            dyn->setUserOff(QPointF(rx, ry));
            // dyn->setMxmlOff(offset);
            dyn->setTrack(track);
            dyn->setTick(tick);
            measure->add(dyn);
            }

      if (!grace) {
            lastLen = ticks;
            tick += ticks;
            }
//      printf(" after inserting note tick=%d\n", tick);
      }

//---------------------------------------------------------
//   addWedge
//---------------------------------------------------------

/**
 Add a MusicXML wedge to the wedge list.

 Called when the wedge start is read. Stores all wedge parameters known at this time.
 */

void MusicXml::addWedge(int no, int startTick, qreal rx, qreal ry, bool above, bool hasYoffset, qreal yoffset, int subType)
      {
      MusicXmlWedge wedge;
      wedge.number = no;
      wedge.startTick = startTick;
      wedge.rx = rx;
      wedge.ry = ry;
      wedge.above = above;
      wedge.hasYoffset = hasYoffset;
      wedge.yoffset = yoffset;
      wedge.subType = subType;

      if (int(wedgeList.size()) > no)
            wedgeList[no] = wedge;
      else
            wedgeList.push_back(wedge);
      }

//---------------------------------------------------------
//   genWedge
//---------------------------------------------------------

/**
 Add a MusicXML wedge to the score.

 Called when the wedge stop is read. Wedge stop tick was unknown until this time.
 */

void MusicXml::genWedge(int no, int endTick, Measure* /*measure*/, int staff)
      {
      Hairpin* hp = new Hairpin(score);

      hp->setTick(wedgeList[no].startTick);
      hp->setTick2(endTick);
      hp->setSubtype(wedgeList[no].subType);
      if (wedgeList[no].hasYoffset) hp->setYoff(wedgeList[no].yoffset);
      else hp->setYoff(wedgeList[no].above ? -3 : 8);
      hp->setUserOff(QPointF(wedgeList[no].rx, wedgeList[no].ry));
      hp->setTrack(staff * VOICES);
      if(hp->check())
          score->add(hp);

// printf("gen wedge %p staff %d, tick %d-%d\n", hp, staff, hp->tick(), hp->tick2());
      }

//---------------------------------------------------------
//   xmlHarmony
//---------------------------------------------------------

void MusicXml::xmlHarmony(QDomElement e, int tick, Measure* measure)
      {
      // type:

      // placement:
      double rx = e.attribute("relative-x", "0").toDouble()*0.1;
      double ry = e.attribute("relative-y", "0").toDouble()*-0.1;

      double styleYOff = score->textStyle(TEXT_STYLE_HARMONY)->yoff;
      OffsetType offsetType = score->textStyle(TEXT_STYLE_HARMONY)->offsetType;
      if (offsetType == OFFSET_ABS) {
            styleYOff = styleYOff * DPMM / score->spatium();
            }

      double dy = e.attribute("default-y", QString::number(styleYOff*-10)).toDouble()*-0.1;

      QString printObject(e.attribute("print-object", "yes"));
      QString printFrame(e.attribute("print-frame"));
      QString printStyle(e.attribute("print-style"));

      QString kind, kindText;
      QList<HDegree> degreeList;

      Harmony* ha = new Harmony(score);
      ha->setUserOff(QPointF(rx, ry + dy - styleYOff));
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "root") {
                  QString step;
                  int alter = 0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "root-step") {
                              // attributes: print-style
                              step = ee.text();
                              }
                        else if (tag == "root-alter") {
                              // attributes: print-object, print-style
                              //             location (left-right)
                              alter = ee.text().toInt();
                              }
                        else
                              domError(ee);
                        }
                  ha->setRootTpc(step2tpc(step, alter));
                  }
            else if (tag == "function") {
                  // attributes: print-style
                  domNotImplemented(e);
                  }
            else if (tag == "kind") {
                  // attributes: use-symbols  yes-no
                  //             text, stack-degrees, parentheses-degree, bracket-degrees,
                  //             print-style, halign, valign

                  kindText = e.attribute("text");
                  kind = e.text();
                  }
            else if (tag == "inversion") {
                  // attributes: print-style
                  }
            else if (tag == "bass") {
                  QString step;
                  int alter = 0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "bass-step") {
                              // attributes: print-style
                              step = ee.text();
                              }
                        else if (tag == "bass-alter") {
                              // attributes: print-object, print-style
                              //             location (left-right)
                              alter = ee.text().toInt();
                              }
                        else
                              domError(ee);
                        }
                  ha->setBaseTpc(step2tpc(step, alter));
                  }
            else if (tag == "degree") {
                  int degreeValue = 0;
                  int degreeAlter = 0;
                  QString degreeType = "";
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "degree-value") {
                              degreeValue = ee.text().toInt();
                              }
                        else if (tag == "degree-alter") {
                              degreeAlter = ee.text().toInt();
                              }
                        else if (tag == "degree-type") {
                              degreeType = ee.text();
                              }
                        else
                              domError(ee);
                        }
                  if (degreeValue <= 0 || degreeValue > 13
                      || degreeAlter < -2 || degreeAlter > 2
                      || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                        printf("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s\n",
                               degreeValue, degreeAlter, qPrintable(degreeType));
                        }
                  else {
                        if (degreeType == "add")
                              degreeList << HDegree(degreeValue, degreeAlter, ADD);
                        else if (degreeType == "alter")
                              degreeList << HDegree(degreeValue, degreeAlter, ALTER);
                        else if (degreeType == "subtract")
                              degreeList << HDegree(degreeValue, degreeAlter, SUBTRACT);
                        }
                  }
            else if (tag == "level") {
                  domNotImplemented(e);
                  }
            else if (tag == "offset") {
                  domNotImplemented(e);
                  }
            else
                  domError(e);
            }

      ha->setTick(tick);

      const ChordDescription* d = ha->fromXml(kind, degreeList);
      if (d == 0) {
            QString degrees;
            foreach(const HDegree& d, degreeList) {
                  if (!degrees.isEmpty())
                        degrees += " ";
                  degrees += d.text();
                  }
            printf("unknown chord txt: <%s> kind: <%s> degrees: %s\n",
               qPrintable(kindText), qPrintable(kind), qPrintable(degrees));

            // Strategy II: lookup "kind", merge in degree list and try to find
            //    harmony in list

            d = ha->fromXml(kind);
            if (d) {
                  ha->setId(d->id);
                  foreach(const HDegree& d, degreeList)
                        ha->addDegree(d);
                  ha->resolveDegreeList();
                  ha->render();
                  }
            else {
                  printf("'kind: <%s> not found in harmony data base\n", qPrintable(kind));
                  QString s = tpc2name(ha->rootTpc(), score->style(ST_useGermanNoteNames).toBool()) + kindText;
                  ha->setText(s);
                  }
            }
      else {
            ha->setId(d->id);
//            ha->resolveDegreeList();
            ha->render();
            }
      ha->setVisible(printObject == "yes");
      measure->add(ha);
      }

//---------------------------------------------------------
//   xmlClef
//---------------------------------------------------------

void MusicXml::xmlClef(QDomElement e, int staffIdx, Measure* measure)
      {
      int clef   = 0;
      int clefno = e.attribute(QString("number"), "1").toInt() - 1;
      QString c;
      int i = 0;
      int line = -1;
      for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
            if (ee.tagName() == "sign")
                  c = ee.text();
            else if (ee.tagName() == "line")
                  line = ee.text().toInt();
            else if (ee.tagName() == "clef-octave-change") {
                  i = ee.text().toInt();
                  if (i && !(c == "F" || c == "G"))
                        printf("clef-octave-change only implemented for F and G key\n");
                  }
            else
                  domError(ee);
            }
      //some software (Primus) don't include line and assume some default
      // it's permitted by MusicXML 2.0 XSD
      if(line == -1) {
          if(c == "G")
                line = 2;
          else if (c == "F")
                line = 4;
          else if (c == "C")
                line = 3;
          }
      
      if (c == "G" && i == 0 && line == 2)
            clef = CLEF_G;
      else if (c == "G" && i == 1 && line == 2)
            clef = CLEF_G1;
      else if (c == "G" && i == 2 && line == 2)
            clef = CLEF_G2;
      else if (c == "G" && i == -1 && line == 2)
            clef = CLEF_G3;
      else if (c == "G" && i == 0 && line == 1)
            clef = CLEF_G4;
      else if (c == "F" && i == 0 && line == 3)
            clef = CLEF_F_B;
      else if (c == "F" && i == 0 && line == 4)
            clef = CLEF_F;
      else if (c == "F" && i == 1 && line == 4)
            clef = CLEF_F_8VA;
      else if (c == "F" && i == 2 && line == 4)
            clef = CLEF_F_15MA;
      else if (c == "F" && i == -1 && line == 4)
            clef = CLEF_F8;
      else if (c == "F" && i == -2 && line == 4)
            clef = CLEF_F15;
      else if (c == "F" && i == 0 && line == 5)
            clef = CLEF_F_C;
      else if (c == "C") {
            if (line == 5)
                  clef = CLEF_C5;
            else if (line == 4)
                  clef = CLEF_C4;
            else if (line == 3)
                  clef = CLEF_C3;
            else if (line == 2)
                  clef = CLEF_C2;
            else if (line == 1)
                  clef = CLEF_C1;
            }
      else if (c == "percussion")
            clef = CLEF_PERC;
      else
            printf("ImportMusicXML: unknown clef <sign=%s line=%d oct ch=%d>\n", qPrintable(c), line, i);
      Staff* part = score->staff(staffIdx + clefno);
      ClefList* ct = part->clefList();
      (*ct)[tick] = clef;
      if (tick) {
            // dont generate symbol for tick 0
            Clef* clefs = new Clef(score, clef);
            clefs->setTick(tick);
            clefs->setTrack((staffIdx + clefno) * VOICES);
            Segment* s = measure->getSegment(clefs);
            s->add(clefs);
            ++clefno;   // ??
            }
      }

