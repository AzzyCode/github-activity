#include <iostream>
#include <string>
#include <curl/curl.h> // HTTP requests
#include "json.hpp"

using json = nlohmann::json;

// Callback to collect response data
size_t writeCallback(void* data, size_t size, size_t nmemb, std::string* s) {
    size_t realSize = size * nmemb; // Size of each data element * numbers of data elements
    try {
        s->append(static_cast<char*>(data), realSize); // Append the data to the string
    } catch(const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return 0;
    }
    return realSize;
}

// Fetch GitHub activity for a given username
std::string fetchGitHubActivity(const std::string& username) {
    CURL* curl = curl_easy_init(); //Initialize cURL handle
    if (!curl) {
        std::cerr << "Failed to initialize cURL" << std::endl;
        return "";
    }

    std::string readBuffer;
    std::string url = "https://api.github.com/users/" + username + "/events/public";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Set the URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback); // Set Callback function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // Pass the string to store data
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "github-activity-cli");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

    CURLcode res = curl_easy_perform(curl); // Request
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (res != CURLE_OK) {
        std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
    } else if (http_code == 403) {
        std::cerr << "API rate limit exceeded. Consider using an API token.";
    } else if (http_code != 200) {
        std::cerr << "HTPP error: " << http_code << std::endl;
    }

    curl_easy_cleanup(curl);
    return readBuffer;
}

// Display formatted GitHub activity
void displayActivity(const std::string& jsonString) {
    if (jsonString.empty()) {
        std::cerr << "No data to display" << std::endl;
        return;
    }
    
    try {
        json jsonData = json::parse(jsonString);
        if (!jsonData.is_array() || jsonData.empty()) {
            std::cout << "No recent activity found." << std::endl;
            return;
        }
        
        std::cout << "\n\nRecent GitHub Activity:\n";
        std::cout << std::string(50, '-') << "\n";
        for (const auto& event : jsonData) {
            std::string type = event.value("type", "Unknown");
            std::string repoName = event["repo"].value("name", "N/A");
            std::string createdAt = event.value("created_at", "N/A");
    
        // Truncate timestamp for readability
        createdAt = createdAt.substr(0, 19);
        
        std::cout   
        << "Event: " << type << "\n"
        << "Repo: " << repoName << "\n"
        << "Time: " << createdAt << "\n"
        << std::string(50, '-') << "\n"; 

        }
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

int main() {
    std::string username;
    std::cout << "Enter GitHub username: ";
    std::getline(std::cin, username);

    if (username.empty()) {
        std::cerr << "Username cannot be empty" << std::endl;
        return 1;
    }

    std::string activityJson = fetchGitHubActivity(username);
    displayActivity(activityJson);

    return 0;
}









