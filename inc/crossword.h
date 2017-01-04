#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <algorithm>
#include <memory>
#include <random>
#include <cctype>
#include <locale>
#include <iomanip>
using namespace std;

#include "svg.h"

// trim from start
inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
	std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
inline std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
	std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

enum class direction { across, down };
struct ivec2 {
	int x, y;
ivec2(int x = 0, int y = 0)
		: x(x), y(y) {}
	inline ivec2 operator +(const ivec2& a) const { return ivec2(x+a.x,y+a.y); }
	inline ivec2 operator +=(const ivec2& a) { return *this = (*this+a); }
	inline bool operator ==(const ivec2& a) const { return x==a.x && y==a.y; }
};
inline ivec2 direction_to_dp(direction d) {
	if(d == direction::across) return ivec2(1, 0);
	else if(d == direction::down) return ivec2(0, 1);
	else throw;
}
inline direction ppdl(direction d) {
	if(d == direction::across) return direction::down;
	else if(d == direction::down) return direction::across;
	else throw;
}
inline ivec2 swap_for_direction(direction d, ivec2 i) {
	if(d == direction::across) return i;
	else if(d == direction::down) return ivec2(i.y, i.x);
	else throw;
}

struct placed_word {
	shared_ptr<placed_word> parent;
	const ivec2 pos;
	const string word;
	const direction dr;
	vector< shared_ptr<placed_word> > children;
	placed_word(shared_ptr<placed_word> par, ivec2 p, const string& w, direction d) :
		word(w), dr(d), pos(p), children(w.size(), nullptr), parent(par) {}

	inline void debug_print(ostream& os, int lvl = 0) {
		for(int i = 0; i < lvl; ++i) os << "\t";
		os << word << endl;
		for(auto c : children) {
			if(c) c->debug_print(os,lvl+1); else {
			//	for(int i = 0; i < lvl+1; ++i) os << "\t";
			//   	os << "***" << endl;
			}
		}
	}
};

class puzzle {
	struct cell {
		char c;
		list<placed_word*> owners;
		cell(char c = ' ', placed_word* w = nullptr) : c(c), owners{w} {}
		inline void append_owner(placed_word* w) { owners.push_back(w); }
	};
	vector<vector<cell>> grid;
	shared_ptr<placed_word> root_word;

	void place_word_in_grid(shared_ptr<placed_word> word);
	bool check_grid(shared_ptr<placed_word> parent, ivec2 p, direction d, const string& w, bool verbose);
	void remove_word_from_grid(shared_ptr<placed_word> w, bool verbose);
	bool complete();
public:
	puzzle(const vector<string>& words, bool verbose = false);
	void print_grid(ostream& os) const;
	void print_final(ostream& os) const;
	float fill_ratio() const;
	shared_ptr<svg::shape> generate_puzzle_svg(bool is_key = true);
};
ostream& operator<<(ostream& os, const puzzle& p);
