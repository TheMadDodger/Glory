#include "GloryAPI.h"

#include <curl/curl.h>
#include <string>
#include <iostream>

namespace Glory
{
    CURL* _curl = nullptr;
    const char* _apiUrl = "https://randomuser.me/api/";


    bool GloryAPI::Initialize()
    {
        _curl = curl_easy_init();
        return _curl;
    }

    void GloryAPI::Cleanup()
    {
        curl_easy_cleanup(_curl);
    }

    size_t GloryAPI::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void GloryAPI::Test()
    {
        /* TODO: Make async */

        std::string readBuffer;
        if (!_curl) return;

        CURLcode res;

        curl_easy_setopt(_curl, CURLOPT_URL, "https://randomuser.me/api/");
        /* Peer has no certificate so skip peer verification */
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        /* We do want to verify the host for security */
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 2L);
        /* Set method to GET */
        curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);

        /* Overwrite the write function */
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(_curl);

        if (res != CURLE_OK)
        {
            std::cout << "Curl Error: " << curl_easy_strerror(res);
            return;
        }

        std::cout << "Curl Response: " << readBuffer << std::endl;
    }
}
