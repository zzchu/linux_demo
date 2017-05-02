/*
 A Minimal Capture Program
 
 This program opens an audio interface for capture, configures it for
 stereo, 16 bit, 44.1kHz, interleaved conventional read/write
 access. Then its reads a chunk of random data from it, and exits. It
 isn't meant to be a real program.
 
 From on Paul David's tutorial : http://equalarea.com/paul/alsa-audio.html
 
 Fixes rate and buffer problems
 
 sudo apt-get install libasound2-dev
 gcc -o alsa-record-example -lasound alsa-record-example.c && ./alsa-record-example hw:0
 */
#include <assert.h>
#include <stddef.h>  // for NULL
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define FUNC_GET_NUM_OF_DEVICE 0
#define FUNC_GET_DEVICE_NAME 1
#define FUNC_GET_DEVICE_NAME_FOR_AN_ENUM 2


namespace webrtc_adm_linux {
// This macro must be invoked in a header to declare a symbol table class.
#define LATE_BINDING_SYMBOL_TABLE_DECLARE_BEGIN(ClassName) \
enum {

// This macro must be invoked in the header declaration once for each symbol
// (recommended to use an X-Macro to avoid duplication).
// This macro defines an enum with names built from the symbols, which
// essentially creates a hash table in the compiler from symbol names to their
// indices in the symbol table class.
#define LATE_BINDING_SYMBOL_TABLE_DECLARE_ENTRY(ClassName, sym) \
ClassName##_SYMBOL_TABLE_INDEX_##sym,

// This macro completes the header declaration.
#define LATE_BINDING_SYMBOL_TABLE_DECLARE_END(ClassName) \
ClassName##_SYMBOL_TABLE_SIZE \
}; \
\
extern const char ClassName##_kDllName[]; \
extern const char *const \
ClassName##_kSymbolNames[ClassName##_SYMBOL_TABLE_SIZE]; \
\
typedef ::webrtc_adm_linux::LateBindingSymbolTable<ClassName##_SYMBOL_TABLE_SIZE, \
ClassName##_kDllName, \
ClassName##_kSymbolNames> \
ClassName;

// This macro must be invoked in a .cc file to define a previously-declared
// symbol table class.
#define LATE_BINDING_SYMBOL_TABLE_DEFINE_BEGIN(ClassName, dllName) \
const char ClassName##_kDllName[] = dllName; \
const char *const ClassName##_kSymbolNames[ClassName##_SYMBOL_TABLE_SIZE] = {

// This macro must be invoked in the .cc definition once for each symbol
// (recommended to use an X-Macro to avoid duplication).
// This would have to use the mangled name if we were to ever support C++
// symbols.
#define LATE_BINDING_SYMBOL_TABLE_DEFINE_ENTRY(ClassName, sym) \
#sym,

#define LATE_BINDING_SYMBOL_TABLE_DEFINE_END(ClassName) \
};

// Index of a given symbol in the given symbol table class.
#define LATESYM_INDEXOF(ClassName, sym) \
(ClassName##_SYMBOL_TABLE_INDEX_##sym)

// Returns a reference to the given late-binded symbol, with the correct type.
#define LATESYM_GET(ClassName, inst, sym) \
(( \
(inst)->GetSymbol(LATESYM_INDEXOF(ClassName, sym))))
}



namespace webrtc_adm_linux_alsa {
    
    // The ALSA symbols we need, as an X-Macro list.
    // This list must contain precisely every libasound function that is used in
    // alsasoundsystem.cc.
#define ALSA_SYMBOLS_LIST \
X(snd_device_name_free_hint) \
X(snd_device_name_get_hint) \
X(snd_device_name_hint) \
X(snd_pcm_avail_update) \
X(snd_pcm_close) \
X(snd_pcm_delay) \
X(snd_pcm_drop) \
X(snd_pcm_open) \
X(snd_pcm_prepare) \
X(snd_pcm_readi) \
X(snd_pcm_recover) \
X(snd_pcm_resume) \
X(snd_pcm_reset) \
X(snd_pcm_state) \
X(snd_pcm_set_params) \
X(snd_pcm_get_params) \
X(snd_pcm_start) \
X(snd_pcm_stream) \
X(snd_pcm_frames_to_bytes) \
X(snd_pcm_bytes_to_frames) \
X(snd_pcm_wait) \
X(snd_pcm_writei) \
X(snd_pcm_info_get_class) \
X(snd_pcm_info_get_subdevices_avail) \
X(snd_pcm_info_get_subdevice_name) \
X(snd_pcm_info_set_subdevice) \
X(snd_pcm_info_get_id) \
X(snd_pcm_info_set_device) \
X(snd_pcm_info_set_stream) \
X(snd_pcm_info_get_name) \
X(snd_pcm_info_get_subdevices_count) \
X(snd_pcm_info_sizeof) \
X(snd_pcm_hw_params) \
X(snd_pcm_hw_params_malloc) \
X(snd_pcm_hw_params_free) \
X(snd_pcm_hw_params_any) \
X(snd_pcm_hw_params_set_access) \
X(snd_pcm_hw_params_set_format) \
X(snd_pcm_hw_params_set_channels) \
X(snd_pcm_hw_params_set_rate_near) \
X(snd_pcm_hw_params_set_buffer_size_near) \
X(snd_card_next) \
X(snd_card_get_name) \
X(snd_config_update) \
X(snd_config_copy) \
X(snd_config_get_id) \
X(snd_ctl_open) \
X(snd_ctl_close) \
X(snd_ctl_card_info) \
X(snd_ctl_card_info_sizeof) \
X(snd_ctl_card_info_get_id) \
X(snd_ctl_card_info_get_name) \
X(snd_ctl_pcm_next_device) \
X(snd_ctl_pcm_info) \
X(snd_mixer_load) \
X(snd_mixer_free) \
X(snd_mixer_detach) \
X(snd_mixer_close) \
X(snd_mixer_open) \
X(snd_mixer_attach) \
X(snd_mixer_first_elem) \
X(snd_mixer_elem_next) \
X(snd_mixer_selem_get_name) \
X(snd_mixer_selem_is_active) \
X(snd_mixer_selem_register) \
X(snd_mixer_selem_set_playback_volume_all) \
X(snd_mixer_selem_get_playback_volume) \
X(snd_mixer_selem_has_playback_volume) \
X(snd_mixer_selem_get_playback_volume_range) \
X(snd_mixer_selem_has_playback_switch) \
X(snd_mixer_selem_get_playback_switch) \
X(snd_mixer_selem_set_playback_switch_all) \
X(snd_mixer_selem_has_capture_switch) \
X(snd_mixer_selem_get_capture_switch) \
X(snd_mixer_selem_set_capture_switch_all) \
X(snd_mixer_selem_has_capture_volume) \
X(snd_mixer_selem_set_capture_volume_all) \
X(snd_mixer_selem_get_capture_volume) \
X(snd_mixer_selem_get_capture_volume_range) \
X(snd_dlopen) \
X(snd_dlclose) \
X(snd_config) \
X(snd_config_search) \
X(snd_config_get_string) \
X(snd_config_search_definition) \
X(snd_config_get_type) \
X(snd_config_delete) \
X(snd_config_iterator_entry) \
X(snd_config_iterator_first) \
X(snd_config_iterator_next) \
X(snd_config_iterator_end) \
X(snd_config_delete_compound_members) \
X(snd_config_get_integer) \
X(snd_config_get_bool) \
X(snd_dlsym) \
X(snd_strerror) \
X(snd_lib_error) \
X(snd_lib_error_set_handler)
    
    LATE_BINDING_SYMBOL_TABLE_DECLARE_BEGIN(AlsaSymbolTable)
#define X(sym) \
LATE_BINDING_SYMBOL_TABLE_DECLARE_ENTRY(AlsaSymbolTable, sym)
    ALSA_SYMBOLS_LIST
#undef X
    LATE_BINDING_SYMBOL_TABLE_DECLARE_END(AlsaSymbolTable)
    
}  // namespace webrtc_adm_linux_alsa


#define LATE(sym) \
LATESYM_GET(webrtc_adm_linux_alsa::AlsaSymbolTable, &AlsaSymbolTable, sym)




int32_t AudioDeviceLinuxALSA::GetDevicesInfo(
                                             const int32_t function,
                                             const bool playback,
                                             const int32_t enumDeviceNo,
                                             char* enumDeviceName,
                                             const int32_t ednLen) const
{
    
    // Device enumeration based on libjingle implementation
    // by Tristan Schmelcher at Google Inc.
    
    const char *type = playback ? "Output" : "Input";
    // dmix and dsnoop are only for playback and capture, respectively, but ALSA
    // stupidly includes them in both lists.
    const char *ignorePrefix = playback ? "dsnoop:" : "dmix:" ;
    // (ALSA lists many more "devices" of questionable interest, but we show them
    // just in case the weird devices may actually be desirable for some
    // users/systems.)
    
    int err;
    int enumCount(0);
    bool keepSearching(true);
    
    // From Chromium issue 95797
    // Loop through the sound cards to get Alsa device hints.
    // Don't use snd_device_name_hint(-1,..) since there is a access violation
    // inside this ALSA API with libasound.so.2.0.0.
    int card = -1;
    while (!(LATE(snd_card_next)(&card)) && (card >= 0) && keepSearching) {
        void **hints;
        err = LATE(snd_device_name_hint)(card, "pcm", &hints);
        if (err != 0)
        {
#if 1
            printf("GetDevicesInfo - device name hint error: %s\n", LATE(snd_strerror)(err));
#else
            WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                         "GetDevicesInfo - device name hint error: %s",
                         LATE(snd_strerror)(err));
#endif

            return -1;
        }
        
        enumCount++; // default is 0
        if ((function == FUNC_GET_DEVICE_NAME ||
             function == FUNC_GET_DEVICE_NAME_FOR_AN_ENUM) && enumDeviceNo == 0)
        {
            strcpy(enumDeviceName, "default");
            
            err = LATE(snd_device_name_free_hint)(hints);
            if (err != 0)
            {
#if 1
                printf("GetDevicesInfo - device name free hint error: %s\n", LATE(snd_strerror)(err));
#else
                WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                             "GetDevicesInfo - device name free hint error: %s",
                             LATE(snd_strerror)(err));
#endif
            }
            
            return 0;
        }
        
        for (void **list = hints; *list != NULL; ++list)
        {
            char *actualType = LATE(snd_device_name_get_hint)(*list, "IOID");
            if (actualType)
            {   // NULL means it's both.
                bool wrongType = (strcmp(actualType, type) != 0);
                free(actualType);
                if (wrongType)
                {
                    // Wrong type of device (i.e., input vs. output).
                    continue;
                }
            }
            
            char *name = LATE(snd_device_name_get_hint)(*list, "NAME");
            if (!name)
            {
#if 1
                printf("Device has no name\n");
#else
                WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                             "Device has no name");
#endif
                // Skip it.
                continue;
            }
            
            // Now check if we actually want to show this device.
            if (strcmp(name, "default") != 0 &&
                strcmp(name, "null") != 0 &&
                strcmp(name, "pulse") != 0 &&
                strncmp(name, ignorePrefix, strlen(ignorePrefix)) != 0)
            {
                // Yes, we do.
                char *desc = LATE(snd_device_name_get_hint)(*list, "DESC");
                if (!desc)
                {
                    // Virtual devices don't necessarily have descriptions.
                    // Use their names instead.
                    desc = name;
                }
                
                if (FUNC_GET_NUM_OF_DEVICE == function)
                {
#if 1
                    printf("    Enum device %d - %s\n", enumCount, name);
#else
                    WEBRTC_TRACE(kTraceInfo, kTraceAudioDevice, _id,
                                 "    Enum device %d - %s", enumCount, name);
#endif
                    
                }
                if ((FUNC_GET_DEVICE_NAME == function) &&
                    (enumDeviceNo == enumCount))
                {
                    // We have found the enum device, copy the name to buffer.
                    strncpy(enumDeviceName, desc, ednLen);
                    enumDeviceName[ednLen-1] = '\0';
                    keepSearching = false;
                    // Replace '\n' with '-'.
                    char * pret = strchr(enumDeviceName, '\n'/*0xa*/); //LF
                    if (pret)
                        *pret = '-';
                }
                if ((FUNC_GET_DEVICE_NAME_FOR_AN_ENUM == function) &&
                    (enumDeviceNo == enumCount))
                {
                    // We have found the enum device, copy the name to buffer.
                    strncpy(enumDeviceName, name, ednLen);
                    enumDeviceName[ednLen-1] = '\0';
                    keepSearching = false;
                }
                
                if (keepSearching)
                    ++enumCount;
                
                if (desc != name)
                    free(desc);
            }
            
            free(name);
            
            if (!keepSearching)
                break;
        }
        
        err = LATE(snd_device_name_free_hint)(hints);
        if (err != 0)
        {
#if 1
            printf("GetDevicesInfo - device name free hint error: %s\n",
                   LATE(snd_strerror)(err));
#else
            WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                         "GetDevicesInfo - device name free hint error: %s",
                         LATE(snd_strerror)(err));
#endif
            // Continue and return true anyway, since we did get the whole list.
        }
    }
    
    if (FUNC_GET_NUM_OF_DEVICE == function)
    {
        if (enumCount == 1) // only default?
            enumCount = 0;
        return enumCount; // Normal return point for function 0
    }
    
    if (keepSearching)
    {
        // If we get here for function 1 and 2, we didn't find the specified
        // enum device.
#if 1
        printf("GetDevicesInfo - Could not find device name or numbers");
#else
        WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                     "GetDevicesInfo - Could not find device name or numbers");
#endif
        return -1;
    }
    
    return 0;
}

main (int argc, char *argv[])
{
    int i;
    int err;
    char *buffer;
    int buffer_frames = 128;
    unsigned int buffer_size = 0;
    unsigned int rate = 44100;
    unsigned int channel_num = 2;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;//SND_PCM_FORMAT_U8;//SND_PCM_FORMAT_S16_LE;
    
    char* pFilename = "capture_audio.pcm";
    FILE *pFile = fopen(pFilename, "wb");
    
    if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
                 argv[1],
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "audio interface opened\n");
    
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params allocated\n");
    
    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params initialized\n");
    
    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params access setted\n");
    
    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params format setted\n");
    
    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params rate setted\n");
    
    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, channel_num)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params channels setted\n");
    
    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params setted\n");
    
    snd_pcm_hw_params_free (hw_params);
    
    fprintf(stdout, "hw_params freed\n");
    
    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "audio interface prepared\n");
    buffer_size = buffer_frames * snd_pcm_format_width(format) / 8 * channel_num;
    buffer = (char *)malloc(buffer_size);
    
    fprintf(stdout, "buffer allocated\n");
    
    for (i = 0; i < 5000; ++i) {
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
            fprintf (stderr, "read from audio interface failed (%s)\n",
                     err, snd_strerror (err));
            exit (1);
        }
        else
        {
            fwrite(buffer, 1, buffer_size, pFile);
            //printf("Wrote %s\n", pFilename);
        }
        //fprintf(stdout, "read %d done\n", i);
    }
    
    free(buffer);
    
    fprintf(stdout, "buffer freed\n");
    
    snd_pcm_close (capture_handle);
    fprintf(stdout, "audio interface closed\n");
    
    fclose(pFile);
    exit (0);
}
