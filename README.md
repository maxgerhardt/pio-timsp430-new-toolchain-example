# PlatformIO example for new TI MSP430 Toolchain

## What is this? 

This is an example which tests a modified version of PlatformIO's current https://github.com/platformio/platform-timsp430. The current one uses a quite old toolchain (based on GCC 4.6.3). The new modified one uses the GCC 9.3.1 [released by TI](https://www.ti.com/tool/MSP430-GCC-OPENSOURCE).

The modified platform also uses the large memory model by default. See the [manual](https://www.ti.com/lit/ug/slau646f/slau646f.pdf) for details, especially regarding
* `-mlarge`
* `-mcode-region=either`
* `-mdata-region=either` 

and their implications regarding code / data placement and induced overhead.

The example code demonstrates the usage of high flash memory (hi-ROM), above 0x10000, which is normally not accessible with the old platform, since the compiler doesn't understand the large memory model, at least in my tests. Placement of the functions in the code is controlled by the `FORCE_HIGHMEM` macro and the [compiler](https://github.com/maxgerhardt/platform-timsp430/blob/438d35feb6ae3a756b3a8d6a3a83e0e0b0325748/builder/main.py#L62-L64) and [linker](https://github.com/maxgerhardt/platform-timsp430/blob/438d35feb6ae3a756b3a8d6a3a83e0e0b0325748/builder/main.py#L80-L82) settings.

Based on a question at https://community.platformio.org/t/msp430-use-all-128kb-not-just-47kb/21326/. 

## Usage

Clone / download the project and use PlatformIO Home -> Open Project. 

Downloading the new platform code and compiler toolchain requires having [git](https://git-scm.com/downloads) installed.

## Caveats

Some of the caveats I've encountered:
* Do not have a `msp430f5529.ld` file in the root of the project, otherwise the compiler will implicitly take that, and that file might not necessarily support the data / code relocation sections. Using the one built-in with the compiler is fine.
* In order to be able to use `#include <msp430f5529.h>`, I had to download the `Header and Support Files` from [TI](https://www.ti.com/tool/download/MSP430-GCC-OPENSOURCE), since they were not included in the "- toolchain only" download. The files had to placed in the toolchain folder according to the [manual](https://www.ti.com/lit/ug/slau646f/slau646f.pdf)
* peripheral names, register flags etc. differ from code for old toolchains(?) Code from e.g. https://www.electrodummies.net/de/msp430-uart-tutorial/ did not compile without changes (which included guessing the correct peripheral -- not 100% if this is correct)
* new TI toolchain download is only provided for Windows in https://github.com/maxgerhardt/pio-toolchaintimsp430-new -- other OSes can be added easily by downloading the correct toolchain version from TI, adding a modified `package.json` and pointing to it using `platform_packages`