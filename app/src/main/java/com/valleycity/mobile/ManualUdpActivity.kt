package com.valleycity.mobile

import android.app.Activity
import android.os.Bundle
import android.text.InputType
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import java.io.IOException
import java.util.concurrent.Executors

class ManualUdpActivity : Activity() {

    private val networkExecutor = Executors.newSingleThreadExecutor()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val hostInput = EditText(this).apply {
            hint = "Host ou endereço IP"
            inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_URI
        }
        val portInput = EditText(this).apply {
            hint = "Porta UDP"
            inputType = InputType.TYPE_CLASS_NUMBER
        }
        val payloadInput = EditText(this).apply {
            hint = "Conteúdo do datagrama"
            inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_MULTI_LINE
        }
        val status = TextView(this).apply {
            text = "Ferramenta manual de diagnóstico UDP."
        }
        val sendButton = Button(this).apply {
            text = "Enviar datagrama UDP"
        }

        sendButton.setOnClickListener {
            val host = hostInput.text.toString().trim()
            val port = portInput.text.toString().toIntOrNull()
            val payload = payloadInput.text.toString()

            if (host.isEmpty()) {
                status.text = "Informe um host ou endereço IP."
            } else if (port == null || port !in 1..65535) {
                status.text = "Informe uma porta entre 1 e 65535."
            } else {
                sendButton.isEnabled = false
                status.text = "Enviando datagrama UDP..."
                networkExecutor.execute {
                    val result = try {
                        UdpTransport().use { transport ->
                            transport.open()
                            transport.send(host, port, payload.toByteArray(Charsets.UTF_8))
                        }
                        "Datagrama enviado. Isso não confirma conexão nem recebimento."
                    } catch (exception: IOException) {
                        "Falha ao enviar datagrama: ${exception.message ?: "erro de rede"}"
                    } catch (exception: IllegalArgumentException) {
                        "Falha ao enviar datagrama: ${exception.message ?: "parâmetros inválidos"}"
                    }

                    runOnUiThread {
                        if (!isFinishing && !isDestroyed) {
                            status.text = result
                            sendButton.isEnabled = true
                        }
                    }
                }
            }
        }

        setContentView(LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(32, 32, 32, 32)
            addView(hostInput, matchWidth())
            addView(portInput, matchWidth())
            addView(payloadInput, matchWidth())
            addView(sendButton, matchWidth())
            addView(status, matchWidth())
        })
    }

    override fun onDestroy() {
        networkExecutor.shutdownNow()
        super.onDestroy()
    }

    private fun matchWidth() = LinearLayout.LayoutParams(
        ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.WRAP_CONTENT
    )
}
