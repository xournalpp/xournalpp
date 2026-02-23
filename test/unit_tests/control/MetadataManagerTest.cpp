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
#include <gtest/gtest.h>

#include "control/settings/MetadataManager.h"
#include "util/PathUtil.h"

#include "config-test.h"


TEST(Metadata, testRead) {
    auto entry = MetadataManager::loadMetadataFile(GET_TESTFILE(u8"metadata/1.metadata"));
    EXPECT_TRUE(entry);
    EXPECT_EQ(entry->time, 1);
    EXPECT_EQ(entry->page, 12);
    EXPECT_EQ(entry->zoom, 3.42);
    EXPECT_EQ(entry->path, fs::path(u8"/test/my/metadata.xopp"));
}
TEST(Metadata, testWriteReadCycle) {
    auto entry = MetadataManager::loadMetadataFile(GET_TESTFILE(u8"metadata/aωkward ⒫ath/2.metadata"));
    EXPECT_TRUE(entry);
    EXPECT_EQ(entry->time, 2);
    EXPECT_EQ(entry->page, 12);
    EXPECT_EQ(entry->zoom, 3.42);
    EXPECT_EQ(entry->path, fs::path(u8"/test/my/metadatⓐ.xopp"));

    entry->time = 1234567;
    entry->page = 98765;
    entry->zoom = 56.734;

    auto tmp = Util::getTmpDirSubfolder() / "1234567.metadata";
    ASSERT_FALSE(fs::exists(tmp));
    MetadataManager::writeMetadataToFile(*entry, tmp);

    auto entry2 = MetadataManager::loadMetadataFile(tmp);
    ASSERT_TRUE(entry2);
    EXPECT_EQ(entry->path, entry2->path);
    EXPECT_EQ(entry->time, entry2->time);
    EXPECT_EQ(entry->page, entry2->page);
    EXPECT_EQ(entry->zoom, entry2->zoom);
    fs::remove(tmp);
}
