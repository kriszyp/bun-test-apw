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
            for (uint32_t i = 0; i < 1000; ++i) {
              progress.Send(&i, 1);
            }
        }
        
        void OnError(const Error &e) {
            HandleScope scope(Env());
            // Pass error onto JS, no data for other parameters
            Callback().Call({String::New(Env(), e.Message())});
        }

        void OnOK() {
            HandleScope scope(Env());
            // Pass no error, give back original data
            Callback().Call({Env().Null(), Number::New(Env(), -1)});
        }

        void OnProgress(const uint32_t* data, size_t /* count */) {
            HandleScope scope(Env());
            // Pass no error, no echo data, but do pass on the progress data
            Callback().Call({Env().Null(), Number::New(Env(), *data)});
        }

};
napi_value start(napi_env env, napi_callback_info info) {
	size_t argc = 1;
	napi_value args[1];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	TestAPW* worker = new TestAPW(Function(env, args[0]));
	worker->Queue();
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
//NAPI_MODULE(extractor, Init)