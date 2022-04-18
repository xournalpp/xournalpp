/**
 * Xournal++
 *
 * @brief Type for parameters of points on a path.
 * Similar to std::pair<size_t, double>, but with named variables
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

struct PathParameter {
    PathParameter() = default;
    PathParameter(size_t index, double t): index(index), t(t) {}
    ~PathParameter() = default;
    bool operator==(const PathParameter& p) const { return index == p.index && t == p.t; };
    bool operator!=(const PathParameter& p) const { return !(*this == p); };
    bool operator<(const PathParameter& p) const { return index < p.index || (index == p.index && t < p.t); };
    bool operator>(const PathParameter& p) const { return index > p.index || (index == p.index && t > p.t); };
    bool operator<=(const PathParameter& p) const { return index < p.index || (index == p.index && t <= p.t); };
    bool operator>=(const PathParameter& p) const { return index > p.index || (index == p.index && t >= p.t); };

    bool isValid() const { return t <= 1.0 && (t > 0.0 || (t == 0.0 && index == 0U)); }

    size_t index;
    double t;
};
