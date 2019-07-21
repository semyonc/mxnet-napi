#include <napi.h>
#include "mxnet/c_predict_api.h"

class Predictor: public Napi::ObjectWrap<Predictor> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports); // Init function for setting the export key to JS
    Predictor(const Napi::CallbackInfo& info); // Constructor to initialise    
private:
    static Napi::FunctionReference constructor; 
    Napi::Value Destroy(const Napi::CallbackInfo& info);
    Napi::Value Forward(const Napi::CallbackInfo& info);
    Napi::Value Partialforward(const Napi::CallbackInfo& info);
    Napi::Value Output(const Napi::CallbackInfo& info);
    Napi::Value SetInput(const Napi::CallbackInfo& info);
    PredictorHandle _handle; 
};