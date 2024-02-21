/*
** sharedmidistate.h
**
** This file is part of mkxp.
**
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
**
** mkxp is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** mkxp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with mkxp.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SHAREDMIDISTATE_H
#define SHAREDMIDISTATE_H

#include "config.h"
#include "debugwriter.h"
#include "fluid-fun.h"
#ifdef MKXP_BUILD_ANDROID
#include "fluidstream.h"
#endif

#include <assert.h>
#include <string>
#include <vector>

#define SYNTH_INIT_COUNT 2
#define SYNTH_SAMPLERATE 44100

struct Synth
{
    fluid_synth_t* synth;
    bool inUse;
};

struct SharedMidiState
{
    bool inited;
    std::vector<Synth> synths;
    const std::string& soundFont;
    fluid_settings_t* flSettings;

    SharedMidiState(const Config& conf): inited(false), soundFont(conf.midi.soundFont) {}

    ~SharedMidiState()
    {
        /* We might have initialized, but if the consecutive libfluidsynth
         * load failed, no resources will have been allocated */
        if (!inited || !HAVE_FLUID) return;

        fluid.delete_settings(flSettings);

        for (size_t i = 0; i < synths.size(); ++i)
        {
            assert(!synths[i].inUse);
            fluid.delete_synth(synths[i].synth);
        }
    }

    void initIfNeeded(const Config& conf)
    {
        if (inited) return;

        inited = true;

        initFluidFunctions();

        if (!HAVE_FLUID) return;

        flSettings = fluid.new_settings();
        fluid.settings_setnum(flSettings, "synth.gain", 1.0f);
        fluid.settings_setnum(flSettings, "synth.sample-rate", SYNTH_SAMPLERATE);
        fluid.settings_setint(flSettings, "synth.chorus.active", conf.midi.chorus);
        fluid.settings_setint(flSettings, "synth.reverb.active", conf.midi.reverb);
#ifdef MKXP_BUILD_ANDROID
        fluid.settings_setstr(flSettings, "audio.driver", "opensles");
#endif

        for (size_t i = 0; i < SYNTH_INIT_COUNT; ++i)
            addSynth(false);
    }

    fluid_synth_t* allocateSynth()
    {
        assert(HAVE_FLUID);
        assert(inited);

        size_t i;

        for (i = 0; i < synths.size(); ++i)
            if (!synths[i].inUse) break;

        if (i < synths.size())
        {
            fluid_synth_t* syn = synths[i].synth;
            fluid.synth_system_reset(syn);
            synths[i].inUse = true;

            return syn;
        }
        else { return addSynth(true); }
    }

    void releaseSynth(fluid_synth_t* synth)
    {
        size_t i;

        for (i = 0; i < synths.size(); ++i)
            if (synths[i].synth == synth) break;

        assert(i < synths.size());

        synths[i].inUse = false;
    }

  private:
    fluid_synth_t* addSynth(bool usedNow)
    {
        fluid_synth_t* syn = fluid.new_synth(flSettings);

        if (!soundFont.empty())
        {
#ifdef MKXP_BUILD_ANDROID
            fluid_sfloader_t* loader = fluid.new_defsfloader(flSettings);
            fluid.sfloader_set_callbacks(loader, fluid_sf_open, fluid_sf_read, fluid_sf_seek, fluid_sf_tell,
                                         fluid_sf_close);

            fluid.synth_add_sfloader(syn, loader);
#endif
            fluid.synth_sfload(syn, soundFont.c_str(), 1);
        }
        else
            Debug() << "Warning: No soundfont specified, sound might be mute";

        Synth synth;
        synth.inUse = usedNow;
        synth.synth = syn;
        synths.push_back(synth);

        return syn;
    }
};

#endif // SHAREDMIDISTATE_H
