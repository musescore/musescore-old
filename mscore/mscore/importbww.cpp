//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include <stdio.h>

#include "lexer.h"
#include "writer.h"
#include "parser.h"

#include "barline.h"
#include "box.h"
#include "chord.h"
//#include "key.h"
#include "keysig.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "pitchspelling.h"
#include "score.h"
#include "staff.h"
#include "timesig.h"

//---------------------------------------------------------
//   addText
//   copied from importxml.cpp
//   TODO: remove duplicate code
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
//   xmlSetPitch
//   copied and adapted from importxml.cpp
//   TODO: remove duplicate code
//---------------------------------------------------------

/**
 Convert MusicXML \a step / \a alter / \a octave to midi pitch,
 set pitch and tpc.
 */

static void xmlSetPitch(Note* n, char step, int alter, int octave)
      {
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

      n->setPitch(pitch);

      //                        a  b  c  d  e  f  g
      static int table1[7]  = { 5, 6, 0, 1, 2, 3, 4 };
      int tpc  = step2tpc(table1[istep], alter);
      n->setTpc(tpc);
      }

namespace Bww {

  /**
   The writer that generates MusicXML output.
   */

  class MsScWriter : public Writer
  {
  public:
    MsScWriter();
    void beginMeasure();
    void endMeasure();
    void header(const QString title, const QString type,
                const QString composer, const QString footer);
    void note(const QString pitch, const QString beam,
              const QString type, const int dots,
              bool tieStart = false, bool tieStop = false,
              StartStop triplet = ST_NONE,
              bool grace = false);
    void setScore(Score* s) { score = s; }
    void tempo(const int beats, const int beat);
    void trailer();
  private:
    static const int WHOLE_DUR = 64;                    ///< Whole note duration
    struct StepAlterOct {                               ///< MusicXML step/alter/oct values
      QChar s;
      int a;
      int o;
      StepAlterOct(QChar step = 'C', int alter = 0, int oct = 1)
        : s(step), a(alter), o(oct) {};
    };
    Score* score;                                       ///< The score
    int beats;                                          ///< Number of beats
    int beat;                                           ///< Beat type
    QMap<QString, StepAlterOct> stepAlterOctMap;        ///< Map bww pitch to step/alter/oct
    QMap<QString, QString> typeMap;                     ///< Map bww note types to MusicXML
    unsigned int measureNumber;                         ///< Current measure number
    unsigned int tick;                                  ///< Current tick
    Measure* currentMeasure;                            ///< Current measure
  };

  /**
   MsScWriter constructor.
   */

  MsScWriter::MsScWriter()
    : score(0),
    beats(4),
    beat(4),
    measureNumber(0),
    tick(0),
    currentMeasure(0)
  {
    qDebug() << "MsScWriter::MsScWriter()";

    stepAlterOctMap["LG"] = StepAlterOct('G', 0, 4);
    stepAlterOctMap["LA"] = StepAlterOct('A', 0, 4);
    stepAlterOctMap["B"] = StepAlterOct('B', 0, 4);
    stepAlterOctMap["C"] = StepAlterOct('C', 1, 5);
    stepAlterOctMap["D"] = StepAlterOct('D', 0, 5);
    stepAlterOctMap["E"] = StepAlterOct('E', 0, 5);
    stepAlterOctMap["F"] = StepAlterOct('F', 1, 5);
    stepAlterOctMap["HG"] = StepAlterOct('G', 0, 5);
    stepAlterOctMap["HA"] = StepAlterOct('A', 0, 5);

    typeMap["1"] = "whole";
    typeMap["2"] = "half";
    typeMap["4"] = "quarter";
    typeMap["8"] = "eighth";
    typeMap["16"] = "16th";
    typeMap["32"] = "32nd";
  }

  /**
   Begin a new measure.
   */

  void MsScWriter::beginMeasure()
  {
      qDebug() << "MsScWriter::beginMeasure()";
      ++measureNumber;

      // create a new measure
      currentMeasure  = new Measure(score);
      currentMeasure->setTick(tick);
      currentMeasure->setNo(measureNumber);
      score->measures()->add(currentMeasure);

      if (measureNumber == 1) {
            // keysig
            KeySigEvent key;
            key.setAccidentalType(2);
            (*score->staff(0)->keymap())[tick] = key;
            // timesig
            TimeSig ts = TimeSig(score, beat, beats);
            int st = ts.subtype();
            if (st) {
                  //ws score->sigmap()->add(tick, TimeSig::getSig(st));
                  TimeSig* timesig = new TimeSig(score);
                  //ws timesig->setTick(tick);
                  timesig->setSubtype(st);
                  timesig->setTrack(0);
                  Segment* s = currentMeasure->getSegment(timesig, tick);
                  s->add(timesig);
                  }
            }
  }

/**
 End the current measure.
 */

void MsScWriter::endMeasure()
{
      qDebug() << "MsScWriter::endMeasure()";
      BarLine* barLine = new BarLine(score);
      bool visible = true;
      barLine->setSubtype(NORMAL_BAR);
      barLine->setTrack(0);
      currentMeasure->setEndBarLineType(barLine->subtype(), false, visible);
}

  /**
   Write a single note.
   */

void MsScWriter::note(const QString pitch, const QString /*TODO beam */,
                      const QString type, const int dots,
                      bool /*TODO tieStart */, bool /*TODO tieStop */,
                      StartStop /*TODO triplet */,
                      bool grace)
{
      qDebug() << "MsScWriter::note()"
            << "type:" << type
            << "dots:" << dots
            << "grace" << grace
            ;

      if (!stepAlterOctMap.contains(pitch)
          || !typeMap.contains(type)) {
            // TODO: error message
            return;
            }
      StepAlterOct sao = stepAlterOctMap.value(pitch);

      int ticks = 4 * AL::division / type.toInt();
      if (dots) ticks = 3 * ticks / 2;
      qDebug() << "ticks:" << ticks;
      Duration durationType(Duration::V_INVALID);
      durationType.setVal(ticks);
      qDebug() << "duration:" << durationType.name();

      BeamMode bm  = BEAM_AUTO;
      Direction sd = AUTO;

      // create chord
      Chord* cr = new Chord(score);
      //ws cr->setTick(tick);
      cr->setBeamMode(bm);
      cr->setTrack(0);
      if (grace) {
            cr->setNoteType(NOTE_GRACE32);
            cr->setDurationType(Duration::V_32ND);
            sd = UP;
            }
      else {
            if (durationType.type() == Duration::V_INVALID)
                  durationType.setType(Duration::V_QUARTER);
            cr->setDurationType(durationType);
            tick += ticks;
            sd = DOWN;
            }
      cr->setDots(dots);
      cr->setStemDirection(sd);
      // add note to chord
      Note* note = new Note(score);
      note->setTrack(0);
      xmlSetPitch(note, sao.s.toAscii(), sao.a, sao.o);
      cr->add(note);
      // add chord to measure
      Segment* s = currentMeasure->getSegment(SegChordRest, tick);
      s->add(cr);
}

  /**
   Write the header.
   */

  void MsScWriter::header(const QString title, const QString type,
                          const QString composer, const QString footer)
  {
    qDebug() << "MsScWriter::header()"
        << "title:" << title
        << "type:" << type
        << "composer:" << composer
        << "footer:" << footer
        ;

//  score->setWorkTitle(title);
      VBox* vbox  = 0;
      addText(vbox, score, title, TEXT_TITLE, TEXT_STYLE_TITLE);
      addText(vbox, score, type, TEXT_SUBTITLE, TEXT_STYLE_SUBTITLE);
      addText(vbox, score, composer, TEXT_COMPOSER, TEXT_STYLE_COMPOSER);
//      addText(vbox, score, strPoet, TEXT_POET, TEXT_STYLE_POET);
//      addText(vbox, score, strTranslator, TEXT_TRANSLATOR, TEXT_STYLE_TRANSLATOR);
      if (vbox) {
            vbox->setTick(0);
            score->measures()->add(vbox);
            }
      if (footer != "") score->setCopyright(footer);
  }

  /**
   Store beats and beat type for later use.
   */

  void MsScWriter::tempo(const int bts, const int bt)
  {
    qDebug() << "MsScWriter::tempo()"
        << "beats:" << bts
        << "beat:" << bt
        ;

    beats = bts;
    beat  = bt;
  }

  /**
   Write the trailer.
   */

  void MsScWriter::trailer()
  {
    qDebug() << "MsScWriter::trailer()"
        ;

  }

} // namespace Bww

//---------------------------------------------------------
//   importBww
//---------------------------------------------------------

bool Score::importBww(const QString& path)
      {
      printf("Score::importBww(%s)\n", qPrintable(path));

      if (path.isEmpty())
            return false;
      QFile fp(path);
      if (!fp.open(QIODevice::ReadOnly))
            return false;

      QString id("importBww");
      Part* part = new Part(this);
      part->setId(id);
      appendPart(part);
      Staff* staff = new Staff(this, part, 0);
      part->staves()->push_back(staff);
      staves().push_back(staff);

      Bww::Lexer lex(&fp);
      Bww::MsScWriter wrt;
      wrt.setScore(this);
      Bww::Parser p(lex, wrt);
      p.parse();

      _saved = false;
      _created = true;
      printf("Score::importBww() done\n");
//      return false;	// error
      return true;	// OK
      }

