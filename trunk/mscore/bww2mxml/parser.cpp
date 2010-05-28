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
 A very simple parser for bww files. The file header is handled one line at a time,
 until a line starting with "&" is found. The parser then builds measures from
 uinterrupted sequences of notes. Any non-note symbol ends the current measure.
 */

#include <iostream>

#include <QStringList>
#include <QtDebug>

#include "lexer.h"
#include "parser.h"
#include "writer.h"

namespace Bww {

  /**
   Parser constructor, using Lexer \a l and Writer \a w.
   */

  Parser::Parser(Lexer& l, Writer& w)
    : lex(l),
    wrt(w),
    inMeasure(false),
    measureNr(0)
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

  void Parser::beginMeasure()
  {
    qDebug() << "Parser::beginMeasure()";

    if (!inMeasure)
    {
      inMeasure = true;
      ++measureNr;
      wrt.beginMeasure();
    }
  }

  /**
   Transition out of the "in measure" state.
   */

  void Parser::endMeasure()
  {
    qDebug() << "Parser::endMeasure()";

    if (inMeasure)
    {
      inMeasure = false;
      wrt.endMeasure();
    }
  }

  /**
   Parse the input stream and write result.
   */

  void Parser::parse()
  {
    // read the header, handling only the strings
    while (lex.symType() == COMMENT || lex.symType() == STRING)
    {
      if (lex.symType() == STRING)
        parseString();
      else if (lex.symType() == COMMENT)
        lex.getSym();
    }
    qDebug() << "Parser::parse()"
        << "title:" << title
        << "type:" << type
        << "composer:" << composer
        << "footer:" << footer
        ;
    wrt.header(title, type, composer, footer);

    // read the actual music
    if (lex.symType() != CLEF)
      errorHandler("clef (&) expected");
    while (lex.symType() != NONE)
    {
      if (lex.symType() == CLEF)
        lex.getSym(); // ignore
      else if (lex.symType() == KEY)
        lex.getSym(); // ignore
      else if (lex.symType() == TEMPO)
        parseTempo();
      else if (lex.symType() == PART)
        parsePart();
      else if (lex.symType() == BAR)
        parseBar();
      else if (lex.symType() == NOTE)
        parseNote();
      else if (lex.symType() == SINGLEGRACE
               || lex.symType() == STRIKE
               || lex.symType() == DOUBLING
               || lex.symType() == HALFDOUBLING
               || lex.symType() == THUMBDOUBLING
               || lex.symType() == SLUR
               || lex.symType() == THROW
               || lex.symType() == BIRL
               || lex.symType() == GRIP
               || lex.symType() == TAORLUATH)
        parseGraces();
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
    std::cerr << "Parse error: " << qPrintable(s) << std::endl;
  }

  /**
   Parse a bww bar symbol.
   */

  void Parser::parseBar()
  {
    qDebug() << "Parser::parseBar() value:" << qPrintable(lex.symValue());
    endMeasure();
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
    if (lex.symType() == DOT)
    {
      qDebug() << " dot" << qPrintable(lex.symValue());
      ++dots;
      lex.getSym();
    }
    beginMeasure();
    wrt.note(caps[1], caps[2], caps[3], dots);
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
      beginMeasure();
      QStringList graces = graceMap.value(lex.symValue()).split(" ");
      for (int i = 0; i < graces.size(); ++i)
        wrt.note(graces.at(i), beam, type, dots, true);
    }
    lex.getSym();
  }

  /**
   Parse a bww part symbol.
   */

  void Parser::parsePart()
  {
    qDebug() << "Parser::parsePart() value:" << qPrintable(lex.symValue());
    endMeasure();
    lex.getSym();
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

    QRegExp rTempo("(\\d+)_(1|2|4|8|16|32)");

    if (rTempo.exactMatch(lex.symValue()))
    {
      QStringList caps = rTempo.capturedTexts();
      if (caps.size() == 3)
      {
        beats = caps.at(1).toInt();
        beat  = caps.at(2).toInt();
        wrt.tempo(beats, beat);
      }
    }
    lex.getSym();
  }

} // namespace Bww
