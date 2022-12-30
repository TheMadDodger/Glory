//#pragma once
//#include <curl/curl.h>
//#include <sstream>
//
//namespace Glory::EditorLauncher
//{
//    class CURLpp
//    {
//    private:
//        CURL* curl;
//        std::stringstream ss;
//        long http_code;
//    public:
//        CURLpp();
//        virtual ~CURLpp();
//        std::string Get(const std::string& url);
//        long GetHttpCode();
//
//    private:
//        static size_t WriteData(void* buffer, size_t size, size_t nmemb, void* userp);
//        size_t Write(void* buffer, size_t size, size_t nmemb);
//    };
//}