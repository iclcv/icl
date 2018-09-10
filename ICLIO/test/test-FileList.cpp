#include "gtest/gtest.h"
#include "ICLIO/FileList.h"

using icl::io::FileList;

TEST(FileListTest, EmptyConstructor) {
	FileList list;
	EXPECT_EQ(0, list.size());
}

TEST(FileListTest, Globbing) {
	FileList listAll("../ICLIO/test/files/*");
	// listAll.show();
	EXPECT_EQ(6, listAll.size());

	FileList listFiles("../ICLIO/test/files/*.txt");
	EXPECT_EQ(2, listFiles.size());
}
