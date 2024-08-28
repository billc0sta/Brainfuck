#include <cstdio>
#include <string>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <vector>
#include <unordered_map>
using namespace std;

#define BYTES_SIZE 30'000

struct Token {
	int type;
	int start;
	int end;
	int count;
};

struct Ins {
	uint16_t index;
	int type;
	int count;
	int tbracket;
};

Token next_token(const string& raw_program, int& fcursor) {
	Token tk;
	string tks = "+-><[].,";
	for (;fcursor < raw_program.length() && tks.find(raw_program[fcursor]) == string::npos; ++fcursor)
		;
	if (fcursor == raw_program.length()) {
		tk.type = 0;
		return tk; 
	}
	tk.type = raw_program[fcursor];
	tk.start = fcursor;
	tk.count = 0;
	switch(raw_program[fcursor]) {
	case '+':
	case '-':
	case '>':
	case '<':
	{
		char chr = raw_program[fcursor];
		tks.erase(tks.find(chr), 1);
		for (;fcursor < raw_program.length() && tks.find(raw_program[fcursor]) == string::npos; ++fcursor) {
			if (raw_program[fcursor] == chr) 
				++tk.count;
		}
		tk.end = fcursor;
	} break;
	case '[':
	case ']':
	case '.':
	case ',':
	{
		tk.count = 1;
		tk.end   = fcursor;
		++fcursor;
	} break;

	default: tk.type = 0; break; 
	}
	return tk;
}

int main(int argc, char *argv[]) {

	// loading file
	if (argc < 2) {
		printf("\nARGUMENT_ERROR: -- didn't provide input file --\n"); 
		return -1;
	}
	string file_path = argv[1];
	ifstream file {file_path};
	if (file.fail()) {
		printf("\nARGUMENT_ERROR: -- couldn't open file, reason: %s --\n", strerror(errno));
		return -1;
	}
	string raw_program;
	char c = 0;
	while ((c = file.get()) != EOF)
		raw_program.push_back(c);
	file.close();

	// let's consider this to be compile time
	Token tk;
	int fcursor = 0;
	vector<Ins> ins;
	vector<int> jump_table;
	unordered_map<pair<uint16_t, int>, count> ins_map;
	int counter = 0;
	while ((tk = next_token(raw_program, fcursor)).type != 0) {
		if (tk.type == '[' || tk.type == ']') {
			for (const auto& pair : ins_map) {
				ins.push_back(
					{pair.first.first,
				 	pair.first.second,
				 	pair.second}
				);
			}
		}
		switch(tk.type) {
		case '+':
		case '-':
		case '>':
		case '<':
		case '.':
		case ',':
		case '[':
			jump_table.emplace_back(counter);
			break;
		case ']':
			if (jump_table.size() == 0) {
				printf("\nSYNTAX_ERROR: -- unmatching closing bracket at: %d --\n", tk.start);
				return -1;
			}
			tk.tbracket = jump_table.back();
			ins[jump_table.back()].tbracket = counter;
			jump_table.pop_back();
			break;
		}
		tokens.emplace_back(tk);
		++counter;
	}
	if (jump_table.size() > 0) {
		printf("\nSYNTAX_ERROR: -- unmatching opening bracket at: %d --\n", tk.start);
		return -1; 
	}

	// runtime
	int dcursor = 0; 
	uint8_t* bytes = new uint8_t[BYTES_SIZE];
	memset(bytes, 0, BYTES_SIZE);
	
	for (int i = 0; i < tokens.size(); ++i){
		Token& tk = tokens[i];

		switch(tk.type) {
		case '+':
			bytes[dcursor] += tk.count; break;
		case '-':
			bytes[dcursor] -= tk.count; break;
		case '>':
			dcursor = dcursor + tk.count; break;
		case '<':
			dcursor = dcursor - tk.count; break;
		case '.':
			fputc(bytes[dcursor], stdout); break;
		case ',':
			bytes[dcursor] = fgetc(stdin); break;
		case '[':
			if (!bytes[dcursor]) i = tk.tbracket; break;
		case ']':
			if (bytes[dcursor])  i = tk.tbracket; break;
		}
	}
	delete[] bytes;
	return 0;
}