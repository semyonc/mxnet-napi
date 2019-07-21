/* cppsrc/main.cpp */
#include <napi.h>

#include <opencv2/opencv.hpp>
#include "mxnet/c_predict_api.h"
#include "predictor.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
   return Predictor::Init(env, exports);
}

NODE_API_MODULE(mxnetaddon, InitAll)