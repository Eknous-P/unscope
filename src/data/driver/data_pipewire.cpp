/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

unscope is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

unscope is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
unscope. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "data_pipewire.h"
#include "audio_common.h"
#include <param.h>
#include <pipewire/keys.h>
#include <pipewire/properties.h>

const DataDriverInfo DataPipeWire::getDriverInfo() const {
  return {
    DATA_PIPEWIRE,
    DRIVERFLAG_OUTPUT|DRIVERFLAG_INPUT|DRIVERFLAG_PAUSE,
    "PipeWire Driver"
  };
}

void DataPipeWire::pwProcessCallback(void* userdata) {
  pwCallbackData* data = (pwCallbackData*)userdata;

  struct pw_buffer* pwBuffer;
  struct spa_buffer* spaBuffer;

  pwBuffer = pw_stream_dequeue_buffer(data->stream);
  if (pwBuffer == NULL) {
    pw_log_warn("PipeWire: out of buffers: %m");
  }

  spaBuffer = pwBuffer->buffer;

  float* samples = (float*)spaBuffer->datas[0].data;
  if (samples == NULL) return;

  const int chans = data->parent->getBufferCount();
  const size_t nSamples = spaBuffer->datas[0].chunk->size / sizeof(float);

  for (size_t i=0; i<nSamples;) {
    for (size_t j=0; j<chans; j++,i++) {
      data->parent->getBuffer(j)->write(samples++);
    }
  }

  pw_stream_queue_buffer(data->stream, pwBuffer);
}

void DataPipeWire::pwHandleExitSignal(void* userdata, int signal) {
  pwCallbackData* data = (pwCallbackData*)userdata;
  pw_stream_destroy(data->stream);
  pw_thread_loop_destroy(data->pwLoop);
  pw_log_error("PipeWire: thread loop terminated from signal %d", signal);
}

int DataPipeWire::setup(USCData* p) {
  parent = p;
  state = 0;

  buffers = {};

  deviceNum = 0;
  sampleRateNum = 5;
  channels = 2;

  config = {
    // Parameter(PARAM_COMBOV_STR, false, "device", "device", NULL, &devices, &deviceNum),
    Parameter(PARAM_COMBO_INT, false, "sampleRate", "sample rate", NULL, (void*)sampleRates, &sampleRateNum),
    Parameter(PARAM_INPUTINT, false, "channels", "channels", NULL, (void*)paramChannelsLimits, &channels)
  };

  for (int i=0; i<16; i++) {
    buffers.push_back(new DataBuffer_Float);
  }

  // and the nonsense begins...
  spaBuilder = SPA_POD_BUILDER_INIT(builderBuf, sizeof(builderBuf));

  pw_init(NULL,NULL);

  callbackData.parent = this;
  // do i need this?
  // pw_loop_add_signal(pw_thread_loop_get_loop(callbackData.pwLoop), SIGINT, pwHandleExitSignal, &callbackData);
  // pw_loop_add_signal(pw_thread_loop_get_loop(callbackData.pwLoop), SIGTERM, pwHandleExitSignal, &callbackData);

  e = 0;
  state |= DRIVERSTATE_OK|DRIVERSTATE_READY;
  return 0;
}

#define IS_PROP_KEY(k) (strncmp(props->items[i].key,k,sizeof(k))==0)

void DataPipeWire::registryCallback(void* data, uint32_t id, uint32_t permissions, const char* type, uint32_t version, const struct spa_dict* props) {
  // printf(INFO_MSG "PipeWire: object: id:%u type: %s/%d" MSG_END, id, type, version);
  deviceEnumCallbackData* devData = (deviceEnumCallbackData*)data;
  vector<string>* devices = devData->first;
  vector<pwDevice>* devicesInternal = devData->second;
  pwDevice potentialDevice;
  bool consider=false;
  for (int i=0; i<props->n_items; i++) {
    // printf("\t %s:%s\n", props->items[i].key, props->items[i].value);
    // TODO: prob replace with pw_properties_get ?
    if (IS_PROP_KEY(PW_KEY_NODE_NAME)) potentialDevice.name = props->items[i].value;
    if (IS_PROP_KEY("node.description")) potentialDevice.desc = props->items[i].value;
    if (
      strncmp(props->items[i].key,"media.class",12)==0 &&
     (strncmp(props->items[i].value, "Audio/Sink", 11)==0 ||
      strncmp(props->items[i].value, "Audio/Source", 13)==0)) consider=true;
  }
  // potentialDevice.props.push_back(NULL);
  if (consider) {
    devicesInternal->push_back(potentialDevice);
    string devName = potentialDevice.desc;
    devices->push_back(devName);
    printf(INFO_MSG "PipeWire: added device %s " MSG_END, devName.c_str());
  }
}

int DataPipeWire::enumerateDevices() { /*
  static const struct pw_registry_events pwRegistryEvents = {
    .version = PW_VERSION_REGISTRY_EVENTS,
    .global = registryCallback,
  };
  struct pw_loop* pwLoop;
  struct pw_context* pwContext;
  struct pw_core* pwCore;
  struct pw_registry* pwRegistry;
  struct spa_hook pwRegistryListener;

  pwLoop = pw_loop_new(NULL);
  pwContext = pw_context_new(pwLoop, NULL, 0);
  pwCore = pw_context_connect(pwContext, NULL, 0);
  pwRegistry = pw_core_get_registry(pwCore, PW_VERSION_REGISTRY, 0);
  spa_zero(pwRegistryListener);
  devices.clear();
  devicesInternal.clear();
  devicesVectorsPointers.first=&devices;
  devicesVectorsPointers.second=&devicesInternal;
  pw_registry_add_listener(pwRegistry, &pwRegistryListener, &pwRegistryEvents, &devicesVectorsPointers);
  for (int i=0; i<100; i++)
    pw_loop_iterate(pwLoop,100);

  pw_proxy_destroy((struct pw_proxy*)pwRegistry);
  pw_core_disconnect(pwCore);
  pw_context_destroy(pwContext);
  pw_loop_destroy(pwLoop);*/
  return 0;
}

void DataPipeWire::activate() {
  static const struct pw_stream_events pwStreamEvents = {
    .version = PW_VERSION_STREAM_EVENTS,
    // .param_changed = on_stream_param_changed,
    .process = pwProcessCallback,
  };
  for (DataBuffer* buf:buffers) {
    buf->destroy();
    delete buf;
  }
  buffers.clear();

  char strbuf[256];
  for (int i=0; i<16; i++)
    buffers[i]->destroy();
  for (int i=0; i<2; i++) {
    snprintf(strbuf, 256, "PipeWire Buffer %d", i+1);
    buffers[i]->init(65536, sampleRates[sampleRateNum+1], strbuf);
  }

  pwProps = pw_properties_new(
    PW_KEY_MEDIA_TYPE, "Audio",
    PW_KEY_MEDIA_CATEGORY, "Capture",
    PW_KEY_MEDIA_ROLE, "DSP",
    PW_KEY_STREAM_CAPTURE_SINK, "true",
    // PW_KEY_NODE_NAME, devicesInternal[deviceNum].name.c_str(),
    NULL
  );

  callbackData.pwLoop = pw_thread_loop_new(PROGRAM_NAME "-audioThread", NULL);
  callbackData.stream = pw_stream_new_simple(
    pw_thread_loop_get_loop(callbackData.pwLoop),
    PROGRAM_NAME "-pw-capture",
    pwProps,
    &pwStreamEvents,
    &callbackData
  );

  spa_audio_info_raw rawInfo = {
    .format = SPA_AUDIO_FORMAT_F32,
  };
  rawInfo.rate = sampleRates[sampleRateNum+1],
  rawInfo.channels = channels;

  streamParams[0] = spa_format_audio_raw_build(&spaBuilder, SPA_PARAM_EnumFormat, &rawInfo);

  e = pw_stream_connect(
    callbackData.stream, 
    PW_DIRECTION_INPUT,
    PW_ID_ANY,
    (pw_stream_flags)(PW_STREAM_FLAG_AUTOCONNECT|PW_STREAM_FLAG_MAP_BUFFERS|PW_STREAM_FLAG_RT_PROCESS),
    (const spa_pod**)streamParams, 1); // shut up already

  if (e != 0) {
    printf(ERROR_MSG "PipeWire: FUCK!" MSG_END);
    state|=DRIVERSTATE_ERROR;
    return;
  }
  state|=DRIVERSTATE_ACTIVE;
}

void DataPipeWire::doPlay(bool play) {
  if (!(state&DRIVERSTATE_ACTIVE)) return;
  if (play) {
    e = pw_thread_loop_start(callbackData.pwLoop);
    if (e != 0) {
      printf(ERROR_MSG "PipeWire: oh fuck (%d)" MSG_END, e);
    }
    state |= DRIVERSTATE_PLAY;
  } else {
    pw_thread_loop_stop(callbackData.pwLoop);
    state &=~DRIVERSTATE_PLAY;
  }
}

void DataPipeWire::deactivate() {
  if (!(state&DRIVERSTATE_ACTIVE)) return;
  doPlay(false);
  pw_stream_destroy(callbackData.stream);
  pw_thread_loop_destroy(callbackData.pwLoop);
  state&=~DRIVERSTATE_ACTIVE;
}

void DataPipeWire::destroy() {
  deactivate();
  pw_deinit();
  for (Parameter& i:config) i.destroy();
  config.clear();
  for (int i=0; i<16; i++) buffers[i]->destroy();
  buffers.clear();
}
