# N-API nodejs add-on to use with MXNET  C predict API

This addon has a functionality similar to standard mxnet.js Javascript interface for MXNET predication API but it uses a  compiled version of MXNET instead of emscripten virtual machine. It has impact at the performance and lets usage of GPU acceleration models inside nodejs application.

To build it 

1. Compile MXNET from sources as described in the manual
[build MXNet from Source](https://mxnet.incubator.apache.org/versions/master/install/build_from_source.html). 

    Please ensure  that USE_CPP_PACKAGE=1 in your cmake confing is set. For example, 
```
cmake -G "Visual Studio 15 2017 Win64" -T host=x64 -DUSE_CUDA=0 -DUSE_CUDNN=0 -DUSE_NVRTC=0 -DUSE_OPENCV=1 -DUSE_OPENMP=1 -DUSE_BLAS=mkl -DUSE_LAPACK=1 -DUSE_DIST_KVSTORE=0 -DCUDA_ARCH_LIST=All -DUSE_MKLDNN=1 -DCMAKE_BUILD_TYPE=Release -DUSE_CPP_PACKAGE=1 -DMKL_ROOT="C:\Program Files (x86)\IntelSWTools\compilers_and_libraries\windows\mkl" "C:\work\AI\mxnet"
```

2. Ensure that the required MXNET, MKLDNN and OpenCV shared libraries are added to the PATH and set path variables MXNET_ROOT and OpenCV_DIR to MXNET local repository root and OpenCV installation dir.

4. Install Chocolatey package manager and build the addon with command ```npm run build``` and install it with ```npm install -g``` command.

5. To use the addon the next code snippet can be used:

```javascript
...
var mx = require('./addon.js');
const model = require("sample.json");
const pred = new mx.Predictor(model, {'data': [1,6,7,9,9] });
pred.setInput("data", ndarray(mydata, [1,6,7,9,9]));
pred.forward();
var output = pred.output(0);
pred.destroy();
...
```
