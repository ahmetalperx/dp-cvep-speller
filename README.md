# C-VEP Speller (Dareplane C/SDL3 Port)

> Radboud University, Donders Institute  
> Supervisor: Dr. Jordy Thielen  
> Erasmus+ Internship Project

## 1. Project Goal
The existing Python/PsychoPy-based c-VEP Speller (`dp-cvep-speller`) loses millisecond precision at high refresh rates (480 Hz) and corrupts EEG signal quality. This project rewrites the exact same functionality in **pure C** using SDL3 to provide:
- Full frame synchronization with Vsync
- Automatic frame-skip time alignment when a frame drops
- A 100% compatible drop-in module for the Dareplane ecosystem

## 2. Requirements & Technologies
To compile and run this project, you need the following tools and libraries:

### System Requirements
- **OS:** Windows 10 / 11
- **Compiler:** GCC (Recommended via MSYS2 / MinGW-w64)

### Libraries
- **SDL3:** Graphics, window management, hardware-accelerated rendering, Vsync.
- **SDL3_ttf:** TTF Font rendering and text display.
- **liblsl (Lab Streaming Layer):** High-precision cross-device time synchronization and marker streaming.
- **Winsock2:** Dareplane TCP server integration (Windows native).
- **Windows SAPI:** Text-to-Speech (TTS) called asynchronously via PowerShell (no extra library needed).

## 3. Installation Guide (Windows)

This project strictly follows a "Suckless" philosophy. It has zero heavy framework dependencies (like CMake or heavy package managers) and compiles directly via a single `gcc` command.

### Step 1: Install MSYS2 & GCC
1. Download and install **MSYS2** from [msys2.org](https://www.msys2.org/). 
2. Open the **MSYS2 UCRT64** terminal.
3. Install the GCC compiler by running:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```

### Step 2: Install Graphics Libraries (SDL3)
In the same UCRT64 terminal, install SDL3 and its font extension:
```bash
pacman -S mingw-w64-ucrt-x86_64-SDL3 mingw-w64-ucrt-x86_64-SDL3_ttf
```

### Step 3: Configure Lab Streaming Layer (LSL)
Since `liblsl` is highly specific to BCI research, it is not available in the MSYS2 Pacman repository.
1. Download the Windows release of `liblsl` from its GitHub page.
2. Copy the library files into your MSYS2 UCRT64 directory to make them globally available for compilation:
   - Copy `lsl.dll` into `C:\msys64\ucrt64\bin`
   - Copy `lsl.lib` (or `liblsl.a`) into `C:\msys64\ucrt64\lib`
   - Copy `lsl_c.h` and the other header files into `C:\msys64\ucrt64\include`


### Step 4: Compile and Run
Open your UCRT64 terminal, navigate to the project directory, and compile using the single command line:
```bash
gcc main.c -l lsl -l ws2_32 -l SDL3 -l SDL3_ttf -l winmm -l avrt -o main.exe
./main.exe
```

---

### Advanced: Portable Lab Environment (e.g., Deep Freeze)
If you are deploying this project on a laboratory computer that wipes its `C:\` drive upon every reboot (e.g., Deep Freeze), you can create a fully portable, persistent compilation environment.

1. **Portable Installation:** When installing MSYS2, install it directly to a persistent drive (e.g., `D:\alper\msys64`). MSYS2 is 100% portable.
2. **Environment Variable Injection:** Instead of permanently modifying Windows `PATH` variables (which get wiped), the `main.py` Python wrapper automatically injects the portable MSYS2 binary folder (`D:\alper\msys64\ucrt64\bin`) into the shell environment at runtime before compiling and starting the Speller. 
There is no need for extra `.bat` scripts; running `python main.py` or launching via Dareplane handles the compilation and PATH injection seamlessly.

## 4. Directory Structure & Architecture
```text
brain_computer_interface_8/
├── main.c                    ← Main entry point, event loop, Unity Build wrapper
├── modules/
│   ├── background.c          ← Background color rendering
│   ├── photodiode.c          ← Optosensor test squares (top-left & top-right)
│   ├── keyboard.c            ← Keyboard grid, state machine, render, update
│   ├── events.c              ← SDL custom event system (TCP/LSL → main loop)
│   ├── server.c              ← Winsock2 TCP server (listens to Dareplane commands)
│   ├── lsl.c                 ← LSL outlet (marker stream) + inlet (decoder stream)
│   ├── output.c              ← Rendered output text
│   ├── tts.c                 ← Windows SAPI Text-to-Speech (non-blocking via PowerShell)
│   └── fps.c                 ← Frame timing, frame drop detection, CSV logging
├── fonts/                    ← Montserrat fonts (.ttf)
└── log.csv                   ← Frame-by-frame performance log created upon exit
```

**Compilation Command:**
```bash
gcc main.c -O3 -l lsl -l ws2_32 -l SDL3 -l SDL3_ttf -l winmm -l avrt -o main.exe
```
*Note: This project uses a "Unity Build" structure. All modules are included inside `main.c`. It compiles with a single command and minimizes external dependencies.*

## 5. Main Loop (Per Frame)
```text
On every Vsync interrupt:
├── update_fps()           → Update frame counters, send "frame_dropped" to LSL if needed
├── handle_server()        → Read TCP commands (TRAINING, ONLINE, STOP, CLOSE, etc.)
├── handle_lsl_inlet()     → Read predicted index from the decoder's LSL stream
├── SDL_PollEvent() loop:
│   ├── Keyboard strokes (ESC, NumPad, letters)
│   ├── Custom TCP/LSL events (idle, training, online, feedback, close)
│   └── Trigger state transitions & LSL markers accordingly
├── update_keyboard()      → Check state machine timers (duration elapsed?)
├── render_background()    → Render black background
├── render_photodiode()    → Render optosensor squares
├── render_output()        → Render typed text
├── render_keyboard()      → Render 28-key grid (with m-sequence modulation)
└── SDL_RenderPresent()    → Present to GPU (waits for Vsync)
```

## 6. LSL Marker Table
| Marker | Description |
|--------|-------------|
| `start_cue;label=X;key=Y` | Sent at the beginning of a cue phase. Contains target key index (`X`) and character (`Y`). |
| `stop_cue` | Sent when the cue phase ends. |
| `start_trial` | Sent when the stimulation (flashing) starts. Acts as the primary trigger for the decoder to epoch EEG data. |
| `stop_trial` | Sent when the stimulation ends. |
| `start_feedback;label=X;key=Y`| Sent when visual feedback is presented (based on decoder prediction). |
| `stop_feedback` | Sent when the feedback phase ends. |
| `start_iti` | Sent at the beginning of the Inter-Trial Interval (wait phase). |
| `stop_iti` | Sent at the end of the Inter-Trial Interval. |
| `frame_dropped` | Sent if a frame drop is detected, used for timing analysis and quality control. |

### LSL Stream Details
**Outlet (Marker Stream):**
- **Name:** `cvep-speller-stream`
- **Type:** `markers`
- **Format:** 1 channel, `cft_string`, `LSL_IRREGULAR_RATE`
- **Source ID:** `cvep-speller-stream-source-id`

**Inlet (Decoder Stream):**
- **Searched stream:** `name` = `cvep-decoder-stream`
- **Format:** `cf_int8` (single char, key index)
- **Polling:** Non-blocking, `timeout = 0.0`, resolved every 2 seconds (to prevent frame drops)

## 7. TCP Server (Dareplane Integration)
**Address:** `127.0.0.1:8084` (non-blocking Winsock2)

| Incoming Command | Triggered Action |
|------------------|------------------|
| `UP` | Health check (is module alive?) |
| `GET_PCOMMS` | Returns supported command list |
| `TRAINING` | Switches to Training mode (SDL custom event) |
| `ONLINE` | Switches to Online mode (SDL custom event) |
| `STOP` | Switches to Idle, sends stop marker for current state |
| `CLOSE` | Terminates the program |

## 8. Keyboard Shortcuts
- `ESC`: Close program
- `NumPad 1/2/3`: Idle / Training / Online mode
- `NumPad 4/6`: Toggle Left/Right optosensor test square
- `NumPad 5`: Toggle output text display
- `NumPad 8`: Read output text aloud (TTS)
- `A-Z, Space, Backspace`: Trigger manual mock decoder simulation in Online mode

## 9. Dareplane Control Room Configuration
Due to a known bug in the Dareplane Control Room config parser regarding `exe` modules, this C program is launched via a Python wrapper (`main.py`). It must be configured in `example_cfg.toml` using the legacy python format:

```toml
[python.modules.dp-cvep-speller]
cwd = 'C:/Users/ahmetalper/Documents/brain_computer_interface_8'
custom_entry_point = 'main'
ip = '127.0.0.1'
port = 8084
retry_after_s = 3.0
max_connect_retries = 10
```
