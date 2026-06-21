
# GAR Browser

This is browser that does not look like other typical web browser it only work in the terminal and it does not save almost any track in the memory while using it is written in c++. GAR routes all your traffic through Tor network anything been done is completely anonymous and private, through Terminal User Interface.

##  Features

### Privacy & Security
- **Native Tor Integration**: All traffic is routed through a local Tor SOCKS5 proxy .
- **Dark Web Ready**: Seamlessly browse both Clearnet domains and `.onion` hidden services.
- **Fingerprint Rotation**: Spoof and rotate HTTP headers and User-Agents to prevent browser fingerprinting tracking.
- **TLS & Header Inspection**: Built-in tools to deeply analyze SSL/TLS certificates and HTTP security headers of any website.
- **DNS Flush & New Identity**: Clear your DNS cache and signal the Tor daemon for a completely new circuit (new IP address) instantly.

### Modern TUI (Terminal User Interface)
- **Interactive UI**: Built using the FTXUI library, featuring a split-pane design, interactive forms, and keyboard navigation.
- **Universal Scrolling**: Custom scroll engine to seamlessly read long web pages directly in the terminal using `Page Up` and `Page Down`.
- **Smart Search**: Built-in DuckDuckGo Lite integration. Type a URL to go directly to it, or type a word/phrase to automatically search DuckDuckGo securely via Tor.
- **Content Parsers**: Automatically extracts Hyperlinks, Forms, and Media into organized side-panel menus using Google's Gumbo HTML parser.
- **Background Threading**: Heavy networking and Tor negotiations happen on background threads, ensuring the UI remains perfectly smooth and responsive.

---

## Prerequisites & Dependencies
To compile and run GAR on Windows, you will need the **MSYS2 (MinGW64)** environment.

**Required Packages:**
- `cmake`
- `gcc` / `g++` (MinGW64)
- `curl` (libcurl)
- `openssl` (libssl and libcrypto)
- `gumbo-parser`

**Automatically Fetched by CMake:**
- `FTXUI`
---

## Usage & Keyboard Shortcuts

Run the TUI browser: GAR_TUI.exe

### Global Keyboard Shortcuts
- `Tab` / `Shift+Tab`: Move focus between UI elements (URL Bar, Side Panel, etc.)
- `Up` / `Down` Arrows: Navigate through menus (Links, Forms, Media, History)
- `Enter`: Submit URL or click buttons
- `Page Up` / `Page Down` or `Fn Up ` / `Fn down`: Scroll the main text content up and down
- `F1`: View Shortcuts
- `F2`: Open **Settings** (Toggle Tor, set Media players)
- `F3`: Open **Security Panel** (Newnym, TLS Inspector, Header Analyzer, Request Replay)
- `F5`: Reload the current page

## Project Architecture

- `/src/ui/`: Contains the entry points for both the CLI and TUI rendering engines.
- `/src/core/`: Contains the custom HTTP Client, Socket integrations, and background threading handlers.
- `/src/anonymity/`: Contains the Tor Controller, Fingerprint Engine, and Secure Memory implementations.
- `/src/security/`: Contains the TLS/Header inspection and analysis modules.
- `/include/`: Header files for all modules.

---

## Prerequisites & Dependencies
To compile and run GAR on Windows, you will need the **MSYS2 (MinGW64)** environment.

**Required Packages:**
- `cmake`
- `gcc` / `g++` (MinGW64)
- `curl` (libcurl)
- `openssl` (libssl and libcrypto)
- `gumbo-parser`

**Automatically Fetched by CMake:**
- `FTXUI`
---