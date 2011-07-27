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
 BWW file lexical analysis.

 Loosely based on BWW file format - BNF specification by tjm found in
 http://forums.bobdunsire.com/forums/showthread.php?t=123219
 */

#include <QtCore/QRegExp>
#include <QtCore/QtDebug>

#include "lexer.h"

namespace Bww {

  /**
   Lexer constructor, \a inDevice is the input.
   */

  Lexer::Lexer(QIODevice *inDevice)
    : in(inDevice),
    lineNumber(-1),
    value(NONE)
  {
    qDebug() << "Lexer::Lexer() begin";

    getSym();

    qDebug() << "Lexer::Lexer() end";
  }

  /**
   Get the next symbol, update type and value.
   */

  void Lexer::getSym()
  {
    qDebug() << "Lexer::getSym()";

    // if unparsed words remaining, use these
    if (list.size() > 0)
    {
      categorizeWord(list.at(0));
      list.removeFirst();
      return;
    }

    // read the next non-empty line
    do
    {
      line = in.readLine();
      ++lineNumber;
      if (line.isNull())
      {
        // end of file
        qDebug() << "-> end of file";
        type = NONE;
        value = "";
        return;
      }
    }
    while (line == "");

    qDebug() << "getSym: read line" << line;
    QRegExp rHeaderIgnore("^Bagpipe Reader|^MIDINoteMappings|^FrequencyMappings"
                          "|^InstrumentMappings|^GracenoteDurations|^FontSizes"
                          "|^TuneFormat");
    QRegExp rTuneTempo("^TuneTempo");
    if (rHeaderIgnore.indexIn(line) == 0)
    {
      type = COMMENT;
      value = "";
      qDebug()
          << "-> header ignore,"
          << "type:" << symbolToString(type)
          << "value:" << value
          ;
      line = "";
      return;
    }
    else if (rTuneTempo.indexIn(line) == 0)
    {
      type = TEMPO;
      value = line;
      qDebug()
          << "-> tempo,"
          << "type:" << symbolToString(type)
          << "value:" << value
          ;
      line = "";
      return;
    }
    else if (line.at(0) == '"')
    {
      type = STRING;
      value = line;
      qDebug()
          << "-> quoted string,"
          << "type:" << symbolToString(type)
          << "value:" << value
          ;
      line = "";
    }
    else
    {
      // split line into space-separated words
      list = line.trimmed().split(QRegExp("\\s+"));
      qDebug()
          << "-> words"
          << list
          ;
      categorizeWord(list.at(0));
      list.removeFirst();
    }
    line = "";
  }

  /**
   Return the current symbols type.
   */

  Symbol Lexer::symType() const
  {
    return type;
  }

  /**
   Return the current symbols value.
   */

  QString Lexer::symValue() const
  {
    return value;
  }

  /**
   Determine the symbol type for \a word.
   */

  void Lexer::categorizeWord(QString word)
  {
    qDebug() << "Lexer::categorizeWord(" << word << ")";

    // default values
    type = NONE;
    value = word;

    QRegExp rClef("&");
    QRegExp rKey("sharp[cf]");
    QRegExp rTSig("\\d+_(1|2|4|8|16|32)");
    QRegExp rPart("I!''|I!|''!I|!I|'intro|[2-9]|'[12]|_'");
    QRegExp rBar("!|!t|!!t");
    QRegExp rNote("(LG|LA|[B-F]|HG|HA)[lr]?_(1|2|4|8|16|32)");
    QRegExp rTie("\\^t[es]");
    QRegExp rTriplet("\\^3[es]");
    QRegExp rDot("'([hl][ag]|[b-f])");
    QRegExp rSingleGrace("[a-gt]g");
    QRegExp rStrike("str([hl][ag]|[b-f])");
    QRegExp rDoubling("db([hl][ag]|[b-f])");
    QRegExp rHalfDoubling("hdb([hl][ag]|[b-f])");
    QRegExp rThumbDoubling("thdb(l[ag]|[b-f]|hg)");
    QRegExp rSlur("l?gstd");
    QRegExp rThrow("thrd");
    QRegExp rBirl("abr|brl|gbrl|tbrl");
    QRegExp rGrip("grp");
    QRegExp rTaorluath("tar");

    if (rClef.exactMatch(word))
      type = CLEF;
    else if (rKey.exactMatch(word))
      type = KEY;
    else if (rTSig.exactMatch(word))
      type = TSIG;
    else if (rPart.exactMatch(word))
      type = PART;
    else if (rBar.exactMatch(word))
      type = BAR;
    else if (rNote.exactMatch(word))
      type = NOTE;
    else if (rTie.exactMatch(word))
      type = TIE;
    else if (rTriplet.exactMatch(word))
      type = TRIPLET;
    else if (rDot.exactMatch(word))
      type = DOT;
    else if (rSingleGrace.exactMatch(word))
      type = SINGLEGRACE;
    else if (rStrike.exactMatch(word))
      type = STRIKE;
    else if (rDoubling.exactMatch(word))
      type = DOUBLING;
    else if (rHalfDoubling.exactMatch(word))
      type = HALFDOUBLING;
    else if (rThumbDoubling.exactMatch(word))
      type = THUMBDOUBLING;
    else if (rSlur.exactMatch(word))
      type = SLUR;
    else if (rThrow.exactMatch(word))
      type = THROW;
    else if (rBirl.exactMatch(word))
      type = BIRL;
    else if (rGrip.exactMatch(word))
      type = GRIP;
    else if (rTaorluath.exactMatch(word))
      type = TAORLUATH;
    else
      type = UNKNOWN;

    qDebug()
        << " type: " << qPrintable(symbolToString(type))
        << " value: '" << qPrintable(value) << "'"
        ;
  }

} // namespace Bww
