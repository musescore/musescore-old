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

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>

#include "writer.h"

namespace Bww {

  class Lexer;

  struct NoteDescription
  {
    QString pitch;
    QString beam;
    QString type;
    int dots;
    bool tieStart;
    bool tieStop;
    StartStop triplet;
    bool grace;
    NoteDescription(const QString _pitch, const QString _beam,
                    const QString _type, const int _dots,
                    bool _tieStart = false, bool _tieStop = false,
                    StartStop _triplet = ST_NONE,
                    bool _grace = false)
                      : pitch(_pitch), beam(_beam),
                      type(_type), dots(_dots),
                      tieStart(_tieStart), tieStop(_tieStop),
                      triplet(_triplet),
                      grace(_grace)
    {}
  };

  struct MeasureDescription
  {
    MeasureBeginFlags mbf;
    QList<NoteDescription> notes;
    MeasureEndFlags mef;
    int duration;
    MeasureDescription()
      : duration(0)
    {}
  };

  /**
   The bww parser.
   */

  class Parser
  {
  public:
    Parser(Lexer& l, Writer& w);
    void parse();
  private:
    void beginMeasure(const Bww::MeasureBeginFlags mbf);
    void endMeasure(const Bww::MeasureEndFlags mef);
    void errorHandler(QString s);
    void parseBar();
    void parseNote();
    void parseGraces();
    void parsePart(Bww::MeasureBeginFlags& mbf, Bww::MeasureEndFlags& mef);
    void parseSeqNonNotes();
    void parseSeqNotes();
    void parseString();
    void parseTempo();
    void parseTSig();
    Lexer& lex;                         ///< The lexer
    Writer& wrt;                        ///< The writer
    QString title;                      ///< Title read from the header
    QString type;                       ///< Type read from the header
    QString composer;                   ///< Composer read from the header
    QString footer;                     ///< Footer read from the header
    int tempo;                          ///< Tune tempo read from the header
    int beat;                           ///< Beat type, read from the clef line
    int beats;                          ///< Number of beats, read from the clef line
    QMap<QString, QString> graceMap;    ///< Map bww embellishments to separate grace notes
    bool inMeasure;                     ///< State: writing the notes in a measure
    int measureNr;                      ///< Current measure number
    bool tieStart;                      ///< Tie start pending
    bool inTie;                         ///< In a tie
    bool tripletStart;                  ///< Triplet start pending
    bool inTriplet;                     ///< In a triplet
    QList<MeasureDescription> measures; ///< Parsed measures
  };

} // namespace Bww

#endif // PARSER_H
