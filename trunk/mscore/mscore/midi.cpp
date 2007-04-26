//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: midi.cpp,v 1.38 2006/03/22 12:04:14 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "mscore.h"
#include "midi.h"
#include "canvas.h"
#include "file.h"
#include "score.h"
#include "key.h"
#include "clef.h"
#include "sig.h"
#include "tempo.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "segment.h"
#include "utils.h"
#include "text.h"
#include "slur.h"
#include "staff.h"
#include "measure.h"
#include "style.h"
#include "part.h"
#include "layout.h"
#include "timesig.h"
#include "barline.h"
#include "pedal.h"
#include "ottava.h"
#include "lyrics.h"

static unsigned const char gmOnMsg[] = { 0x7e, 0x7f, 0x09, 0x01 };
static unsigned const char gsOnMsg[] = { 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41 };
static unsigned const char xgOnMsg[] = { 0x43, 0x10, 0x4c, 0x00, 0x00, 0x7e, 0x00 };
static unsigned const int  gmOnMsgLen = sizeof(gmOnMsg);
static unsigned const int  gsOnMsgLen = sizeof(gsOnMsg);
static unsigned const int  xgOnMsgLen = sizeof(xgOnMsg);

static MidiInstrument minstr[] = {
      // Piano
      { 7, 0, 0,  0, 60, "Grand Piano" },
      { 4, 0, 1,  0, 60, "GrndPnoK" },
      { 4, 0, 18, 0, 60, "MelloGrP" },
      { 4, 0, 40, 0, 60, "PianoStr" },
      { 4, 0, 41, 0, 60, "Dream" },
      { 7, 0, 0,  1, 60, "Bright Piano" },
      { 4, 0, 1,  1, 60, "BritPnoK" },
      { 7, 0, 0,  2, 60, "E.Grand" },
      { 4, 0, 1,  2, 60, "ElGrPnoK" },
      { 4, 0, 32, 2, 60, "Det.CP80" },
      { 4, 0, 40, 2, 60, "ElGrPno1" },
      { 4, 0, 41, 2, 60, "ElGrPno2" },
      { 7, 0, 0,  3, 60, "Honky-tonk" },
      { 4, 0, 1,  3, 60, "HonkyTonkK" },
      { 7, 0, 0,  4, 60, "E.Piano" },
      { 4, 0, 1,  4, 60, "El.Pno1K" },
      { 4, 0, 18, 4, 60, "MelloEP1" },
      { 4, 0, 32, 4, 60, "Chor.EP1" },
      { 4, 0, 40, 4, 60, "HardEl.P" },
      { 4, 0, 45, 4, 60, "VXElP1" },
      { 4, 0, 64, 4, 60, "60sEl.P" },
      { 7, 0, 0,  5, 60, "E.Piano 2" },
      { 4, 0, 1,  5, 60, "El.Pno2K" },
      { 4, 0, 32, 5, 60, "Chor.EP2" },
      { 4, 0, 33, 5, 60, "DX.Hard" },
      { 4, 0, 34, 5, 60, "DXLegend" },
      { 4, 0, 40, 5, 60, "DXPhase" },
      { 4, 0, 41, 5, 60, "DX+Analg" },
      { 4, 0, 42, 5, 60, "DXKotoEP" },
      { 4, 0, 45, 5, 60, "VXEl.P2" },
      { 7, 0, 0,  6, 60, "Harpsichord" },
      { 4, 0, 1,  6, 60, "Harpsi.K" },
      { 4, 0, 25, 6, 60, "Harpsi.2" },
      { 4, 0, 35, 6, 60, "Harpsi.3" },
      { 7, 0, 0,  7, 60, "Clav." },
      { 4, 0, 1,  7, 60, "Clavi.K" },
      { 4, 0, 27, 7, 60, "ClaviWah" },
      { 4, 0, 64, 7, 60, "PulseClv" },
      { 4, 0, 65, 7, 60, "PierceCl" },

// Chromatic Perc
      { 7, 0, 0, 8, 0, "Celesta" },
      { 7, 0, 0, 9, 0, "Glockenspiel" },
      { 7, 0, 0, 10, 0, "Music Box" },
      { 4, 0, 64, 10, 0, "Orgel" },
      { 7, 0, 0, 11, 0, "Vibraphone" },
      { 4, 0, 1, 11, 0, "VibesK" },
      { 4, 0, 45, 11, 0, "HardVibe" },
      { 7, 0, 0, 12, 0, "Marimba" },
      { 4, 0, 1, 12, 0, "MarimbaK" },
      { 4, 0, 64, 12, 0, "SineMrmb" },
      { 4, 0, 96, 12, 0, "Balafon" },
      { 4, 0, 97, 12, 0, "Balafon2" },
      { 4, 0, 98, 12, 0, "LogDrum" },
      { 7, 0, 0, 13, 0, "Xylophone" },
      { 7, 0, 0, 14, 0, "Tubular Bells" },
      { 4, 0, 96, 14, 0, "ChrchBel" },
      { 4, 0, 97, 14, 0, "Carillon" },
      { 7, 0, 0, 15, 0, "Dulcimer" },
      { 4, 0, 35, 15, 0, "Dulcimr2" },
      { 4, 0, 96, 15, 0, "Cimbalom" },
      { 4, 0, 97, 15, 0, "Santur" },
// Organ
      { 7, 0, 0, 16, 0, "Drawbar Organ" },
      { 4, 0, 32, 16, 0, "DelDrwOr" },
      { 4, 0, 33, 16, 0, "60sDrOr1" },
      { 4, 0, 34, 16, 0, "60sDrOr2" },
      { 4, 0, 35, 16, 0, "70sDrOr1" },
      { 4, 0, 36, 16, 0, "DrawOrg2" },
      { 4, 0, 37, 16, 0, "60sDrOr3" },
      { 4, 0, 38, 16, 0, "EvenBar" },
      { 4, 0, 40, 16, 0, "16+2\"2/3" },
      { 4, 0, 64, 16, 0, "OrganBa" },
      { 4, 0, 65, 16, 0, "70sDrOr2" },
      { 4, 0, 66, 16, 0, "CheezOrg" },
      { 4, 0, 67, 16, 0, "DrawOrg3" },
      { 7, 0, 0, 17, 0, "Perc. Organ" },
      { 4, 0, 24, 17, 0, "70sPcOr1" },
      { 4, 0, 32, 17, 0, "DetPrcOr" },
      { 4, 0, 33, 17, 0, "LiteOrg" },
      { 4, 0, 37, 17, 0, "PercOrg2" },
      { 7, 0, 0, 18, 0, "Rock Organ" },
      { 4, 0, 64, 18, 0, "RotaryOr" },
      { 4, 0, 65, 18, 0, "SloRotar" },
      { 4, 0, 66, 18, 0, "FstRotar" },
      { 7, 0, 0, 19, 0, "Church Organ" },
      { 4, 0, 32, 19, 0, "ChurOrg3" },
      { 4, 0, 35, 19, 0, "ChurOrg2" },
      { 4, 0, 40, 19, 0, "NotreDam" },
      { 4, 0, 64, 19, 0, "OrgFlute" },
      { 4, 0, 65, 19, 0, "TrmOrgFl" },
      { 7, 0, 0, 20, 0, "Reed Organ" },
      { 4, 0, 40, 20, 0, "PuffOrg" },
      { 7, 0, 0, 21, 0, "Akkordion" },
      { 4, 0, 32, 21, 0, "Accordlt" },
      { 7, 0, 0, 22, 0, "Harmonica" },
      { 4, 0, 32, 22, 0, "Harmo2" },
      { 7, 0, 0, 23, 0, "Bandoneon" },
      { 4, 0, 64, 23, 0, "TngoAcd2" },
// Guitar
      { 7, 0, 0, 24, 0, "Nylon Gtr." },
      { 4, 0, 16, 24, 0, "NylonGt2" },
      { 4, 0, 25, 24, 0, "NylonGt3" },
      { 4, 0, 43, 24, 0, "VelGtHrm" },
      { 4, 0, 96, 24, 0, "Ukelele" },
      { 7, 0, 0, 25, 0, "Steel Gtr." },
      { 4, 0, 16, 25, 0, "SteelGt2" },
      { 4, 0, 35, 25, 0, "12StrGtr" },
      { 4, 0, 40, 25, 0, "Nylon-Stl" },
      { 4, 0, 41, 25, 0, "Stl-Body" },
      { 4, 0, 96, 25, 0, "Mandolin" },
      { 7, 0, 0, 26, 0, "Jazz Guitar" },
      { 4, 0, 18, 26, 0, "MelloGtr" },
      { 4, 0, 32, 26, 0, "JazzAmp" },
      { 4, 0, 96, 26, 0, "PdlSteel" },
      { 7, 0, 0, 27, 0, "Clean Guitar" },
      { 4, 0, 32, 27, 0, "ChorusGt" },
      { 4, 0, 64, 27, 0, "CleanGt2" },
      { 7, 0, 0, 28, 0, "Muted Guitar" },
      { 4, 0, 40, 28, 0, "FunkGtr1" },
      { 4, 0, 41, 28, 0, "MuteStlG" },
      { 4, 0, 43, 28, 0, "FunkGtr2" },
      { 4, 0, 45, 28, 0, "JazzMan" },
      { 4, 0, 96, 28, 0, "Mu.DstGt" },
      { 7, 0, 0, 29, 0, "Overdrive Gtr" },
      { 4, 0, 43, 29, 0, "Gt.Pinch" },
      { 7, 0, 0, 30, 0, "DistortionGtr" },
      { 4, 0, 12, 30, 0, "DstRthmG" },
      { 4, 0, 24, 30, 0, "DistGtr2" },
      { 4, 0, 35, 30, 0, "DistGtr3" },
      { 4, 0, 36, 30, 0, "PowerGt2" },
      { 4, 0, 37, 30, 0, "PowerGt1" },
      { 4, 0, 38, 30, 0, "Dst.5ths" },
      { 4, 0, 40, 30, 0, "FeedbkGt" },
      { 4, 0, 41, 30, 0, "FeedbGt2" },
      { 4, 0, 43, 30, 0, "RkRythm2" },
      { 4, 0, 45, 30, 0, "RockRthm" },
      { 7, 0, 0, 31, 0, "Gtr.Harmonics" },
      { 4, 0, 65, 31, 0, "GtFeedbk" },
      { 4, 0, 66, 31, 0, "GtrHrmo2" },
      { 4, 0, 64, 31, 0, "AcoHarmo" },
// Bass
      { 7, 0, 0, 32, 0, "Acoustic Bass" },
      { 4, 0, 40, 32, 0, "JazzRthm" },
      { 4, 0, 45, 32, 0, "VXUprght" },
      { 7, 0, 0, 33, 0, "Fingered Bass" },
      { 4, 0, 18, 33, 0, "FingrDrk" },
      { 4, 0, 27, 33, 0, "FlangeBa" },
      { 4, 0, 40, 33, 0, "Ba-DstEG" },
      { 4, 0, 43, 33, 0, "FngrSlap" },
      { 4, 0, 45, 33, 0, "FngBass2" },
      { 4, 0, 64, 33, 0, "JazzBass" },
      { 4, 0, 65, 33, 0, "ModAlem" },
      { 7, 0, 0, 34, 0, "Picked Bass" },
      { 4, 0, 28, 34, 0, "MutePkBa" },
      { 7, 0, 0, 35, 0, "Fretless Bass" },
      { 4, 0, 32, 35, 0, "Fretles2" },
      { 4, 0, 33, 35, 0, "Fretles3" },
      { 4, 0, 34, 35, 0, "Fretles4" },
      { 4, 0, 96, 35, 0, "SynFretl" },
      { 4, 0, 97, 35, 0, "Smooth" },
      { 7, 0, 0, 36, 0, "Slap Bass 1" },
      { 4, 0, 27, 36, 0, "ResoSlap" },
      { 4, 0, 32, 36, 0, "PunchThm" },
      { 7, 0, 0, 37, 0, "Slap Bass 2" },
      { 4, 0, 43, 37, 0, "VeloSlap" },
      { 7, 0, 0, 38, 0, "Synth Bass 1" },
      { 4, 0, 18, 38, 0, "SynBa1Dk" },
      { 4, 0, 20, 38, 0, "FastResB" },
      { 4, 0, 24, 38, 0, "AcidBass" },
      { 4, 0, 35, 38, 0, "ClvBass" },
      { 4, 0, 40, 38, 0, "TeknoBa" },
      { 4, 0, 64, 38, 0, "Oscar" },
      { 4, 0, 65, 38, 0, "SqrBass" },
      { 4, 0, 66, 38, 0, "RubberBa" },
      { 4, 0, 96, 38, 0, "Hammer" },
      { 7, 0, 0, 39, 0, "Synth Bass 2" },
      { 4, 0, 6, 39, 0, "MelloSB1" },
      { 4, 0, 12, 39, 0, "SeqBass" },
      { 4, 0, 18, 39, 0, "ClkSynBa" },
      { 4, 0, 19, 39, 0, "SynBa2Dk" },
      { 4, 0, 32, 39, 0, "SmthBa2" },
      { 4, 0, 40, 39, 0, "ModulrBa" },
      { 4, 0, 41, 39, 0, "DXBass" },
      { 4, 0, 64, 39, 0, "XWireBa" },
// Strings/Orch
      { 7, 0, 0, 40, 0, "Violin" },
      { 4, 0, 8, 40, 0, "SlowVln" },
      { 7, 0, 0, 41, 0, "Viola" },
      { 7, 0, 0, 42, 0, "Cello" },
      { 7, 0, 0, 43, 0, "Contrabass" },
      { 7, 0, 0, 44, 0, "Tremolo Str." },
      { 4, 0, 8, 44, 0, "SlowTrSt" },
      { 4, 0, 40, 44, 0, "SuspStr" },
      { 7, 0, 0, 45, 0, "PizzicatoStr." },
      { 7, 0, 0, 46, 0, "Harp" },
      { 4, 0, 40, 46, 0, "YangChin" },
      { 7, 0, 0, 47, 0, "Timpani" },
// Ensemble
      { 7, 0, 0, 48, 0, "Strings 1" },
      { 4, 0, 3, 48, 0, "S.Strngs" },
      { 4, 0, 8, 48, 0, "SlowStr" },
      { 4, 0, 24, 48, 0, "ArcoStr" },
      { 4, 0, 35, 48, 0, "60sStrng" },
      { 4, 0, 40, 48, 0, "Orchestr" },
      { 4, 0, 41, 48, 0, "Orchstr2" },
      { 4, 0, 42, 48, 0, "TremOrch" },
      { 4, 0, 45, 48, 0, "VeloStr" },
      { 7, 0, 0, 49, 0, "Strings 2" },
      { 4, 0, 3, 49, 0, "S.SlwStr" },
      { 4, 0, 8, 49, 0, "LegatoSt" },
      { 4, 0, 40, 49, 0, "WarmStr" },
      { 4, 0, 41, 49, 0, "Kingdom" },
      { 4, 0, 64, 49, 0, "70sStr" },
      { 4, 0, 65, 49, 0, "StrEns3" },
      { 7, 0, 0, 50, 0, "Syn.Strings 1" },
      { 4, 0, 27, 50, 0, "ResoStr" },
      { 4, 0, 64, 50, 0, "SynStr4" },
      { 4, 0, 65, 50, 0, "SSStr" },
      { 4, 0, 35, 50, 0, "SynStr3" },
      { 7, 0, 0, 51, 0, "Syn.Strings 2" },
      { 7, 0, 0, 52, 0, "Choir Aahs" },
      { 4, 0, 3, 52, 0, "S.Choir" },
      { 4, 0, 16, 52, 0, "Ch.Aahs2" },
      { 4, 0, 32, 52, 0, "MelChoir" },
      { 4, 0, 40, 52, 0, "ChoirStr" },
      { 4, 0, 64, 52, 0, "StrngAah" },
      { 4, 0, 65, 52, 0, "MaleAah" },
      { 7, 0, 0, 53, 0, "Voice Oohs" },
      { 4, 0, 64, 53, 0, "VoiceDoo" },
      { 4, 0, 96, 53, 0, "VoiceHmn" },
      { 7, 0, 0, 54, 0, "Synth Voice" },
      { 4, 0, 40, 54, 0, "SynVox2" },
      { 4, 0, 41, 54, 0, "Choral" },
      { 4, 0, 64, 54, 0, "AnaVoice" },
      { 7, 0, 0, 55, 0, "Orchestra Hit" },
      { 4, 0, 35, 55, 0, "OrchHit2" },
      { 4, 0, 64, 55, 0, "Impact" },
      { 4, 0, 66, 55, 0, "DoublHit" },
      { 4, 0, 67, 55, 0, "BrStab80" },
// Brass
      { 7, 0, 0, 56, 0, "Trumpet" },
      { 4, 0, 16, 56, 0, "Trumpet2" },
      { 4, 0, 17, 56, 0, "BriteTrp" },
      { 4, 0, 32, 56, 0, "WarmTrp" },
      { 4, 0, 96, 56, 0, "FluglHrn" },
      { 7, 0, 0, 57, 0, "Trombone" },
      { 4, 0, 18, 57, 0, "Trmbone2" },
      { 7, 0, 0, 58, 0, "Tuba" },
      { 4, 0, 16, 58, 0, "Tuba2" },
      { 7, 0, 0, 59, 0, "Muted Trumpet" },
      { 4, 2, 64, 59, 0, "MuteTrp2" },
      { 7, 0, 0, 60, 0, "French Horn" },
      { 4, 0, 6, 60, 0, "FrHrSolo" },
      { 4, 0, 32, 60, 0, "FrHorn2" },
      { 4, 0, 37, 60, 0, "HornOrch" },
      { 7, 0, 0, 61, 0, "Brass Section" },
      { 4, 0, 35, 61, 0, "Tp-TbSec" },
      { 4, 0, 40, 61, 0, "BrssSec2" },
      { 4, 0, 41, 61, 0, "HiBrass" },
      { 4, 0, 42, 61, 0, "MelloBrs" },
      { 4, 0, 14, 61, 0, "SfrzndBr" },
      { 4, 0, 39, 61, 0, "BrssFall" },
      { 7, 0, 0, 62, 0, "Synth Brass 1" },
      { 4, 0, 12, 62, 0, "QuackBr" },
      { 4, 0, 20, 62, 0, "RezSynBr" },
      { 4, 0, 24, 62, 0, "PolyBrss" },
      { 4, 0, 27, 62, 0, "SynBras3" },
      { 4, 0, 32, 62, 0, "JumpBrss" },
      { 4, 0, 45, 62, 0, "AnaVelBr" },
      { 4, 0, 64, 62, 0, "AnaBrss1" },
      { 7, 0, 0, 63, 0, "Synth Brass 2" },
      { 4, 0, 18, 63, 0, "SoftBrs" },
      { 4, 0, 40, 63, 0, "SynBras4" },
      { 4, 0, 41, 63, 0, "ChoBrss" },
      { 4, 0, 45, 63, 0, "VelBras2" },
      { 4, 0, 64, 63, 0, "AnaBras2" },
// Reed
      { 7, 0, 0, 64, 0, "Soprano Sax" },
      { 7, 0, 0, 65, 0, "Alto Sax" },
      { 4, 0, 40, 65, 0, "SaxSect" },
      { 4, 0, 43, 65, 0, "HyprAlto" },
      { 7, 0, 0, 66, 0, "Tenor Sax" },
      { 4, 0, 40, 66, 0, "BrthTnSx" },
      { 4, 0, 41, 66, 0, "SoftTenr" },
      { 4, 0, 64, 66, 0, "TnrSax2" },
      { 7, 0, 0, 67, 0, "Baritone Sax" },
      { 7, 0, 0, 68, 0, "Oboe" },
      { 7, 0, 0, 69, 0, "English Horn" },
      { 7, 0, 0, 70, 0, "Bassoon" },
      { 7, 0, 0, 71, 0, "Clarinet" },
      { 4, 0, 96, 71, 0, "BassClar" },
// Pipe
      { 7, 0, 0, 72, 0, "Piccolo" },
      { 7, 0, 0, 73, 0, "Flute" },
      { 7, 0, 0, 74, 0, "Recorder" },
      { 7, 0, 0, 75, 0, "Pan Flute" },
      { 4, 0, 64, 75, 0, "PanFlut2" },
      { 4, 0, 96, 75, 0, "Kawala" },
      { 7, 0, 0, 76, 0, "Blown Bottle" },
      { 7, 0, 0, 77, 0, "Shakuhachi" },
      { 7, 0, 0, 78, 0, "Whistle" },
      { 7, 0, 0, 79, 0, "Ocarina" },
// SynthLead
      { 7, 0, 0, 80, 0, "Square Wave" },
      { 4, 0, 6, 80, 0, "Square2" },
      { 4, 0, 8, 80, 0, "LMSquare" },
      { 4, 0, 18, 80, 0, "Hollow" },
      { 4, 0, 19, 80, 0, "Shmoog" },
      { 4, 0, 64, 80, 0, "Mellow" },
      { 4, 0, 65, 80, 0, "SoloSine" },
      { 4, 0, 66, 80, 0, "SineLead" },
      { 7, 0, 0, 81, 0, "Saw Wave" },
      { 4, 0, 6, 81, 0, "Saw2" },
      { 4, 0, 8, 81, 0, "ThickSaw" },
      { 4, 0, 18, 81, 0, "DynaSaw" },
      { 4, 0, 19, 81, 0, "DigiSaw" },
      { 4, 0, 20, 81, 0, "BigLead" },
      { 4, 0, 24, 81, 0, "HeavySyn" },
      { 4, 0, 25, 81, 0, "WaspySyn" },
      { 4, 0, 40, 81, 0, "PulseSaw" },
      { 4, 0, 41, 81, 0, "Dr.Lead" },
      { 4, 0, 45, 81, 0, "VeloLead" },
      { 4, 0, 96, 81, 0, "SeqAna" },
      { 7, 0, 0, 82, 0, "Calliope" },
      { 4, 0, 65, 82, 0, "PurePad" },
      { 4, 0, 64, 82, 0, "VentSyn" },
      { 7, 0, 0, 83, 0, "Chiffer Lead" },
      { 4, 0, 64, 83, 0, "Rubby" },
      { 7, 0, 0, 84, 0, "Charang" },
      { 4, 0, 64, 84, 0, "DistLead" },
      { 4, 0, 65, 84, 0, "WireLead" },
      { 7, 0, 0, 85, 0, "Solo Vox" },
      { 4, 0, 24, 85, 0, "SynthAah" },
      { 4, 0, 64, 85, 0, "VoxLead" },
      { 7, 0, 0, 86, 0, "Fifth Saw" },
      { 4, 0, 35, 86, 0, "BigFive" },
      { 7, 0, 0, 87, 0, "Bass Lead" },
      { 4, 0, 16, 87, 0, "Big-Low" },
      { 4, 0, 64, 87, 0, "Fat-Prky" },
      { 4, 0, 65, 87, 0, "SoftWurl" },
// Synth Pad
      { 7, 0, 0, 88, 0, "New Age Pad" },
      { 4, 0, 64, 88, 0, "Fantasy2" },
      { 7, 0, 0, 89, 0, "Warm Pad" },
      { 4, 0, 16, 89, 0, "ThickPad" },
      { 4, 0, 17, 89, 0, "SoftPad" },
      { 4, 0, 18, 89, 0, "SinePad" },
      { 4, 0, 64, 89, 0, "HornPad" },
      { 4, 0, 65, 89, 0, "RotarStr" },
      { 7, 0, 0, 90, 0, "Polysynth Pad" },
      { 4, 0, 64, 90, 0, "PolyPd80" },
      { 4, 0, 65, 90, 0, "ClickPad" },
      { 4, 0, 66, 90, 0, "AnaPad" },
      { 4, 0, 67, 90, 0, "SquarPad" },
      { 7, 0, 0, 91, 0, "Choir Pad" },
      { 4, 0, 64, 91, 0, "Heaven2" },
      { 4, 0, 66, 91, 0, "Itopia" },
      { 4, 0, 67, 91, 0, "CCPad" },
      { 4, 0, 65, 91, 0, "LitePad" },
      { 7, 0, 0, 92, 0, "Bowed Pad" },
      { 4, 0, 64, 92, 0, "Glacier" },
      { 4, 0, 65, 92, 0, "GlassPad" },
      { 7, 0, 0, 93, 0, "Metallic Pad" },
      { 4, 0, 64, 93, 0, "TinePad" },
      { 4, 0, 65, 93, 0, "PanPad" },
      { 7, 0, 0, 94, 0, "Halo Pad" },
      { 7, 0, 0, 95, 0, "Sweep Pad" },
      { 4, 0, 20, 95, 0, "Shwimmer" },
      { 4, 0, 27, 95, 0, "Converge" },
      { 4, 0, 64, 95, 0, "PolarPad" },
      { 4, 0, 66, 95, 0, "Celstial" },
      { 4, 0, 65, 95, 0, "Sweepy" },
// Synth FX
      { 7, 0, 0, 96, 0, "Rain" },
      { 4, 0, 45, 96, 0, "ClaviPad" },
      { 4, 0, 64, 96, 0, "HrmoRain" },
      { 4, 0, 65, 96, 0, "AfrcnWnd" },
      { 4, 0, 66, 96, 0, "Caribean" },
      { 7, 0, 0, 97, 0, "Soundtrack" },
      { 4, 0, 27, 97, 0, "Prologue" },
      { 4, 0, 64, 97, 0, "Ancestrl" },
      { 4, 0, 65, 97, 0, "Rave" },
      { 7, 0, 0, 98, 0, "Crystal" },
      { 4, 0, 12, 98, 0, "SynDrCmp" },
      { 4, 0, 14, 98, 0, "Popcorn" },
      { 4, 0, 18, 98, 0, "TinyBell" },
      { 4, 0, 35, 98, 0, "RndGlock" },
      { 4, 0, 40, 98, 0, "GlockChi" },
      { 4, 0, 41, 98, 0, "ClearBel" },
      { 4, 0, 42, 98, 0, "ChorBell" },
      { 4, 0, 64, 98, 0, "SynMalet" },
      { 4, 0, 65, 98, 0, "SftCryst" },
      { 4, 0, 66, 98, 0, "LoudGlok" },
      { 4, 0, 67, 98, 0, "XmasBell" },
      { 4, 0, 68, 98, 0, "VibeBell" },
      { 4, 0, 69, 98, 0, "DigiBell" },
      { 4, 0, 70, 98, 0, "AirBells" },
      { 4, 0, 71, 98, 0, "BellHarp" },
      { 4, 0, 72, 98, 0, "Gamelmba" },
      { 7, 0, 0, 99, 0, "Athmosphere" },
      { 4, 0, 18, 99, 0, "WarmAtms" },
      { 4, 0, 19, 99, 0, "HollwRls" },
      { 4, 0, 40, 99, 0, "NylonEP" },
      { 4, 0, 64, 99, 0, "NylnHarp" },
      { 4, 0, 65, 99, 0, "HarpVox" },
      { 7, 0, 0, 100, 0, "Brightness" },
      { 7, 0, 0, 101, 0, "Goblins" },
      { 4, 0, 69, 101, 0, "MilkyWay" },
      { 4, 0, 72, 101, 0, "Puffy" },
      { 7, 0, 0, 102, 0, "Echoes" },
      { 7, 0, 0, 103, 0, "Sci-Fi" },
      { 4, 0, 65, 103, 0, "Odyssey" },
// Ethnic
      { 7, 0, 0, 104, 0, "Sitar" },
      { 4, 0, 32, 104, 0, "DetSitar" },
      { 4, 0, 35, 104, 0, "Sitar2" },
      { 4, 0, 96, 104, 0, "Tambra" },
      { 4, 0, 97, 104, 0, "Tamboura" },
      { 7, 0, 0, 105, 0, "Banjo" },
      { 4, 0, 28, 105, 0, "MuteBnjo" },
      { 4, 0, 96, 105, 0, "Rabab" },
      { 4, 0, 97, 105, 0, "Gopichnt" },
      { 4, 0, 98, 105, 0, "Oud" },
      { 7, 0, 0, 106, 0, "Shamisen" },
      { 4, 0, 96, 106, 0, "Tsugaru" },
      { 7, 0, 0, 107, 0, "Koto" },
      { 4, 0, 96, 107, 0, "T.Koto" },
      { 4, 0, 97, 107, 0, "Kanoon" },
      { 7, 0, 0, 108, 0, "Kalimba" },
      { 4, 0, 64, 108, 0, "BigKalim" },
      { 7, 0, 0, 109, 0, "Bagpipe" },
      { 7, 0, 0, 110, 0, "Fiddle" },
      { 7, 0, 0, 111, 0, "Shanai" },
      { 4, 0, 64, 111, 0, "Shanai2" },
      { 4, 0, 96, 111, 0, "Pungi" },
      { 4, 0, 97, 111, 0, "Hichriki" },
// Percussive
      { 7, 0, 0, 112, 0, "Tinkle Bell" },
      { 4, 0, 96, 112, 0, "Bonang" },
      { 4, 0, 97, 112, 0, "Gender" },
      { 4, 0, 98, 112, 0, "Gamelan" },
      { 4, 0, 99, 112, 0, "S.Gamlan" },
      { 4, 0, 100, 112, 0, "RamaCym" },
      { 4, 0, 101, 112, 0, "AsianBel" },
      { 7, 0, 0, 113, 0, "Agogo" },
      { 4, 0, 96, 113, 0, "Atrigane" },
      { 7, 0, 0, 114, 0, "Steel Drums" },
      { 4, 0, 97, 114, 0, "GlasPerc" },
      { 4, 0, 98, 114, 0, "ThaiBell" },
      { 4, 0, 96, 114, 0, "Tablas" },
      { 7, 0, 0, 115, 0, "Woodblock" },
      { 4, 0, 96, 115, 0, "Castanet" },
      { 7, 0, 0, 116, 0, "Taiko Drum" },
      { 4, 0, 96, 116, 0, "Gr.Cassa" },
      { 7, 0, 0, 117, 0, "Melodic Drum" },
      { 4, 0, 64, 117, 0, "MelTom2" },
      { 4, 0, 65, 117, 0, "RealTom" },
      { 4, 0, 66, 117, 0, "RockTom" },
      { 7, 0, 0, 118, 0, "Synth Drum" },
      { 4, 0, 64, 118, 0, "AnaTom" },
      { 4, 0, 65, 118, 0, "ElecPerc" },
      { 7, 0, 0, 119, 0, "Rev. Cymbal" },
      { 4, 0, 64, 119, 0, "RevCym2" },
      { 4, 0, 96, 119, 0, "RevSnar1" },
      { 4, 0, 97, 119, 0, "RevSnar2" },
      { 4, 0, 98, 119, 0, "RevKick1" },
      { 4, 0, 99, 119, 0, "RevConBD" },
      { 4, 0, 100, 119, 0, "RevTom1" },
      { 4, 0, 101, 119, 0, "RevTom2" },
// Special FX
      { 7,  0, 0, 120, 0, "GtrFret Noise" },
      { 7,  0, 0, 121, 0, "Breath Noise" },
      { 7,  0, 0, 122, 0, "Seashore" },
      { 7,  0, 0, 123, 0, "Bird Tweed" },
      { 7,  0, 0, 124, 0, "Telephone" },
      { 7,  0, 0, 125, 0, "Helicopter" },
      { 7,  0, 0, 126, 0, "Applaus" },
      { 7,  0, 0, 127, 0, "Gunshot" },
// Drums
      { 6, 17, 0,   0, 0, "Standard" },
      { 4, 17, 0,   1, 0, "Standrd2" },
      { 6, 17, 0,   8, 0, "Room" },
      { 4, 17, 0,  16, 0, "Rock" },
      { 6, 17, 0,  24, 0, "Electro" },
      { 6, 17, 0,  25, 0, "Analog" },
      { 6, 17, 0,  32, 0, "Jazz" },
      { 6, 17, 0,  40, 0, "Brush" },
      { 6, 17, 0,  48, 0, "Classic" },
      { 2, 17, 0,  16, 0, "Power" },
      { 2, 17, 0,  56, 0, "SFX1" },
      { 2, 17, 0, 127, 0, "GM" },
      { 4, 16, 0,   0, 0, "SFX1" },
      { 4, 16, 0,   1, 0, "SFX2" },
      { 4,  4, 0,   0, 0, "CuttngNz" },
      { 4,  4, 0,   1, 0, "CuttngNz2" },
      { 4,  4, 0,   3, 0, "StrSlap" },
      { 4,  4, 0,  16, 0, "Fl.KClik" },
      { 4,  4, 0,  32, 0, "Rain" },
      { 4,  4, 0,  33, 0, "Thunder" },
      { 4,  4, 0,  34, 0, "Wind" },
      { 4,  4, 0,  35, 0, "Stream" },
      { 4,  4, 0,  36, 0, "Bubble" },
      { 4,  4, 0,  37, 0, "Feed" },
      { 4,  4, 0,  48, 0, "Dog" },
      { 4,  4, 0,  49, 0, "Horse" },
      { 4,  4, 0,  50, 0, "Bird2" },
      { 4,  4, 0,  54, 0, "Ghost" },
      { 4,  4, 0,  55, 0, "Maou" },
      { 4,  4, 0,  64, 0, "Tel.Dial" },
      { 4,  4, 0,  65, 0, "DoorSqek" },
      { 4,  4, 0,  66, 0, "DoorSlam" },
      { 4,  4, 0,  67, 0, "Scratch" },
      { 4,  4, 0,  68, 0, "Scratch2" },
      { 4,  4, 0,  69, 0, "WindChm" },
      { 4,  4, 0,  70, 0, "Telphon2" },
      { 4,  4, 0,  80, 0, "CarEngin" },
      { 4,  4, 0,  81, 0, "CarStop" },
      { 4,  4, 0,  82, 0, "CarPass" },
      { 4,  4, 0,  83, 0, "CarCrash" },
      { 4,  4, 0,  84, 0, "Siren" },
      { 4,  4, 0,  85, 0, "Train" },
      { 4,  4, 0,  86, 0, "Jetplane" },
      { 4,  4, 0,  87, 0, "Starship" },
      { 4,  4, 0,  88, 0, "Burst" },
      { 4,  4, 0,  89, 0, "Coaster" },
      { 4,  4, 0,  90, 0, "SbMarine" },
      { 4,  4, 0,  96, 0, "Laughing" },
      { 4,  4, 0,  97, 0, "Scream" },
      { 4,  4, 0,  98, 0, "Punch" },
      { 4,  4, 0,  99, 0, "Heart" },
      { 4,  4, 0, 100, 0, "FootStep" },
      { 4,  4, 0, 112, 0, "MchinGun" },
      { 4,  4, 0, 113, 0, "LaserGun" },
      { 4,  4, 0, 114, 0, "Xplosion" },
      { 4,  4, 0, 115, 0, "FireWork" },
      { 4,  4, 0,   2, 0, "DstCutNz" },
      { 4,  4, 0,   4, 0, "B.Slide" },
      { 4,  4, 0,   5, 0, "P.Scrape" },
      { 4,  4, 0,  51, 0, "Kitty" },
      { 4,  4, 0,  52, 0, "Growl" },
      { 4,  4, 0,  53, 0, "Haunted" },
      { 4,  4, 0, 101, 0, "Applaus2" },
      };

//---------------------------------------------------------
//   MidiEvent
//---------------------------------------------------------

MidiEvent::MidiEvent()
      {
      data    = 0;
      dataLen = 0;
      len     = 0;
      tick    = 0;
      channel = -1;
      port    = -1;
      }

MidiEvent::~MidiEvent()
      {
      if (data)
            delete data;
      }

//---------------------------------------------------------
//   exportMidi
//---------------------------------------------------------

class ExportMidi : public SaveFile {
      Score* cs;

      void writeHeader();

   public:
      MidiFile mf;

      ExportMidi(Score* s) : mf(s) { cs = s; }
      virtual bool saver();
      };

//---------------------------------------------------------
//   exportMidi
//---------------------------------------------------------

void MuseScore::exportMidi()
      {
      ExportMidi em(cs);
      em.save(this, QString("."), QString(".mid"), tr("MuseScore: Export as Midi (SMF)"));
      }

//---------------------------------------------------------
//   writeHeader
//---------------------------------------------------------

void ExportMidi::writeHeader()
      {
      MidiTrack* track  = mf.tracks()->front();
      EventList* events = track->events();
      Measure* measure  = cs->mainLayout()->first();
      foreach(const Element* e, *measure->pel()) {
            if (e->type() == TEXT) {
                  Text* text = (Text*)(e);
                  QString str = text->getText();
                  int len     = str.length() + 1;
                  switch (text->subtype()) {
                        case TEXT_TITLE:
                              {
                              MidiEvent* ev = new MidiEvent;
                              ev->type      = ME_META;
                              ev->dataA     = META_TITLE;
                              ev->len       = len;
                              ev->data      = new unsigned char[len];
                              strcpy((char*)(ev->data), str.toLatin1().data());
                              events->insert(0, ev);
                              }
                              break;
                        case TEXT_SUBTITLE:
                              {
                              MidiEvent* ev = new MidiEvent;
                              ev->type      = ME_META;
                              ev->dataA     = META_SUBTITLE;
                              ev->len       = len;
                              ev->data      = new unsigned char[len];
                              strcpy((char*)(ev->data), str.toLatin1().data());
                              events->insert(0, ev);
                              }
                              break;
                        case TEXT_COMPOSER:
                              {
                              MidiEvent* ev = new MidiEvent;
                              ev->type      = ME_META;
                              ev->dataA     = META_COMPOSER;
                              ev->len       = len;
                              ev->data      = new unsigned char[len];
                              strcpy((char*)(ev->data), str.toLatin1().data());
                              events->insert(0, ev);
                              }
                              break;
                        case TEXT_TRANSLATOR:
                              {
                              MidiEvent* ev = new MidiEvent;
                              ev->type      = ME_META;
                              ev->dataA     = META_TRANSLATOR;
                              ev->len       = len;
                              ev->data      = new unsigned char[len];
                              strcpy((char*)(ev->data), str.toLatin1().data());
                              events->insert(0, ev);
                              }
                              break;
                        case TEXT_POET:
                              {
                              MidiEvent* ev = new MidiEvent;
                              ev->type      = ME_META;
                              ev->dataA     = META_POET;
                              ev->len       = len;
                              ev->data      = new unsigned char[len];
                              strcpy((char*)(ev->data), str.toLatin1().data());
                              events->insert(0, ev);
                              }
                              break;
                        }
                  }
            }

      //--------------------------------------------
      //    write time signature
      //--------------------------------------------

      SigList* sigmap = cs->sigmap;
      for (iSigEvent is = sigmap->begin(); is != sigmap->end(); ++is) {
            SigEvent se = is->second;
            int tick = is->first;
            MidiEvent* ev = new MidiEvent;
            ev->type      = ME_META;
            ev->dataA     = META_TIME_SIGNATURE;
            ev->len  = 4;
            ev->data = new unsigned char[4];
            ev->data[0] = se.nominator;
            int n;
            switch(se.denominator) {
                  case 1: n = 0; break;
                  case 2: n = 1; break;
                  case 4: n = 2; break;
                  case 8: n = 3; break;
                  case 16: n = 4; break;
                  case 32: n = 5; break;
                  default:
                        n = 2;
                        printf("ExportMidi: unknown time signature %d/%d\n", se.nominator, n);
                        break;
                  }
            ev->data[1] = n;
            ev->data[2] = 24;
            ev->data[3] = 8;
            events->insert(tick, ev);
            }

      //--------------------------------------------
      //    write key signatures
      //--------------------------------------------

      KeyList* keymap = cs->staff(0)->keymap(); // TODO_K
      for (iKeyEvent ik = keymap->begin(); ik != keymap->end(); ++ik) {
            int tick      = ik->first;
            int key       = ik->second;
            MidiEvent* ev = new MidiEvent;
            ev->type      = ME_META;
            ev->dataA     = META_KEY_SIGNATURE;
            ev->len       = 2;
            ev->data      = new unsigned char[2];
            ev->data[0]   = key;
            ev->data[1]   = 0;  // major
            events->insert(tick, ev);
            }

      //--------------------------------------------
      //    write tempo changes
      //--------------------------------------------

      TempoList* tempomap = cs->tempomap;
      for (iTEvent it = tempomap->begin(); it != tempomap->end(); ++it) {
            int tick      = it->second->tick;
            int tempo     = it->second->tempo;
            MidiEvent* ev = new MidiEvent;
            ev->type      = ME_META;
            ev->dataA     = META_TEMPO;
            ev->len       = 3;
            ev->data      = new unsigned char[3];
            ev->data[0]   = tempo >> 16;
            ev->data[1]   = tempo >> 8;
            ev->data[2]   = tempo;
            events->insert(tick, ev);
            }
      }

//---------------------------------------------------------
//  saver
//    export midi file
//    return true on error
//---------------------------------------------------------

bool ExportMidi::saver()
      {
      mf.setFp(&f);

      MidiTrackList* tracks = mf.tracks();
      int nparts = cs->parts()->size();
      for (int i = 0; i < nparts; ++i)
            tracks->append(new MidiTrack);

      int gateTime = 80;  // 100 - legato (100%)
      writeHeader();
      int staffIdx = 0;
      int partIdx = 0;
      foreach (Part* part, *cs->parts()) {
            MidiTrack* track = tracks->at(partIdx);
            EventList* events = track->events();
            int channel  = part->midiChannel();
            track->setOutPort(0);
            track->setOutChannel(channel);

            MidiEvent* ev = new MidiEvent;
            ev->type  = ME_PROGRAM;
            ev->dataA = part->midiProgram();
            events->insert(0, ev);

            ev        = new MidiEvent;
            ev->type  = ME_CONTROLLER;
            ev->dataA = CTRL_VOLUME;
            ev->dataB = part->volume();
            events->insert(1, ev);

            ev        = new MidiEvent;
            ev->type  = ME_CONTROLLER;
            ev->dataA = CTRL_PANPOT;
            ev->dataB = part->pan();
            events->insert(2, ev);

            ev        = new MidiEvent;
            ev->type  = ME_CONTROLLER;
            ev->dataA = CTRL_REVERB_SEND;
            ev->dataA = part->reverb();
            events->insert(3, ev);

            ev        = new MidiEvent;
            ev->type  = ME_CONTROLLER;
            ev->dataA = CTRL_CHORUS_SEND;
            ev->dataA = part->chorus();
            events->insert(4, ev);

            for (int i = 0; i < part->staves()->size(); ++i) {
                  QList<OttavaE> ol;
                  for (Measure* m = cs->mainLayout()->first(); m; m = m->next()) {
                        foreach(Element* e, *m->el()) {
                              if (e->type() == OTTAVA) {
                                    Ottava* ottava = (Ottava*)e;
                                    OttavaE oe;
                                    oe.offset = ottava->pitchShift();
                                    oe.start  = ottava->tick();
                                    oe.end    = ottava->tick2();
                                    ol.append(oe);
                                    }
                              }
                        }
                  for (Measure* m = cs->mainLayout()->first(); m; m = m->next()) {
                        for (int voice = 0; voice < VOICES; ++voice) {
                              for (Segment* seg = m->first(); seg; seg = seg->next()) {
                                    Element* el = seg->element(staffIdx * VOICES + voice);
                                    if (el) {
                                          if (el->type() != CHORD)
                                                continue;
                                          Chord* chord = (Chord*)el;
                                          NoteList* nl = chord->noteList();

                                          for (iNote in = nl->begin(); in != nl->end(); ++in) {
                                                Note* note = in->second;
                                                if (note->tieBack())
                                                      continue;
                                                unsigned len = 0;
                                                while (note->tieFor()) {
                                                      len += note->chord()->tickLen();
                                                      note = note->tieFor()->endNote();
                                                      }
                                                len += (note->chord()->tickLen() * gateTime / 100);

                                                unsigned tick   = chord->tick();
                                                len = len * gateTime / 100;
                                                int pitch  = note->pitch();

                                                foreach(OttavaE o, ol) {
                                                      if (tick >= o.start && tick <= o.end) {
                                                            pitch += o.offset;
                                                            break;
                                                            }
                                                      }

                                                MidiEvent* ev = new MidiEvent;
                                                ev->type = ME_NOTEON;
                                                ev->dataA = pitch;
                                                ev->dataB = 0x60;
                                                events->insert(tick, ev);

                                                ev = new MidiEvent;
                                                ev->type  = ME_NOTEON;
                                                ev->dataA = pitch;
                                                ev->dataB = 0x0;
                                                events->insert(tick + len, ev);
                                                }
                                          }
                                    }
                              }

                        foreach(Element* e, *m->el())
                              switch(e->type()) {
                                    case PEDAL:
                                          {
                                          Pedal* pedal = (Pedal*)e;
                                          MidiEvent* ev = new MidiEvent;
                                          ev->type  = ME_CONTROLLER;
                                          ev->dataA = CTRL_SUSTAIN;
                                          ev->dataB = 0x7f;
                                          events->insert(pedal->tick(), ev);
                                          ev        = new MidiEvent;
                                          ev->type  = ME_CONTROLLER;
                                          ev->dataA = CTRL_SUSTAIN;
                                          ev->dataB = 0;
                                          events->insert(pedal->tick2(), ev);
                                          }
                                          break;
                                    default:
                                          break;
                              }
                        }
                  ++staffIdx;
                  }
            ++partIdx;
            }
      return mf.write();
      }

//---------------------------------------------------------
//   LoadMidi
//---------------------------------------------------------

class LoadMidi : public LoadFile {
   public:
      MidiFile mf;
      LoadMidi(Score* s) : mf(s) {}
      virtual bool loader(QFile* f);
      };

//---------------------------------------------------------
//   importMidi
//---------------------------------------------------------

void MuseScore::importMidi()
      {
      QString fn = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Import Midi File"),
         QString("."),
         QString("Midi SMF Files (*.mid *.mid.gz *.mid.bz2);; All files (*)")
         );
      if (fn.isEmpty())
            return;
      Score* score = new Score();
      score->read(fn);
      score->setCreated(true);
      appendScore(score);
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   importMidi
//---------------------------------------------------------

void Score::importMidi(const QString& name)
      {
      LoadMidi lm(this);
      lm.load(name);
      _saved = false;
      convertMidi(&(lm.mf));
      _created = true;
      }

//---------------------------------------------------------
//   loader
//    import midi file
//    return true on error
//---------------------------------------------------------

bool LoadMidi::loader(QFile* fp)
      {
      mf.setFp(fp);
      if (mf.read()) {
            error = mf.error;
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

MidiFile::MidiFile(Score* s)
      {
      cs        = s;
      fp        = 0;
      error     = 0;
      timesig_z = 4;
      timesig_n = 4;
      curPos    = 0;
      _midiType = MT_GM;      // MT_UNKNOWN;
      }

//---------------------------------------------------------
//   setFp
//---------------------------------------------------------

void MidiFile::setFp(QFile* _fp)
      {
      fp = _fp;
      }

//---------------------------------------------------------
//   write
//    returns true on error
//---------------------------------------------------------

bool MidiFile::write()
      {
      write("MThd", 4);
      writeLong(6);                 // header len
      writeShort(1);                // format
      writeShort(_tracks.size());
      writeShort(division);
      foreach(MidiTrack* t, _tracks)
            writeTrack(t);
      return fp->error() != QFile::NoError;
      }

//---------------------------------------------------------
//   writeTrack
//---------------------------------------------------------

bool MidiFile::writeTrack(const MidiTrack* t)
      {
      const EventList* events = t->events();
      write("MTrk", 4);
      qint64 lenpos = fp->pos();
      writeLong(0);                 // dummy len

      status = -1;
      int tick = 0;
      for (ciEvent i = events->begin(); i != events->end(); ++i) {
            int ntick = i.key();
            if (ntick < tick) {
                  printf("MidiFile::writeTrack: ntick %d < tick %d\n", ntick, tick);
                  ntick = tick;
                  }
            putvl(ntick - tick);    // write tick delta
            writeEvent(t->outChannel(), i.value());
            tick = ntick;
            }

      //---------------------------------------------------
      //    write "End Of Track" Meta
      //    write Track Len
      //

      putvl(0);
      put(0xff);        // Meta
      put(0x2f);        // EOT
      putvl(0);         // len 0
      qint64 endpos = fp->pos();
      fp->seek(lenpos);
      writeLong(endpos-lenpos-4);   // tracklen
      fp->seek(endpos);
      return false;
      }

//---------------------------------------------------------
//   writeEvent
//---------------------------------------------------------

void MidiFile::writeEvent(int c, const MidiEvent* event)
      {
      int nstat = event->type;

      // we dont save meta data into smf type 0 files:

      nstat |= c;
      //
      //  running status; except for Sysex- and Meta Events
      //
      if (((nstat & 0xf0) != 0xf0) && (nstat != status)) {
            status = nstat;
            put(nstat);
            }
      switch (event->type) {
            case ME_NOTEOFF:
            case ME_NOTEON:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
            case ME_PITCHBEND:
                  put(event->dataA);
                  put(event->dataB);
                  break;
            case ME_PROGRAM:        // Program Change
            case ME_AFTERTOUCH:     // Channel Aftertouch
                  put(event->dataA);
                  break;
            case ME_SYSEX:
                  put(0xf0);
                  putvl(event->len + 1);  // including 0xf7
                  write(event->data, event->len);
                  put(0xf7);
                  status = -1;      // invalidate running status
                  break;
            case ME_META:
                  put(0xff);
                  put(event->dataA);
                  putvl(event->len);
                  write(event->data, event->len);
                  status = -1;
                  break;
            }
      }

//---------------------------------------------------------
//   readMidi
//    return true on error
//---------------------------------------------------------

bool MidiFile::read()
      {
      errorBuffer[0] = 0;
      error = errorBuffer;
      _tracks.clear();

      char tmp[4];

      read(tmp, 4);
      int len = readLong();
      if (memcmp(tmp, "MThd", 4) || len < 6) {
            error = "bad midifile: MThd expected\n";
            return true;
            }

      int format  = readShort();
      int ntracks = readShort();
      fileDivision = readShort();

      if (fileDivision < 0)
            fileDivision = (-(fileDivision/256)) * (fileDivision & 0xff);
      if (len > 6)
            skip(len-6); // skip the excess

printf("read midi file type %d, tracks %d\n", format, ntracks);

      switch (format) {
            case 0:
                  if (readTrack(true))
                        return true;
                  break;
            case 1:
                  for (int i = 0; i < ntracks; i++) {
                        if (readTrack(false))
                              return true;
                        }
                  break;
            default:
            case 2:
                  sprintf(errorBuffer, "midi fileformat %d not implemented!\n", format);
                  return true;
            }

      foreach(MidiTrack* t, _tracks)
            processTrack(t);
      return false;
      }

//---------------------------------------------------------
//   readTrack
//    return true on error
//---------------------------------------------------------

bool MidiFile::readTrack(bool)
      {
      char tmp[4];
      read(tmp, 4);
      if (memcmp(tmp, "MTrk", 4)) {
            sprintf(errorBuffer, "bad midifile: MTrk expected\n");
            return true;
            }
      int len       = readLong();       // len
      int endPos    = curPos + len;
      status        = -1;
      sstatus       = -1;  // running status, will not be reset on meta or sysex
      channelprefix = -1;
      click         =  0;
      MidiTrack* track  = new MidiTrack();
      _tracks.push_back(track);
      EventList* el = track->events();

      int port = 0;
      track->setOutPort(port);
      track->setOutChannel(-1);

      for (;;) {
            MidiEvent* event = readEvent();
            if (event == 0)
                  return true;

            // check for end of track:
            if ((event->type == ME_META)) {
                  int mt = event->dataA;
                  if (mt == 0x2f)
                        break;

                  switch (mt) {
                        case META_TRACK_NAME:
                              track->setName(QString((char*)event->data));
                              delete event;
                              break;
                        case META_TRACK_COMMENT:
                              track->setComment(QString((char*)event->data));
                              delete event;
                              break;
                        case META_TITLE:
                              title = (char*)event->data;
                              delete event;
                              break;
                        case META_SUBTITLE:
                              subTitle = (char*)event->data;
                              delete event;
                              break;
                        case META_COMPOSER:
                              composer = (char*)(event->data);
                              delete event;
                              break;
                        case META_TRANSLATOR:
                              translator = (char*)event->data;
                              delete event;
                              break;
                        case META_POET:
                              poet = (char*)event->data;
                              delete event;
                              break;
                        case META_LYRIC:
                        case META_TEMPO:
                        case META_TIME_SIGNATURE:
                        case META_KEY_SIGNATURE:
                              el->insert(event->tick, event);
                              break;
                        default:
                              printf("importMidi: meta event 0x%02x not implemented\n", mt);
                              delete event;
                              break;
                        }
                  continue;
                  }
            if (event->channel == -1) {
                  printf("importMidi: event type 0x%02x not implemented\n", event->type);
                  // el->insert(event->tick, event);
                  delete event;
                  continue;
                  }
            if (track->outChannel() == -1)
                  track->setOutChannel(event->channel);
            else if (track->outChannel() != event->channel) {
                  printf("importMidi: channel changed on track %d - %d\n",
                     track->outChannel(), event->channel);
                  }

#if 0
            if (lastchan != channel) {
                  mergeChannels = true;   // DEBUG
                  if (!mergeChannels) {
                        //
                        // try to insert in approp track
                        //
                        int i = 0;
                        for (; i < _tracks.size(); ++i) {
                              MidiTrack* mtrack = _tracks.at(i);
                              if (mtrack == 0)
                                    continue;
                              if (mtrack->outChannel() == lastchan) {
                                    mtrack->events()->insert(event->tick, event);
                                    break;
                                    }
                              }
                        if (i == _tracks.size())
                              printf("no approp. track found\n");
                        }
                  else {
                        //
                        // event with new channel in track
                        //    try to find channel with appropr. track
                        //    or create new track
                        //
                        int i = 0;
                        MidiTrack* mtrack = 0;
                        for (; i < _tracks.size(); ++i) {
                              mtrack = _tracks.at(i);
                              if (mtrack == 0)
                                    continue;
                              if (mtrack->outChannel() == lastchan)
                                    break;
                              }
                        if (i == _tracks.size()) {
                              mtrack = new MidiTrack();
                              mtrack->setOutChannel(lastchan);
                              mtrack->setOutPort(port);
                              _tracks.push_back(mtrack);
                              }
                        track   = mtrack;
                        channel = lastchan;
                        el = track->events();
                        }
                  }
#endif
            el->insert(event->tick, event);
            }
      if (curPos != endPos) {
            printf("bad track len: %d != %d, %d bytes too much\n",
               endPos, curPos, endPos - curPos);
            if (curPos < endPos) {
                  printf("skip %d\n", endPos - curPos);
                  skip(endPos - curPos);
                  }
            }
      return false;
      }

/*---------------------------------------------------------
 *    read
 *    return false on error
 *---------------------------------------------------------*/

bool MidiFile::read(void* p, qint64 len)
      {
      curPos += len;
      qint64 rv = fp->read((char*)p, len);
      return rv != len;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool MidiFile::write(const void* p, qint64 len)
      {
      qint64 rv = fp->write((char*)p, len);
      if (rv == len)
            return false;
      printf("write midifile failed: %s\n", fp->errorString().toLatin1().data());
      return true;
      }

//---------------------------------------------------------
//   readShort
//---------------------------------------------------------

int MidiFile::readShort()
      {
      short format;
      read(&format, 2);
      return BE_SHORT(format);
      }

//---------------------------------------------------------
//   writeShort
//---------------------------------------------------------

void MidiFile::writeShort(int i)
      {
      int format = BE_SHORT(i);
      write(&format, 2);
      }

//---------------------------------------------------------
//   readLong
//   writeLong
//---------------------------------------------------------

int MidiFile::readLong()
      {
      int format;
      read(&format, 4);
      return BE_LONG(format);
      }

//---------------------------------------------------------
//   writeLong
//---------------------------------------------------------

void MidiFile::writeLong(int i)
      {
      int format = BE_LONG(i);
      write(&format, 4);
      }

/*---------------------------------------------------------
 *    skip
 *    This is meant for skipping a few bytes in a
 *    file or fifo.
 *---------------------------------------------------------*/

bool MidiFile::skip(qint64 len)
      {
      if (len <= 0)
            return false;
      char tmp[len];
      return read(tmp, len);
      }

/*---------------------------------------------------------
 *    getvl
 *    Read variable-length number (7 bits per byte, MSB first)
 *---------------------------------------------------------*/

int MidiFile::getvl()
      {
      int l = 0;
      for (int i = 0;i < 16; i++) {
            uchar c;
            if (read(&c, 1))
                  return -1;
            l += (c & 0x7f);
            if (!(c & 0x80)) {
                  return l;
                  }
            l <<= 7;
            }
      return -1;
      }

/*---------------------------------------------------------
 *    putvl
 *    Write variable-length number (7 bits per byte, MSB first)
 *---------------------------------------------------------*/

void MidiFile::putvl(unsigned val)
      {
      unsigned long buf = val & 0x7f;
      while ((val >>= 7) > 0) {
            buf <<= 8;
            buf |= 0x80;
            buf += (val & 0x7f);
            }
      for (;;) {
            put(buf);
            if (buf & 0x80)
                  buf >>= 8;
            else
                  break;
            }
      }

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

MidiTrack::MidiTrack()
      {
      _events     = new EventList;
      _outChannel = 0;
      _outPort    = 0;
      _drumTrack  = false;
      _minor      = false;
      _key        = 0;
      hbank       = 0;
      lbank       = 0;
      program     = 0;
      minPitch    = 127;
      maxPitch    = 0;
      medPitch    = 64;
      }

MidiTrack::~MidiTrack()
      {
      delete _events;
      }

//---------------------------------------------------------
//   checkSysex
//---------------------------------------------------------

bool MidiFile::checkSysex(MidiTrack* track, unsigned len, unsigned char* buf)
      {
      if ((len == gmOnMsgLen) && memcmp(buf, gmOnMsg, gmOnMsgLen) == 0) {
            _midiType = MT_GM;
            return false;
            }
      if ((len == gsOnMsgLen) && memcmp(buf, gsOnMsg, gsOnMsgLen) == 0) {
            _midiType = MT_GS;
            return false;
            }
      if (buf[0] == 0x41) {   // Roland
            if (_midiType != MT_XG)
                  _midiType = MT_GS;
            }

      else if (buf[0] == 0x43) {   // Yamaha
            _midiType = MT_XG;
            int type  = buf[1] & 0xf0;
            switch (type) {
                  case 0x00:  // bulk dump
                        buf[1] = 0;
                        break;
                  case 0x10:
                        if (buf[1] != 0x10) {
// printf("SYSEX Device changed from %d(0x%x) to 1\n", (buf[1] & 0xf) + 1, buf[1]);
                              buf[1] = 0x10;    // fix to Device 1
                              }
                        if (len == 7 && buf[2] == 0x4c && buf[3] == 0x08 && buf[5] == 7) {
                              // part mode
                              // 0 - normal
                              // 1 - DRUM
                              // 2 - DRUM 1
                              // 3 - DRUM 2
                              // 4 - DRUM 3
                              // 5 - DRUM 4
                              printf("xg set part mode channel %d to %d\n", buf[4]+1, buf[6]);
                              if (buf[6] != 0)
                                    track->setDrumTrack(true);
                              }
                        break;
                  case 0x20:
                        printf("YAMAHA DUMP REQUEST\n");
                        return true;
                  case 0x30:
                        printf("YAMAHA PARAMETER REQUEST\n");
                        return true;
                  default:
                        printf("YAMAHA unknown SYSEX: data[2]=%02x\n", buf[1]);
//                        dump(buf, len);
                        return true;
                  }
            }
      else if (buf[0] == 0x42) {   // Korg
//            int device = buf[1] & 0xf;
            if ((buf[1] & 0xf0) != 0x30) {
                  printf("KORG SYSEX??\n");
                  }
//            buf[1] &= 0xf;    // fest auf Device 1 klemmen
            fflush(stdout);
            }
      else if (buf[0] == 0x41) {   // Korg
//            int device = buf[1] & 0xf;
            if ((buf[1] & 0xf0) != 0x10) {
                  printf("GM SYSEX??\n");
                  }
//            buf[1] &= 0xf;    // fest auf Device 1 klemmen
            fflush(stdout);
            }

      if ((len == xgOnMsgLen) && memcmp(buf, xgOnMsg, xgOnMsgLen) == 0) {
            _midiType = MT_XG;
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   readEvent
//---------------------------------------------------------

MidiEvent* MidiFile::readEvent()
      {
      uchar me, a, b;

      int nclick = getvl();
      if (nclick == -1) {
            printf("readEvent: error 1\n");
            return 0;
            }
      click += nclick;
      for (;;) {
            if (read(&me, 1)) {
                  sprintf(errorBuffer, "readEvent: error 2\n");
                  return 0;
                  }
            if (me >= 0xf1 && me <= 0xfe && me != 0xf7) {
                  if (debugMode)
                        printf("Midi: Unknown Message 0x%02x\n", me & 0xff);
                  }
            else
                  break;
            }

      MidiEvent* event = new MidiEvent;
      event->tick = (click * division + fileDivision/2) / fileDivision;

      if (me == 0xf0 || me == 0xf7) {
            status  = -1;                  // no running status
            int len = getvl();
            if (len == -1) {
                  printf("readEvent: error 3\n");
                  delete event;
                  return 0;
                  }
            event->data    = new unsigned char[len];
            event->dataLen = len;
            event->type    = ME_SYSEX;
            if (read(event->data, len)) {
                  printf("readEvent: error 4\n");
                  delete event;
                  return 0;
                  }
            if (event->data[len-1] != 0xf7) {
                  printf("SYSEX does not end with 0xf7!\n");
                  // more to come?
                  }
            else
                  event->dataLen--;      // don't count 0xf7
            }
      else if (me == ME_META) {
            status = -1;                  // no running status
            uchar type;
            if (read(&type, 1)) {
                  printf("readEvent: error 5\n");
                  delete event;
                  return 0;
                  }
            int len = getvl();                // read len
            if (len == -1) {
                  printf("readEvent: error 6\n");
                  delete event;
                  return 0;
                  }
            event->type    = ME_META;
            event->dataLen = len;
            event->data    = new unsigned char[len+1];
            event->dataA   = type;

            if (len) {
                  if (read(event->data, len)) {
                        printf("readEvent: error 7\n");
                        delete event;
                        return 0;
                        }
                  }
            event->data[len] = 0;
            }
      else {
            if (me & 0x80) {                     // status byte
                  status   = me;
                  sstatus  = status;
                  if (read(&a, 1)) {
                        printf("readEvent: error 9\n");
                        delete event;
                        return 0;
                        }
                  }
            else {
                  if (status == -1) {
                        printf("readEvent: no running status, read 0x%02x\n", me);
                        printf("sstatus ist 0x%02x\n", sstatus);
                        if (sstatus == -1) {
                              delete event;
                              return 0;
                              }
                        status = sstatus;
                        }
                  a = me;
                  }
            event->channel = status & 0x0f;
            event->type    = status & 0xf0;
            event->dataA   = a & 0x7f;
            b = 0;
            switch (status & 0xf0) {
                  case 0x80:        // note off
                  case 0x90:        // note on
                  case 0xa0:        // polyphone aftertouch
                  case 0xb0:        // controller
                  case 0xe0:        // pitch bend
                        if (read(&b, 1)) {
                              printf("readEvent: error 15\n");
                              delete event;
                              return 0;
                              }
                        if ((status & 0xf0) == 0xe0)
                              event->dataA = (((((b & 0x80) ? 0 : b) << 7) + a) - 8192);
                        else
                              event->dataB = (b & 0x80 ? 0 : b);
                        break;
                  case 0xc0:        // Program Change
                  case 0xd0:        // Channel Aftertouch
                        break;
                  default:          // f1 f2 f3 f4 f5 f6 f7 f8 f9
                        printf("BAD STATUS 0x%02x, me 0x%02x\n", status, me);
                        delete event;
                        return 0;
                  }

            if ((a & 0x80) || (b & 0x80)) {
                  printf("8't bit in data set(%02x %02x): tick %d read 0x%02x  status:0x%02x\n",
                    a & 0xff, b & 0xff, click, me, status);
                  printf("readEvent: error 16\n");
                  if (b & 0x80) {
                        // Try to fix: interpret as channel byte
                        status   = b;
                        sstatus  = status;
                        return event;
                        }
                  delete event;
                  return 0;
                  }
            }
      return event;
      }

//---------------------------------------------------------
//   processTrack
//---------------------------------------------------------

void MidiFile::processTrack(MidiTrack* track)
      {
      if (track->outChannel() == 9 && midiType() != MT_UNKNOWN)
            track->setDrumTrack(true);

      //---------------------------------------------------
      //    get initial controller state
      //---------------------------------------------------

      EventList* tevents = track->events();
      track->hbank   = -1;
      track->lbank   = -1;
      track->program = -1;

      foreach(MidiEvent* ev, *tevents) {
            if (ev->type == ME_PROGRAM) {
                  if (track->program == -1) {
                        track->program = ev->dataA;
                        tevents->remove(ev->tick, ev);
                        }
                  }
            }

      //---------------------------------------------------
      //    change NoteOff events into Note events
      //    with len
      //---------------------------------------------------

      foreach (MidiEvent* event, *tevents)      // invalidate all note len
            if (event->isNote())
                  event->len = -1;

      foreach (MidiEvent* ev, *tevents) {
            if (!ev->isNote())
                  continue;
            int tick = ev->tick;
            if (ev->isNoteOff()) {
                  // printf("NOTE OFF without Note ON at tick %d\n", tick);
                  continue;
                  }

            iEvent k;
            for (k = tevents->lowerBound(tick); k != tevents->end(); ++k) {
                  MidiEvent* event = k.value();
                  if (ev->isNoteOff(event)) {
                        int t = k.key() - tick;
                        if (t <= 0)
                              t = 1;
                        ev->len = t;
                        ev->dataC = event->dataC;
                        break;
                        }
                  }
            if (k == tevents->end()) {
                  printf("-no note-off! %d pitch %d velo %d\n",
                     tick, ev->dataA, ev->dataB);
                  //
                  // note off at end of bar
                  //
                  int endTick = ev->tick + 1; // song->roundUpBar(ev->tick + 1);
                  ev->len = endTick - ev->tick;
                  }
            else {
                  tevents->erase(k);      // memory leak
                  }
            }

      // DEBUG: any note off left?

      for (iEvent i = tevents->begin(); i != tevents->end(); ++i) {
            MidiEvent* ev  = i.value();
            if (ev->isNoteOff()) {
                  printf("+extra note-off! %d pitch %d velo %d\n",
                           i.key(), ev->dataA, ev->dataB);
                  }
            }
      }

//---------------------------------------------------------
//   empty
//    check for empty track
//---------------------------------------------------------

bool MidiTrack::empty() const
      {
      return _events->empty();
      }

//---------------------------------------------------------
//   instrName
//---------------------------------------------------------

QString MidiTrack::instrName(int type) const
      {
      if (program != -1) {
            for (unsigned int i = 0; i < sizeof(minstr)/sizeof(*minstr); ++i) {
                  MidiInstrument* mi = &minstr[i];
                  if ((mi->patch == program)
                     && (mi->type & type)
                     && (mi->hbank == hbank || hbank == -1)
                     && (mi->lbank == lbank || lbank == -1)) {
                        return QString(mi->name);
                        }
                  }
            }
      return name();
      }

// prepare:
//    * remove empty tracks
//    * Tracknamen feststellen; sollen Kommentare oder
//      Instrumentennamen verwendet werden?
//    - Instrumente feststellen
//          - Name (kommentar?)
//    - Schlagzeugtrack markieren
//    - Quantisierung festlegen:
//    for every measure:
//          * create measure
//          - insert notes
//          - Triolen?

//---------------------------------------------------------
//   quantizeLen
//---------------------------------------------------------

static int quantizeLen(int len)
      {
      if (len < division/12)
            len = division/8;
      else if (len < division/6)
            len = division/4;
      else if (len < division/3)
            len = division/2;
      else if (len < division+division/2)
            len = division;
      else if (len < division * 3)
            len = division*2;
      else if (len < division * 6)
            len = division * 4;
      else if (len < division * 12)
            len = division * 8;
      else
            len = division * 16;
      return len;
      }

//---------------------------------------------------------
//   convertMidi
//---------------------------------------------------------

void Score::convertMidi(MidiFile* mf)
      {
      MidiTrackList* tracks = mf->tracks();

      //---------------------------------------------------
      //  remove empty tracks
      //---------------------------------------------------

      foreach (MidiTrack* track, *tracks) {
		int events = 0;
            EventList* el = track->events();
            for (iEvent ie = el->begin(); ie != el->end(); ++ie) {
                  if (ie.value()->isNote()) {
                        ++events;
                        int pitch = ie.value()->pitch();
                        if (pitch > track->maxPitch)
                              track->maxPitch = pitch;
                        if (pitch < track->minPitch)
                              track->minPitch = pitch;
                        track->medPitch += pitch;
                        }
                  }
            if (events == 0)
		      tracks->removeAt(tracks->indexOf(track));
            else
	            track->medPitch = track->medPitch / events;
            }

      //---------------------------------------------------
      //  create instruments
      //---------------------------------------------------

      foreach (MidiTrack* midiTrack, *tracks) {
            int staves = (midiTrack->maxPitch - midiTrack->minPitch > 36) ? 2 : 1;
            Part* part = new Part(this);
            for (int staff = 0; staff < staves; ++staff) {
                  Staff* s = new Staff(this, part, staff);
                  part->insertStaff(s);
                  _staves->push_back(s);
                  if (staves == 2) {
                        s->clef()->setClef(0, staff == 0 ? 0 : 4);
                        }
                  else {
                        s->clef()->setClef(0, midiTrack->medPitch < 58 ? 4 : 0);
                        }
                  }
            if (midiTrack->name().isEmpty()) {
                  // Text t(midiTrack->instrName(mf->midiType()), TEXT_STYLE_INSTRUMENT_LONG);
                  QString t(midiTrack->instrName(mf->midiType()));
                  part->setLongName(t);
                  }
            else
                  // part->setLongName(Text(midiTrack->name(), TEXT_STYLE_INSTRUMENT_LONG));
                  part->setLongName(midiTrack->name());
            part->setTrackName(part->longName().toPlainText());
            part->setMidiChannel(midiTrack->outChannel());
            part->setMidiProgram(midiTrack->program);
            _parts.push_back(part);
            }

      int lastTick = 0;
      foreach (MidiTrack* midiTrack, *tracks) {
            EventList* el = midiTrack->events();
            if (!el->empty()) {
                  iEvent i = el->end();
                  --i;
                  if (i.key() >lastTick)
                        lastTick = i.key();
                  }
            }

      //---------------------------------------------------
      //  remove empty measures at beginning
      //---------------------------------------------------

      int startBar, endBar, beat, tick;
      sigmap->tickValues(lastTick, &endBar, &beat, &tick);
      if (beat || tick)
            ++endBar;
      for (startBar = 0; startBar < endBar; ++startBar) {
            int tick1 = sigmap->bar2tick(startBar, 0, 0);
            int tick2 = sigmap->bar2tick(startBar + 1, 0, 0);
            int events = 0;
            foreach (MidiTrack* midiTrack, *tracks) {
                  EventList* el = midiTrack->events();
                  iEvent i1     = el->lowerBound(tick1);
                  iEvent i2     = el->lowerBound(tick2);
                  for (iEvent ie = i1; ie != i2; ++ie) {
                        if (ie.value()->isNote()) {
                              ++events;
                              break;
                              }
                        }
                  }
            if (events)
                  break;
            }

      //---------------------------------------------------
      //  count measures
      //---------------------------------------------------

      int bars = 1;
      foreach (MidiTrack* midiTrack, *tracks) {
            EventList* el = midiTrack->events();
            for (iEvent ie = el->begin(); ie != el->end(); ++ie) {
                  if (!(ie.value()->isNote()))
                        continue;
                  int tick = ie.key() + ie.value()->len;
                  int bar, beat, rest;
                  sigmap->tickValues(tick, &bar, &beat, &rest);
                  if ((bar+1) > bars)
                        bars = bar + 1;
                  }
            }

      //---------------------------------------------------
      //  create measures
      //---------------------------------------------------

      for (int i = startBar; i < bars; ++i) {
            Measure* measure  = new Measure(this);
            int tick = sigmap->bar2tick(i, 0, 0);
            measure->setTick(tick);

            for (iStaff i = _staves->begin(); i != _staves->end(); ++i) {
            	Staff* s = *i;
	            if (s->isTop()) {
      	            BarLine* barLine = new BarLine(this);
            	      barLine->setStaff(s);
	                  measure->setEndBarLine(barLine);
      	            }
                  }
      	_layout->push_back(measure);
            }

	foreach (MidiTrack* midiTrack, *tracks)
            preprocessTrack(midiTrack);

	int staffIdx = 0;
	foreach (MidiTrack* midiTrack, *tracks)
            convertTrack(midiTrack, staffIdx++);

      for (iSigEvent is = sigmap->begin(); is != sigmap->end(); ++is) {
            SigEvent se = is->second;
            int tick    = is->first;
            Measure* m  = tick2measure(tick);
            if (m) {
                  for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                        TimeSig* ts = new TimeSig(this);
                        ts->setTick(tick);
                        ts->setSig(se.nominator, se.denominator);
                        ts->setStaff(staff(staffIdx));
                        Segment* seg = m->getSegment(ts);
                        seg->add(ts);
                        }
                  }
            }
      Measure* measure = tick2measure(0);
      if (measure == 0)
            return;
      if (!mf->title.isEmpty()) {
            Text* text = new Text(this);
            text->setSubtype(TEXT_TITLE);
            text->setText(mf->title);
            measure->add(text);
            }
      if (!mf->subTitle.isEmpty()) {
            Text* text = new Text(this);
            text->setSubtype(TEXT_SUBTITLE);
            text->setText(mf->subTitle);
            measure->add(text);
            }
      if (!mf->composer.isEmpty()) {
            Text* text = new Text(this);
            text->setSubtype(TEXT_COMPOSER);
            text->setText(mf->composer);
            measure->add(text);
            }
      if (!mf->translator.isEmpty()) {
            Text* text = new Text(this);
            text->setSubtype(TEXT_TRANSLATOR);
            text->setText(mf->translator);
            measure->add(text);
            }
      if (!mf->poet.isEmpty()) {
            Text* text = new Text(this);
            text->setSubtype(TEXT_POET);
            text->setText(mf->poet);
            measure->add(text);
            }
      }

//---------------------------------------------------------
//   preprocessTrack
//	 - perform silly quantisation
//	 - remove overlapping
//---------------------------------------------------------

void Score::preprocessTrack(MidiTrack* midiTrack)
	{
      static const int tickRaster = division/2; 	// 1/8 quantize

	EventList* sl = midiTrack->events();
      EventList* dl = new EventList;

      //
      //	quantize
      //
      for (iEvent i = sl->begin(); i != sl->end(); ++i) {
            MidiEvent* e = i.value();
            if (e->isNote()) {
	            int len  = quantizeLen(e->len);
      	      int tick = (e->tick / tickRaster) * tickRaster;

	            e->tick = tick;
      	      e->len  = len;
                  }
		dl->insert(e->tick, e);
            }
      //
      //
      //
      sl->clear();

      for (iEvent i = dl->begin(); i != dl->end(); ++i) {
            MidiEvent* e = i.value();
            if (e->isNote()) {
                  iEvent ii = i;
                  ++ii;
                  for (; ii != dl->end(); ++ii) {
                        MidiEvent* ee = ii.value();
                        if (!ee->isNote() || ee->pitch() != e->pitch())
                              continue;
                        if (ee->tick >= e->tick + e->len)
                              break;
                        e->len = ee->tick - e->tick;
                        break;
                        }
                  if (e->len <= 0)
                        continue;
// printf("Note %6d %3d\n", e->tick, e->len);
                  }
		sl->insert(e->tick, e);
            }
      delete dl;
      }

//---------------------------------------------------------
//   MNote
//	Midi Note
//---------------------------------------------------------

struct MNote {
	int pitch, velo;
      int tick;
      int len;
      Tie* tie;

      MNote(int p, int v, int t, int l)
         : pitch(p), velo(v), tick(t), len(l), tie(0) {}
      };

//---------------------------------------------------------
//   convertTrack
//---------------------------------------------------------

void Score::convertTrack(MidiTrack* midiTrack, int staffIdx)
	{
	EventList* el = midiTrack->events();
      QList<MNote*> notes;

	int ctick = 0;
      for (iEvent i = el->begin(); i != el->end();) {
            MidiEvent* e = i.value();
            if (!(e->isNote())) {
                  ++i;
                  continue;
                  }
            //
            // process pending notes
            //
            while (!notes.isEmpty()) {
                  int tick = notes[0]->tick;
                  int len  = i.key() - tick;
                  if (len <= 0)
                        break;
            	foreach (MNote* n, notes) {
                  	if (n->len < len)
                              len = n->len;
                        }
      		Measure* measure = tick2measure(tick);

                  // split notes on measure boundary
                  if ((tick + len) > measure->tick() + measure->tickLen()) {
                        len = measure->tick() + measure->tickLen() - tick;
                        }
                  Chord* chord = new Chord(this, tick);
                  chord->setStaff(staff(staffIdx));
                  chord->setTickLen(len);
                  Segment* s = measure->getSegment(chord);
                  s->add(chord);

            	foreach (MNote* n, notes) {
            		Note* note = new Note(this, n->pitch, false);
            		note->setStaff(staff(staffIdx));
            		chord->add(note);
            		// note->setTicks(len);
                        note->setTick(tick);
                        if (n->tie) {
                              n->tie->setEndNote(note);
                              n->tie->setStaff(note->staff());
                              note->setTieBack(n->tie);
                              }
                        if (n->len <= len) {
                              notes.removeAt(notes.indexOf(n));
                              continue;
                              }
				n->tie = new Tie(this);
                        n->tie->setStartNote(note);
				note->setTieFor(n->tie);
	                  n->tick += len;
                        n->len  -= len;
                        }
                  ctick += len;
                  }
            //
            // check for gap and fill with rest
            //
            int restLen = i.key() - ctick;
            if (restLen > 0) {
                  Rest* rest = new Rest(this, ctick, restLen);
      		Measure* measure = tick2measure(ctick);
                  rest->setStaff(staff(staffIdx));
                  Segment* s = measure->getSegment(rest);
                  s->add(rest);
                  }
            ctick = i.key();

            //
            // collect all notes on current
            // tick position
            //
            for (;i != el->end(); ++i) {
            	MidiEvent* e = i.value();
                  if (!e->isNote())
                        continue;
                  if (i.key() != ctick)
                        break;
            	MNote* n = new MNote(e->pitch(), e->velo(), e->tick, e->len);
      	      notes.append(n);
                  }
            }

      for (iEvent i = el->begin(); i != el->end(); ++i) {
            MidiEvent* e = i.value();
            if (e->type == ME_META) {
                  switch(e->dataA) {
                        case META_LYRIC:
                              {
      		            Measure* measure = tick2measure(e->tick);
                              Segment* seg = measure->findSegment(Segment::SegChordRest, e->tick);
                              if (seg) {
                                    Lyrics* l = new Lyrics(this);
                                    QString txt((char*)(e->data));
                                    l->setText(txt);
                                    seg->setLyrics(staffIdx, l);
                                    }
                              }
                              break;
                        case META_TEMPO:
                              break;
                        case META_TIME_SIGNATURE:
                              break;
                        case META_KEY_SIGNATURE:
                              break;
                        }
                  }
            }
      //
	// process pending notes
      //
      if (notes.isEmpty())
            return;
      int tick = notes[0]->tick;
	Measure* measure = tick2measure(tick);
      Chord* chord = new Chord(this, tick);
      chord->setStaff(staff(staffIdx));
      Segment* s = measure->getSegment(chord);
      s->add(chord);
      int len = 0x7fffffff;      // MAXINT;
	foreach (MNote* n, notes) {
      	if (n->len < len)
                  len = n->len;
            }
      chord->setTickLen(len);
	foreach (MNote* n, notes) {
		Note* note = new Note(this, n->pitch, false);
		note->setStaff(staff(staffIdx));
//		note->setTicks(len);
            note->setTick(tick);
		chord->add(note);
            n->len -= len;
            if (n->len <= 0) {
                  notes.removeAt(notes.indexOf(n));
                  delete n;
                  }
            else
                  n->tick += len;
            }
      ctick += len;
      //
      // check for gap and fill with rest
      //
      int restLen = measure->tick() + measure->tickLen() - ctick;
      if (restLen > 0) {
            Rest* rest = new Rest(this, ctick, restLen);
		Measure* measure = tick2measure(ctick);
            rest->setStaff(staff(staffIdx));
            Segment* s = measure->getSegment(rest);
            s->add(rest);
            }
      }

