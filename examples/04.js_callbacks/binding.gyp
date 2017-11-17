{ "target_defaults": {
    "cflags": ["-Wall", "-Wextra", "-Wno-unused_parameter"]
  , "defines": ["V8_DEPRECATION_WARNINGS=1"]
  , "include_dirs": ["<!(node -e \"require('nan')\")", "<!(node -e \"require('../..')\")"]
  }
, "targets": [
    {"target_name": "js_callbacks", "sources": ["js_callbacks.cpp"]}
  ]
}
