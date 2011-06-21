//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __TIMESIG_H__
#define __TIMESIG_H__

#include "element.h"
#include "al/sig.h"
#include "mscore.h"

class MuseScoreView;
class Segment;
class Painter;

// subtypes:

enum {
      TSIG_NORMAL,            // use sz/sn text
      TSIG_FOUR_FOUR,
      TSIG_ALLA_BREVE
      };

//---------------------------------------------------------
//   TimeSig
//    Time Signature
//---------------------------------------------------------

class TimeSig : public Element {
      bool	_showCourtesySig;
      QString sz, sn;         // calculated from actualSig() if !customText
      QPointF pz, pn;
      Fraction _nominal;
      Fraction _stretch;      // _nominal / aktualSig()
      bool customText;        // if false, sz and sn are calculated from actualSig()

   public:
      TimeSig(Score*);
      TimeSig(Score* s, int st);
      TimeSig(Score* s, int z, int n);
      TimeSig(Score* s, const Fraction& f);

      TimeSig* clone() const             { return new TimeSig(*this); }
      ElementType type() const           { return TIMESIG; }
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      void setSubtype(int val);
      void draw(Painter*) const;
      void write(Xml& xml) const;
      void read(QDomElement);
      void layout();
      Space space() const;

      Fraction sig() const               { return _nominal; }
      void setSig(const Fraction& f)     { _nominal = f;    }
      void setSig(int a, int b)          { _nominal.set(a,b); }

      Fraction stretch() const           { return _stretch;  }
      void setStretch(const Fraction& f) { _stretch = f;    }
      Fraction actualSig() const         { return _nominal / _stretch; }
      void setActualSig(const Fraction& f);

      bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      Element* drop(const DropData&);

      Segment* segment() const { return (Segment*)parent(); }
      Measure* measure() const { return (Measure*)parent()->parent(); }

      bool showCourtesySig() const       { return _showCourtesySig; };
      void setShowCourtesySig(bool v)    { _showCourtesySig = v;    };

      QString zText() const              { return sz; }
      QString nText() const              { return sn; }
      void setText(const QString&, const QString&);
      void setFrom(const TimeSig*);
      };

#endif

