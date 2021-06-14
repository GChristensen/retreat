module;

#include <cmath>
#include <cstdlib>

export module algorithms;

import <list>;
import <vector>;
import <memory>;
import <numbers>;
import <algorithm>;
import <functional>;

inline long round_int(double x) {
	return (long)floor((x)+0.5);
}

export struct POINT2 {
	long x;
	long y;
};

struct EDGE
{
	long Yl;	// lower y point
	long Yu;	// upper y point
	double Xl;	// current x intersection
	double w;	// 1/slope of the edge

	bool horisontal() { return Yl == Yu; }

	static bool edge_x_sort_p(const EDGE& e1, const EDGE& e2)
	{
		return e1.Xl < e2.Xl;
	}

	static bool edge_xw_sort_p(const EDGE& e1, const EDGE& e2)
	{
		return e1.Xl <= e2.Xl && e1.w < e2.w;
	}
};

export int randRange(int min, int max)
{
	return min + rand() % (max - min);
}

export auto generateStar(long x, long y, int r, int ang) {
	using namespace std;
	using namespace std::numbers;

	const int vertexes = 10;
	const double rad = pi / 180;
	const double rcp_fi2 = 0.38197; // 1/fi^2, magic constant (fi - golden ratio)
	const int section = 360 / vertexes;

	int inner_cricle = round_int(r * rcp_fi2);

	auto ppoints = make_unique<vector<POINT2>>(vertexes + 1);
	auto &points = *ppoints;

	// find vertexes of a star
	for (int i = 0; i < vertexes; ++i)
	{
		double pt_angle = ((ang + i * section) % 360) * rad;
		int pt_r = (i % 2 == 0) ? r : inner_cricle;

		points[i].x = round_int(pt_r * cos(pt_angle)) + x;
		points[i].y = round_int(pt_r * sin(pt_angle)) + y;
	}

	points[vertexes] = points[0];

	return ppoints;
}

// a variation of fast scan-conversion fill algorithm
export void fillPolygon(const std::vector<POINT2> &points, 
		std::function<void (long, long, long, long)> drawLine) {
	using namespace std;

	int npoints = points.size() - 1;
	vector<EDGE> edges(npoints);

	int first_line = points[0].y;
	int last_line = points[0].y;

	// find edges
	
	for (int i = 0; i < npoints; ++i) {
		EDGE &edge = edges[i];
		double Xu;

		int next = i + 1;

		if (points[i].y < points[next].y) {
			edge.Yl = points[i].y;
			edge.Yu = points[next].y;
			edge.Xl = points[i].x;
			Xu = points[next].x;
		}
		else {
			edge.Yl = points[next].y;
			edge.Yu = points[i].y;
			edge.Xl = points[next].x;
			Xu = points[i].x;
		}

		if (edge.Yl < first_line)
			first_line = edge.Yl;

		if (edge.Yu > last_line)
			last_line = edge.Yu;

		if (!edge.horisontal())
			// reciprocal slope of edge
			edge.w = (Xu - edge.Xl) / (edge.Yu - edge.Yl);
	}

	int lines = last_line - first_line + 1;

	typedef list<EDGE> edge_list_t;
	vector<edge_list_t> edge_table(lines);

	// fill edge table
	for (EDGE& edge : edges) {
		if (!edge.horisontal()) {
			long y = edge.Yl - first_line;
			edge_table[y].emplace_back(edge);
		}
	}

	edge_list_t active_edge_table;

	for (int y = 0; y < lines; ++y) {
		long scanline = y + first_line;

		// add edges to active edge table
		edge_list_t& edge_list = edge_table[y];
		if (!edge_list.empty()) {
			active_edge_table.insert(
				active_edge_table.end(),
				edge_list.begin(),
				edge_list.end()
			);

			// sort by x
			active_edge_table.sort(EDGE::edge_x_sort_p);

			// then put w in ascending order within elements with equal x
			// these two steps are a some kind of bucket sort
			stable_sort(
				active_edge_table.begin(),
				active_edge_table.end(),
				EDGE::edge_xw_sort_p
			);
		}

		// remove rendered edges from active edge table
		active_edge_table.remove_if([scanline](const EDGE& edge) { return edge.Yu == scanline; });

		if (active_edge_table.empty())
			break;

		edge_list_t::iterator current = active_edge_table.begin();
		edge_list_t::iterator next = ++active_edge_table.begin();
		int k = 0;

		// draw scanline parts
		do {
			if (k++ % 2 == 0)
				drawLine(round_int(current->Xl), scanline, round_int(next->Xl), scanline);

			current = next;
			++next;
		} while (next != active_edge_table.end());

		// increment the current x value for each edge
		for (EDGE &edge : active_edge_table)
			edge.Xl += edge.w;
	}
}

constexpr int SLICE_SIDE = 120;

export auto randomlyPlacePointsOnArea(int width, int height) {
	using namespace std;

	int slices_x = width / SLICE_SIDE;
	int slices_y = height / SLICE_SIDE;
	int slice_cx = width / slices_x;
	int slice_cy = height / slices_y;

	auto ppoints = make_unique<vector<POINT2>>(slices_x * slices_y);
	auto& points = *ppoints;

	int k = 0;
	for (int i = 0; i < slices_x; ++i)
		for (int j = 0; j < slices_y; ++j)
		{
			points[k].x = randRange(slice_cx * i, slice_cx * (i + 1));
			points[k].y = randRange(slice_cy * j, slice_cy * (j + 1));

			k += 1;
		}

	return ppoints;
}