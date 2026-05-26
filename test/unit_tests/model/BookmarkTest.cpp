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

#include <config-test.h>
#include <gtest/gtest.h>

#include "model/Document.h"
#include "model/XojPage.h"
#include "model/PageRef.h"

using Bm = std::pair<std::string, size_t>;

// Single page

TEST(Bookmark, testNewPageHasNoBookmark) {
    XojPage page(210, 297);

    EXPECT_FALSE(page.getBookmark().has_value());
}

TEST(Bookmark, testSetBookmarkStoresName) {
    XojPage page(210, 297);

    page.setBookmark("Chapter 1");

    ASSERT_TRUE(page.getBookmark().has_value());
    EXPECT_EQ(page.getBookmark().value(), "Chapter 1");
}

TEST(Bookmark, testSetBookmarkOverwritesPrevious) {
    XojPage page(210, 297);

    page.setBookmark("Old Name");
    page.setBookmark("New Name");

    ASSERT_TRUE(page.getBookmark().has_value());
    EXPECT_EQ(page.getBookmark().value(), "New Name");
}

TEST(Bookmark, testDeleteBookmarkClearsValue) {
    XojPage page(210, 297);

    page.setBookmark("To Delete");
    page.deleteBookmark();

    EXPECT_FALSE(page.getBookmark().has_value());
}

TEST(Bookmark, testDeleteOnPageWithNoBookmarkIsNoop) {
    XojPage page(210, 297);

    page.deleteBookmark();

    EXPECT_FALSE(page.getBookmark().has_value());
}

TEST(Bookmark, testBookmarkNamePreservesUnicode) {
    XojPage page(210, 297);
    const std::string name = "Capítulo 1: Introdução";

    page.setBookmark(name);

    ASSERT_TRUE(page.getBookmark().has_value());
    EXPECT_EQ(page.getBookmark().value(), name);
}

TEST(Bookmark, testDocumentAddBookmark) {
    Document doc(nullptr);

    PageRef page = std::make_unique<XojPage>(210, 297);
    doc.addPage(page);
    doc.setBookmark("Very Important Note", 0);

    auto bookmarks = doc.listBookmarks();
    auto expected = std::vector<Bm>({Bm("Very Important Note", 0)});

    EXPECT_EQ(bookmarks, expected);
}

TEST(Bookmark, testAddBookmarksReturnsTheOldBookmark) {
    Document doc(nullptr);

    PageRef page = std::make_unique<XojPage>(210, 297);
    page->setBookmark("Chapter 1");
    auto oldBookmark = page->getBookmark();

    doc.addPage(page);
    auto addBookmarkRet = doc.setBookmark("Very Important Note", 0);

    EXPECT_EQ(oldBookmark, addBookmarkRet);
}

TEST(Bookmark, testListBookmarksEmptyDocument) {
    Document doc(nullptr);

    auto bookmarks = doc.listBookmarks();
    auto expected = std::vector<Bm>({});

    EXPECT_EQ(bookmarks, expected);
}


// Multi page

TEST(Bookmark, testClonePreservesBookmark) {
    auto page = std::make_shared<XojPage>(100, 100);
    page->setBookmark("Chapter 1");

    XojPage* cloned = page->clone();

    ASSERT_TRUE(cloned->getBookmark().has_value());
    EXPECT_EQ(cloned->getBookmark().value(), "Chapter 1");
    delete cloned;
}

TEST(Bookmark, testSetEmptyDifferentFromDelete) {
    auto page = std::make_shared<XojPage>(100, 100);
    page->setBookmark("Chapter 1");
    XojPage* cloned = page->clone();

    page->deleteBookmark();
    cloned->setBookmark("");

    EXPECT_FALSE(page->getBookmark().has_value());
    EXPECT_TRUE(cloned->getBookmark().has_value());
}

TEST(Bookmark, testCloneWithNoBookmark) {
    auto page = std::make_shared<XojPage>(100, 100);
    XojPage* cloned = page->clone();
    EXPECT_FALSE(cloned->getBookmark().has_value());
    delete cloned;
}

TEST(Bookmark, testBookmarksShowInCorrectOrder) {
    Document doc(nullptr);

    PageRef page0 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page0);
    PageRef page1 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page1);
    PageRef page2 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page2);
    PageRef page3 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page3);

    doc.setBookmark("A", 0);
    doc.setBookmark("B", 3);

    auto bookmarks = doc.listBookmarks();
    auto expected = std::vector<Bm>({Bm("A", 0), Bm("B", 3)});

    EXPECT_EQ(bookmarks, expected);
}


TEST(Bookmark, testClearDocumentClearsBookmarks) {
    Document doc(nullptr);

    PageRef page0 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page0);
    PageRef page1 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page1);
    PageRef page2 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page2);
    PageRef page3 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page3);

    doc.setBookmark("A", 0);
    doc.setBookmark("B", 3);

    doc.clearDocument(true);

    auto bookmarks = doc.listBookmarks();
    auto expected = std::vector<Bm>({});

    EXPECT_EQ(bookmarks, expected);
}

TEST(Bookmark, testInsertingPageUpdatesBookmarkPageNumbers) {
    Document doc(nullptr);

    PageRef page0 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page0);
    PageRef page1 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page1);
    PageRef page2 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page2);

    doc.setBookmark("A", 0);
    doc.setBookmark("B", 2);

    PageRef newPage = std::make_shared<XojPage>(100, 100);
    doc.insertPage(newPage, 1);

    auto bookmarks = doc.listBookmarks();
    auto expected = std::vector<Bm>({Bm("A", 0), Bm("B", 3)});

    EXPECT_EQ(bookmarks, expected);
}

TEST(Bookmark, testDeletingPageUpdatesBookmarksPageNumbers) {
    Document doc(nullptr);

    PageRef page0 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page0);
    PageRef page1 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page1);
    PageRef page2 = std::make_shared<XojPage>(100, 100);
    doc.addPage(page2);

    doc.setBookmark("A", 0);
    doc.setBookmark("B", 1);
    doc.setBookmark("C", 2);

    doc.deletePage(1);

    auto bookmarks = doc.listBookmarks();
    auto expected = std::vector<Bm>({Bm("A", 0), Bm("C", 1)});

    EXPECT_EQ(bookmarks, expected);
}

TEST(Bookmark, testTryAddBookmarkInOutOfRangePage) {
    Document doc(nullptr);

    PageRef page = std::make_shared<XojPage>(100, 100);
    doc.addPage(page);

    auto oldBookmark = doc.setBookmark("Bookmark", 99);

    EXPECT_EQ(oldBookmark, std::nullopt);
}
