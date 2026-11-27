# Pages publishing

The static landing page is in site/. The public [modern SDV reference architecture](https://susmithmrm.github.io/ZARC/modern-architecture.html) uses locally hosted SVG and responsive HTML/CSS, with no external rendering script. Only site/ is included in the Pages deployment artifact; the remainder of the repository is excluded.

Preview locally with python -m http.server --directory site 8000 and open http://localhost:8000.

GitHub Actions is the configured publishing source. The workflow can be manually triggered and also runs on main when site/ or the Pages workflow changes. Local HTML links and SVG XML are checked before deployment. Review public site changes before pushing. Do not add private material to site/.
