# CMakeLists.txt

# Declare app name
app(dht11_sensor)

# Add source file
sources(
    .\dht11_sensor\dht11_sensor.c
)

# (Optional) Set stack size (if needed)
# set_target_properties(dht11_sensor PROPERTIES STACK_SIZE 4096)

# Set app manifest
manifest(.\dht11_sensor\dht11_sensor app.manifest)

# End of file
