#include <vector>
#include <string>
#include <fstream>
#include <map>

typedef std::map<size_t, std::vector<size_t>> student_map; // A student is an id mapping to an array of preferences
typedef std::map<size_t, size_t> supervisor_map; // A supervisor is an id mapping to a capacity

const student_map parseStudentsCsv(const std::string& csvFilename) {
    const std::string delimiter = ",";
    std::ifstream inputFs(csvFilename);
    std::string lineBuffer, token;
    size_t loc{0};
    student_map students;

    while (getline(inputFs, lineBuffer)) {
        auto pos = 0;
        size_t id;
        std::vector<size_t> preferences;

        while ((loc = lineBuffer.find(delimiter)) != std::string::npos) {  
            token = lineBuffer.substr(1, loc - 2);
            lineBuffer.erase(0, loc + delimiter.length());

            if (pos == 0) {
                id = stoi(token.substr(token.find_first_of("0123456789")));
            } else {
                preferences.push_back(stoi(token));
            }

            pos++;
        }

        token = lineBuffer.substr(1, 1);
        preferences.push_back(stoi(token));

        students[id] = move(preferences);
    }
    inputFs.close();

    return students;
}

const supervisor_map parseSupervisorsCsv(const std::string& csvFilename) {
    const std::string delimiter = ",";
    std::ifstream inputFs(csvFilename);
    std::string lineBuffer, token;
    size_t loc{0};
    supervisor_map supervisors;

    while (getline(inputFs, lineBuffer)) {
        auto pos = 0;
        size_t id;
        size_t capacity;

        while ((loc = lineBuffer.find(delimiter)) != std::string::npos) {  
            token = lineBuffer.substr(1, loc - 2);
            lineBuffer.erase(0, loc + delimiter.length());

            if (pos == 0) {
                id = stoi(token.substr(token.find_first_of("0123456789")));
            }

            pos++;
        }

        token = lineBuffer.substr(1, 1);
        capacity = stoi(token);

        supervisors[id] = capacity;
    }
    inputFs.close();

    return supervisors;
}