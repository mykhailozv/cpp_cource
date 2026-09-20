// ДЗ15: постинг результатів симуляції на сервер курсу.
// Використання:
//   dz15_poster <studentId> [resultsDir]
//
// resultsDir має містити результати:
//   <resultsDir>/T01/simulation.json ... або <resultsDir>/T01.json ...
// Тест, для якого немає файлу, пропускається.

#include <httplib.h>

#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr const char* kHost = "cppmiltech.com.ua";
constexpr int kPort = 80;
constexpr const char* kApiKey = "dz12-vX7mK4qT9r2w";
constexpr int kMaxAttempts = 5;
constexpr auto kRetryPause = std::chrono::seconds(1);
constexpr int kConnectTimeoutSec = 2;         // таймаут з'єднання <= 2 c
constexpr int kReadTimeoutSec = 2;            // таймаут читання <= 2 c

struct TestResult {
    std::string testId;
    std::string status;
    std::string detail;
    int attempts = 0;
    bool verified = false;
};

std::string readFile(const fs::path& path, bool& ok) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        ok = false;
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    ok = true;
    return ss.str();
}

// Мінімальна перевірка, що файл — коректний JSON-об'єкт
bool looksLikeJsonObject(const std::string& body) {
    size_t i = body.find_first_not_of(" \t\r\n");
    if (i == std::string::npos || body[i] != '{') return false;
    int depth = 0;
    bool inString = false;
    bool escaped = false;
    for (; i < body.size(); ++i) {
        char c = body[i];
        if (inString) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') inString = false;
            continue;
        }
        if (c == '"') inString = true;
        else if (c == '{') ++depth;
        else if (c == '}') {
            --depth;
            if (depth == 0) return true;
        }
    }
    return false;
}

void tryPost(const std::string& host, int port, const std::string& apiKey,
             const std::string& payload, int& code, std::string& body,
             std::string& err) {
    httplib::Client cli(host, port);
    // keep-alive вимикаємо — сокет не перевикористовується.
    cli.set_keep_alive(false);
    cli.set_connection_timeout(kConnectTimeoutSec, 0);
    cli.set_read_timeout(kReadTimeoutSec, 0);
    cli.set_write_timeout(kReadTimeoutSec, 0);
    httplib::Headers headers = {{"x-api-key", apiKey}};

    httplib::Result res = cli.Post("/api/dz12/results", headers, payload,
                                   "application/json");
    if (!res) {
        code = -1;
        err = httplib::to_string(res.error());
        return;
    }
    code = res.value().status;
    body = res.value().body;
}

int tryGet(const std::string& host, int port, const std::string& apiKey,
           const std::string& testId, const std::string& studentId) {
    httplib::Client cli(host, port);
    cli.set_keep_alive(false);
    cli.set_connection_timeout(kConnectTimeoutSec, 0);
    cli.set_read_timeout(kReadTimeoutSec, 0);
    httplib::Headers headers = {{"x-api-key", apiKey}};
    const std::string path = "/api/dz12/results/" + testId + "/" + studentId;
    httplib::Result res = cli.Get(path, headers);
    return res ? static_cast<int>(res.value().status) : -1;
}

TestResult uploadTest(const std::string& host, int port,
                      const std::string& apiKey,
                      const std::string& studentId, const std::string& testId,
                      const fs::path& simulationPath) {
    TestResult result;
    result.testId = testId;
    result.status = "FAILED";

    bool ok = false;
    const std::string simulation = readFile(simulationPath, ok);
    if (!ok) {
        result.detail = "не вдалося прочитати " + simulationPath.string();
        return result;
    }
    if (!looksLikeJsonObject(simulation)) {
        result.detail = "simulation.json не схожий на JSON-об'єкт";
        return result;
    }

    // {"studentId": "...", "testId": "...", "simulation": {...}}
    std::ostringstream payload;
    payload << "{\"studentId\":\"" << studentId << "\",\"testId\":\"" << testId
            << "\",\"simulation\":" << simulation << "}";
    const std::string body = payload.str();

    for (int attempt = 1; attempt <= kMaxAttempts; ++attempt) {
        result.attempts = attempt;
        std::string respBody;
        std::string err;
        int code = -1;
        tryPost(host, port, apiKey, body, code, respBody, err);

        if (code >= 200 && code < 300) {
            // Успіх (201 — збережено вперше, 200 — перезаписано).
            // Перевіряємо GET-ом, що результат справді на сервері.
            const int getCode = tryGet(host, port, apiKey, testId, studentId);
            if (getCode == 200) {
                result.status = "OK";
                result.verified = true;
                result.detail = std::string(code == 201 ? "saved (201)"
                                                        : "overwritten (200)") +
                                ", GET verified";
                return result;
            }
            // POST вдався, але GET не підтвердив — повторюємо.
            result.detail = "POST " + std::to_string(code) +
                            ", GET verify failed (" +
                            (getCode < 0 ? "timeout" : std::to_string(getCode)) + ")";
        } else if (code == 400 || code == 401) {
            // Хибні дані або ключ
            result.detail = std::to_string(code) + " (no retry): " + respBody;
            return result;
        } else if (code == 503) {
            result.detail = "503 (attempt " + std::to_string(attempt) + ")";
        } else if (code > 0) {
            // Інші помилки — повторення не передбачене протоколом.
            result.detail = std::to_string(code) + " (no retry): " + respBody;
            return result;
        } else {
            // Таймаут / обрив з'єднання — повторюємо.
            result.detail = err + " (attempt " + std::to_string(attempt) + ")";
        }

        if (attempt < kMaxAttempts) std::this_thread::sleep_for(kRetryPause);
    }
    return result;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << (argc > 0 ? argv[0] : "dz15_poster")
                  << " <studentId> [resultsDir]\n";
        return 2;
    }
    const std::string studentId = argv[1];
    const fs::path resultsDir =
        (argc >= 3) ? fs::path(argv[2]) : fs::path("data");

    // Хост/порт можна перевизначити (для локальних тестів з mock-сервером)
    const char* envHost = std::getenv("DZ15_HOST");
    const std::string host =
        (envHost && *envHost) ? std::string(envHost) : std::string(kHost);
    const char* envPort = std::getenv("DZ15_PORT");
    const int port = (envPort && *envPort) ? std::atoi(envPort) : kPort;
    // Ключ теж можна перевизначити (для тестування обробки 401).
    const char* envKey = std::getenv("DZ15_API_KEY");
    const std::string apiKey =
        (envKey && *envKey) ? std::string(envKey) : std::string(kApiKey);

    // Збираємо пари тестId -> шлях до simulation.json (T01..T10).
    struct Entry { std::string id; fs::path path; };
    std::vector<Entry> entries;
    for (int i = 1; i <= 10; ++i) {
        const std::string id = (i < 10 ? "T0" : "T") + std::to_string(i);
        const fs::path inDir = resultsDir / id / "simulation.json";
        const fs::path flat = resultsDir / (id + ".json");
        if (fs::exists(inDir)) entries.push_back({id, inDir});
        else if (fs::exists(flat)) entries.push_back({id, flat});
        // Тест без файлу пропускаємо
    }

    if (entries.empty()) {
        std::cerr << "no simulation results found under "
                  << resultsDir.string() << "\n";
        return 2;
    }

    std::vector<TestResult> results;
    for (const auto& e : entries) {
        std::cout << ">>> " << e.id << " <- " << e.path.string() << "\n"
                  << std::flush;
        results.push_back(uploadTest(host, port, apiKey, studentId, e.id, e.path));
    }

    // Підсумковий звіт
    std::cout << "\n========== REPORT ==========\n";
    int failures = 0;
    for (const auto& r : results) {
        std::cout << r.testId << " | " << r.status << " | attempts: "
                  << r.attempts << (r.verified ? " | GET-verified" : "")
                  << " | " << r.detail << "\n";
        if (r.status != "OK") ++failures;
    }
    std::cout << "============================\n";
    std::cout << "total: " << results.size()
              << ", ok: " << (results.size() - failures)
              << ", failed: " << failures << "\n";
    return failures == 0 ? 0 : 1;
}