#pragma once
/*
 *  File: synth.h
 *
 *  synth unit for braids
 *
 */

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstdio>

#include <algorithm>

#include <arm_neon.h>

#include "unit_osc.h"   // Note: Include base definitions for osc units

#include "stmlib/utils/dsp.h"
#include "macro_oscillator.h"
//#include "braids/signature_waveshaper.h"
//#include "braids/vco_jitter_source.h"

using namespace stmlib;
enum Params {
    Param1 = 0,
    Param2,
    Shape,
    ModTarget,
    ModDelay,
    FMAmount,
    Pitch,
    Resolution,
    SampleRate,
    PARAMCOUNT,
};

const uint16_t bit_reduction_masks[] = {
    0xc000,
    0xe000,
    0xf000,
    0xf800,
    0xff00,
    0xfff0,
    0xffff
};

const uint16_t decimation_factors[] = { 12, 8, 6, 3, 2, 1 };

class Osc {
public:
    Osc(void) {}
    ~Osc(void) {}

    inline int8_t Init(const unit_runtime_desc_t * desc) {
        if (!desc)
            return k_unit_err_undef;

        if (desc->target != unit_header.target)
            return k_unit_err_target;

        if (!UNIT_API_IS_COMPAT(desc->api))
            return k_unit_err_api_version;

        if (desc->samplerate != 48000)
            return k_unit_err_samplerate;

        if (desc->input_channels != 2 || desc->output_channels != 1)  // should be stereo input / mono output
            return k_unit_err_geometry;

        runtime_desc_ = *desc;

        std::memset(&osc_, 0, sizeof(osc_));
        osc_.Init();
//        ws_.Init(0x42636877U); // in the original src, MPU's unique id is used 
//        jitter_source_.Init();

#ifdef LILYVA
        p_[Shape] = 0;
#endif
#ifdef LILYFM
        p_[Shape] = 16;
#endif
#ifdef LILYRS
        p_[Shape] = 28;
#endif
#ifdef LILYWT
        p_[Shape] = 37;
#endif
#ifdef LILYNZ
        p_[Shape] = 41;
#endif
        osc_.set_shape(static_cast<braids::MacroOscillatorShape>(p_[Shape]));

        p_[Resolution] = 6;
        p_[SampleRate] = 5;
        lfoz_ = 0;
        lfo_timbre_ = 1;
        lfo_color_ = 0;
        lfo_fm_amount_ = 0;

        started_ = false;

        return k_unit_err_none;
    }

    inline void Teardown() {}

    inline void Reset() {
        gate_ = 0;
    }

    inline void Resume() {}

    inline void Suspend() {}

    fast_inline void Process(const float * in, float * out, size_t frames) {
        // This started_ flag is for avoiding the firmware bug
        started_ = true;
        const float * __restrict in_p = in;
        float * __restrict out_p = out;
        const unit_runtime_osc_context_t *ctxt = static_cast<const unit_runtime_osc_context_t *>(runtime_desc_.hooks.runtime_context);
        uint16_t lfo = ctxt->shape_lfo >> 16;
        uint16_t lfoz = lfoz_;
        const uint16_t lfo_inc = (lfo - lfoz) / frames;
        uint32_t lfo_count = lfo_count_;
        const int bufsize = 24; // size of temp_buffer in macro_oscillator.h

        int16_t buf[bufsize] = {};
        const uint8_t sync[bufsize] = {};
        size_t decimation_factor = decimation_factors[p_[SampleRate]];
        uint16_t bit_mask = bit_reduction_masks[p_[Resolution]];
//        uint16_t signature = p_[Signature] * p_[Signature] * 4095;
        static uint32_t n = 0;
        static int16_t current_sample = 0;
        for(uint32_t p = 0; p < frames; p += bufsize) {
            uint32_t lfo_delayed = (lfoz << 16) / ((1 << 10) + lfo_count) >> 6;
            if (lfo_count) {
                lfo_count--;
            }

            // Set timbre and color: parameter value + LFO modulation.
            int32_t timbre = timbre_;
            timbre += lfo_delayed * lfo_timbre_;
            CONSTRAIN(timbre, 0, 32767);

            int32_t color = color_;
            color += lfo_delayed * lfo_color_;
            CONSTRAIN(color, 0, 32767);
            osc_.set_parameters(timbre, color);

            // Set pitch: parameter value + audio_in * (parameter value + LFO).
            const unit_runtime_osc_context_t *ctxt = static_cast<const unit_runtime_osc_context_t *>(runtime_desc_.hooks.runtime_context);
            int32_t pitch = ctxt->pitch >> 1;
            int32_t fm = (p_[FMAmount] * 256) + lfo_fm_amount_ * lfo_delayed;
            pitch += (*in_p++ + *in_p++) * fm;
            pitch += p_[Pitch];

            if (pitch > 16383) {
                pitch = 16383;
            } else if (pitch < 0) {
                pitch = 0;
            }

            osc_.set_pitch(pitch);

            size_t r_size = (bufsize < (frames - p)) ? bufsize : frames - p;
            osc_.Render(sync, buf, r_size);

            
            // Copy to the buffer with sample rate and bit reduction applied.
            for(uint32_t i = 0; i < r_size ; i++, n++, out_p += 1) {
                if ((n % decimation_factor) == 0) {
                    current_sample = buf[i] & bit_mask;
                }
                *out_p = current_sample / 32768.f;
            }
            lfoz += lfo_inc;
        }
        lfoz_ = lfoz;
        lfo_count_ = lfo_count;
    }

    inline void setParameter(uint8_t index, int32_t value) {
        // This if statement is for avoiding the firmware bug
        if (!started_) { 
            return;
        }
        switch(index) {
        case Shape:
            CONSTRAIN(value, 0, 47);
            break;
        case Param1:
        case Param2:
            CONSTRAIN(value, -2560, 256);
            break;
        case ModTarget:
            CONSTRAIN(value, 0, 2);
            break;
        case ModDelay:
            CONSTRAIN(value, 0, 31);
            break;
        case FMAmount:
            CONSTRAIN(value, -127, 127);
            break;
        case Pitch:
            CONSTRAIN(value, -127, 127);
            break;
        case Resolution:
            CONSTRAIN(value, 0, 6);
            break;
        case SampleRate:
            CONSTRAIN(value, 0, 5);
            break;
        }
        switch (index) {
        case Shape:   // 0..47
#ifdef LILYNZ
            // for easter egg
            if (value == 47 && p_[Shape] != 47) {
                if (p_[Param1] == -256 &&
                    p_[Param2] == 255 &&
                    p_[FMAmount] == 123) {
                    value = 47;
                } else {
                    value = 46;
                }
            }
#endif
            osc_.set_shape(static_cast<braids::MacroOscillatorShape>(value));
            break;
        case Param1:  // -256..255
            // timbre and color must be 0..32767
            timbre_ = (value + 256) << 6;
            break;
        case Param2:
            color_ = (value + 256) << 6;
            break;
        case ModTarget:
            switch(value) {
            case 0:
            case 1:
                lfo_color_ = value;
                lfo_timbre_ = 1 - lfo_color_;
                lfo_fm_amount_ = 0;
                break;
            case 2:
                lfo_fm_amount_ = 1;
                lfo_color_ = 0;
                lfo_timbre_ = 0;
                break;
            default:
                lfo_color_ = 0;
                lfo_timbre_ = 0;
                lfo_fm_amount_ = 0;
            }
            break;
        case ModDelay:
            lfo_delay_ = runtime_desc_.samplerate / 240 * value; 
            break;
        default:
            break;
        }
        p_[index] = value;
    }

    inline int32_t getParameterValue(uint8_t index) const {
        return p_[index];
    }

    inline const char * getParameterStrValue(uint8_t index, int32_t value) const {
        (void)value;
        switch (index) {
        case Shape:
            if (value < 47) {
                return ShapeStr[value];
            } else if (p_[Shape] == 47) {
                return ShapeStr[47];
            } else {
                return ShapeStr[46];
            }

        case Resolution:
            if (value < 7) {
                return BitsStr[value];
            } else {
                return nullptr;
            }

        case SampleRate:
            if (value < 6) {
                return RateStr[value];
            } else {
                return nullptr;
            }
        case ModTarget:
            CONSTRAIN(value, 0, 2);
            return ModTargetStr[value];
/*
        case Signature:
            if (value < 5) {
                return IntensityStr[value];
            } else {
                return nullptr;
            }

        case VCO_Flatten:
            if (value < 2) {
                return OffOnStr[value];
            } else {
                return nullptr;
            }

        case VCO_Drift:
            if (value < 5) {
                return IntensityStr[value];
            } else {
                return nullptr;
            }
*/
        default:
            break;
        }
        return nullptr;
    }

    inline const uint8_t * getParameterBmpValue(uint8_t index,
                                              int32_t value) const {
        (void)value;
        switch (index) {
        default:
            break;
        }
        return nullptr;
    }

    inline void setTempo(uint32_t tempo) {
        // const float t = (tempo >> 16) + (tempo & 0xFFFF) /
        // static_cast<float>(0x10000);
        (void) tempo;
    }

    inline void tempo4ppqnTick(uint32_t counter) {
        (uint32_t)counter;
    }

    inline void NoteOn(uint8_t note, uint8_t velocity) {
        pitch_ = note << 7;
        GateOn(velocity);
    }

    inline void NoteOff(uint8_t note) {
        (void)note;
        GateOff();
    }

    inline void GateOn(uint8_t velocity) {
        gate_ += 1;
        osc_.Strike();
        lfo_count_ = lfo_delay_;
    }

    inline void GateOff() {
        if (gate_ > 0 ) {
            gate_ -= 1;
        }
    }

    inline void AllNoteOff() {}

    inline void PitchBend(uint16_t bend) { (void)bend; }

    inline void ChannelPressure(uint8_t pressure) { (void)pressure; }

    inline void AfterTouch(uint8_t note, uint8_t aftertouch) {
        (void)note;
        (void)aftertouch;
    }

private:
    int32_t p_[PARAMCOUNT];
    bool started_;

    unit_runtime_desc_t runtime_desc_;

    braids::MacroOscillator osc_;
//    braids::SignatureWaveshaper ws_;
//    braids::VcoJitterSource jitter_source_;

    int16_t pitch_;
    int16_t timbre_;
    int16_t color_;
    int16_t gate_;
    uint16_t lfoz_;
    uint16_t lfo_timbre_;
    uint16_t lfo_color_;
    uint16_t lfo_fm_amount_;
    uint32_t lfo_delay_;
    uint32_t lfo_count_;

    /* Private Methods. */
    /* Constants. */
    const char *ShapeStr[48] = {
#ifdef LILYVA
        "CSSAW",  // 0
        "Morph",
        "SawSqr",
        "Folded",
        "Combx2",
        "SqrSub",
        "SawSub",
        "SqrSync",
        "SawSync",
        "3xSaw",
        "3xSqr", // 10
        "3xTri",
        "3xSin",
        "RingSin",// 13
        "Swarm",
        "SawComb",
        "","","","","","","","","","",
        "","","","","","","","","","",
        "","","","","","","","","","",
        "","",
#endif
#ifdef LILYFM
        "","","","","","","","","","",
        "","","","","","",
        "Toy-Lofi",
        "PhzLPF",
        "PhzPkF",
        "PhzBPF",
        "PhzHPF", // 20
        "Vosim",
        "Vowel",
        "Vwl-FOF",
        "AddHarm",
        "PlainFM",
        "FB-FM",
        "ChaoticFM", // 27
        "","","","","","","","","","",
        "","","","","","","","","","",
#endif
#ifdef LILYRS
        "","","","","","","","","","",
        "","","","","","","","","","",
        "","","","","","","","",
        "Plucked",
        "Bowed",
        "Blown",   // 30
        "Fluted",
        "Bells",
        "MtlDrum",
        "808Kick",
        "Cymbal",
        "Snare",
        "","","","","","","","","","",
        "",
#endif
#ifdef LILYWT
        "","","","","","","","","","",
        "","","","","","","","","","",
        "","","","","","","","","","",
        "","","","","","","",
        "WaveTbl",
        "WaveMap",
        "WavLine",
        "WvTblx4", // 40
        "","","","","","","",
#endif
#ifdef LILYNZ
        "","","","","","","","","","",
        "","","","","","","","","","",
        "","","","","","","","","","",
        "","","","","","","","","","",
        "",
        "FltdNois",
        "TwinQ",
        "Clocked",
        "GrnlrCld",
        "PartclNz",
        "Modem",    // 46
        "Q-Mark",
#endif
    };

    const char *ModTargetStr[3] = {
        "TMBR",
        "COLR",
        "FMLV",
    };

    const char *BitsStr[7] = {
        " 2",
        " 3",
        " 4",
        " 6",
        " 8",
        "12",
        "16",
    };

    const char *RateStr[6] = {
        " 4K",
        " 6K",
        " 8K",
        "16K",
        "24K",
        "48K",
    };

};
