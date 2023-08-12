#include "GloryAPI.h"

#include <curl/curl.h>
#include <string>
#include <iostream>
#include <future>
#include <Version.h>

#include <rapidjson/document.h>

#ifdef GLORY_ENABLE_API
#define VERIFY_CURL()\
if (!curl)\
{\
    std::cerr << "GloryAPI: Cannot make API request: curl is not initialized!";\
    return;\
}\

#define RUN(ret, f)\
Run([&]() -> bool {\
    static bool isDone = true;\
    static std::future<ret> async;\
    if (isDone)\
    {\
        async = std::async(f); \
        isDone = false; \
    }\
    \
    if (async.wait_for(std::chrono::microseconds(1)) == std::future_status::ready)\
    {\
        Version version = async.get(); \
        isDone = true; \
        callbackStore(version); \
        return true; \
    }\
    return false;\
})
#else
#define VERIFY_CURL()
#define RUN(ret, f)
#endif

#define GLORY_VERSION_API_URL "http://glory-engine.com:5000/versions"

namespace Glory
{
    CURL* curl = nullptr;
    std::vector<std::function<bool()>> requests;

    bool GloryAPI::Initialize()
    {
#ifdef GLORY_ENABLE_API
        curl = curl_easy_init();
        return curl;
#else
        return false;
#endif
    }

    void GloryAPI::Cleanup()
    {
#ifdef GLORY_ENABLE_API
        curl_easy_cleanup(curl);
#endif
    }

    void GloryAPI::Run(std::function<bool()> func)
    {
        requests.push_back(func);
    }

    size_t GloryAPI::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void GloryAPI::FetchEditorVersion(std::function<void(const Version&)> callback)
    {
        VERIFY_CURL();

        static std::function<void(const Version&)> callbackStore;
        callbackStore = callback;

        RUN(Version, FetchEditorVersion_Impl);
    }

    void GloryAPI::RunRequests()
    {
#ifdef GLORY_ENABLE_API
        std::vector<size_t> toRemoveIndices;
        for (size_t i = requests.size(); i > 0; --i)
        {
            if (requests[i - 1]())
                toRemoveIndices.push_back(i - 1);
        }

        for (size_t i = 0; i < toRemoveIndices.size(); ++i)
        {
            requests.erase(requests.begin() + i);
        }
#endif
    }

    Version GloryAPI::FetchEditorVersion_Impl()
    {
        std::string readBuffer;
        if (!curl) return {};

        CURLcode res;

        curl_easy_setopt(curl, CURLOPT_URL, GLORY_VERSION_API_URL);
        /* Peer has no certificate so skip peer verification */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        /* We do want to verify the host for security */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        /* Set method to GET */
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        /* Overwrite the write function */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) return {};

        /* Parse JSON response */
        rapidjson::Document doc;
        doc.Parse(readBuffer.c_str());

        if (doc.HasParseError() || !doc.IsArray()) return {};
        if (!doc.Size()) return {};
        rapidjson::Value& latest = doc[0];
        if (!latest.IsObject() || !latest.HasMember("appVersion") || !latest["appVersion"].IsString()) return {};
        const char* versionString = latest["appVersion"].GetString();
        return Version::Parse(versionString);
    }
}
