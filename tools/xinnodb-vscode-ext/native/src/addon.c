#include <node_api.h>
#include <assert.h>

static napi_value greet(napi_env env, napi_callback_info info) {
	napi_value result;
	napi_status status = napi_create_string_utf8(env, "Hello from native N-API addon!", NAPI_AUTO_LENGTH, &result);
	assert(status == napi_ok);
	return result;
}

static napi_value Init(napi_env env, napi_value exports) {
	napi_value fn;
	napi_status status = napi_create_function(env, "greet", NAPI_AUTO_LENGTH, greet, 0, &fn);
	assert(status == napi_ok);
	status = napi_set_named_property(env, exports, "greet", fn);
	assert(status == napi_ok);
	return exports;
}

NAPI_MODULE(addon, Init)


