#include "crow.h"
#include "base62.h"
#include <mysql/mysql.h>
#include <sw/redis++/redis++.h>

using namespace std;
using namespace sw::redis;

Redis redis("tcp://127.0.0.1:6379");

MYSQL *conn;

void initDB() {
    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "",
                       "urlshort", 0, NULL, 0);
}

string generateShortUrl(const string &longUrl) {
    string query = "INSERT INTO urls (long_url) VALUES ('" + longUrl + "')";
    mysql_query(conn, query.c_str());

    long long id = mysql_insert_id(conn);
    string shortCode = encodeBase62(id);

    string update =
        "UPDATE urls SET short_code='" + shortCode +
        "' WHERE id=" + to_string(id);
    mysql_query(conn, update.c_str());

    return shortCode;
}

string getLongUrl(const string &code) {
    auto val = redis.get(code);
    if (val) return *val;

    string query =
        "SELECT long_url FROM urls WHERE short_code='" + code + "'";
    mysql_query(conn, query.c_str());

    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (!row) return "";

    string longUrl = row[0];
    redis.set(code, longUrl);

    return longUrl;
}

int main(int argc, char* argv[]) {
    initDB();
    crow::SimpleApp app;

    int port = 8080;
    if (argc > 1) {
        port = stoi(argv[1]);
    }

    CROW_ROUTE(app, "/shorten")
    ([&](const crow::request &req) {
        auto url = req.url_params.get("url");
        if (!url) return crow::response(400);

        string code = generateShortUrl(url);
        return crow::response("http://localhost:" + to_string(port) + "/" + code);
    });

    CROW_ROUTE(app, "/<string>")
    ([&](string code) {
        string url = getLongUrl(code);
        if (url.empty()) return crow::response(404);
        crow::response res;
        res.code = 302;
        res.add_header("Location", url);
        return res;
    });

    app.port(port).multithreaded().run();
}
