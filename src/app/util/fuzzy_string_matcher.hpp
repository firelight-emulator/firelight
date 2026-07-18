#pragma once
#include <QString>
#include <QVector>

class FuzzyStringMatcher {
public:
    // Returns an integer score. Higher = better match. Returns -1 if it doesn't match
    static int match(const QString &text, const QString &pattern) {
        if (pattern.isEmpty()) return 0;
        if (text.isEmpty()) return -1;

        QString t = text.toLower();
        QString p = pattern.toLower();

        int score = 0;
        int tIdx = 0;
        int pIdx = 0;
        int consecutiveMatches = 0;

        while (tIdx < t.length() && pIdx < p.length()) {
            if (t.at(tIdx) == p.at(pIdx)) {
                // Base match points
                score += 10;

                // Bonus for consecutive hits
                if (consecutiveMatches > 0) score += 5;

                // Bonus for matching start of words or camelCase / snake_case boundaries
                if (tIdx == 0 || text.at(tIdx).isUpper() || text.at(tIdx - 1) == '_' || text.at(tIdx - 1) == ' ') {
                    score += 20;
                }

                pIdx++;
                consecutiveMatches++;
            } else {
                consecutiveMatches = 0;
                // Light penalty for skipping characters to prefer tighter matches
                score -= 1;
            }
            tIdx++;
        }

        // Must match the full pattern to be considered valid
        return (pIdx == p.length()) ? score : -1;
    }
};
