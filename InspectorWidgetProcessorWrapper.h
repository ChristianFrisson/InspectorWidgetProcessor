/**
 * @file InspectorWidgetProcessorWrapper.h
 * @brief Node.js wrapper for InspectorWidgetProcessor
 * @author Christian Frisson
 */

#ifndef InspectorWidgetProcessorWrapper_H
#define InspectorWidgetProcessorWrapper_H

#include <InspectorWidgetProcessor.h>

#include <nan.h>

class InspectorWidgetProcessorWrapper : public Nan::ObjectWrap {
public:
  static void Init(v8::Local<v8::Object> exports);
  InspectorWidgetProcessor* getServer(){return server;}

private:
  explicit InspectorWidgetProcessorWrapper();
  ~InspectorWidgetProcessorWrapper();

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Run(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static Nan::Persistent<v8::Function> constructor;
  InspectorWidgetProcessor* server;
};

#endif
