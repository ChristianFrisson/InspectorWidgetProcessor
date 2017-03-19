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
        v8::Local<v8::Value> id = Nan::New(args[0]).ToLocalChecked();

        v8::Local<v8::Value> argv[] = {
            id,
            error /*Nan::Null()*/
            ,success /*Nan::Null()*/
        };

        callback->Call(3, argv);
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
    Nan::SetPrototypeMethod(tpl, "abort", Abort);
    Nan::SetPrototypeMethod(tpl, "status", Status);
    Nan::SetPrototypeMethod(tpl, "annotationStatus", AnnotationStatus);
    Nan::SetPrototypeMethod(tpl, "accessibilityHover", AccessibilityHover);
    Nan::SetPrototypeMethod(tpl, "extractTemplate", ExtractTemplate);

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
    std::cout << "new InspectorWidgetProcessor() " << std::endl;
    InspectorWidgetProcessorWrapper* obj = ObjectWrap::Unwrap<InspectorWidgetProcessorWrapper>(info.Holder());
    obj->createServer();
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

    InspectorWidgetProcessorWrapper* obj = ObjectWrap::Unwrap<InspectorWidgetProcessorWrapper>(info.Holder());

    Nan::AsyncQueueWorker(new InspectorWidgetProcessorAsyncWorker(callback, obj->getServer() /*new InspectorWidgetProcessor()*/, args ));

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

void InspectorWidgetProcessorWrapper::Abort(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    int argc = info.Length();
    if (argc != 1) {
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
    Nan::Callback *callback = new Nan::Callback(info[argc-1].As<v8::Function>());

    InspectorWidgetProcessorWrapper* obj = ObjectWrap::Unwrap<InspectorWidgetProcessorWrapper>(info.Holder());

    if(obj->getServer()){
        obj->getServer()->abort();
    }
}

void InspectorWidgetProcessorWrapper::Status(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    int argc = info.Length();
    if (argc != 2) {
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

    InspectorWidgetProcessorWrapper* obj = ObjectWrap::Unwrap<InspectorWidgetProcessorWrapper>(info.Holder());

    v8::Local<v8::Value> error = Nan::Null();
    v8::Local<v8::Value> success = Nan::Null();
    v8::Local<v8::Value> phase = Nan::Null();
    v8::Local<v8::Value> progress = Nan::Null();
    std::cout << "obj->getServer() " << obj->getServer() << std::endl;
    if(obj->getServer()){
        std::string error_string = obj->getServer()->getStatusError();
        std::string success_string = obj->getServer()->getStatusSuccess();
        std::string phase_string = obj->getServer()->getStatusPhase();
        float progress_float = obj->getServer()->getStatusProgress();
        std::cout << "InspectorWidgetProcessorWrapper: error " << error_string  << " success " << success_string << " phase " << phase_string << std::endl;
        error = Nan::New(error_string.c_str()).ToLocalChecked();
        success = Nan::New(success_string).ToLocalChecked();
        phase = Nan::New(phase_string).ToLocalChecked();
        progress = Nan::New(progress_float);

        //std::vector<std::string> template_list = server->getTemplateList();
    }
    v8::Local<v8::Value> id = Nan::New(args[0]).ToLocalChecked();

    v8::Local<v8::Value> argv[] = {
        id,
        error /*Nan::Null()*/
        ,success /*Nan::Null()*/
        ,phase
        ,progress
    };

    callback->Call(5, argv);


}

void InspectorWidgetProcessorWrapper::AccessibilityHover(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    int argc = info.Length();
    if (argc != 5) {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }
    /*for (int i=0; i<argc-1; i++){
        if(!info[i]->IsString()){
            std::stringstream error;
            error << "Argument " << i << " should be a string";
            Nan::ThrowTypeError(error.str().c_str());
            return;
        }
    }*/
    if(!info[argc-1]->IsFunction()){
        Nan::ThrowTypeError("The last argument should be a callback function");
        return;
    }

    std::vector<std::string> args;
    for (int i=0; i<argc-1; i++){
        v8::String::Utf8Value arg(info[i]);
        std::string buffer  = std::string(*arg);
        //std::cout << "Buffer '" << buffer << "'" << std::endl;
        args.push_back(buffer);
    }

    Nan::Callback *callback = new Nan::Callback(info[argc-1].As<v8::Function>());

    InspectorWidgetProcessorWrapper* obj = ObjectWrap::Unwrap<InspectorWidgetProcessorWrapper>(info.Holder());

    v8::Local<v8::Value> error = Nan::Null();
    v8::Local<v8::Value> success = Nan::Null();
    v8::Local<v8::Value> x = Nan::Null();
    v8::Local<v8::Value> y = Nan::Null();
    v8::Local<v8::Value> w = Nan::Null();
    v8::Local<v8::Value> h = Nan::Null();
    v8::Local<v8::Value> axTreeChildren = Nan::Null();
    v8::Local<v8::Value> axTreeParents = Nan::Null();
    if(obj->getServer()){
        std::string error_string = obj->getServer()->getStatusError();
        std::string success_string = obj->getServer()->getStatusSuccess();
        std::string phase_string = obj->getServer()->getStatusPhase();
        InspectorWidgetAccessibilityHoverInfo result = obj->getServer()->getAccessibilityHover(atof(args[1].c_str()),atof(args[2].c_str()),atof(args[3].c_str()));
        //std::vector<float> rect = obj->getServer()->getAccessibilityUnderMouse(atof(args[1].c_str()),atof(args[2].c_str()),atof(args[3].c_str()));
        //std::cout << "InspectorWidgetProcessorWrapper: error " << error_string  << " success " << success_string << " phase " << phase_string << std::endl;
        std::vector<float> rect = result.rect;
        if(rect.size() == 4){
            x = Nan::New(rect[0]);
            y = Nan::New(rect[1]);
            w = Nan::New(rect[2]);
            h = Nan::New(rect[3]);
        }
        axTreeChildren = Nan::New(result.xml_tree_children.c_str()).ToLocalChecked();
        axTreeParents = Nan::New(result.xml_tree_parents.c_str()).ToLocalChecked();
        error = Nan::New(error_string.c_str()).ToLocalChecked();
        success = Nan::New(success_string).ToLocalChecked();
    }
    v8::Local<v8::Value> id = Nan::New(args[0]).ToLocalChecked();

    v8::Local<v8::Value> argv[] = {
        id
        ,error
        ,x
        ,y
        ,w
        ,h
        ,axTreeParents
        ,axTreeChildren
    };
    callback->Call(8, argv);
}

void InspectorWidgetProcessorWrapper::ExtractTemplate(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    int argc = info.Length();
    if (argc != 8) {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }
    /*for (int i=0; i<argc-1; i++){
        if(!info[i]->IsString()){
            std::stringstream error;
            error << "Argument " << i << " should be a string";
            Nan::ThrowTypeError(error.str().c_str());
            return;
        }
    }*/
    if(!info[argc-1]->IsFunction()){
        Nan::ThrowTypeError("The last argument should be a callback function");
        return;
    }

    std::vector<std::string> args;
    for (int i=0; i<argc-1; i++){
        v8::String::Utf8Value arg(info[i]);
        std::string buffer  = std::string(*arg);
        //std::cout << "Buffer '" << buffer << "'" << std::endl;
        args.push_back(buffer);
    }

    Nan::Callback *callback = new Nan::Callback(info[argc-1].As<v8::Function>());

    InspectorWidgetProcessorWrapper* obj = ObjectWrap::Unwrap<InspectorWidgetProcessorWrapper>(info.Holder());

    v8::Local<v8::Value> error = Nan::Null();
    if(obj->getServer()){
        std::string id = args[0];
        std::string name = args[1];
        float x = atof(args[2].c_str());
        float y = atof(args[3].c_str());
        float w = atof(args[4].c_str());
        float h = atof(args[5].c_str());
        float time = atof(args[6].c_str());
        bool success = obj->getServer()->extractTemplate(name,x,y,w,h,id,time);
        std::string error_string = success ? "":"Could not extract template";
        error = Nan::New(error_string.c_str()).ToLocalChecked();
    }
    v8::Local<v8::Value> id = Nan::New(args[0]).ToLocalChecked();
    v8::Local<v8::Value> name = Nan::New(args[1]).ToLocalChecked();

    v8::Local<v8::Value> argv[] = {
        id
        ,name
        ,error
    };
    callback->Call(3, argv);
}

void InspectorWidgetProcessorWrapper::AnnotationStatus(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    int argc = info.Length();
    if (argc != 3) {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }
    /*if(!info[1]->IsArray()){
        std::stringstream error;
        error << "Argument 1 should be an array";
        Nan::ThrowTypeError(error.str().c_str());
        return;
    }*/
    for (int i=0; i<argc-1; i++){
        if(/*i!=1 &&*/ !info[i]->IsString()){
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

    /*v8::Local<v8::Array> names = info[1].As<v8::Array>();
    std::vector<std::string> _names;
    for (int i=0; i<names->Length(); i++){
        v8::String::Utf8Value name(names->Get(i));
        _names.push_back(*name);
    }*/

    std::vector<std::string> args;
    for (int i=1; i<argc-1; i++){
        v8::String::Utf8Value arg(info[i]);
        std::string buffer  = std::string(*arg);
        //std::cout << "Buffer '" << buffer << "'" << std::endl;
        args.push_back(buffer);
    }

    Nan::Callback *callback = new Nan::Callback(info[argc-1].As<v8::Function>());

    InspectorWidgetProcessorWrapper* obj = ObjectWrap::Unwrap<InspectorWidgetProcessorWrapper>(info.Holder());

    v8::Local<v8::Value> error = Nan::Null();
    v8::Local<v8::Value> success = Nan::Null();
    v8::Local<v8::Value> phase = Nan::Null();
    v8::Local<v8::Value> progress = Nan::Null();
    v8::Local<v8::Array> annotations;// = Nan::Null();
    if(obj->getServer()){
        std::string error_string = obj->getServer()->getStatusError();
        std::string success_string = obj->getServer()->getStatusSuccess();
        std::string phase_string = obj->getServer()->getStatusPhase();
        float progress_float = obj->getServer()->getStatusProgress();
        //std::vector<std::string> annotations_array = obj->getServer()->getAnnotations(_names);
        std::cout << " get Annotation from " << args[0] << std::endl;
        InspectorWidgetAnnnotationProgress info = obj->getServer()->getAnnotation(args[0]);
        std::vector<std::string> annotations_array;
        annotations_array.push_back(info.name);
        annotations_array.push_back(std::to_string(info.progress));
        annotations_array.push_back(info.annotation);
        //std::cout << "InspectorWidgetProcessorWrapper: error " << error_string  << " success " << success_string << " phase " << phase_string << std::endl;
        error = Nan::New(error_string.c_str()).ToLocalChecked();
        success = Nan::New(success_string).ToLocalChecked();
        phase = Nan::New(phase_string).ToLocalChecked();
        progress = Nan::New(progress_float);
        //annotations = Nan::New(annotation_string).ToLocalChecked();
        annotations = Nan::New<v8::Array>(annotations_array.size());
        for (unsigned i=0; i < annotations_array.size(); i++) {
            annotations->Set( i, Nan::New(annotations_array[i]).ToLocalChecked() );
        }

    }
    v8::Local<v8::Value> id = Nan::New(args[0]).ToLocalChecked();

    v8::Local<v8::Value> argv[] = {
        id,
        error /*Nan::Null()*/
        ,annotations /*Nan::Null()*/
        ,phase
        ,progress
    };

    callback->Call(5, argv);
}
