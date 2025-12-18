# 🎨 Preview Giao diện WindowsBehaviorMonitor v1.0

## Main Menu Preview

```
  ╔═══════════════════════════════════════════════════════════════════╗
  ║                                                                   ║
  ║        █     █ █ █   █ ███  ███  █   █ ███                       ║
  ║        █  █  █ █ ██  █ █  █ █  █ █   █ █                         ║
  ║        ███████ █ █ █ █ █  █ █  █ █ █ █ ███                       ║
  ║        █     █ █ █  ██ █  █ █  █ █ █ █ █                         ║
  ║        █     █ █ █   █ ███  ███  ███ █ ███                       ║
  ║                                                                   ║
  ║              BEHAVIOR MONITOR SYSTEM v1.0                         ║
  ║          User-Mode Endpoint Detection & Response Tool            ║
  ╚═══════════════════════════════════════════════════════════════════╝


  ─────────────────────────────────────────────────────────────────────
  Status: All modules idle
  ─────────────────────────────────────────────────────────────────────

  ┌───────────────────────────────────────────────────────────────────┐
  │                         MAIN MENU                                 │
  └───────────────────────────────────────────────────────────────────┘

  【 MODULE SELECTION 】

    [1] PMM - Process & Memory Monitoring
        └─ Monitor processes and detect memory injection

    [2] PFM - Persistence & File-system Monitoring
        └─ Monitor Registry & Startup folder changes

    [3] NMM - Network Monitoring Module
        └─ Monitor TCP connections in real-time

    [0] Exit Program

  ────────────────────────────────────────────────────────────────────
  ► Select option: _
```

## PMM Module Menu

```
  ╔═══════════════════════════════════════════════════════════════════╗
  ║        █     █ █ █   █ ███  ███  █   █ ███                       ║
  ║              BEHAVIOR MONITOR SYSTEM v1.0                         ║
  ╚═══════════════════════════════════════════════════════════════════╝

  ┌───────────────────────────────────────────────────────────────────┐
  │         PMM - Process & Memory Monitoring Module                  │
  └───────────────────────────────────────────────────────────────────┘

    [1] List All Running Processes
    [2] Scan Single Process Memory
    [3] Enable SeDebugPrivilege (Administrator)
    [4] Scan All Processes for Suspicious Memory
    [5] Test Memory Scanner (MockMalwareSim)

    [0] Back to Main Menu

  ────────────────────────────────────────────────────────────────────
  ► Select option: _
```

## PFM Module Menu

```
  ┌───────────────────────────────────────────────────────────────────┐
  │      PFM - Persistence & File-system Monitoring Module           │
  └───────────────────────────────────────────────────────────────────┘

    [1] Start Registry & Startup Monitoring (Real-time)
    [2] Stop Monitoring
    [3] Show Registry Baseline (HKCU\Run)
    [4] List Startup Folder Files

    [0] Back to Main Menu
```

## NMM Module Menu

```
  ┌───────────────────────────────────────────────────────────────────┐
  │            NMM - Network Monitoring Module                        │
  └───────────────────────────────────────────────────────────────────┘

    [1] Start Network Monitoring (Real-time)
    [2] Stop Monitoring
    [3] Display Current TCP Connections

    [0] Back to Main Menu
```

## Example Output - Process List

```
  ┌─── Running Processes ───────────────────────────────────────────┐

  PID     ParentPID   CreationTimeTicks       ProcessName
  ───────────────────────────────────────────────────────────────────
  0       0           0                       System Idle Process
  4       0           133123456789012345      System
  108     4           133123456789123456      Registry
  1234    4           133123456789234567      svchost.exe
  5678    4           133123456789345678      chrome.exe
  ...

  ✓ Total processes: 145
```

## Example Output - Memory Scan

```
  ┌─── Scan Process Memory ─────────────────────────────────────────┐

  Enter Process ID (PID): 1234

  Scanning process 1234...

  ⚠ WARNING: Suspicious memory regions detected:

  Suspicious region: Base=0x00007FF123450000, Size=0x1000
  Suspicious region: Base=0x00007FF123460000, Size=0x2000
```

## Example Output - Real-time Monitoring

```
  ✓ PFM Monitoring started!
  Monitoring:
    • Registry: HKCU\Software\Microsoft\Windows\CurrentVersion\Run
    • Startup Folder: %APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup

  Check console output for alerts...

  [ALERT] New Run value: MaliciousApp -> C:\malware\bad.exe
  [ALERT] Startup File Added: suspicious.lnk
```

## Status Bar - Active Modules

```
  ─────────────────────────────────────────────────────────────────────
  Status: PMM:ON PFM:ON NMM:ON
  ─────────────────────────────────────────────────────────────────────
```

## Color Scheme

- **Cyan (11)** - Banner và prompts
- **Yellow (14)** - Headers và titles
- **Green (10)** - Success messages, NMM menu
- **Red (12)** - Errors, warnings, Exit options
- **Magenta (13)** - PMM menu
- **Gray (8)** - Status bar, hints
- **White (7)** - Default text

## Icons Used

- ✓ - Success
- ✗ - Error/Failed
- ⚠ - Warning
- ► - Prompt
- • - List item
- └─ - Submenu indicator
- [!] - Alert

## Features Highlight

✨ **Beautiful ASCII Banner**
- Eye-catching header
- Professional look

📊 **Real-time Status Bar**
- Shows which modules are running
- Updates dynamically

🎨 **Color-coded Menus**
- Each module has distinct color
- Easy to identify current section

🔄 **Multi-threading Support**
- Can run multiple modules simultaneously
- Non-blocking real-time monitoring

🎯 **Clean Navigation**
- Clear menu hierarchy
- Easy to go back to main menu

---
**This is how your Windows Behavior Monitor will look! 🚀**
