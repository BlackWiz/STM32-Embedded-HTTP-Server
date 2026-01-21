```mermaid

flowchart LR
    CLIENT["Postman / curl<br/>(HTTP Client)"]
    LAN["Local Network<br/>(Ethernet)"]
    ENC["ENC28J60<br/>Ethernet Controller"]

    subgraph STM32["STM32 MCU"]
        ETHDRV["Ethernet Driver"]
        LWIP["lwIP Stack<br/>(NO_SYS)"]
        APP["Application Layer<br/>HTTP Parser<br/>JSON Parser<br/>Command Executor"]
        GPIO["GPIO Driver"]
    end

    LED["LED<br/>(Phase-1 Action)"]

    CLIENT -->|HTTP + JSON| LAN
    LAN -->|Ethernet Frames| ENC
    ENC -->|SPI| ETHDRV
    ETHDRV --> LWIP
    LWIP --> APP
    APP --> GPIO
    GPIO --> LED
