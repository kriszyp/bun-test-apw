#include <napi.h>

#include <chrono>
#include <thread>

using namespace Napi;

class TestAPW : public AsyncProgressWorker<uint32_t> {
    public:
        TestAPW(Function& okCallback)
        : AsyncProgressWorker(okCallback) {}

        ~TestAPW() {}
        
        // This code will be executed on the worker thread
        void Execute(const ExecutionProgress& progress) {
            // Need to simulate cpu heavy task
            // Note: This Send() call is not guaranteed to trigger an equal
            // number of OnProgress calls (read documentation above for more info)
            fprintf(stderr, "start executing\n");
            for (uint32_t i = 0; i < 1000; ++i) {
              progress.Send(&i, 1);
            }
            fprintf(stderr, "done executing\n");
        }
        
        void OnError(const Error &e) {
            HandleScope scope(Env());
            // Pass error onto JS, no data for other parameters
            Callback().Call({String::New(Env(), e.Message())});
        }

        void OnOK() {
            fprintf(stderr, "OnOK\n");
            // return -1
            Callback().Call({Env().Null(), Number::New(Env(), -1)});
        }

        void OnProgress(const uint32_t* data, size_t /* count */) {
            fprintf(stderr, "OnProgress\n");

            // this does a callback while avoiding the escapable scopes
            napi_value result;
            napi_call_function(Env(), Env().Undefined(), Callback().Value(), 0, {}, &result);
            // this does a callback, but I think it uses escapable scopes
            //Callback().Call({Env().Null(), Number::New(Env(), *data)});
        }

};
napi_value start(napi_env env, napi_callback_info info) {
	size_t argc = 1;
	napi_value args[1];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    Function callback = Function(env, args[0]);
    fprintf(stderr, "created\n");
	TestAPW* worker = new TestAPW(callback);
	worker->Queue();
    fprintf(stderr, "queued\n");
	napi_value result;
	napi_get_undefined(env, &result);
	return result;

}
#define EXPORT_NAPI_FUNCTION(name, func) { napi_property_descriptor desc = { name, 0, func, 0, 0, 0, (napi_property_attributes) (napi_writable | napi_configurable), 0 }; napi_define_properties(env, exports, 1, &desc); }
napi_value Init(napi_env env, napi_value exports) {
	EXPORT_NAPI_FUNCTION("start", start);
	return exports;
}


NAPI_MODULE_INIT() {
	EXPORT_NAPI_FUNCTION("start", start);
	return exports;
}
// According to the docs, this is another valid way to register a NAPI module, but this fails with:  TypeError: symbol 'napi_register_module_v1' not found in native module. Is this a Node API (napi) module?
//NAPI_MODULE(extractor, Init)
