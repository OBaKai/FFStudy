package com.llk.ff

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.Toast
import kotlinx.android.synthetic.main.activity_main.*


class MainActivity : AppCompatActivity(), View.OnClickListener {

    lateinit var ff: FFPlayer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)
        btn_play.setOnClickListener(this)

        ff = FFPlayer(surface_view)
        btn_play.text = "播放按钮（版本：${ff.getAvCodecVersion()}）"
    }

    override fun onClick(v: View?) {
        when(v?.id){
            R.id.btn_play -> {
                if (ff.isCreatedSurface) {
                    ff.play(FileUtils.getAssetsCacheFile(this, "aa.mp4"))
                }else{
                    Toast.makeText(this, "surface还没初始化完", Toast.LENGTH_SHORT)
                        .show()
                }
            }
        }
    }
}
