# LibreWeb Browser

[![Telegram](https://img.shields.io/badge/chat-on%20telegram-brightgreen)](https://t.me/libreweb)
[![Matrix](https://img.shields.io/badge/chat-on%20matrix-brightgreen)](https://matrix.to/#/#libreweb:melroy.org)
[![Gitter](https://img.shields.io/badge/chat-on%20gitter-brightgreen)](https://gitter.im/LibreWeb/Browser)

Decentralized Web-Browser; a revolution of the WWW.

What would you do different; if you could **reinvent** The Internet in 21st century? With all the knowledge and new technologies available today.

I was inspired by Douglas Engelbart, Tim Berners-Lee and Ted Nelson as well as projects like IPFS, Jekyll, ARPANET, and more.

*Note:* This project is still work in progress. However, we have a working [alpha version available](https://gitlab.melroy.org/libreweb/browser/-/releases).

## For Users

### Download

Just download the latest LibreWeb release and get started:

* [Download the latest release](https://gitlab.melroy.org/libreweb/browser/-/releases)

### Documentation

Visit the [dedicated documentation site](https://docs.libreweb.org) for *user* documentation.

### Screenshots

![Browser Screenshot](./misc/browser_screenshot.png)  
![Browser Markdown Editor](./misc/browser_screenshot_2.png)

### Community

Join our [Telegram group](https://t.me/libreweb) or [Matrix channel](https://matrix.to/#/#libreweb:melroy.org?via=melroy.org) and become part of our community!

### Ideas / Features

The current success criteria:

* Everyone should be able to easily **read** and **create** a site/blog/news page and publish the content online (without minimal technical knowledge);
* Built-in easy-to-use **editor** (whenever you want to publish some content without programming language knowledge);
* **Decentralized** (no single-point of failure or censorship), like: P2P, DHT and IPFS;
* *No* client-server approach (the client is also the server and visa versa) - think **mesh network**.
* **Encrypted** transfers;
* Data is stored **redundantly** within the network (no single-point of failure);
* **Versioning**/revisions of content and documenents (automatically solves broken 'links', that can't happy anymore);
* Publisher user should be able to add additional information about the document/page, eg. title or path (similar in how Jekyll is using the `YML` format for meta data)
* Human-readable source-code (eg. `Markdown` format, could be extended as well);
* You are in control about the layout and styling (just like with e-books);
* Content is King;
* Fast and Extensible!

*Note:* Since HyperText (so is HTML) is not used, you can even ditch the HTTP protocol. However TLS, for encryption, can still be used.

---

The sections below are mainly relevant for software developers, who want to contribute or help LibreWeb Browser.

## For Developers

Decentralized Browser is written C++ together with some [external libraries](/lib). LibreWeb is using the [cmark-gfm](https://github.com/github/cmark-gfm) library for example, which is used for CommonMark (markdown) parsing.  
We're using markdown as the source-code of the content/site. No HTML and JavaScript anymore, content is king after all.

LibreWeb Browser is also using [Gnome GTK3](https://developer.gnome.org/gtk3/stable/) framework for the GUI. Using the C++ bindings, called [Gtkmm](https://gtkmm.org/en/).

### Development Environment

Personally, I'm using VSCodium editor, with the following extensions installed: `C/C++`, `CMake`, `CMake Tools`, `PlantUML`, `Markdown All in One`, `vscode-icons` and `GitLab Workflow`.

But that is up to you.

### Build Dependencies

For the GNU/Linux build you need at least:

* GCC 9 or higher (`build-essential`, `g++-9`)
* CMake (Package: `cmake`)
* Ninja build system (Package: `ninja-build`)
* Libcurl (Package: `libcurl4-openssl-dev`)
* GTK & Pango (including C++ bindings):
  * Package: `libgtkmm-3.0-dev` under Debian based distros
* Clang-format (Package: `clang-format`)

*Note:* For cross-compiling towards Windows, see the cross-compile section below.

### Build

Clone the source-code with SSH (do not forget `--recurse-submodules`):

```sh
git clone --recurse-submodules -j5 git@gitlab.melroy.org:libreweb/browser.git
```

Start the build, which is using CMake and Ninja build system, using the wrapper script:

```sh
./scripts/build.sh
```

Optionally, use the VSCode `CMake Tools` extension to start the build or build with debug targets.


Build a release target, including packaging under GNU/Linux, using: `./scripts/build_prod.sh`

### C++ Style Guidelines

We use our [own Clang LLVM C++ Programming Style Format](.clang-format), using [clang-format](https://clang.llvm.org/docs/ClangFormat.html) command.

To automatically comply to our style format execute following script (inplace edits are performed for you):

```sh
./scripts/fix_format.sh
```

Check only for errors, run: `./scripts/check_format.sh`

### Doxygen

See latest [Developer Documentation](https://gitlab.melroy.org/libreweb/browser/-/jobs/artifacts/master/file/build/docs/html/index.html?job=doxygen).

### Memory Leaks

First **build** the (GNU/Linux) target with *debug symbols*. Build target file should be present: `./build/bin/libreweb-browser`.

Next, check for memory leaks using `valgrind` by executing:

```sh
./scripts/valgrind.sh
```

### Cross-compiling Build Dependencies

For the cross-compiling towards **Windows** (under GNU/Linux), you need at least:

* [MXE Gtkmm3 / Curl Binary packages](mxe.cc) (static build using Meson build with GCC10, see below for more info)
* CMake (Package: `cmake`)
* Ninja (Package: `ninja-build`)

For more information and the latest GTK3 Windows MXE download, please [visit my other GitLab project](https://gitlab.melroy.org/melroy/gtk-3-bundle-for-windows).

**Note:** We're currently busy trying to upgrade the [whole GTK stack](https://github.com/danger89/mxe/tree/update_gtk).

We used the following build command to get the Windows dependencies and MXE cross-compilation toolset:

```sh
make gtkmm3 curl -j 16 MXE_TARGETS='x86_64-w64-mingw32.static' MXE_PLUGIN_DIRS='plugins/gcc10'
```

Add the following line to the end of the `~/.bashrc` file:

```bash
export PATH="/opt/mxe/usr/bin:$PATH"
```

#### Cross-compile Build

Be sure you meet all the requirements above. 

Start the cross-compiling build towards Windows 64-bit (using GNU/Linux as host), execute:

```sh
./scripts/build_win_prod.sh
```

### Research

For [research document](https://gitlab.melroy.org/libreweb/research_lab/-/blob/master/research.md) plus findings including explanation (like [diagrams](https://gitlab.melroy.org/libreweb/research_lab/-/blob/master/diagrams.md)) see the:

* [LibreWeb Research Lab Project](https://gitlab.melroy.org/libreweb/research_lab/-/tree/master)
