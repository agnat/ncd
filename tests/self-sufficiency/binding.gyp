{ "target_defaults": {
    "cflags": [ "-Wall", "-Wextra", "-Wno-unused_parameter" ]
  , "defines": [ "V8_DEPRECATION_WARNINGS=1" ]
  , "include_dirs": [ "<!(node -e \"require('nan')\")", "<!(node -e \"require('../..')\")" ]
  , "cflags_cc!" : [ "-fno-rtti", "-fno-exceptions" ]
  , "conditions": [ [ 'OS=="mac"', { "xcode_settings" : { "OTHER_CFLAGS": [ "-std=c++14" ] } }]]
  }
, "targets": [
    { "target_name": "async_error_hpp",         "sources": [ "async_error_hpp.cpp" ] }
  , { "target_name": "async_event_emitter_hpp", "sources": [ "async_event_emitter_hpp.cpp" ] }
  , { "target_name": "async_function_hpp",      "sources": [ "async_function_hpp.cpp" ] }
  , { "target_name": "async_handle_hpp",        "sources": [ "async_handle_hpp.cpp" ] }
  , { "target_name": "default_work_queue_hpp",  "sources": [ "default_work_queue_hpp.cpp" ] }
  , { "target_name": "function_hpp",            "sources": [ "function_hpp.cpp" ] }
  , { "target_name": "main_queue_hpp",          "sources": [ "main_queue_hpp.cpp" ] }
  , { "target_name": "meta_hpp",                "sources": [ "meta_hpp.cpp" ] }
  , { "target_name": "queue_getters_hpp",       "sources": [ "queue_getters_hpp.cpp" ] }
  , { "target_name": "streams_hpp",             "sources": [ "streams_hpp.cpp" ] }
  , { "target_name": "uv_hpp",                  "sources": [ "uv_hpp.cpp" ] }
  , { "target_name": "v8_utils_hpp",            "sources": [ "v8_utils_hpp.cpp" ] }
  , { "target_name": "work_queue_hpp",          "sources": [ "work_queue_hpp.cpp" ] }
  ]
}
