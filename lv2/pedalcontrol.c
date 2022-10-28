/*
  Copyright 2013-2016 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "lv2/atom/atom.h"
#include "lv2/atom/util.h"
#include "lv2/core/lv2.h"
#include "lv2/core/lv2_util.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/urid/urid.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PEDALCONTROL_URI "http://paingrille.fr/plugins/pedalcontrol"

typedef enum {
  PEDALBOARD_EXPRTYPE = 0,
  PEDALBOARD_EXPRCHAN,
  PEDALBOARD_EXPRADDR,
  PEDALBOARD_EXPRCURV,
  PEDALBOARD_MIDI_IN,
  PEDALBOARD_MIDI_OUT,
} PortIndex;

typedef struct {
  // inputs
  float *exprType;
  float *exprChan;
  float *exprAddr;
  float *exprCurv;

  // values
  int type;
  int chan;
  int addr;
  int curv;

  // Port buffers
  const LV2_Atom_Sequence* midiIn;
  LV2_Atom_Sequence* midiOut;

  // Features
  LV2_URID_Map*  map;
  LV2_Log_Logger logger;

  struct {
    LV2_URID midi_MidiEvent;
  } uris;

} Pedalcontrol;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  Pedalcontrol* self = (Pedalcontrol*)calloc(1, sizeof(Pedalcontrol));
  if (!self) {
    return NULL;
  }

  // Scan host features for URID map
  // clang-format off
  const char* missing = lv2_features_query(
    features,
    LV2_LOG__log,  &self->logger.log, false,
    LV2_URID__map, &self->map,        true,
    NULL);
  // clang-format on

  lv2_log_logger_set_map(&self->logger, self->map);
  if (missing) {
    lv2_log_error(&self->logger, "Missing feature <%s>\n", missing);
    free(self);
    return NULL;
  }

  self->uris.midi_MidiEvent =
    self->map->map(self->map->handle, LV2_MIDI__MidiEvent);

  return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance, uint32_t port, void* data)
{
  Pedalcontrol* self = (Pedalcontrol*)instance;

  self->type = -1;
  self->chan = -1;
  self->addr = -1;
  self->curv = -1;

  switch ((PortIndex)port) {
  case PEDALBOARD_EXPRTYPE:
    self->exprType = (float*)data;
    break;
  case PEDALBOARD_EXPRCHAN:
    self->exprChan = (float*)data;
    break;
  case PEDALBOARD_EXPRADDR:
    self->exprAddr = (float*)data;
    break;
  case PEDALBOARD_EXPRCURV:
    self->exprCurv = (float*)data;
    break;
  case PEDALBOARD_MIDI_IN:
    self->midiIn = (const LV2_Atom_Sequence*)data;
    break;
  case PEDALBOARD_MIDI_OUT:
    self->midiOut = (LV2_Atom_Sequence*)data;
    break;
  }
}

static void
activate(LV2_Handle instance)
{
  Pedalcontrol* self       = (Pedalcontrol*)instance;
}

static void
run(LV2_Handle instance, uint32_t sample_count)
{
	Pedalcontrol* self   = (Pedalcontrol*)instance;
	const uint32_t out_capacity = self->midiOut->atom.size;

	// Struct for a 3 byte MIDI event, used for writing notes
	typedef struct {
		LV2_Atom_Event event;
		uint8_t        msg[3];
	} MIDINoteEvent;

	MIDINoteEvent cfg;

	lv2_atom_sequence_clear(self->midiOut);
	self->midiOut->atom.type = self->midiIn->atom.type;

	LV2_ATOM_SEQUENCE_FOREACH(self->midiIn, ev) {
		// just forward everything
		lv2_atom_sequence_append_event(self->midiOut, out_capacity, ev);
	}
	cfg.event.time.frames = 0;
	cfg.event.body.type = self->uris.midi_MidiEvent;
	cfg.event.body.size = 3; // ???
	cfg.msg[0] = 0x90;

	if ((int)*(self->exprType) != self->type) {
		self->type = (int)*(self->exprType);
		cfg.msg[1] = 0;
		cfg.msg[2] = self->type;
		lv2_atom_sequence_append_event(self->midiOut, out_capacity, &cfg.event);
		printf("new type\n");
	}
	if ((int)*(self->exprChan) != self->chan) {
		self->chan = (int)*(self->exprChan);
		cfg.msg[1] = 1;
		cfg.msg[2] = self->chan;
		lv2_atom_sequence_append_event(self->midiOut, out_capacity, &cfg.event);
		printf("new chan\n");
	}
	if ((int)*(self->exprAddr) != self->addr) {
		self->addr = (int)*(self->exprAddr);
		cfg.msg[1] = 2;
		cfg.msg[2] = self->addr;
		lv2_atom_sequence_append_event(self->midiOut, out_capacity, &cfg.event);
		printf("new addr\n");
	}
	if ((int)*(self->exprCurv) != self->curv) {
		self->curv = (int)*(self->exprCurv);
		cfg.msg[1] = 3;
		cfg.msg[2] = self->curv;
		lv2_atom_sequence_append_event(self->midiOut, out_capacity, &cfg.event);
		printf("new curve\n");
	}


}

/**
   We have no resources to free on deactivation.
   Note that the next call to activate will re-initialise the state, namely
   self->n_active_notes, so there is no need to do so here.
*/
static void
deactivate(LV2_Handle instance)
{}

static void
cleanup(LV2_Handle instance)
{
  free(instance);
}

/**
   This plugin also has no extension data to return.
*/
static const void*
extension_data(const char* uri)
{
  return NULL;
}

static const LV2_Descriptor descriptor = {PEDALCONTROL_URI,
                                          instantiate,
                                          connect_port,
                                          activate,
                                          run,
                                          deactivate,
                                          cleanup,
                                          extension_data};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
  return index == 0 ? &descriptor : NULL;
}
