# Arduino batch testing

Batch testing Arduinos using I2C.

## Examples

For building and flasing on the command line (e.g. in Windows PowerShell or Bash).

### To upload code to master arduino

```
cd master
platformio run -e master --upload-port /dev/ttyUSB0 -t upload
```

Or, to set `MAX_SLAVES` manually:

```
cd master
export PLATFORMIO_BUILD_FLAGS='-MAX_SLAVES=28'
platformio run -e master --upload-port /dev/ttyUSB0 -t upload
```

### To upload code to slave arduinos

The following can be used to flash a slave with a chosen ID.

```
# Slave ID 1
cd slave_target
platformio run -e slave_target_1 --upload-port /dev/ttyUSB1 -t upload

# Slave ID 2
cd slave_target
platformio run -e slave_target_2 --upload-port /dev/ttyUSB1 -t upload

# Slave ID 3
cd slave_target
platformio run -e slave_target_3 --upload-port /dev/ttyUSB1 -t upload

# Slave ID 4
cd slave_target
platformio run -e slave_target_4 --upload-port /dev/ttyUSB1 -t upload

# Slave with manually set ID
cd slave_target
export PLATFORMIO_BUILD_FLAGS='-SLAVE_ID=0'
platformio run -e slave_target --upload-port /dev/ttyUSB1 -t upload
```
