# Pages publishing

The static landing page is in site/. Only that directory is included in the manual Pages deployment workflow. Architecture, code, requirements and other private material are excluded. The workflow is manually triggered to prevent accidental publication.

Preview locally with python -m http.server --directory site 8000 and open http://localhost:8000.

GitHub Pages on a personal private repository requires an eligible GitHub plan; its published site is public. Review the landing page before enabling Pages in Settings and selecting GitHub Actions as source, then manually run Publish ZARC Pages. Do not add private material to site/.
