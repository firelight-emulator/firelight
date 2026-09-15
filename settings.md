Settings

## Video

- Vsync
- Fullscreen
- Custom resolution + lock?
- Open game in new window

## Audio

- Output device
- Master Volume
- Mute
- Show global mute indicator
- UI SFX Volume
- Notification sounds
- Mono
- Sound fade L/R?

## Library

- Combine variants
- Preferred region
- Preferred language
- 

## Controllers

- Reorder controllers
- Button mappings
- Triggers
- Thumb sticks sensitivity, invert, deadzones
- Analog to digital stick
- Preferred controller per console/game
- Preferred controllers
- Turbo

## General
- Pause on focus loss
- Don't pause but block input on focus loss
- BIOS?
- Performance overlay
- Discord rich presence

## Notifications

## Controls?

- Only allow P1 to control menus
- Allow sticks to do analog inputs
- Only allow P1 to do hotkeys
- Global hotkey suppression

## Appearance?

- Accent color
- Background customization

## Accessibility
- Interface scale
- UI density

## Emulation Picture

- Picture Mode / Aspect Ratio
- Integer scale #
- BFI
- Runahead frames
- Warn on runahead lag
- Rotate picture

## Emulation Sound

- Game audio
- Mute while fastforward
- Mute while slowing down
- Dynamic rate control
- Audio latency (buffer size)

## Emulation General

- Rewind window
- Rewind period
- Rewind enabled
- Auto save frequency
- Auto save slots
- Suspend point slots
- Screenshot enable
- Enable video clips
- Video clip timing
- Sync method
- Save file location
- Fast forward multiplier
- 

## Achievements

- Credentials
- Hardcore mode
- Unofficial achievements 
- Replay mode or whatever
- Show challenge indicators
- Show mastery / completed
- Show leaderboards
- Notifications
- Screenshot on achievements
- Video clip on achievements




######

# Emulator and frontend settings survey

What nine emulators and frontends expose as settings, how they group them, and where users get confused.

Prepared 2026-09-12.

## 0. Method and sources

Settings lists for RetroArch, ares, mGBA, DuckStation, PCSX2, Dolphin and Snes9x were read out of their current source trees — menu builders, Qt `.ui` files and settings widgets — so the category names and option labels are the strings those programs actually show. ES-DE and the Steam Deck have no source I read here; their structure comes from ES-DE's own user guide and from Valve's documented Quick Access Menu, cross-checked against community guides. Sentiment and confusion notes come from project documentation, developer blog posts, popular setup guides and forum threads, and are labelled where a specific source matters.

| Project | Commit / source | Date |
|---|---|---|
| RetroArch | `cd8580f` (`menu/menu_displaylist.c`, `menu/menu_setting.c`, `config.def.h`) | 2026-09-10 |
| ares | `af4cbb0` (`desktop-ui/settings/*.cpp`) | 2026-09-06 |
| mGBA | `543a197` (`src/platform/qt/SettingsView.ui`) | 2026-09-10 |
| DuckStation | `3b30876` (`src/duckstation-qt/*.ui`, `src/core/fullscreenui_settings.cpp`) | 2026-09-09 |
| PCSX2 | `4a4cbb8` (`pcsx2-qt/Settings/*.ui`) | 2026-09-10 |
| Dolphin | `a2efdf1` (`Source/Core/DolphinQt/Config`, `Settings`) | 2026-09-06 |
| Snes9x | `7a8878f` (`gtk/src/snes9x.ui`) | 2026-09-04 |
| ES-DE | User guide menu reference | fetched 2026-09-12 |
| Steam Deck | SteamOS Quick Access Menu and Settings, per Valve's documentation and community guides | fetched 2026-09-12 |

A caveat on option lists: where a list is long I have kept every distinct option but dropped enumerated values (the twelve entries of a dropdown become one line). Nothing is invented; if I could not verify an item it is not here.

## 1. The shape of the problem

Nine programs, four organizing philosophies.

**Frontend-shaped (RetroArch).** Settings are organized around the *frontend's* subsystems — drivers, video, audio, input, latency, playlists, directories — and anything console-specific is pushed into a separate, per-core surface. Twenty-nine top-level categories, several hundred options.

**Console-shaped (DuckStation, PCSX2, Dolphin, Snes9x).** Categories name parts of the emulated machine or of the rendering pipeline: Console, BIOS, Memory Cards, Graphics, Game Fixes. One console means the categories can be concrete.

**Minimal (ares, mGBA).** Eleven and nine categories respectively, flat, with per-system quirks confined to a single page.

**Library-shaped (ES-DE, Steam Deck).** The settings are about the *shelf* — scraping, collections, themes, sorting — plus, in Steam Deck's case, a per-game performance layer that has nothing to do with emulation at all.

The two largest distinctions that cut across all of them, and that cause most of the confusion documented below:

- **Whose setting is it?** Frontend settings and emulator/core settings live in different places, are saved to different files, and often have overlapping names. RetroArch's split between its own Settings tree and per-core Options is the canonical example.
- **What scope does it apply at?** Global, per-system, per-game — and in RetroArch also per-content-directory. Every program that supports more than one scope has generated confusion about which one a change landed in.

## 2. RetroArch

### 2.1 Top-level Settings categories

Drivers · Video · Audio · Input · Latency · Core · Configuration · Saving · Logging · File Browser · Frame Throttle · Recording · On-Screen Display · User Interface · AI Service · Accessibility · Power Management · Achievements · Network · Playlists · User · Directory · Steam.

Platform builds add: Bluetooth, Wi-Fi, CPU Performance and Power, Sustained Performance Mode, GameMode, Lakka Services, Lakka Switch Options.

Separately from this tree there is the **Quick Menu**, reachable only with a game loaded, which holds Core Options, Controls (remaps), Shaders, Cheats, Disc Control, Save States, Achievements and the override save actions.

### 2.2 Contents, by category

**Video** — split into Output, Scaling, Synchronization, Fullscreen Mode, Windowed Mode, HDR and CRT SwitchRes sub-menus, plus screensaver suspend, software filter selection, brightness and shader delay.

- *Output:* video driver, threaded video, GPU index, monitor index, swapchain bit depth, rotation, screen orientation, resolution, window offsets, gamma, soft filter, flicker filter, refresh rate (set, polled, automatic), frame-time sampling from display, automatic refresh-rate switching with a PAL threshold, force sRGB off.
- *Scaling:* integer scale, integer scale axis and mode, aspect ratio index, custom aspect ratio.
- *Synchronization:* vsync, swap interval, shader subframes, scan subframes, black frame insertion and dark frames, adaptive vsync, hard GPU sync and frames, waitable swapchains, threaded present repeat, threaded display pacing, present timing from display, max frame latency, max swapchain images, scanline sync, frame delay and automatic frame delay, sync to exact content framerate.

**Audio** — Output, Microphone, Synchronization, MIDI and Mixer sub-menus, plus volumes, mutes, fast-forward and rewind mute, fast-forward speedup, respect silent mode, DSP plugin.

- *Output:* enable, driver, device, output rate, latency, resampler driver and quality, s16 fast path, format negotiation, channel layout, virtual surround for headphones, WASAPI exclusive mode and shared buffer length, ASIO channel and control panel.
- *Synchronization:* audio sync, threaded pipeline, thread priority, max timing skew, rate control delta, sink rate estimation.

**Input** — RetroPad binds, turbo fire, hotkeys, menu controls, sensors, haptic feedback, max users, auto mouse grab, auto game focus, bind timeout and hold, autoconfiguration, remap enable and sorting, keyboard-to-gamepad mapping, descriptor display, axis thresholds, analog deadzone and sensitivity, mouse scale, an extensive touch and virtual-mouse group, block timeout, poll type behavior, input and joypad drivers.

**Latency** — automatic frame delay and frame delay, run-ahead mode, run-ahead frames, preemptive frames, hide run-ahead warnings, audio latency, microphone latency, and duplicates of the vsync, hard sync, waitable swapchain, max frame latency, max swapchain images and scanline sync settings from Video.

**Frame Throttle** — rewind sub-menu, frame time counter sub-menu, fast-forward ratio, fast-forward frameskip, fast-forward mute and speedup, slow-motion ratio, sync to exact content framerate, menu throttle.

**Saving** — cloud sync, save file and savestate sorting (by core, by content), keeping saves in the content directory, save and savestate compression, autosave interval, SRAM overwrite block, savestate autosave/autoload/auto-index/max keep, savestate thumbnails, replay auto-index, max keep, checkpoint interval, screenshot placement, GPU screenshot, runtime logging.

**Core** — updater sub-menu, core info cache, savestate bypass, system files in content directory, core option categories, driver switch, allow rotation, shared hardware context, dummy core on shutdown, always reload core, core manager.

**Configuration** — save config on exit, minimal save, save remaps on exit, game-specific options, automatic overrides, automatic remaps, initial disk change, global core options.

**User Interface** — menu appearance, menu visibility ("Views"), menu controls, notifications, overlays, file browser, plus pause behaviors (pause when menu open, when inactive, on disconnect), resume-on-savestate and on-disk-insert, quit and confirm behaviors, remember selection, startup page, navigation wraparound, show advanced settings, kiosk mode and password, threaded data runloop, screensaver timeout and animation, desktop-menu options (view type, thumbnails, cache limits, theme), language, menu driver.

**On-Screen Display** — notifications (widgets, scale, font, position, colors, background) and overlays.

**Playlists** — playlist manager, on-demand thumbnails, filename display, history and favorites size, truncation, sorting, portable paths, rename and remove, sublabels and runtime display, fuzzy archive matching, scan behavior, format and compression.

**Directory** — roughly thirty path settings, one per asset class.

**Achievements** — enable, credentials, appearance and visibility sub-menus, hardcore mode, leaderboards, rich presence, badges, unofficial achievements, unlock sound, automatic screenshot, start active.

### 2.3 Commentary

**The volume is the identity.** RetroArch's settings surface is both its main draw and its most-cited barrier. RetroGameCorps' widely-used starter guide, which many people treat as the de-facto manual, opens by warning about the learning curve before it explains anything. The "Show Advanced Settings" toggle exists precisely because the developers know this, but it is on by default in most builds, so new users see everything.

**The four-way configuration split is the single biggest confusion source.** Frontend settings save to `retroarch.cfg` and can be overridden per core, per content directory, or per game; input remaps are a separate `.rmp` system; core options are a third system in `.opt` files; shader presets are a fourth. RetroArch's own documentation lists these as four parallel mechanisms, and RetroGameCorps flags the situation as confusing in the guide text itself. The practical failure mode is well documented: someone follows a guide that says "set internal resolution to 4x", looks for it in Settings → Video, and never finds it, because it is a core option in the Quick Menu.

**Quick Menu invisibility.** Everything per-game — core options, remaps, shaders, overrides — requires a game to be loaded first. Users looking for these in the main Settings tree with no content running conclude the features do not exist.

**Silent override precedence.** Once a core or game override exists, later changes made in the global Settings tree appear to be discarded. This has generated long-lived bug reports and forum threads; it is working as designed, but the design is not visible in the UI.

**Duplicate options across categories.** Vsync, hard sync, max swapchain images and frame delay appear in both Video → Synchronization and Latency. That is deliberate (two tasks, one setting), and it does help task-oriented navigation, but it also means a user who changed something in one place and then looks at the other cannot tell whether they are looking at the same setting or a second one.

**Terminology that only means something to insiders.** "Max swapchain images", "hard GPU sync", "shader subframes", "poll type behavior", "threaded display pacing", "audio max timing skew". The sublabels are genuinely good where they exist, but many of these settings have measurable interactions the sublabel cannot convey.

**What people praise.** Breadth, shaders, run-ahead and per-game precision once mastered; the fact that any setting can be scoped down to one game; and the extensive sublabels, which are more informative than most emulators' tooltips.

## 3. ares

### 3.1 Categories

Video · Audio · Input · Hotkeys · Emulators · Options · Firmware · Paths · Cores · Developer · Settings File.

### 3.2 Contents

- **Video:** color adjustment (luminance, saturation, gamma); display settings (color bleed, color emulation, interframe blending, overscan, pixel accuracy mode); device settings (driver, fullscreen monitor, format, exclusive mode, force sRGB, threaded, native fullscreen).
- **Audio:** sound effects (volume, balance); device settings (output device, frequency, latency, dynamic rate).
- **Input:** focus-loss behavior (pause, block input, allow input), digital-to-analog conversion mode and center-to-edge time, virtual gamepad assignment.
- **Options:** synchronization (sync to audio, sync to video); emulator options (run-ahead, periodic memory auto-save, suppress secondary media prompts); rewind settings (enable, frequency, length, mute while rewinding).
- **Cores:** per-system settings on one page — N64 Expansion Pak, controller pak size, video interface processing, weave deinterlacing, resolution scale and supersampling; GBA Game Boy Player; SNES deep black boost; Mega Drive TMSS boot ROM.
- **Firmware:** a table of BIOS locations by emulator, type and region.
- **Paths:** home, firmware, saves, screenshots, debugging files, arcade ROMs, each with a "same as game path" option.
- **Emulators:** which systems appear in the load menu.
- **Developer:** debug server port and IPv4, homebrew development mode, deterministic entropy, force interpreter.

### 3.3 Commentary

**Deliberate scarcity.** ares has about forty settings total. Where RetroArch offers six pacing controls, ares offers two radio buttons and a note recommending audio sync. The design position is that an accuracy-focused emulator should not need tuning, and for its audience that lands well.

**Self-documenting labels.** Nearly every toggle carries a one-line description directly under it ("Blurs adjacent pixels for translucency effects" under Color Bleed). This is a small thing that repeatedly gets praised in comparison with emulators whose options are bare nouns.

**The Cores page is the pressure point.** One page holds all per-system settings for every supported machine, and it carries a note that changes need a game reload. As ares adds systems this page will not scale, and there is no per-game scope at all: a user who wants 4x rendering for one N64 game and native for another has no mechanism.

**Confusion areas.** "Dynamic rate" in Audio is unexplained and, on current master, appears not to be wired into the SDL3 driver at all. "Pixel accuracy mode" and "color emulation" are not distinguished for users who do not already know what they do. Run-ahead is a single checkbox with no frame count, which is simpler but also less capable than the multi-frame implementations elsewhere.

## 4. mGBA

### 4.1 Categories

Audio/Video · Gameplay · Interface · Update · Emulation · Enhancements · BIOS · Paths · Logging · Game Boy.

### 4.2 Contents

- **Audio/Video:** audio driver, buffer size, sample rate, volume, mute, fast-forward volume, multiplayer audio routing; display driver, frameskip, lock aspect ratio, force integer scaling, interframe blending, bilinear filtering, FPS target (including a "Native (59.7275)" entry), sync to video and/or audio.
- **Gameplay:** on-load behavior (load last state, load cheats, autorun scripts), periodic autosave, save entered cheats, savestate extra data (screenshot, save game, cheat codes) for both save and load, Discord rich presence.
- **Interface:** language, library list or tree view, show when no game open, filename versus ROM name, clear cache, allow opposing directions, suspend screensaver, pause when inactive or minimized, dynamic window title, FPS in title bar, OSD messages, frame count in OSD, emulation info on reset, custom border.
- **Emulation:** fast-forward speed and held speed (both with an unbounded option), autofire interval, rewind enable, history and speed, idle loop handling, preload entire ROM, Game Boy Player features, VBA bug compatibility.
- **Enhancements:** video renderer (software or OpenGL), high-resolution scale.
- **BIOS:** per-model BIOS paths, use BIOS if found, skip BIOS intro.
- **Paths:** save games, savestates, screenshots, patches, cheats, each with a "same directory as the ROM" option.
- **Game Boy:** model selection per cartridge type, Game Boy palettes and presets, Super Game Boy borders, palette preference order, Game Boy Camera source.

### 4.3 Commentary

**A good middle.** Nine categories, everything visible, no scoping model to learn. For a single-family emulator this is close to the right size, and mGBA is rarely criticized for its settings.

**Two genuinely confusing items.** First, "FPS target" defaults to 60 while the GBA runs at 59.7275, which means the default configuration runs games about 0.46% fast with pitch raised to match; the menu offers the native rate but does not explain why you would want it. Second, "Sync: Video / Audio" are two independent checkboxes rather than a single choice, and the interaction between them (and with the FPS target) is not documented in the UI.

**The per-ROM save path option is the pattern users ask for elsewhere.** mGBA's Paths page gives every save class a "same directory as the ROM" checkbox. This is exactly the convention that frontends with hash-keyed save folders get asked to support.

## 5. DuckStation

### 5.1 Categories

Summary · Interface · Game List · BIOS · Console · Emulation · Patches · Cheats · Memory Cards · Graphics · On-Screen Display · Post-Processing · Audio · Achievements · Capture · Advanced · Debugging. The Big Picture UI presents the same set, minus the desktop-only pages.

### 5.2 Contents

- **Console:** region, frame rate, fast boot, fast-forward memory card access, fast-forward boot, 8MB RAM; CPU execution mode, clock speed control, recompiler ICache; CD-ROM read and seek speedup, preload image to RAM, ignore subcode, apply image patches, switch disc on stop.
- **Emulation:** speed control (normal, fast-forward, turbo); latency control (vsync, sync to host refresh rate, optimal frame pacing, reduce input latency, skip duplicate frame display, frame time buffer); rewind (enable, save frequency, buffer size); runahead (frames, software renderer low-VRAM mode, enable for analog input).
- **Graphics:** renderer, adapter, internal resolution, down-sampling, texture and sprite filtering, dithering, deinterlacing, aspect ratio, crop and fine crop, scaling and FMV scaling, PGXP geometry correction, PGXP depth buffer, force 4:3 for FMVs, FMV chroma smoothing, widescreen rendering, rounded texture coordinates, exclusive fullscreen, disable mailbox presentation, blit swap chain, multi-sampling, line detection, threaded rendering, max queued frames, plus a large PGXP sub-group, a texture replacement group, and a device options group of a dozen "disable feature X" toggles.
- **Audio:** backend, output device, stretch mode, buffer size, output latency (with a "minimal" option), a live readout of total latency broken into stretch, buffer and output components, volumes and mutes, and a time-stretching group (sequence length, seek window, overlap, quick seek, anti-aliasing filter).
- **On-Screen Display:** display scale and margins, theme and fonts, message behavior and per-severity durations, location, and fourteen individual "Show X" toggles for FPS, speed, CPU and GPU usage, resolution, GPU statistics, frame times, latency statistics, controller input and settings.
- **Interface:** behavior (confirm close, save state on close, inhibit screensaver, pause on disconnect or focus loss, background input, Big Picture on start, Discord), game display (fullscreen, render to separate window, cursor, window resizing), appearance (language, theme), updates.
- **Advanced / Debugging:** log level and sinks, debug menu, RAIntegration, cache and cover directories, and a set of rendering-internals toggles.
- Plus BIOS, Game List, Memory Cards, Patches, Cheats, Post-Processing, Achievements and Capture pages.

### 5.3 Commentary

**The stated philosophy is on the project's front page:** hack-style options are discouraged, and the default configuration is meant to run everything playable, with only some enhancements causing compatibility problems. That posture is visible in the UI — there is a Safe Mode control that disables all enhancements at once — and it is the main reason DuckStation's settings are generally praised rather than complained about.

**Per-game settings are first-class and legible.** Every page can be opened in a per-game context where each control shows "Use Global Setting" until you change it. This is the clearest implementation of scoping in the group: the widget itself tells you which scope you are in. RetroArch's override system is more powerful and much less visible.

**The confusion has moved from "how do I configure this" to "how much should I enable".** Current setup guides converge on the same three changes (Vulkan, 4x internal resolution, PGXP geometry correction) and then explicitly warn against stacking more; one recent guide frames the problem as enthusiasm working against the user, since every enhancement is another thing that can break a specific game. PGXP in particular has a geometry mode that is broadly safe and a depth-buffer mode labelled low-compatibility in the UI itself, and users frequently enable both.

**The audio latency readout is a small masterpiece.** Rather than three independent numbers, the page computes and displays total latency as the sum of its parts, updating live. Very few emulators do this, and it removes an entire class of "why is my audio delayed" question.

**Where it still trips people:** the distinction between "Optimal Frame Pacing", "Reduce Input Latency" and "Sync To Host Refresh Rate" is not obvious from the labels; "Maximum Latency" in audio versus "Output Latency" reads as a duplicate until you see the breakdown; and the Graphics page's device-options group is a wall of negative toggles that look alarming to a novice who scrolls that far.

## 6. PCSX2

### 6.1 Categories

Interface · Game List · BIOS · Emulation · Graphics (tabbed) · Audio · Memory Cards · Network & HDD · Achievements · Folders · Advanced · Debug, plus per-game Patches, Cheats and Game Fixes.

Graphics is itself tabbed: Display · Hardware Rendering · Software Rendering · Hardware Fixes · Upscaling Fixes · Texture Replacement · Post-Processing · OSD · Media Capture · Advanced.

### 6.2 Contents

- **Emulation:** speed control (normal, fast-forward, slow-motion); system settings (EE cycle rate, EE cycle skipping, enable cheats, MTVU, host filesystem, fast CDVD, CDVD precaching, thread pinning); frame pacing and latency (maximum frame latency, sync to host refresh rate, vsync, optimal frame pacing, use host vsync timing, skip presenting duplicate frames, advanced frame display); real-time clock.
- **Graphics → Display:** aspect ratio, FMV aspect ratio override, deinterlacing, crop, vertical stretch, fullscreen mode, bilinear filtering, integer scaling, widescreen and no-interlacing patches, anti-blur, disable interlace offset, screen offsets, show overscan.
- **Graphics → Hardware Rendering:** internal resolution, mipmapping, texture filtering, trilinear filtering, anisotropic filtering, dithering, blending accuracy, force 32-bit, accurate alpha test, AA1, rasterizer ordered view, manual hardware fixes toggle.
- **Graphics → Hardware Fixes and Upscaling Fixes:** roughly thirty expert controls — CPU sprite render size, software CLUT render, auto flush, texture inside RT, skip draw range, half-pixel offset, round sprite, align sprite, merge sprite, native scaling, texture offsets, preload frame data, GPU palette conversion, and so on.
- **Audio:** driver, backend, output device, expansion mode, synchronization, buffer size, output latency with a live maximum-latency readout, volumes.
- **Advanced:** IOP recompiler options, VU rounding and clamping modes per unit, instant VU1, mVU flag hack, game fixes and compatibility patches toggles, savestate backup and compression, PINE.
- **OSD:** scale, margin, message and performance positions, font and style, and around twenty "Show X" toggles.

### 6.3 Commentary

**PCSX2 2.0 was explicitly a settings-reduction project.** The team's own release post describes the old layout — graphics fixes buried in a plugin, CPU modes in their own tabs, speed hacks in another, game fixes somewhere else — and says the result was confusing. Their fix was automation: a shipped game index that applies the right fixes per title, and an Automatic renderer that picks a backend based on tested GPU behavior, with the advice to trust it. That is a rare and instructive move: the settings still exist, but the expectation is that nobody touches them.

**The warning banner is doing real work.** The Advanced page carries a notice that changing these options can break games and that the team will not support such configurations. This is the most explicit "here be dragons" fence in the group, and it is a reasonable pattern for any app with an expert tier.

**The legacy of the old guide culture is the main confusion source.** A decade of "best settings" articles told users to enable speed hacks, set EE cycle rate and skipping, and configure frameskip. Those articles are still highly ranked, still recommend settings the project now considers harmful, and still describe a plugin architecture that no longer exists. Users arrive with instructions that do not match the UI.

**Hardware Fixes and Upscaling Fixes are unusable without per-game knowledge.** Options like half-pixel offset, round sprite and texture inside RT only make sense against a specific rendering artifact in a specific game. The game index removes the need for most users, but anyone who opens those tabs sees three dozen controls with no way to evaluate them.

**"Skip Presenting Duplicate Frames" is a good example of a setting whose effect is invisible where users look for it.** It has a documented interaction with external frame-generation tools that only surfaces in forum threads, not in the UI.

## 7. Dolphin

### 7.1 Categories

General · Interface · Audio · Controllers · Graphics (General, Enhancements, Hacks, Advanced) · GameCube · Wii · Paths · Advanced, plus per-game properties.

### 7.2 Contents

- **General:** dual core (labelled a speedhack), cheats, load whole game into memory, allow mismatched region settings, change discs automatically, Discord presence, speed limit, fallback region, update channel, usage statistics.
- **Interface:** theme and style, language, built-in game name database, debugging UI, hotkey focus requirement, screensaver inhibit, play time tracking, keep window on top, confirm on stop, panic handlers, active title in window title, pause on focus loss, cursor visibility and lock.
- **Audio:** DSP emulation engine (HLE, LLE recompiler, LLE interpreter), volume, backend, Dolby Pro Logic II decoder and decoding quality (each option labelled with its latency), output device, latency, fill audio gaps, preserve audio pitch, mute when disabling speed limit, audio buffer size, Wii Remote audio routing.
- **Advanced:** CPU emulation engine, MMU, pause on panic, write-back cache; timing (correct time drift, rush frame presentation, smooth early presentation); clock override, VBI frequency override, memory size override, custom RTC.
- **Graphics → General:** backend, adapter, aspect ratio and custom aspect ratio, vsync, precision frame timing, start in fullscreen, auto-adjust window size, render to main window, shader compilation mode, compile shaders before starting.
- **Graphics → Enhancements:** internal resolution, anti-aliasing, texture filtering, output resampling, color correction, post-processing effect, scaled EFB copy, per-pixel lighting, widescreen hack, disable fog, force 24-bit color, disable copy filter, arbitrary mipmap detection, HDR post-processing, and a stereoscopy group.
- **Graphics → Hacks:** EFB group (skip access from CPU, ignore format changes, store copies to texture only, defer copies to RAM), texture cache accuracy slider, GPU texture decoding, XFB group (store copies to texture only, immediately present XFB, skip presenting duplicate frames), fast depth calculation, disable bounding box, vertex rounding, save texture cache to state, VBI skip.
- **Graphics → Advanced:** debugging and dumping tools, custom textures, graphics mods, frame dumping, progressive scan, backend multithreading, borderless fullscreen, and experimental toggles.

### 7.3 Commentary

**Dolphin labels its own dangerous settings, which is unusual and good.** "Enable Dual Core (speedhack)", "Enable Write-Back Cache (slow)", "LLE Interpreter (very slow)", and the Hacks page being called Hacks. The audio decoding quality dropdown puts the latency cost in each option's label. This is a pattern worth stealing outright.

**The tooltips are the best in the group.** Each option gets a paragraph explaining the trade-off and ending with a standard "if unsure, leave this unchecked" line. That closing sentence resolves the most common user question — *should I touch this?* — without requiring them to understand the setting.

**The GameINI system is invisible, which causes a recurring class of confusion.** Dolphin ships per-game settings that silently override defaults. Community guidance repeatedly has to explain that if dual core is off for a particular game, it is off deliberately. Users who change a global setting and see no effect, or who see a setting mysteriously flip, are meeting the game database.

**The timing options added recently are expert-only and read as similar.** Correct Time Drift, Rush Frame Presentation and Smooth Early Presentation all concern the same subsystem, all default off, and their tooltips have to explain interactions between them (Rush is described as generally making pacing worse and recommends a larger audio buffer). Even with good tooltip text, three interacting toggles is a lot of surface.

**The EFB/XFB vocabulary is a wall.** "Skip EFB Access from CPU", "Store XFB Copies to Texture Only", "Defer EFB Copies to RAM" mean nothing without GameCube graphics-pipeline knowledge, and they are among the most commonly recommended settings in old guides, so users flip them without understanding.

## 8. Snes9x (GTK port)

### 8.1 Categories

Display (Basic Settings, Scaling, Hardware Acceleration) · Sound · Emulation (Speed Control, Rewind, Hacks) · Files · Joypads (Buttons, Turbo/Sticky, Joystick Options) · Shortcut keys · Netplay.

### 8.2 Contents

- **Display:** fullscreen on ROM open, show local time, frame rate, pressed keys, fast-forward and pause indicators, overscanned height, OSD size, fullscreen resolution; scale to fit, aspect ratio, high-resolution effect, scaling filter, NTSC video presets (composite, S-Video, RGB, monochrome) with artifacts, sharpness, brightness, contrast, saturation, hue, gamma, fringing, bleed; merge odd and even fields, scanline intensity; bilinear filter output, FreeSync/G-Sync mode, sync to vertical blank, reduce input lag, shader, byte-order inversion.
- **Sound:** sound driver, automatically adjust input rate to display, dynamic rate control, mute, mute on turbo or rewind, playback rate, buffer size, dynamic rate limit, input rate, video rate.
- **Emulation:** throttling method; rewind buffer size and frames between snapshots; hacks — allow invalid VRAM access, allow opposing d-pad directions, overclock CPU, remove sprite limit, redirect echo buffer overflow, SuperFX clock speed percentage, sound filter.
- **Files:** SRAM, savestate, cheat, patch and export locations; SRAM autosave interval.
- **Joypads:** per-pad bindings, turbo and sticky button flags per button, axis threshold, calibration.

### 8.3 Commentary

**An honest, old-fashioned preferences dialog.** Tabs, dense grids, no scoping, no per-game anything. For its audience that is fine, and the NTSC filter controls are more thorough than most modern emulators offer.

**The audio section is unusually well-specified and unusually jargon-heavy at the same time.** It exposes input rate, video rate, dynamic rate control and a dynamic rate limit — four related controls with no explanation of how they interact. The default rate limit of 5 corresponds to ±0.5% correction, which is the same magnitude RetroArch and bsnes use, but nothing in the UI says so.

**"Reduce input lag" and "Sync to vertical blank" sit next to each other with no indication that one affects the other.** This is the same problem RetroArch's Latency page has, in miniature.

**The Hacks group is honest about being hacks,** and "Redirect echo buffer overflow" is a good example of a setting that is meaningful to maybe a hundred people and harmless to leave alone — which is an argument for an expert tier rather than for removal.

## 9. ES-DE (EmulationStation Desktop Edition)

### 9.1 Categories

Main menu: Scraper · UI Settings · Sound Settings · Input Device Settings · Game Collection Settings · Other Settings · Quit. Sub-menus under Scraper include Account settings, Content settings, Miximage settings and its own Other settings. UI Settings contains Media Viewer settings and Screensaver settings (with slideshow and video sub-pages). There is also a per-gamelist Options menu — sort, filter, collection editing, metadata editor — which is separate from the main settings.

### 9.2 Character of each area

- **Scraper:** which scraper service and account, what media types and metadata to fetch, region and language preferences, miximage generation options, and behavior on conflicts.
- **UI Settings:** theme, theme variant and color scheme, transitions and menu effects, startup system, gamelist behaviors, favorites sorting and markers, on-screen help, scroll indicators, media viewer and screensaver configuration.
- **Sound Settings:** navigation sounds, video audio, system volumes.
- **Input Device Settings:** controller configuration, swap A/B, ignore keyboard.
- **Game Collection Settings:** automatic collections, custom collections, grouping behaviors.
- **Other Settings:** alternative emulators (with a per-game toggle), media and ROM directories, VRAM limit, display and rendering options, save behavior for metadata, custom event scripts, hidden files and games, kid/kiosk mode controls, debug options.

### 9.3 Commentary

**The categories are library-shaped, not machine-shaped,** which is correct for a frontend and is the closest structural analogue in this survey to what a launcher needs. Note that ES-DE has no emulation settings at all: it launches external emulators, so anything about rendering or pacing is out of scope by construction.

**"Other Settings" is where everything hard ended up.** Alternative emulators, media directories, event scripts, VRAM limit and debug mode all live in the same bucket, and the bucket is where guides keep sending people. Three separate community projects' setup instructions consist largely of "press Start, go to Other Settings, enable these three toggles". A category named for what is left over becomes the category users visit most.

**Alternative emulator selection has a design flaw the project's own issue tracker documents:** the per-system choice is written into the gamelist metadata file rather than into system configuration, which breaks when the same library is shared between platforms that need different emulators. This is a good cautionary example about where scoped settings get stored.

**Kiosk and Kid modes are a genuinely good idea executed simply** — they restrict the menu rather than hiding individual settings, with an unlock code to return.

**Sentiment is broadly positive about defaults** and negative about scraper complexity, which is the part with the most conditional behavior (which media types, from which source, overwriting what).

## 10. Steam Deck (SteamOS Game Mode)

### 10.1 Two separate surfaces

**Quick Access Menu → Performance** (per-game when Advanced View and the per-game profile toggle are on): Framerate Limit, Refresh Rate, Allow Tearing, Half Rate Shading, Thermal Power (TDP) Limit, Manual GPU Clock Control and GPU Clock Frequency, Scaling Filter (including FSR and its sharpness, NIS, integer, linear, nearest), plus the performance overlay level. Per-game profiles were added after launch; before that every setting was global, and press coverage at the time treated the addition as a significant fix.

**Settings** (system-wide): System, Bluetooth, Internet, Display, Power, Notifications, Storage, Audio, Controller, Accessibility, Customization, Security, Developer.

**Per-game Properties** (a third surface): compatibility tool (Proton version), launch options, controller layout, shader pre-caching, game resolution and scaling mode.

### 10.2 Commentary

**The best idea here is the per-game performance profile with a global fallback.** One toggle switches the whole panel between "this game" and "everything", the panel itself is unchanged, and a game with no profile silently uses the global settings. It is the cleanest scoping UI in this entire survey, and it is worth studying next to DuckStation's "Use Global Setting" approach: Steam's is one switch for the page, DuckStation's is per-control.

**Refresh Rate and Framerate Limit being separate controls confuses nearly everyone at first.** The forum record is full of people asking why they cannot set 40 fps, the answer being that the display refresh must be set to 40 Hz first. Valve's own 40 Hz advocacy made this worse before the coupling was made clearer in the UI.

**"Half Rate Shading" is a good example of a well-named-but-unexplained setting.** Community guides list it in every per-game recommendation table without ever explaining what it does, and the values recommended are inconsistent across guides for the same game.

**The three-surface split is the real weakness.** Performance settings are in the QAM, Proton and resolution in Properties, and controller and display in Settings. Emulation-focused Deck users, who typically also have EmuDeck and ES-DE in the mix, end up with settings in five places.

## 11. Cross-cutting analysis

### 11.1 The recurring taxonomy

Strip away naming differences and almost every program in this survey has the same eight buckets:

| Bucket | Appears as |
|---|---|
| Display / video output | Video (RetroArch), Graphics (DuckStation, PCSX2), Display (Snes9x, ares), Audio/Video (mGBA) |
| Audio output | Audio everywhere |
| Input and binding | Input, Controllers, Joypads, Hotkeys |
| Speed, pacing and latency | Latency + Frame Throttle (RetroArch), Emulation (DuckStation, PCSX2), Advanced → Timing (Dolphin), Options → Synchronization (ares), Performance (Steam Deck) |
| System and firmware | Console, BIOS, Cores, Firmware, GameCube/Wii |
| Saves and state | Saving (RetroArch), Memory Cards, Files, Paths |
| Library and presentation | Playlists, Game List, UI Settings, Interface |
| Diagnostics and expert | Advanced, Debugging, Logging, Hacks |

Three programs split pacing across two or more categories (RetroArch, Dolphin, PCSX2) and every one of them has confusion attributable to that split.

### 11.2 Scoping models, ranked by clarity

1. **Steam Deck.** One per-game toggle on the page; unset means global. Visible, single mechanism.
2. **DuckStation.** Per-control "Use Global Setting" placeholder in a per-game context. Slightly more work to read, but no ambiguity about what is overridden.
3. **PCSX2.** Per-game settings plus a shipped game index that silently applies known-good fixes, with a UI indicator that patches are active.
4. **Dolphin.** Per-game INI plus a shipped GameINI database, but with no in-UI indication that a game-specific value is in force.
5. **ES-DE.** Per-game alternative emulator, stored in gamelist metadata, with a badge on affected entries.
6. **RetroArch.** Four parallel mechanisms (overrides, remaps, core options, shader presets), three scopes each, no in-UI indication of which file a value came from.
7. **ares, mGBA, Snes9x.** No scoping at all.

The pattern is clear: power correlates with confusion, and the mitigation that works is showing the scope in the control itself, not documenting it.

### 11.3 Where users consistently get lost

- **Frontend settings versus emulator settings.** Every multi-system program has this and every one of them generates the same support question. Naming does not solve it; placement does. DuckStation avoids it by being one program with one settings tree.
- **Pacing and latency vocabulary.** Vsync, swap interval, frame delay, pre-frame sleep, optimal frame pacing, max frame latency, sync to host refresh. Across nine programs there are at least six different names for adjacent concepts, and no two programs agree.
- **Audio latency arithmetic.** Buffer size, output latency, device latency and stretch buffers all contribute, and only DuckStation and PCSX2 show the total.
- **Enhancement stacking.** The failure mode has shifted from "I cannot make it run" to "I turned everything on and now a specific game is broken". DuckStation's Safe Mode and PCSX2's warning banner are the two direct responses.
- **Stale guidance.** Old best-settings articles outlive the UIs they describe. PCSX2 is the worst affected; RetroArch is next.
- **Settings that only exist while a game is running.** RetroArch's Quick Menu is the extreme case.

### 11.4 Patterns worth stealing

- Dolphin's "if unsure, leave this unchecked" tooltip ending, and its practice of putting the cost in the label ("speedhack", "slow", "Latency ~40 ms").
- DuckStation's live total-latency readout that decomposes into its parts, and its Safe Mode switch.
- PCSX2's shipped per-game index with an explicit unsafe-settings warning surface.
- Steam Deck's single per-game profile toggle with global fallback.
- ares's one-line description under every toggle.
- mGBA's "same directory as the ROM" option on every path.
- ES-DE's Kiosk and Kid modes as whole-menu restrictions rather than per-setting hiding.
- RetroArch's "Show Advanced Settings" idea, executed better — as a default-off expert tier rather than a toggle most builds ship enabled.

### 11.5 Anti-patterns

- A category named "Other" (ES-DE) or "Advanced" (several) becoming the dumping ground users visit most.
- The same setting duplicated across categories with no indication they are the same value (RetroArch's vsync group).
- Expert controls with no visible precondition — PCSX2's upscaling fixes, Dolphin's EFB toggles — presented at the same visual weight as ordinary options.
- Scoped values with no in-UI provenance, so a user cannot tell why a global change did nothing.
- Per-game state stored in library metadata rather than configuration (ES-DE's alternative emulators), which breaks when the library is shared.

