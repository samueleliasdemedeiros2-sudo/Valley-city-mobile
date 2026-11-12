# Valley-city-mobile

## Build and tests

Install Android SDK Platform 35 and Build Tools 35, then build with:

```sh
./gradlew assembleDebug
./gradlew testDebugUnitTest
```

The Debug APK is written to `app/build/outputs/apk/debug/app-debug.apk`.

The app starts with a city selector, currently listing LAS VEGAS as SERVIDOR 01.
Selecting it only records the city in the UI: connection is not started until
the official server address and port are configured. A separate manual UDP
diagnostic tool remains available; sending a datagram does not confirm that a
server is online or that it received the packet. The SA-MP/open.mp handshake
and login flow are not implemented.

The city screen also opens a procedural native 3D prototype rendered with
OpenGL ES 2.0 through the Android NDK, CMake and JNI. Its geometric terrain,
roads, sidewalks and buildings are test geometry, not a GTA map, and it uses no
GTA assets or server connection.
