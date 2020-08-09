#include "pch.h"
#include "Common/AssetProcessing/STBTextureReader.h"

TEST(STBTextureReaderTest, CanReadPNG) {
	Flux::STBTextureReader tTextureReader;
	auto tAsset = tTextureReader.LoadTexture("Resources/tex0.png");
	EXPECT_EQ(tAsset->mWidth, 2);
	EXPECT_EQ(tAsset->mHeight, 2);
	EXPECT_TRUE(tAsset->mData[0] == 0 && tAsset->mData[1] == 0 && tAsset->mData[2] == 255 && tAsset->mData[3] == 255);
	EXPECT_TRUE(tAsset->mData[4] == 0 && tAsset->mData[5] == 255 && tAsset->mData[6] == 0 && tAsset->mData[7] == 255);
	EXPECT_TRUE(tAsset->mData[8] == 255 && tAsset->mData[9] == 0 && tAsset->mData[10] == 0 && tAsset->mData[11] == 255);
	EXPECT_TRUE(tAsset->mData[12] == 255 && tAsset->mData[13] == 255 && tAsset->mData[14] == 0 && tAsset->mData[15] == 255);
}

TEST(STBTextureReaderTest, CanReadJPG) {
	Flux::STBTextureReader tTextureReader;
	auto tAsset = tTextureReader.LoadTexture("Resources/tex0.jpg");
	EXPECT_EQ(tAsset->mWidth, 2);
	EXPECT_EQ(tAsset->mHeight, 2);
	EXPECT_TRUE(tAsset->mData[0] < 5 && tAsset->mData[1] < 5 && tAsset->mData[2] > 250 && tAsset->mData[3] > 250);
	EXPECT_TRUE(tAsset->mData[4] < 5 && tAsset->mData[5] > 250 && tAsset->mData[6] < 5 && tAsset->mData[7] > 250);
	EXPECT_TRUE(tAsset->mData[8] > 250 && tAsset->mData[9] < 5 && tAsset->mData[10] < 5 && tAsset->mData[11] > 250);
	EXPECT_TRUE(tAsset->mData[12] > 250 && tAsset->mData[13] > 250 && tAsset->mData[14] < 5 && tAsset->mData[15] > 250);
}

TEST(STBTextureReaderTest, CanReadBMP) {
	Flux::STBTextureReader tTextureReader;
	auto tAsset = tTextureReader.LoadTexture("Resources/tex0.bmp");
	EXPECT_EQ(tAsset->mWidth, 2);
	EXPECT_EQ(tAsset->mHeight, 2);
	EXPECT_TRUE(tAsset->mData[0] == 0 && tAsset->mData[1] == 0 && tAsset->mData[2] == 255 && tAsset->mData[3] == 255);
	EXPECT_TRUE(tAsset->mData[4] == 0 && tAsset->mData[5] == 255 && tAsset->mData[6] == 0 && tAsset->mData[7] == 255);
	EXPECT_TRUE(tAsset->mData[8] == 255 && tAsset->mData[9] == 0 && tAsset->mData[10] == 0 && tAsset->mData[11] == 255);
	EXPECT_TRUE(tAsset->mData[12] == 255 && tAsset->mData[13] == 255 && tAsset->mData[14] == 0 && tAsset->mData[15] == 255);
}

TEST(STBTextureReaderTest, CanReadTGA) {
	Flux::STBTextureReader tTextureReader;
	auto tAsset = tTextureReader.LoadTexture("Resources/tex0.tga");
	EXPECT_EQ(tAsset->mWidth, 2);
	EXPECT_EQ(tAsset->mHeight, 2);
	EXPECT_TRUE(tAsset->mData[0] == 0 && tAsset->mData[1] == 0 && tAsset->mData[2] == 255 && tAsset->mData[3] == 255);
	EXPECT_TRUE(tAsset->mData[4] == 0 && tAsset->mData[5] == 255 && tAsset->mData[6] == 0 && tAsset->mData[7] == 255);
	EXPECT_TRUE(tAsset->mData[8] == 255 && tAsset->mData[9] == 0 && tAsset->mData[10] == 0 && tAsset->mData[11] == 255);
	EXPECT_TRUE(tAsset->mData[12] == 255 && tAsset->mData[13] == 255 && tAsset->mData[14] == 0 && tAsset->mData[15] == 255);
}

TEST(STBTextureReaderTest, RaisesExceptionReadingNonExistentFile) {
	Flux::STBTextureReader tTextureReader;
	try {
		tTextureReader.LoadTexture("Resources/not_a_file.png");
	}
	catch (Flux::ErrorAssetFileNotFound &e) {
		EXPECT_NE(strstr(e.what(), "Resources/not_a_file.png"), nullptr);
		return;
	}
	GTEST_FAIL();
}
