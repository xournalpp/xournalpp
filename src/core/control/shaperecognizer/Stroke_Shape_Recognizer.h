#pragma once
#ifndef SHAPE_RECOGNIZER_H
#define SHAPE_RECOGNIZER_H

#include <cmath>

#include <config-debug.h>

#include "../core/model/Stroke.h"

class StrokeSegment {
public:
    StrokeSegment(const std::vector<Point>& points, size_t start, size_t end);
    StrokeSegment(const StrokeSegment& segment, const StrokeSegment& next_segment);  // fuse adjacent segments
    bool is_line(double max_minor_is_line) const;
    Point project_onto_line(const Point& point) const;
    Point intersect(const StrokeSegment& other) const;
    double cos_angle_with(const StrokeSegment& other) const;
    Point mean() const;
    Point axis() const;
    size_t start() const;
    size_t end() const;

protected:
    void compute_main_axis();
    size_t start_ind, end_ind;
    std::array<double, 3> cov_accumulator;
    std::array<double, 2> mean_accumulator;
    Point center;
    Point main_axis;  // unit length
    double minor, major;
};

struct RecognizerParameter {
    RecognizerParameter():
            max_nb_segments{10},
            min_nb_points_per_segment{7},
            initial_nb_segments{10},
            nb_segments_per_split{4},
            maximal_segment_cos_angle{deg_to_rad(12)},
            max_minor_is_line{0.35},
            close_polygon_tolerance{5},
            regularize_angle_deviation{deg_to_rad(10)},
            regularize_rel_radius_deviation{0.25},
            regularize_rectangle_rel_length_deviation{0.1} {}

    size_t max_nb_segments;
    size_t min_nb_points_per_segment;
    size_t initial_nb_segments;
    size_t nb_segments_per_split;
    double maximal_segment_cos_angle;
    double max_minor_is_line;
    double close_polygon_tolerance;
    double regularize_angle_deviation;
    double regularize_rel_radius_deviation;
    double regularize_rectangle_rel_length_deviation;

    static double deg_to_rad(double deg) { return std::cos(std::atan(1) * double(8) * deg / double(360)); }
};

class Recognizer {
public:
    Recognizer(RecognizerParameter parameter = RecognizerParameter());
    void set_parameter(RecognizerParameter parameter);

    std::unique_ptr<Stroke> recognize(const Stroke* strk);

protected:
    bool check_minimal_size();
    bool is_polygon();
    void fuse_split_segments(size_t segment_size_factor);
    bool fuse_split_final();
    void split_into_segments(const std::vector<Point>& points, size_t start, size_t end, size_t segment_size_factor);
    size_t nb_non_linear_segments(bool& splittable);
    size_t decrease_segment_size_factor(size_t segment_size_factor);
    bool segment_is_splittable(const StrokeSegment& segmnent);
    void segments_to_polygon();
    bool regularize_polygon();
    bool stabilize_parallelogram();
    bool stabilize_triangles();
    bool check_minimal_angle();
    // bool is_circle();
    inline static double sign(double val) { return double(val > 0) - double(val < 0); }

    const Stroke* stroke;
    RecognizerParameter parameter;
    bool seg_selector;
    std::array<std::vector<StrokeSegment>, 2> segments;
    std::vector<Point> polygon;
    struct Polar {
        Polar(double r, double phi): r{r}, phi{phi} {}
        double r;
        double phi;
    };
    std::vector<Polar> polar;

    const double pi_2{std::atan(1) * double(2)};
};
#endif  // SHAPE_RECOGNIZER_H
