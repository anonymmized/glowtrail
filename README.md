# fireplace

A minimal **terminal fireplace** written in **C** — smooth flame simulation rendered with ANSI truecolor.

Designed as a clean background animation for a terminal window.

---

## Features

- Smooth flame diffusion (heat buffer)
- ANSI 24-bit color (truecolor)
- No external dependencies
- Handles terminal resize
- Runs in an alternate screen buffer (no terminal trash)

---

## Build

### macOS / Linux

```bash
cc -O2 fireplace.c -o fireplace
```

---

## Run

```bash
./fireplace
```

Exit with:

```text
Ctrl + C
```

---

## Notes

- Best viewed in a modern terminal with truecolor support.
- For the smoothest look, use a larger terminal window.

---
