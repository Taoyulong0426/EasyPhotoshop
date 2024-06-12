#ifndef SIMPLEPHOTOSHOP_BMP_H
#define SIMPLEPHOTOSHOP_BMP_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned char uchar;

#pragma pack(1)
struct BmpFileHeader //位图文件头 固定14个字节
{
	ushort bfType;  //2字节，文件类型，标识该文件为bmp文件,判断文件是否为bmp文件，即用该值与"0x4d42"比较是否相等即可，0x4d42 = 19778
	ulong bfSize;   //4字节，说明文件大小，包括这14个字节，以字节为单位
	ushort bfReserved1; //2字节，预保留位,必须设置为0
	ushort bfReserved2; //2字节，预保留位,必须设置为0
	ulong bfOffBits;    //4字节，从头到位图数据的偏移，即图像数据区的起始位置
};

struct BmpInfoHeader //位图信息头 固定大小为40个字节
{
	ulong biSize;    //4字节，信息头大小
	long biWidth;    //4字节，以像素为单位说明图像的宽度
	long biHeight;    //4字节，以像素为单位说明图像的高度，同时如果为正，说明位图倒立（即数据表示从图像的左下角到右上角），如果为负说明正向
	ushort biPlanes;    //2字节，位平面数，总被设置为1
	ushort biBitCount;  //2字节，每像素位数，常用的值为1（黑白二色图），8（256色），24（真彩色图）
	ulong biCompression;    //4字节，说明图像的压缩类型，有效的值为BI_RGB，BI_RLE8，BI_RLE4，BI_BITFIELDS(都是一些Windows定义好的常量)，最常用的就是0（BI_RGB），表示不压缩
	ulong biSizeImage;      //4字节，说明位图数据的大小，当用BI_RGB格式时，可以设置为0
	long biXPelsPerMeter;  //表示水平分辨率，单位是像素/米
	long biYPelsPerMeter;  //表示垂直分辨率，单位是像素/米
	ulong biClrUsed;    //说明位图使用的调色板中的颜色索引数，为0说明用到的颜色数为2^biBitCount
	ulong biClrImportant;  //说明对图像显示有重要影响的颜色索引数，为0说明都重要
};

struct Palette //调色板
{
	uchar b;
	uchar g;
	uchar r;
	uchar alpha; //预保留位，为0
};

class BmpImage {
public:
	BmpImage() = default;
	vector<vector<uchar>> data; //图片数据
	BmpFileHeader bmpFileHeader{};
	BmpInfoHeader bmpInfoHeader{};
	//获取图像高度
	int getHeight() const {
		return height;
	}
	//获取图像宽度
	int getWidth() const {
		return width;
	}
	void setWidth(long newWidth) {
		width = newWidth;
		bmpInfoHeader.biWidth = newWidth;
	}

	void setHeight(long newHeight) {
		height = newHeight;
		bmpInfoHeader.biHeight = newHeight;
	}
	//获得通道数
	int getChannels() const {
		return channels;
	}
	//设置通道数（在rgb转gray时有用）
	void setChannels(int newChannel){
		channels = newChannel;
	}

	ushort getBitcount() {
		return bmpInfoHeader.biBitCount;
	}
	//设置bitcount（在rgb转gray时有用）
	void setBitCount(ushort bitcount) {
		bmpInfoHeader.biBitCount = bitcount;
	}

	vector<vector<uchar>> getData() {
		return data;
	}

	//设置data内容（在rgb转gray时有用）
	void setData(vector<vector<uchar>> new_data) {
		data.clear();
		for (int i = 0; i < new_data.size(); i++) {
			vector<uchar> dataV;
			for (int j = 0; j < new_data[i].size(); j++) {
				dataV.push_back(new_data[i][j]);
			}
			data.push_back(dataV);
		}
	}
	//设置调色板内容（在rgb转gray时有用）
	void setPalette(Palette p) {
		palette.push_back(p);
	}

	vector<Palette> getPalette() {
		return palette;
	}
	//判断是否读取成功
	bool is_loaded() const {
		return loaded;
	}
	
	bool LoadImage(const string &path);

	bool SaveImage(const string &path);

	bool SaveImage() {
		string path;
		printf("\n******请输入图片输出路径，例(""C:\\..."")******\n");
		getline(std::cin, path);
		SaveImage(path);
		return true;
	}
	//展示位图信息
	void ShowBMPInfo();

private:
	bool loaded = false;
	int width; //长度
	int height; //宽度
	int channels; //通道数
	vector<Palette> palette; //调色板数据
};

bool BmpImage::LoadImage(const string &path) {

	ifstream fileReader(path, ios::binary);//以二进制形式打开文件
	if (!fileReader.is_open()) {
		cerr << "文件打开失败" << endl;//判断文件是否打开成功
		return false;
	}
	fileReader.read(reinterpret_cast<char *>(&bmpFileHeader), sizeof(bmpFileHeader));//读取文件头
	if (bmpFileHeader.bfType != 0x4D42) {
		cerr << "此文件不是bmp文件" << endl;//判断打开文件是否为bmp文件
		return false;
	}
	fileReader.read(reinterpret_cast<char *>(&bmpInfoHeader), sizeof(bmpInfoHeader));

	//cout << bmpInfoHeader.biBitCount << " bit " << "image" << endl;
	//cout << "data offset: " << bmpFileHeader.bfOffBits << endl;
	int dataOffset = bmpFileHeader.bfOffBits;
	channels = bmpInfoHeader.biBitCount / 8;
	if (channels != 1 && channels != 3) {
		cerr << "系统不支持此类bmp文件" << endl;
		return false;
	}
	width = bmpInfoHeader.biWidth;
	height = bmpInfoHeader.biHeight;
	//以下为读取8位与24位bmp文件
	if (channels == 1 && dataOffset == 1078) {
		while (fileReader.tellg() < dataOffset) {
			Palette temp{};
			fileReader.read(reinterpret_cast<char *>(&temp), sizeof(temp));
			palette.push_back(temp);//8位图像需要读取调色板
		}
	}
	int widthBytes = width * channels;
	int offset = widthBytes % 4;
	if (offset != 0) {
		offset = 4 - offset;
		widthBytes += offset;
	}//每一行的字节数必须是4的整倍数，如果不是，则需要补齐
	for (int i = 0; i < height; ++i) {
		vector<uchar> dataValues;
		for (int j = 0; j < widthBytes; ++j) {
			uchar dataValue;
			fileReader.read(reinterpret_cast<char *>(&dataValue), sizeof(dataValue));
			dataValues.push_back(dataValue);
		}
		for (int k = 0; k < offset; ++k) {
			dataValues.pop_back();
		}
		data.push_back(dataValues);
	}
	
	fileReader.close();
	loaded = true;
	return true;
}

bool BmpImage::SaveImage(const string &path) {
	if (!is_loaded()) {
		cerr << "bmp文件没有获取，无法保存" << endl;
		return false;
	}
	//bmpInfoHeader.biHeight = -bmpInfoHeader.biHeight;
	ofstream fileWriter;
	fileWriter.open(path, ios::out | ios::binary | ios::trunc);//数据流方式以二进制写
	if (!fileWriter.is_open()) {
		cerr << "文件打开失败" << endl;
		return false;
	}
	fileWriter.write(reinterpret_cast<char *>(&bmpFileHeader), sizeof(bmpFileHeader));//写文件头
	fileWriter.write(reinterpret_cast<char *>(&bmpInfoHeader), sizeof(bmpInfoHeader));//写信息头
	if (!palette.empty()) {
		for (auto value : palette) {
			fileWriter.write(reinterpret_cast<char *>(&value), sizeof(value));
		}
	}
	for (const auto &dataValues : data) {
		for (auto dataValue : dataValues) {
			fileWriter.write(reinterpret_cast<char *>(&dataValue), sizeof(dataValue));
		}
	}
	fileWriter.close();
	cout << "******输出成功******" << endl;
	return true;
}

void BmpImage::ShowBMPInfo() {
	if (!is_loaded()) {
		cerr << "没有读取bmp文件，无法获取图片信息" << endl;
		return;
	}
	cout << "文件类型: " << hex << bmpFileHeader.bfType << endl;
	cout << "文件大小: " << dec << bmpFileHeader.bfSize << endl;
	cout << "保留字1: " << dec << bmpFileHeader.bfReserved1 << endl;
	cout << "保留字2: " << dec << bmpFileHeader.bfReserved2 << endl;
	cout << "数据偏移: " << dec << bmpFileHeader.bfOffBits << endl;
	cout << "信息头大小: " << dec << bmpInfoHeader.biSize << endl;
	cout << "图片宽度: " << dec << bmpInfoHeader.biWidth << endl;
	cout << "图片高度: " << dec << bmpInfoHeader.biHeight << endl;
	cout << "图片颜色平面数: " << dec << bmpInfoHeader.biPlanes << endl;
	cout << "图片位深度: " << dec << bmpInfoHeader.biBitCount << endl;
	cout << "图片压缩类型: " << dec << bmpInfoHeader.biCompression << endl;
	cout << "位图数据大小: " << dec << bmpInfoHeader.biSizeImage << endl;
	cout << "水平分辨率: " << dec << bmpInfoHeader.biXPelsPerMeter << endl;
	cout << "垂直分辨率: " << dec << bmpInfoHeader.biYPelsPerMeter << endl;
	cout << "调色板颜色索引数: " << dec << bmpInfoHeader.biClrUsed << endl;
	cout << "重要颜色索引数: " << dec << bmpInfoHeader.biClrImportant << endl;

}

#endif //SIMPLEPHOTOSHOP_BMP_H