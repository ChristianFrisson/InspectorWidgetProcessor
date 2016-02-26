/**
 * @file InspectorWidgetProcessorWrapper.cc
 * @brief Node.js wrapper for InspectorWidgetProcessor
 * @author Christian Frisson
 */

#include "InspectorWidgetProcessorWrapper.h"
#include <vector>

class InspectorWidgetProcessorAsyncWorker : public Nan::AsyncWorker {
public:
  InspectorWidgetProcessorAsyncWorker(Nan::Callback *callback, InspectorWidgetProcessor* server, std::vector<std::string> args)
    : Nan::AsyncWorker(callback), server(server), args(args),estimate(0) {}
  ~InspectorWidgetProcessorAsyncWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
    if(server){
        server->init(args);
      }
    //estimate = Estimate(points);
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    Nan::HandleScope scope;

    v8::Local<v8::Value> error = Nan::Null();
    v8::Local<v8::Value> success = Nan::Null();
    if(server){
        std::string error_string = server->getStatusError();
        std::string success_string = server->getStatusSuccess();
        error = Nan::New(error_string.c_str()).ToLocalChecked();
        success = Nan::New(success_string).ToLocalChecked();

        //std::vector<std::string> template_list = server->getTemplateList();
      }

    v8::Local<v8::Value> argv[] = {
      error /*Nan::Null()*/
      ,success /*Nan::Null()*/
    };

    callback->Call(2, argv);
  }

private:
  InspectorWidgetProcessor* server;
  std::vector<std::string> args;
  double estimate;
};

Nan::Persistent<v8::Function> InspectorWidgetProcessorWrapper::constructor;

InspectorWidgetProcessorWrapper::InspectorWidgetProcessorWrapper() {
  server = new InspectorWidgetProcessor();
}

InspectorWidgetProcessorWrapper::~InspectorWidgetProcessorWrapper() {
}

void InspectorWidgetProcessorWrapper::Init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("InspectorWidgetProcessor").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "run", Run);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("InspectorWidgetProcessor").ToLocalChecked(), tpl->GetFunction());
}

void InspectorWidgetProcessorWrapper::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.IsConstructCall()) {
      // Invoked as constructor: `new InspectorWidgetProcessorWrapper(...)`
      InspectorWidgetProcessorWrapper* obj = new InspectorWidgetProcessorWrapper();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      // Invoked as plain function `InspectorWidgetProcessorWrapper(...)`, turn into construct call.
      const int argc = 0;
      v8::Local<v8::Value> argv[argc] = { };
      v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
      info.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

void InspectorWidgetProcessorWrapper::Run(const Nan::FunctionCallbackInfo<v8::Value>& info) {

  int argc = info.Length();

  if (argc != 4) {
      Nan::ThrowTypeError("Wrong number of arguments");
      return;
    }

  for (int i=0; i<argc-1; i++){
      if(!info[i]->IsString()){
          std::stringstream error;
          error << "Argument " << i << " should be a string";
          Nan::ThrowTypeError(error.str().c_str());
          return;
        }
    }
  if(!info[argc-1]->IsFunction()){
      Nan::ThrowTypeError("The last argument should be a callback function");
      return;
    }

  std::vector<std::string> args;
  for (int i=0; i<argc-1; i++){
      v8::String::Utf8Value arg(info[i]);
      std::string buffer  = std::string(*arg);
      std::cout << "Buffer '" << buffer << "'" << std::endl;
      args.push_back(buffer);
    }

  Nan::Callback *callback = new Nan::Callback(info[argc-1].As<v8::Function>());

  //InspectorWidgetProcessorWrapper* obj = ObjectWrap::Unwrap<InspectorWidgetProcessorWrapper>(info.Holder());

  Nan::AsyncQueueWorker(new InspectorWidgetProcessorAsyncWorker(callback, /*obj->getServer()*/ new InspectorWidgetProcessor(), args ));


  /*bool success = obj->getServer()->init(args);

    if(!success){
        Nan::ThrowTypeError("Couldn't init the server.");
        return;
    }*/

  /*int points = info[0]->Uint32Value();
    double est = Estimate(points);
    info.GetReturnValue().Set(est);*/

  // return 0;

}
