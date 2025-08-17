#include "Mail.hpp"
#include <vector>
#include <cassert>
#include <iostream>

size_t MailService::SearchCallback(void *contents, const size_t size, const size_t nmemb, void *userp) {
    std::string res = static_cast<char *>(contents);
    auto mailIds = static_cast<std::vector<int> *>(userp);
    res = res.substr(0, res.find("\r\n"));
    if (res.starts_with("* SEARCH")) {
        res.erase(0, 8);
        size_t pos = 0;
        while ((pos = res.find(' ')) != std::string::npos && res.front() != '\r') {
            std::string idStr = res.substr(0, pos);
            if (!idStr.empty()) {
                mailIds->push_back(std::stoi(idStr));
            }
            res.erase(0, pos + 1);
        }
        if (!res.empty()) {
            mailIds->push_back(std::stoi(res));
        }
    }
    else {
        throw std::invalid_argument("Invalid search string");
    }
    std::cout << "Found " << mailIds->size() << " unseen mails." << std::endl;
    return size * nmemb;
}

size_t MailService::FetchCallback(void *contents, const size_t size, const size_t nmemb, void *userp) {
    std::string res = static_cast<char *>(contents);
    res = res.substr(res.find("\r\n")+2);
    res = res.substr(0, res.find("\r\n"));
    auto mailContent = static_cast<std::string *>(userp);
    mailContent->assign(res);
    return size * nmemb;
}

size_t MailService::SMTPCallback(void *contents, const size_t size, const size_t nmemb, void *userp) {

}

MailService::MailService() {
    m_Curl = curl_easy_init();
    if (!m_Curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    m_Username = "";
    m_Password = "";
    m_CertPath = "";
}

MailService::~MailService() {
    if (m_Initialized) {
        CleanUpCURL();
    }
}

void MailService::Init(const std::string &username, const std::string &password,
                       const std::string &certPath) {
    m_Username = username;
    m_Password = password;
    m_CertPath = certPath;
    curl_easy_setopt(m_Curl, CURLOPT_USERNAME, m_Username.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_PASSWORD, m_Password.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_CAINFO, m_CertPath.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(m_Curl, CURLOPT_CERTINFO, 1L);
    m_Initialized = true;
}

void MailService::Init(const std::string &confFilePath) {
    std::ifstream inputFile(confFilePath);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Failed to open configuration file: " + confFilePath);
    }
    std::getline(inputFile, m_Username);
    std::getline(inputFile, m_Password);
    std::getline(inputFile, m_CertPath);
    inputFile.close();
    curl_easy_setopt(m_Curl, CURLOPT_USERNAME, m_Username.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_PASSWORD, m_Password.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_CAINFO, m_CertPath.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(m_Curl, CURLOPT_CERTINFO, 1L);
    m_Initialized = true;
}

void MailService::CheckMail() {
    assert(m_Initialized == true);
    CURL *localCURL = curl_easy_duphandle(m_Curl);
    if (!localCURL) {
        throw std::runtime_error("Failed to duplicate CURL handle");
    }
    std::vector<int> mailIds;

    curl_easy_setopt(localCURL, CURLOPT_WRITEFUNCTION, SearchCallback);
    curl_easy_setopt(localCURL, CURLOPT_WRITEDATA, &mailIds);
    curl_easy_setopt(localCURL, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX");
    curl_easy_setopt(localCURL, CURLOPT_CUSTOMREQUEST, "SEARCH UNSEEN");

    if (const CURLcode res = curl_easy_perform(localCURL); res != CURLE_OK) {
        CleanUpCURL();
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }
    for (const auto &mailId : mailIds) {
        HandleMailCommand(mailId);
    }
    curl_easy_cleanup(localCURL);
    localCURL = nullptr;
}

void MailService::CleanUpCURL() {
    assert(m_Initialized == true);
    if (m_Curl) {
        curl_easy_cleanup(m_Curl);
        m_Curl = nullptr;
    }
    m_Initialized = false;
    m_Username.clear();
    m_Password.clear();
    m_CertPath.clear();
}

void MailService::HandleMailCommand(const int mailId) {
    assert(m_Initialized == true);
    CURL *localCURL = curl_easy_duphandle(m_Curl);
    if (localCURL == nullptr) {
        throw std::runtime_error("Failed to duplicate CURL handle");
    }
    std::string mailContent;
    curl_easy_setopt(localCURL, CURLOPT_WRITEFUNCTION, FetchCallback);
    curl_easy_setopt(localCURL, CURLOPT_WRITEDATA, &mailContent);
    curl_easy_setopt(localCURL, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX");
    curl_easy_setopt(localCURL, CURLOPT_CUSTOMREQUEST, std::format("FETCH {} BODY[1]", mailId).c_str());
    if (const CURLcode res = curl_easy_perform(localCURL); res != CURLE_OK) {
        CleanUpCURL();
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }
    std::cout << "Mail ID: " << mailId << ", Content: " << mailContent << std::endl;
    curl_easy_cleanup(localCURL);
    localCURL = nullptr;
}

void MailService::Response(const std::string &to, const std::string &body, const char *attachment) {
    assert(m_Initialized == true);
    CURL *localCURL = curl_easy_duphandle(m_Curl);
    if (localCURL == nullptr) {
        throw std::runtime_error("Failed to duplicate CURL handle");
    }
    curl_easy_setopt(localCURL, CURLOPT_URL, "smtp://smtp.gmail.com:587");
    curl_easy_setopt(localCURL, CURLOPT_MAIL_FROM, m_Username.c_str());
    curl_easy_setopt(localCURL, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(localCURL, CURLOPT_READFUNCTION, SMTPCallback);

    curl_slist *recipients = nullptr;
    recipients = curl_slist_append(recipients, to.c_str());
    curl_easy_setopt(localCURL, CURLOPT_MAIL_RCPT, recipients);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, std::format("To: {}", to).c_str());
    headers = curl_slist_append(headers, std::format("From: {}", m_Username).c_str());
    headers = curl_slist_append(headers, "Subject: Test Email from Management System Client");
    curl_easy_setopt(localCURL, CURLOPT_HTTPHEADER, headers);

    curl_mime *mime = curl_mime_init(localCURL);
    curl_mimepart *part = curl_mime_addpart(mime);
    curl_mime_data(part, body.c_str(), CURL_ZERO_TERMINATED);
    curl_mime_type(part, "text/plain");
    curl_easy_setopt(localCURL, CURLOPT_MIMEPOST, mime);

    const CURLcode code = curl_easy_perform(localCURL);
    if (code != CURLE_OK) {
        CleanUpCURL();
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(code)));
    }
    curl_mime_free(mime);
    curl_easy_cleanup(localCURL);
    localCURL = nullptr;
    std::cout << "Email sent to " << to << " with body: " << body << std::endl;
}
