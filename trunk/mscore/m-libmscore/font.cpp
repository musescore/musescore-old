

#include <QtCore/QString>

#include "font.h"


void Font::setBold(bool) {}
void Font::setItalic(bool) {}
void Font::setUnderline(bool) {}

//---------------------------------------------------------
//   FontMetricsF
//---------------------------------------------------------

FontMetricsF::FontMetricsF(const Font& f)
      {
      _font = f;
      }

QRectF FontMetricsF::boundingRect(const QString& s) const
      {
      qreal asc, desc, leading;

      qreal w = textMetrics(_font.family(), s, _font.size(),
        &asc, &desc, &leading);

      return QRectF(0, -asc, w, asc + desc);
      }

QRectF FontMetricsF::tightBoundingRect(const QString& s) {
      return boundingRect(s);
      }

QRectF FontMetricsF::boundingRect(const QChar& c) const
      {
      QString s = c;
      return boundingRect(s);
      }

QRectF FontMetricsF::boundingRect(const QRectF&, int, const QString& s)
      {
      return boundingRect(s);
      }

qreal FontMetricsF::width(const QChar& c) const
      {
      return boundingRect(c).width();
      }

qreal FontMetricsF::width(const QString& s) const
      {
      return boundingRect(s).width();
      }

qreal FontMetricsF::height() const
      {
      return boundingRect("lj").height();
      }

qreal FontMetricsF::lineSpacing() const
      {
      return height() + 2.0;
      }


