#include <algorithm>
#include <string>
#include <vector>
#include "LCS.hpp"

int findLCS(const std::string& x, const std::string& y) {
    std::vector<std::vector<int>> C(
        x.size() + 1, std::vector<int>(y.size() + 1, 0));

    for (std::string::size_type i = 1; i <= x.size(); ++i) {
        for (std::string::size_type j = 1; j <= y.size(); ++j) {
            if (x[i - 1] == y[j - 1]) {
                C[i][j] = C[i - 1][j - 1] + 1;
            } else {
                C[i][j] = std::max(C[i - 1][j], C[i][j - 1]);
            }
        }
    }

    return C[x.size()][y.size()];
}
