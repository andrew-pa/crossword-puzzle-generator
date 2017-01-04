#include "crossword.h"

int main(int argc, char* argv[]) {
	if(argc < 2) {
		cout << "no arguments" << endl;
		return -1;
	}
	vector<string> words;
	{
		vector<string> words_raw;
		ifstream wordsf(argv[1]);
		while(wordsf) {
			string s; getline(wordsf, s);
			s = trim(s);
			if(s.empty()) continue;
			words_raw.push_back(s);
		}
		random_device rd;
		mt19937 g(rd());
		shuffle(words_raw.begin(), words_raw.end(), g);
		words = words_raw;
	}

	bool verbose = false;
	if(argc > 2 && *argv[2] == 'v') verbose = true;

	puzzle p(words, verbose);
	cout << p << endl;
	cout << "puzzle fill ratio: " << p.fill_ratio() << endl;
	//getchar();
	ofstream svgo("puzzle.svg");
	svgo << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"3000px\" height=\"3000px\">";
	auto pv = p.generate_puzzle_svg();
	pv->generate_svg(svgo);
	svgo << "</svg>";
	svgo.flush();
	//getchar();
}
