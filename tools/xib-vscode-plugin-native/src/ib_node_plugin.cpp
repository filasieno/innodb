#include <node_api.h>

#define IB_NODE_PLUGIN_NAME addon

// Simple N-API function: returns a greeting string
napi_value greet(napi_env env, napi_callback_info info) {
  napi_value result;
  napi_create_string_utf8(env, "Hello from native N-API addon!", NAPI_AUTO_LENGTH, &result);
  return result;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_value fn;
  napi_create_function(env, NULL, 0, greet, NULL, &fn);
  napi_set_named_property(env, exports, "greet", fn);
  return exports;
}

NAPI_MODULE(IB_NODE_PLUGIN_NAME, Init)