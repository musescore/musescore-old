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

#include "pdf.h"
extern "C" {
#include <fitz.h>
#include <mupdf.h>
      }

int Pdf::references;
static fz_context* ctx;
// static fz_glyph_cache* cache;

//---------------------------------------------------------
//   numPages
//---------------------------------------------------------

int Pdf::numPages() const
      {
      return fz_count_pages(doc);
      }

//---------------------------------------------------------
//   Pdf
//---------------------------------------------------------

Pdf::Pdf()
      {
      if (references == 0) {
            // ctx = fz_new_context(&fz_alloc_default, 256 << 20);  // 256MB cache
            ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);  // 256MB cache
//            cache = fz_new_glyph_cache(ctx);
            }
      ++references;
      doc = 0;
      }

//---------------------------------------------------------
//   open
//---------------------------------------------------------

bool Pdf::open(const QString& path)
      {
      char* name = path.toAscii().data();
      fz_try(ctx) {
            doc = fz_open_document(ctx, name);
            // pdf_load_page_tree(xref);
            }
      fz_catch(ctx) {
            fz_close_document(doc);
            doc = 0;
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   ~Pdf
//---------------------------------------------------------

Pdf::~Pdf()
      {
      if (doc)
            fz_close_document(doc);
      doc = 0;
      --references;
      if (references == 0) {
//            fz_free_glyph_cache(ctx, cache);
//            cache = 0;
            fz_free_context(ctx);
            ctx = 0;
            }
      }

//---------------------------------------------------------
//   page
//---------------------------------------------------------

QImage Pdf::page(int i)
      {
      fz_page* page = fz_load_page(doc, i);
      if (page == 0) {
            printf("cannot load page %d\n", i);
            return QImage();
            }
      static const float resolution = 300.0;
      const float zoom = resolution / 72.0;

      fz_rect bounds = fz_bound_page(doc, page);
      fz_matrix ctm  = fz_scale(zoom, zoom);
      fz_bbox bbox   = fz_round_rect(fz_transform_rect(ctm, bounds));
      fz_pixmap* pix = fz_new_pixmap_with_rect(ctx, fz_device_gray, bbox);

      fz_clear_pixmap_with_value(ctx, pix, 255);
      fz_device* dev = fz_new_draw_device(ctx, pix);
      fz_run_page(doc, page, dev, ctm, NULL);
      fz_free_device(dev);
      dev = NULL;

      int w = pix->w;
      int h = pix->h;

      QImage image(w, h, QImage::Format_MonoLSB);
      QVector<QRgb> ct(2);
      ct[0] = qRgb(255, 255, 255);
      ct[1] = qRgb(0, 0, 0);
      image.setColorTable(ct);

      uchar* s   = pix->samples;
      int stride = image.bytesPerLine();
      int bytes  = w >> 3;
      for (int line = 0; line < h; ++line) {
            uchar* d = image.bits() + stride * line;
            for (int col = 0; col < bytes; ++col) {
                  uchar data = 0;
                  for (int i = 0; i < 8; ++i) {
                        uchar v = *s++;
                        s++;
                        data >>= 1;
                        if (v < 128)
                              data |= 0x80;
                        }
                  *d++ = data;
                  }
            }
      fz_drop_pixmap(ctx, pix);
      // fz_free_page(ctx, page);
      return image;
      }

