{ "target_defaults": {
    "cflags": [ "-Wall", "-Wextra", "-Wno-unused_parameter" ]
  , "defines": [ "V8_DEPRECATION_WARNINGS=1" ]
  , "include_dirs": [ "<!(node -e \"require('nan')\")", "<!(node -e \"require('..')\")" ]
  }
, "includes" : ["self-sufficiency/test_headers.gypi"]
, "targets": [
    {"target_name": "100_basic_work", "sources": ["100_basic_work.cpp"]}
  ]
}
