// Generates img/hero.svg: a Windows desktop with Excel and Word open, covered by the kind of
// translucent star overlay Enso Retreat shows during a break (compare media/screen.jpg).
//
//   node tools/gen-hero.mjs

import { writeFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import path from "node:path";

const OUT = path.join(path.dirname(fileURLToPath(import.meta.url)), "..", "img", "hero.svg");
const W = 1200, H = 750;
const WORD = "#2B579A", EXCEL = "#217346", HEADING = "#2F5496";

// deterministic, so regenerating doesn't reshuffle the stars
function mulberry32(seed) {
  return () => {
    seed = (seed + 0x6D2B79F5) | 0;
    let t = Math.imul(seed ^ (seed >>> 15), 1 | seed);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

const n = (v) => +v.toFixed(1);
const attrs = (o) => Object.entries(o).map(([k, v]) => ` ${k}="${v}"`).join("");
const rect = (x, y, w, h, fill, o = {}) => `<rect${attrs({ x: n(x), y: n(y), width: n(w), height: n(h), fill, ...o })}/>`;
const line = (x1, y1, x2, y2, stroke, o = {}) => `<line${attrs({ x1: n(x1), y1: n(y1), x2: n(x2), y2: n(y2), stroke, ...o })}/>`;
const text = (x, y, s, o = {}) => `<text${attrs({ x: n(x), y: n(y), ...o })}>${s}</text>`;
const mid = { "text-anchor": "middle" };
const end = { "text-anchor": "end" };
const white = { fill: "#fff" };

// ---------------------------------------------------------------- ribbon parts

const chevron = (x, y) => `<path d="M${n(x)} ${n(y)}l3 3 3-3" fill="none" stroke="#555"/>`;
const box = (x, y, w, label) =>
  rect(x, y, w, 20, "#fff", { stroke: "#C6C6C6" }) + text(x + 5, y + 14, label) + chevron(x + w - 12, y + 8);

const icon = {
  cut: (x, y) => `<g fill="none" stroke="#555" stroke-width="1.1"><circle cx="${x + 3}" cy="${y + 9}" r="2.2"/>` +
    `<circle cx="${x + 9}" cy="${y + 9}" r="2.2"/><path d="M${x + 4} ${y + 7}L${x + 9} ${y}M${x + 8} ${y + 7}L${x + 3} ${y}"/></g>`,
  copy: (x, y) => rect(x, y, 8, 9, "#fff", { stroke: "#555" }) + rect(x + 4, y + 3, 8, 9, "#fff", { stroke: "#555" }),
  brush: (x, y) => rect(x, y, 11, 5, "#E8B84A", { stroke: "#8A6D1F", "stroke-width": ".8" }) +
    line(x + 5.5, y + 5, x + 5.5, y + 12, "#555", { "stroke-width": 2 }),
  cells: (x, y) => rect(x, y, 12, 10, "#fff", { stroke: "#777" }) + line(x, y + 5, x + 12, y + 5, "#777") +
    line(x + 6, y, x + 6, y + 10, "#777"),
  cond: (x, y) => rect(x, y, 12, 11, "#fff", { stroke: "#777" }) + rect(x + 2, y + 2, 8, 3, "#F4B183") +
    rect(x + 2, y + 6, 5, 3, "#A9D18E"),
  table: (x, y) => rect(x, y, 12, 11, "#fff", { stroke: "#777" }) + rect(x, y, 12, 3, "#5B9BD5") +
    line(x, y + 7, x + 12, y + 7, "#777"),
  styles: (x, y) => rect(x, y, 12, 11, "#fff", { stroke: "#777" }) + rect(x + 2, y + 2, 8, 7, "#FFD966"),
  wrap: (x, y) => `<path d="M${x} ${y + 2}h12M${x} ${y + 6}h10a2 2 0 0 1 0 4h-3M${x} ${y + 10}h5" fill="none" stroke="#555"/>`,
  merge: (x, y) => rect(x, y, 12, 11, "#fff", { stroke: "#777" }) + `<path d="M${x + 3} ${y + 5.5}h6" stroke="#2B579A"/>`,
};
const item = (x, y, kind, label) => icon[kind](x, y) + text(x + 17, y + 10, label);

const alignIcon = (x, y, align, widths = [13, 9, 13, 9]) => widths.map((lw, i) => {
  const x1 = align === "left" ? x : align === "right" ? x + 13 - lw : x + (13 - lw) / 2;
  return line(x1, y + i * 3, x1 + lw, y + i * 3, "#555");
}).join("");
const listIcon = (x, y) => [0, 1, 2].map((j) =>
  rect(x, y + j * 4, 2, 2, "#555") + line(x + 4, y + 1 + j * 4, x + 13, y + 1 + j * 4, "#555")).join("");
const vAlignIcon = (x, y, pos) => rect(x, y, 13, 12, "none", { stroke: "#999" }) +
  line(x + 3, y + [3, 6, 9][pos], x + 10, y + [3, 6, 9][pos], "#555", { "stroke-width": 2 });

const clipboard = (gx, gy) =>
  rect(gx + 10, gy + 8, 24, 30, "#B98B4E", { rx: 2 }) + rect(gx + 16, gy + 5, 12, 6, "#6B6B6B", { rx: 1.5 }) +
  rect(gx + 18, gy + 15, 18, 24, "#fff", { stroke: "#8A8A8A", "stroke-width": ".8" }) +
  `<path d="M${gx + 21} ${gy + 21}h12M${gx + 21} ${gy + 25}h12M${gx + 21} ${gy + 29}h8" stroke="#AAA"/>` +
  text(gx + 22, gy + 54, "Paste", mid) + chevron(gx + 19, gy + 60) +
  item(gx + 48, gy + 8, "cut", "Cut") + item(gx + 48, gy + 28, "copy", "Copy") +
  item(gx + 48, gy + 48, "brush", "Format Painter");

const font = (gx, gy) =>
  box(gx + 6, gy + 8, 112, "Calibri (Body)") + box(gx + 122, gy + 8, 36, "11") +
  text(gx + 166, gy + 23, "A", { "font-size": 14 }) + text(gx + 180, gy + 23, "A", { "font-size": 10 }) +
  text(gx + 10, gy + 50, "B", { "font-weight": 700, "font-size": 13 }) +
  text(gx + 30, gy + 50, "I", { "font-style": "italic", "font-size": 13, "font-family": "Georgia, serif" }) +
  text(gx + 48, gy + 50, "U", { "font-size": 13 }) + line(gx + 47, gy + 53, gx + 57, gy + 53, "#262626") +
  text(gx + 68, gy + 50, "abc") + line(gx + 67, gy + 46, gx + 86, gy + 46, "#262626") +
  text(gx + 100, gy + 50, "A", { "font-size": 13 }) + rect(gx + 98, gy + 53, 12, 3, "#E03A3A") +
  text(gx + 124, gy + 50, "ab") + rect(gx + 123, gy + 53, 14, 3, "#FFE600") +
  text(gx + 150, gy + 50, "Aa", { "font-size": 12 }) + chevron(gx + 168, gy + 44);

const paragraph = (gx, gy) =>
  listIcon(gx + 8, gy + 10) + chevron(gx + 23, gy + 13) + listIcon(gx + 36, gy + 10) + chevron(gx + 51, gy + 13) +
  alignIcon(gx + 70, gy + 10, "right", [13, 9, 9, 13]) + alignIcon(gx + 92, gy + 10, "right", [13, 9, 9, 13]) +
  text(gx + 116, gy + 22, "&#182;", { "font-size": 14 }) +
  alignIcon(gx + 8, gy + 42, "left") + alignIcon(gx + 30, gy + 42, "center") +
  alignIcon(gx + 52, gy + 42, "right") + alignIcon(gx + 74, gy + 42, "left", [13, 13, 13, 13]) +
  alignIcon(gx + 98, gy + 42, "left", [13, 13, 13, 13]) + rect(gx + 120, gy + 41, 13, 12, "#fff", { stroke: "#777", "stroke-dasharray": "2 1" });

const stylesGallery = (gx, gy) => [
  ["AaBbCcDd", "Normal", {}, true],
  ["AaBbCcDd", "No Spacing", {}],
  ["AaBbCc", "Heading 1", { fill: HEADING, "font-size": 13 }],
  ["AaB", "Title", { "font-size": 20, "font-family": "Calibri Light, Segoe UI Light, sans-serif", "font-weight": 300 }],
].map(([sample, name, o, sel], i) => {
  const x = gx + 6 + i * 61;
  return rect(x, gy + 6, 58, 62, sel ? "#DCE9F8" : "#fff", { stroke: sel ? "#8DB2E3" : "#E1E1E1" }) +
    text(x + 29, gy + 36, sample, { ...mid, ...o }) + text(x + 29, gy + 60, name, { ...mid, "font-size": 9.5, fill: "#444" });
}).join("");

const alignment = (gx, gy) =>
  vAlignIcon(gx + 8, gy + 8, 0) + vAlignIcon(gx + 26, gy + 8, 1) + vAlignIcon(gx + 44, gy + 8, 2) +
  item(gx + 72, gy + 9, "wrap", "Wrap Text") +
  alignIcon(gx + 8, gy + 40, "left") + alignIcon(gx + 26, gy + 40, "center") + alignIcon(gx + 44, gy + 40, "right") +
  item(gx + 72, gy + 39, "merge", "Merge &amp; Center");

const number = (gx, gy) =>
  box(gx + 6, gy + 8, 124, "General") +
  ["$", "%", ",", ".0", ".00"].map((s, i) => text(gx + 12 + i * 22, gy + 50, s, { "font-size": 12 })).join("");

const xlStyles = (gx, gy) =>
  item(gx + 8, gy + 8, "cond", "Conditional Formatting") + item(gx + 8, gy + 28, "table", "Format as Table") +
  item(gx + 8, gy + 48, "styles", "Cell Styles");

const cellsGroup = (gx, gy) =>
  item(gx + 8, gy + 8, "cells", "Insert") + item(gx + 8, gy + 28, "cells", "Delete") + item(gx + 8, gy + 48, "cells", "Format");

// ---------------------------------------------------------------- window chrome

let clipN = 0;
function appWindow({ x, y, w, h, color, title, tabs, groups, body }) {
  const id = `clip${clipN++}`;
  const ry = y + 60;
  let s = `<clipPath id="${id}"><rect x="${x}" y="${y}" width="${w}" height="${h}" rx="8"/></clipPath>`;
  s += rect(x, y, w, h, "#fff", { rx: 8, filter: "url(#shadow)" });
  s += `<g clip-path="url(#${id})">`;
  s += rect(x, y, w, 60, color);

  // quick access toolbar: save, undo, redo
  s += `<g fill="none" stroke="#fff" stroke-width="1.2" stroke-linejoin="round">` +
    `<path d="M${x + 14} ${y + 10}h10l3 3v9h-13z M${x + 17} ${y + 10}v4h6v-4 M${x + 17} ${y + 22}v-5h7v5"/>` +
    `<path d="M${x + 40} ${y + 14}h8a4 4 0 0 1 0 8h-4 M${x + 40} ${y + 14}l3-3 M${x + 40} ${y + 14}l3 3"/>` +
    `<path d="M${x + 70} ${y + 14}h-8a4 4 0 0 0 0 8h4 M${x + 70} ${y + 14}l-3-3 M${x + 70} ${y + 14}l-3 3"/></g>`;
  s += text(x + w / 2, y + 21, title, { ...mid, ...white, "font-size": 12 });

  // minimize, maximize, close
  const r = x + w;
  s += `<g stroke="#fff" fill="none"><path d="M${r - 120} ${y + 16.5}h10"/>` +
    `<rect x="${r - 74}" y="${y + 11.5}" width="10" height="10"/><path d="M${r - 28} ${y + 11.5}l10 10m0-10l-10 10"/></g>`;

  let tx = x + 6;
  tabs.forEach((name, i) => {
    const tw = name.length * 6.4 + 22;
    if (i === 1) s += rect(tx, y + 34, tw, 26, "#F3F3F3");
    s += text(tx + tw / 2, y + 51, name, { ...mid, "font-size": 12, fill: i === 1 ? color : "#fff" });
    tx += tw;
  });
  if (tx + 200 < r - 70)
    s += `<circle cx="${n(tx + 20)}" cy="${y + 46}" r="4.5" fill="none" stroke="#fff"/>` +
      line(tx + 18, y + 53, tx + 22, y + 53, "#fff") +
      text(tx + 32, y + 51, "Tell me what you want to do", { ...white, "font-size": 12, "fill-opacity": ".85" });
  s += `<circle cx="${r - 70}" cy="${y + 43}" r="3" fill="none" stroke="#fff"/>` +
    `<path d="M${r - 76} ${y + 53}a6 5 0 0 1 12 0" fill="none" stroke="#fff"/>` +
    text(r - 58, y + 51, "Share", { ...white, "font-size": 12 });

  s += rect(x, ry, w, 86, "#F3F3F3") + line(x, ry + 86, r, ry + 86, "#D4D4D4");
  let gx = x + 4;
  for (const g of groups) {
    s += g.draw(gx, ry) + text(gx + g.w / 2, ry + 80, g.label, { ...mid, "font-size": 10, fill: "#666" }) +
      line(gx + g.w, ry + 6, gx + g.w, ry + 80, "#D6D6D6");
    gx += g.w;
  }

  s += body(x, ry + 87, w, y + h);
  s += "</g>";
  s += rect(x + .5, y + .5, w - 1, h - 1, "none", { rx: 8, stroke: "#000", "stroke-opacity": ".25" });
  return s;
}

// ---------------------------------------------------------------- Word

function wordBody(x, by, w, bottom) {
  const status = bottom - 24;
  const pw = 480, px = x + (w - 14 - pw) / 2, py = by + 34;
  let s = rect(x, by, w, status - by, "#E6E6E6");

  s += rect(x, by, w, 20, "#F3F3F3") + rect(px, by + 4, pw, 12, "#fff");
  let d = "";
  for (let i = 0; i <= 50; i++) d += `M${n(px + i * pw / 50)} ${by + (i % 5 ? 11 : 7)}V${by + 14}`;
  s += `<path d="${d}" stroke="#999"/>`;

  s += rect(px, py, pw, status - py + 20, "#fff", { filter: "url(#page)" });

  const lx = px + 60;
  let yy = py + 64;
  const body = { "font-size": 10.5 };
  const heading = { "font-size": 15, fill: HEADING };
  const doc = [
    [0, "Quarterly Report", { "font-size": 26, fill: HEADING, "font-family": "Calibri Light, Segoe UI Light, Segoe UI, sans-serif", "font-weight": 300 }],
    [22, "Operations summary &#183; Q3 2026", { "font-size": 11.5, fill: "#5A5A5A" }],
    [34, "Overview", heading],
    [22, "Revenue grew 12% over the previous quarter, driven mostly by the", body],
    [16, "northern and western regions. Support tickets fell for the third", body],
    [16, "quarter in a row, and the new onboarding flow cut the average", body],
    [16, "setup time from two days to under five hours.", body],
    [30, "Highlights", heading],
    [22, "&#8226;", body, "Two new enterprise customers signed in August."],
    [16, "&#8226;", body, "Churn is down to 1.8%, the lowest since 2023."],
    [16, "&#8226;", body, "The Berlin office opened on schedule and under budget."],
    [30, "Next steps", heading],
    [22, "Hire three support engineers before the end of October and finish", body],
    [16, "the migration of the billing system to the new platform.", body],
  ];
  for (const [dy, s1, o, s2] of doc) {
    yy += dy;
    s += text(s2 ? lx + 4 : lx, yy, s1, o);
    if (s2) s += text(lx + 18, yy, s2, o);
  }
  s += line(lx, yy + 6, lx, yy + 19, "#000");

  s += rect(x + w - 14, by + 20, 14, status - by - 20, "#F0F0F0") + rect(x + w - 11, by + 26, 8, 110, "#C1C1C1", { rx: 4 });

  s += rect(x, status, w, 24, WORD);
  s += text(x + 12, status + 16, "Page 1 of 3", white) + text(x + 92, status + 16, "612 words", white) +
    text(x + 172, status + 16, "English (United States)", white);
  s += [0, 1, 2].map((i) => rect(x + w - 236 + i * 20, status + 7, 12, 10, "none", { stroke: "#fff" })).join("");
  s += text(x + w - 164, status + 16, "&#8722;", white) + line(x + w - 152, status + 12, x + w - 64, status + 12, "#fff") +
    rect(x + w - 109, status + 7, 3, 10, "#fff") + text(x + w - 58, status + 16, "+", white) +
    text(x + w - 12, status + 16, "100%", { ...white, ...end });
  return s;
}

// ---------------------------------------------------------------- Excel

function excelBody(x, by, w, bottom) {
  const status = bottom - 24, tabsY = status - 24;
  let s = rect(x, by, w, 28, "#fff") + line(x, by + 28, x + w, by + 28, "#D4D4D4");
  s += box(x + 6, by + 4, 80, "F4");
  s += `<path d="M${x + 100} ${by + 10}l8 8m0-8l-8 8M${x + 116} ${by + 14}l3 4 6-8" stroke="#999" fill="none"/>`;
  s += text(x + 134, by + 18, "fx", { "font-style": "italic", "font-family": "Georgia, serif", "font-size": 13, fill: "#555" });
  s += line(x + 152, by + 4, x + 152, by + 24, "#D4D4D4") + text(x + 160, by + 18, "=SUM(B4:E4)");

  const gy0 = by + 30, rh = 20, hw = 34;
  const letters = "ABCDEFGHIJ".split("");
  const widths = letters.map((c) => (c === "A" ? 118 : 76));
  const colX = [];
  let cx = x + hw;
  for (const cw of widths) { colX.push(cx); cx += cw; }
  const rows = Math.floor((tabsY - gy0 - rh) / rh);
  const top = (row) => gy0 + rh * row;

  s += rect(x, gy0, w, rh, "#F3F3F3") + rect(x, gy0, hw, tabsY - gy0, "#F3F3F3");
  let d = `M${x + hw} ${gy0}V${tabsY}`;
  for (const c of colX) d += `M${c + widths[colX.indexOf(c)]} ${gy0}V${tabsY}`;
  for (let row = 1; row <= rows + 1; row++) d += `M${x} ${top(row)}H${x + w}`;
  s += `<path d="${d}" stroke="#E1E1E1"/>`;

  // active column/row headers for the selected cell F4
  s += rect(colX[5], gy0, widths[5], rh, "#D2D2D2") + line(colX[5], gy0 + rh - 1, colX[5] + widths[5], gy0 + rh - 1, EXCEL, { "stroke-width": 2 });
  s += rect(x, top(4), hw, rh, "#D2D2D2") + line(x + hw - 1, top(4), x + hw - 1, top(5), EXCEL, { "stroke-width": 2 });
  letters.forEach((c, i) => s += text(colX[i] + widths[i] / 2, gy0 + 14, c, { ...mid, fill: i === 5 ? EXCEL : "#444" }));
  for (let row = 1; row <= rows; row++) s += text(x + hw / 2, top(row) + 14, row, { ...mid, fill: row === 4 ? EXCEL : "#444" });

  const data = [
    ["North", 48210, 51930, 55480, 58120],
    ["South", 39870, 41020, 40560, 44300],
    ["East", 52440, 54110, 57890, 61250],
    ["West", 45300, 47760, 50020, 53610],
    ["Central", 31550, 33080, 34900, 36470],
  ];
  const fmt = (v) => v.toLocaleString("en-US");
  const tableW = colX[5] + widths[5] - colX[0];
  const cell = (c, row, v, o = {}) => typeof v === "number"
    ? text(colX[c] + widths[c] - 6, top(row) + 14, fmt(v), { ...end, ...o })
    : text(colX[c] + 5, top(row) + 14, v, o);

  s += text(colX[0] + 5, top(1) + 15, "Regional sales, 2026", { "font-weight": 700, "font-size": 13 });
  s += rect(colX[0], top(3), tableW, rh, EXCEL);
  ["Region", "Q1", "Q2", "Q3", "Q4", "Total"].forEach((h, c) =>
    s += c ? text(colX[c] + widths[c] - 6, top(3) + 14, h, { ...end, ...white, "font-weight": 700 })
      : cell(0, 3, h, { ...white, "font-weight": 700 }));
  data.forEach((rowData, i) => {
    const row = 4 + i;
    if (i % 2 === 0) s += rect(colX[0], top(row), tableW, rh, "#E2EFDA");
    rowData.forEach((v, c) => s += cell(c, row, v));
    s += cell(5, row, rowData.slice(1).reduce((a, b) => a + b, 0));
  });
  const totals = [1, 2, 3, 4].map((c) => data.reduce((a, r) => a + r[c], 0));
  s += line(colX[0], top(9), colX[0] + tableW, top(9), EXCEL, { "stroke-width": 1.5 });
  s += cell(0, 9, "Total", { "font-weight": 700 });
  totals.forEach((v, i) => s += cell(i + 1, 9, v, { "font-weight": 700 }));
  s += cell(5, 9, totals.reduce((a, b) => a + b, 0), { "font-weight": 700 });

  s += rect(colX[5], top(4), widths[5], rh, "none", { stroke: EXCEL, "stroke-width": 2 }) +
    rect(colX[5] + widths[5] - 3, top(5) - 3, 6, 6, EXCEL, { stroke: "#fff" });

  s += rect(x, tabsY, w, 24, "#F3F3F3") + line(x, tabsY, x + w, tabsY, "#D4D4D4");
  s += `<path d="M${x + 16} ${tabsY + 8}l-5 4 5 4zM${x + 26} ${tabsY + 8}l5 4-5 4z" fill="#999"/>`;
  s += rect(x + 44, tabsY, 72, 22, "#fff") + line(x + 44, tabsY + 21, x + 116, tabsY + 21, EXCEL, { "stroke-width": 2 }) +
    text(x + 80, tabsY + 15, "Sheet1", { ...mid, fill: EXCEL, "font-weight": 700 }) +
    text(x + 150, tabsY + 15, "Summary", { ...mid, fill: "#444" }) +
    `<circle cx="${x + 200}" cy="${tabsY + 12}" r="7" fill="none" stroke="#888"/>` +
    `<path d="M${x + 196} ${tabsY + 12}h8M${x + 200} ${tabsY + 8}v8" stroke="#888"/>`;
  s += rect(x, status, w, 24, EXCEL) + text(x + 12, status + 16, "Ready", white) +
    text(x + w - 12, status + 16, "100%", { ...white, ...end });
  return s;
}

// ---------------------------------------------------------------- desktop

const label = (cx, y, s) => text(cx, y, s, { ...mid, ...white, "font-size": 11, stroke: "#000", "stroke-opacity": ".4", "stroke-width": 2, "paint-order": "stroke" });

const desktopIcons = () =>
  `<path d="M55 598h22l-2 26h-18z" fill="#DDE6F2" stroke="#6F84A3"/>` + rect(53, 593, 26, 5, "#C9D5E6", { stroke: "#6F84A3", rx: 1 }) +
  `<path d="M62 603v17M66 603v17M70 603v17" stroke="#8FA0BA"/>` + label(66, 644, "Recycle Bin") +
  `<path d="M130 598h10l3 3h17v24h-30z" fill="#E8A317"/>` + rect(130, 604, 30, 21, "#FFC83D", { rx: 1.5 }) +
  label(146, 644, "Projects");

function taskbar() {
  const ty = H - 48, cy = ty + 24;
  let s = rect(0, ty, W, 48, "#EEF2F8", { "fill-opacity": ".9" }) + line(0, ty + .5, W, ty + .5, "#fff", { "stroke-opacity": ".7" });
  const tile = (color, letter) => (x) => rect(x - 12, cy - 12, 24, 24, color, { rx: 4 }) +
    text(x, cy + 5, letter, { ...mid, ...white, "font-weight": 700, "font-size": 14 });
  const apps = [
    (x) => [[-11, -11], [.5, -11], [-11, .5], [.5, .5]].map(([dx, dy]) => rect(x + dx, cy + dy, 10.5, 10.5, "url(#win)")).join(""),
    (x) => `<g fill="none" stroke="#1F1F1F" stroke-width="1.8"><circle cx="${x - 2}" cy="${cy - 2}" r="7"/><path d="M${x + 3} ${cy + 3}l6 6"/></g>`,
    (x) => rect(x - 11, cy - 9, 14, 12, "#fff", { stroke: "#1F1F1F", "stroke-width": 1.4, rx: 2 }) + rect(x - 3, cy - 3, 14, 12, "#4A4A4A", { rx: 2 }),
    (x) => `<path d="M${x - 12} ${cy - 10}h9l3 3h12v17h-24z" fill="#E8A317"/>` + rect(x - 12, cy - 4, 24, 14, "#FFC83D", { rx: 1.5 }),
    (x) => `<circle cx="${x}" cy="${cy}" r="12" fill="url(#edge)"/>` +
      `<path d="M${x - 7} ${cy + 3}a7.5 7.5 0 0 1 14.5-3c0 3.5-3 5-6.5 5" fill="none" stroke="#fff" stroke-width="3" stroke-opacity=".85"/>`,
    tile("#185ABD", "W"),
    tile("#107C41", "X"),
    tile("#0F6CBD", "O"),
  ];
  const x0 = W / 2 - (apps.length - 1) * 22;
  const wx = x0 + 5 * 44, ex = x0 + 6 * 44;
  s += rect(wx - 20, cy - 20, 40, 40, "#fff", { rx: 4, "fill-opacity": ".8" });
  apps.forEach((draw, i) => s += draw(x0 + i * 44));
  s += rect(wx - 8, ty + 43, 16, 3, "#0067C0", { rx: 1.5 }) + rect(ex - 3, ty + 43, 6, 3, "#8A8A8A", { rx: 1.5 });

  s += `<circle cx="24" cy="${cy}" r="8" fill="#FFB900"/>` + text(40, cy - 2, "72&#176;F", { "font-size": 12, "font-weight": 600 }) +
    text(40, cy + 12, "Sunny", { fill: "#555" });

  const tray = W - 150;
  s += `<path d="M${tray - 48} ${cy + 2}l4-4 4 4" fill="none" stroke="#1F1F1F"/>`;
  s += `<g fill="none" stroke="#1F1F1F" stroke-width="1.5" stroke-linecap="round"><path d="M${tray - 26} ${cy - 3}a11 11 0 0 1 16 0M${tray - 23} ${cy + 1}a7 7 0 0 1 10 0"/></g>` +
    `<circle cx="${tray - 18}" cy="${cy + 5}" r="1.6" fill="#1F1F1F"/>`;
  s += `<path d="M${tray - 4} ${cy - 3}h3l5-4v14l-5-4h-3z" fill="#1F1F1F"/><path d="M${tray + 7} ${cy - 4}a6 6 0 0 1 0 8" fill="none" stroke="#1F1F1F" stroke-width="1.4"/>`;
  s += rect(tray + 18, cy - 5, 18, 10, "none", { stroke: "#1F1F1F", rx: 2 }) + rect(tray + 20, cy - 3, 11, 6, "#1F1F1F") + rect(tray + 36, cy - 2, 2, 4, "#1F1F1F");
  s += text(W - 16, cy - 3, "10:42 AM", { ...end, "font-size": 11.5 }) + text(W - 16, cy + 13, "9/22/2026", { ...end, "font-size": 11.5 });
  return s;
}

// ---------------------------------------------------------------- stars

// Evenly spread like the real break screen: one star per cell of a jittered grid.
function stars() {
  const rand = mulberry32(20260922);
  const palette = ["#F7A541", "#F5D63D", "#8ED96B", "#4CC38A", "#4AA8F0", "#6F8EF2", "#B08AE8", "#F48FB1", "#F2706F"];
  const byColor = new Map(palette.map((c) => [c, []]));
  const cell = 72;
  for (let gy = -cell / 3; gy < H; gy += cell)
    for (let gx = -cell / 3; gx < W; gx += cell) {
      const x = gx + rand() * cell, y = gy + rand() * cell;
      const scale = 1.2 + rand() ** 2 * 1.6;
      const rot = Math.round(rand() * 72);
      const color = palette[Math.floor(rand() * palette.length)];
      const op = .6 + rand() * .25;
      byColor.get(color).push(`<use href="#s" transform="translate(${n(x)} ${n(y)}) rotate(${rot}) scale(${scale.toFixed(2)})" fill-opacity="${op.toFixed(2)}"/>`);
    }
  return [...byColor].map(([c, uses]) => `<g fill="${c}">${uses.join("")}</g>`).join("");
}

// ---------------------------------------------------------------- assemble

const svg = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ${W} ${H}" font-family="Segoe UI, system-ui, sans-serif" font-size="11" fill="#262626" role="img" aria-label="A Windows desktop with Excel and Word open, covered edge to edge by small translucent stars in many colours: Enso Retreat's break screen.">
<defs>
<linearGradient id="wall" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#DCE8F7"/><stop offset=".55" stop-color="#9EC0EC"/><stop offset="1" stop-color="#4F7FD0"/></linearGradient>
<radialGradient id="bloom" cx=".7" cy=".6" r=".5"><stop offset="0" stop-color="#1E5BD6" stop-opacity=".8"/><stop offset=".5" stop-color="#3B7BE8" stop-opacity=".4"/><stop offset="1" stop-color="#3B7BE8" stop-opacity="0"/></radialGradient>
<linearGradient id="win" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#3AA0F3"/><stop offset="1" stop-color="#0067C0"/></linearGradient>
<linearGradient id="edge" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#35C1F1"/><stop offset=".6" stop-color="#1B8BD6"/><stop offset="1" stop-color="#0C59A4"/></linearGradient>
<filter id="shadow" x="-10%" y="-10%" width="120%" height="130%"><feDropShadow dx="0" dy="10" stdDeviation="14" flood-color="#0B2350" flood-opacity=".35"/></filter>
<filter id="page" x="-5%" y="-5%" width="110%" height="110%"><feDropShadow dx="0" dy="1" stdDeviation="2" flood-opacity=".25"/></filter>
<path id="s" d="M0-10L2.59-3.56 9.51-3.09 4.18 1.36 5.88 8.09 0 4.4-5.88 8.09-4.18 1.36-9.51-3.09-2.59-3.56Z"/>
</defs>
${rect(0, 0, W, H, "url(#wall)")}${rect(0, 0, W, H, "url(#bloom)")}
${desktopIcons()}
${appWindow({
  x: 40, y: 76, w: 700, h: 500, color: EXCEL, title: "Regional sales.xlsx - Excel",
  tabs: ["File", "Home", "Insert", "Page Layout", "Formulas", "Data", "Review", "View"],
  groups: [
    { label: "Clipboard", w: 150, draw: clipboard },
    { label: "Font", w: 200, draw: font },
    { label: "Alignment", w: 190, draw: alignment },
    { label: "Number", w: 140, draw: number },
    { label: "Styles", w: 170, draw: xlStyles },
    { label: "Cells", w: 90, draw: cellsGroup },
  ],
  body: excelBody,
})}
${appWindow({
  x: 372, y: 100, w: 790, h: 580, color: WORD, title: "Quarterly report.docx - Word",
  tabs: ["File", "Home", "Insert", "Design", "Layout", "References", "Mailings", "Review", "View"],
  groups: [
    { label: "Clipboard", w: 150, draw: clipboard },
    { label: "Font", w: 200, draw: font },
    { label: "Paragraph", w: 150, draw: paragraph },
    { label: "Styles", w: 256, draw: stylesGallery },
  ],
  body: wordBody,
})}
${taskbar()}
${stars()}
${text(40, 58, "00:50", { "font-size": 30, "font-weight": 600, fill: "#E8485F", stroke: "#fff", "stroke-opacity": ".6", "stroke-width": 3, "paint-order": "stroke" })}
</svg>
`;

await writeFile(OUT, svg, "utf8");
console.log(`${OUT}: ${Buffer.byteLength(svg)} bytes`);
