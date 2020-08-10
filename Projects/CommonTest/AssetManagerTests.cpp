#include "pch.h"
#include "Common/AssetProcessing/AssetManager.h"

TEST(AssetManagerTest, CanLoadTexturePNG) {
	Flux::AssetManager tAssetManager;
	auto tAsset = tAssetManager.LoadTexture("Resources/tex0.png");
	EXPECT_EQ(tAsset->mWidth, 2);
	EXPECT_EQ(tAsset->mHeight, 2);
	EXPECT_TRUE(tAsset->mData[0] == 0 && tAsset->mData[1] == 0 && tAsset->mData[2] == 255 && tAsset->mData[3] == 255);
	EXPECT_TRUE(tAsset->mData[4] == 0 && tAsset->mData[5] == 255 && tAsset->mData[6] == 0 && tAsset->mData[7] == 255);
	EXPECT_TRUE(tAsset->mData[8] == 255 && tAsset->mData[9] == 0 && tAsset->mData[10] == 0 && tAsset->mData[11] == 255);
	EXPECT_TRUE(tAsset->mData[12] == 255 && tAsset->mData[13] == 255 && tAsset->mData[14] == 0 && tAsset->mData[15] == 255);
}

TEST(AssetManagerTest, RaisesExceptionLoadingUnsupportedTextureType) {
	Flux::AssetManager tAssetManager;
	try {
		tAssetManager.LoadTexture("Resources/tex0.invalid");
	}
	catch (Flux::ErrorUnsupportedAssetType &e) {
		EXPECT_NE(strstr(e.what(), "Resources/tex0.invalid"), nullptr);
		return;
	}
	GTEST_FAIL();
}

TEST(AssetManagerTest, ReLoadingAssetSharesMemory) {
	Flux::AssetManager tAssetManager;
	auto tAssetFirst = tAssetManager.LoadTexture("Resources/tex0.jpg");
	auto tAssetSecond = tAssetManager.LoadTexture("Resources/tex0.jpg");
	EXPECT_EQ(tAssetFirst.get(), tAssetSecond.get());
}
