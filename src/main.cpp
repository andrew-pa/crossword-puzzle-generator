#include <cpr/cpr.h>
#include <rapidjson/document.h>
#include "crossword.h"

int main(int argc, char* argv[]) {

	bool verbose = false;
	for(int i = 0; i < argc; ++i) {
		if(argv[i][0] == '/' && argv[i][1] == 'v') verbose = true;
	}

	vector<string> words;
	if(argc < (verbose ? 3 : 2)) {
		cout << "no arguments, using Wordnik API to get random words" << endl;
		auto res = cpr::Get(
			cpr::Url{"http://api.wordnik.com/v4/words.json/randomWords"},
			cpr::Parameters{
				{"api_key", "ceca0dbc0ba60ada110070e411500109a95f670a3f7837662"},
				{"hasDictionaryDef", "true"},
				{"minLength", "6"},
				{"maxLength", "8"},
				{"limit", "100"}
			});
		rapidjson::Document d;
		d.Parse(res.text.c_str());
		for(auto& w : d.GetArray())
			words.push_back(w["word"].GetString());
		cout << "got " << words.size() << " words" << endl;
	}
	else {
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
