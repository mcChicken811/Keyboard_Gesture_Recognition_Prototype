#include "GestureRecognizer.hpp"
#include "LCS.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace {

constexpr std::array<std::string_view, static_cast<std::size_t>(Gesture::COUNT)>
    gestureNames = {
        "SWIPE_LEFT", "SWIPE_RIGHT", "STIR", "TRIANGLE", "W"
    };

std::vector<std::string> parseTsvRow(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;
    bool quoteClosed = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quoted) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    quoted = false;
                    quoteClosed = true;
                }
            } else {
                field += c;
            }
        } else if (c == '\t') {
            fields.push_back(field);
            field.clear();
            quoteClosed = false;
        } else if (quoteClosed) {
            throw std::runtime_error("Unexpected character after a quoted TSV field");
        } else if (c == '"') {
            if (!field.empty()) {
                throw std::runtime_error("Unexpected quote in a TSV field");
            }
            quoted = true;
        } else {
            field += c;
        }
    }

    if (quoted) {
        throw std::runtime_error("Unterminated quoted TSV field");
    }
    fields.push_back(field);
    return fields;
}

} // namespace

GestureRecognizer::GestureRecognizer(const std::string& dataPath) {
    std::ifstream file(dataPath);
    if (!file) {
        throw std::runtime_error("Cannot open gesture data: " + dataPath);
    }

    std::string line;
    bool headerRead = false;
    bool samplesRead = false;
    std::size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        if (lineNumber == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
            line.erase(0, 3);
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        const auto fields = parseTsvRow(line);
        if (std::all_of(fields.begin(), fields.end(),
                        [](const std::string& field) { return field.empty(); })) {
            continue;
        }
        if (!headerRead) {
            if (fields.size() < 2 || fields[0] != "Gesture" || fields[1] != "inputs") {
                throw std::runtime_error("Expected tab-delimited header: Gesture\\tinputs");
            }
            headerRead = true;
            continue;
        }

        const auto gesture = std::find(gestureNames.begin(), gestureNames.end(), fields[0]);
        if (gesture == gestureNames.end()) {
            throw std::runtime_error("Unknown gesture on TSV line " +
                                     std::to_string(lineNumber) + ": " + fields[0]);
        }

        auto& row = data_[static_cast<std::size_t>(gesture - gestureNames.begin())];
        bool rowHasSamples = false;
        for (std::size_t column = 1; column < fields.size(); ++column) {
            // Empty cells are padding; preserve all characters in actual samples.
            if (!fields[column].empty()) {
                row.push_back(fields[column]);
                rowHasSamples = true;
            }
        }
        if (!rowHasSamples) {
            throw std::runtime_error("No samples on TSV line " + std::to_string(lineNumber));
        }
        samplesRead = true;
    }

    if (file.bad()) {
        throw std::runtime_error("Failed to read gesture data: " + dataPath);
    }
    if (!headerRead || !samplesRead) {
        throw std::runtime_error("Gesture data contains no samples: " + dataPath);
    }
}

const std::vector<std::string>& GestureRecognizer::samples(Gesture gesture) const {
    return data_.at(static_cast<std::size_t>(gesture));
}

std::vector<GestureScore> GestureRecognizer::rank(const std::string& input) const {
    std::vector<GestureScore> results;
    results.reserve(data_.size());

    for (std::size_t i = 0; i < data_.size(); ++i) {
        const auto& row = data_[i];
        if (row.empty()) {
            continue;
        }

        double totalScore = 0.0;
        for (const auto& sample : row) {
            totalScore += findLCS(input, sample) / static_cast<double>(sample.size());
        }

        results.push_back({static_cast<Gesture>(i),
                           totalScore / static_cast<double>(row.size())});
    }

    std::sort(results.begin(), results.end(),
              [](const GestureScore& a, const GestureScore& b) {
                  if (a.score != b.score) {
                      return a.score > b.score;
                  }
                  return a.gesture < b.gesture;
              });
    return results;
}

std::string_view GestureRecognizer::gestureName(Gesture gesture) {
    return gestureNames.at(static_cast<std::size_t>(gesture));
}
