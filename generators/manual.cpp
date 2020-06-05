/*
    Converts text to binary representation
*/
#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char** argv) {
	if (argc < 1) {
		printf("Usage: manual <filename>\n");
	    exit(1);
	}

	const char* filename = argv[1];
	std::ofstream file (filename, std::ios::binary | std::ios::out);
	if (!file.is_open()) {
		printf("Can't write to destination file\n");
		exit(1);
	}

    std::string line;
    while(std::getline(std::cin, line)) {

		if(line.size() == 0) {
			break;
		}

		float f = atof(line.c_str());
        file.write(reinterpret_cast<const char *>(&f), sizeof(float));
    }
	    
    file.close();
	return 0;
}