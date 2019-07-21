#include "predictor.h"

// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

Napi::FunctionReference Predictor::constructor;

Napi::Object Predictor::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Predictor", {
    InstanceMethod("destroy", &Predictor::Destroy),
    InstanceMethod("forward", &Predictor::Forward),
    InstanceMethod("partialforward", &Predictor::Partialforward),
    InstanceMethod("output", &Predictor::Output),
    InstanceMethod("setInput", &Predictor::SetInput)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Predictor", func);
  return exports;
}

Predictor::Predictor(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Predictor>(info)  {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    auto length = info.Length();
    if (length != 2 || !info[0].IsObject() || !info[1].IsObject()) {
        Napi::TypeError::New(env, "Expected (modelobj, input_shapes) arguments").ThrowAsJavaScriptException();        
    }

    Napi::Object model = info[0].As<Napi::Object>();

    Napi::Value symbol = model["symbol"];
    Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
    Napi::Function stringify = json.Get("stringify").As<Napi::Function>();
    std::string jsonString = (std::string)stringify.Call(json, { symbol }).As<Napi::String>();

    Napi::Value parambase64 = model["parambase64"];
    Napi::String parambase64Str = parambase64.As<Napi::String>();
    std::string paramBytes = base64_decode(parambase64Str);

    Napi::Object input_shapes = info[1].As<Napi::Object>();
    Napi::Array props = input_shapes.GetPropertyNames();
    std::vector<std::string> names(props.Length());
    std::unique_ptr<char*[]> input_keys = std::make_unique<char*[]>(props.Length());
    std::unique_ptr<mx_uint[]> input_shape_indptr = std::make_unique<mx_uint[]>(props.Length() + 1);
    input_shape_indptr[0] = 0;
    std::vector<mx_uint> flatten;
    for (uint32_t k = 0; k < props.Length(); k++) {
        names[k] = props.Get(k).ToString().Utf8Value();  
        input_keys[k] = (char *)names[k].c_str();
        Napi::Value shape = input_shapes[names[k]];
        if (!shape.IsArray()) {
            Napi::TypeError::New(env, "Expected input_shapes is array").ThrowAsJavaScriptException();        
        }
        Napi::Array shape_array = shape.As<Napi::Array>();
        input_shape_indptr[k + 1] = input_shape_indptr[k] + shape_array.Length();
        for (uint32_t s = 0; s < shape_array.Length(); s++) 
            flatten.push_back(shape_array.Get(s).ToNumber().Int32Value());
    }  
    std::unique_ptr<mx_uint[]> input_shape_data = std::make_unique<mx_uint[]>(flatten.size());
    std::copy(flatten.begin(), flatten.end(), input_shape_data.get()); 

    if (MXPredCreate(jsonString.c_str(), paramBytes.c_str(), paramBytes.size(), 1, 0, 
        props.Length(), (const char **)input_keys.get(), input_shape_indptr.get(), input_shape_data.get(), &_handle)) {
            const char* lastError = MXGetLastError();
            Napi::TypeError::New(env, lastError).ThrowAsJavaScriptException();
        }                   
}

Napi::Value Predictor::Destroy(const Napi::CallbackInfo& info) {
    if (_handle != NULL) {
        MXPredFree(_handle);
        _handle = NULL;
    }

    return Napi::Value::Value();
}

Napi::Value Predictor::Forward(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (MXPredForward(_handle)) {
        const char* lastError = MXGetLastError();
        Napi::TypeError::New(env, lastError).ThrowAsJavaScriptException();
    }

    return Napi::Value::Value();
}

Napi::Value Predictor::Partialforward(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    auto length = info.Length();
    if (length != 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected (step) argument").ThrowAsJavaScriptException();        
    }

    int step = info[0].ToNumber().Int32Value();
    int step_left;
    if (MXPredPartialForward(_handle, step, &step_left)) {
        const char* lastError = MXGetLastError();
        Napi::TypeError::New(env, lastError).ThrowAsJavaScriptException();
    }

    return Napi::Number::New(env, step_left);
}

Napi::Value Predictor::Output(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    auto length = info.Length();
    if (length != 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected (index) argument").ThrowAsJavaScriptException();        
    }

    mx_uint index = info[0].As<Napi::Number>().Uint32Value();

    mx_uint *shape_data;
    mx_uint shape_ndim;		
    if (MXPredGetOutputShape(_handle, index, &shape_data, &shape_ndim)) {
        const char* lastError = MXGetLastError();
        Napi::TypeError::New(env, lastError).ThrowAsJavaScriptException();
    }

    Napi::TypedArrayOf<uint32_t> shape = Napi::TypedArrayOf<uint32_t>::New(env, shape_ndim);
    size_t reduced_size = 1;
    for (uint32_t k = 0; k < shape_ndim; k++) {
        shape[k] = shape_data[k];
        reduced_size *= shape_data[k];
    }

    std::unique_ptr<float[]> ptr_data = std::make_unique<float[]>(reduced_size);
    if (MXPredGetOutput(_handle, index, ptr_data.get(), reduced_size)) {
        const char* lastError = MXGetLastError();
        Napi::TypeError::New(env, lastError).ThrowAsJavaScriptException();
    }

    Napi::TypedArrayOf<float> data = Napi::TypedArrayOf<float>::New(env, reduced_size);
    for (uint32_t k = 0; k < reduced_size; k++)
        data[k] = ptr_data[k];

    Napi::Object res = Napi::Object::New(env);
    res["data"] = data;
    res["shape"] = shape;
    return res;
}

Napi::Value Predictor::SetInput(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    auto length = info.Length();
    if (length != 2 || !info[0].IsString() || !info[1].IsObject()) {
        Napi::TypeError::New(env, "Expected (key, ndarray) arguments").ThrowAsJavaScriptException();        
    }

    std::string key = (std::string)info[0].As<Napi::String>();  
    Napi::Object ndarray = info[1].As<Napi::Object>();
    Napi::Value data = ndarray["data"];
    Napi::TypedArrayOf<float> floatArray = data.As<Napi::TypedArrayOf<float>>();
    if (MXPredSetInput(_handle, key.c_str(), floatArray.Data(), floatArray.ElementLength())) {
        const char* lastError = MXGetLastError();
        Napi::TypeError::New(env, lastError).ThrowAsJavaScriptException();
    }

    return Napi::Value::Value();
}

