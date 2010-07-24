//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#include "fret.h"
#include "measure.h"
#include "system.h"
#include "score.h"
#include "fretcanvas.h"
#include "preferences.h"
#include "tablature.h"
#include "chord.h"
#include "note.h"

static const int DEFAULT_STRINGS = 6;
static const int DEFAULT_FRETS = 5;

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::FretDiagram(Score* score)
   : Element(score)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _strings    = DEFAULT_STRINGS;
      _frets      = DEFAULT_FRETS;
      _maxFrets   = 24;
      maxStrings  = 0;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      _fretOffset = 0;
      font.setFamily("FreeSans");
      int size = lrint(4.0 * DPI * mag()/ PPI);
      font.setPixelSize(size);
      }

FretDiagram::FretDiagram(const FretDiagram& f)
   : Element(f)
      {
      _strings    = f._strings;
      _frets      = f._frets;
      _fretOffset = f._fretOffset;
      _maxFrets   = f._maxFrets;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      font        = f.font;

      if (f._dots) {
            _dots = new char[_strings];
            memcpy(_dots, f._dots, _strings);
            }
      if (f._marker) {
            _marker = new char[_strings];
            memcpy(_marker, f._marker, _strings);
            }
      if (f._fingering) {
            _fingering = new char[_strings];
            memcpy(_fingering, f._fingering, _strings);
            }
      }

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::~FretDiagram()
      {
      delete _dots;
      delete _marker;
      delete _fingering;
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF FretDiagram::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      double yp = y();
      if (system)
            yp += system->staffY(staffIdx());
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF FretDiagram::dragAnchor() const
      {
      Measure* m     = measure();
      System* system = m->system();
      double yp      = system->staff(staffIdx())->y() + system->y();
      double xp      = m->tick2pos(segment()->tick()) + m->canvasPos().x();
      QPointF p1(xp, yp);

      double tw = width();
      double th = height();
      double x  = 0.0;
      double y  = 0.0;
      if (_align & ALIGN_BOTTOM)
            y = th;
      else if (_align & ALIGN_VCENTER)
            y = (th * .5);
      else if (_align & ALIGN_BASELINE)
            y = baseLine();
      if (_align & ALIGN_RIGHT)
            x = tw;
      else if (_align & ALIGN_HCENTER)
            x = (tw * .5);
      return QLineF(p1, abbox().topLeft() + QPointF(x, y));
      }

//---------------------------------------------------------
//   setStrings
//---------------------------------------------------------

void FretDiagram::setStrings(int n)
      {
      if (n <= maxStrings) {
            _strings = n;
            return;
            }
      maxStrings = n;
      if (_dots) {
            char* ndots = new char[n];
            memcpy(ndots, _dots, _strings);
            for (int i = _strings; i < n; ++i)
                  ndots[i] = 0;
            delete _dots;
            _dots = ndots;
            }
      if (_marker) {
            char* nmarker = new char[n];
            memcpy(nmarker, _marker, _strings);
            for (int i = _strings; i < n; ++i)
                  nmarker[i] = 0;
            delete _marker;
            _marker = nmarker;
            }
      if (_fingering) {
            char* nfingering = new char[n];
            memcpy(nfingering, _fingering, _strings);
            for (int i = _strings; i < n; ++i)
                  nfingering[i] = 0;
            delete _fingering;
            _fingering = nfingering;
            }
      _strings = n;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void FretDiagram::init(Tablature* tab, Chord* chord)
      {
      if (tab == 0)
            setStrings(6);
      else
            setStrings(tab->strings());
      if (tab) {
            for (int string = 0; string < _strings; ++string)
                  _marker[string] = 'X';
            foreach(const Note* note, chord->notes()) {
                  int string;
                  int fret;
                  if (tab->convertPitch(note->ppitch(), &string, &fret))
                        setDot(string, fret);
                  }
            _maxFrets = tab->frets();
            }
      else
            _maxFrets = 6;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FretDiagram::draw(QPainter& p, ScoreView*) const
      {
      QPen pen(p.pen());
      double _spatium = spatium();
      pen.setWidthF(lw2);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);
      p.setBrush(pen.color());
      double x2 = (_strings-1) * stringDist;
      p.drawLine(QLineF(-lw1*.5, 0.0, x2+lw1*.5, 0.0));

      pen.setWidthF(lw1);
      p.setPen(pen);
      double y2 = (_frets+1) * fretDist - fretDist*.5;
      for (int i = 0; i < _strings; ++i) {
            double x = stringDist * i;
            p.drawLine(QLineF(x, _fretOffset ? -_spatium*.2 : 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            double y = fretDist * i;
            p.drawLine(QLineF(0.0, y, x2, y));
            }
      for (int i = 0; i < _strings; ++i) {
            if (_dots && _dots[i]) {
                  double dotd = stringDist * .6;
                  int fret = _dots[i] - 1;
                  double x = stringDist * i - dotd * .5;
                  double y = fretDist * fret + fretDist * .5 - dotd * .5;
                  p.drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            if (_marker && _marker[i]) {
                  p.setFont(font);
                  double x = stringDist * i;
                  double y = -fretDist * .1;
                  p.drawText(QRectF(x, y, 0.0, 0.0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QChar(_marker[i]));
                  }
            }
      if (_fretOffset > 0) {
            p.drawText(QRectF(-stringDist * .4, 0.0, 0.0, fretDist),
               Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip,
               QString("%1").arg(_fretOffset+1));
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
      {
      double _spatium = spatium();
      lw1             = _spatium * 0.08;
      lw2             = _fretOffset ? lw1 : _spatium * 0.2;
      stringDist      = _spatium * .7;
      fretDist        = _spatium * .8;

      double w = stringDist * (_strings-1);
      double h = _frets * fretDist + fretDist * .5;
      double y = 0.0;
      double dotd = stringDist * .6;
      double x = -((dotd+lw1) * .5);
      w += dotd + lw1;
      if (_marker) {
            QFontMetricsF fm(font);
            y = -(fretDist * .1 + fm.height());
            h -= y;
            }
      setbbox(QRectF(x, y, w, h));
      Element::layout();      // alignment & offset
      setPos(ipos() + QPointF(-w * .5, - (h + _spatium * 1.5)));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FretDiagram::write(Xml& xml) const
      {
      xml.stag("FretDiagram");
      Element::writeProperties(xml, 0);

      if (_strings != DEFAULT_STRINGS)
            xml.tag("strings", _strings);
      if (_frets != DEFAULT_FRETS)
            xml.tag("frets", _frets);
      if (_fretOffset)
            xml.tag("fretOffset", _fretOffset);
      for (int i = 0; i < _strings; ++i) {
            if ((_dots && _dots[i]) || (_marker && _marker[i]) || (_fingering && _fingering[i])) {
                  xml.stag(QString("string no=\"%1\"").arg(i));
                  if (_dots && _dots[i])
                        xml.tag("dot", _dots[i]);
                  if (_marker && _marker[i])
                        xml.tag("marker", _marker[i]);
                  if (_fingering && _fingering[i])
                        xml.tag("fingering", _fingering[i]);
                  xml.etag();
                  }
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FretDiagram::read(QDomElement e)
      {
      delete _dots;
      delete _marker;
      delete _fingering;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      _fretOffset = 0;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "strings")
                  _strings = val;
            else if (tag == "frets")
                  _frets = val;
            else if (tag == "fretOffset")
                  _fretOffset = val;
            else if (tag == "string") {
                  int no = e.attribute("no").toInt();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        int val = ee.text().toInt();
                        if (tag == "dot")
                              setDot(no, val);
                        else if (tag == "marker")
                              setMarker(no, val);
                        else if (tag == "fingering")
                              setFingering(no, val);
                        else
                              domError(ee);
                        }
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setDot
//---------------------------------------------------------

void FretDiagram::setDot(int string, int fret)
      {
      if (_dots == 0) {
            _dots = new char[_strings];
            memset(_dots, 0, _strings);
            }
      _dots[string] = fret;
      setMarker(string, 0);
      }

//---------------------------------------------------------
//   setMarker
//---------------------------------------------------------

void FretDiagram::setMarker(int string, int marker)
      {
      if (_marker == 0) {
            _marker = new char[_strings];
            memset(_marker, 0, _strings);
            }
      _marker[string] = marker;
      }

//---------------------------------------------------------
//   setFingering
//---------------------------------------------------------

void FretDiagram::setFingering(int string, int finger)
      {
      if (_fingering == 0) {
            _fingering = new char[_strings];
            memset(_fingering, 0, _strings);
            }
      _fingering[string] = finger;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool FretDiagram::genPropertyMenu(QMenu* popup) const
      {
      QAction* a;
      if (visible())
            a = popup->addAction(tr("Set Invisible"));
      else
            a = popup->addAction(tr("Set Visible"));
      a->setData("invisible");
      a = popup->addAction(tr("Color..."));
      a->setData("color");
      a = popup->addAction(tr("Fret Diagram Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void FretDiagram::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            FretDiagram* nFret = const_cast<FretDiagram*>(clone());
            FretDiagramProperties fp(nFret, 0);
            int rv = fp.exec();
            if (rv) {
                  score()->undoChangeElement(this, nFret);
                  return;
                  }
            delete nFret;
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   FretDiagramProperties
//---------------------------------------------------------

FretDiagramProperties::FretDiagramProperties(FretDiagram* _fd, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      fd = _fd;
      frets->setValue(fd->frets());
      strings->setValue(fd->strings());
      diagram->setFretDiagram(fd);

      diagramScrollBar->setRange(0, fd->maxFrets());
      diagramScrollBar->setValue(fd->fretOffset());

      connect(strings, SIGNAL(valueChanged(int)), SLOT(stringsChanged(int)));
      connect(frets, SIGNAL(valueChanged(int)), SLOT(fretsChanged(int)));
      connect(diagramScrollBar, SIGNAL(valueChanged(int)), SLOT(fretOffsetChanged(int)));
      }

//---------------------------------------------------------
//   fretsChanged
//---------------------------------------------------------

void FretDiagramProperties::fretsChanged(int val)
      {
      fd->setFrets(val);
      diagram->update();
      }

//---------------------------------------------------------
//   stringsChanged
//---------------------------------------------------------

void FretDiagramProperties::stringsChanged(int val)
      {
      fd->setStrings(val);
      diagram->update();
      }

//---------------------------------------------------------
//   fretOffsetChanged
//---------------------------------------------------------

void FretDiagramProperties::fretOffsetChanged(int val)
      {
      fd->setFretOffset(val);
      diagram->update();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void FretCanvas::paintEvent(QPaintEvent* ev)
      {
      double mag        = 1.5;
      double _spatium   = 20.0 * mag;
      double lw1        = _spatium * 0.08;
      int fretOffset    = diagram->fretOffset();
      double lw2        = fretOffset ? lw1 : _spatium * 0.2;
      double stringDist = _spatium * .7;
      double fretDist   = _spatium * .8;
      int _strings      = diagram->strings();
      int _frets        = diagram->frets();
      char* _dots       = diagram->dots();
      char* _marker     = diagram->marker();

      double w  = (_strings - 1) * stringDist;
      double xo = (width() - w) * .5;
      double h  = (_frets * fretDist) + fretDist * .5;
      double yo = (height() - h) * .5;

      QFont font("FreeSans");
      int size = lrint(18.0 * mag);
      font.setPixelSize(size);

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.translate(xo, yo);

      QPen pen(p.pen());
      pen.setWidthF(lw2);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);
      p.setBrush(pen.color());
      double x2 = (_strings-1) * stringDist;
      p.drawLine(QLineF(-lw1 * .5, 0.0, x2+lw1*.5, 0.0));

      pen.setWidthF(lw1);
      p.setPen(pen);
      double y2 = (_frets+1) * fretDist - fretDist*.5;
      for (int i = 0; i < _strings; ++i) {
            double x = stringDist * i;
            p.drawLine(QLineF(x, fretOffset ? -_spatium*.2 : 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            double y = fretDist * i;
            p.drawLine(QLineF(0.0, y, x2, y));
            }
      for (int i = 0; i < _strings; ++i) {
            p.setPen(Qt::NoPen);
            if (_dots && _dots[i]) {
                  double dotd = stringDist * .6 + lw1;
                  int fret = _dots[i] - 1;
                  double x = stringDist * i - dotd * .5;
                  double y = fretDist * fret + fretDist * .5 - dotd * .5;
                  p.drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            p.setPen(pen);
            if (_marker && _marker[i]) {
                  p.setFont(font);
                  double x = stringDist * i;
                  double y = -fretDist * .1;
                  p.drawText(QRectF(x, y, 0.0, 0.0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QChar(_marker[i]));
                  }
            }
      if (cfret > 0 && cfret <= _frets && cstring >= 0 && cstring < _strings) {
            double dotd;
            if (_dots[cstring] != cfret) {
                  p.setPen(Qt::NoPen);
                  dotd = stringDist * .6 + lw1;
                  }
            else {
                  p.setPen(pen);
                  dotd = stringDist * .6;
                  }
            double x = stringDist * cstring - dotd * .5;
            double y = fretDist * (cfret-1) + fretDist * .5 - dotd * .5;
            p.setBrush(Qt::lightGray);
            p.drawEllipse(QRectF(x, y, dotd, dotd));
            }
      if (fretOffset > 0) {
            p.drawText(QRectF(-stringDist * .4, 0.0, 0.0, fretDist),
               Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip,
               QString("%1").arg(fretOffset+1));
            }
      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   getPosition
//---------------------------------------------------------

void FretCanvas::getPosition(const QPointF& p, int* string, int* fret)
      {
      double mag = 1.5;
      double _spatium   = 20.0 * mag;
      int _strings      = diagram->strings();
      int _frets        = diagram->frets();
      double stringDist = _spatium * .7;
      double fretDist   = _spatium * .8;

      double w  = (_strings - 1) * stringDist;
      double xo = (width() - w) * .5;
      double h  = (_frets * fretDist) + fretDist * .5;
      double yo = (height() - h) * .5;
      *fret  = (p.y() - yo + fretDist) / fretDist;
      *string = (p.x() - xo + stringDist * .5) / stringDist;
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void FretCanvas::mousePressEvent(QMouseEvent* ev)
      {
      int string;
      int fret;
      getPosition(ev->pos(), &string, &fret);

      int _strings = diagram->strings();
      int _frets   = diagram->frets();
      if (fret < 0 || fret > _frets || string < 0 || string >= _strings)
            return;

      char* _marker = diagram->marker();
      char* _dots   = diagram->dots();
      if (fret == 0) {
            switch(_marker[string]) {
                  case 'O':
                        _marker[string] = 'X';
                        break;
                  case 'X':
                        _marker[string] = 'O';
                        break;
                  default:
                        _marker[string] = 'O';
                        _dots[string] = 0;
                        break;
                  }
            }
      else {
            if (_dots[string] == fret) {
                  _dots[string] = 0;
                  _marker[string] = 'O';
                  }
            else {
                  _dots[string] = fret;
                  _marker[string] = 0;
                  }
            }
      update();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void FretCanvas::mouseMoveEvent(QMouseEvent* ev)
      {
      int string;
      int fret;
      getPosition(ev->pos(), &string, &fret);
      if (string != cstring || cfret != fret) {
            cfret = fret;
            cstring = string;
            update();
            }
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void FretCanvas::mouseReleaseEvent(QMouseEvent*)
      {
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void FretCanvas::dragEnterEvent(QDragEnterEvent*)
      {
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void FretCanvas::dragMoveEvent(QDragMoveEvent*)
      {
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void FretCanvas::dropEvent(QDropEvent*)
      {
      }

//---------------------------------------------------------
//   FretCanvas
//---------------------------------------------------------

FretCanvas::FretCanvas(QWidget* parent)
   : QFrame(parent)
      {
      setAcceptDrops(true);
      setFrameStyle(QFrame::Raised | QFrame::Panel);
      cstring = -2;
      cfret   = -2;
      }

//---------------------------------------------------------
//   setFretDiagram
//---------------------------------------------------------

void FretCanvas::setFretDiagram(FretDiagram* fd)
      {
      diagram = fd;
      update();
      }

