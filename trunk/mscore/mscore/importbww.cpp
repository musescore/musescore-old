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

#include <QMap>
#include <QString>
#include <QtDebug>

#include "lexer.h"
#include "writer.h"
#include "parser.h"

#include "box.h"
#include "measure.h"
#include "part.h"
#include "score.h"
#include "staff.h"

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
  };

  /**
   MsScWriter constructor.
   */

  MsScWriter::MsScWriter()
    : score(0),
    beats(4),
    beat(4),
    measureNumber(0)
  {
    qDebug() << "MsScWriter::MsScWriter()";

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
  }

  /**
   End the current measure.
   */

  void MsScWriter::endMeasure()
  {
    qDebug() << "MsScWriter::endMeasure()";
  }

  /**
   Write a single note.
   */

  void MsScWriter::note(const QString pitch, const QString /*TODO beam */,
                        const QString type, const int dots,
                        bool grace)
  {
    qDebug() << "MsScWriter::note()";
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

            // example of how to add a measure
            Measure* measure  = new Measure(this);
            measure->setTick(0);
            measure->setNo(1);
            measures()->add(measure);

      _saved = false;
      _created = true;
      printf("Score::importBww() done\n");
//      return false;	// error
      return true;	// OK
      }

