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
	int type;
	int param;
};

Token lex(const string& raw_program, int& fcursor) {
	Token tk;
	string tks = "+-><[].,";
	for (;fcursor < raw_program.length() && 
		tks.find(raw_program[fcursor]) == string::npos; ++fcursor)
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
		for (;fcursor < raw_program.length() && 
			tks.find(raw_program[fcursor]) == string::npos; ++fcursor) {
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

int compile(const string& raw_program, vector<Ins>& ret) {
	vector<pair<int, int>> jump_table;
	Token tk;
	int fcursor = 0, counter = 0;
	while ((tk = lex(raw_program, fcursor)).type != 0) {
		Ins ins = { tk.type, 0 };
		switch(tk.type) {
		case '+':
		case '-':
		case '>':
		case '<':
			ins.param = tk.count;
			break;
		case '[':
			jump_table.emplace_back(pair<int, int>{counter, fcursor});
			break; 
		case ']':
			{
			if (jump_table.size() == 0) {
				fprintf(stderr, "\nSYNTAX_ERROR: -- unmatching closing bracket at: %d--\n", dcursor);
				return -1;
			}
			int tb = jump_table.back().first;
			ret[tb].param = counter;
			ins.param     = tb;
			jump_table.pop_back();
			break;
			}
		}
		ret.emplace_back(ins); 
		++counter;
	}
	if (jump_table.size() > 0) {
		for (const auto& jump : jump_table)
			fprintf(stderr, "\nSYNTAX_ERROR: -- unmatching opening bracket at: %d--\n", jump.second);
		return -1;
	}
	return 0; 
}

int run(const vector<Ins>& instructs) {
	uint8_t* bytes = new uint8_t[BYTES_SIZE];
	memset(bytes, 0, BYTES_SIZE);
	int dcursor = 0;

	for (int i = 0; i < instructs.size(); ++i){
		Ins ins = instructs[i];
		switch(ins.type) {
		case '+': bytes[dcursor] += ins.param;   break;
		case '-': bytes[dcursor] -= ins.param;   break;
		case '>': dcursor += ins.param; 		 break;
		case '<': dcursor -= ins.param;		     break; 
		case '.': fputc(bytes[dcursor], stdout); break;
		case ',': bytes[dcursor] = fgetc(stdin); break;
		case '[':
			if (!bytes[dcursor]) i = ins.param;  break;
		case ']':
			if ( bytes[dcursor]) i = ins.param;  break;
		}
	}
	delete[] bytes;
}

int main(int argc, char *argv[]) {

	// loading file
	if (argc < 2) {
		fprintf(stderr, "\nARGUMENT_ERROR: -- didn't provide input file --\n"); 
		return -1;
	}
	string file_path = argv[1];
	ifstream file {file_path};
	if (file.fail()) {
		fprintf(stderr, "\nARGUMENT_ERROR: -- couldn't open file, reason: %s --\n", strerror(errno));
		return -1;
	}
	string raw_program;
	char c = 0;
	while ((c = file.get()) != EOF)
		raw_program.push_back(c);
	file.close();

	// compile
	vector<Ins> instructs;
	if (compile(raw_program, instructs) != 0)
		return -1; 
	// for (const auto& ins : instructs) {
	// 		printf("[type: %c, param: %d, param2: %d]\n", ins.type, ins.param, ins.param2);
	// }
	// run
	printf("running...\n");
	run(instructs);
	return 0;
}