{
    "targets": [{
        "target_name": "mxnetaddon",
        "msvs_settings": {
            "VCCLCompilerTool": {
            "ExceptionHandling": 1
            }
        },
        "sources": [
            "cppsrc/main.cpp",
            "cppsrc/predictor.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")", 
            "<!(echo %MXNET_ROOT%)/include",
            "<!(echo %MXNET_ROOT%)/3rdparty/dmlc-core/include",
            "<!(echo %MXNET_ROOT%)/3rdparty/tvm/nnvm/include",
            "<!(echo %MXNET_ROOT%)/cpp-package/include",
            "<!(echo %OpenCV_DIR%)/include"
        ],
        'libraries': [
            "-l<!(echo %MXNET_ROOT%)/build/Release/libmxnet",
            "-l<!(echo %OpenCV_DIR%)/x64/vc15/lib/opencv_world341",
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [  ]
    }]
}