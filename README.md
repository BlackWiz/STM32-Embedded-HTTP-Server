# HTTP-JSON-Assistant

## Overview

HTTP-JSON-Assistant is a bare-metal embedded HTTP server on STM32G071RB that receives JSON commands over Ethernet and executes hardware actions deterministically.

## Phase-1 MVP Focus

Validating the communication and execution pipeline:
- RCC: System clock configuration (64 MHz)
- GPIO: LED control and SPI pin configuration
- SPI: Interrupt-driven communication with ENC28J60
- Delay: SysTick-based timing
- Network: lwIP stack (NO_SYS mode)
- Protocol: HTTP request parsing
- Command: JSON parser and executor

See [MVP.md](MVP.md) for detailed phase breakdown.

## Build Instructions

### Prerequisites
- ARM GCC toolchain (`arm-none-eabi-gcc`)
- OpenOCD for flashing
- ST-Link debugger
- STM32G071RB development board

### Build
```bash
make clean
make  all
```

### Flash to Target
```bash
make flash
```

### View Disassembly
```bash
make disasm
```

## Project Structure

```
HTTP-JSON-Assistant/
├── src/                 # Source code
│   ├── drivers/         # Hardware drivers (RCC, GPIO, SPI, etc.)
│   ├── net/             # lwIP network stack
│   ├── protocol/        # HTTP parser
│   └── app/             # JSON parser, command executor
├── tests/               # TDD test infrastructure
│   ├── unit/            # Driver unit tests
│   ├── integration/     # Hardware integration tests
│   └── system/          # End-to-end tests
├── linker/              # Linker script
└── docs/                # Private documentation (not in git)
```

## Development Philosophy

- **Bare-metal**: No HAL, direct register access
- **TDD**: Test every 10-20 lines before proceeding
- **State machines**: Interrupt-driven with explicit state tracking
- **Application-specific**: Drivers for actual hardware, not generic library

## License

Private project for learning and interviews.

/***end of file ***/
