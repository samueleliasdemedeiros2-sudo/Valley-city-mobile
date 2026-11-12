package com.valleycity.mobile

data class City(
    val id: String,
    val name: String,
    val serverLabel: String
)

object CityCatalog {
    val availableCities = listOf(
        City(
            id = "las-vegas",
            name = "LAS VEGAS",
            serverLabel = "SERVIDOR 01"
        )
    )

    fun findById(id: String): City? = availableCities.find { it.id == id }
}
