# Pages publishing

The static landing page is in site/. Only that directory is included in the manual Pages deployment workflow. Architecture, code, requirements and other repository documentation are excluded. The workflow is manually triggered to prevent accidental publication.

Preview locally with python -m http.server --directory site 8000 and open http://localhost:8000.

The repository and Pages site are public. GitHub Actions is the configured publishing source. Review changes to site/ and manually run Publish ZARC Pages. Do not add private material to site/.
