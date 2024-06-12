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
struct BmpFileHeader //λͼ�ļ�ͷ �̶�14���ֽ�
{
	ushort bfType;  //2�ֽڣ��ļ����ͣ���ʶ���ļ�Ϊbmp�ļ�,�ж��ļ��Ƿ�Ϊbmp�ļ������ø�ֵ��"0x4d42"�Ƚ��Ƿ���ȼ��ɣ�0x4d42 = 19778
	ulong bfSize;   //4�ֽڣ�˵���ļ���С��������14���ֽڣ����ֽ�Ϊ��λ
	ushort bfReserved1; //2�ֽڣ�Ԥ����λ,��������Ϊ0
	ushort bfReserved2; //2�ֽڣ�Ԥ����λ,��������Ϊ0
	ulong bfOffBits;    //4�ֽڣ���ͷ��λͼ���ݵ�ƫ�ƣ���ͼ������������ʼλ��
};

struct BmpInfoHeader //λͼ��Ϣͷ �̶���СΪ40���ֽ�
{
	ulong biSize;    //4�ֽڣ���Ϣͷ��С
	long biWidth;    //4�ֽڣ�������Ϊ��λ˵��ͼ��Ŀ��
	long biHeight;    //4�ֽڣ�������Ϊ��λ˵��ͼ��ĸ߶ȣ�ͬʱ���Ϊ����˵��λͼ�����������ݱ�ʾ��ͼ������½ǵ����Ͻǣ������Ϊ��˵������
	ushort biPlanes;    //2�ֽڣ�λƽ�������ܱ�����Ϊ1
	ushort biBitCount;  //2�ֽڣ�ÿ����λ�������õ�ֵΪ1���ڰ׶�ɫͼ����8��256ɫ����24�����ɫͼ��
	ulong biCompression;    //4�ֽڣ�˵��ͼ���ѹ�����ͣ���Ч��ֵΪBI_RGB��BI_RLE8��BI_RLE4��BI_BITFIELDS(����һЩWindows����õĳ���)����õľ���0��BI_RGB������ʾ��ѹ��
	ulong biSizeImage;      //4�ֽڣ�˵��λͼ���ݵĴ�С������BI_RGB��ʽʱ����������Ϊ0
	long biXPelsPerMeter;  //��ʾˮƽ�ֱ��ʣ���λ������/��
	long biYPelsPerMeter;  //��ʾ��ֱ�ֱ��ʣ���λ������/��
	ulong biClrUsed;    //˵��λͼʹ�õĵ�ɫ���е���ɫ��������Ϊ0˵���õ�����ɫ��Ϊ2^biBitCount
	ulong biClrImportant;  //˵����ͼ����ʾ����ҪӰ�����ɫ��������Ϊ0˵������Ҫ
};

struct Palette //��ɫ��
{
	uchar b;
	uchar g;
	uchar r;
	uchar alpha; //Ԥ����λ��Ϊ0
};

class BmpImage {
public:
	BmpImage() = default;
	vector<vector<uchar>> data; //ͼƬ����
	BmpFileHeader bmpFileHeader{};
	BmpInfoHeader bmpInfoHeader{};
	//��ȡͼ��߶�
	int getHeight() const {
		return height;
	}
	//��ȡͼ����
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
	//���ͨ����
	int getChannels() const {
		return channels;
	}
	//����ͨ��������rgbתgrayʱ���ã�
	void setChannels(int newChannel){
		channels = newChannel;
	}

	ushort getBitcount() {
		return bmpInfoHeader.biBitCount;
	}
	//����bitcount����rgbתgrayʱ���ã�
	void setBitCount(ushort bitcount) {
		bmpInfoHeader.biBitCount = bitcount;
	}

	vector<vector<uchar>> getData() {
		return data;
	}

	//����data���ݣ���rgbתgrayʱ���ã�
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
	//���õ�ɫ�����ݣ���rgbתgrayʱ���ã�
	void setPalette(Palette p) {
		palette.push_back(p);
	}

	vector<Palette> getPalette() {
		return palette;
	}
	//�ж��Ƿ��ȡ�ɹ�
	bool is_loaded() const {
		return loaded;
	}
	
	bool LoadImage(const string &path);

	bool SaveImage(const string &path);

	bool SaveImage() {
		string path;
		printf("\n******������ͼƬ���·������(""C:\\..."")******\n");
		getline(std::cin, path);
		SaveImage(path);
		return true;
	}
	//չʾλͼ��Ϣ
	void ShowBMPInfo();

private:
	bool loaded = false;
	int width; //����
	int height; //���
	int channels; //ͨ����
	vector<Palette> palette; //��ɫ������
};

bool BmpImage::LoadImage(const string &path) {

	ifstream fileReader(path, ios::binary);//�Զ�������ʽ���ļ�
	if (!fileReader.is_open()) {
		cerr << "�ļ���ʧ��" << endl;//�ж��ļ��Ƿ�򿪳ɹ�
		return false;
	}
	fileReader.read(reinterpret_cast<char *>(&bmpFileHeader), sizeof(bmpFileHeader));//��ȡ�ļ�ͷ
	if (bmpFileHeader.bfType != 0x4D42) {
		cerr << "���ļ�����bmp�ļ�" << endl;//�жϴ��ļ��Ƿ�Ϊbmp�ļ�
		return false;
	}
	fileReader.read(reinterpret_cast<char *>(&bmpInfoHeader), sizeof(bmpInfoHeader));

	//cout << bmpInfoHeader.biBitCount << " bit " << "image" << endl;
	//cout << "data offset: " << bmpFileHeader.bfOffBits << endl;
	int dataOffset = bmpFileHeader.bfOffBits;
	channels = bmpInfoHeader.biBitCount / 8;
	if (channels != 1 && channels != 3) {
		cerr << "ϵͳ��֧�ִ���bmp�ļ�" << endl;
		return false;
	}
	width = bmpInfoHeader.biWidth;
	height = bmpInfoHeader.biHeight;
	//����Ϊ��ȡ8λ��24λbmp�ļ�
	if (channels == 1 && dataOffset == 1078) {
		while (fileReader.tellg() < dataOffset) {
			Palette temp{};
			fileReader.read(reinterpret_cast<char *>(&temp), sizeof(temp));
			palette.push_back(temp);//8λͼ����Ҫ��ȡ��ɫ��
		}
	}
	int widthBytes = width * channels;
	int offset = widthBytes % 4;
	if (offset != 0) {
		offset = 4 - offset;
		widthBytes += offset;
	}//ÿһ�е��ֽ���������4����������������ǣ�����Ҫ����
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
		cerr << "bmp�ļ�û�л�ȡ���޷�����" << endl;
		return false;
	}
	//bmpInfoHeader.biHeight = -bmpInfoHeader.biHeight;
	ofstream fileWriter;
	fileWriter.open(path, ios::out | ios::binary | ios::trunc);//��������ʽ�Զ�����д
	if (!fileWriter.is_open()) {
		cerr << "�ļ���ʧ��" << endl;
		return false;
	}
	fileWriter.write(reinterpret_cast<char *>(&bmpFileHeader), sizeof(bmpFileHeader));//д�ļ�ͷ
	fileWriter.write(reinterpret_cast<char *>(&bmpInfoHeader), sizeof(bmpInfoHeader));//д��Ϣͷ
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
	cout << "******����ɹ�******" << endl;
	return true;
}

void BmpImage::ShowBMPInfo() {
	if (!is_loaded()) {
		cerr << "û�ж�ȡbmp�ļ����޷���ȡͼƬ��Ϣ" << endl;
		return;
	}
	cout << "�ļ�����: " << hex << bmpFileHeader.bfType << endl;
	cout << "�ļ���С: " << dec << bmpFileHeader.bfSize << endl;
	cout << "������1: " << dec << bmpFileHeader.bfReserved1 << endl;
	cout << "������2: " << dec << bmpFileHeader.bfReserved2 << endl;
	cout << "����ƫ��: " << dec << bmpFileHeader.bfOffBits << endl;
	cout << "��Ϣͷ��С: " << dec << bmpInfoHeader.biSize << endl;
	cout << "ͼƬ���: " << dec << bmpInfoHeader.biWidth << endl;
	cout << "ͼƬ�߶�: " << dec << bmpInfoHeader.biHeight << endl;
	cout << "ͼƬ��ɫƽ����: " << dec << bmpInfoHeader.biPlanes << endl;
	cout << "ͼƬλ���: " << dec << bmpInfoHeader.biBitCount << endl;
	cout << "ͼƬѹ������: " << dec << bmpInfoHeader.biCompression << endl;
	cout << "λͼ���ݴ�С: " << dec << bmpInfoHeader.biSizeImage << endl;
	cout << "ˮƽ�ֱ���: " << dec << bmpInfoHeader.biXPelsPerMeter << endl;
	cout << "��ֱ�ֱ���: " << dec << bmpInfoHeader.biYPelsPerMeter << endl;
	cout << "��ɫ����ɫ������: " << dec << bmpInfoHeader.biClrUsed << endl;
	cout << "��Ҫ��ɫ������: " << dec << bmpInfoHeader.biClrImportant << endl;

}

#endif //SIMPLEPHOTOSHOP_BMP_H