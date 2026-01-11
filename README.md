# Classics Live 64

**Classics Live 64** is a suite of tools that allows a real Nintendo 64 console to connect to the Classics Live website. This enables earning achievements and submitting leaderboard entries directly from original hardware.

## Requirements
* Nintendo 64 console
* Nintendo 64 Expansion Pak
* Compatible flashcart

  * *Currently supported:* **SummerCart 64 only**
* USB-C cable
* Computer with Internet access

## Setup & Usage
1. Download the following files from the Releases page:
   * `sc64menu.n64`
   * `payload.bin`

2. Copy both files to the root directory of the SD card used by your flashcart.

3. Download and launch **cl64-host** for your operating system:

4. Edit the provided `login.txt` file and enter your Classics Live username and password.

5. Connect your **SummerCart 64** to your computer using a USB-C cable.

6. Launch a compatible game via **N64FlashcartMenu**.

## Limitations

* The Classics Live 64 payload injects itself into the Expansion Pak memory. As such, games that require or utilize the Expansion Pak are **currently not supported**.

## Future Plans

* Support for additional USB-enabled flashcarts, including:

  * EverDrive-64 X7
  * EverDrive-64 V3
