/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_AUDIO_OREO_COMPAT_H
#define ANDROID_HARDWARE_AUDIO_OREO_COMPAT_H

// Compatibility views of the Android 8.1 (Oreo) audio HAL ABI.
//
// The vendor primary HAL on this device (audio.primary.universal7870_go.so)
// was built against the Android 8.1 headers, while this wrapper is compiled
// against the Android 10 headers. Between the two releases:
//   - audio_hw_device gained get_microphones between close_input_stream and
//     dump, shifting every field from dump to set_audio_port_config by one
//     pointer,
//   - audio_stream_in gained four trailing fields,
//   - audio_stream_out gained one trailing field,
//   - audio_port_config gained the framework-only 'flags' field before 'ext'.
// The stream structs only gained *trailing* fields, so the wrapper skips the
// Q-only calls for legacy HALs instead of needing a shadow type. For the
// device and port structs the layout actually shifted, so shadow types
// mirroring the 8.1 layout are provided below. All of this only matters on
// 32-bit targets, where the Q 'flags' insertion shifts 'ext' by 4 bytes.

#include <hardware/audio.h>

#include <stddef.h>
#include <string.h>

namespace oreo {

struct audio_port_config_oreo {
    audio_port_handle_t      id;           /* port unique ID */
    audio_port_role_t        role;         /* sink or source */
    audio_port_type_t        type;         /* device, mix ... */
    unsigned int             config_mask;  /* e.g AUDIO_PORT_CONFIG_ALL */
    unsigned int             sample_rate;  /* sampling rate in Hz */
    audio_channel_mask_t     channel_mask; /* channel mask if applicable */
    audio_format_t           format;       /* format if applicable */
    struct audio_gain_config gain;         /* gain to apply if applicable */
    union {
        struct audio_port_config_device_ext  device;  /* device specific info */
        struct audio_port_config_mix_ext     mix;     /* mix specific info */
        struct audio_port_config_session_ext session; /* session specific info */
    } ext;
};

struct audio_port_oreo {
    audio_port_handle_t      id;                /* port unique ID */
    audio_port_role_t        role;              /* sink or source */
    audio_port_type_t        type;              /* device, mix ... */
    char                     name[AUDIO_PORT_MAX_NAME_LEN];
    unsigned int             num_sample_rates;  /* number of sampling rates in following array */
    unsigned int             sample_rates[AUDIO_PORT_MAX_SAMPLING_RATES];
    unsigned int             num_channel_masks; /* number of channel masks in following array */
    audio_channel_mask_t     channel_masks[AUDIO_PORT_MAX_CHANNEL_MASKS];
    unsigned int             num_formats;       /* number of formats in following array */
    audio_format_t           formats[AUDIO_PORT_MAX_FORMATS];
    unsigned int             num_gains;         /* number of gains in following array */
    struct audio_gain        gains[AUDIO_PORT_MAX_GAINS];
    struct audio_port_config_oreo active_config;     /* current audio port configuration */
    union {
        struct audio_port_device_ext  device;
        struct audio_port_mix_ext     mix;
        struct audio_port_session_ext session;
    } ext;
};

struct audio_hw_device_oreo {
    /**
     * Common methods of the audio device.  This *must* be the first member of audio_hw_device
     * as users of this structure will cast a hw_device_t to audio_hw_device pointer in contexts
     * where it's known the hw_device_t references an audio_hw_device.
     */
    struct hw_device_t common;

    /**
     * used by audio flinger to enumerate what devices are supported by
     * each audio_hw_device implementation.
     *
     * Return value is a bitmask of 1 or more values of audio_devices_t
     *
     * NOTE: audio HAL implementations starting with
     * AUDIO_DEVICE_API_VERSION_2_0 do not implement this function.
     * All supported devices should be listed in audio_policy.conf
     * file and the audio policy manager must choose the appropriate
     * audio module based on information in this file.
     */
    uint32_t (*get_supported_devices)(const struct audio_hw_device_oreo *dev);

    /**
     * check to see if the audio hardware interface has been initialized.
     * returns 0 on success, -ENODEV on failure.
     */
    int (*init_check)(const struct audio_hw_device_oreo *dev);

    /** set the audio volume of a voice call. Range is between 0.0 and 1.0 */
    int (*set_voice_volume)(struct audio_hw_device_oreo *dev, float volume);

    /**
     * set the audio volume for all audio activities other than voice call.
     * Range between 0.0 and 1.0. If any value other than 0 is returned,
     * the software mixer will emulate this capability.
     */
    int (*set_master_volume)(struct audio_hw_device_oreo *dev, float volume);

    /**
     * Get the current master volume value for the HAL, if the HAL supports
     * master volume control.  AudioFlinger will query this value from the
     * primary audio HAL when the service starts and use the value for setting
     * the initial master volume across all HALs.  HALs which do not support
     * this method may leave it set to NULL.
     */
    int (*get_master_volume)(struct audio_hw_device_oreo *dev, float *volume);

    /**
     * set_mode is called when the audio mode changes. AUDIO_MODE_NORMAL mode
     * is for standard audio playback, AUDIO_MODE_RINGTONE when a ringtone is
     * playing, and AUDIO_MODE_IN_CALL when a call is in progress.
     */
    int (*set_mode)(struct audio_hw_device_oreo *dev, audio_mode_t mode);

    /* mic mute */
    int (*set_mic_mute)(struct audio_hw_device_oreo *dev, bool state);
    int (*get_mic_mute)(const struct audio_hw_device_oreo *dev, bool *state);

    /* set/get global audio parameters */
    int (*set_parameters)(struct audio_hw_device_oreo *dev, const char *kv_pairs);

    /*
     * Returns a pointer to a heap allocated string. The caller is responsible
     * for freeing the memory for it using free().
     */
    char * (*get_parameters)(const struct audio_hw_device_oreo *dev,
                             const char *keys);

    /* Returns audio input buffer size according to parameters passed or
     * 0 if one of the parameters is not supported.
     * See also get_buffer_size which is for a particular stream.
     */
    size_t (*get_input_buffer_size)(const struct audio_hw_device_oreo *dev,
                                    const struct audio_config *config);

    /** This method creates and opens the audio hardware output stream.
     * The "address" parameter qualifies the "devices" audio device type if needed.
     * The format format depends on the device type:
     * - Bluetooth devices use the MAC address of the device in the form "00:11:22:AA:BB:CC"
     * - USB devices use the ALSA card and device numbers in the form  "card=X;device=Y"
     * - Other devices may use a number or any other string.
     */

    int (*open_output_stream)(struct audio_hw_device_oreo *dev,
                              audio_io_handle_t handle,
                              audio_devices_t devices,
                              audio_output_flags_t flags,
                              struct audio_config *config,
                              struct audio_stream_out **stream_out,
                              const char *address);

    void (*close_output_stream)(struct audio_hw_device_oreo *dev,
                                struct audio_stream_out* stream_out);

    /** This method creates and opens the audio hardware input stream */
    int (*open_input_stream)(struct audio_hw_device_oreo *dev,
                             audio_io_handle_t handle,
                             audio_devices_t devices,
                             struct audio_config *config,
                             struct audio_stream_in **stream_in,
                             audio_input_flags_t flags,
                             const char *address,
                             audio_source_t source);

    void (*close_input_stream)(struct audio_hw_device_oreo *dev,
                               struct audio_stream_in *stream_in);

    /** This method dumps the state of the audio hardware */
    int (*dump)(const struct audio_hw_device_oreo *dev, int fd);

    /**
     * set the audio mute status for all audio activities.  If any value other
     * than 0 is returned, the software mixer will emulate this capability.
     */
    int (*set_master_mute)(struct audio_hw_device_oreo *dev, bool mute);

    /**
     * Get the current master mute status for the HAL, if the HAL supports
     * master mute control.  AudioFlinger will query this value from the primary
     * audio HAL when the service starts and use the value for setting the
     * initial master mute across all HALs.  HALs which do not support this
     * method may leave it set to NULL.
     */
    int (*get_master_mute)(struct audio_hw_device_oreo *dev, bool *mute);

    /**
     * Routing control
     */

    /* Creates an audio patch between several source and sink ports.
     * The handle is allocated by the HAL and should be unique for this
     * audio HAL module. */
    int (*create_audio_patch)(struct audio_hw_device_oreo *dev,
                               unsigned int num_sources,
                               const struct audio_port_config_oreo *sources,
                               unsigned int num_sinks,
                               const struct audio_port_config_oreo *sinks,
                               audio_patch_handle_t *handle);

    /* Release an audio patch */
    int (*release_audio_patch)(struct audio_hw_device_oreo *dev,
                               audio_patch_handle_t handle);

    /* Fills the list of supported attributes for a given audio port.
     * As input, "port" contains the information (type, role, address etc...)
     * needed by the HAL to identify the port.
     * As output, "port" contains possible attributes (sampling rates, formats,
     * channel masks, gain controllers...) for this port.
     */
    int (*get_audio_port)(struct audio_hw_device_oreo *dev,
                          struct audio_port_oreo *port);

    /* Set audio port configuration */
    int (*set_audio_port_config)(struct audio_hw_device_oreo *dev,
                         const struct audio_port_config_oreo *config);

};
// Size of the audio_port_config prefix shared by 8.1 and Q (everything up to
// Q's framework-only 'flags' field, which sits where 8.1 starts 'ext').
constexpr size_t portConfigPrefixSize() {
#ifndef AUDIO_NO_SYSTEM_DECLARATIONS
    return offsetof(audio_port_config, flags);
#else
    return offsetof(audio_port_config, ext);
#endif
}

static_assert(portConfigPrefixSize() == offsetof(audio_port_config_oreo, ext),
              "8.1 and Q audio_port_config share their prefix layout");
static_assert(offsetof(audio_port, active_config) == offsetof(audio_port_oreo, active_config),
              "8.1 and Q audio_port share their prefix layout");

// Reinterpret a Q audio_hw_device_t as the 8.1 layout.
inline audio_hw_device_oreo* dev(audio_hw_device_t* device) {
    return reinterpret_cast<audio_hw_device_oreo*>(device);
}

inline void portConfigToOreo(const audio_port_config& src, audio_port_config_oreo* dst) {
    memset(dst, 0, sizeof(*dst));
    memcpy(dst, &src, portConfigPrefixSize());
    memcpy(&dst->ext, &src.ext, sizeof(dst->ext) < sizeof(src.ext) ? sizeof(dst->ext)
                                                                   : sizeof(src.ext));
}

inline void portConfigFromOreo(const audio_port_config_oreo& src, audio_port_config* dst) {
    memset(dst, 0, sizeof(*dst));
    memcpy(dst, &src, portConfigPrefixSize());
    memcpy(&dst->ext, &src.ext, sizeof(dst->ext) < sizeof(src.ext) ? sizeof(dst->ext)
                                                                   : sizeof(src.ext));
}

inline void portToOreo(const audio_port& src, audio_port_oreo* dst) {
    memset(dst, 0, sizeof(*dst));
    memcpy(dst, &src, offsetof(audio_port, active_config));
    portConfigToOreo(src.active_config, &dst->active_config);
    memcpy(&dst->ext, &src.ext, sizeof(dst->ext) < sizeof(src.ext) ? sizeof(dst->ext)
                                                                   : sizeof(src.ext));
}

inline void portFromOreo(const audio_port_oreo& src, audio_port* dst) {
    memset(dst, 0, sizeof(*dst));
    memcpy(dst, &src, offsetof(audio_port, active_config));
    portConfigFromOreo(src.active_config, &dst->active_config);
    memcpy(&dst->ext, &src.ext, sizeof(dst->ext) < sizeof(src.ext) ? sizeof(dst->ext)
                                                                   : sizeof(src.ext));
}

}  // namespace oreo

#endif  // ANDROID_HARDWARE_AUDIO_OREO_COMPAT_H
