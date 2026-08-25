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

This project strictly follows a **"Suckless" philosophy**, prioritizing simplicity, extreme performance, and minimal dependencies:
- **Unity Build:** No CMake, no Makefiles, no complex dependency trees. The entire project compiles via a single `gcc` command.
- **Zero-Blocking I/O:** All network operations (Dareplane TCP commands and Lab Streaming Layer (LSL) data streams) are strictly non-blocking. They never freeze the render loop, ensuring the 480 Hz visual stimulation remains flawless.
- **Lazy Loading Memory:** Hardcoded sequence arrays have been eliminated. M-sequences are lazy-loaded from `.txt` files directly into memory during the first frame.
- **Global Dependencies:** External libraries are installed globally into the compiler's environment rather than cluttering the project repository.

---

## 3. Ultimate Installation Guide (Windows)

To ensure maximum reliability, especially in laboratory environments where computers might use "Deep Freeze" (wiping the `C:\` drive upon every reboot), we use a fully portable compilation environment.

### Step 1: Portable MSYS2 & GCC
1. Download **MSYS2** from [msys2.org](https://www.msys2.org/). 
2. Install it directly to a persistent, non-wiped drive (e.g., `D:\alper\msys64`). This ensures your compiler survives reboots.
3. Open the **MSYS2 UCRT64** terminal and install the GCC compiler:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```

### Step 2: Install Graphics Libraries (SDL3)
In the same UCRT64 terminal, install SDL3 and its font extension:
```bash
pacman -S mingw-w64-ucrt-x86_64-SDL3 mingw-w64-ucrt-x86_64-SDL3_ttf
```

### Step 3: Global Lab Streaming Layer (LSL) Setup
Since `liblsl` is highly specific to BCI research, it is not available in the MSYS2 Pacman repository. We will embed it directly into your portable MSYS2 installation:
1. Download the Windows release of `liblsl` from its official GitHub repository.
2. Copy the library files into your MSYS2 UCRT64 directory:
   - Copy `lsl.dll` into `D:\alper\msys64\ucrt64\bin`
   - Copy `lsl.lib` (or `liblsl.a`) into `D:\alper\msys64\ucrt64\lib`
   - Copy `lsl_c.h` (and related headers) into `D:\alper\msys64\ucrt64\include`

*Your environment is now completely self-sufficient and portable!*

---

## 4. Running the Speller (Dareplane Integration)

The Speller is designed to be launched directly by the **Dareplane Control Room**. 

### The Python Wrapper (`main.py`)
To bridge Dareplane's Python ecosystem with our C application, we use a wrapper script (`main.py`). This script is incredibly powerful for lab environments:
- **Automatic Environment Injection:** Before running anything, `main.py` dynamically injects the persistent MSYS2 binary path (`D:\alper\msys64\ucrt64\bin`) into the runtime environment.
- **On-the-fly Compilation:** It compiles the latest C code into an `.exe` silently.
- **Execution:** It launches the Speller module.

You do not need to manually configure Windows PATH variables or run manual compilation scripts. Just launch it!

### Dareplane Configuration
Add the module to your Dareplane `example_cfg.toml` using the legacy python format:
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
├── main.c                    ← Main entry point, event loop, Unity Build wrapper
├── main.py                   ← Dareplane Python wrapper (handles runtime compilation)
├── modules/
│   ├── background.c          ← Background color rendering
│   ├── photodiode.c          ← Optosensor test squares (top-left & top-right)
│   ├── keyboard.c            ← Keyboard grid, state machine, lazy sequence loader
│   ├── events.c              ← SDL custom event system (TCP/LSL → main loop)
│   ├── server.c              ← Winsock2 TCP server (listens to Dareplane commands)
│   ├── lsl.c                 ← LSL outlet (marker stream) & non-blocking inlet (decoder)
│   ├── output.c              ← Rendered output text
│   ├── tts.c                 ← Windows SAPI Text-to-Speech (async via PowerShell)
│   └── fps.c                 ← Frame timing, drop detection, CSV logging
├── codes/                    ← Contains .txt files for m-sequences
├── fonts/                    ← Montserrat fonts (.ttf)
└── log.csv                   ← Frame-by-frame performance log created upon exit
```

### The Main Render Loop
Running at the hardware's exact refresh rate (e.g., 480 Hz), the main loop is structured for zero latency:
1. Check frame timings (`update_fps`)
2. Poll non-blocking TCP network for Dareplane commands (`handle_server`)
3. Poll LSL network for Decoder predictions (`handle_lsl_inlet`) - *Throttled strictly to prevent drops.*
4. Process custom SDL events (State transitions)
5. Render background, optosensor squares, output text, and the flashing keyboard grid.
6. Block on `SDL_RenderPresent()` exactly until the Vsync interrupt.

---

## 6. Communication Protocol

### LSL Markers (Outlet: `cvep-speller-stream`)
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
| `frame_dropped` | Sent immediately if the renderer misses a Vsync deadline. |

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

## 7. Controls & Keyboard Shortcuts
When testing manually or running offline experiments, use these physical keyboard shortcuts:
- `ESC`: Close program safely (and dump performance logs)
- `NumPad 1`: Switch to Idle Mode
- `NumPad 2`: Switch to Training Mode
- `NumPad 3`: Switch to Online Mode
- `NumPad 4 & 6`: Toggle Left/Right optosensor test squares for the photodiode
- `NumPad 5`: Toggle output text visibility
- `NumPad 8`: Read output text aloud (TTS)
- `A-Z, Space, Backspace`: Trigger a manual "mock" decoder simulation in Online mode for testing.
