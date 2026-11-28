# Translation and review workflow

1. Treat the [English README quick start](../../README.md#quick-start) as the
   source. Translate the explanatory text; copy command and code blocks exactly.
2. Use the shared [EN / 简体中文 / 한국어 glossary](GLOSSARY.md). Add a term
   only when the source uses it and preserve its meaning across both languages.
3. Update [Chinese](quickstart.zh-CN.md) and [Korean](quickstart.ko.md)
   versions together. Keep each translation marked
   as a draft until a native-language technical reviewer checks meaning,
   terminology, commands, and scope limitations.
4. Record the reviewed source revision, reviewer, date, and review reference in
   the change that updates translation status. Do not imply partner acceptance
   without a separate recorded review.
5. Run `python tests/localization_test.py` and
   `python docs/verify_documentation.py` after edits. The localization test
   checks glossary coverage, source links, and verbatim quick-start code blocks.

Current status: Chinese and Korean quick starts are drafts; technical review
and partner acceptance are open.
