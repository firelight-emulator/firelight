# The settings catalog

Every setting Firelight has, declared once. The catalog is read at startup from this folder — every
`.json` under it, recursively, in sorted path order. Each file is the same shape as a whole catalog
carrying only the keys it needs, so where a setting lives is organisation, not meaning.

    _layout.json        pages + groups
    NN-<page>.json      the `app` settings for that page
    40-emulation.json   `common` settings
    cores/*.json        one file per core

The `NN-` prefix is the owning page's `order`, so reading the folder in name order reads the settings
in the order the UI shows them. It is presentation, not meaning: nothing parses the filename.

One unreadable or unparseable file fails the whole load and leaves the previous catalog in place. A
half-loaded catalog is worse than none: a missing key resolves to an empty default, which looks
exactly like a setting the user never touched.

## Which array a setting goes in

Every setting Firelight has, declared once. 'app' settings are frontend concepts (appearance,
general): single-valued, no core mapping, read only at the global tier. 'common' settings are
emulation concepts applied to every core (rewind, aspect ratio, ...) and are overridable per-
platform and per-game. 'cores' settings are keyed by CORE (not platform) because core option
keys differ per core, and are likewise overridable. Each 'cores.<coreName>' entry has 'settings'
(friendly, shown in the UI) and 'defaults' (Firelight's opinionated overrides of the core's own
raw option defaults). Which array a setting is in decides how it's read — there is no scope
field. A friendly setting with a 'mapping' drives one or more core options ('values' maps
friendly value -> core value; omit for identity, and omit 'mapping' entirely when the setting's
own key IS the core option key). 'visibleWhen'/'enabledWhen' gate a setting on other settings'
values (clauses AND-ed; a clause holds when the named setting is one of 'values').

## Pages, groups and ordering

'pages' are the settings nav entries (one route each); 'groups' are the titled section cards
within a page. Every setting names a 'group', and the group names its 'page' — that's what lets
a page auto-render and what tells search where a result lives. Both take an 'order' (ascending;
ties keep declaration order). A setting's 'keywords' are extra search terms beyond its
label/description — the words users actually think in ('vsync' for Sync method). A page's
'keywords' match the page itself.

## Types and widgets

'type' accepts author-friendly aliases: 'boolean'/'toggle' -> BOOLEAN; 'options'/'dropdown' ->
OPTIONS (needs 'options'); 'segmented' -> OPTIONS as a segmented control; 'radio' -> OPTIONS as
a radio list; 'integer'/'slider' -> INTEGER (slider); 'number'/'spinbox' -> INTEGER (spinbox);
'stepper' -> INTEGER (stepper); 'custom' -> CUSTOM (needs an explicit 'widget' naming a QML
delegate, e.g. 'gbc-palette'); 'game-picker' -> OPTIONS dropdown whose options are the user's
eligible library games (filled in at runtime; the stored value is the chosen game's content
hash); 'audio-device' -> OPTIONS dropdown whose options are the machine's audio outputs (filled
in at runtime; '' means the system default); 'text' -> STRING free-text field (optional
'placeholder'); 'color' -> STRING hex color (optional 'options' as preset swatches);
'file'/'file-picker' -> STRING filesystem path (optional 'extensions' array, no dot);
'folder'/'folder-picker' -> STRING directory path; 'key-binding' -> STRING captured key; 'multi-
select' -> OPTIONS checklist over 'options' whose value is a JSON array of selected option
values. An explicit 'widget' overrides the derived control. Slider/number settings take 'min',
'max', and 'step' (all numeric). 'step' may be fractional (e.g. 0.25 -> float slider), a whole
number other than 1 (e.g. 5 -> increments of 5), and defaults to 1 when omitted. A 'game-picker'
takes 'eligiblePlatformIds' (array of platform ids; omit for any platform).
