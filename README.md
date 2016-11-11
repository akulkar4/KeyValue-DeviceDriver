# KeyValue-DeviceDriver
A loadable kernel module that creates a pseudo device which maintains a key-value store in the kernel and allows different processes to share data by accessing this device.

## Directions to compile
1. Clone the repo.
2. Change to the library & kernel module directory to compile the library and load the module
3. Include keyvalue.h to access the device by means of the library functions. 
