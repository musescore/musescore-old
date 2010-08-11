//=============================================================================
//  BWW to MusicXML converter
//  Part of MusE Score
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

/**
 \file
 A simple parser for bww files. The file header is handled one line at a time,
 until a line starting with "&" is found. The parser then builds measures from
 uinterrupted sequences of notes.
 */

#include <iostream>

#include <QtCore/QStringList>
#include <QtCore/QtDebug>

#include "lexer.h"
#include "parser.h"
#include "writer.h"

/**
 Determine if symbol is a grace sequence
 */

static bool isGrace(Bww::Symbol sym)
{
  return (sym == Bww::SINGLEGRACE
          || sym == Bww::STRIKE
          || sym == Bww::DOUBLING
          || sym == Bww::HALFDOUBLING
          || sym == Bww::THUMBDOUBLING
          || sym == Bww::SLUR
          || sym == Bww::THROW
          || sym == Bww::BIRL
          || sym == Bww::GRIP
          || sym == Bww::TAORLUATH);
}

/**
 Determine if symbol is part of a note sequence
 */

static bool isNote(Bww::Symbol sym)
{
  return (sym == Bww::NOTE
          || sym == Bww::TIE
          || sym == Bww::TRIPLET
          || isGrace(sym));
}

/**
 Determine if symbol is part of a non-note sequence
 */

static bool isNonNote(Bww::Symbol sym)
{
  return (sym == Bww::CLEF
          || sym == Bww::KEY
          || sym == Bww::TSIG
          || sym == Bww::PART
          || sym == Bww::BAR);
}

namespace Bww {

  /**
   Parser constructor, using Lexer \a l and Writer \a w.
   */

  Parser::Parser(Lexer& l, Writer& w)
    : lex(l),
    wrt(w),
    tempo(0),
    inMeasure(false),
    measureNr(0),
    tieStart(false),
    inTie(false),
    tripletStart(false),
    inTriplet(false)
  {
    qDebug() << "Parser::Parser()";

    // Initialize the grace note translation table
    // Grace sequence definitions were taken from Lilyponds bagpipe.ly
    // For some Lilypond sequences the bww name is unknown

    // Single grace notes
    graceMap["ag"] = "LA";
    graceMap["bg"] = "B";
    graceMap["cg"] = "C";
    graceMap["dg"] = "D";
    graceMap["eg"] = "E";
    graceMap["fg"] = "F";
    graceMap["gg"] = "HG";
    graceMap["tg"] = "HA";

    // Strikes (same as single grace notes)
    graceMap["strlg"] = "LG";
    graceMap["strla"] = "LA";
    graceMap["strb"]  = "B";
    graceMap["strc"]  = "C";
    graceMap["strd"]  = "D";
    graceMap["stre"]  = "E";
    graceMap["strf"]  = "F";
    graceMap["strhg"] = "HG";
    graceMap["strha"] = "HA";

    // Doublings
    graceMap["dblg"] = "HG LG D";
    graceMap["dbla"] = "HG LA D";
    graceMap["dbb"]  = "HG B D";
    graceMap["dbc"]  = "HG C D";
    graceMap["dbd"]  = "HG D E";
    graceMap["dbe"]  = "HG E F";
    graceMap["dbf"]  = "HG F HG";
    graceMap["dbhg"] = "HG F";
    graceMap["dbha"] = "HA HG";

    // Half doublings
    graceMap["hdblg"] = "LG D";
    graceMap["hdbla"] = "LA D";
    graceMap["hdbb"]  = "B D";
    graceMap["hdbc"]  = "C D";
    graceMap["hdbd"]  = "D E";
    graceMap["hdbe"]  = "E F";
    graceMap["hdbf"]  = "F HG";
    graceMap["hdbhg"] = "HG F";
    graceMap["hdbha"] = "HA HG";

    // Thumb doublings
    graceMap["thdblg"] = "HA LG D";
    graceMap["thdbla"] = "HA LA D";
    graceMap["thdbb"]  = "HA B D";
    graceMap["thdbc"]  = "HA C D";
    graceMap["thdbd"]  = "HA D E";
    graceMap["thdbe"]  = "HA E F";
    graceMap["thdbf"]  = "HA F HG";
    graceMap["thdbhg"] = "HA HG F";

    // Shakes
    // Half shakes
    // Thumb shakes
    // ???

    // Slurs
    graceMap["gstd"] = "HG D LG";
    graceMap["lgstd"] = "HG D C";

    // Half slurs
    // Thumb slurs
    // ???

    // Catches
    // ???

    // Throws
    graceMap["thrd"] = "LG D C";
    //    graceMap["???"] = "D C";
    //    graceMap["???"] = "LG D LG C";
    //    graceMap["???"] = "F E HG E";

    //  Birls
    graceMap["abr"] = "LA LG LA LG";
    graceMap["brl"] = "LG LA LG";
    graceMap["gbrl"] = "HG LA LG LA LG";
    graceMap["tbrl"] = "D LA LG LA LG";

    // Grips
    graceMap["grp"] = "LG D LG";

    // Taorluaths
    graceMap["tar"] = "LG D LG E";

    // Crunluaths
    // ???
  }

  /**
   Transition to the "in measure" state.
   */

  void Parser::beginMeasure(const Bww::MeasureBeginFlags mbf)
  {
    qDebug() << "Parser::beginMeasure("
        << "repeatBegin:" << mbf.repeatBegin
        << "endingFirst:" << mbf.endingFirst
        << "endingSecond:" << mbf.endingSecond
        << ")";

    if (!inMeasure)
    {
      inMeasure = true;
      ++measureNr;
      wrt.beginMeasure(mbf);
    }
  }

  /**
   Transition out of the "in measure" state.
   */

  void Parser::endMeasure(const Bww::MeasureEndFlags mef)
  {
    qDebug() << "Parser::endMeasure("
        << "repeatEnd:" << mef.repeatEnd
        << "endingEnd:" << mef.endingEnd
        << ")";

    if (inMeasure)
    {
      inMeasure = false;
      wrt.endMeasure(mef);
    }
  }

  /**
   Parse the input stream and write result.
   */

  void Parser::parse()
  {
    // read the header, handling only strings and tune tempo
    while (lex.symType() == COMMENT
           || lex.symType() == STRING
           || lex.symType() == TEMPO)
    {
      if (lex.symType() == STRING)
        parseString();
      else if (lex.symType() == TEMPO)
        parseTempo();
      else if (lex.symType() == COMMENT)
        lex.getSym();
    }
    qDebug() << "Parser::parse()"
        << "title:" << title
        << "type:" << type
        << "composer:" << composer
        << "footer:" << footer
        ;
    wrt.header(title, type, composer, footer, tempo);

    // read the actual music
    if (lex.symType() != CLEF)
      errorHandler("clef ('&') expected");
    while (lex.symType() != NONE)
    {
      if (isNonNote(lex.symType()))
        parseSeqNonNotes();
      else if (isNote(lex.symType()))
        parseSeqNotes();
      else if (lex.symType() == UNKNOWN)
      {
        errorHandler("unknown symbol '" + lex.symValue() + "'");
        lex.getSym();
      }
      else
      {
        ; // others not implemented yet: silently ignored
        lex.getSym();
      }
    }

    // trailer
    wrt.trailer();
  }

  /**
   Display error \a s.
   */

  void Parser::errorHandler(QString s)
  {
    std::cerr << "Parse error line "
        << lex.symLineNumber() + 1
        << ": "
        << qPrintable(s)
        << std::endl;
  }

  /**
   Parse a bww bar symbol.
   */

  void Parser::parseBar()
  {
    qDebug() << "Parser::parseBar() value:" << qPrintable(lex.symValue());
    lex.getSym();
  }

  /**
   Parse a bww note.
   */

  void Parser::parseNote()
  {
    qDebug() << "Parser::parseNote() value:" << qPrintable(lex.symValue());

    QRegExp rNotes("(LG|LA|[B-F]|HG|HA)([lr]?)_(1|2|4|8|16|32)");

    QStringList caps;
    if (rNotes.exactMatch(lex.symValue()))
    {
      caps = rNotes.capturedTexts();
      qDebug() << " match" << caps.size();
      if (caps.size() == 4)
      {
        qDebug()
            << "caps[1]" << caps.at(1)
            << "caps[2]" << caps.at(2)
            << "caps[3]" << caps.at(3)
            ;
      }
    }
    lex.getSym();

    int dots = 0;
    bool tieStop = false;
    if (tieStart) inTie = true;
    bool tripletStop = false;
    if (tripletStart) inTriplet = true;
    if (lex.symType() == DOT)
    {
      qDebug() << " dot" << qPrintable(lex.symValue());
      ++dots;
      lex.getSym();
    }
    else if (lex.symType() == TIE)
    {
      qDebug() << " tie" << qPrintable(lex.symValue());
      if (lex.symValue() == "^ts")
      {
        if (inTie) errorHandler("tie start ('^ts') unexpected");
      }
      else
      {
        if (!inTie) errorHandler("tie end ('^te') unexpected");
        else
        {
          tieStop = true;
          inTie = false;
        }
      }
      lex.getSym();
    }
    else if (lex.symType() == TRIPLET)
    {
      qDebug() << " triplet" << qPrintable(lex.symValue());
      if (lex.symValue() == "^3s")
      {
        if (inTriplet) errorHandler("triplet start ('^3s') unexpected");
      }
      else
      {
        if (!inTriplet) errorHandler("triplet end ('^3e') unexpected");
        else
        {
          tripletStop = true;
        }
      }
      lex.getSym();
    }
    StartStop triplet = ST_NONE;
    if (inTriplet)
    {
      if (tripletStart) triplet = ST_START;
      else if (tripletStop) triplet = ST_STOP;
      else triplet = ST_CONTINUE;
    }
    qDebug() << " tie start" << tieStart << " tie stop" << tieStop;
    qDebug() << " triplet start" << tripletStart << " triplet stop" << tripletStop;
    wrt.note(caps[1], caps[2], caps[3], dots, tieStart, tieStop, triplet);
    tieStart = false;
    tripletStart = false;
    if (tripletStop)
    {
      inTriplet = false;
      tripletStop = false;
    }
  }

  /**
   Parse a bww embellishment.
   */

  void Parser::parseGraces()
  {
    qDebug() << "Parser::parseGraces() value:" << qPrintable(lex.symValue());

    const QString beam = " "; // TODO
    const QString type = "32";
    const int dots = 0;
    if (graceMap.contains(lex.symValue()))
    {
      QStringList graces = graceMap.value(lex.symValue()).split(" ");
      for (int i = 0; i < graces.size(); ++i)
        wrt.note(graces.at(i), beam, type, dots, false, false, ST_NONE, true);
    }
    lex.getSym();
  }

  /**
   Parse a bww part symbol.
   */

  void Parser::parsePart(Bww::MeasureBeginFlags& mbf, Bww::MeasureEndFlags& mef)
  {
    qDebug() << "Parser::parsePart() value:" << qPrintable(lex.symValue());
    if (lex.symValue() == "I!''")
    {
      mbf.repeatBegin = true;
    }
    else if (lex.symValue() == "'1")
    {
      mbf.endingFirst = true;
    }
    else if (lex.symValue() == "'2")
    {
      mbf.endingSecond = true;
    }
    else if (lex.symValue() == "''!I")
    {
      mef.repeatEnd = true;
    }
    else if (lex.symValue() == "_'")
    {
      mef.endingEnd = true;
    }
    else
    {
      ; // other silently ignored
    }
    lex.getSym();
  }

  /**
   Parse a sequence of non-notes.
   */

  void Parser::parseSeqNonNotes()
  {
    qDebug() << "Parser::parseSeqNonNotes() value:" << qPrintable(lex.symValue());
    MeasureBeginFlags mbf;
    MeasureEndFlags mef;
    while (isNonNote(lex.symType()))
    {
      if (lex.symType() == CLEF)
        lex.getSym(); // ignore
      else if (lex.symType() == KEY)
        lex.getSym(); // ignore
      else if (lex.symType() == TSIG)
        parseTSig();
      else if (lex.symType() == PART)
        parsePart(mbf, mef);
      else if (lex.symType() == BAR)
        parseBar();
    }
    // First end the previous measure
    // Note: endMeasure does not do anything for the first measure
    endMeasure(mef);
    // Then start a new measure, if necessary
    if (isNote(lex.symType())) beginMeasure(mbf);
  }

  /**
   Parse a sequence of notes.
   Includes handling ties and triplets, but without extensive error checking.
   May break on invalid input.
   */

  void Parser::parseSeqNotes()
  {
    qDebug() << "Parser::parseSeqNotes() value:" << qPrintable(lex.symValue());
    while (isGrace(lex.symType()) || lex.symType() == NOTE || lex.symType() == TIE || lex.symType() == TRIPLET)
    {
      if (isGrace(lex.symType())) parseGraces();
      else if (lex.symType() == NOTE) parseNote();
      else if (lex.symType() == TIE)
      {
        if (lex.symValue() == "^ts")
        {
          if (inTie) errorHandler("tie start ('^ts') unexpected");
          else tieStart = true;
        }
        else
        {
          errorHandler("tie end ('^te') unexpected");
        }
        lex.getSym();
      }
      else if (lex.symType() == TRIPLET)
      {
        if (lex.symValue() == "^3s")
        {
          if (inTriplet) errorHandler("triplet start ('^3s') unexpected");
          else tripletStart = true;
        }
        else
        {
          errorHandler("triplet end ('^3e') unexpected");
        }
        lex.getSym();
      }
    }
  }

  /**
   Parse a bww string. Extract text and type.
   Example: "Air",(Y,C,0,0,Times New Roman,14,400,0,0,18,0,0,0)
   */

  void Parser::parseString()
  {
    qDebug() << "Parser::parseString() value:" << qPrintable(lex.symValue());

    QRegExp rString("\\\"(.*)\\\",\\(([A-Z]),.*\\)");

    if (rString.exactMatch(lex.symValue()))
    {
      QStringList caps = rString.capturedTexts();
      if (caps.size() == 3)
      {
        if (caps.at(2) == "T") title = caps.at(1);
        if (caps.at(2) == "Y") type = caps.at(1);
        if (caps.at(2) == "M") composer = caps.at(1);
        if (caps.at(2) == "F") footer = caps.at(1);
      }
    }
    lex.getSym();
  }

  /**
   Parse a bww tempo symbol.
   */

  void Parser::parseTempo()
  {
    qDebug() << "Parser::parseTempo() value:" << qPrintable(lex.symValue());

    QRegExp rTempo("^TuneTempo,(\\d+)");

    if (rTempo.exactMatch(lex.symValue()))
    {
      QStringList caps = rTempo.capturedTexts();
      if (caps.size() == 2)
      {
        tempo = caps.at(1).toInt();
      }
    }
    lex.getSym();
  }
  /**
   Parse a bww tsig symbol.
   */

  void Parser::parseTSig()
  {
    qDebug() << "Parser::parseTSig() value:" << qPrintable(lex.symValue());

    QRegExp rTSig("(\\d+)_(1|2|4|8|16|32)");

    if (rTSig.exactMatch(lex.symValue()))
    {
      QStringList caps = rTSig.capturedTexts();
      if (caps.size() == 3)
      {
        beats = caps.at(1).toInt();
        beat  = caps.at(2).toInt();
        wrt.tsig(beats, beat);
      }
    }
    lex.getSym();
  }

} // namespace Bww
