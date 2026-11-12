package com.valleycity.mobile

import java.io.Closeable
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress

data class UdpPacket(
    val address: String,
    val port: Int,
    val payload: ByteArray
)

class UdpTransport : Closeable {

    @Volatile
    private var socket: DatagramSocket? = null

    val localPort: Int
        get() = socket?.takeUnless { it.isClosed }?.localPort
            ?: error("UDP ainda não foi iniciado")

    @Synchronized
    fun open() {
        if (socket?.isClosed == false) return

        socket = DatagramSocket().apply {
            soTimeout = 3000
        }
    }

    fun send(host: String, port: Int, data: ByteArray) {
        require(port in 1..65535) { "Porta inválida" }

        val current = socket ?: error("UDP ainda não foi iniciado")
        val address = InetAddress.getByName(host)
        val packet = DatagramPacket(
            data,
            data.size,
            address,
            port
        )

        current.send(packet)
    }

    fun receive(): UdpPacket {
        val current = socket ?: error("UDP ainda não foi iniciado")

        val buffer = ByteArray(2048)
        val packet = DatagramPacket(buffer, buffer.size)

        current.receive(packet)

        return UdpPacket(
            address = packet.address.hostAddress ?: "",
            port = packet.port,
            payload = packet.data.copyOf(packet.length)
        )
    }

    @Synchronized
    override fun close() {
        socket?.close()
        socket = null
    }
}
