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
    void createServer(){server = new InspectorWidgetProcessor();}    

    private:
    explicit InspectorWidgetProcessorWrapper();
    ~InspectorWidgetProcessorWrapper();

    static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void Run(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void Abort(const Nan::FunctionCallbackInfo<v8::Value>& info);    
    static void Status(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void AnnotationStatus(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void AccessibilityUnderMouse(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static Nan::Persistent<v8::Function> constructor;
    InspectorWidgetProcessor* server;
};

#endif
