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

### Windows Performance Tuning
The following OS-level optimizations are applied at startup in `main.c` to guarantee the lowest possible latency:
- **Process Priority:** `SetPriorityClass(HIGH_PRIORITY_CLASS)` elevates the process above normal system tasks.
- **CPU Affinity:** `SetThreadAffinityMask` pins the render thread to a single CPU core to prevent context-switching jitter.
- **MMCSS (Multimedia Class Scheduler):** `AvSetMmThreadCharacteristicsW("Pro Audio")` with `AVRT_PRIORITY_CRITICAL` tells Windows to treat this thread as a real-time audio/video workload, giving it the highest scheduling priority.
- **Timer Resolution:** `timeBeginPeriod(1)` sets the system timer granularity to 1ms for precise frame timing.

---

## 3. Installation Guide (Windows)

To ensure maximum reliability, especially in laboratory environments where computers might use "Deep Freeze" (wiping the `C:\` drive upon every reboot), we use a fully portable compilation environment.

### Step 0: Python Prerequisites
Although the core Speller is written in C, it exists within the Python-based Dareplane ecosystem.
1. Install **Python 3.10+** (ensure "Add Python to PATH" is checked during Windows installation).
2. The wrapper `main.py` uses only standard libraries (`subprocess`, `os`), so no extra `pip install` is needed just to run the Speller.
3. *(Optional)* If you wish to generate new stimulation sequences using `codes/generate_codes.py`, you will need to install its dependencies:
   ```bash
   pip install numpy pyntbci
   ```

### Step 1: Portable MSYS2 & GCC
**MSYS2** is a Unix-like development environment for Windows. It provides a package manager (`pacman`) and a terminal to install compilers and libraries.

1. Download **MSYS2** from [msys2.org](https://www.msys2.org/). 
2. Install it directly to a persistent, non-wiped drive (e.g., `D:\Users\alper\msys64`). This ensures your compiler survives reboots.
3. Open the **MSYS2 UCRT64** terminal (not the default MSYS terminal!) and install the **GCC C compiler**. GCC is needed to compile `main.c` into `main.exe`:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```

### Step 2: Install Graphics Libraries (SDL3)
**SDL3** is the graphics library that handles window creation, GPU-accelerated rendering, and VSync synchronization. **SDL3_ttf** is its font extension for rendering text on screen (keyboard letters and output text).

In the same UCRT64 terminal:
```bash
pacman -S mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-ttf
```

> ⚠️ **Package names are lowercase and use dashes** (not `SDL3` or `SDL3_ttf`). Pacman is case-sensitive.

### Step 3: Global Lab Streaming Layer (LSL) Setup
**LSL (Lab Streaming Layer)** is the real-time data streaming protocol used in BCI research. It allows the Speller to send event markers (e.g., `start_trial`, `frame_dropped`) that are time-synchronized with EEG recordings. Since `liblsl` is not available in the MSYS2 package repository, we manually embed it into the compiler environment.

1. Go to the official liblsl releases page: [github.com/sccn/liblsl/releases](https://github.com/sccn/liblsl/releases)
2. Download the latest **Windows AMD64** release (e.g., `liblsl-1.16.2-Win_amd64.zip`)
3. Extract the ZIP file. Inside you will find:
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
   - Copy `bin/lsl.dll` → `D:\Users\alper\msys64\ucrt64\bin\lsl.dll`
   - Copy `lib/lsl.lib` → `D:\Users\alper\msys64\ucrt64\lib\lsl.lib`
   - Copy `include/lsl_c.h` → `D:\Users\alper\msys64\ucrt64\include\lsl_c.h`
   - Copy the entire `include/lsl/` folder → `D:\Users\alper\msys64\ucrt64\include\lsl\`

*Your environment is now completely self-sufficient and portable!*

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
Add the module to your Dareplane `example_cfg.toml`:
```toml
[python.modules.dp-cvep-speller]

custom_entry_point = 'main'

ip = '127.0.0.1'

port = 8084

retry_after_s = 3.0

max_connect_retries = 10
```

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
│   ├── keyboard.c                ← 28-key grid (7×4), state machine, lazy TXT sequence loader
│   ├── events.c                  ← SDL custom event system (TCP/LSL → main loop)
│   ├── server.c                  ← Winsock2 non-blocking TCP server (Dareplane commands)
│   ├── lsl.c                     ← LSL outlet (marker stream) & non-blocking inlet (decoder)
│   ├── output.c                  ← Rendered output text (green, centered, Montserrat Medium 20pt)
│   ├── tts.c                     ← Windows SAPI Text-to-Speech (async via PowerShell)
│   ├── fps.c                     ← Frame timing, drop detection, deferred CSV logging
│   └── dictionary.c              ← Predictive text engine using words/en.txt
├── codes/
│   ├── generate_codes.py         ← Generates m-sequences (outputs both .npz and .txt)
│   ├── mgold_61_6521.txt         ← Modulated Gold codes read by C Speller (used in experiment)
│   ├── mgold_61_6521.npz         ← Modulated Gold codes read by Python Decoder
│   ├── gold_61_6521.txt          ← Raw Gold codes (TXT format)
│   ├── gold_61_6521.npz          ← Raw Gold codes (NPZ format)
│   ├── mseq_61_shift.txt         ← Shifted m-sequences (TXT format)
│   └── mseq_61_shift.npz         ← Shifted m-sequences (NPZ format)
├── fonts/
│   ├── montserrat_medium.ttf     ← Output text font (20pt)
│   └── montserrat_extrabold.ttf  ← Keyboard letter font (64pt)
├── words/
│   └── en.txt                    ← 14,000 word English dictionary for predictive text
└── log.csv                       ← Frame-by-frame performance log created upon exit
```

### The Main Render Loop
Running at the hardware's exact refresh rate (e.g., 60 Hz, 144 Hz, 480 Hz), the main loop is structured for zero latency:
1. **`update_fps`** — Calculate frame index from hardware clock, detect frame drops.
2. **`update_server`** — Poll non-blocking TCP socket for Dareplane commands.
3. **`update_lsl`** — Poll LSL network for Decoder predictions. Decoder search is throttled to once every **2 seconds** to prevent network broadcast from blocking the render loop.
4. **`SDL_PollEvent`** — Process keyboard input, TCP events, and LSL events as custom SDL events that trigger state transitions.
5. **`update_keyboard`** — Advance the keyboard state machine based on elapsed frame time.
6. **Render** — Draw background, optosensor squares, output text, and the flashing keyboard grid.
7. **`SDL_RenderPresent`** — Block exactly until the next Vsync interrupt.

---

## 6. Keyboard & Visual Layout

### Keyboard Grid
The on-screen keyboard consists of **28 keys** arranged in a **7×4 grid**:
- **Keys:** A-Z, Space (`-`), Backspace (`<`)
- **Key size:** 128×128 pixels with a 4px border
- **Sequence file:** `codes/mgold_61_6521.txt` (Contains 63 sequences × 126 bits, Speller uses the first 28)

### Photodiode Squares
Two 64×64px optosensor squares are rendered for external timing verification with a photodiode sensor:
- **Top-Left (refresh rate):** Toggles black/white every hardware frame. Used to verify the monitor's true refresh rate.
- **Top-Right (presentation rate):** Toggles black/white every stimulus frame. Used to verify the stimulus presentation rate matches the target.

### Output Text
Decoded letters are displayed as green centered text above the keyboard (Montserrat Medium, 20pt). In Online mode, each decoded letter is appended, with Space and Backspace support. Furthermore, `dictionary.c` provides grey predictive text auto-completions based on `words/en.txt` with case-insensitive checking (`tolower`).

---

## 7. Keyboard State Machine

The experiment progresses through a series of timed phases (states). Each state transition emits LSL markers for precise time-locked EEG analysis:

```text
Training Mode (10 trials per session):
  CUE (0.7s) → FLASHING (4.2s) → ITI (0.3s) → CUE → ... (repeats 10 times)

Online Mode (continuous until STOP):
  FLASHING → [wait for Decoder prediction] → FEEDBACK (0.7s) → ITI (0.3s) → FLASHING → ...
```

| State | Duration | Description |
|-------|----------|-------------|
| `keyboard_state_idle` (ITI) | 0.3s | Inter-Trial Interval. Brief pause between trials. |
| `keyboard_state_cue` | 0.7s | Target key is highlighted in yellow (border). Training mode only. |
| `keyboard_state_flashing` | 4.2s | All 28 keys modulated by m-sequence (white flash = bit 1, dark = bit 0). EEG is recorded during this phase. |
| `keyboard_state_feedback` | 0.7s | Decoded key is highlighted in blue (border). TTS reads aloud the predicted letter. |

---

## 8. Communication Protocol

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
| `start_cue;label=X;key=Y` | Sent at the beginning of a cue phase. Contains target key index (`X`) and character (`Y`). |
| `stop_cue` | Sent when the cue phase ends. |
| `start_trial` | Sent when the stimulation (flashing) starts. Primary trigger for the decoder to epoch EEG data. |
| `stop_trial` | Sent when the stimulation ends. |
| `start_feedback;label=X;key=Y`| Sent when visual feedback is presented (based on decoder prediction). |
| `stop_feedback` | Sent when the feedback phase ends. |
| `start_iti` | Sent at the beginning of the Inter-Trial Interval. |
| `stop_iti` | Sent at the end of the Inter-Trial Interval. |
| `frame_dropped` | Sent immediately if the renderer misses a Vsync deadline. |

### LSL Decoder Stream (Inlet)
| Property | Value |
|----------|-------|
| **Searched Stream Name** | `cvep-decoder-stream` |
| **Channel Format** | `cft_char8` (single byte, key index) |
| **Pull Timeout** | `0.0` (non-blocking) |
| **Resolve Interval** | Every 2000 ms (throttled to prevent frame drops) |

### TCP Commands (Dareplane Server: `127.0.0.1:8084`)
| Incoming Command | Triggered Action |
|------------------|------------------|
| `UP` | Health check (is module alive?) |
| `GET_PCOMMS` | Returns supported command list |
| `TRAINING` | Switches to Training mode |
| `ONLINE` | Switches to Online mode |
| `STOP` | Switches to Idle, halts current stimulation |
| `CLOSE` | Terminates the program safely |

---

## 9. Performance Logging (`log.csv`)

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

## 10. Controls & Keyboard Shortcuts
When testing manually or running offline experiments, use these physical keyboard shortcuts:
- `ESC`: Close program safely (and dump performance logs)
- `1 / NumPad 1`: Switch to Idle Mode
- `2 / NumPad 2`: Switch to Training Mode
- `3 / NumPad 3`: Switch to Online Mode
- `4 / NumPad 4`: Toggle Left optosensor (refresh rate photodiode)
- `5 / NumPad 5`: Toggle output text visibility
- `6 / NumPad 6`: Toggle Right optosensor (presentation rate photodiode)
- `8 / NumPad 8`: Read output text aloud (TTS)
- `9 / NumPad 9`: Accept TTS prediction text (reads whole string)
- `A-Z, Space, Backspace`: Trigger a manual "mock" decoder simulation in Online mode for testing.

---

## 11. Configuration Reference (Hardcoded Parameters)

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
| `keyboard_key_count` | `28` | Total number of keys on the on-screen keyboard |
| `keyboard_sequence_file_path` | `"codes/mgold_61_6521.txt"` | Original modulated m-sequence file. Automatically upsampled at runtime. |

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
| Resolve throttle | `2000` ms | How often to search for the decoder stream when disconnected (previously 5000ms, tightened for faster coupling) |

### `modules/server.c` — TCP Server Configuration
| Parameter | Default | Description |
|-----------|---------|-------------|
| `server_ip` | `"127.0.0.1"` | IP address the TCP server binds to |
| `server_port` | `8084` | TCP port the server listens on |

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
| `output_text_y` | `106.0` px | Vertical position of the output text |
| `output_text_color` | Green `(0,255,0)` | Color of the decoded output text |
| `predicted_text_color` | Gray `(128,128,128)` | Color of the predictive autocomplete text |
| `output_is_visible` | `1` (visible) | Whether the output text is shown at startup |

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
| `MAX_WORDS` | `20000` | Max number of words allocated in dictionary |
| `MAX_WORD_LENGTH` | `256` | Max character length per word line to prevent buffer overflows |
