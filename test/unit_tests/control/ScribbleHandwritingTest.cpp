#include <algorithm>
#include <cmath>
#include <vector>

#include <config-test.h>
#include <gtest/gtest.h>

#include "control/tools/ScribbleGestureRecognizer.h"
#include "control/xojfile/LoadHandler.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/XojPage.h"

TEST(ScribbleHandwritingTest, preservesExistingHandwritingFixture) {
    auto document = LoadHandler{}.loadDocument(GET_TESTFILE(u8"benchmark/handwritten-text.xopp"));
    size_t strokes = 0;
    for (size_t i = 0; i < document->getPageCount(); ++i) {
        for (const auto& layer: document->getPage(i)->getLayers()) {
            for (const auto& element: layer->getElements()) {
                if (element->getType() != ELEMENT_STROKE) {
                    continue;
                }
                SCOPED_TRACE(strokes++);
                const auto& stroke = static_cast<const Stroke&>(*element);
                EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(stroke.getPointVector()));
            }
        }
    }
    EXPECT_GT(strokes, 100);
    RecordProperty("strokes", static_cast<int>(strokes));
}

TEST(ScribbleHandwritingTest, recognizesRecordedUnevenScrubsAndPreservesRecordedWriting) {
    auto document = LoadHandler{}.loadDocument(GET_TESTFILE(u8"scribble/uneven-scrubs.xopp"));
    ASSERT_NE(document, nullptr);
    ASSERT_EQ(document->getPageCount(), 3);
    for (size_t page = 0; page < 2; ++page) {
        const auto& elements = document->getPage(page)->getSelectedLayer()->getElements();
        ASSERT_EQ(elements.size(), page == 0 ? 6 : 7);
        for (size_t i = 0; i < elements.size(); ++i) {
            SCOPED_TRACE(testing::Message() << "page=" << page << " stroke=" << i);
            const auto& stroke = static_cast<const Stroke&>(*elements[i]);
            EXPECT_EQ(ScribbleGestureRecognizer::isCandidate(stroke.getPointVector()), page == 0);
        }
    }
}

TEST(ScribbleHandwritingTest, preservesRecordedDecisionsAcrossTransformsAndEventDuplication) {
    auto recorded = LoadHandler{}.loadDocument(GET_TESTFILE(u8"scribble/uneven-scrubs.xopp"));
    ASSERT_NE(recorded, nullptr);
    size_t cases = 0;
    for (size_t page = 0; page < 2; ++page) {
        size_t sample = 0;
        for (const auto& element: recorded->getPage(page)->getSelectedLayer()->getElements()) {
            const auto& original = static_cast<const Stroke&>(*element).getPointVector();
            for (double angle: {0.0, 0.5, 1.5707963267948966, 2.7}) {
                // Stay above the intentional four-logical-unit minimum extent.
                for (double scale: {0.5, 1.0, 4.0, 10.0}) {
                    for (bool reverse: {false, true}) {
                        for (bool duplicate: {false, true}) {
                            SCOPED_TRACE(testing::Message()
                                         << "page=" << page << " sample=" << sample << " angle=" << angle
                                         << " scale=" << scale << " reverse=" << reverse << " duplicate=" << duplicate);
                            std::vector<Point> points;
                            for (const auto& p: original) {
                                Point transformed{500 + scale * (p.x * std::cos(angle) - p.y * std::sin(angle)),
                                                  -300 + scale * (p.x * std::sin(angle) + p.y * std::cos(angle))};
                                points.push_back(transformed);
                                if (duplicate) {
                                    points.push_back(transformed);
                                }
                            }
                            if (reverse) {
                                std::reverse(points.begin(), points.end());
                            }
                            EXPECT_EQ(ScribbleGestureRecognizer::isCandidate(points), page == 0);
                            ++cases;
                        }
                    }
                }
            }
            ++sample;
        }
    }
    RecordProperty("recorded_transform_cases", static_cast<int>(cases));
}

TEST(ScribbleHandwritingTest, recordsSensitivityTradeoffsWithoutRelaxingDefaultPreservation) {
    auto document = LoadHandler{}.loadDocument(GET_TESTFILE(u8"benchmark/handwritten-text.xopp"));
    ASSERT_NE(document, nullptr);
    for (int sensitivity: {0, 25, 50, 75, 100}) {
        for (int passes: {2, 4, 8}) {
            unsigned candidates = 0;
            for (size_t i = 0; i < document->getPageCount(); ++i) {
                for (const auto& layer: document->getPage(i)->getLayers()) {
                    for (const auto& element: layer->getElements()) {
                        if (element->getType() == ELEMENT_STROKE &&
                            ScribbleGestureRecognizer::isCandidate(
                                    static_cast<const Stroke&>(*element).getPointVector(), {sensitivity, passes}))
                            ++candidates;
                    }
                }
            }
            RecordProperty("candidate_s" + std::to_string(sensitivity) + "_p" + std::to_string(passes),
                           static_cast<int>(candidates));
            if (sensitivity == 50 && passes == 4)
                EXPECT_EQ(candidates, 0);
        }
    }
}
