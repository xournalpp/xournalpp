#include <gtest/gtest.h>
#include "control/shaperecognizer/ShapeRecognizer.h"
#include "model/Stroke.h"
#include "model/Point.h"

TEST(ShapeRecognizerTest, RecognizesThinRectangle) {
    ShapeRecognizer recognizer;
    Stroke originalStroke;
    
    originalStroke.addPoint(Point(0, 0));
    originalStroke.addPoint(Point(500, 2));    
    originalStroke.addPoint(Point(1000, 0));
    
    originalStroke.addPoint(Point(1000, 50));  
    originalStroke.addPoint(Point(1000, 100));
    
    originalStroke.addPoint(Point(500, 98));   
    originalStroke.addPoint(Point(0, 100));
    
    originalStroke.addPoint(Point(0, 50));     
    originalStroke.addPoint(Point(0, 0));      
    
    originalStroke.setWidth(2.4);
    
    auto resultStroke = recognizer.recognizePatterns(&originalStroke, 2.0);
    
    ASSERT_NE(resultStroke, nullptr) << "Error: Stroke not recognized.";
    
    double minX = resultStroke->getPoint(0).x;
    double maxX = resultStroke->getPoint(0).x;
    double minY = resultStroke->getPoint(0).y;
    double maxY = resultStroke->getPoint(0).y;

    for (size_t i = 1; i < resultStroke->getPointCount(); ++i) {
        minX = std::min(minX, resultStroke->getPoint(i).x);
        maxX = std::max(maxX, resultStroke->getPoint(i).x);
        minY = std::min(minY, resultStroke->getPoint(i).y);
        maxY = std::max(maxY, resultStroke->getPoint(i).y);
    }
    
    EXPECT_NEAR(minX, 0, 1.0);
    EXPECT_NEAR(maxX, 1000, 1.0);
    EXPECT_NEAR(minY, 0, 1.0);
    EXPECT_NEAR(maxY, 100, 1.0);
}

TEST(ShapeRecognizerTest, RecognizesVeryThinRectangle) {
    ShapeRecognizer recognizer;
    Stroke originalStroke;
    
    originalStroke.addPoint(Point(0, 0));
    originalStroke.addPoint(Point(1000, 0));
    
    originalStroke.addPoint(Point(1000, 10)); 
    originalStroke.addPoint(Point(1000, 20));
    
    originalStroke.addPoint(Point(0, 20));
    
    originalStroke.addPoint(Point(0, 10));
    originalStroke.addPoint(Point(0, 0));
    
    originalStroke.setWidth(2.4);
    
    
    auto resultStroke = recognizer.recognizePatterns(&originalStroke, 2.0);
    
    ASSERT_NE(resultStroke, nullptr) << "Error: Stroke not recognized.";
    
    double minY = resultStroke->getPoint(0).y;
    double maxY = resultStroke->getPoint(0).y;
    for (size_t i = 1; i < resultStroke->getPointCount(); ++i) {
        minY = std::min(minY, resultStroke->getPoint(i).y);
        maxY = std::max(maxY, resultStroke->getPoint(i).y);
    }
    
    EXPECT_NEAR(maxY - minY, 20.0, 1.0) << "20px height was not preserved!";
}
