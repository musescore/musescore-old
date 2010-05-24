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

#ifndef WRITER_H
#define WRITER_H

/**
 \file
 Definition of class Writer
 */

#include <QMap>
#include <QString>
#include <QTextStream>

class QIODevice;

namespace Bww {

  /**
   The writer that generates the output.
   */

  class Writer
  {
  public:
    Writer(QIODevice *outDevice);
    void header(const QString title, const QString type,
                const QString composer, const QString footer);
    void tempo(const int beats, const int beat);
    void trailer();
    void beginMeasure();
    void endMeasure();
    void note(const QString pitch, const QString beam,
              const QString type, const int dots,
              bool grace = false);
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

#endif // WRITER_H
