module;

#include "stdafx.h"

export module Shapes;

import algorithms;

export COLORREF SHAPE_COLORS[] = {
	RGB(31, 176, 255),
	RGB(141, 255,  65),
	RGB(252,  75, 105),
	RGB(189, 161, 255),
	RGB(243, 159,  58),
	RGB(250, 255,  91),
	RGB(255, 173, 207)
};

export COLORREF getRandomShapeColor() {
	return SHAPE_COLORS[rand() % (sizeof(SHAPE_COLORS) / sizeof(COLORREF))];
}

// draws star with the circumcircle radius *r*, at the center point *c*, the initial vertex
// angle *ang*, filled by the color *color* on the appropriate DC *dc*
export void drawStar(HDC dc, POINT2 &c, int r, int ang, COLORREF color) {
	auto star = generateStar(c.x, c.y, r, ang);

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, color);

	HGDIOBJ old_obj = ::SelectObject(dc, pen);

	auto drawLine = [dc](long x1, long y1, long x2, long y2) {
		::MoveToEx(dc, x1, y1, NULL);
		::LineTo(dc, x2, y2);
	};

	fillPolygon(*star, drawLine);

	Polyline(dc, (const POINT*)star->data(), star->size());

	::SelectObject(dc, old_obj);
}


// zvezdochki
export void randomStars(HDC dc, int width, int height) {
	auto points = randomlyPlaceObjectsOnArea(width, height);

	for (POINT2 &point : *points) {
		drawStar(
			dc,
			point,
			randRange(20, 90),
			rand() % 360,
			SHAPE_COLORS[rand() % (sizeof(SHAPE_COLORS) / sizeof(COLORREF))]
		);
	}
}
