#pragma once

const size_t MAX_IAMGE_DATA_SIZE = 548;
const size_t MAX_IMG_BUFFER_SIZE = 1024 * 1024;

// 数据包 = 标记位 + 包头 + 图像数据
typedef struct tagImagePackageHeader
{
	unsigned int nMarkSize;		// 标记位大小
	unsigned int nHeaderSize;	// 包头大小
	unsigned int nDataSize;		// 此包图像数据的大小
	unsigned int nImageSize;	// 此图片总大小
	unsigned int nPackageNum;	// 一共分成了多少个小包
	unsigned int nPackageIndex;	// 当前包是第几个小包
	unsigned int nDataOffset;	// 当前图像数据在图像中的偏移量
	unsigned int nImageId;		// 图片ID
}ImagePackageHeader;
