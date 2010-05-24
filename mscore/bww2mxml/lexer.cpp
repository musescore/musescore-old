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

#include <QRegExp>
#include <QtDebug>

#include "lexer.h"

namespace Bww {

  /**
   Lexer constructor, \a inDevice is the input.
   */

  Lexer::Lexer(QIODevice *inDevice)
    : in(inDevice), value(NONE)
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
                          "|^TuneFormat|^TuneTempo");
    if (rHeaderIgnore.indexIn(line) == 0)
    {
      type = COMMENT;
      value = "";
      qDebug()
          << "-> header ignore,"
          << " type: " << symbolToString(type)
          << " value: " << value
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
          << " type: " << symbolToString(type)
          << " value: " << value
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

  Symbol Lexer::symType()
  {
    return type;
  }

  /**
   Return the current symbols value.
   */

  QString Lexer::symValue()
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
    QRegExp rTempo("\\d+_(1|2|4|8|16|32)");
    QRegExp rParts("I!''|I!|''!I|!I|'intro|[2-9]|'[12]|_'");
    QRegExp rBars("!|!t|!!t");
    QRegExp rNotes("(LG|LA|[B-F]|HG|HA)[lr]?_(1|2|4|8|16|32)");
    QRegExp rTies("\\^t[es]");
    QRegExp rDots("'([hl][ag]|[b-f])");
    QRegExp rSingleGraces("[a-gt]g");

    if (rClef.exactMatch(word))
      type = CLEF;
    else if (rKey.exactMatch(word))
      type = KEY;
    else if (rTempo.exactMatch(word))
      type = TEMPO;
    else if (rParts.exactMatch(word))
      type = PART;
    else if (rBars.exactMatch(word))
      type = BAR;
    else if (rNotes.exactMatch(word))
      type = NOTE;
    else if (rTies.exactMatch(word))
      type = TIE;
    else if (rDots.exactMatch(word))
      type = DOT;
    else if (rSingleGraces.exactMatch(word))
      type = SINGLEGRACE;
    else
    {
      type = UNKNOWN;
      value = "";
    }

    qDebug()
        << " type: " << qPrintable(symbolToString(type))
        << " value: '" << qPrintable(value) << "'"
        ;
  }

} // namespace Bww
