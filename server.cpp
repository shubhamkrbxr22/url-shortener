#include "crow.h"
#include "base62.h"

// Optional dependencies
#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif

#ifdef USE_REDIS
#include <sw/redis++/redis++.h>
using namespace sw::redis;
#endif

#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

// ================= MEMORY FALLBACK =================
unordered_map<string, string> memoryStore;
long long memoryId = 1;

// ================= REDIS =================
#ifdef USE_REDIS
unique_ptr<Redis> redisClient;
bool redisAvailable = false;
#endif

// ================= MYSQL =================
#ifdef USE_MYSQL
MYSQL *conn;
bool mysqlAvailable = false;
#endif

// ================= INIT MYSQL =================
void initDB() {
#ifdef USE_MYSQL
    conn = mysql_init(NULL);

    if (mysql_real_connect(conn, "localhost", "root", "shubham@2005s",
                           "urlshort", 0, NULL, 0)) {
        mysqlAvailable = true;
        cout << "✅ MySQL connected\n";
    } else {
        cout << "⚠️ MySQL not available, using memory\n";
    }
#else
    cout << "⚠️ MySQL disabled at compile time\n";
#endif
}

// ================= INIT REDIS =================
void initRedis() {
#ifdef USE_REDIS
    try {
        redisClient = make_unique<Redis>("tcp://127.0.0.1:6379");
        redisClient->ping();
        redisAvailable = true;
        cout << "✅ Redis connected\n";
    } catch (...) {
        cout << "⚠️ Redis not available, using memory\n";
    }
#else
    cout << "⚠️ Redis disabled at compile time\n";
#endif
}

// ================= SHORT URL GENERATOR =================
string generateShortUrl(const string &longUrl) {

    // ===== MYSQL PATH =====
#ifdef USE_MYSQL
    if (mysqlAvailable) {
        char escaped[2048];
        mysql_real_escape_string(conn, escaped, longUrl.c_str(), longUrl.length());

        string query = "INSERT INTO urls (long_url) VALUES ('" + string(escaped) + "')";

        if (mysql_query(conn, query.c_str()) == 0) {
            long long id = mysql_insert_id(conn);
            string shortCode = encodeBase62(id);

            string updateQuery =
                "UPDATE urls SET short_code='" + shortCode +
                "' WHERE id=" + to_string(id);

            mysql_query(conn, updateQuery.c_str());

#ifdef USE_REDIS
            if (redisAvailable) redisClient->set(shortCode, longUrl);
#endif

            return shortCode;
        }
    }
#endif

    // ===== MEMORY FALLBACK =====
    long long id = memoryId++;
    string shortCode = encodeBase62(id);
    memoryStore[shortCode] = longUrl;
    return shortCode;
}

// ================= MAIN =================
int main() {

    initDB();
    initRedis();

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([] {
        return "URL Shortener is running 🚀";
    });

    // ===== SHORTEN =====
    CROW_ROUTE(app, "/shorten")
    ([&](const crow::request &req) {
        auto url = req.url_params.get("url");
        if (!url) return crow::response(400, "Missing URL");

        string shortCode = generateShortUrl(url);
        return crow::response("Short URL: http://localhost:8080/" + shortCode);
    });

    // ===== REDIRECT =====
    CROW_ROUTE(app, "/<string>")
    ([&](string code) {

#ifdef USE_REDIS
        if (redisAvailable) {
            auto val = redisClient->get(code);
            if (val) {
                crow::response r;
                r.code = 302;
                r.add_header("Location", *val);
                return r;
            }
        }
#endif

        // memory fallback
        if (memoryStore.count(code)) {
            crow::response r;
            r.code = 302;
            r.add_header("Location", memoryStore[code]);
            return r;
        }

        return crow::response(404, "Not found");
    });

    cout << "🚀 Server running on http://localhost:8080\n";
    app.port(8080).multithreaded().run();
}