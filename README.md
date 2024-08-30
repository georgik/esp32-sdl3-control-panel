# Control Panle for ESP32-S3-BOX-3 and P4 based on


## Requirements

`idf_component_manager` 2.x - install manually

## Build

```
git clone git@github.com:georgik/esp32-sdl3-control-panel.git
cd esp32-sdl3-control-panel

idf.py @boards/esp-box-3.cfg set-target esp32-s3
idf.py @boards/esp-box-3.cfg build
```

### Other boards

- ESP32-S3-BOX-3
```shell
idf.py @boards/esp-box-3.cfg reconfigure
```

- ESP32-S3-BOX (prior Dec. 2023)
```shell
idf.py @boards/esp-box.cfg reconfigure
```

- ESP32-P4
```shell
idf.py @boards/esp32_p4_function_ev_board.cfg reconfigure
```

- M5Stack-CoreS3
```shell
idf.py @boards/m5stack_core_s3.cfg reconfigure
```

## Credits

- FreeSans.ttf - https://github.com/opensourcedesign/fonts/blob/master/gnu-freefont_freesans/FreeSans.ttf
