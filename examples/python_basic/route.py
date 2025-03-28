import ctypes
import os
import sys
import platform

# get the current working directory
current_dir = os.path.dirname(os.path.abspath(__file__))

# get the path to the dll
if platform.system() == "Windows":
    dll_path = os.path.join(current_dir, "libroute.dll")
elif platform.system() == "Linux":
    dll_path = os.path.join(current_dir, "libroute.so")
elif platform.system() == "Darwin":
    dll_path = os.path.join(current_dir, "libroute.dylib")
else:
    raise Exception("Unsupported platform")

# load the dll
libroute = ctypes.CDLL(dll_path)

# Define the argument and return types of the exported function
libroute.try_route_packet.argtypes = [
    ctypes.c_char_p,  # packet_string
    ctypes.c_char_p,  # router_callsign_string
    ctypes.c_char_p,  # router_path_string
    ctypes.c_char_p,  # router_packet_string
    ctypes.POINTER(ctypes.c_size_t)  # buffer_size
]
libroute.try_route_packet.restype = ctypes.c_bool

# Wrapper function for try_route_packet
def try_route_packet(packet_string, router_callsign_string, router_path_string):
    buffer_size = ctypes.c_size_t(1024)  # Initial buffer size
    router_packet_string = ctypes.create_string_buffer(buffer_size.value)

    success = libroute.try_route_packet(
        packet_string.encode('utf-8'),
        router_callsign_string.encode('utf-8'),
        router_path_string.encode('utf-8'),
        router_packet_string,
        ctypes.byref(buffer_size)
    )

    if not success:
        raise Exception("Failed to route packet or buffer size is insufficient.")

    return router_packet_string.value.decode('utf-8')

# Test the function
if __name__ == "__main__":
    packet_string = "N0CALL>APRS,WIDE1-1,WIDE2-1:data"
    router_callsign_string = "DIGI"
    router_path_string = "WIDE1-1,WIDE2-2"

    try:
        result = try_route_packet(packet_string, router_callsign_string, router_path_string)
        print(f"Routed Packet: {result}")
    except Exception as e:
        print(f"Error: {e}")