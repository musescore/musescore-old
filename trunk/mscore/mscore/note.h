//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
class Bend;

extern const int noteHeads[2][HEAD_GROUPS][4];

//---------------------------------------------------------
//   NoteHead
//---------------------------------------------------------

class NoteHead : public Symbol {

   public:
      NoteHead(Score* s) : Symbol(s) {}
      NoteHead &operator=(const NoteHead&);
      virtual NoteHead* clone() const  { return new NoteHead(*this); }
      virtual ElementType type() const { return NOTEHEAD; }
      virtual void write(Xml& xml) const;
      };

//---------------------------------------------------------
//   NoteDot
//---------------------------------------------------------

class NoteDot : public Symbol {

      int _idx;

   public:
      NoteDot(Score*);
      NoteDot &operator=(const NoteHead&);
      virtual NoteDot* clone() const  { return new NoteDot(*this); }
      virtual ElementType type() const { return NOTEDOT; }
      int idx() const      { return _idx; }
      void setIdx(int val) { _idx = val; }
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
      char _line;             ///< y-Position; 0 - top line.
      char _fret;             ///< for tablature view
      char _string;
      bool _ghost;            ///< ghost note (guitar: death note)

      bool dragMode;
      char _pitch;            ///< Note pitch as midi value (0 - 127).
      char _tpc;              ///< tonal pitch class

      char _ppitch;           ///< played pitch (honor voltas etc.); cached value

      bool _hidden;           ///< markes this note as the hidden one if there are
                              ///< overlapping notes; hidden notes are not played
                              ///< and heads + accidentals are not shown

      double _tuning;         ///< pitch offset in cent, playable only by internal synthesizer

      ValueType _veloType;
      int _veloOffset;        ///< velocity user offset in percent, or absolute velocity for this note

      int _onTimeOffset;      ///< start note offset in ticks
      int _onTimeUserOffset;  ///< start note user offset

      int _offTimeOffset;     ///< stop note offset in ticks
      int _offTimeUserOffset; ///< stop note user offset

      char _headGroup;
      NoteHeadType _headType;

      bool _mirror;           ///< True if note is mirrored at stem.
      DirectionH _userMirror; ///< user override of mirror

      AccidentalType _userAccidental;
      Accidental* _accidental;

      ElementList _el;        ///< fingering, other text, symbols or images
      Tie* _tieFor;
      Tie* _tieBack;
      Bend* _bend;

      NoteDot* _dots[3];

      int _lineOffset;        ///< Used during mouse dragging.

      virtual QRectF drag(const QPointF& s);
      virtual void endDrag();
      virtual void endEdit();

   public:
      Note(Score* s = 0);
      Note(const Note&);
      Note &operator=(const Note&);
      ~Note();
      virtual Note* clone() const      { return new Note(*this); }
      virtual ElementType type() const { return NOTE; }
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      virtual void layout();
      void layout10(char* tversatz);
      virtual void scanElements(void* data, void (*func)(void*, Element*));
      virtual void setTrack(int val);

      int playTicks() const;

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
      int ppitch() const;
      double tuning() const           { return _tuning;   }
      void setTuning(double v)        { _tuning = v;      }

      int tpc() const                 { return _tpc;      }
      void setTpc(int v);
      void setTpcFromPitch();

      void setAccidentalType(int);
      int accidentalType() const;

      AccidentalType userAccidental() const    { return _userAccidental; }
      void setUserAccidental(AccidentalType v) { _userAccidental = v;    }
      Accidental* accidental() const              { return _accidental;     }

      int line() const                { return _line + _lineOffset;   }
      void setLine(int n);

      int fret() const                { return _fret;   }
      void setFret(int val)           { _fret = val;    }
      int string() const              { return _string; }
      void setString(int val);
      bool ghost() const              { return _ghost;  }
      void setGhost(bool val)         { _ghost = val;   }

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

      virtual void draw(QPainter&, ScoreView*) const;
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
      int veloOffset() const           { return _veloOffset;        }
      void setVeloOffset(int v)        { _veloOffset = v;           }

      int onTimeOffset() const         { return _onTimeOffset;      }
      void setOnTimeOffset(int v)      { _onTimeOffset = v;         }
      int onTimeUserOffset() const     { return _onTimeUserOffset;  }
      void setOnTimeUserOffset(int v)  { _onTimeUserOffset = v;     }

      int offTimeOffset() const        { return _offTimeOffset;     }
      void setOffTimeOffset(int v)     { _offTimeOffset = v;        }
      int offTimeUserOffset() const    { return _offTimeUserOffset; }
      void setOffTimeUserOffset(int v) { _offTimeUserOffset = v;    }

      Bend* bend() const               { return _bend; }
      void setBend(Bend* b)            { _bend = b;    }
      int customizeVelocity(int velo) const;
      NoteDot* dot(int n)              { return _dots[n];           }
      void updateAccidental(char* tversatz);
      };

extern Sym* noteHeadSym(bool up, int group, int n);

#endif

