//=============================================================================
//  MuseScore
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

#ifndef __SOUNDFONT_H__
#define __SOUNDFONT_H__

#include "libmscore/xml.h"

//---------------------------------------------------------
//   sfVersionTag
//---------------------------------------------------------

struct sfVersionTag {
      int major;
      int minor;
      };

enum Modulator {
      };
enum Generator {
      Gen_StartAddrOfs, Gen_EndAddrOfs, Gen_StartLoopAddrOfs,
      Gen_EndLoopAddrOfs, Gen_StartAddrCoarseOfs, Gen_ModLFO2Pitch,
      Gen_VibLFO2Pitch, Gen_ModEnv2Pitch, Gen_FilterFc, Gen_FilterQ,
      Gen_ModLFO2FilterFc, Gen_ModEnv2FilterFc, Gen_EndAddrCoarseOfs,
      Gen_ModLFO2Vol, Gen_Unused1, Gen_ChorusSend, Gen_ReverbSend, Gen_Pan,
      Gen_Unused2, Gen_Unused3, Gen_Unused4,
      Gen_ModLFODelay, Gen_ModLFOFreq, Gen_VibLFODelay, Gen_VibLFOFreq,
      Gen_ModEnvDelay, Gen_ModEnvAttack, Gen_ModEnvHold, Gen_ModEnvDecay,
      Gen_ModEnvSustain, Gen_ModEnvRelease, Gen_Key2ModEnvHold,
      Gen_Key2ModEnvDecay, Gen_VolEnvDelay, Gen_VolEnvAttack,
      Gen_VolEnvHold, Gen_VolEnvDecay, Gen_VolEnvSustain, Gen_VolEnvRelease,
      Gen_Key2VolEnvHold, Gen_Key2VolEnvDecay, Gen_Instrument,
      Gen_Reserved1, Gen_KeyRange, Gen_VelRange,
      Gen_StartLoopAddrCoarseOfs, Gen_Keynum, Gen_Velocity,
      Gen_Attenuation, Gen_Reserved2, Gen_EndLoopAddrCoarseOfs,
      Gen_CoarseTune, Gen_FineTune, Gen_SampleId, Gen_SampleModes,
      Gen_Reserved3, Gen_ScaleTune, Gen_ExclusiveClass, Gen_OverrideRootKey,
      Gen_Dummy
      };

enum Transform { Linear };

//---------------------------------------------------------
//   ModulatorList
//---------------------------------------------------------

struct ModulatorList {
      Modulator src;
      Generator dst;
      int amount;
      Modulator amtSrc;
      Transform transform;
      };

//---------------------------------------------------------
//   GeneratorList
//---------------------------------------------------------

union GeneratorAmount {
      short sword;
      ushort uword;
      struct {
            uchar lo, hi;
            };
      };

struct GeneratorList {
      Generator gen;
      GeneratorAmount amount;
      };

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

struct Zone {
      QList<GeneratorList*> generators;
      QList<ModulatorList*> modulators;
      int instrumentIndex;
      };

//---------------------------------------------------------
//   Preset
//---------------------------------------------------------

struct Preset {
      char* name;
      int preset;
      int bank;
      int presetBagNdx; // used only for read
      int library;
      int genre;
      int morphology;
      QList<Zone*> zones;
      };

//---------------------------------------------------------
//   SfInstrument
//---------------------------------------------------------

struct SfInstrument {
      char* name;
      int index;        // used only for read
      QList<Zone*> zones;

      SfInstrument();
      ~SfInstrument();
      };

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

struct Sample {
      char* name;
      unsigned int start;
      unsigned int end;
      unsigned int loopstart;
      unsigned int loopend;
      unsigned int samplerate;
      int origpitch;
      int pitchadj;
      int sampletype;

      Sample();
      ~Sample();
      };

//---------------------------------------------------------
//   SoundFont
//---------------------------------------------------------

class SoundFont {
      QString path;
      sfVersionTag version;
      char* engine;
      char* name;
      char* date;
      char* comment;
      char* tools;
      char* creator;
      char* product;
      char* copyright;

      int samplePos;
      int sampleLen;

      QList<Preset*> presets;
      QList<SfInstrument*> instruments;

      QList<Zone*> pZones;
      QList<Zone*> iZones;
      QList<Sample*> samples;

      QFile* file;
      unsigned readDword();
      int readWord();
      int readShort();
      int readByte();
      int readChar();
      int readFourcc(const char*);
      int readFourcc(char*);
      void readSignature(const char* signature);
      void readSignature(char* signature);
      void skip(int);
      void readSection(const char* fourcc, int len);
      void readVersion();
      char* readString(int);
      void readPhdr(int);
      void readBag(int, QList<Zone*>*);
      void readMod(int, QList<Zone*>*);
      void readGen(int, QList<Zone*>*);
      void readInst(int);
      void readShdr(int);

      void writeDword(int);
      void writeWord(unsigned short int);
      void writeByte(unsigned char);
      void writeChar(char);
      void writeShort(short);
      void write(const char* p, int n);
      void write(Xml&, Zone*);
      bool writeSampleFile(Sample*, QString);
      void writeSample(const Sample*);
      void writeStringSection(const char* fourcc, char* s);
      void writePreset(int zoneIdx, const Preset*);
      void writeModulator(const ModulatorList*);
      void writeGenerator(const GeneratorList*);
      void writeInstrument(int zoneIdx, const SfInstrument*);

      void writeIfil();
      void writeSmpl();
      void writePhdr();
      void writeBag(const char* fourcc, QList<Zone*>*);
      void writeMod(const char* fourcc, const QList<Zone*>*);
      void writeGen(const char* fourcc, QList<Zone*>*);
      void writeInst();
      void writeShdr();

      int writeCompressedSample(Sample*);
      char* readCompressedSample(Sample*);

   public:
      SoundFont(const QString&);
      ~SoundFont();
      bool read();
      bool write(QFile*);
      bool readXml(QFile*);
      bool writeXml(QFile*);
      };
#endif

