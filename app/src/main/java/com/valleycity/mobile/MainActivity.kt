package com.valleycity.mobile

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.view.ViewGroup
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView

class MainActivity : Activity() {

    private var selectedCity: City? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        selectedCity = savedInstanceState
            ?.getString(SELECTED_CITY_KEY)
            ?.let(CityCatalog::findById)
        renderScreen()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        selectedCity?.let { outState.putString(SELECTED_CITY_KEY, it.id) }
        super.onSaveInstanceState(outState)
    }

    private fun renderScreen() {
        val content = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(32, 32, 32, 32)
        }

        content.addView(TextView(this).apply {
            text = "VALLEY CITY"
            textSize = 30f
        }, matchWidth())

        val city = selectedCity
        if (city == null) {
            content.addView(TextView(this).apply {
                text = "SELECIONE SUA CIDADE"
                textSize = 18f
            }, matchWidth())

            CityCatalog.availableCities.forEach { availableCity ->
                content.addView(Button(this).apply {
                    text = "${availableCity.name}\n${availableCity.serverLabel}"
                    setOnClickListener {
                        selectedCity = availableCity
                        renderScreen()
                    }
                }, matchWidth())
            }
        } else {
            content.addView(TextView(this).apply {
                text = city.name
                textSize = 22f
            }, matchWidth())
            content.addView(TextView(this).apply {
                text = city.serverLabel
                textSize = 16f
            }, matchWidth())
            content.addView(TextView(this).apply {
                text = "Cidade selecionada. Conexão não iniciada: os dados oficiais do servidor ainda não foram configurados."
            }, matchWidth())
            content.addView(Button(this).apply {
                text = "Abrir protótipo 3D"
                setOnClickListener {
                    startActivity(Intent(this@MainActivity, Game3DActivity::class.java))
                }
            }, matchWidth())
            content.addView(Button(this).apply {
                text = "Voltar às cidades"
                setOnClickListener {
                    selectedCity = null
                    renderScreen()
                }
            }, matchWidth())
        }

        content.addView(Button(this).apply {
            text = "Ferramenta manual UDP (diagnóstico)"
            setOnClickListener {
                startActivity(Intent(this@MainActivity, ManualUdpActivity::class.java))
            }
        }, matchWidth())

        setContentView(content)
    }

    private fun matchWidth() = LinearLayout.LayoutParams(
        ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.WRAP_CONTENT
    )

    private companion object {
        const val SELECTED_CITY_KEY = "selected_city_id"
    }
}
