# Space Invaders Intel 8080 Emulator

A custom Intel 8080 CPU emulator written in C++ designed to natively execute the original Space Invaders arcade ROM. The project is integrated with a macOS Cocoa frontend to handle real-time framebuffer rendering, input mapping, and audio playback.

**Watch the demo here on YouTube:** [Gameplay Showcase](https://youtu.be/2DM2j7QYyKo)

## Features

* **Cycle-Accurate CPU Emulation:** Custom implementation of the Intel 8080 instruction set, managing registers, condition codes, and stack pointers.
* **Native macOS Frontend:** Built with Objective-C++ and Cocoa for a lightweight, responsive desktop application experience.
* **Precise Hardware Timing:** Emulates the original 2 MHz arcade processor speed with accurate 60Hz and 120Hz interrupt generation for half-screen and full-screen drawing.
* **Dedicated Hardware Support:** Includes implementation of the specific shift register hardware used in the original Midway arcade cabinets.
* **Memory-Mapped Rendering:** Reads directly from the emulated `0x2400` memory address to render the framebuffer in real-time.
* **Trigger-Based Audio:** Evaluates I/O port state changes to trigger native macOS audio playback for game events (shooting, explosions, fleet movement).

## System Architecture

The project is split into a backend core and a frontend wrapper to keep the emulation logic platform-agnostic:

* **`shellhead.cpp` (Core Emulator):** Contains the complete C++ emulation logic. It handles the CPU execution loop, memory management, flag calculations (Zero, Sign, Parity, Carry, Auxiliary Carry), and decodes all 8080 opcodes.
* **`SpaceInvadersMachine.mm` (Cocoa Wrapper):** An Objective-C++ bridge connecting the CPU state to the macOS environment. It calculates frame timings, schedules CPU execution intervals, maps keyboard inputs to the emulator's memory space, and manages the audio subsystem.

## Controls

Make sure to insert coins before attempting to start the game.

* **Insert Coin:** `C`
* **Start Game:** `Enter`
* **Move Left:** `Left Arrow`
* **Move Right:** `Right Arrow`
* **Shoot:** `Spacebar`

## Setup & Requirements

To build and run this project, you will need:
* macOS with Xcode installed.
* The original Space Invaders arcade ROM, specifically named `invaders`, placed in the application bundle.
* The required audio files (`0.wav` through `8.wav`) placed in the application bundle to support sound effects like the UFO, shooting, and fleet movements.
