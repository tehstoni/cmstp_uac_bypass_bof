## Overview

The CMSTP BOF leverages the auto-elevation functionality of the Windows Connection Manager Profile Installer (CMSTP.exe) to bypass UAC restrictions. It uses a specially crafted INF file to execute arbitrary commands with elevated privileges.

### How It Works

1. Generates a malicious INF file containing the specified command
2. Uses CMSTP.exe to "install" the INF file, which auto-elevates due to Windows trusted binary status
3. Handles window interactions automatically to complete the installation
4. Executes the specified command with elevated privileges
5. Cleans up temporary files

## Project Components

- `cmstp.c` - Main BOF implementation
- `beacon.h` - Cobalt Strike beacon API definitions
- `cmstp.cna` - Aggressor script for Cobalt Strike integration
- `build.sh` - Build script to compile the BOF

## Building

To build the BOF:

1. Ensure you have the required build environment (mingw-w64)
2. Run the build script:
```bash
./build.sh
```

## Installation

1. Copy all files to your Cobalt Strike installation:
```
cmstp.cna
cmstp.x64.o
```

2. Load the script in your Cobalt Strike client:
```
Cobalt Strike -> Scripts -> Load
```

## Usage

From a Beacon prompt in Cobalt Strike:

```
beacon> help cmstp_bof
Usage: cmstp_bof <command>

beacon> cmstp_bof "cmd.exe /c whoami > C:\temp\test.txt"
```

## Technical Details

The BOF implementation:
- Creates a temporary INF file with the specified command
- Leverages CMSTP.exe's auto-elevation capabilities
- Handles GUI automation for installation dialogs
- Implements proper cleanup procedures
- Provides detailed debug output for troubleshooting
