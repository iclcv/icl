#include "gtest/gtest.h"
#include "ICLIO/FileList.h"

using icl::io::FileList;

TEST(FileListTest, EmptyConstructor) {
	FileList list;
	EXPECT_EQ(0, list.size());
}

TEST(FileListTest, Globbing) {
	FileList listAll("doc/*");
	EXPECT_EQ(6, listAll.size());

	FileList listFiles("doc/*.cmake");
	EXPECT_EQ(2, listFiles.size());
}
