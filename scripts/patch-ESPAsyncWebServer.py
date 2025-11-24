# Because __has_includes doesn't work very well in GCC8, we need to remove its use in the ESPAsyncWebServer library

from os.path import join, isfile

targetPath = '.pio/libdeps/esp32-s3-devkitc-1/ESPAsyncWebServer/src/ESPAsyncWebServer.h'
targetPattern = '#if __has_include("ArduinoJson.h")'
replacementPattern = '#if 1'

try:
    with open(targetPath, 'r') as file:
        content = file.read()
    
    if targetPattern in content:
        newContent = content.replace(targetPattern, replacementPattern)
        with open(targetPath, 'w') as file:
            file.write(newContent)
    
except FileNotFoundError:
    print('Error: The file was not found.')
    raise 'Could not patch ESPAsyncWebServer.h'
