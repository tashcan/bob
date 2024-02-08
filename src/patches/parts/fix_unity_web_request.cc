#include "config.h"

#include <spud/detour.h>

#include <il2cpp/il2cpp_helper.h>

#include "utils.h"

#include "prime/HttpJob.h"
#include "prime/HttpRequest.h"
#include "prime/HttpResponse.h"

#include <spdlog/spdlog.h>

#include "utils.h"

static void ProcessSending_Hook(auto original, void *_this, HttpJob *item)
{

  static auto get_downloadedBytes = il2cpp_resolve_icall<uint64_t(UnityWebRequest *)>(
      "UnityEngine.Networking.UnityWebRequest::get_downloadedBytes()");
  static auto get_uploadedBytes = il2cpp_resolve_icall<uint64_t(UnityWebRequest *)>(
      "UnityEngine.Networking.UnityWebRequest::get_uploadedBytes()");
  static auto get_uploadHandler = il2cpp_resolve_icall<uint64_t(UnityWebRequest *)>(
      "UnityEngine.Networking.UnityWebRequest::get_uploadHandler()");
  static auto get_uploadProgress = il2cpp_resolve_icall<float(UnityWebRequest *)>(
      "UnityEngine.Networking.UnityWebRequest::GetUploadProgress");
  static auto get_IsExecuting =
      il2cpp_resolve_icall<bool(UnityWebRequest *)>("UnityEngine.Networking.UnityWebRequest::IsExecuting()");
  static auto get_isNetworkError = il2cpp_resolve_icall<bool(UnityWebRequest *)>(
      "UnityEngine.Networking.UnityWebRequest::get_isNetworkError()");
  static auto get_ResponseCode = il2cpp_resolve_icall<int32_t(UnityWebRequest *)>(
      "UnityEngine.Networking.UnityWebRequest::get_responseCode()");

  if (!get_IsExecuting || !get_ResponseCode || !get_uploadProgress) {
    return original(_this, item);
  }

  auto unityRequest = item->UnityRequest;

  if (!get_IsExecuting(unityRequest) && get_ResponseCode(unityRequest) == 200) {
    item->State = HttpJobState::ResponseReceived;
    return;
  }

  auto uploadProgress = get_uploadProgress(unityRequest);
  if (!unityRequest->get_uploadHandler() || uploadProgress == 1.0f) {
    item->State = HttpJobState::ResponseReceiving;
    return;
  }

  original(_this, item);
}

static void SendWebRequest_Hook(auto original, UnityWebRequest *_this)
{
  static auto get_use100Continue = il2cpp_resolve_icall<bool(UnityWebRequest *)>(
      "UnityEngine.Networking.UnityWebRequest::get_use100Continue");
  static auto set_use100Continue = il2cpp_resolve_icall<void(UnityWebRequest *, bool)>(
      "UnityEngine.Networking.UnityWebRequest::set_use100Continue");
  set_use100Continue(_this, false);
  original(_this);
}

static void                         *unity_web_request_multi_handle = nullptr;
int64_t                              curl_multi_cleanup(void *multi_handle);
static decltype(curl_multi_cleanup) *ocurl_multi_cleanup = nullptr;
static int64_t                       curl_multi_cleanup(void *multi_handle)
{
  if (multi_handle == unity_web_request_multi_handle) {
    return 0;
  }
  return ocurl_multi_cleanup(multi_handle);
}

void                             *curl_multi_init();
static decltype(curl_multi_init) *ocurl_multi_init = nullptr;
static int                        re_create_count  = 0;
static void                      *curl_multi_init()
{
  // The only requests to this are for the internal UnityWebRequest impl things
  // so remember the handle as the `unity_web_request_multi_handle`
  if (!unity_web_request_multi_handle) {
    unity_web_request_multi_handle = ocurl_multi_init();
  } else {
    spdlog::debug("Connection re-create {}, preventing keep-alive drop", re_create_count++);
  }
  return unity_web_request_multi_handle;
}

void InstallWebRequestHooks()
{
  return;

  /*
  if (!Config::Get().fix_unity_web_requests) {
    return;
  }
  auto screen_manager_helper =
      il2cpp_get_class_helper("Digit.Engine.HTTPClient.Runtime", "Digit.Networking.Network", "DigitHttpClient");

  auto meow = screen_manager_helper.GetMethod("ProcessSending");
  if (!meow) {
    return;
  }
  SPUD_STATIC_DETOUR(meow, ProcessSending_Hook);

  auto send_web_request_ptr = il2cpp_resolve_icall<void(UnityWebRequest *)>(
      "UnityEngine.Networking.UnityWebRequest::BeginWebRequest()");
  SPUD_STATIC_DETOUR(send_web_request_ptr, SendWebRequest_Hook);
  */
}