<div align="center">

# 🎮 Ollama 3DS

**Run local LLMs on your Nintendo 3DS — powered by Ollama**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Nintendo%203DS-red.svg)](https://www.nintendo.com/en-gb/Hardware/Nintendo-3DS-Family/)
[![Built with](https://img.shields.io/badge/Built%20with-devkitPro%20%2B%20libctru-blue.svg)](https://devkitpro.org/)
[![Ollama](https://img.shields.io/badge/Requires-Ollama-black.svg)](https://ollama.com)

*Chat with Gemma, Mistral, LLaMA and more — right from your 3DS!*

</div>

---

## 📖 Overview

**Ollama 3DS** is a homebrew application for the Nintendo 3DS family of consoles that connects to an [Ollama](https://ollama.com) server running on your local network and lets you perform **LLM inference** directly from your handheld.

Type a message using the 3DS software keyboard, send it to any model running on your PC, and read the response — all from your couch, with a 3DS in your hands.

> ⚠️ **Requires a modded 3DS** with the Homebrew Launcher (e.g. via [3ds.hacks.guide](https://3ds.hacks.guide)) and an Ollama server running on your local Wi-Fi network.

---

## ✨ Features

- 💬 **Full chat interface** — Send messages and receive responses from any Ollama model
- 🧠 **5-message conversation context** — The last 5 exchanges are remembered and sent with each request, giving the model conversation memory
- 🌐 **Multi-language UI** — Switch between 7 languages at runtime:
  - 🇬🇧 English
  - 🇮🇹 Italiano
  - 🇫🇷 Français
  - 🇪🇸 Español
  - 🇩🇪 Deutsch
  - 🇺🇦 Українська *(romanized)*
  - 🇯🇵 日本語 *(romanized)*
- ⚙️ **In-app configuration** — Change IP, port and model name on the fly, no recompilation needed
- 💾 **Persistent settings** — Configuration and language choice are saved to SD card (`/3ds/ollama3ds/config.ini`) and restored on next launch
- 🗑️ **Memory reset** — Clear conversation context instantly with one button
- 🎨 **Dual-screen UI** — Top screen shows the chat, bottom screen shows live context preview
- 📜 **Credits screen** — View tools, libraries and author info

---

## 🕹️ Controls

### Home Screen

| Button | Action |
|--------|--------|
| `A` | Open keyboard and write a message |
| `Y` | Clear conversation memory |
| `SELECT` | Open settings menu |
| `X` | Change language |
| `L` / `R` | View credits |
| `START` | Exit the app |

### Settings Menu

| Button | Action |
|--------|--------|
| `Up` / `Down` | Navigate |
| `A` | Edit selected field |
| `B` / `SELECT` | Back to home |

### Credits Screen

| Button | Action |
|--------|--------|
| `B` | Back to home |

---

## 📋 Requirements

### On the 3DS
- A **Nintendo 3DS / 3DS XL / 2DS / New 3DS** (any model)
- Custom firmware with **Homebrew Launcher** support
  - Follow [3ds.hacks.guide](https://3ds.hacks.guide) if you haven't already
- Wi-Fi connection to the same network as your server

### On your PC / Server
- [Ollama](https://ollama.com) installed and running
- At least one model pulled (e.g. `gemma3`, `mistral`, `phi3:mini`)
- Ollama must be listening on all interfaces:

```bash
OLLAMA_HOST=0.0.0.0 ollama serve
```

---

## 🚀 Getting Started

### Option A — Download (easiest)

1. Download the latest `ollama3ds.3dsx` from the [Releases](https://github.com/Dzhmelyk135/Ollama3DS/releases) page
2. Copy it to `/3ds/ollama3ds.3dsx` on your 3DS SD card
3. Launch via Homebrew Launcher

### Option B — Build from source

#### Prerequisites

Install [devkitPro](https://devkitpro.org/wiki/Getting_Started), then:

```bash
dkp-pacman -S 3ds-dev
```

#### Build

```bash
git clone https://github.com/Dzhmelyk135/Ollama3DS.git
cd Ollama3DS
make
```

This produces `ollama3ds.3dsx`. Copy it to `/3ds/` on your SD card.

---

## ⚙️ Configuration

On first launch, the app uses these defaults:

| Setting | Default |
|---------|---------|
| Host | `192.168.1.100` |
| Port | `11434` |
| Model | `gemma3` |
| Language | English |

Press **SELECT** in-app to change them. Press **"Save to SD"** to persist across reboots.

Settings are stored at:
```
/3ds/ollama3ds/config.ini
```

Example `config.ini`:
```ini
host=192.168.1.50
port=11434
model=mistral
lang=1
```

Language IDs: `0`=EN, `1`=IT, `2`=FR, `3`=ES, `4`=DE, `5`=UK, `6`=JA

---

## 🧠 How It Works

```
┌─────────────┐        Wi-Fi (HTTP)        ┌──────────────────┐
│  Nintendo   │  ─── POST /api/chat ──▶   │   Your PC        │
│    3DS      │  ◀── NDJSON stream ───    │   Ollama Server  │
│  Ollama3DS  │                            │   (any model)    │
└─────────────┘                            └──────────────────┘
```

1. You type a message using the **3DS software keyboard**
2. The app builds a JSON payload with the **full conversation history** (up to 5 pairs) and POSTs it to `http://<host>:<port>/api/chat`
3. Ollama streams back an **NDJSON response** (one JSON object per token)
4. The app collects all chunks, **assembles the full response**, and displays it
5. Both your message and the AI reply are saved to the **in-memory context** for the next turn

---

## 🤖 Tested Models

| Model | Works | Notes |
|-------|-------|-------|
| `gemma3` | ✅ | Recommended |
| `mistral` | ✅ | Good quality |
| `llama3.2` | ✅ | |
| `phi3:mini` | ✅ | Fastest |
| Large models (>13B) | ⚠️ | May time out or exceed buffer |

> 💡 **Tip:** Use smaller/quantized models for faster responses. The 3DS has a 4 KB response buffer limit — very long replies may be truncated.

---

## 📁 Project Structure

```
Ollama3DS/
├── source/
│   └── main.c          # Full app source (C99)
├── Makefile             # devkitPro build system
├── icon.png             # 48x48 app icon (shown in Homebrew Launcher)
├── LICENSE              # MIT License
└── README.md
```

---

## 🛠️ Built With

| Tool / Library | Purpose |
|----------------|---------|
| [devkitPro](https://devkitpro.org/) | 3DS homebrew toolchain |
| [devkitARM](https://devkitpro.org/wiki/devkitARM) | ARM cross-compiler |
| [libctru](https://github.com/devkitPro/libctru) | 3DS system library (HTTP, keyboard, graphics, sockets) |
| [Ollama](https://ollama.com) | Local LLM inference server |
| [GitHub Copilot](https://github.com/features/copilot) | AI coding assistant |

---

## 👤 Credits

<div align="center">

| Role | Info |
|------|------|
| **Developer** | [Dzhmelyk135](https://github.com/Dzhmelyk135) |
| **Repository** | [github.com/Dzhmelyk135/Ollama3DS](https://github.com/Dzhmelyk135/Ollama3DS) |

</div>

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 🤝 Contributing

Contributions, issues and feature requests are welcome!
Feel free to open an [issue](https://github.com/Dzhmelyk135/Ollama3DS/issues) or submit a pull request.

---

## ⚠️ Disclaimer

This project is an **unofficial homebrew application** and is not affiliated with, endorsed by, or connected to Nintendo Co., Ltd. or Ollama in any way.
Use of homebrew software may void your warranty. Use at your own risk.

---

<div align="center">

*Made with ❤️ and a Nintendo 3DS*

**[⬆ Back to top](#-ollama-3ds)**

</div>
