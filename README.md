# glowtrail

A minimal terminal animation written in **C**: a smooth moving **glow trail** (comet-like) rendered with ANSI truecolor.

Designed to be lightweight, clean, and visually pleasing in any modern terminal.

---

## Preview

Run it in a fullscreen terminal for the best look.

---

## Features

- Smooth trail effect (fade buffer)
- ANSI **24-bit color** (truecolor)
- No external dependencies
- Handles terminal resize
- Clean redraw (no leftover characters)
- Works great as a background terminal “vibe” animation

---

## Build

### Linux / macOS

```bash
cc -O2 glowtrail.c -o glowtrail -lm
```

## Run 
```bash
./glowtrail
```
Exit with: 
`Ctrl + C`

## Notes
- Best viewed in a terminal that supports truecolor.
- If your terminal looks “blocky”, try increasing font size or running in a larger window.
