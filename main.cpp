#include "GestureRecognizer.hpp"
#include "GestureWindow.hpp"

#include <exception>
#include <iomanip>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <tsv_path> [input]\n"
                  << "Omit input to open the live recognition window.\n";
        return 1;
    }

    try {
        const GestureRecognizer recognizer(argv[1]);
        if (argc == 2) {
            runGestureWindow(recognizer);
            return 0;
        }
        const auto results = recognizer.rank(argv[2]);

        std::cout << std::fixed << std::setprecision(6);
        for (std::size_t i = 0; i < results.size(); ++i) {
            std::cout << i + 1 << ". "
                      << GestureRecognizer::gestureName(results[i].gesture)
                      << ": " << results[i].score / results[0].score << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
