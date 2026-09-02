# C-VEP Speller (Dareplane C/SDL3 Port)

> **Affiliation:** Radboud University, Donders Institute for Brain, Cognition and Behaviour  
> **Program:** Erasmus+ Internship Project  
> **Supervisor:** Dr. Jordy Thielen  
> **Author / Developer:** Ahmet Alper Erdoğan  

---

## 1. Project Overview & Motivation

The original Brain-Computer Interface (BCI) c-VEP Speller was built using Python and PsychoPy. While PsychoPy is excellent for standard behavioral experiments, it struggles to maintain absolute millisecond precision at extremely high monitor refresh rates (e.g., 480 Hz). This lack of precision causes micro-stutters and frame drops, which corrupts the time-locked EEG signal quality crucial for c-VEP decoding.

**The Solution:** This project is a complete rewrite of the c-VEP Speller in **pure C** using hardware-accelerated **SDL3**. By stripping away heavy frameworks and utilizing a "Suckless" low-level architecture, this Speller guarantees **0-frame-drop performance**, strict Vsync alignment, and perfect hardware-level timing. It serves as a 100% compatible, drop-in replacement module for the existing Dareplane BCI ecosystem.

---

## 2. Architectural Philosophy

This project strictly follows a **"Suckless" philosophy**, prioritizing simplicity, extreme performance, zero-latency, and minimal dependencies:
- **Unity Build:** No CMake, no Makefiles, no complex dependency trees. The entire project compiles via a single `gcc` command.
- **Zero-Blocking I/O:** All network operations (Dareplane TCP commands and Lab Streaming Layer (LSL) data streams) are strictly non-blocking. They never freeze the render loop, ensuring the high refresh rate visual stimulation remains flawless. Network packets are parsed via in-place buffer shifts (`memmove`); LSL markers use variadic inline formatting (`vsnprintf`), reducing the need for temporary heap allocations.
- **Asynchronous Subsystems:** Heavy operations like Text-to-Speech (TTS) are pushed to native Windows OS threads (`CreateThread`) ensuring they never block the primary rendering loop, even when processing long strings or repeating keys.
- **Deferred Logging:** Frame timing data is accumulated in memory during the experiment and written to disk (`log.csv`) only upon program exit, ensuring zero disk I/O during the render loop.
- **Lazy Loading:** Hardcoded sequence arrays have been eliminated. M-sequences are lazy-loaded from `.txt` files directly into a static buffer during the first rendered frame.
- **Global Dependencies:** External libraries (SDL3, LSL) are installed globally into the compiler's environment (MSYS2) rather than cluttering the project repository.
- **Fullscreen & VSync:** The window opens in exclusive fullscreen mode (`SDL_WINDOW_FULLSCREEN`) with hardware VSync enabled (`SDL_SetRenderVSync(renderer, 1)`), ensuring the render loop is locked precisely to the monitor's refresh rate.

---

## 3. Installation Guide (Windows) - Complete Step-by-Step

To ensure maximum reliability, especially in laboratory environments where computers might use "Deep Freeze" (wiping the `C:\` drive upon every reboot), we use a fully portable compilation environment. Every step below is written so that **someone with zero prior experience** can follow along.

### 🛑 WARNING: Prerequisites for 0-Frame-Drop at 480 FPS 🛑
In lab tests we achieved 0-1 frame drops at 480 Hz. To maintain this performance and prevent data loss during experiments, you **must** configure the following:

**Performance & Frame Drops:**
1. **Disable the internet connection:** Use the desktop "Internet Off" shortcut or disable Wi-Fi/Ethernet. Background updates cause micro-stutters.
2. **Close all background applications:** Spotify, Discord, Chrome, unnecessary Windows services — close everything.
3. **Multi-Monitor Setup:** If using two monitors (e.g., experimenter at 60Hz and subject at 480Hz), use **Extend these displays**, NEVER "Duplicate". Set the 480 Hz monitor as the **Primary Display**, otherwise Windows will cap VSync to 60 Hz.
4. **Disable Overlays & Game Bar:** Turn off Windows Game Bar, Game Mode, and GPU overlays (like ShadowPlay). They hijack fullscreen exclusive mode.
5. **Power Management:** Set the Windows Power Plan to **High Performance**. Disable **USB Selective Suspend** in advanced power settings to prevent the EEG amplifier from disconnecting or stuttering.
6. **Focus Assist (Do Not Disturb):** Turn on Focus Assist to suppress all system toast notifications (e.g., "Disk Space Low") which can forcibly minimize the fullscreen app.

**Data & Network (Lab Specifics):**
7. **Deep Freeze Data Loss:** Lab computers often wipe the `C:\` drive on reboot. **ALL** recorded EEG data (`.xdf` files) and performance logs (`log.csv`) **MUST** be saved to a thawed drive (e.g., `D:\`) or a USB stick. Otherwise, they will be permanently lost.
8. **Windows Defender Firewall:** LSL relies on local UDP/TCP broadcasts. Ensure the firewall allows both `main.exe` and Python/LabRecorder to communicate over private networks, or LSL markers will fail to reach the decoder.

---

### Step 1: Install Prerequisites (Git & Python)

**🛑 CRITICAL FOR LAB COMPUTERS (Deep Freeze):** Do not install to the `C:\` drive! It will be wiped when the computer restarts. Always change the installation paths to your persistent `D:\` drive.

**1.1 Git**
1. Go to **git-scm.com/download/win** in your browser.
2. Click `64-bit Git for Windows Setup` to download and run it.
3. **Important Changes during setup:**
   - **Destination Location:** Change this to `D:\Users\alper\Git` (or your equivalent D: path).
   - **PATH Environment:** Ensure "Git from the command line and also from 3rd-party software" is selected.
   - Click **"Next"** for all other settings.

**1.2 Miniconda (Python)**
1. Go to **docs.anaconda.com/miniconda/install/#windows**.
2. Download the Windows installer and run it.
3. **Important Changes during setup:**
   - **Installation Type:** Select **"Just Me"**. (Selecting "All Users" requires Administrator rights, which are blocked on lab computers).
   - **Destination Folder:** Change this to `D:\Users\alper\Miniconda3`.
   - Click **"Next"** for all other settings.

**⚠️ Deep Freeze Warning (Start Menu):**
Because the Windows Start Menu is on the `C:\` drive, your "Anaconda Prompt" shortcut will disappear after you restart the computer. To open the Anaconda Prompt after a reboot, do not look in the Start Menu. Instead, manually run this file:
`D:\Users\alper\Miniconda3\Scripts\activate.bat`

---

### Step 2: Dareplane & LSL Ecosystem Setup (Python Side)

**ℹ️ IMPORTANT NOTE:** You do **NOT** need to manually clone this repository (dp-cvep-speller). The master setup repository (dp-cvep) contains a Python script that will automatically download and configure all necessary Dareplane modules (including this C Speller, the Decoder, and the Control Room) in one go.

1. Open the Start menu (press the Windows key). *(Note: Ensure the Git installer from Step 1 has completely finished before opening the prompt, otherwise Git won't be recognized).*
2. Type **Anaconda Prompt** and click the black-icon result. A black terminal window will open.
3. Install the required Python packages (internet must be on):
   ```bash
   pip install waitress dash GitPython toml
   ```
   *(Troubleshooting: If you get an SSL or Certificate error on a university network, use `pip install waitress dash GitPython toml --trusted-host pypi.org --trusted-host files.pythonhosted.org` instead).*
4. Navigate to your Desktop and clone the main setup repository:
   ```bash
   cd Desktop
   git clone https://github.com/ahmetalperx/dp-cvep.git
   cd dp-cvep
   ```
5. Run the setup script:
   ```bash
   python setup_cvep_demo_biosemi.py
   ```
   This may take a while as it clones repositories from GitHub. If prompted with `[y/N]`, type `y` and press Enter.

**🚨 CRITICAL: Fix the Lab Recorder Path**
After setup completes, you **must** manually fix a path, otherwise recording will fail:
1. Navigate to `Desktop` → `dp-cvep` → `cvep_speller_env` → `dp-lsl-recording` → `configs`.
2. Open `lsl_conf.toml` with Notepad.
3. Find the line `lsl_recorder_exe_path`.
4. Change it to the actual path of **LabRecorder.exe** on your lab computer. For example:
   `lsl_recorder_exe_path = 'C:\Users\bsdlab\Desktop\LabRecorder\LabRecorder.exe'`
   Save and close the file.

*Note: All Python modules (decoder included) now read `.txt` code files instead of `.npz`.*

**🚨 CRITICAL: Windows Defender Firewall Popup**
When you run the Dareplane Control Room or the Speller module for the first time, a Windows Security Alert may pop up asking for network access for Python or `main.exe`. You **MUST** click **"Allow access"** (check both Private and Public networks if prompted). If you click "Cancel", the modules will not be able to communicate, and the experiment will break.


---

### Step 3: Install the C Compiler (MSYS2)

1. Go to **msys2.org** in your browser.
2. Download the installer (`msys2-x86_64-xxxxxxxx.exe`).
3. Run the installer.
4. **IMPORTANT:** When asked for the "Installation Folder", enter exactly:
   `D:\Users\alper\msys64`
   *(If you don't have a D: drive, you can use `C:\msys64` instead, but the lab computers use the D: path.)* Click "Next" to finish and leave "Run MSYS2" checked at the end.

**3.1 Install Packages**
When the MSYS2 terminal opens:
1. Type the following and press **Enter** (this updates the core system):
   ```bash
   pacman -Syu
   ```
   *If prompted, type `Y` and press Enter. If core packages are updated, the terminal will automatically close itself.*
2. **If it closed**, reopen **"MSYS2 MSYS"** from the Start menu. You **must** run the update command one more time to finish the rest of the system packages:
   ```bash
   pacman -Su
   ```
3. Now install the compiler and SDL3 libraries. **Copy-paste** the entire command below (right-click to paste) and press **Enter**:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-ttf
   ```
   *Type `Y` if prompted. Wait for it to finish.*

---

### Step 4: Global Lab Streaming Layer (LSL) Setup

**LSL (Lab Streaming Layer)** is the real-time data streaming protocol used in BCI research. It allows the Speller to send event markers (e.g., `start_trial`, `frame_dropped`) that are time-synchronized with EEG recordings. Since `liblsl` is not available in the MSYS2 package repository, we manually embed it into the compiler environment.

1. Go to the official liblsl releases page: [github.com/sccn/liblsl/releases](https://github.com/sccn/liblsl/releases)
2. Download the latest **Windows AMD64** release (e.g., `liblsl-1.16.2-Win_amd64.zip`).
3. Extract the ZIP file (right-click → Extract All). You will need the following files:
   ```
   liblsl-X.XX.X-Win_amd64/
   ├── bin/lsl.dll           ← Runtime library (NEEDED)
   ├── lib/lsl.lib           ← Linker library (NEEDED)
   ├── include/lsl_c.h       ← C header file (NEEDED)
   ├── include/lsl/...       ← Additional headers (NEEDED)
   ├── cmake/...             ← CMake config (NOT NEEDED, ignore)
   └── share/...             ← Documentation (NOT NEEDED, ignore)
   ```
4. Copy the required files into your MSYS2 UCRT64 directories:
   - `bin/lsl.dll` → `D:\Users\alper\msys64\ucrt64\bin\lsl.dll`
   - `lib/lsl.lib` → `D:\Users\alper\msys64\ucrt64\lib\liblsl.dll.a` *(Note: renaming to .dll.a matches the native MinGW standard)*
   - `include/lsl_c.h` → `D:\Users\alper\msys64\ucrt64\include\lsl_c.h`
   - Copy the entire `include/lsl/` folder → `D:\Users\alper\msys64\ucrt64\include\lsl\`

---



## 4. Running the Speller (Dareplane Integration)

The Speller is designed to be launched directly by the **Dareplane Control Room**. 

### The Python Wrapper (`main.py`)
To bridge Dareplane's Python ecosystem with our C application, we use a wrapper script (`main.py`). This script is incredibly powerful for lab environments:
- **Instance Cleanup:** Before anything else, `taskkill /f /im main.exe` kills any previously running Speller instance to prevent TCP port conflicts.
- **Automatic Environment Injection:** `main.py` dynamically injects the persistent MSYS2 binary path (`D:\Users\alper\msys64\ucrt64\bin`) into the shell environment at runtime via `set PATH=...;%PATH%`.
- **On-the-fly Compilation:** It compiles the latest C code into an `.exe` silently.
- **Execution:** It launches the Speller module.

You do not need to manually configure Windows PATH variables or run manual compilation scripts. Just launch it!

### Dareplane Configuration
You do **not** need to manually edit `example_cfg.toml`. The `setup_cvep_demo_biosemi.py` script automatically generates a custom Dareplane configuration file at `cvep_speller_env/dp-control-room/configs/cvep_speller.toml` containing all necessary module endpoints and macros. 
The setup script also automatically creates a launch script (`run_cvep_experiment.ps1`) that natively points to this custom config using the `--setup_cfg_path` flag.

*(Note: If you wish to compile it manually for testing outside Dareplane, use this command:)*
```bash
gcc main.c -O3 -l lsl -l ws2_32 -l SDL3 -l SDL3_ttf -l winmm -l avrt -o main.exe
```

---

## 5. Directory Structure & Architecture

```text
dp-cvep-speller/
├── main.c                        ← Main entry point, event loop, Unity Build wrapper
├── main.py                       ← Dareplane Python wrapper (handles runtime compilation)
├── main.exe                      ← Compiled executable (generated automatically at runtime)
├── modules/
│   ├── background.c              ← Background color rendering (solid black)
│   ├── photodiode.c              ← Optosensor test squares (top-left & top-right)
│   ├── keyboard.c                ← 32-key grid (8×4), state machine, lazy TXT sequence loader
│   ├── events.c                  ← SDL custom event system (TCP/LSL → main loop)
│   ├── server.c                  ← Winsock2 non-blocking TCP server (Dareplane commands)
│   ├── lsl.c                     ← LSL outlet (marker stream) & non-blocking inlet (decoder)
│   ├── output.c                  ← Rendered output text (green typed + grey predicted)
│   ├── tts.c                     ← Windows SAPI Text-to-Speech (async via CreateThread + PowerShell)
│   ├── fps.c                     ← Frame timing, drop detection, deferred CSV logging
│   └── dictionary.c              ← Predictive text engine using words/en.txt
├── codes/
│   ├── generate_codes.py         ← Generates m-sequences (outputs .txt files)
│   ├── mgold_61_6521.txt         ← Modulated Gold codes read by both C Speller and Python Decoder
│   ├── gold_61_6521.txt          ← Raw Gold codes (TXT format)
│   ├── mseq_61_shift.txt         ← Shifted m-sequences (TXT format)
├── fonts/
│   ├── montserrat_medium.ttf     ← Output text font (20pt)
│   ├── montserrat_extrabold.ttf  ← Keyboard letter font (64pt)
│   └── fa-solid-900.ttf          ← Font Awesome icons for special action keys (48pt)
├── words/
│   └── en.txt                    ← 14,000 word English dictionary for predictive text
└── log.csv                       ← Frame-by-frame performance log created upon exit
```

### The Main Render Loop
Running at the hardware's exact refresh rate (e.g., 60 Hz, 144 Hz, 480 Hz), the main loop is structured for zero latency:
1. **`update_fps`** — Calculate frame index from hardware clock, detect frame drops.
2. **`update_server`** — Poll non-blocking TCP socket for Dareplane commands.
3. **`update_lsl`** — Poll LSL network for Decoder predictions. **Only runs when `keyboard_state != keyboard_state_flashing`** to guarantee zero interference during visual stimulation. Decoder search is throttled to once every **5 seconds** to prevent network broadcast from blocking the render loop.
4. **`SDL_PollEvent`** — Process keyboard input, TCP events, and LSL events as custom SDL events that trigger state transitions.
5. **`update_keyboard`** — Advance the keyboard state machine based on elapsed frame time.
6. **Render** — Draw background, optosensor squares, output text, and the flashing keyboard grid.
7. **`SDL_RenderPresent`** — Block exactly until the next Vsync interrupt.

---

## 6. DEEP DIVE: Engine & Timing Architecture

The core challenge of the `dp-cvep-speller` project is rendering high-frequency visual stimulation (up to 480 Hz) with absolute millisecond precision, while preventing the Windows operating system from preempting the render loop. This is managed by a tight interplay between four core modules: `main.c`, `fps.c`, `photodiode.c`, and `background.c`.

### OS-Level Thread Optimization (`main.c`)
At ultra-high refresh rates like 480 Hz, a single frame lasts only **~2.08 milliseconds**. Any standard background task from the OS can cause a micro-stutter, resulting in a dropped frame. To combat this, `main.c` aggressively hooks into the Windows kernel:

*   **MMCSS, Priority & CPU Affinity (`AvSetMmThreadCharacteristicsW` & `SetPriorityClass`):** The engine registers the main thread with the Windows Multimedia Class Scheduler Service (MMCSS) under the `"Pro Audio"` profile, applying `AVRT_PRIORITY_CRITICAL`, and elevates the entire OS process via `SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)`. While traditionally used to prevent audio dropouts, here it protects the visual render loop by strictly prioritizing it over almost all other Windows processes.
*   **CPU Core Affinity (`SetThreadAffinityMask`):** The thread is locked to CPU Core 1 (`1ULL << 1`). This eliminates OS-level context switching across cores, preserving L1/L2 CPU cache integrity and avoiding microsecond delays.
*   **High-Resolution Timer (`timeBeginPeriod(1)`):** Forces the Windows system timer to 1-millisecond resolution, ensuring that internal thread scheduling and performance counters behave deterministically.

### Dual-Rate Timing System
The Speller operates on a **dual-rate** timing architecture, which is the foundation of its precision:
- **Refresh Rate:** The monitor's native hardware refresh rate (e.g., 60 Hz, 144 Hz, 480 Hz), auto-detected at startup via `SDL_GetCurrentDisplayMode`.
- **Presentation Rate:** The rate at which visual stimuli (m-sequence bits) change on screen. The default target is **60 Hz**, but it is automatically snapped to the nearest evenly divisible rate: `presentation_rate = refresh_rate / round(refresh_rate / 60)`.
  - Example: On a **480 Hz** monitor → `480 / round(480/60)` = `480/8` = **60 Hz** → every 8th hardware frame advances the stimulus by one bit.
  - Example: On a **144 Hz** monitor → `144 / round(144/60)` = `144/2` = **72 Hz** → every 2nd hardware frame advances the stimulus.

This ensures that stimulus transitions always land on exact hardware frame boundaries, eliminating sub-frame jitter entirely.

### Runtime Upsampling
The m-sequence codes stored in the `.txt` file are the **original** modulated codes (e.g., 63 keys × 126 bits at 60 Hz). The C Speller **automatically upsamples** these codes at runtime to match the monitor's refresh rate:

1. At startup, the `frames_per_stimulus` ratio is calculated: `N = refresh_rate / presentation_rate` (e.g., `240 / 60 = 4`).
2. Each original bit is repeated `N` times into an `upsampled_matrix_buffer` (e.g., 126 bits × 4 = 504 upsampled bits).
3. The render loop indexes directly into this upsampled buffer using the hardware `refresh_rate_frame_index`.

This means **a single `.txt` file works on any monitor** (60 Hz, 144 Hz, 240 Hz, 480 Hz) without needing to regenerate or pre-upsample the codes in Python.

### Time-Based Frame Drop Compensation (`fps.c`)
The fatal flaw of traditional render loops is incrementing frames sequentially (`frame++`). If a frame drops, the entire stimulus sequence lags behind real time, causing cumulative drift that destroys EEG synchronization. 

*   **Deterministic Frame Indexing:** The current frame is calculated purely mathematically from the high-resolution CPU clock (`SDL_GetPerformanceCounter()`).
*   **Automatic Frame Skip:** Because the index is derived from the hardware clock, if the engine misses a Vsync window (e.g., a frame takes 4ms instead of 2ms), the calculated index will automatically jump by 2. The visual sequence skips the lost frame and immediately realigns with real-world time.
*   **Dual-Rate Tracking:** `fps.c` tracks both the monitor's raw speed (`refresh_rate_frame_index`, e.g., 480 Hz) and the target stimulus speed (`presentation_rate_frame_index`, e.g., 60 Hz). Stimulus frames are calculated using strict integer division, ensuring they trigger perfectly across multiple Vsync cycles (e.g., exactly 8 refresh frames per 1 presentation frame).
*   **LSL Event Integration:** Whenever `fps.c` detects that `current_index > previous_index + 1`, it instantly pushes a `frame_dropped` marker via LSL, allowing the downstream Dareplane rCCA EEG decoder to mathematically compensate for the irregularity in that specific epoch.

### Hardware Validation via Optosensors (`photodiode.c`)
Software logs can be misleading; true BCI precision must be validated physically using an optosensor/photodiode attached directly to the monitor glass. `photodiode.c` implements a dual-validation system by rendering two separate tracking squares:
*   **Refresh Rate Photodiode (Top-Left, 0x0):** Toggles exactly every raw Vsync frame (`fps.refresh_rate_frame_index % 2`). At 480 Hz, it creates a 240 Hz flicker. This allows an oscilloscope to confirm if the physical monitor is truly keeping up with the GPU.
*   **Presentation Rate Photodiode (Top-Right, 1856x0):** Toggles exactly every stimulus presentation frame (`fps.presentation_rate_frame_index % 2`). At a 60 Hz presentation rate, it creates a 30 Hz flicker. This validates the software's internal clock and the actual on-screen speed of the c-VEP sequence.

---

## 7. DEEP DIVE: Networking & Event Architecture

The `dp-cvep-speller` seamlessly integrates into the modular Dareplane ecosystem, which coordinates BCI paradigms through TCP for control commands and Lab Streaming Layer (LSL) for high-frequency synchronization.

### TCP Server & Command Parsing (`server.c`)
Dareplane's `dp-control-room` orchestrates the experiment by sending string-based TCP commands (e.g., `TRAINING`, `ONLINE`, `GET_PCOMMS`). The `server.c` module implements a custom Winsock2 TCP server:
*   **Non-Blocking Winsock2:** During socket initialization, `ioctlsocket(..., FIONBIO, ...)` sets both the listening socket and the accepted client socket to non-blocking mode. The `update_server()` function is polled every frame. If no data is present, `recv()` returns `WSAEWOULDBLOCK` and the render loop proceeds instantly.
*   **Delimiter-Based Parsing:** Network packets are appended into a continuous `server_buffer` (1024 bytes). The parser iterates through the buffer looking for delimiters like `\n`, `\r`, `;`, or `|`. Once found, it injects a null-terminator (`\0`), isolating the command string, and maps it to a custom SDL Event via functions like `push_event_training()`.
*   **Buffer Shifting with `memmove`:** Because TCP is a stream protocol, a single `recv()` might read half of a command (e.g., `"TRAI"`). The `start_idx` keeps track of fully parsed commands. Any leftover bytes at the end of the buffer are safely shifted to the beginning using `memmove(server->server_buffer, server->server_buffer + start_idx, remaining)`. This guarantees that partial commands are preserved and successfully completed on the next loop iteration, without overflowing the buffer.
*   **Connection Handshake:** When a client connects, the server immediately sends `"connected to dp-cvep-speller\n"` as acknowledgment. The `GET_PCOMMS` command responds with `"TRAINING|ONLINE|STOP|CLOSE|GET_PCOMMS|UP"`. The `UP` command responds with `"1"` (health check).

### LSL Emission & Throttle (`lsl.c`)
*   **Variadic Marker Function:** To keep the codebase clean, LSL emission is wrapped in a variadic function: `void send_lsl_marker(lsl_t *lsl, const char *format, ...)`. Similar to `printf`, it uses `va_list` and `vsnprintf` to dynamically construct strings inside a 256-byte `lsl_marker_buffer`, then pushes via `lsl_push_sample_str`.
*   **Time-Based Throttle for Decoder Resolution:** Finding an LSL stream on the network via `lsl_resolve_byprop()` is an inherently expensive operation. If called on every frame while the decoder is offline, it floods the network and causes massive frame drops. `update_lsl()` implements an `SDL_GetTicks()` throttle. It only attempts to resolve the stream once every **5 seconds** (the code checks `current_time - lsl->lsl_last_resolve_time > 5000`).
*   **Flashing Guard:** In `main.c`, the call to `update_lsl()` is wrapped inside `if (keyboard.keyboard_state != keyboard_state_flashing)`. This means **LSL decoder polling is completely disabled during visual stimulation**, providing an absolute guarantee that no network operation can interfere with frame timing during the critical EEG recording phase.
*   **Non-Blocking Pulling:** Once the inlet is established, predictions (a single `char` index) are pulled in a `while` loop using `lsl_pull_sample_c(...)` with timeout `0.0`. All samples are drained, and only the `last_valid` index is used, ensuring any queue backlog is flushed instantly.

### Unifying State Transitions via SDL Events (`events.c`)
Directly modifying global states from network callbacks can introduce race conditions and spaghetti code. The architecture circumvents this by filtering everything through the native SDL Event Loop.
*   **Custom SDL Events:** `events.c` registers custom IDs using `SDL_RegisterEvents(1)` for 5 paradigm events: `IDLE`, `TRAINING`, `ONLINE`, `FEEDBACK`, and `CLOSE`.
*   **Payload Injection:** The `push_custom_event()` function populates an `SDL_Event` struct. For commands carrying payloads—such as the target index received from LSL—the integer is safely cast and stored in `event.user.data1` via `(void*)(intptr_t)`.
*   **Centralized Dispatch:** Using `SDL_PushEvent(&event)`, TCP commands and LSL predictions are queued identically to native keyboard strokes or window resizes. The main application loop simply polls `SDL_PollEvent(&event)` and delegates state transitions cleanly.

### Dareplane Orchestrator Wrapper (`main.py`)
Though the application is pure C, it must be launched by Dareplane's Python-based `dp-control-room`. `main.py` serves as this bridge:
*   **Instance Cleanup:** `taskkill /f /im main.exe` kills any stale instance before compilation.
*   **JIT Compilation & Execution:** When Dareplane starts the module, `main.py` injects the MSYS2 path, compiles the C source code with all necessary libraries (`-l lsl -l ws2_32 -l SDL3 -l SDL3_ttf -l winmm -l avrt`), and launches `main.exe` asynchronously via the Windows `start` command. This ensures the executable is always up-to-date.

---

## 8. DEEP DIVE: UI & State Machine

### Keyboard Grid (`keyboard.c`)
The on-screen keyboard consists of **32 keys** arranged in an **8×4 grid**:
- **Letters:** A-Z (26 keys) fill 7 columns across 4 rows.
- **Action Keys:** The 8th (rightmost) column holds 4 action keys, plus 2 more in the bottom row:
  - **Space** (`-`) — Inserts a space character
  - **Backspace** (`<`) — Deletes the last character
  - **Accept Prediction** (`>`) — Accepts the grey auto-complete suggestion
  - **Speak** (`*`) — Reads the current output text aloud via TTS
  - **Caps Lock** (`^`) — Toggles uppercase/lowercase letter display. Destroys and recreates TTF text objects for all letter keys.
  - **Clear All** (`#`) — Clears the entire output text buffer
- **Icon Rendering:** All 6 special action keys render **Font Awesome icons** (`fa-solid-900.ttf`, 48pt) using raw UTF-8 byte sequences (e.g., `"\xEF\x80\xA8"` for the volume icon) instead of text characters.
- **Key size:** 128×128 pixels with a 4px border.
- **Sequence file:** `codes/mgold_61_6521.txt` (Contains 63 sequences × 126 bits). The Speller uses `index % keyboard_sequence_num_keys` for sequence wrapping across all 32 keys.

### The 4 Core States of the Paradigm
The core of the speller is managed by the `keyboard_state_t` state machine. This tightly controls the BCI visual paradigm, dictating exactly what is shown on screen and when LSL markers are dispatched for EEG synchronization.

- **Idle (Inter-Trial Interval - ITI):** The system is in a rest phase. No flashing occurs. If the native Windows TTS is still speaking (`is_tts_speaking`), the state machine explicitly pauses the transition (`return`), ensuring auditory feedback does not overlap with visual stimuli.
- **Cue (Training Mode Only):** The target key is highlighted with a yellow border. This instructs the user on which key to focus on during a calibration phase. The target key index is chosen randomly via `rand() % keyboard_key_count`.
- **Flashing (Stimulation):** All keys on the grid begin flickering black/white according to their assigned stimulus sequences from the upsampled matrix buffer. A `start_trial` marker is fired. In Online mode, flashing continues until a Feedback event arrives from the decoder. A fallback safety timeout of `state_flashing_duration * 1.5f` (6.3s) prevents infinite flashing if the decoder disconnects.
- **Feedback (Closed-Loop Decoding):** Triggered when the `dp-cvep-decoder` successfully predicts the target key (or a manual mock key is pressed). The selected key is highlighted with a blue border. The corresponding action (letter append, space, backspace, speak, accept, caps lock, clear) is executed. TTS is triggered **only** when the Speak (*) key is selected.

### Mathematical Upsampling of the 60Hz Sequence
To bridge the gap between 480 Hz monitors and 60 Hz fundamental sequences without losing precision, `keyboard.c` mathematically "upsamples" the sequences when `upsample_sequences()` is called:
*   It calculates `frames_per_stimulus = monitor_refresh_rate / sequence_rate` (e.g., 480 Hz / 60 Hz = 8).
*   A flat 1D array (`upsampled_matrix_buffer`) is allocated via `malloc(num_keys * upsampled_num_bits)`.
*   It iterates through the original sequence bits and repeats each bit exactly `frames_per_stimulus` times.
*   During `render_keyboard()`, the exact pixel color for any key at any moment is derived via: `upsampled_matrix_buffer[(index % num_keys) * upsampled_num_bits + (frame_index % upsampled_num_bits)]`.

### Lazy Loading & Zero-Overhead `.txt` Parsing
The stimulus sequences are pre-calculated by `generate_codes.py` using m-sequences and Gold codes, then modulated and output as `.txt` files containing `0`s and `1`s. `keyboard.c` uses a highly optimized, lazy-loaded two-pass parser:
1. **Lazy Evaluation:** `load_sequence_from_txt` is only invoked inside `render_keyboard` upon the first frame render if `keyboard_sequence_num_keys == 0`.
2. **First Pass (Counting):** The function streams the file using `fgetc`, counting `0`s, `1`s, and `\n` characters to determine the exact matrix dimensions (`rows` and `cols`) without storing anything in memory.
3. **Single Allocation:** It allocates the exact required memory block for the entire matrix once, as a contiguous 1D array (`malloc(rows * cols)`).
4. **Second Pass (Populating):** The file pointer is rewound (`rewind(file)`), and the array is populated directly character-by-character with overflow protection (`if (total_elements >= rows * cols) break`).

---

## 9. DEEP DIVE: Prediction & Output

The Dareplane c-VEP speller implements an ultra-fast, lightweight predictive text engine using pure C.

### Static Loading & Memory Management (`dictionary.c`)
When the application starts, `initialize_dictionary(filepath)` is called to load `words/en.txt` into memory. 
*   **Fixed Bounds:** It allocates an array of string pointers limited by `MAX_DICTIONARY_WORDS` (20,000 words) to guarantee deterministic memory usage.
*   **Safe File Reading:** It reads lines using `fgets()` with a buffer of `MAX_WORD_LENGTH` (256 bytes). If a word in the file exceeds this limit, the function safely flushes the rest of the line until it encounters a newline or EOF, preventing buffer overflows.
*   **Pre-Processing (Optimization):** As words are loaded, any trailing newline/carriage return characters (`\n`, `\r`) are stripped. Every string is converted to uppercase using `toupper()` *before* it is copied to the heap via `strdup()`. Normalizing the dictionary in memory eliminates the need to perform case conversions on dictionary words during the high-frequency rendering loop.
*   **Cleanup:** `cleanup_dictionary()` frees all heap-allocated word strings on program exit.

### Prediction Logic & Prefix Matching
The core prediction function is `get_prediction(const char *current_text)`:
1. **Word Extraction:** It scans the user's current typed text backwards to find the most recent word delimiter (a space `' '` or hyphen `'-'`).
2. **Dynamic Casing for Matching:** It safely copies this last word into a local buffer (`upper_last_word`) using `strncpy()` and immediately converts the user's input to uppercase.
3. **Linear Search:** It loops through the `dictionary_words` array and checks for prefix matches using `strncmp(dictionary_words[index], upper_last_word, len)`. 
4. **Suffix Return:** Because the dictionary and the query are both uppercase, the `strncmp` acts as a highly optimized case-insensitive match. Once a match is found, the engine simply uses pointer arithmetic (`return dictionary_words[index] + len`) to return *only* the remaining letters (the suffix) of the predicted word.

### Dual-Color Rendering: Green vs. Grey (`output.c`)
The Speller uses a two-tone UI. The user's decoded text is rendered in solid **Green** `{0, 255, 0, 255}`, while the predicted auto-complete suffix is rendered in **Grey** `{128, 128, 128, 255}`.
*   **Lazy Re-rendering:** Rather than rendering text from scratch every frame, `render_output()` only updates the `SDL3_ttf` text objects if the `output_text_changed` flag is `1`.
*   **Dynamic Case Adjustment:** The engine looks at the last character the user typed. If the user typed a lowercase letter (`'a'` to `'z'`), the module iterates over the predicted suffix (which is inherently uppercase from `dictionary.c`) and converts it to lowercase using `tolower()`. This ensures the grey predicted text visually perfectly matches the case of the green typed text.
*   **Side-by-Side Centering:** The module calculates the exact pixel width of both the green `output_ttf_text` and the grey `predicted_ttf_text` using `TTF_GetTextSize()`. It adds both widths together (`total_width = output_width + pred_width`) to find the exact starting `X` coordinate needed to mathematically center the combined string on the screen. It then issues two `TTF_DrawRendererText()` draw calls.
*   **Accept Prediction:** The `accept_prediction()` function concatenates the predicted suffix to the output text and automatically appends a trailing space `' '`, provided it fits within the 127-character buffer limit.
*   **Welcome Message:** The output text initializes to `"WELCOME TO CVEP SPELLER"` at startup.

---

## 10. DEEP DIVE: Audio & TTS

The `tts.c` module handles Text-to-Speech (TTS) feedback. In a high-frequency BCI environment, maintaining exact frame timing is paramount. Even a single missed frame ruins the synchronization.

### Why We Avoid Conventional Approaches
1. **Cloud APIs (e.g., Google/ElevenLabs):** Network latency introduces unpredictable delays, making cloud TTS useless for real-time feedback.
2. **Heavyweight Audio Libraries:** Using `SDL_mixer` or `OpenAL` violates the "suckless", zero-dependency philosophy.
3. **Direct C COM Integration (Windows SAPI):** Initially, the OS-native Windows SAPI was invoked directly via C COM objects. However, when users typed quickly or held down a key (triggering key repeat events), the COM object initialization and synchronization locked the main event loop, resulting in critical frame drops.

### The Solution: Asynchronous Threading + Hidden PowerShell Processes
To guarantee **0-frame-drops** under all circumstances, `tts.c` shifts the entire TTS workload to the OS process scheduler.

#### 1. Offloading to `CreateThread`
When `text_to_speech()` is called, it allocates a `tts_thread_args_t` struct on the heap containing the formatted PowerShell command string, and immediately spawns an isolated Windows thread via `CreateThread`. The thread handle is immediately closed (`CloseHandle(CreateThread(...))`) since we don't need to join it. The main thread instantly returns to rendering.

#### 2. Spawning a Hidden PowerShell Executable
Inside the worker thread (`tts_thread_func`), the code uses `CreateProcessA` with the `CREATE_NO_WINDOW` flag to launch a hidden PowerShell instance leveraging the native `.NET System.Speech` synthesizer. Settings like Voice Gender (1=Female, 2=Male) and Age (3=Adult) are passed as direct integer parameters matching the Microsoft SAPI API enum values.

#### 3. Aggressive Overlap Management (`TerminateProcess`)
When the Speak key is triggered rapidly, multiple TTS requests can overlap. If a previous TTS process is `STILL_ACTIVE` (checked via `GetExitCodeProcess`), the program calls `TerminateProcess` to instantly kill the old PowerShell instance before launching the new one. This ensures that a new speech request immediately cuts off the previous audio without any blocking or memory leaks.

#### 4. Preventing Audio Clipping
Because the PowerShell process can be killed immediately when the script finishes, the native audio buffer sometimes cuts out early. To prevent clipping, the command appends `Start-Sleep -m 200`. This artificial 200ms delay ensures the audio buffer is fully flushed to the speakers before the process terminates naturally.

#### 5. TTS-Aware State Machine
In `main.c`, the `is_tts_speaking` flag is computed every frame by checking `GetExitCodeProcess(tts.tts_process_handle, &exit_code) && exit_code == STILL_ACTIVE`. This flag is passed to `update_keyboard()`, which uses it to **pause the ITI→next trial transition** until TTS has finished speaking. This prevents the next visual stimulus from starting while audio feedback is still playing, ensuring a clean cognitive separation for the user.

---

## 11. Communication Protocol Reference

### LSL Marker Stream (Outlet)
| Property | Value |
|----------|-------|
| **Stream Name** | `cvep-speller-stream` |
| **Stream Type** | `markers` |
| **Channel Count** | 1 |
| **Channel Format** | `cft_string` |
| **Sampling Rate** | `LSL_IRREGULAR_RATE` |

| Marker | Description |
|--------|-------------|
| `start_cue;label=X;key=Y` | Sent at the beginning of a cue phase. Contains target key index (`X`) and character (`Y`). For special keys, `Y` is a name like `space`, `backspace`, `speak`, `accept`, `capslock`, `clearall`. |
| `stop_cue` | Sent when the cue phase ends. |
| `start_trial` | Sent when the stimulation (flashing) starts. Primary trigger for the decoder to epoch EEG data. |
| `stop_trial` | Sent when the stimulation ends. |
| `start_feedback;label=X;key=Y`| Sent when visual feedback is presented (based on decoder prediction). Same key naming convention as cue. |
| `stop_feedback` | Sent when the feedback phase ends. |
| `start_iti` | Sent at the beginning of the Inter-Trial Interval. |
| `stop_iti` | Sent at the end of the Inter-Trial Interval. |
| `frame_dropped` | Sent immediately if the renderer misses a Vsync deadline. |

### LSL Decoder Stream (Inlet)
| Property | Value |
|----------|-------|
| **Searched Stream Name** | `cvep-decoder-stream` |
| **Channel Format** | `cft_int8` (single byte, key index) |
| **Pull Timeout** | `0.0` (non-blocking) |
| **Resolve Interval** | Every 5000 ms (throttled to prevent frame drops) |
| **Flashing Guard** | `update_lsl()` is **not called** during `keyboard_state_flashing` |

### TCP Commands (Dareplane Server: `127.0.0.1:8084`)
| Incoming Command | Triggered Action |
|------------------|------------------|
| `UP` | Health check — responds with `"1"` |
| `GET_PCOMMS` | Returns supported commands: `"TRAINING\|ONLINE\|STOP\|CLOSE\|GET_PCOMMS\|UP"` |
| `TRAINING` | Switches to Training mode (Cue → Flashing → ITI loop, 10 trials) |
| `ONLINE` | Switches to Online mode (continuous Flashing → Feedback → ITI loop) |
| `STOP` | Switches to Idle, halts current stimulation |
| `CLOSE` | Terminates the program safely |

---

## 12. Performance Logging (`log.csv`)

Upon program exit, all frame timing data accumulated during the session is written to `log.csv`. This file contains one row per rendered frame with the following columns:

| Column | Description |
|--------|-------------|
| `timestamp_ms` | Timestamp in milliseconds since program start |
| `frame_index` | Hardware refresh rate frame counter |
| `sequence_index` | Presentation rate (stimulus) frame counter |
| `refresh_rate` | Monitor refresh rate (Hz) |
| `presentation_rate` | Stimulus presentation rate (Hz) |
| `current_fps` | Measured FPS for this frame |
| `difference_fps` | Deviation from target FPS |
| `target_ms` | Expected frame duration (ms) |
| `current_ms` | Actual frame duration (ms) |
| `difference_ms` | Deviation from expected duration (ms) |
| `is_dropped` | `DROP` if the frame was missed, empty otherwise |

---

## 13. Controls & Keyboard Shortcuts
When testing manually or running offline experiments, use these physical keyboard shortcuts:
- `ESC`: Close program safely (and dump performance logs)
- `1 / NumPad 1`: Switch to Idle Mode (STOP)
- `2 / NumPad 2`: Switch to Training Mode
- `3 / NumPad 3`: Switch to Online Mode
- `4 / NumPad 4`: Toggle Left optosensor (refresh rate photodiode)
- `5 / NumPad 5`: Toggle output text visibility
- `6 / NumPad 6`: Toggle Right optosensor (presentation rate photodiode)
- `7 / NumPad 7`: Simulate Clear All (`#`) key selection
- `8 / NumPad 8 / Tab`: Simulate Accept Prediction (`>`) key selection
- `9 / NumPad 9`: Simulate Speak (`*`) key selection
- `0 / NumPad 0 / CapsLock`: Simulate Caps Lock (`^`) key selection
- `A-Z`: Simulate the corresponding letter key selection (mock decoder)
- `Space`: Simulate Space (`-`) key selection
- `Backspace`: Simulate Backspace (`<`) key selection

---

## 14. Configuration Reference (Hardcoded Parameters)

This project does **not** use any external configuration file (e.g., `.json`, `.toml`, `.ini`). Following the Suckless philosophy, **all parameters are hardcoded directly in the C source files** as struct initializers. To change any parameter, you must edit the corresponding `.c` file and **recompile** the project.

### `modules/fps.c` — Display & Timing Parameters
| Parameter | Default | Description |
|-----------|---------|-------------|
| `presentation_rate` | `60.0` Hz | Target stimulus frequency. Code automatically rounds it to nearest integer divisor of the hardware refresh rate. |
| `log_filename` | `"log.csv"` | Output filename for the frame timing log |
| `log_capacity` | `1000000` | Maximum number of frame entries stored in memory |

### `modules/keyboard.c` — Experiment Parameters
| Parameter | Default | Description |
|-----------|---------|-------------|
| `state_idle_duration` | `0.3` s | Inter-Trial Interval (pause between trials) |
| `state_cue_duration` | `0.7` s | Duration of the cue highlight (Training mode only) |
| `state_flashing_duration` | `4.2` s | Duration of the visual stimulation phase |
| `state_feedback_duration` | `0.7` s | Duration of the decoded feedback highlight |
| `cue_count` | `10` | Number of trials per Training session |
| `keyboard_key_count` | `32` | Total number of keys on the on-screen keyboard (A-Z + 6 action keys) |
| `keyboard_sequence_file_path` | `"codes/mgold_61_6521.txt"` | Original modulated m-sequence file. Automatically upsampled at runtime. |
| `is_lowercase` | `0` (uppercase) | Whether to display and type lowercase letters (toggled by Caps Lock key) |

### `modules/keyboard.c` — Visual Style (Color Scheme)
| State | Border Color | Background Color | Text Color |
|-------|-------------|-----------------|------------|
| Idle | Gray `(85,85,85)` | Dark `(16,16,16)` | Gray `(85,85,85)` |
| Cue | Yellow `(255,255,0)` | Dark `(16,16,16)` | Gray `(85,85,85)` |
| Flashing (bit=1) | White `(255,255,255)` | White `(255,255,255)` | Black `(0,0,0)` |
| Feedback | Blue `(0,0,255)` | Dark `(16,16,16)` | Gray `(85,85,85)` |

### `modules/lsl.c` — LSL Stream Configuration
| Parameter | Default | Description |
|-----------|---------|-------------|
| `lsl_marker_stream_name` | `"cvep-speller-stream"` | Name of the outgoing LSL marker stream |
| `lsl_decoder_stream_name` | `"cvep-decoder-stream"` | Name of the incoming decoder LSL stream to search for |
| Resolve throttle | `5000` ms | How often to search for the decoder stream when disconnected |
| `lsl_marker_buffer` | 256 bytes | Static buffer used by the variadic `send_lsl_marker()` function |

### `modules/server.c` — TCP Server Configuration
| Parameter | Default | Description |
|-----------|---------|-------------|
| `server_ip` | `"127.0.0.1"` | IP address the TCP server binds to |
| `server_port` | `8084` | TCP port the server listens on |
| `server_buffer` | 1024 bytes | Accumulation buffer for TCP stream parsing with `memmove` |

### `modules/photodiode.c` — Optosensor Squares
| Parameter | Default | Description |
|-----------|---------|-------------|
| Refresh rate square position | Top-Left `(0, 0)` | Position of the refresh rate photodiode |
| Presentation rate square position | Top-Right `(1856, 0)` | Position of the presentation rate photodiode |
| Square size | `64×64` px | Size of both optosensor squares |
| `photodiode_is_visible` | `1` (visible) | Whether the square is shown at startup |

### `modules/output.c` — Output Text Display
| Parameter | Default | Description |
|-----------|---------|-------------|
| `output_text` | `"WELCOME TO CVEP SPELLER"` | Initial welcome message displayed at startup |
| `output_text_y` | `106.0` px | Vertical position of the output text |
| `output_text_color` | Green `(0,255,0)` | Color of the decoded output text |
| `predicted_text_color` | Gray `(128,128,128)` | Color of the predictive autocomplete text |
| `output_is_visible` | `1` (visible) | Whether the output text is shown at startup |
| Max output length | 127 characters | Hard buffer limit preventing overflow |

### `modules/tts.c` — Text-to-Speech
| Parameter | Default | Description |
|-----------|---------|-------------|
| `text_to_speech_volume` | `100` | Speech volume (0-100) |
| `text_to_speech_speed` | `0` | Speech rate (-10 slowest to 10 fastest, 0 = normal) |
| `text_to_speech_gender` | `1` | Windows SAPI VoiceGender enum (1 = Female, 2 = Male) |
| `text_to_speech_age` | `3` | Windows SAPI VoiceAge enum (0 = NotSet, 1 = Child, 2 = Teen, 3 = Adult, 4 = Senior) |

### `modules/background.c` — Background
| Parameter | Default | Description |
|-----------|---------|-------------|
| `background_color` | Black `(0,0,0)` | Screen background color |

### `modules/dictionary.c` — Predictive Dictionary
| Parameter | Default | Description |
|-----------|---------|-------------|
| Dictionary File | `"words/en.txt"` | 14,000 word English dictionary used for predictive typing logic |
| `MAX_DICTIONARY_WORDS` | `20000` | Max number of words allocated in dictionary |
| `MAX_WORD_LENGTH` | `256` | Max character length per word line to prevent buffer overflows |
