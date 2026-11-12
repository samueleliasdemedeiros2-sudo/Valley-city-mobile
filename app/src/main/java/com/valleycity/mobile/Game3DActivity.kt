package com.valleycity.mobile

import android.app.Activity
import android.graphics.Color
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.Gravity
import android.view.MotionEvent
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.TextView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class Game3DActivity : Activity() {

    private lateinit var surfaceView: CameraSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        surfaceView = CameraSurfaceView()
        surfaceView.setEGLContextClientVersion(2)
        surfaceView.setRenderer(object : GLSurfaceView.Renderer {
            override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
                nativeSurfaceCreated()
            }

            override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
                nativeSurfaceChanged(width, height)
            }

            override fun onDrawFrame(gl: GL10?) {
                nativeDrawFrame()
            }
        })
        surfaceView.renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY

        setContentView(FrameLayout(this).apply {
            addView(
                surfaceView,
                FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT
                )
            )
            addView(TextView(this@Game3DActivity).apply {
                text = "Cenário 3D procedural — não é mapa GTA; sem conexão"
                setTextColor(Color.WHITE)
                setBackgroundColor(Color.argb(190, 24, 32, 42))
                setPadding(20, 12, 20, 12)
            }, FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.TOP or Gravity.CENTER_HORIZONTAL
            ))
        })
    }

    override fun onResume() {
        super.onResume()
        if (::surfaceView.isInitialized) surfaceView.onResume()
    }

    override fun onPause() {
        if (::surfaceView.isInitialized) surfaceView.onPause()
        super.onPause()
    }

    private external fun nativeSurfaceCreated()
    private external fun nativeSurfaceChanged(width: Int, height: Int)
    private external fun nativeDrawFrame()
    private external fun nativeOrbit(deltaX: Float, deltaY: Float)
    private external fun nativeZoom(scale: Float)

    private inner class CameraSurfaceView : GLSurfaceView(this@Game3DActivity) {
        private var previousX = 0f
        private var previousY = 0f
        private var previousPinchDistance = 0f

        override fun onTouchEvent(event: MotionEvent): Boolean {
            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN -> {
                    previousX = event.x
                    previousY = event.y
                    previousPinchDistance = 0f
                }

                MotionEvent.ACTION_POINTER_DOWN -> {
                    previousPinchDistance = pinchDistance(event)
                }

                MotionEvent.ACTION_MOVE -> {
                    if (event.pointerCount >= 2) {
                        val distance = pinchDistance(event)
                        if (previousPinchDistance > 0f && distance > 0f) {
                            val scale = distance / previousPinchDistance
                            queueEvent { nativeZoom(scale) }
                        }
                        previousPinchDistance = distance
                    } else {
                        val deltaX = event.x - previousX
                        val deltaY = event.y - previousY
                        queueEvent { nativeOrbit(deltaX, deltaY) }
                    }
                    previousX = event.x
                    previousY = event.y
                }

                MotionEvent.ACTION_POINTER_UP,
                MotionEvent.ACTION_UP,
                MotionEvent.ACTION_CANCEL -> {
                    previousPinchDistance = 0f
                }
            }
            return true
        }

        private fun pinchDistance(event: MotionEvent): Float {
            if (event.pointerCount < 2) return 0f
            val deltaX = event.getX(0) - event.getX(1)
            val deltaY = event.getY(0) - event.getY(1)
            return kotlin.math.sqrt(deltaX * deltaX + deltaY * deltaY)
        }
    }

    private companion object {
        init {
            System.loadLibrary("valleycity3d")
        }
    }
}
