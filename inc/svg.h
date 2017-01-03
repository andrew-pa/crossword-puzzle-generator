#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <iomanip>
#include <memory>
using namespace std;

namespace svg {
	ostream& operator <<(ostream& os, function<void(ostream& os)> f) {
		f(os);
		return os;
	}
	enum class rgba_colors : unsigned int {
		transparent = 0x0000'0000,
		black = 0x0000'00ff,
		white = 0xffff'ffff,
		red = 0xff00'00ff,
		green = 0x00ff'00ff,
		blue = 0x0000'ffff
	};
	union RGBAvalue {
	   	unsigned int v; unsigned char a, r, g, b;
		RGBAvalue(unsigned int V) : v(V) {}
		RGBAvalue(rgba_colors c) : v((unsigned int)c) {}
	};
	ostream& operator <<(ostream& os, RGBAvalue v) {
		os << "#" << setfill('0') << setw(8) << hex << v.v << dec;
		return os;
	}
	struct color {
	   	RGBAvalue fill;
		RGBAvalue stroke;
		double stroke_width;
		color(RGBAvalue fill = rgba_colors::black) : fill(fill), stroke(0), stroke_width(0) {}
		color(RGBAvalue stroke, double stroke_width) : fill(0), stroke(stroke), stroke_width(stroke_width) {}
		color(RGBAvalue fill, RGBAvalue stroke, double stroke_width) : fill(fill), stroke(stroke), stroke_width(stroke_width) {}
		virtual void generate_svg(ostream& os) const {
			os << " fill=\"" << fill << "\" ";
			if(stroke.v > 0) os << "stroke=\"" << stroke << "\" ";
			if(stroke_width > 0) os << "stroke-width=\"" << stroke_width << "\"";
		}
	};
	struct shape {
		color col;
		shape(color c = color()) : col(c) {}
		virtual void generate_svg(ostream& os) const = 0;
	protected:
		void generate_common_svg(ostream& os, const string& elmnm, const vector<pair<string,string>>& attrb) const  {
			os << "<" << elmnm;
			for(const auto& atr : attrb) {
				os << " " << atr.first << " =\"" << atr.second << "\"";
			}
			col.generate_svg(os);
			os << "/>";
		}
	};
	struct rectangle : public shape {
		double x, y, width, height;
		rectangle(double x, double y, double w, double h, color c)
			: x(x), y(y), width(w), height(h), shape(c) {}
		void generate_svg(ostream& os) const override {
			generate_common_svg(os, "rect", {
						{"x", to_string(x)},
						{"y", to_string(y)},
						{"width", to_string(width)},
						{"height", to_string(height)},
					});
			/*os << "<rect x=\"" << x << "\" y=\"" << y << 
					"\" width=\"" << width << "\" height=\"" << height << "\"" <<
					[&](ostream& os){col.generate_svg(os);} << "/>"; */
		}
	};
	struct group : public shape {
		vector<shared_ptr<shape>> children;
		group(const vector<shared_ptr<shape>>& ch = {}, color c = color()) : children(ch), shape(c) {}

		void generate_svg(ostream& os) const override {
			os << "<g>";
			for(const auto& s : children) {
				s->generate_svg(os);
			}
			os << "</g>";
		}

	};
	struct font {
		double size;
		double weight;
		string family;
		font(const string& f = "", double s = -1.0, double w = -1.0) : size(s), weight(w), family(f) {}
		void generate_svg(ostream& os) const {
			if(size > 0.0) os << " font-size=\"" << size << "\" ";
			if(weight > 0.0) os << " font-weight=\"" << weight << "\" ";
			if(!family.empty()) os << " font-family=\"" << family << "\" ";
		}
	};
	struct text : public shape {
		string chars;
		double x,y;
		font fnt;
		text(double x, double y, const string& ch, const font& f = font(), color c = color())
			: chars(ch), x(x), y(y), fnt(f), shape(c) {}

		void generate_svg(ostream& os) const override {
			os << "<text x=\"" << x << "\" y=\"" << y << "\"";
				//<< [&](ostream& os) { fnt.generate_svg(os); }
				//<< [&](ostream& os) { col.generate_svg(os); } 
			fnt.generate_svg(os);
			col.generate_svg(os);
			os << ">";
			os << chars;
			os << "</text>";
		}
	};
}
