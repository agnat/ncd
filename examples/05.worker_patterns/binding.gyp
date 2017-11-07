{ "target_defaults": {
    "cflags": [ "-Wall", "-Wextra", "-Wno-unused_parameter" ]
  , "defines": [ "V8_DEPRECATION_WARNINGS=1" ]
  , "include_dirs": [ "<!(node -e \"require('nan')\")", "<!(node -e \"require('../..')\")" ]
  , "cflags_cc!" : [ "-fno-rtti", "-fno-exceptions" ]
  }
, "targets": [
    { "target_name": "worker_patterns"
    , "sources"    : [ "worker_patterns.cpp" ]
    , "conditions": [
      [ "OS=='mac'", {
        "xcode_settings" : {
            "OTHER_CFLAGS": [ "-std=c++14" ]
          , "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
          , "GCC_ENABLE_CPP_RTTI": "YES"
          }
        }]
      ]
    }
  ]
}
