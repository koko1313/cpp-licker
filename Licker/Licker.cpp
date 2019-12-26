#include "stdafx.h"
#include "boost/filesystem.hpp"
#include <string.h>
#include <Windows.h>
#include <iostream>

using namespace std;

// ����������� string ���� � ����� �����
void toLowerCase(string &str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}

// ����� ��������� �� ������ �����
string getParameterFromFile(string optionLookingFor) {
	ifstream configFile("config.txt", ios::in);
	string row;
	while (configFile >> row) { // �� ����� ���
		string option = row.substr(0, row.find("=")); // ������� �������

		// ������������� � ����� ����� �� �� ����������
		toLowerCase(option);
		toLowerCase(optionLookingFor);

		string parameter = row.substr(row.find("=") + 1, row.length()); // ������� ����������
		if (optionLookingFor == "copyfrom" && parameter.find("currentUser") != std::string::npos && option == optionLookingFor) { // ��� ������� � copyfrom � ����� currentUser
			char * username = getenv("USERNAME"); // ����� user name �� ������� user
			parameter.replace(parameter.find("currentUser"), 11, username); // �������� 11 ������� (currentuser) � ����� �� user-�
			return parameter;
		}
		else if (option == optionLookingFor) { // �� ����� �������� ����� ������� ����������
			return parameter;
		}
	}
	configFile.close();
}

/**
����� ������ � ������ ������� ��� ���������� � ������ ����������.

- searchDirectory - ������������, ����� �����������
- type - "file" ��� "folder"
*/
vector<string> allFiles(string searchDirectory, string type) {
	boost::filesystem::path directory(searchDirectory); // �������� ������������, ����� �� �����������
	boost::filesystem::directory_iterator end_itr;
	vector<string> files; // ������� ��������
	int i = 0; // �����
	for (boost::filesystem::directory_iterator itr(directory); itr != end_itr; ++itr, i++) { // �� ����� ����
		if (type == "files" && is_regular_file(itr->path())) { // ��� ������ �������
			files.push_back(itr->path().filename().string());
		} 
		else if (type == "folders" && !is_regular_file(itr->path())) { // ��� ������ �����
			files.push_back(itr->path().filename().string());
		}
	}
	return files;
}

/**
�������� � ����� ������� ����� �� ����������
- pasteTo - �����, � ����� ������ �� �������� ����������
- directoryNumber - ������, �� ����� ������ �� ��������
*/
string generateDirectoryNumber(string &pasteTo, int directoryNumber) {
	vector<string> files = allFiles("./", "folders");
	int j = 0; // �����
	for (vector<string>::iterator i = files.begin(); i != files.end(); i++, j++) { // ��������� ������� � ������ ������
		if (files.at(j) == pasteTo + to_string(directoryNumber)) { // ��� �������� ����� � �������� ����� ����������
			generateDirectoryNumber(pasteTo, ++directoryNumber); // ���������� ��������� ��� ���������
		}
	}
	return to_string(directoryNumber);
}

// ������� ����������
void createDirectory(string &pasteTo) {
	pasteTo += generateDirectoryNumber(pasteTo, 1); // �������� ����� �� ������������, �� �� �� �� ����������
	boost::filesystem::create_directory(pasteTo); // ��������� �
}

int main() {
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false); // �������� ���������
	string pasteTo = getParameterFromFile("pasteTo");
	createDirectory(pasteTo);

	vector<string> files = allFiles(getParameterFromFile("copyFrom"), "files");
	int j = 0; // �����
	for (vector<string>::iterator i = files.begin(); i != files.end(); i++, j++) { // ��������� ������� � ������ �������
		string current_file = files.at(j); // ������ ��� �� �����
		string fileFormat = current_file.substr(current_file.find('.') + 1, current_file.length()); // ���� ������� �� �����
		
		toLowerCase(fileFormat);
		
		if (fileFormat == getParameterFromFile("format")) { // ��� ����� � � ��������� ������
			string path = getParameterFromFile("copyFrom") + current_file; // ����, �� ����� �� ��������

			ifstream originalFile(path, ios::binary); // �������� � ������� ���
			ofstream copyFile(pasteTo + "/" + current_file, ios::binary);

			copyFile << originalFile.rdbuf(); // ��������

			originalFile.close();
			copyFile.close();
		}
	}
}