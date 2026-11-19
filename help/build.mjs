// Builds help/retreat.html, the help page the installer ships and the settings window opens
// through file:///, from help/index.html, the copy served on the website.
//
//   node help/build.mjs
//
// retreat.html has to work as a single file, so the stylesheet links and the favicon that index.html
// takes from ../landing are inlined, fonts as data: URIs. No npm packages needed.

import { readFile, writeFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import path from "node:path";

const here = path.dirname(fileURLToPath(import.meta.url));
const fontTypes = { woff2: "font/woff2", woff: "font/woff", ttf: "font/ttf", otf: "font/otf" };

async function inlineStylesheet(cssPath) {
  let css = await readFile(cssPath, "utf8");
  const re = /url\(\s*(['"]?)([^'")]+\.(woff2?|ttf|otf))\1\s*\)/gi;
  for (const [match, , ref, ext] of [...css.matchAll(re)]) {
    const font = await readFile(path.resolve(path.dirname(cssPath), ref));
    css = css.replace(match, `url(data:${fontTypes[ext.toLowerCase()]};base64,${font.toString("base64")})`);
  }
  return css;
}

let html = await readFile(path.join(here, "index.html"), "utf8");

// the comment that explains the link below only applies to index.html
html = html.replace(/[ \t]*<!-- help\/build\.mjs [^>]*-->\r?\n/, "");

const links = [...html.matchAll(/<link rel="stylesheet" href="([^"]+)">/g)];
for (const [tag, href] of links) {
  const css = await inlineStylesheet(path.resolve(here, href));
  html = html.replace(tag, `<style>\n${css.trim()}\n</style>`);
}

html = await (async () => {
  const m = html.match(/<link rel="icon" type="image\/svg\+xml" href="([^"]+)">/);
  if (!m) return html;
  const svg = await readFile(path.resolve(here, m[1]));
  return html.replace(m[1], `data:image/svg+xml;base64,${svg.toString("base64")}`);
})();

const out = path.join(here, "retreat.html");
await writeFile(out, html);
console.log(`wrote ${path.relative(process.cwd(), out)} (${Math.round(Buffer.byteLength(html) / 1024)} KB)`);
