//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __NOTE_H__
#define __NOTE_H__

/**
 \file
 Definition of classes Note and ShadowNote.
*/

#include "globals.h"
#include "element.h"
#include "symbol.h"
#include "accidental.h"
#include "durationtype.h"

class Tie;
class Chord;
class Text;
class Score;
class Sym;
class ScoreView;
class AccidentalState;

extern const int noteHeads[2][HEAD_GROUPS][4];

//---------------------------------------------------------
//   NoteHead
//---------------------------------------------------------

class NoteHead : public Symbol {

   public:
      NoteHead(Score*);
      NoteHead &operator=(const NoteHead&);
      virtual NoteHead* clone() const  { return new NoteHead(*this); }
      virtual ElementType type() const { return NOTEHEAD; }
      virtual void write(Xml& xml) const;
      };

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

/**
 Graphic representation of a note.
*/

class Note : public Element {
      Q_DECLARE_TR_FUNCTIONS(Note)

      int _subchannel;        ///< articulation
      int _line;              ///< y-Position; 0 - top line.

      bool dragMode;
      char _pitch;            ///< Note pitch as midi value (0 - 127).
      char _tpc;              ///< tonal pitch class

      char _ppitch;           ///< played pitch (honor voltas etc.); cached value
      double _tuning;         ///< pitch offset in cent, playable only by internal synthesizer

      ValueType _veloType;
      char _velocity;         ///< midi playback velocity (0 - 127);
      int _veloOffset;        ///< velocity user offset in percent

      ValueType _onTimeType;
      int _onTimeOffset;      ///< start note offset in ticks
      int _onTimeUserOffset;  ///< start note user offset

      ValueType _offTimeType;
      int _offTimeOffset;     ///< stop note offset in ticks
      int _offTimeUserOffset; ///< stop note user offset

      char _headGroup;
      NoteHeadType _headType;

      bool _mirror;           ///< True if note is mirrored at stem.
      DirectionH _userMirror; ///< user override of mirror

      bool _hidden;           ///< markes this note as the hidden one if there are
                              ///< overlapping notes; hidden notes are not played
                              ///< and heads + accidentals are not shown

      int _userAccidental;
      Accidental* _accidental;

      ElementList _el;        ///< fingering, other text, symbols or images
      Tie* _tieFor;
      Tie* _tieBack;

      int _lineOffset;        ///< Used during mouse dragging.

      virtual bool isMovable() const;
      virtual QRectF drag(const QPointF& s);
      virtual void endDrag();

   public:
      Note(Score* s = 0);
      Note(const Note&);
      Note &operator=(const Note&);
      ~Note();
      virtual Note* clone() const      { return new Note(*this); }
      virtual ElementType type() const { return NOTE; }
      virtual QRectF bbox() const;
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      virtual void layout();
      void layout1(AccidentalState*);
      virtual void scanElements(void* data, void (*func)(void*, Element*));
      virtual void setTrack(int val);

      int totalTicks() const;

      double headWidth() const;
      double headHeight() const;
      int noteHead() const;
      int headGroup() const            { return _headGroup; }
      NoteHeadType headType() const    { return _headType;  }
      void setHeadGroup(int val);
      void setHeadType(NoteHeadType t) { _headType = t;     }

      int pitch() const               { return _pitch;    }
      void setPitch(int val);
      void setPitch(int a, int b);
      int ppitch() const              { return _ppitch;   }
      void setPpitch(int v)           { _ppitch = v;      }
      double tuning() const           { return _tuning;   }
      void setTuning(double v)        { _tuning = v;      }

      int tpc() const                 { return _tpc;      }
      void setTpc(int v);
      void changePitch(int val);
      void setTpcFromPitch();

      void setAccidentalType(int);
      int accidentalType() const;

      int userAccidental() const      { return _userAccidental; }
      void setUserAccidental(int v)   { _userAccidental = v;    }
      Accidental* accidental() const  { return _accidental;     }

      int line() const                { return _line + _lineOffset;   }
      void setLine(int n);

      virtual void add(Element*);
      virtual void remove(Element*);

      bool mirror() const             { return _mirror; }
      void setMirror(bool val)        { _mirror = val;  }

      Tie* tieFor() const             { return _tieFor;     }
      Tie* tieBack() const            { return _tieBack;    }
      void setTieFor(Tie* t)          { _tieFor = t;        }
      void setTieBack(Tie* t);

      Chord* chord() const            { return (Chord*)parent(); }
      void setChord(Chord* a)         { setParent((Element*)a);    }

      virtual void draw(QPainter&) const;
      virtual void read(QDomElement);
      void write(Xml& xml, int start, int end) const;
      virtual void write(Xml& xml) const { write(xml, 0, 0); }

      QPointF stemPos(bool upFlag) const;    ///< Point to connect stem.
      double stemYoff(bool upFlag) const;

      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      bool hidden() const              { return _hidden; }
      void setHidden(bool val)         { _hidden = val;  }

      NoteType noteType() const;

      ElementList* el()                { return &_el; }
      const ElementList* el() const    { return &_el; }

      int subchannel() const           { return _subchannel; }
      void setSubchannel(int val)      { _subchannel = val;  }

      DirectionH userMirror() const    { return _userMirror; }
      void setUserMirror(DirectionH d) { _userMirror = d; }

      virtual void toDefault();
      virtual void setMag(double val);

      ValueType veloType() const       { return _veloType;          }
      void setVeloType(ValueType v)    { _veloType = v;             }
      int velocity() const             { return _velocity;          }
      void setVelocity(int v)          { _velocity = v;             }
      int veloOffset() const           { return _veloOffset;        }
      void setVeloOffset(int v)        { _veloOffset = v;           }

      ValueType onTimeType() const     { return _onTimeType;        }
      void setOnTimeType(ValueType v)  { _onTimeType = v;           }
      int onTimeOffset() const         { return _onTimeOffset;      }
      void setOnTimeOffset(int v)      { _onTimeOffset = v;         }
      int onTimeUserOffset() const     { return _onTimeUserOffset;  }
      void setOnTimeUserOffset(int v)  { _onTimeUserOffset = v;     }

      ValueType offTimeType() const    { return _offTimeType;       }
      void setOffTimeType(ValueType v) { _offTimeType = v;          }
      int offTimeOffset() const        { return _offTimeOffset;     }
      void setOffTimeOffset(int v)     { _offTimeOffset = v;        }
      int offTimeUserOffset() const    { return _offTimeUserOffset; }
      void setOffTimeUserOffset(int v) { _offTimeUserOffset = v;    }
      };

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

/**
 Graphic representation of a shadow note,
 which shows the note insert position in note entry mode.
*/

class ShadowNote : public Element {
      int _line;
      int _headGroup;
      int _head;

   public:
      ShadowNote(Score*);
      virtual ShadowNote* clone() const { return new ShadowNote(*this); }
      virtual ElementType type() const  { return SHADOW_NOTE; }
      int line() const                  { return _line;   }
      void setLine(int n)               { _line = n;      }
      virtual QRectF bbox() const;
      virtual void draw(QPainter& p) const;
      int headGroup() const            { return _headGroup; }
      void setHeadGroup(int val);
      void setHead(int val)            { _head = val; }
      int head() const                 { return _head; }
      };

#endif

