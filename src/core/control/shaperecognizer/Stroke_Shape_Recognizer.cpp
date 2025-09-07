#include "Stroke_Shape_Recognizer.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>

#include "model/Point.h"

StrokeSegment::StrokeSegment(const std::vector<Point>& points, size_t start, size_t end):
        start_ind{start}, end_ind{end}, cov_accumulator{}, mean_accumulator{} {
    assert(start < end);
    for (size_t ind{start}; ind < end; ++ind) {
        mean_accumulator[0] += points[ind].x;
        mean_accumulator[1] += points[ind].y;
        cov_accumulator[0] += std::pow(points[ind].x, 2);
        cov_accumulator[1] += points[ind].x * points[ind].y;
        cov_accumulator[2] += std::pow(points[ind].y, 2);
    }
    compute_main_axis();
}

StrokeSegment::StrokeSegment(const StrokeSegment& segment, const StrokeSegment& next_segment):
        start_ind{segment.start()}, end_ind{next_segment.end()} {
    assert(segment.end() == next_segment.start());
    for (size_t ind{0}; ind < cov_accumulator.size(); ++ind)
        cov_accumulator[ind] = segment.cov_accumulator[ind] + next_segment.cov_accumulator[ind];
    for (size_t ind{0}; ind < mean_accumulator.size(); ++ind)
        mean_accumulator[ind] = segment.mean_accumulator[ind] + next_segment.mean_accumulator[ind];
    compute_main_axis();
}

void StrokeSegment::compute_main_axis() {
    const double inv_weight{double(1) / double(end_ind - start_ind)};
    const double sq_inv_weight{std::pow(inv_weight, 2)};
    center.x = mean_accumulator[0] * inv_weight;
    center.y = mean_accumulator[1] * inv_weight;
    std::array<double, 3> covariance{};
    covariance[0] = cov_accumulator[0] * inv_weight - mean_accumulator[0] * mean_accumulator[0] * sq_inv_weight;
    covariance[1] = cov_accumulator[1] * inv_weight - mean_accumulator[0] * mean_accumulator[1] * sq_inv_weight;
    covariance[2] = cov_accumulator[2] * inv_weight - mean_accumulator[1] * mean_accumulator[1] * sq_inv_weight;
    const double half_trace{(covariance[0] + covariance[2]) / double(2)};
    const double shift{std::sqrt(pow(half_trace, 2) - covariance[0] * covariance[2] + covariance[1] * covariance[1])};
    minor = std::max(half_trace - shift, std::numeric_limits<double>::epsilon());
    major = std::max(half_trace + shift, minor);
    main_axis.x = half_trace + shift - covariance[2];
    main_axis.y = covariance[1];
    const double inv_lenght{double(1) / std::hypot(main_axis.x, main_axis.y)};
    main_axis.x *= inv_lenght;
    main_axis.y *= inv_lenght;
}

Point StrokeSegment::project_onto_line(const Point& point) const {
    double ip{(point.x - center.x) * main_axis.x + (point.y - center.y) * main_axis.y};
    return Point(center.x + ip * main_axis.x, center.y + ip * main_axis.y);
}

Point StrokeSegment::intersect(const StrokeSegment& other) const {
    Point h(center.x - other.center.x, center.y - other.center.y);
    const double t_other{(h.x * main_axis.y - h.y * main_axis.x) /
                         (other.main_axis.x * main_axis.y - other.main_axis.y * main_axis.x)};
    return Point(other.center.x + t_other * other.main_axis.x, other.center.y + t_other * other.main_axis.y);
}

inline double StrokeSegment::cos_angle_with(const StrokeSegment& other) const {
    return main_axis.x * other.main_axis.x + main_axis.y * other.main_axis.y;
}

inline bool StrokeSegment::is_line(double max_minor_is_line) const { return minor < max_minor_is_line; }

inline Point StrokeSegment::mean() const { return center; }

inline Point StrokeSegment::axis() const { return main_axis; }

inline size_t StrokeSegment::start() const { return start_ind; }

inline size_t StrokeSegment::end() const { return end_ind; }


Recognizer::Recognizer(RecognizerParameter parameter): parameter(std::move(parameter)) {}

inline void Recognizer::set_parameter(RecognizerParameter parameter) { this->parameter = std::move(parameter); }

std::unique_ptr<Stroke> Recognizer::recognize(const Stroke* strk) {
#ifdef DEBUG_RECOGNIZER
    g_message("- Recognizer.recognize -");
#endif
    stroke = strk;
    if (stroke == nullptr or !check_minimal_size())
        return nullptr;

    if (is_polygon() && check_minimal_angle()) {
        segments_to_polygon();
        auto new_stroke = std::make_unique<Stroke>();
        new_stroke->applyStyleFrom(stroke);
        new_stroke->setPointVector(polygon);
        return new_stroke;
    }
    return nullptr;
}

inline bool Recognizer::check_minimal_size() {
    return stroke->getPointCount() > 2 * parameter.min_nb_points_per_segment;
}

inline size_t Recognizer::decrease_segment_size_factor(size_t segment_size_factor) {
    return std::max(size_t(double(segment_size_factor) / double(parameter.nb_segments_per_split)), size_t(1));
}

bool Recognizer::is_polygon() {
    seg_selector = true;
    segments[!seg_selector].clear();
    size_t nb_points{stroke->getPointCount()};
    size_t segment_size_factor{std::max(
            size_t(double(nb_points) / double(parameter.min_nb_points_per_segment * parameter.initial_nb_segments)),
            size_t(1))};
    split_into_segments(stroke->getPointVector(), 0, nb_points, segment_size_factor);
    segment_size_factor = decrease_segment_size_factor(segment_size_factor);
    seg_selector = !seg_selector;

    bool splittable{false};
    size_t nb_non_linear{nb_non_linear_segments(splittable)};
    while (nb_non_linear > 0 && splittable) {
        if (nb_non_linear >= parameter.max_nb_segments) {
#ifdef DEBUG_RECOGNIZER
            g_message("- Recognizer: max_nb_segments reached -");
#endif
            return false;
        }
        fuse_split_segments(segment_size_factor);
        segment_size_factor = decrease_segment_size_factor(segment_size_factor);
        nb_non_linear = nb_non_linear_segments(splittable);
    }
    fuse_split_segments(segment_size_factor);  // fusing
    if (nb_non_linear == 0)
        return true;
    return fuse_split_final();
}

inline bool Recognizer::segment_is_splittable(const StrokeSegment& segment) {
    return segment.end() > 2 * parameter.min_nb_points_per_segment + segment.start();
}

void Recognizer::split_into_segments(const std::vector<Point>& points, size_t start, size_t end,
                                     size_t segment_size_factor) {
    assert(segment_size_factor >= 1);
    const size_t increment{parameter.min_nb_points_per_segment * segment_size_factor};
    size_t current_end{start};
    do {
        size_t start{current_end};
        current_end = std::min(start + increment, end);
        if (end < current_end + parameter.min_nb_points_per_segment)
            current_end = end;
        segments[!seg_selector].push_back(StrokeSegment(points, start, current_end));
    } while (current_end < end);
}

void Recognizer::fuse_split_segments(size_t segment_size_factor) {
    segments[!seg_selector].clear();
    size_t working_ind{0};
    size_t end{segments[seg_selector].size()};
    while (working_ind < end) {
        const StrokeSegment& segment{segments[seg_selector][working_ind]};
        if (!segment.is_line(parameter.max_minor_is_line)) {
            if (segment_is_splittable(segment))
                split_into_segments(stroke->getPointVector(), segment.start(), segment.end(), segment_size_factor);
            else
                segments[!seg_selector].push_back(segment);
        } else if (!segments[!seg_selector].empty()) {
            StrokeSegment fused{segments[!seg_selector].back(), segment};
            if (fused.is_line(parameter.max_minor_is_line))
                segments[!seg_selector].back() = fused;
            else
                segments[!seg_selector].push_back(segment);
        } else
            segments[!seg_selector].push_back(segment);
        ++working_ind;
    }
    seg_selector = !seg_selector;
}

bool Recognizer::fuse_split_final() {
    segments[!seg_selector].clear();
    size_t working_ind{0};
    size_t end{segments[seg_selector].size()};
    while (working_ind < end) {
        const StrokeSegment& segment{segments[seg_selector][working_ind]};
        if (segment.is_line(parameter.max_minor_is_line))
            segments[!seg_selector].push_back(segment);
        else {
            if (working_ind == 0 || (working_ind + 1) == end) {
#ifdef DEBUG_RECOGNIZER
                g_message("- Recognizer: internal start/edn segment not linear -");
#endif
                return false;
            }

            const StrokeSegment& prev_segment{segments[!seg_selector].back()};
            const StrokeSegment& next_segment{segments[seg_selector][working_ind + 1]};

            size_t split_ind{segment.start() + 1};
            while (split_ind < segment.end()) {
                StrokeSegment fused_prev(prev_segment,
                                         StrokeSegment(stroke->getPointVector(), segment.start(), split_ind));
                StrokeSegment fused_next(StrokeSegment(stroke->getPointVector(), split_ind, segment.end()),
                                         next_segment);
                if (fused_prev.is_line(parameter.max_minor_is_line) &&
                    fused_next.is_line(parameter.max_minor_is_line)) {
                    segments[!seg_selector].back() = fused_prev;
                    segments[!seg_selector].push_back(fused_next);
                    break;
                }
                ++split_ind;
            }
            if (split_ind >= segment.end()) {
#ifdef DEBUG_RECOGNIZER
                g_message("- Recognizer: internal segment not linear -");
#endif
                return false;
            }
        }
        ++working_ind;
    }
    seg_selector = !seg_selector;
    return true;
}

size_t Recognizer::nb_non_linear_segments(bool& splittable) {
    size_t nb_non_linear{0};
    splittable = false;
    for (auto segment: segments[seg_selector]) {
        nb_non_linear += size_t(!segment.is_line(parameter.max_minor_is_line));
        splittable = splittable || (!segment.is_line(parameter.max_minor_is_line) && segment_is_splittable(segment));
    }
    return nb_non_linear;
}

bool Recognizer::check_minimal_angle() {
    for (size_t seg_ind{0}; (seg_ind + 1) < segments[seg_selector].size(); ++seg_ind) {
        const StrokeSegment& prev_segment{segments[seg_selector][seg_ind]};
        const StrokeSegment& current_segment{segments[seg_selector][seg_ind + 1]};
        if (std::abs(prev_segment.cos_angle_with(current_segment)) >= parameter.maximal_segment_cos_angle) {
#ifdef DEBUG_RECOGNIZER
            g_message("- Recognizer: minimal angle rejected -");
#endif
            return false;
        }
    }
    return true;
}

void Recognizer::segments_to_polygon() {
    constexpr double half{double(1) / double(2)};
    polygon.clear();
    const StrokeSegment& first_segment{segments[seg_selector][0]};
    Point first{first_segment.project_onto_line(stroke->getPointVector()[first_segment.start()])};
    const StrokeSegment& last_segment{segments[seg_selector][segments[seg_selector].size() - 1]};
    Point last{last_segment.project_onto_line(stroke->getPointVector()[last_segment.end() - 1])};
    bool is_closed{std::hypot(first.x - last.x, first.y - last.y) < parameter.close_polygon_tolerance};
    if (is_closed) {
        first = Point((first.x + last.x) * half, (first.y + last.y) * half);
        last = first;
    }
    polygon.push_back(first);
    for (size_t seg_ind{0}; (seg_ind + 1) < segments[seg_selector].size(); ++seg_ind) {
        const StrokeSegment& prev_segment{segments[seg_selector][seg_ind]};
        const StrokeSegment& current_segment{segments[seg_selector][seg_ind + 1]};
        Point intersection{prev_segment.intersect(current_segment)};
        polygon.push_back(intersection);
    }
    polygon.push_back(last);
    if (is_closed) {
        stabilize_triangles() || stabilize_parallelogram() || regularize_polygon();
    }
}

bool Recognizer::stabilize_parallelogram() {
    assert(polygon.front().x == polygon.back().x && polygon.front().y == polygon.back().y);  // closed polygon
    if (polygon.size() != 5)
        return false;
    std::array<double, 4> side_length;
    for (size_t ind{0}; ind < 4; ++ind)
        side_length[ind] = std::hypot(polygon[ind + 1].x - polygon[ind].x, polygon[ind + 1].y - polygon[ind].y);
    if (std::abs(side_length[2] - side_length[0]) / side_length[0] <
                parameter.regularize_rectangle_rel_length_deviation &&
        std::abs(side_length[3] - side_length[1]) / side_length[1] <
                parameter.regularize_rectangle_rel_length_deviation) {
        g_message("- Recognizer.stabilize_parallelogram not implemented yet -");
        /*
        - check angle approx. pi/2
        - check side lenght approx. equal
        - > 1,1->square or 1,0->rectangle or 0,1->rhombus or 0,0 -> rhomboid
        each case:
            - generate parametrerized hypothesis array<Point,4> hypothesis_xx(free_params_xx)
            - use local optimization (initial configuration from current polygon, derivative is easy (der. free might
        work too)) on objective(free_params_xx) = sum_of_square_distance(polygon,hypothesis_xx(free_params_xx))
        */
        return true;
    }
    return false;
}
bool Recognizer::stabilize_triangles() {
    assert(polygon.front().x == polygon.back().x && polygon.front().y == polygon.back().y);  // closed polygon
    if (polygon.size() != 4)
        return false;
#ifdef DEBUG_RECOGNIZER
    g_message("- Recognizer.stabilize_triangles not implemented yet -");
#endif
    /*
     - use same approach as with parallelogram
    */
    return false;
}

bool Recognizer::regularize_polygon() {
    assert(polygon.front().x == polygon.back().x && polygon.front().y == polygon.back().y);  // closed polygon

    Point center{};
    for (auto point: polygon) {
        center.x += point.x;
        center.y += point.y;
    }
    double scaler{double(1) / double(polygon.size())};
    center.x *= scaler;
    center.y *= scaler;

    polar.clear();
    double mean_radius{0};
    for (auto point: polygon) {
        Point h(point.x - center.x, point.y - center.y);
        const double radius{std::hypot(h.x, h.y)};
        const double phi{std::atan2(h.y, h.x)};
        polar.push_back(Polar(radius, phi));
        mean_radius += radius;
    }
    mean_radius /= double(polygon.size());

    double regular_diff_angle(pi_2 * double(4) / (double(polygon.size() - 1)));
    bool is_regular_angle{true};
    bool is_regular_radius{true};
    for (size_t ind{0}; ind < polygon.size(); ++ind) {
        double diff_angle{polar[ind].phi - polar[0].phi};
        double deviation{std::abs(diff_angle - sign(diff_angle) * regular_diff_angle * double(ind))};
        is_regular_angle &= deviation < parameter.regularize_angle_deviation;
        is_regular_radius &=
                std::abs((polar[ind].r - mean_radius) / mean_radius) < parameter.regularize_rel_radius_deviation;
    }

    if (is_regular_angle && is_regular_radius) {
        for (size_t ind{0}; ind < polygon.size(); ++ind) {
            double diff_angle{polar[ind].phi - polar[0].phi};
            polar[ind].phi = polar[0].phi + sign(diff_angle) * regular_diff_angle * double(ind);
            polar[ind].r = mean_radius;
        }
        polar.back() = polar.front();
#ifdef DEBUG_RECOGNIZER
        std::string msg{"- Recognizer: regularizing closed polygon -"};
        g_message("%s", msg.c_str());
#endif
        polygon.clear();
        for (size_t ind{0}; ind < polar.size(); ++ind)
            polygon.push_back(Point(center.x + polar[ind].r * std::cos(polar[ind].phi),
                                    center.y + polar[ind].r * std::sin(polar[ind].phi)));
        return true;
    }
    return false;
}
