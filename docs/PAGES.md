# GitHub Pages publication guide

The public site presents the intended ZARC architecture, partner integration and verification plan. Only `site/` is published. The sedan visualization is illustrative and read-only; candidate configuration records are not physical support or completed implementation evidence.

Preview the static site with:

```sh
python -m http.server --directory site 8000
```

Open http://localhost:8000. GitHub Actions is the publishing source. Validate local HTML links, SVG XML and renderer behavior before deployment. Keep private customer information out of the public site. The authoritative requirements are in [Z12](architecture/12-Requirements-Verification.csv).
