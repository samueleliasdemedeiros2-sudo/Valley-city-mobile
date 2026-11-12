package com.valleycity.mobile

import java.net.InetAddress
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Test

class UdpTransportTest {

    @Test
    fun sendsAndReceivesDatagramOnLoopback() {
        val receiver = UdpTransport()
        val sender = UdpTransport()
        val executor = Executors.newSingleThreadExecutor()

        try {
            receiver.open()
            sender.open()
            val pendingPacket = executor.submit<UdpPacket> { receiver.receive() }
            val expectedPayload = "udp-test".toByteArray(Charsets.UTF_8)

            sender.send(InetAddress.getLoopbackAddress().hostAddress!!, receiver.localPort, expectedPayload)

            val receivedPacket = pendingPacket.get(4, TimeUnit.SECONDS)
            assertArrayEquals(expectedPayload, receivedPacket.payload)
            assertEquals(sender.localPort, receivedPacket.port)
        } finally {
            sender.close()
            receiver.close()
            executor.shutdownNow()
        }
    }

    @Test
    fun rejectsInvalidDestinationPort() {
        val transport = UdpTransport()

        assertThrows(IllegalArgumentException::class.java) {
            transport.send("127.0.0.1", 0, byteArrayOf(1))
        }
    }

    @Test
    fun receiveFailsWhenTransportHasNotBeenOpened() {
        assertThrows(IllegalStateException::class.java) {
            UdpTransport().receive()
        }
    }
}
