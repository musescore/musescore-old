//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: exportxml.cpp,v 1.71 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2007 Werner Schweer and others
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
 Lilypond export.
 */

#include "config.h"
#include "score.h"
#include "part.h"
#include "measure.h"
#include "staff.h"
#include "layout.h"
#include "chord.h"
#include "note.h"
#include "rest.h"
#include "timesig.h"
#include "clef.h"
#include "keysig.h"
#include "key.h"
#include "page.h"
#include "text.h"
#include "slur.h"

//---------------------------------------------------------
//   ExportLy
//---------------------------------------------------------

class ExportLy {
      Score* score;
      QFile f;
      QTextStream os;
      int level;        // indent level
      int curTicks;
      bool slur;

      void indent();
      int getLen(int ticks, int* dots);
      void writeLen(int);
      QString tpc2name(int tpc);
      void checkSlur(int, int);

      void writeScore();
      void writeMeasure(Measure*, int);
      void writeKeySig(int);
      void writeTimeSig(TimeSig*);
      void writeClef(int);
      void writeChord(Chord*);
      void writeRest(int, int, int);

   public:
      ExportLy(Score* s) {
            score  = s;
            level  = 0;
            curTicks = division;
            slur   = false;
            }
      bool write(const QString& name);
      };

//---------------------------------------------------------
//   indent
//---------------------------------------------------------

void ExportLy::indent()
      {
      for (int i = 0; i < level; ++i)
            os << "  ";
      }

//---------------------------------------------------------
//   exportLilypond
//---------------------------------------------------------

bool Score::saveLilypond(const QString& name)
      {
      ExportLy em(this);
      return em.write(name);
      }

//---------------------------------------------------------
//   writeClef
//---------------------------------------------------------

void ExportLy::writeClef(int clef)
      {
      os << "\\clef ";
      switch(clef) {
            case CLEF_G:      os << "treble"; break;
            case CLEF_F:      os << "bass"; break;

            case CLEF_G1:
            case CLEF_G2:
            case CLEF_G3:
            case CLEF_F8:
            case CLEF_F15:
            case CLEF_F_B:
            case CLEF_F_C:
            case CLEF_C1:
            case CLEF_C2:
            case CLEF_C3:
            case CLEF_C4:
            case CLEF_TAB:
            case CLEF_PERC:
                  os << "treble"; break;        // TODO
                  break;
            }
      os << " ";
      }

//---------------------------------------------------------
//   writeTimeSig
//---------------------------------------------------------

void ExportLy::writeTimeSig(TimeSig* sig)
      {
      int z, n;
      sig->getSig(&n, &z);
      os << "\\time " << z << "/" << n << " ";
      }

//---------------------------------------------------------
//   writeKeySig
//---------------------------------------------------------

void ExportLy::writeKeySig(int st)
      {
      st = char(st & 0xff);
      if (st == 0)
            return;
      os << "\\key ";
      switch(st) {
            case 7:  os << "fis"; break;
            case 6:  os << "h";   break;
            case 5:  os << "e";   break;
            case 4:  os << "a";   break;
            case 3:  os << "d";   break;
            case 2:  os << "g";   break;
            case 1:  os << "c";   break;
            case -7: os << "ces"; break;
            case -6: os << "ges"; break;
            case -5: os << "des"; break;
            case -4: os << "as";  break;
            case -3: os << "es";  break;
            case -2: os << "bes"; break;
            case -1: os << "f";   break;
            default:
                  printf("illegal key %d\n", st);
                  break;
            }
      os << " \\major ";
      }

//---------------------------------------------------------
//   tpc2name
//---------------------------------------------------------

QString ExportLy::tpc2name(int tpc)
      {
      const char names[] = "fcgdaeb";
      int acc   = ((tpc+1) / 7) - 2;
      QString s(names[(tpc + 1) % 7]);
      switch(acc) {
            case -2: s += "eses"; break;
            case -1: s += "es";  break;
            case  1: s += "is";  break;
            case  2: s += "isis"; break;
            case  0: break;
            default: s += "??"; break;
            }
      return s;
      }

//---------------------------------------------------------
//   writeChord   48
//---------------------------------------------------------

void ExportLy::writeChord(Chord* c)
      {
      Direction d = c->stemDirection();
      if (d == UP)
            os << "\\stemUp ";
      else if (d == DOWN)
            os << "\\stemDown ";

      NoteList* nl = c->noteList();
      if (nl->size() > 1)
            os << "<";
      for (iNote i = nl->begin();;) {
            Note* n = i->second;
            int octave = n->pitch() / 12 - 4;

            os << tpc2name(n->tpc());

            if (octave > 0) {
                  while (octave-- > 0)
                        os << "'";
                  }
            else if (octave < 0) {
                  while (octave++ < 0)
                        os << ",";
                  }
            ++i;
            if (i == nl->end())
                  break;
            os << " ";
            }
      if (nl->size() > 1)
            os << ">";
      writeLen(c->tickLen());
      foreach(NoteAttribute* a, *c->getAttributes()) {
            switch(a->subtype()) {
                  case UfermataSym:
                        os << "\\fermata";
                        break;
                  case DfermataSym:
                        break;
                  case ThumbSym:
                        os << "\\thumb";
                        break;
                  case SforzatoaccentSym:
                        break;
                  case EspressivoSym:
                        os << "\\espressivo";
                        break;
                  case StaccatoSym:
                        os << "\\staccato";
                        break;
                  case UstaccatissimoSym:
                        os << "\\staccatissimo";
                        break;
                  case DstaccatissimoSym:
                        break;
                  case TenutoSym:
                        os << "\\tenuto";
                        break;
                  case UportatoSym:
                        os << "\\portato";
                        break;
                  case DportatoSym:
                        break;
                  case UmarcatoSym:
                        os << "\\markato";
                        break;
                  case DmarcatoSym:
                        break;
                  case OuvertSym:
                        break;
                  case PlusstopSym:
                        os << "\\stopped";
                        break;
                  case UpbowSym:
                        os << "\\upbow";
                        break;
                  case DownbowSym:
                        os << "\\downbow";
                        break;
                  case ReverseturnSym:
                        os << "\\reverseturn";
                        break;
                  case TurnSym:
                        os << "\\turn";
                        break;
                  case TrillSym:
                        os << "\\trill";
                        break;
                  case PrallSym:
                        os << "\\prall";
                        break;
                  case MordentSym:
                        os << "\\mordent";
                        break;
                  case PrallPrallSym:
                        os << "\\prallprall";
                        break;
                  case PrallMordentSym:
                        os << "\\prallmordent";
                        break;
                  case UpPrallSym:
                        os << "\\prallup";
                        break;
                  case DownPrallSym:
                        os << "\\pralldown";
                        break;
                  case UpMordentSym:
                        os << "\\upmordent";
                        break;
                  case DownMordentSym:
                        os << "\\downmordent";
                        break;
#if 0 // TODO: this are now Repeat() elements
                  case SegnoSym:
                        os << "\\segno";
                        break;
                  case CodaSym:
                        os << "\\coda";
                        break;
                  case VarcodaSym:
                        os << "\\varcoda";
                        break;
#endif
                  default:
                        printf("unsupported note attribute %d\n", a->subtype());
                        break;
                  }
            }
      checkSlur(c->tick(), c->staffIdx() * VOICES + c->voice());

      os << " ";
      }

//---------------------------------------------------------
//   getLen
//---------------------------------------------------------

int ExportLy::getLen(int l, int* dots)
      {
      int len  = 4;

      if (l == 6 * division) {
            len  = 1;
            *dots = 1;
            }
      else if (l == 5 * division) {
            len = 1;
            *dots = 2;
            }
      else if (l == 4 * division)
            len = 1;
      else if (l == 3 * division) {
            len = 2;
            *dots = 1;
            }
      else if (l == 2 * division)
            len = 2;
      else if (l == division)
            len = 4;
      else if (l == division / 2)
            len = 8;
      else if (l == division / 4)
            len = 16;
      else if (l == division / 8)
            len = 32;
      else if (l == division / 16)
            len = 64;
      else
            printf("unsupported len %d (%d,%d)\n", l, l/division, l % division);
      return len;
      }

//---------------------------------------------------------
//   writeLen
//---------------------------------------------------------

void ExportLy::writeLen(int ticks)
      {
      int dots = 0;
      int len = getLen(ticks, &dots);
      if (ticks != curTicks) {
            os << len;
            for (int i = 0; i < dots; ++i)
                  os << ".";
            curTicks = ticks;
            }
      }

//---------------------------------------------------------
//   writeRest
//    type = 0    normal rest
//    type = 1    whole measure rest
//    type = 2    spacer rest
//---------------------------------------------------------

void ExportLy::writeRest(int tick, int l, int type)
      {
      if (type == 1) {
            // write whole measure rest
            int z, n;
            score->sigmap->timesig(tick, z, n);
            os << "R1*" << z << "/" << n;
            curTicks = -1;
            }
      else if (type == 2) {
            os << "s";
            writeLen(l);
            }
      else {
            os << "r";
            writeLen(l);
            }
      os << " ";
      }

//---------------------------------------------------------
//   writeMeasure
//---------------------------------------------------------

void ExportLy::writeMeasure(Measure* m, int staffIdx)
      {
      os << "% measure " << m->no()+1 << "\n";
      bool voiceActive[4];

      indent();
      for (int voice = 0; voice < VOICES; ++voice) {
            voiceActive[voice] = false;

            for(Segment* s = m->first(); s; s = s->next()) {
                  Element* e = s->element(staffIdx * VOICES + voice);
                  if (e) {
                        voiceActive[voice] = true;
                        break;
                        }
                  }
            }
      int activeVoices = 0;
      for (int voice = 0; voice < VOICES; ++voice) {
            if (voiceActive[voice])
                  ++activeVoices;
            }
      if (activeVoices > 1) {
            os << "<<\n";
            ++level;
            indent();
            }
      int nvoices = activeVoices;
      for (int voice = 0; voice < VOICES; ++voice) {
            if (!voiceActive[voice])
                  continue;
            if (activeVoices > 1) {
                  indent();
                  os << "{ ";
                  }
            int tick = m->tick();
            for(Segment* s = m->first(); s; s = s->next()) {
                  Element* e = s->element(staffIdx * VOICES + voice);
                  if (e == 0 || e->generated())
                        continue;
                  switch(e->type()) {
                        case CLEF:
                              writeClef(e->subtype());
                              break;
                        case TIMESIG:
                              writeTimeSig((TimeSig*)e);
                              break;
                        case KEYSIG:
                              writeKeySig(e->subtype());
                              break;
                        case CHORD:
                              {
                              if (voice) {
                                    int ntick = e->tick() - tick;
                                    if (ntick > 0)
                                          writeRest(tick, ntick, 2);
                                    tick += ntick;
                                    }
                              writeChord((Chord*)e);
                              tick += e->tickLen();
                              }
                              break;
                        case REST:
                              {
                              int l = e->tickLen();
                              if (l == 0) {
                                    l = ((Rest*)e)->segment()->measure()->tickLen();
                                    writeRest(e->tick(), l, 1);
                                    }
                              else
                                    writeRest(e->tick(), l, 0);
                              tick += l;
                              }
                              break;
                        case BAR_LINE:    // automatically set by lilypond
                              break;
                        case BREATH:
                              os << "\\breathe ";
                              break;
                        default:
                              printf("Export Lilypond: unsupported element <%s>\n", e->name());
                              break;
                        }
                  }
            if (activeVoices > 1) {
                  os << "}\n";
                  indent();
                  }
            --nvoices;
            if (nvoices == 0)
                  break;
            os << "\\\\\n";
            indent();
            }
      if (activeVoices > 1) {
            os << ">>";
            --level;
            }
      }

//---------------------------------------------------------
//   writeScore
//---------------------------------------------------------

void ExportLy::writeScore()
      {
      int staffIdx = 0;
      int np = score->parts()->size();
      if (np > 1) {
            indent();
            os << "<<\n";
            ++level;
            }
      foreach(Part* part, *score->parts()) {
            int n = part->staves()->size();
            if (n > 1) {
                  indent();
                  os << "\\new PianoStaff <<\n";
                  ++level;
                  }
            foreach(Staff* staff, *part->staves()) {
                  indent();
                  os << "\\new Staff {\n";
                  ++level;
                  indent();
                  writeClef(staff->clef()->clef(0));
                  writeKeySig(staff->keymap()->key(0));
                  for(Measure* m = score->mainLayout()->first(); m; m = m->next()) {
                        os << "\n";
                        writeMeasure(m, staffIdx);
                        }
                  os << "\n";
                  indent();
                  os << "}\n";
                  --level;
                  ++staffIdx;
                  }
            if (n > 1) {
                  indent();
                  os << ">>\n";
                  --level;
                  }
            }
      if (np > 1) {
            indent();
            os << ">>\n";
            --level;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ExportLy::write(const QString& name)
      {
      f.setFileName(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;
      os.setDevice(&f);
      os << "%=============================================\n"
            "%   created by MuseScore Version: " << VERSION << "\n"
            "%=============================================\n"
            "\n"
            "\\version \"2.10.5\"\n";     // target lilypond version

      //---------------------------------------------------
      //    Page format
      //---------------------------------------------------

      PageFormat* pf = score->pageFormat();
      os << "#(set-default-paper-size ";
      switch(pf->size) {
            default:
            case  0: os << "\"a4\""; break;
            case  2: os << "\"letter\""; break;
            case  8: os << "\"a3\""; break;
            case  9: os << "\"a5\""; break;
            case 10: os << "\"a6\""; break;
            case 29: os << "\"11x17\""; break;
            }
      if (pf->landscape)
            os << " 'landscape";
      os << ")\n";

      double lw = pf->width() - pf->evenLeftMargin - pf->evenRightMargin;
      os << "\\paper {\n"
            "  line-width    = " << lw * INCH << "\\mm\n"
            "  left-margin   = " << pf->evenLeftMargin * INCH << "\\mm\n"
            "  top-margin    = " << pf->evenTopMargin * INCH << "\\mm\n"
            "  bottom-margin = " << pf->evenBottomMargin * INCH << "\\mm\n"
            "  }\n\n";

      //---------------------------------------------------
      //    score
      //---------------------------------------------------

      os << "\\book {\n";
      ++level;

      indent();
      os << "\\header {\n";
      ++level;
      const Measure* m = score->mainLayout()->first();
      foreach(const Element* e, *m->pel()) {
            if (e->type() != TEXT)
                  continue;
            QString s = ((Text*)e)->getText();
            indent();
            switch(e->subtype()) {
                  case TEXT_TITLE:
                        os << "title = ";
                        break;
                  case TEXT_SUBTITLE:
                        os << "subtitle = ";
                        break;
                  case TEXT_COMPOSER:
                        os << "composer = ";
                        break;
                  case TEXT_POET:
                        os << "poet = ";
                        break;
                  default:
                        printf("text-type %d not supported\n", e->subtype());
                        os << "subtitle = ";
                        break;
                  }
            os << "\"" << s << "\"\n";
            }
      indent();
      os << "}\n";
      --level;

      indent();
      os << "\\score {\n";
      ++level;

      writeScore();
      --level;
      indent();
      os << "}\n";
      --level;
      indent();
      os << "}\n";

      f.close();
      return f.error() == QFile::NoError;
      }

//---------------------------------------------------------
//   checkSlur
//---------------------------------------------------------

void ExportLy::checkSlur(int tick, int track)
      {
      for (const Measure* m = score->mainLayout()->first(); m; m = m->next()) {
            foreach(Element* e, *m->el()) {
                  if (e->type() != SLUR)
                        continue;
                  Slur* s = (Slur*)e;
                  if ((s->tick() == tick) && (s->track() == track)) {
                        os << "(";
printf("startSlur %d-%d  %d-%d\n", tick, track, s->tick2(), s->track2());
                        }
                  if ((s->tick2() == tick) && (s->track2() == track)) {
                        os << ")";
printf("endSlur %d-%d  %d-%d\n", s->tick(), s->track(), tick, track);
                        }
                  }
            }
      }

