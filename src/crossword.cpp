#include "crossword.h"

void puzzle::place_word_in_grid(shared_ptr<placed_word> word) {
		ivec2 p = word->pos;
		for(int i = 0; i < word->word.size(); ++i) {
			if(grid[p.y][p.x].c == word->word[i])
				grid[p.y][p.x].append_owner(word.get());
			else
				grid[p.y][p.x] = cell(word->word[i], word.get());
			p += direction_to_dp(word->dr);
		}
	}
	bool puzzle::check_grid(shared_ptr<placed_word> parent, ivec2 p, direction d, const string& w, bool verbose) {
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
	void puzzle::remove_word_from_grid(shared_ptr<placed_word> w, bool verbose) {
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
	bool puzzle::complete() {
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
	puzzle::puzzle(const vector<string>& words, bool verbose) :
		grid(20+rand()%4, vector<cell>(20+rand()%4, cell()))
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
					remove_word_from_grid(last_placed_word_stack.top(), verbose);
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
						if(check_grid(current_placed_word, pos, D, word, verbose)) {
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

	void puzzle::print_grid(ostream& os) const {
		for(const auto& v : grid) {
			for(auto c : v)
				os << c.c;
			os << endl;
		}
	}
	void puzzle::print_final(ostream& os) const {
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
	float puzzle::fill_ratio() const {
		float filled_cells = 0.f;
		for(int y = 0; y < grid.size(); ++y)
			for(int x = 0; x < grid[y].size(); ++x) {
				if(grid[y][x].c != ' ') filled_cells++;
			}
		return filled_cells/((float)(grid.size()*grid[0].size()));
	}


	shared_ptr<svg::shape> puzzle::generate_puzzle_svg(bool is_key) {
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

ostream& operator<<(ostream& os, const puzzle& p) {
	p.print_final(os);
	return os;
}
