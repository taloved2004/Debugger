# Custom Debugger for C/C++ Applications

## Overview

Welcome to my custom debugger for C/C++ applications! This project is a powerful debugging tool designed to help developers gain better insights into the execution of their C/C++ programs. Whether you're preparing for an interview or simply looking to improve your debugging skills, this debugger can be an invaluable asset in your toolkit.

## Features

- **Function Breakpoint**: You can specify a function's name within an executable file, and the debugger will automatically place a breakpoint at the beginning of that function. This feature allows you to halt program execution at the desired location.

- **Register Inspection**: The debugger provides the ability to inspect the values of registers when the program execution is paused at a breakpoint. This can be immensely helpful for tracking variable values and understanding how your program is behaving at the assembly level.

- **Memory Address Inspection**: You can also examine the value stored at a specific memory address while the program is paused. This feature is useful for pinpointing memory-related issues or verifying data at specific locations.

- **Linux Environment**: This debugger is designed to work seamlessly in a Linux environment. It leverages the power of the Linux debugging tools to provide a robust debugging experience.

- **Project Planning and Development Skills**: During the development of this project, I acquired various skills, including project planning, software architecture, debugging techniques, and proficiency in Linux environments. 

## Getting Started

To use this debugger, follow these steps:

1. Clone this repository to your local machine:

   ```bash
   git clone https://github.com/taloved2004/Debugger.git
   ```

2. Build the debugger by running the build script:

   ```bash
   chmod +x ./build.sh
   ./build.sh
   ```
   
3. Run the debugger, specifying the target executable :

   ./debug your-program.exe

4. You will then be asked for the function's name which you want to set a breakpoint.

5. Use the debugger's commands to inspect registers and memory as needed.

## Usage

The debugger provides a set of commands to control and inspect the program execution. Some of the common commands include:

- `run`: Start/Continue the program execution.
- `step`: Execute the program one instruction at a time.
- `registers`: View the current values of CPU registers.
- `mem 0x***`: Examine the value at a specific memory address.
- `--help`: View the set of commands available.
- `quit`: Exit the debugger.

Refer to the documentation or help menu for a full list of available commands and their usage.

## Contributions

Contributions to this project are welcome. If you have ideas for improvements or encounter any issues, please open an issue on GitHub. Feel free to fork the repository and submit pull requests as well.


## Contact

If you have any questions or found a bug, you can reach me at [taloved2004@gmail.com](mailto:taloved2004@gmail.com).

Thank you for checking out my custom debugger project! I hope you find it helpful in your debugging endeavors.