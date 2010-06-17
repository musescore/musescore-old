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

#ifndef PARSER_H
#define PARSER_H

/**
 \file
 Definition of class Parser
 */

#include <QtCore/QMap>
#include <QtCore/QString>

namespace Bww {

  class Lexer;
  class Writer;

  /**
   The bww parser.
   */

  class Parser
  {
  public:
    Parser(Lexer& l, Writer& w);
    void parse();
  private:
    void beginMeasure();
    void endMeasure();
    void errorHandler(QString s);
    void parseBar();
    void parseNote();
    void parseGraces();
    void parsePart();
    void parseSeqNotes();
    void parseString();
    void parseTempo();
    Lexer& lex;                         ///< The lexer
    Writer& wrt;                        ///< The writer
    QString title;                      ///< Title read from the header
    QString type;                       ///< Type read from the header
    QString composer;                   ///< Composer read from the header
    QString footer;                     ///< Footer read from the header
    int beat;                           ///< Beat type, read from the clef line
    int beats;                          ///< Number of beats, read from the clef line
    QMap<QString, QString> graceMap;    ///< Map bww embellishments to separate grace notes
    bool inMeasure;                     ///< State: writing the notes in a measure
    int measureNr;                      ///< Current measure number
    bool tieStart;                      ///< Tie start pending
    bool inTie;                         ///< In a tie
    bool tripletStart;                  ///< Triplet start pending
    bool inTriplet;                     ///< In a triplet
  };

} // namespace Bww

#endif // PARSER_H
