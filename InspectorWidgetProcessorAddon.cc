/**
 * @file InspectorWidgetProcessorAddon.cc
 * @brief Node.js addon definition for InspectorWidgetProcessor
 * @author Christian Frisson
 */

#include <nan.h>
#include "InspectorWidgetProcessorWrapper.h"

void InitAll(v8::Local<v8::Object> exports) {
  InspectorWidgetProcessorWrapper::Init(exports);
}

NODE_MODULE(addon, InitAll)
