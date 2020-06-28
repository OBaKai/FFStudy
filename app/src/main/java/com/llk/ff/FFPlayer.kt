package com.llk.ff

import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView

/**
 * author: llk
 * group: hudong
 * createDate: 2020-06-21
 * detail:
 */
class FFPlayer constructor(view: SurfaceView) : SurfaceHolder.Callback {

    companion object {
        init {
            System.loadLibrary("ff")
        }
    }

    //================= native 方法 =================

    external fun getAvCodecVersion() : String

    private external fun playFromNative(filePath: String, surface: Surface?)

    //================= native 方法 =================

    private var helper: SurfaceHolder? = null
    var isCreatedSurface: Boolean = false

    init {
        helper?.removeCallback(this)

        helper = view.holder
        helper?.addCallback(this)
    }

    fun play(filePath: String){
        playFromNative(filePath, helper?.surface)
    }


    override fun surfaceChanged(_holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
        helper = _holder

    }

    override fun surfaceDestroyed(_holder: SurfaceHolder?) {
        isCreatedSurface = false
    }

    override fun surfaceCreated(_holder: SurfaceHolder?) {
        isCreatedSurface = true
    }
}