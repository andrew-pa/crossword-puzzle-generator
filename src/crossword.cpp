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

bool verbose = false;

class puzzle {
	struct cell {
		char c;
		list<placed_word*> owners;
		cell(char c = ' ', placed_word* w = nullptr) : c(c), owners{w} {}
		inline void append_owner(placed_word* w) { owners.push_back(w); }
	};
	vector<vector<cell>> grid;
	shared_ptr<placed_word> root_word;

	inline void place_word_in_grid(shared_ptr<placed_word> word) {
		ivec2 p = word->pos;
		for(int i = 0; i < word->word.size(); ++i) {
			if(grid[p.y][p.x].c == word->word[i])
				grid[p.y][p.x].append_owner(word.get());
			else
				grid[p.y][p.x] = cell(word->word[i], word.get());
			p += direction_to_dp(word->dr);
		}
	}
	inline bool check_grid(shared_ptr<placed_word> parent, ivec2 p, direction d, const string& w) {
		if(verbose) cout << "checking word " << w << " for placement" << endl;
		for(int i = 0; i < w.size(); ++i) {
			if(p.x < 0 || p.x >= grid[0].size() || p.y < 0 || p.y >= grid.size()) return false;
			if(verbose) cout << "w = " << w[i] << " g = " << grid[p.y][p.x].c << endl;
			if(grid[p.y][p.x].c != ' ' && grid[p.y][p.x].c != w[i]) return false;
			for(int dy = -1; dy <= 1; ++dy)
				for(int dx = -1; dx <= 1; ++dx) {
					if(p.x+dx < 0 || p.x+dx >= grid[0].size() || p.y+dy < 0 || p.y+dy >= grid.size()) continue;
					if (grid[p.y + dy][p.x + dx].c != ' ') {// && grid[p.y + dy][p.x + dx].c != w[i + (d == direction::across ? dx : dy)]) {
						if(verbose) cout << ">> " << w[i+(d == direction::across ? dx : dy)] << " " << grid[p.y+dy][p.x+dx].c << endl;
						auto o = grid[p.y+dy][p.x+dx].owners;
						auto pi = find(o.begin(), o.end(), parent.get());
						if(pi == o.end()) return false;
					}
				}
			p += direction_to_dp(d);
		}
		return true;
	}
	inline void remove_word_from_grid(shared_ptr<placed_word> w) {
		if(verbose) cout << "removing word " << w->word << endl;
		auto f = find(w->parent->children.begin(), w->parent->children.end(), w);
		if(f != w->parent->children.end()) *f = nullptr;
		ivec2 p = w->pos;
		for(int i = 0; i < w->word.size(); ++i) {
			//	cout << "[ ";
			//	for(auto o : grid[p.y][p.x].owners) cout << o << " ";
			//	cout << "]";
			if(grid[p.y][p.x].owners.size() == 1)
				grid[p.y][p.x] = cell();
			else {
				auto f = find(grid[p.y][p.x].owners.begin(), grid[p.y][p.x].owners.end(), w.get());
				if(f == grid[p.y][p.x].owners.end()) throw;
				grid[p.y][p.x].owners.erase(f);
			}
			p += direction_to_dp(w->dr);
		}
	}
	inline bool complete() {
		// check that we have the proper number of words down and across
		stack<shared_ptr<placed_word>> s;
		s.push(root_word);
		size_t words_down = 0, words_across = 0;
		while(!s.empty()) {
			auto n = s.top(); s.pop();
			for(auto c : n->children) if(c != nullptr) s.push(c);
			if(n->dr == direction::across) words_across++; else if(n->dr == direction::down) words_down++;
		}
		return words_down >= 15 && words_across >= 15;
	}
public:
	puzzle(const vector<string>& words) :
		grid(18+rand()%4, vector<cell>(18+rand()%4, cell()))
	{
		/**** Puzzle generation algorithm ****/
		stack<shared_ptr<placed_word>> last_placed_word_stack;
		list<string> words_to_place(words.begin(), words.end());
		const auto grid_width = grid[0].size();
		const auto grid_height = grid.size();
		root_word = make_shared<placed_word>(nullptr, ivec2(1,5), words_to_place.front(), direction::across);
		place_word_in_grid(root_word);
		words_to_place.pop_front();
		//For each word in dictionary
		while(!words_to_place.empty()) {
			if(complete()) break;
			const string word = words_to_place.front(); words_to_place.pop_front();
			if(verbose) cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
			if(verbose) cout << "trying to place word: " << word << endl;
			stack<shared_ptr<placed_word>> search_stack;
			//Start at the root word
			search_stack.push(root_word);
			while(true) {
				if(verbose) print_grid(cout);
				if(verbose) root_word->debug_print(cout);
				if(verbose) cout << "~~" << endl;
				if(verbose) getchar();
				//*1* Pop a placed word out of the search stack for the current placed word
				//If the stack is in fact empty:
				if(search_stack.empty()) {
					//At this point a placement for the word could not be found. This means that some backtracking must be necessary
					//If there are no placed words left, this word could not be placed, so restart puzzle generation
					if(last_placed_word_stack.empty()) {
						if(verbose) cout << "could not place word " << word << endl;
						search_stack.push(root_word);
						break;
					}
					//Pop the last word placed off the last placed word stack and remove it, then start at the root word and attempt to place the current word to be placed
					remove_word_from_grid(last_placed_word_stack.top());
					words_to_place.push_back(last_placed_word_stack.top()->word);
					last_placed_word_stack.pop();
					search_stack.push(root_word);
					continue;
				}
				auto current_placed_word = search_stack.top(); search_stack.pop();
				if(verbose) cout << "checking word: " << current_placed_word->word << endl;
				//Search the current placed word for a letter that it has in common with the word to be placed
				bool placed = false;
				for(int chr = 0; chr < word.size(); ++chr) {
					auto nxt_ltr = current_placed_word->word.find_first_of(word[chr]);
					if(verbose) cout << "checking letter " << word[chr] << endl;
					for(; nxt_ltr != string::npos; nxt_ltr = current_placed_word->word.find_first_of(word[chr], nxt_ltr+1)) {
						if (chr == 0 && nxt_ltr == 0) continue;
						if(verbose) cout << "intersection @ " << chr << ", " << nxt_ltr << " letter=" << word[chr] << endl;
						//calculate placement of word at this matching letter
						auto D = ppdl(current_placed_word->dr);
						auto pos = current_placed_word->pos + swap_for_direction(current_placed_word->dr, ivec2(nxt_ltr, -chr));
						//check the grid to see if that placement would be valid
						if(check_grid(current_placed_word, pos, D, word)) {
							//If the placement is valid:
								//place the word in the grid and move on to the next word to be placed
								auto wrd = make_shared<placed_word>(current_placed_word, pos, word, D);
								place_word_in_grid(wrd);
								current_placed_word->children[nxt_ltr] = wrd;
								//push the placed word in the last word placed stack
								last_placed_word_stack.push(wrd);
								placed = true;
								if(verbose) cout << "!";
								break;
						}
						//Else:
							//continue on to the next common letter
					}
					if(placed) break;
				}
				//Else if you don't find any common letters:
				if(!placed) {
					if(verbose) cout << "couldn't place word " << word << endl;
					//push all the children on to the search stack and repeat from *1*
					for(const auto ch : current_placed_word->children) {
						if(ch == nullptr) continue;
						if(verbose) cout << "searching: " << ch->word << endl;
						search_stack.push(ch);
					}
				} else {
					if(verbose) cout << "placed word " << word << endl;
					break;
				}
			}
		}
		/*************************************/
	}

	inline void print_grid(ostream& os) const {
		for(const auto& v : grid) {
			for(auto c : v)
				os << c.c;
			os << endl;
		}
	}
	inline void print_final(ostream& os) const {
		stack<shared_ptr<placed_word>> s;
		s.push(root_word);
		vector<shared_ptr<placed_word>> down,across;
		while(!s.empty()) {
			auto n = s.top(); s.pop();
			for(auto c : n->children) if(c != nullptr) s.push(c);
			if(n->dr == direction::across) across.push_back(n); else if(n->dr == direction::down) down.push_back(n);
		}
		for(size_t y = 0; y < grid.size(); ++y) {
			for(size_t x = 0; x < grid[y].size(); ++x)
				cout << "+---";
			cout << "+" << endl;
			for(size_t x = 0; x < grid[y].size(); ++x) {
				cout << "|";
			 	if(grid[y][x].c != ' ') {
					bool found_head = false;
					for(auto ow : grid[y][x].owners) {
						if(ow->pos == ivec2(x,y)) {
							vector<shared_ptr<placed_word>>* wl = nullptr;
							if(ow->dr == direction::across)
								wl = &across;
							else if(ow->dr == direction::down)
								wl = &down;
							auto f = find_if(wl->begin(), wl->end(), 
									[&ow](shared_ptr<placed_word> w) {
										return w.get() == ow;
									});
							cout << setw(2) << distance(wl->begin(), f)+1;
							found_head = true;
							break;
						}
					}
					if(!found_head) cout << "  ";
				} else {
					cout << "  ";
				}
				cout << grid[y][x].c; 
			}
			cout << "|" << endl;
		}
		for(size_t x = 0; x < grid[0].size(); ++x)
			cout << "+---";
		cout << "+" << endl;

		cout << "down:" << endl;
		for(int i = 0; i < down.size(); ++i) {
			cout << (i+1) << ". " << down[i]->word << endl;
		}
		cout << "across:" << endl;
		for(int i = 0; i < across.size(); ++i) {
			cout << (i+1) << ". " << across[i]->word << endl;
		}
	}
	inline float fill_ratio() const {
		float filled_cells = 0.f;
		for(int y = 0; y < grid.size(); ++y)
			for(int x = 0; x < grid[y].size(); ++x) {
				if(grid[y][x].c != ' ') filled_cells++;
			}
		return filled_cells/((float)(grid.size()*grid[0].size()));
	}


	inline shared_ptr<svg::shape> generate_puzzle_svg(bool is_key = true) {
		vector<shared_ptr<svg::shape>> cells;
		stack<shared_ptr<placed_word>> s;
		s.push(root_word);
		vector<shared_ptr<placed_word>> down,across;
		while(!s.empty()) {
			auto n = s.top(); s.pop();
			for(auto c : n->children) if(c != nullptr) s.push(c);
			if(n->dr == direction::across) across.push_back(n); else if(n->dr == direction::down) down.push_back(n);
		}
		double cell_w = 32.0, cell_h = 32.0;
		for(size_t y = 0; y < grid.size(); ++y) {
			for(size_t x = 0; x < grid[y].size(); ++x) {
			 	if(grid[y][x].c != ' ') {
					for(auto ow : grid[y][x].owners) {
						if(ow->pos == ivec2(x,y)) {
							vector<shared_ptr<placed_word>>* wl = nullptr;
							if(ow->dr == direction::across)
								wl = &across;
							else if(ow->dr == direction::down)
								wl = &down;
							auto f = find_if(wl->begin(), wl->end(), 
									[&ow](shared_ptr<placed_word> w) {
										return w.get() == ow;
									});
							auto ix = distance(wl->begin(), f)+1;
							cells.push_back(make_shared<svg::text>(5+x*cell_w+cell_w*0.1, 5+y*cell_h+cell_w*0.3, to_string(ix), svg::font("", 10, 600)));
							break;
						}
					}
				}
				if(grid[y][x].c != ' ') { 
					cells.push_back(make_shared<svg::rectangle>(5+x*cell_w,5+y*cell_h,cell_w,cell_h,svg::color(svg::rgba_colors::black, 2.0)));
					if(is_key) cells.push_back(make_shared<svg::text>(5+x*cell_w+cell_w*0.3, 5+y*cell_h+cell_w*0.7, 
							string(1,grid[y][x].c)));
				}
			}
		}
		double listy = 18;
		cells.push_back(make_shared<svg::text>(grid.size()*cell_w + 5, listy, "down:", svg::font("", 16, 900)));
		//listy += 16;
		for(int i = 0; i < down.size(); ++i) {
			ostringstream oss;
			oss << (i+1) << ". " << down[i]->word << endl;
			cells.push_back(make_shared<svg::text>(grid.size()*cell_w + 5, listy += 16, oss.str()));
		}
		listy += 32;
		cells.push_back(make_shared<svg::text>(grid.size()*cell_w + 5, listy+=16, "across:", svg::font("", 16, 900)));
		for(int i = 0; i < across.size(); ++i) {
			ostringstream oss;
			oss << (i+1) << ". " << across[i]->word << endl;
			cells.push_back(make_shared<svg::text>(grid.size()*cell_w + 5, listy+=16, oss.str()));
		}

		return make_shared<svg::group>(cells);
	}
};

ostream& operator<<(ostream& os, const puzzle& p) {
	p.print_final(os);
	return os;
}




//need some way to backtrack
//probably need to be able to take words out of the puzzle and restart
//need to be able to not place words so that they make gibberish words with their neghbors
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

	if(argc > 2 && *argv[2] == 'v') verbose = true;

	puzzle p(words);
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
