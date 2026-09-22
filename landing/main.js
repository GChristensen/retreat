// General page logic: iframe-aware links, latest-release download link, demo video.
(function () {
  "use strict";

  var RELEASES_API = "https://api.github.com/repos/GChristensen/retreat/releases/latest";

  // ---- iframe-aware link handling ----
  // When embedded in an iframe, links open in a new tab; standalone, they
  // navigate normally in the same tab.
  function isEmbedded() {
    try {
      return window.self !== window.top;
    } catch (e) {
      // cross-origin parent access throws, which itself implies embedding
      return true;
    }
  }

  if (isEmbedded()) {
    var links = document.querySelectorAll("a[href]");
    for (var i = 0; i < links.length; i++) {
      links[i].target = "_blank";
      links[i].rel = "noopener noreferrer";
    }
  }

  // ---- latest-release download link ----
  // The buttons already point at the releases page as a working fallback;
  // this swaps in the direct .exe asset when the GitHub API is reachable.
  function setDownloadLinks() {
    fetch(RELEASES_API, { headers: { Accept: "application/vnd.github+json" } })
      .then(function (res) { return res.ok ? res.json() : null; })
      .then(function (release) {
        if (!release || !release.assets) return;
        var asset = release.assets.find(function (a) { return /\.exe$/i.test(a.name); });
        if (!asset) return;
        var links = document.querySelectorAll(".js-download-link");
        for (var i = 0; i < links.length; i++) links[i].href = asset.browser_download_url;
        var versions = document.querySelectorAll(".js-download-version");
        for (var i = 0; i < versions.length; i++) versions[i].textContent = release.tag_name;
      })
      .catch(function () { /* keep the releases-page fallback */ });
  }

  // ---- demo poster / video ----
  function initDemo() {
    var poster = document.getElementById("demo-poster");
    var wrap = document.getElementById("demo-video");
    if (!poster || !wrap) return;
    var video = wrap.querySelector("video");

    function play() {
      poster.hidden = true;
      wrap.hidden = false;
      video.currentTime = 0;
      video.play().catch(function () { /* autoplay may be blocked; controls remain */ });
    }

    poster.addEventListener("click", play);
    poster.addEventListener("keydown", function (e) {
      if (e.key === "Enter" || e.key === " ") { e.preventDefault(); play(); }
    });
  }

  document.addEventListener("DOMContentLoaded", function () {
    setDownloadLinks();
    initDemo();
  });
})();
