package com.example.aprsrouter

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.example.aprsrouter.ui.theme.APRSRouterDemoTheme

class MainActivity : ComponentActivity() {
    companion object {
        init {
            System.loadLibrary("aprsrouter") // .so file name (libaprsrouter.so)
        }
    }

    // Native function implemented in C++
    external fun routePacket(callsign: String, path: String, originalPacket: String): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            APRSRouterDemoTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    RoutingScreen(
                        modifier = Modifier.padding(innerPadding),
                        routePacket = { cs, p, pkt -> routePacket(cs, p, pkt) }
                    )
                }
            }
        }
    }
}

@Composable
fun RoutingScreen(
    modifier: Modifier = Modifier,
    routePacket: (String, String, String) -> String
) {
    val callsign = remember { mutableStateOf("DIGI") }
    val path = remember { mutableStateOf("WIDE1-1,WIDE2-2") }
    val originalPacket = remember { mutableStateOf("N0CALL>APRS,WIDE1-1,WIDE2-1:data") }
    val routedPacket = remember { mutableStateOf("<empty>") }

    Column(modifier = modifier.padding(16.dp)) {
        OutlinedTextField(
            value = callsign.value,
            onValueChange = { callsign.value = it },
            label = { Text("callsign") },
            modifier = Modifier.fillMaxWidth()
        )
        Spacer(modifier = Modifier.height(8.dp))
        OutlinedTextField(
            value = path.value,
            onValueChange = { path.value = it },
            label = { Text("path") },
            modifier = Modifier.fillMaxWidth()
        )
        Spacer(modifier = Modifier.height(8.dp))
        OutlinedTextField(
            value = originalPacket.value,
            onValueChange = { originalPacket.value = it },
            label = { Text("original packet") },
            modifier = Modifier.fillMaxWidth(),
            maxLines = 5,
            singleLine = false
        )
        Spacer(modifier = Modifier.height(8.dp))
        OutlinedTextField(
            value = routedPacket.value,
            onValueChange = { routedPacket.value = it },
            label = { Text("routed packet") },
            readOnly = true,
            modifier = Modifier.fillMaxWidth(),
            maxLines = 5,
            singleLine = false
        )
        Spacer(modifier = Modifier.height(16.dp))

        Button(onClick = {
            val result = routePacket(callsign.value, path.value, originalPacket.value)
            routedPacket.value = result
            println("C++ routed packet: $result")
        }) {
            Text("Route")
        }
    }
}

@Preview(showBackground = true)
@Composable
fun RoutingScreenPreview() {
    APRSRouterDemoTheme {
        RoutingScreen(routePacket = { _, _, _ -> "Mocked routed packet" })
    }
}
