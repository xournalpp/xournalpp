/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <cstdlib>
#include <ctime>

#include <gtest/gtest.h>

#include "util/XojPreviewExtractor.h"

#include "config-test.h"


using namespace std;


TEST(UtilXojPreviewExtractor, testNonExistingFile) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("THIS FILE DOES NOT EXIST.xoj"));

    EXPECT_EQ(PREVIEW_RESULT_COULD_NOT_OPEN_FILE, result);
}

TEST(UtilXojPreviewExtractor, testExtensionCheck) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("test.xoi"));

    EXPECT_EQ(PREVIEW_RESULT_BAD_FILE_EXTENSION, result);
}

TEST(UtilXojPreviewExtractor, testLoadGzipped) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test.xoj"));

    EXPECT_EQ(PREVIEW_RESULT_IMAGE_READ, result);

    gsize dataLen = 0;
    unsigned char* imageData = extractor.getData(dataLen);
    EXPECT_EQ(string("CppUnitTestString"), string((char*)imageData, (size_t)dataLen));
}

TEST(UtilXojPreviewExtractor, testLoadGzipped2) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test2.xoj"));

    EXPECT_EQ(PREVIEW_RESULT_IMAGE_READ, result);

    gsize dataLen = 0;
    extractor.getData(dataLen);
    EXPECT_EQ((std::string::size_type)2856, dataLen);
}

TEST(UtilXojPreviewExtractor, testLoad1Unzipped) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test.unzipped.xoj"));

    EXPECT_EQ(PREVIEW_RESULT_IMAGE_READ, result);

    gsize dataLen = 0;
    unsigned char* imageData = extractor.getData(dataLen);
    EXPECT_EQ(string("CppUnitTestString"), string((char*)imageData, (size_t)dataLen));
}

TEST(UtilXojPreviewExtractor, testLoad1Zipped) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("packaged_xopp/testPreview.xopp"));

    EXPECT_EQ(PREVIEW_RESULT_IMAGE_READ, result);

    gsize dataLen = 0;
    unsigned char* imageData = extractor.getData(dataLen);
    EXPECT_EQ(string("CppUnitTestString \n"), string((char*)imageData, (size_t)dataLen));
}

TEST(UtilXojPreviewExtractor, testLoad2Zipped) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("packaged_xopp/testPreview2.xopp"));

    EXPECT_EQ(PREVIEW_RESULT_IMAGE_READ, result);

    gsize dataLen = 0;
    extractor.getData(dataLen);
    EXPECT_EQ((std::string::size_type)804, dataLen);
}

TEST(UtilXojPreviewExtractor, testNoPreview) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test-no-preview.unzipped.xoj"));

    EXPECT_EQ(PREVIEW_RESULT_NO_PREVIEW, result);
}

TEST(UtilXojPreviewExtractor, testInvalidFile) {
    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test-invalid.xoj"));

    EXPECT_EQ(PREVIEW_RESULT_ERROR_READING_PREVIEW, result);
}
