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
 Output writer for BWW to MusicXML converter.
 */

#include <QtCore/QtDebug>

#include "mxmlwriter.h"

namespace Bww {

  /**
   MxmlWriter constructor.
   */

  MxmlWriter::MxmlWriter()
    : beats(4),
    beat(4),
    measureNumber(0),
    tempo(0)
  {
    qDebug() << "MxmlWriter::MxmlWriter()";

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

  void MxmlWriter::beginMeasure(const Bww::MeasureBeginFlags mbf)
  {
    qDebug() << "MxmlWriter::beginMeasure()";
    ++measureNumber;
    out << "    <measure number=\"" << measureNumber << "\">" << endl;
    if (mbf.repeatBegin || mbf.endingFirst || mbf.endingSecond)
    {
      out << "      <barline location=\"left\">" << endl;
      if (mbf.repeatBegin)
      {
        out << "        <bar-style>heavy-light</bar-style>" << endl;
        out << "        <repeat direction=\"forward\"/>" << endl;
      }
      if (mbf.endingFirst)
      {
        out << "        <ending number=\"1\" type=\"start\"/>" << endl;
      }
      if (mbf.endingSecond)
      {
        out << "        <ending number=\"2\" type=\"start\"/>" << endl;
      }
      out << "      </barline>" << endl;
    }
    if (measureNumber == 1)
    {
      out << "      <attributes>" << endl;
      out << "        <divisions>" << WHOLE_DUR / 4 << "</divisions>" << endl;
      out << "        <key>" << endl;
      out << "          <fifths>2</fifths>" << endl;
      out << "          <mode>major</mode>" << endl;
      out << "        </key>" << endl;
      out << "        <time>" << endl;
      out << "          <beats>" << beats << "</beats>" << endl;
      out << "          <beat-type>" << beat << "</beat-type>" << endl;
      out << "        </time>" << endl;
      out << "        <clef>" << endl;
      out << "          <sign>G</sign>" << endl;
      out << "          <line>2</line>" << endl;
      out << "        </clef>" << endl;
      out << "      </attributes>" << endl;
      if (tempo)
      {
        out << "      <sound tempo=\"" << tempo << "\"/>" << endl;
      }
    }
  }

  /**
   End the current measure.
   */

  void MxmlWriter::endMeasure(const Bww::MeasureEndFlags mef)
  {
    qDebug() << "MxmlWriter::endMeasure()";
    if (mef.repeatEnd || mef.endingEnd)
    {
      out << "      <barline location=\"right\">" << endl;
      if (mef.repeatEnd)
      {
        out << "        <bar-style>light-heavy</bar-style>" << endl;
        out << "        <repeat direction=\"backward\"/>" << endl;
      }
      if (mef.endingEnd)
      {
        out << "        <ending type=\"stop\"/>" << endl;
      }
      out << "      </barline>" << endl;
    }
    out << "    </measure>" << endl;
  }

  /**
   Write a single note.
   */

  void MxmlWriter::note(const QString pitch, const QString /*TODO beam */,
                        const QString type, const int dots,
                        bool tieStart, bool tieStop,
                        StartStop triplet,
                        bool grace)
  {
    qDebug() << "MxmlWriter::note()";

    if (!stepAlterOctMap.contains(pitch)
      || !typeMap.contains(type))
      {
      // TODO: error message
      return;
    }
    StepAlterOct sao = stepAlterOctMap.value(pitch);

    int dur = WHOLE_DUR / type.toInt();
    if (dots == 1) dur = 3 * dur / 2;
    out << "      <note>" << endl;
    if (grace) out << "        <grace/>" << endl;
    out << "        <pitch>" << endl;
    out << "          <step>" << sao.s << "</step>" << endl;
    if (sao.a) out << "          <alter>" << sao.a << "</alter>" << endl;
    out << "          <octave>" << sao.o << "</octave>" << endl;
    out << "        </pitch>" << endl;
    if (!grace)
      out << "        <duration>" << dur << "</duration>" << endl;
    if (tieStart)
      out << "        <tie type=\"start\"/>" << endl;
    if (tieStop)
      out << "        <tie type=\"stop\"/>" << endl;
    out << "        <type>" << typeMap.value(type) << "</type>" << endl;
    if (dots == 1) out << "        <dot/>" << endl;
    if (triplet != ST_NONE)
    {
      out << "        <time-modification>" << endl;
      out << "          <actual-notes>3</actual-notes>" << endl;
      out << "          <normal-notes>2</normal-notes>" << endl;
      out << "        </time-modification>" << endl;
    }
    if (grace)
      out << "        <stem>up</stem>" << endl;
    else
      out << "        <stem>down</stem>" << endl;
    if (tieStart || tieStop || triplet == ST_START || triplet == ST_STOP)
    {
      out << "        <notations>" << endl;
      if (tieStart)
        out << "          <tied type=\"start\"/>" << endl;
      if (tieStop)
        out << "          <tied type=\"stop\"/>" << endl;
      if (triplet == ST_START)
        out << "          <tuplet type=\"start\"/>" << endl;
      if (triplet == ST_STOP)
        out << "          <tuplet type=\"stop\"/>" << endl;
      out << "        </notations>" << endl;
    }
    out << "      </note>" << endl;
  }

  /**
   Write the header.
   */

  void MxmlWriter::header(const QString title, const QString type,
                          const QString composer, const QString footer,
                          const unsigned int temp)
  {
    qDebug() << "MxmlWriter::header()"
        << "title:" << title
        << "type:" << type
        << "composer:" << composer
        << "footer:" << footer
        << "temp:" << temp
        ;

    // save tempo for later use
    tempo = temp;

    // write the header
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    out << "<!DOCTYPE score-partwise PUBLIC "
        << "\"-//Recordare//DTD MusicXML 2.0 Partwise//EN\" "
        << "\"http://www.musicxml.org/dtds/partwise.dtd\">" << endl;
    out << "<score-partwise>" << endl;
    out << "  <work>" << endl;
    out << "    <work-title>" << title << "</work-title>" << endl;
    // TODO work-number is not allowed, replace
    // out << "    <work-number>" << type << "</work-number>" << endl;
    out << "  </work>" << endl;
    out << "  <identification>" << endl;
    out << "    <creator type=\"composer\">" << composer << "</creator>" << endl;
    out << "    <rights>" << footer << "</rights>" << endl;
    out << "    <encoding>" << endl;
    out << "      <software>bww2mxml</software>" << endl;
    // TODO fill in real date
    // out << "      <encoding-date>TBD</encoding-date>" << endl;
    out << "    </encoding>" << endl;
    out << "  </identification>" << endl;
    out << "  <part-list>" << endl;
    out << "    <score-part id=\"P1\">" << endl;
    out << "      <part-name>Music</part-name>" << endl;
    out << "      <score-instrument id=\"P1-I1\">" << endl;
    out << "        <instrument-name>Music</instrument-name>" << endl;
    out << "      </score-instrument>" << endl;
    out << "      <midi-instrument id=\"P1-I1\">" << endl;
    out << "        <midi-channel>1</midi-channel>" << endl;
    out << "        <midi-program>1</midi-program>" << endl;
    out << "      </midi-instrument>" << endl;
    out << "    </score-part>" << endl;
    out << "  </part-list>" << endl;
    out << "  <part id=\"P1\">" << endl;
  }

  /**
   Store beats and beat type for later use.
   */

  void MxmlWriter::tsig(const int bts, const int bt)
  {
    qDebug() << "MxmlWriter::tsig()"
        << "beats:" << bts
        << "beat:" << bt
        ;

    beats = bts;
    beat  = bt;
  }

  /**
   Write the trailer.
   */

  void MxmlWriter::trailer()
  {
    qDebug() << "MxmlWriter::trailer()"
        ;

    out << "  </part>" << endl;
    out << "</score-partwise>" << endl;
  }

} // namespace Bww
