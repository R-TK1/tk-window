![top_banner](./.github/banner.jpg)

---

### TKWindow
![workflow_badge](https://img.shields.io/github/actions/workflow/status/R-TK1/tk-window/build.yml?label=Build%20Test)
![loc_badge](https://github.com/R-TK1/tk-window/blob/badges/loc.svg)
![latest_release_badge](https://img.shields.io/github/v/release/R-TK1/tk-window?label=Experimental&include_preleases=true)

A very tiny, very specialized windowing wrapper over Wayland and X11 written in [C23](https://en.wikipedia.org/wiki/C23_(C_standard_revision)). It's built to be performant and simple, supporting as many Linux [windowing backends](https://en.wikipedia.org/wiki/Windowing_system) as possible. **MacOS is not currently supported, but planned. Windows is not planned.**

---

#### Dependencies
TKWindow requires, currently, one of the following windowing libraries.

- [Wayland](https://wayland.freedesktop.org/): Wayland is a newer windowing library quickly being adopted over X11. This is by **far** preferred to X11 when talking about TKWindow. It's better tested on and smaller.
- [X11](https://www.x.org/wiki/): X11 is a reliable, battle-hardened windowing library that's been around since desktop on Linux was really a thing.

---

![bottom_banner](./.github/banner.jpg)