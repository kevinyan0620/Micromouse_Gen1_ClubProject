# Autonomous Micromouse (Generation 1)

This repository details the hardware and firmware design of a Generation 1 autonomous Micromouse. Built from scratch, the project encompasses component selection, schematic capture, 2-layer PCB layout, and embedded C programming. 

Here is some pictures of our PCB design and our Micromouse!

| Our Group Logo | Final Micromouse |
| --- | --- |
|<img width="350" height="400" alt="Design_Pictures" src="https://github.com/user-attachments/assets/0988e14d-979a-4d74-b5b3-c1779c3ea999" /> |<img width="509" height="378" alt="Micromouse_Pictures" src="https://github.com/user-attachments/assets/f26f483a-c1f1-401f-9c03-cb9af995729a" /> |

* **Competition Result:** Placed **5th Nationally** and **2nd Schoolwide** at the All American Micromouse Competition (AAMC).
* **See It In Action:** [Watch our competition run on AAMC Live (starts at 51:30)](https://youtu.be/dCLFL7v4gUA?t=3090)

We are actively iterating on this design to compete again in the upcoming AAMC!

## Repository Structure

* `/Micromouse_STM32ApplicationCode`: Contains the embedded C firmware and project files configured for the STM32 environment.

* `/The Mouse`: The master KiCad project directory. Includes the full system schematics (wiring) and the 2-layer PCB layout files.

* `/Micromouse_Parts`: Custom KiCad footprint and symbol libraries created for this project. These can be directly imported into KiCad for viewing or reuse.

* `/Pictures`: Photographic documentation of the hardware assembly, revisions, and competition runs.

* `Final Pinout.png`: A comprehensive STM32 pin mapping reference diagram required to understand the hardware-firmware interface.

* `Micromouse_BillOfMaterials_AssemblyVersion.xlsx`: A detailed assembly BOM mapped by reference designator (e.g., R10) and subsystem, used for manufacturing guidance and population.

* `Micromouse_BillOfMaterials_SubmitVersion.xlsx`: A consolidated purchasing BOM with grouped components and quantities, optimized for vendor procurement (e.g., Digi-Key).

* `Test_Run_With_Small_Maze.MOV`: Video demonstration of the maze-solving algorithm in a scaled-down test environment prior to the competition.

## Hardware Architecture

The electrical hardware was designed to balance form factor with sensor coverage and ease of routing. Components were carefully sourced via Digi-Key, maintaining a strict Bill of Materials (BOM) throughout the project lifecycle.

*   **Microcontroller:** STM32 MCU (STM32F205RET7) serving as the core processor, with pinouts optimized during the layout phase to minimize trace crossings and accommodate physical wiring constraints.
*   **PCB Design:** A fully hand-soldered, custom 2-layer board. Components were sourced via Digi-Key with a comprehensive Bill of Materials (BOM) managed throughout the project lifecycle.
*   **Sensor Array:** Custom 45-degree angled IR sensor placement for superior wall-detection accuracy compared to standard orthogonal layouts.
*   **Inertial Measurement:** Adafruit BNO085 9-DOF IMU (Gyroscope/Accelerometer) integrated to ensure highly accurate, drift-compensated turns.
*   **User Interface:** 4 onboard user LEDs and 2 tactile buttons for rapid field-diagnostics and mode switching.

## Firmware and Control Systems

The software stack is written entirely in bare-metal C, focusing on real-time control and algorithmic efficiency.

*   **Navigation:** Implemented a flood-fill algorithm for autonomous maze solving and optimal path calculation.
*   **Motor Control:** Closed-loop PID control utilizing encoder feedback to maintain precise velocities.
*   **Traction Management:** Engineered acceleration and deceleration profiles to prevent wheel slip, ensuring accurate encoder tick counts.
*   **Physical Realignment:** The mouse was designed to purposefully "tap" the wall during the run once every 10 actions (forward or turning), when appropriate. This is done by moving forward to tap the front wall, then slightly back up to the center of the maze. This helps realign the micromouse in the maze and avoid tiny errors from accumulating and ruining the run.
*   **Operational Modes:** Designed a dual-mode system allowing a seamless transition from "Search Mode" (active IR and flood-fill) to "Speed Run Mode" (sensorless execution of the stored optimal path).

## Assembly, Debugging, and Rework

Building the physical hardware required extensive hands-on debugging, microscope soldering, and creative problem-solving.

| Challenge | Root Cause | Engineering Solution |
| :--- | :--- | :--- |
| **Broken PCB Pad** | Mechanical stress / heat | Reworked using a microscope and microscopic gauge wire to bridge the broken connection. |
| **MCU Assembly** | Fine-pitch footprint | Successfully hand-soldered the tiny MCU using heavy flux, a microscope, and drag-soldering techniques within two attempts. |
| **IR Sensor Verification** | Invisible IR spectrum | Utilized a smartphone camera to detect the IR emitter's dim purple glow, and a phone flashlight (which emits IR) to stimulate and test the IR receivers. |

## Lessons Learned & Generation 2 Roadmap

This first iteration provided invaluable lessons in Design for Manufacturability (DFM) and edge-case software handling. The following improvements are slated for the Generation 2 Micromouse:

*   **Component Selection:** Transitioning from Surface Mount (SMD) IR sensors to Through-Hole (THT) alternatives. SMD optoelectronics proved highly susceptible to thermal damage during hand-soldering.
*   **Connector Standardization:** Replacing the non-standard ST-Link header with a standard 0.1-inch pitch 1x4 connector to streamline firmware flashing.
*   **Firmware Edge Cases:** Patching a known bug that occasionally causes indefinite looping turns under extreme, undocumented maze conditions.
*   **Path Optimization:** Upgrading the Speed Run mode algorithm to calculate and execute diagonal movements, significantly reducing cornering time.
