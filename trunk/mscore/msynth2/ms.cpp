//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "msynth.h"
#include "event.h"

#include <stdio.h>
#include <signal.h>
#include <jack/jack.h>
#include <jack/midiport.h>

jack_port_t* input_port;
jack_port_t* output_portL;
jack_port_t* output_portR;

jack_client_t* client;
MSynth* synth;

//---------------------------------------------------------
//   signal_handler
//---------------------------------------------------------

static void signal_handler(int sig)
      {
      jack_client_close(client);
      fprintf(stderr, "signal received, exiting...\n");
      exit(0);
      }

//---------------------------------------------------------
//   jack_shutdown
//---------------------------------------------------------

static void jack_shutdown(void*)
      {
      exit(1);
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

static int process(jack_nframes_t nframes, void*)
      {
      void*  mi = jack_port_get_buffer(input_port, nframes);
      float* lb = (float *) jack_port_get_buffer (output_portL, nframes);
      float* rb = (float *) jack_port_get_buffer (output_portR, nframes);

      jack_nframes_t event_count = jack_midi_get_event_count(mi);
      for (int i = 0; i < event_count; i++) {
            jack_midi_event_t event;
            jack_midi_event_get(&event, mi, i);
            int type = *(event.buffer) & 0xf0;
            if (type == 0x90)
                  synth->play(Event(ME_NOTEON, 0, *(event.buffer+1), *(event.buffer+2)));
            else if (type == 0x80)
                  synth->play(Event(ME_NOTEON, 0, *(event.buffer+1), 0));
            else if (type == ME_CONTROLLER)
                  synth->play(Event(ME_CONTROLLER, 0, *(event.buffer+1), *(event.buffer+2)));

            }
      synth->process(nframes, lb, rb, 1);
      return 0;
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      if ((client = jack_client_open("ms", JackNullOption, 0)) == 0)
            return 1;
      int sampleRate = jack_get_sample_rate(client);

      synth = new MSynth();
      synth->setSamplerate(sampleRate);
      if (!synth->loadInstrument("piano.msfz")) {
            fprintf(stderr, "cannot load instrument\n");
            return 1;
            }

      jack_set_process_callback(client, process, 0);
      jack_on_shutdown(client, jack_shutdown, 0);

      input_port = jack_port_register(client, "midi_in",
         JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
      output_portL = jack_port_register(client, "audio_outL",
         JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
      output_portR = jack_port_register(client, "audio_outR",
         JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

      if (jack_activate(client)) {
            fprintf(stderr, "cannot activate client");
            return 1;
            }
      jack_connect(client, "ms:audio_outL", "system:playback_1");
      jack_connect(client, "ms:audio_outR", "system:playback_2");
      jack_connect(client, "system:midi_capture_1", "ms:midi_in");
      jack_connect(client, "system:midi_capture_2", "ms:midi_in");
      jack_connect(client, "system:midi_capture_3", "ms:midi_in");
      jack_connect(client, "system:midi_capture_4", "ms:midi_in");
      jack_connect(client, "system:midi_capture_5", "ms:midi_in");
      jack_connect(client, "system:midi_capture_6", "ms:midi_in");
      jack_connect(client, "system:midi_capture_7", "ms:midi_in");
      jack_connect(client, "system:midi_capture_8", "ms:midi_in");
      jack_connect(client, "system:midi_capture_9", "ms:midi_in");
      jack_connect(client, "system:midi_capture_10", "ms:midi_in");
      signal(SIGQUIT, signal_handler);
      signal(SIGTERM, signal_handler);
      signal(SIGHUP, signal_handler);
      signal(SIGINT, signal_handler);

      while(true)
            sleep(1);
      jack_client_close(client);
      return 0;
      }

