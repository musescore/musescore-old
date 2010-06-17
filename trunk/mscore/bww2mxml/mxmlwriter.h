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

#ifndef MXMLWRITER_H
#define MXMLWRITER_H

/**
 \file
 Definition of class MxmlWriter
 */

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include "writer.h"

class QIODevice;

namespace Bww {

  /**
   The writer that generates MusicXML output.
   */

  class MxmlWriter : public Writer
  {
  public:
    MxmlWriter();
    void beginMeasure();
    void endMeasure();
    void header(const QString title, const QString type,
                const QString composer, const QString footer);
    void note(const QString pitch, const QString beam,
              const QString type, const int dots,
              bool tieStart = false, bool tieStop = false,
              StartStop triplet = ST_NONE,
              bool grace = false);
    void setOutDevice(QIODevice *outDevice) { out.setDevice(outDevice); }
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
    QTextStream out;                                    ///< The output text stream
    int beats;                                          ///< Number of beats
    int beat;                                           ///< Beat type
    QMap<QString, StepAlterOct> stepAlterOctMap;        ///< Map bww pitch to step/alter/oct
    QMap<QString, QString> typeMap;                     ///< Map bww note types to MusicXML
    unsigned int measureNumber;                         ///< Current measure number
  };

} // namespace Bww

#endif // MXMLWRITER_H
