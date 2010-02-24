//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __PAGE_H__
#define __PAGE_H__

class Scan;

struct Measure {
      QRectF boundingRect;
      };

struct Staff {
      QRectF boundingRect;
      QList<double> lines;
      };

struct System {
      QList<Staff*> staves;
      };

struct HLine {
      int x1, x2, y;
      HLine() {}
      HLine(int a, int b, int c) : x1(a), x2(b), y(c) {}
      };

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

class Page {
      Scan* _scan;
      QImage _image;
      int cropL, cropR;       // crop values in words (32 bit) units
      int cropT, cropB;       // crop values in pixel units

      QList<QRect> _slices;
      QList<QRectF> staves;
      QList<HLine> slines;

      double _spatium;

      QList<QLine>  lines;
      QList<QLineF> barlines;
      QList<QRect> _notes;

      void read1();
      bool dot(int x, int y) const;
      void crop();
      void slice();
      double skew(const QRect&);
      void deSkew();
      void getStaffLines();
      double xproject2(int y);
      int xproject(const uint* p, int wl);
      void radonTransform(ulong* projection, int w, int n, const QRect&);
      void searchNotes(int line, int x1, int x2, int y);

   public:
      Page(Scan* _parent);
      void setImage(const QImage& i)     { _image = i; }
      const QImage& image() const        { return _image; }
      void read();
      int width() const                  { return _image.width(); }
      int height() const                 { return _image.height(); }
      const uint* scanLine(int y) const  { return (const uint*)_image.scanLine(y); }
      int wordsPerLine() const           { return (_image.bytesPerLine() + 3)/4; }

      const QList<QLine>& sl()           { return lines;    }
      const QList<HLine>& l()            { return slines;   }
      const QList<QRectF>& r()           { return staves;   }
      const QList<QLineF>& bl()          { return barlines; }
      const QList<QRect>& slices() const { return _slices;  }
      const QList<QRect>& notes() const  { return _notes;   }
      double spatium() const             { return _spatium; }
      double staffDistance() const;
      double systemDistance() const;
      };

#endif


