const ffi = require('ffi-napi');
const ref = require('ref-napi');
const path = require('path');
const os = require('os');

// Determine the correct shared library name
let libName;
switch (os.platform()) {
  case 'win32':
    libName = 'route.dll';
    break;
  case 'darwin':
    libName = 'libroute.dylib';
    break;
  default:
    libName = 'libroute.so';
}

// Construct full path to the shared library
const libPath = path.join(__dirname, libName);

// Define C types
const size_t = ref.types.size_t;
const bool = ref.types.bool;
const size_tPtr = ref.refType(size_t);
const charPtr = ref.types.CString;

// Load the shared library
let lib;
try {
  lib = ffi.Library(libPath, {
    'try_route_packet': [bool, [charPtr, charPtr, charPtr, charPtr, size_tPtr]]
  });
} catch (err) {
  console.error('❌ Failed to load library:', err.message);
  process.exit(1);
}

// Prepare input strings
const packet_string = "N0CALL>APRS,WIDE1-1,WIDE2-1:data";
const router_callsign_string = "DIGI";
const router_path_string = "WIDE1-1,WIDE2-2";

// Prepare output buffer and size
const bufferSize = 1024;
const outputBuffer = Buffer.alloc(bufferSize);
const sizeRef = ref.alloc(size_t, bufferSize);

// Call the C function
const success = lib.try_route_packet(
  packet_string,
  router_callsign_string,
  router_path_string,
  outputBuffer,
  sizeRef
);

// Output result
if (success) {
  const actualSize = sizeRef.deref();
  const result = outputBuffer.toString('utf8', 0, actualSize);
  console.log('✅ Routed packet:', result);
} else {
  console.log('❌ Routing failed.');
}
