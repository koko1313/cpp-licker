#include "stdafx.h"
#include "boost/filesystem.hpp"
#include <string.h>
#include <Windows.h>
#include <iostream>

using namespace std;

// преобразува string само в малки букви
void toLowerCase(string &str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}

// Връща параметър на дадена опция
string getParameterFromFile(string optionLookingFor) {
	ifstream configFile("config.txt", ios::in);
	string row;
	while (configFile >> row) { // за всеки ред
		string option = row.substr(0, row.find("=")); // взимаме опцията

		// преобразуваме в малки букви за да сравняваме
		toLowerCase(option);
		toLowerCase(optionLookingFor);

		string parameter = row.substr(row.find("=") + 1, row.length()); // взимаме параметъра
		if (optionLookingFor == "copyfrom" && parameter.find("currentUser") != std::string::npos && option == optionLookingFor) { // ако оприята е copyfrom и имаме currentUser
			char * username = getenv("USERNAME"); // взима user name на текущия user
			parameter.replace(parameter.find("currentUser"), 11, username); // заменяме 11 символа (currentuser) с името на user-а
			return parameter;
		}
		else if (option == optionLookingFor) { // за всяка останала опция връщаме параметъра
			return parameter;
		}
	}
	configFile.close();
}

/**
връща списък с всички файлове или директории в дадена директория.

- searchDirectory - директорията, която разглеждаме
- type - "file" или "folder"
*/
vector<string> allFiles(string searchDirectory, string type) {
	boost::filesystem::path directory(searchDirectory); // отваряме директорията, която ще разглеждаме
	boost::filesystem::directory_iterator end_itr;
	vector<string> files; // някаква колекция
	int i = 0; // брояч
	for (boost::filesystem::directory_iterator itr(directory); itr != end_itr; ++itr, i++) { // за всеки файл
		if (type == "files" && is_regular_file(itr->path())) { // ако искаме файлове
			files.push_back(itr->path().filename().string());
		} 
		else if (type == "folders" && !is_regular_file(itr->path())) { // ако искаме папки
			files.push_back(itr->path().filename().string());
		}
	}
	return files;
}

/**
генерира и връща пореден номер на директория
- pasteTo - името, с което искаме да направим директория
- directoryNumber - номера, от който искаме да започнем
*/
string generateDirectoryNumber(string &pasteTo, int directoryNumber) {
	vector<string> files = allFiles("./", "folders");
	int j = 0; // брояч
	for (vector<string>::iterator i = files.begin(); i != files.end(); i++, j++) { // обхождаме списъка с всички фалове
		if (files.at(j) == pasteTo + to_string(directoryNumber)) { // ако текущата папка с поредния номер съществува
			generateDirectoryNumber(pasteTo, ++directoryNumber); // рекурсивно извикваме пак функцията
		}
	}
	return to_string(directoryNumber);
}

// създава директория
void createDirectory(string &pasteTo) {
	pasteTo += generateDirectoryNumber(pasteTo, 1); // добавяме номер на директорията, за да не се презаписва
	boost::filesystem::create_directory(pasteTo); // създаваме я
}

int main() {
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false); // скриваме конзолата
	string pasteTo = getParameterFromFile("pasteTo");
	createDirectory(pasteTo);

	vector<string> files = allFiles(getParameterFromFile("copyFrom"), "files");
	int j = 0; // брояч
	for (vector<string>::iterator i = files.begin(); i != files.end(); i++, j++) { // обхождаме списъка с всички файлове
		string current_file = files.at(j); // цялото име на файла
		string fileFormat = current_file.substr(current_file.find('.') + 1, current_file.length()); // само формата на файла
		
		toLowerCase(fileFormat);
		
		if (fileFormat == getParameterFromFile("format")) { // ако файла е в съотвения формат
			string path = getParameterFromFile("copyFrom") + current_file; // пътя, от който ще копираме

			ifstream originalFile(path, ios::binary); // отваряме в двоичен вид
			ofstream copyFile(pasteTo + "/" + current_file, ios::binary);

			copyFile << originalFile.rdbuf(); // копираме

			originalFile.close();
			copyFile.close();
		}
	}
}