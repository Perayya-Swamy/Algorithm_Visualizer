#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;
using Clock = chrono::steady_clock;

string getBody(const string& request) {
    size_t p = request.find("\r\n\r\n");
    return p == string::npos ? "" : request.substr(p + 4);
}

string getStringField(const string& body, const string& key, const string& fallback) {
    string token = "\"" + key + "\"";
    size_t p = body.find(token);
    if (p == string::npos) return fallback;
    p = body.find(':', p + token.size());
    if (p == string::npos) return fallback;
    p = body.find('"', p + 1);
    if (p == string::npos) return fallback;
    size_t e = body.find('"', p + 1);
    if (e == string::npos) return fallback;
    return body.substr(p + 1, e - p - 1);
}

vector<int> getArrayField(const string& body) {
    vector<int> values;
    size_t p = body.find("\"values\"");
    if (p == string::npos) return values;
    p = body.find('[', p);
    size_t e = body.find(']', p);
    if (p == string::npos || e == string::npos) return values;
    string inside = body.substr(p + 1, e - p - 1);
    stringstream ss(inside);
    string part;
    while (getline(ss, part, ',')) {
        try {
            size_t used = 0;
            int n = stoi(part, &used);
            if (used > 0) values.push_back(n);
        } catch (...) {}
    }
    return values;
}

string jsonArray(const vector<int>& a) {
    string out = "[";
    for (size_t i = 0; i < a.size(); ++i) {
        if (i) out += ",";
        out += to_string(a[i]);
    }
    return out + "]";
}

string escapeJson(const string& s) {
    string out;
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        if (c == '\n') { out += "\\n"; continue; }
        out += c;
    }
    return out;
}

struct Result {
    vector<int> values;
    vector<vector<int>> steps;
    long long comparisons = 0;
    long long swaps = 0;
    long long passes = 0;
    int foundIndex = -1;
    double milliseconds = 0;
    string algorithm;
    string complexity;
    string explanation;
};

Result runAlgorithm(vector<int> values, const string& algorithm, int target ) {
    Result r;
    r.values = values;
    r.algorithm = algorithm;
    auto start = Clock::now();
    if (values.empty()) {
        r.explanation = "Enter at least one integer.";
        r.complexity = "N/A";
        r.milliseconds = 0;
        return r;
    }

    if (algorithm == "bubble") {
        r.complexity = "O(n^2)";
        r.explanation = "Repeatedly compare adjacent values and swap them when they are out of order.";
        r.steps.push_back(values);
        for (size_t i = 0; i < values.size(); ++i) {
            bool changed = false;
            for (size_t j = 0; j + 1 < values.size() - i; ++j) {
                ++r.comparisons;
                if (values[j] > values[j + 1]) {
                    swap(values[j], values[j + 1]);
                    ++r.swaps;
                    changed = true;
                    r.steps.push_back(values);
                }
            }
            ++r.passes;
            if (!changed) break;
        }
        r.values = values;
        if (r.passes == 1 && r.swaps == 0) {
    r.complexity = "O(n) - Best case";
} else {
    r.complexity = "O(n^2) - Average/Worst case";
}
    } else if (algorithm == "selection") {
        r.complexity = "O(n^2) - Best/Average/Worst case";
        r.explanation = "Find the smallest remaining value and place it at the next position.";
         r.steps.push_back(values);
        for (size_t i = 0; i < values.size(); ++i) {
            size_t minIndex = i;
            for (size_t j = i + 1; j < values.size(); ++j) {
                ++r.comparisons;
                if (values[j] < values[minIndex]) minIndex = j;
            }
            if (minIndex != i) {
                swap(values[i], values[minIndex]);
                ++r.swaps;
                r.steps.push_back(values);
            }
            ++r.passes;
        }
        r.values = values;
        
    }else if (algorithm == "insertion") {
    r.explanation =
        "Insert each element into its correct position in the sorted left section.";

    vector<int> originalValues = values;
     r.steps.push_back(values);
    for (size_t i = 1; i < values.size(); ++i) {
        int key = values[i];
        int j = static_cast<int>(i) - 1;

        while (j >= 0) {
            ++r.comparisons;

            if (values[j] <= key) {
                break;
            }

            values[j + 1] = values[j];
            --j;
            ++r.swaps; // Counts shifts
            r.steps.push_back(values);
        }

        values[j + 1] = key;

        // Save only the valid array after inserting the key.
        r.steps.push_back(values);
        ++r.passes;
    }

    if (std::is_sorted(originalValues.begin(), originalValues.end())) {
        r.complexity = "O(n) - Best case";
    } else {
        r.complexity = "O(n^2) - Average/Worst case";
    }

    r.values = values;
}
    else if (algorithm == "linear") {
        r.complexity = "O(n)";
        r.explanation = "Check each value from left to right until the target is found.";
        for (size_t i = 0; i < values.size(); ++i) {
            ++r.comparisons;
            r.steps.push_back(values);
            if (values[i] == target) {
                r.foundIndex = static_cast<int>(i);
                break;
            }
        }
        if (r.foundIndex == 0) {
    r.complexity = "O(1) - Best case";
} else if (r.foundIndex > 0) {
    r.complexity = "O(n) - Average case ";
} else {
    r.complexity = "O(n) - Worst case (not found)";
}
    } else if (algorithm == "binary") {
        r.complexity = "O(log n)";
        r.explanation = "Search a sorted array by repeatedly discarding half of the remaining range.";
        r.steps.push_back(values);
        sort(values.begin(), values.end());
        int left = 0, right = static_cast<int>(values.size()) - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            ++r.comparisons;
            r.steps.push_back(values);
            if (values[mid] == target) {
                r.foundIndex = mid;
                break;
            }
            if (values[mid] < target) left = mid + 1;
            else right = mid - 1;
        }
        r.values = values;
      if (r.foundIndex == 0) {
    r.complexity = "O(1) - Best case";
} else if (r.foundIndex > 0) {
    r.complexity = "O(log n) - Average case (approx.)";
} else {
    r.complexity = "O(log n) - Worst case (not found)";
}}


    else {
        r.explanation = "Unknown algorithm selected.";
        r.complexity = "N/A";
    }

    if (r.steps.empty()) r.steps.push_back(r.values);
    auto end = Clock::now();
    r.milliseconds = chrono::duration<double, milli>(end - start).count();
    return r;
}

string resultJson(const Result& r, int target) {
    string out = "{";
    out += "\"ok\":true,";
    out += "\"algorithm\":\"" + escapeJson(r.algorithm) + "\",";
    out += "\"values\":" + jsonArray(r.values) + ",";
    out += "\"steps\":[";
    for (size_t i = 0; i < r.steps.size(); ++i) {
        if (i) out += ",";
        out += jsonArray(r.steps[i]);
    }
    out += "],";
    out += "\"comparisons\":" + to_string(r.comparisons) + ",";
    out += "\"swaps\":" + to_string(r.swaps) + ",";
    out += "\"passes\":" + to_string(r.passes) + ",";
    out += "\"milliseconds\":" + to_string(r.milliseconds) + ",";
    out += "\"complexity\":\"" + r.complexity + "\",";
    out += "\"explanation\":\"" + escapeJson(r.explanation) + "\",";
    out += "\"target\":" + to_string(target) + ",";
    out += "\"foundIndex\":" + to_string(r.foundIndex);
    out += "}";
    return out;
}

void sendResponse(SOCKET client, int status, const string& statusText, const string& contentType, const string& body) {
    string header = "HTTP/1.1 " + to_string(status) + " " + statusText + "\r\n";
    header += "Content-Type: " + contentType + "; charset=utf-8\r\n";
    header += "Access-Control-Allow-Origin: *\r\n";
    header += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    header += "Access-Control-Allow-Headers: Content-Type\r\n";
    header += "Content-Length: " + to_string(body.size()) + "\r\n";
    header += "Connection: close\r\n\r\n";
    string response = header + body;
    send(client, response.c_str(), static_cast<int>(response.size()), 0);
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Could not start Winsock.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Could not create socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(8080);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        cerr << "Could not bind port 8080. Is another app using it?\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    listen(serverSocket, SOMAXCONN);
    cout << "AlgoLab C++ server running at http://127.0.0.1:8080\n";
    cout << "Keep this window open while using the website.\n";

    while (true) {
        SOCKET client = accept(serverSocket, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;
        string request;
        char buffer[4096];
        int received;
        do {
            received = recv(client, buffer, sizeof(buffer), 0);
            if (received > 0) request.append(buffer, received);
        } while (received == static_cast<int>(sizeof(buffer)) && request.find("\r\n\r\n") == string::npos);

        if (request.rfind("OPTIONS ", 0) == 0) {
            sendResponse(client, 204, "No Content", "text/plain", "");
        } else if (request.rfind("GET /health", 0) == 0) {
            sendResponse(client, 200, "OK", "application/json", "{\"ok\":true,\"message\":\"C++ server connected\"}");
        } else if (request.rfind("POST /run", 0) == 0) {
            string body = getBody(request);
            string algorithm = getStringField(body, "algorithm", "bubble");
            vector<int> values = getArrayField(body);
            string targetText = getStringField(body, "target", "0");
            int target = 0;
            try { target = stoi(targetText); } catch (...) {}
            if (values.size() > 100) {
                sendResponse(client, 400, "Bad Request", "application/json", "{\"ok\":false,\"error\":\"Please use 100 values or fewer.\"}");
            } else {
                Result r = runAlgorithm(values, algorithm, target);
                sendResponse(client, 200, "OK", "application/json", resultJson(r, target));
            }
        } else {
            sendResponse(client, 404, "Not Found", "application/json", "{\"ok\":false,\"error\":\"Route not found\"}");
        }
        shutdown(client, SD_SEND);
        closesocket(client);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
