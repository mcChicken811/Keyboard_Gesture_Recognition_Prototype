#ifndef GESTURE_RECOGNIZER_HPP
#define GESTURE_RECOGNIZER_HPP

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

enum class Gesture : std::size_t {
    SWIPE_LEFT,
    SWIPE_RIGHT,
    STIR,
    TRIANGLE,
    COUNT
};

struct GestureScore {
    Gesture gesture;
    double score;
};

class GestureRecognizer {
public:
    using GestureData = std::array<std::vector<std::string>,
                                   static_cast<std::size_t>(Gesture::COUNT)>;

    explicit GestureRecognizer(const std::string& csvPath);

    // Rows are indexed by Gesture; columns hold that gesture's input samples.
    const std::vector<std::string>& samples(Gesture gesture) const;

    // Score = average of LCS(input, sample) / sample length.
    // Returns populated gestures in descending score order, breaking ties by enum.
    std::vector<GestureScore> rank(const std::string& input) const;

    static std::string_view gestureName(Gesture gesture);

private:
    GestureData data_;
};

#endif
