#pragma once
#include "curl/curl.h"
#include <fstream>
#include <queue>

class MailService {
    CURL *m_Curl;
    std::string m_Username;
    std::string m_Password;
    std::string m_CertPath;

    static size_t SearchCallback(void *contents, size_t size, size_t nmemb, void *userp);

    static size_t FetchCallback(void *contents, size_t size, size_t nmemb, void *userp);

    static size_t SMTPCallback(void *contents, size_t size, size_t nmemb, void *userp);

    bool m_Initialized = false;

    std::queue<std::string> m_Jobs;
public:
    MailService();

    ~MailService();

    void Init(const std::string &username, const std::string &password, const std::string &certPath);

    void Init(const std::string &confFilePath);

    void CheckMail();

    void CleanUpCURL();

    void HandleMailCommand(int mailId);

    void Response(const std::string &to, const std::string &body, const char *attachment);
};
